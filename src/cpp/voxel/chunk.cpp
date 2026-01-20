#include "hpp/voxel/chunk.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"
#include "hpp/tools/log_stream.hpp"
#include "hpp/tools/string.hpp"
#include "hpp/voxel/constants.hpp"
#include "hpp/voxel/world.hpp"
#include <cstdint>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

namespace Voxel
{
    void Chunk::_ready()
    {
        set_notify_transform(true);
        initialize_block_data();
        ensure_instance();
        sync_instance_transform();
    }

    void Chunk::_exit_tree()
    {
        RenderingServer *pRenderingServer = RenderingServer::get_singleton();

        if (pRenderingServer && m_instance_rid.is_valid())
        {
            pRenderingServer->free_rid(m_instance_rid);
            m_instance_rid = RID();
        }
    }

    void Chunk::_notification(int p_what)
    {
        if (p_what == NOTIFICATION_TRANSFORM_CHANGED)
        {
            sync_instance_transform();
        }
    }

    void Chunk::ensure_instance()
    {
        if (m_instance_rid.is_valid())
        {
            return;
        }

        RenderingServer *pRenderingServer = RenderingServer::get_singleton();
        if (!pRenderingServer)
            return;

        Ref<World3D> world = get_world_3d();
        if (world.is_null())
            return;

        const RID scenario = world->get_scenario();
        if (!scenario.is_valid())
            return;

        m_instance_rid = pRenderingServer->instance_create2(m_mesh->get_rid(), scenario);
        // RenderingServer::get_singleton()->instance_set_base(m_instance_rid, m_mesh->get_rid());
        pRenderingServer->instance_set_custom_aabb(m_instance_rid, AABB(CHUNK_AAA(), CHUNK_BBB()));
        pRenderingServer->instance_set_visible(m_instance_rid, true);
    }

    void Chunk::initialize_block_data()
    {
        m_pBlocks = std::make_unique<Block *[]>(CHUNK_BLOCK_COUNT_MAX);

        // Don't just use zero-initialized because the world is the owner of when to actually build the chunk
        for (uint32_t i{}; i < CHUNK_BLOCK_COUNT_MAX; i++)
        {
            auto block = new Block();
            block->set_solid(false);
            m_pBlocks[i] = block;
        }

        remesh();
    }

    void Chunk::sync_instance_transform()
    {
        if (!m_instance_rid.is_valid())
            return;

        RenderingServer *rs = RenderingServer::get_singleton();
        if (!rs)
            return;

        rs->instance_set_transform(m_instance_rid, get_global_transform());
    }

    static void add_face(
            PackedVector3Array &vertices,
            PackedVector3Array &vertex_normals,
            PackedVector2Array &uvs,
            PackedInt32Array &indices,
            const Vector3 &v0,
            const Vector3 &v1,
            const Vector3 &v2,
            const Vector3 &v3,
            const Vector3 &normal)
    {
        const int base_index = vertices.size();

        vertices.push_back(v0);
        vertices.push_back(v1);
        vertices.push_back(v2);
        vertices.push_back(v3);

        vertex_normals.push_back(normal);
        vertex_normals.push_back(normal);
        vertex_normals.push_back(normal);
        vertex_normals.push_back(normal);

        uvs.push_back(Vector2(0, 0));
        uvs.push_back(Vector2(1, 0));
        uvs.push_back(Vector2(1, 1));
        uvs.push_back(Vector2(0, 1));

        // Clockwise winding for Godot
        indices.push_back(base_index + 0);
        indices.push_back(base_index + 2);
        indices.push_back(base_index + 1);

        indices.push_back(base_index + 0);
        indices.push_back(base_index + 3);
        indices.push_back(base_index + 2);
    }

    const Block *Chunk::get_block_at(godot::Vector3 p_pos)
    {
        return get_block_at(p_pos.x, p_pos.y, p_pos.z);
    }

    const Block *Chunk::get_block_at(uint32_t x, uint32_t y, uint32_t z)
    {
        if (!m_pBlocks)
        {
            Tools::Log::error() << "Attempted to access block at "
                                << Tools::String::xyz_to_string(x, y, z)
                                << " but the chunk's block data wasn't initialized.";
        }

        return m_pBlocks[get_block_index_local(x, y, z)];
    }

    void Chunk::generate_blocks()
    {
        const uint32_t XZ = CHUNK_AXIS_LENGTH_U;
        const uint32_t Y = CHUNK_HEIGHT_U;

        auto rng = m_pWorld->get_generation_rng();
        auto settings = m_pWorld->get_settings();
        auto sea_level = settings->get_sea_level();

        for (int y = 0; y < Y; y++)
        {
            for (int z = 0; z < XZ; z++)
            {
                for (int x = 0; x < XZ; x++)
                {
                    // 1/20 solid vs air at/below sea level, 1 / 100 above
                    auto block = new Block();
                    block->set_solid(rng->randi_range(1, y < sea_level ? 20 : 100) == 1);
                    m_pBlocks[get_block_index_local(x, y, z)] = block;
                }
            }
        }

        Tools::Log::debug() << "(Re)generated blocks for chunk at " << Tools::String::to_string(m_pos) << ".";

        remesh();
    }

    void Chunk::remesh()
    {
        if (m_mesh.is_valid() && m_mesh->get_surface_count() > 0)
        {
            m_mesh->clear_surfaces();
        }
        else
        {
            m_mesh.instantiate();
        }

        PackedVector3Array vertices;
        PackedVector3Array vertex_normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const uint32_t XZ = CHUNK_AXIS_LENGTH_U;
        const uint32_t Y = CHUNK_HEIGHT_U;

        for (int y = 0; y < Y; y++)
        {
            for (int z = 0; z < XZ; z++)
            {
                for (int x = 0; x < XZ; x++)
                {
                    auto block = get_block_at(x, y, z);
                    if (!block || !block->is_solid())
                        continue;

                    const Vector3 o(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

                    // Unit cube corners at this voxel
                    const Vector3 p000 = o + Vector3(0, 0, 0);
                    const Vector3 p100 = o + Vector3(1, 0, 0);
                    const Vector3 p110 = o + Vector3(1, 1, 0);
                    const Vector3 p010 = o + Vector3(0, 1, 0);

                    const Vector3 p001 = o + Vector3(0, 0, 1);
                    const Vector3 p101 = o + Vector3(1, 0, 1);
                    const Vector3 p111 = o + Vector3(1, 1, 1);
                    const Vector3 p011 = o + Vector3(0, 1, 1);

                    // +Z (front)
                    if (z == XZ - 1 || !get_block_at(x, y, z + 1)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p001, p101, p111, p011, Vector3(0, 0, 1));

                    // -Z (back)
                    if (z == 0 || !get_block_at(x, y, z - 1)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p100, p000, p010, p110, Vector3(0, 0, -1));

                    // +X (right)
                    if (x == XZ - 1 || !get_block_at(x + 1, y, z)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p101, p100, p110, p111, Vector3(1, 0, 0));

                    // -X (left)
                    if (x == 0 || !get_block_at(x - 1, y, z)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p000, p001, p011, p010, Vector3(-1, 0, 0));

                    // +Y (top)
                    if (y == Y - 1 || !get_block_at(x, y + 1, z)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p011, p111, p110, p010, Vector3(0, 1, 0));

                    // -Y (bottom)
                    if (y == 0 || !get_block_at(x, y - 1, z)->is_solid())
                        add_face(vertices, vertex_normals, uvs, indices, p000, p100, p101, p001, Vector3(0, -1, 0));
                }
            }
        }

        if (indices.size() > 0)
        {
            Array arrays;
            arrays.resize(Mesh::ARRAY_MAX);
            arrays[Mesh::ARRAY_VERTEX] = vertices;
            arrays[Mesh::ARRAY_NORMAL] = vertex_normals;
            arrays[Mesh::ARRAY_TEX_UV] = uvs;
            arrays[Mesh::ARRAY_INDEX] = indices;

            m_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

            auto generic_material = m_pallet->get_generic_material();

            if (generic_material.is_valid() && m_mesh->get_surface_count() > 0)
            {
                m_mesh->surface_set_material(0, generic_material);
            }
        }

        if (m_instance_rid.is_valid())
        {
            auto rs = RenderingServer::get_singleton();
            rs->instance_set_base(m_instance_rid, m_mesh->get_rid());
            rs->instance_set_visible(m_instance_rid, true);
        }

        if (m_mesh.is_valid() && m_mesh->get_surface_count() > 0)
        {
            Tools::Log::debug() << "Mesh has " << m_mesh->get_surface_count()
                                << " surfaces and " << m_mesh->surface_get_array_len(0) << " vertices "
                                << "for chunk" << Tools::String::to_string(m_pos);
        }
        else
        {
            Tools::Log::debug() << "Mesh is empty for chunk " << Tools::String::to_string(m_pos) << ".";
        }
    }
} //namespace Voxel
