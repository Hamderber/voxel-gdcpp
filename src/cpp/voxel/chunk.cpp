#include "hpp/voxel/chunk.hpp"
#include "godot_cpp/classes/random_number_generator.hpp"
#include "hpp/tools/material.hpp"
#include <cstdlib>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/aabb.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/color.hpp>
#include <vector>

using namespace godot;

namespace Voxel
{
    // TODO: Split this apart and stop using a monolithic hello world chunk

    static constexpr float CHUNK_AXIS_LENGTH = 16.f;

    void Chunk::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("get_material"), &Chunk::get_material);
        ClassDB::bind_method(D_METHOD("set_material", "material"), &Chunk::set_material);

        ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"),
                     "set_material", "get_material");
    }

    void Chunk::_ready()
    {
        set_notify_transform(true);
        Tools::Material::EnsureDefaultMaterial(m_material, "HelloChunk");
        m_rng.instantiate();
        build_dummy_chunk_mesh();
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

        pRenderingServer->instance_set_custom_aabb(m_instance_rid, AABB(Vector3(0, 0, 0),
                                                                        Vector3(CHUNK_AXIS_LENGTH, CHUNK_AXIS_LENGTH, CHUNK_AXIS_LENGTH)));
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

    godot::Ref<godot::StandardMaterial3D> Chunk::get_material() const
    {
        return m_material;
    }

    void Chunk::set_material(const godot::Ref<godot::StandardMaterial3D> &material)
    {
        m_material = material;

        // If mesh already exists, apply immediately
        if (m_mesh.is_valid() && m_mesh->get_surface_count() > 0)
        {
            m_mesh->surface_set_material(0, m_material);
        }
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

    void Chunk::build_dummy_chunk_mesh()
    {
        m_mesh.instantiate();

        PackedVector3Array vertices;
        PackedVector3Array vertex_normals;
        PackedVector2Array uvs;
        PackedInt32Array indices;

        const int L = CHUNK_AXIS_LENGTH;
        const size_t count = static_cast<size_t>(L) * static_cast<size_t>(L) * static_cast<size_t>(L);

        // 1) Randomly fill solids
        std::vector<uint8_t> solid(count, 0);

        auto idx_of = [L](int x, int y, int z) -> size_t
        {
            return static_cast<size_t>(x + L * (y + L * z));
        };

        for (int z = 0; z < L; ++z)
        {
            for (int y = 0; y < L; ++y)
            {
                for (int x = 0; x < L; ++x)
                {
                    // 50/50 solid vs air
                    solid[idx_of(x, y, z)] = (m_rng->randi_range(0, 1) != 0) ? 1 : 0;
                }
            }
        }

        auto is_solid = [&](int x, int y, int z) -> bool
        {
            if (x < 0 || x >= L || y < 0 || y >= L || z < 0 || z >= L)
                return false;
            return solid[idx_of(x, y, z)] != 0;
        };

        // 2) Emit only visible faces for solid voxels
        for (int z = 0; z < L; ++z)
        {
            for (int y = 0; y < L; ++y)
            {
                for (int x = 0; x < L; ++x)
                {
                    if (!is_solid(x, y, z))
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
                    if (z == L - 1 || !is_solid(x, y, z + 1))
                        add_face(vertices, vertex_normals, uvs, indices, p001, p101, p111, p011, Vector3(0, 0, 1));

                    // -Z (back)
                    if (z == 0 || !is_solid(x, y, z - 1))
                        add_face(vertices, vertex_normals, uvs, indices, p100, p000, p010, p110, Vector3(0, 0, -1));

                    // +X (right)
                    if (x == L - 1 || !is_solid(x + 1, y, z))
                        add_face(vertices, vertex_normals, uvs, indices, p101, p100, p110, p111, Vector3(1, 0, 0));

                    // -X (left)
                    if (x == 0 || !is_solid(x - 1, y, z))
                        add_face(vertices, vertex_normals, uvs, indices, p000, p001, p011, p010, Vector3(-1, 0, 0));

                    // +Y (top)
                    if (y == L - 1 || !is_solid(x, y + 1, z))
                        add_face(vertices, vertex_normals, uvs, indices, p011, p111, p110, p010, Vector3(0, 1, 0));

                    // -Y (bottom)
                    if (y == 0 || !is_solid(x, y - 1, z))
                        add_face(vertices, vertex_normals, uvs, indices, p000, p100, p101, p001, Vector3(0, -1, 0));
                }
            }
        }

        // 3) Build mesh (guard: could be empty)
        if (indices.size() > 0)
        {
            Array arrays;
            arrays.resize(Mesh::ARRAY_MAX);
            arrays[Mesh::ARRAY_VERTEX] = vertices;
            arrays[Mesh::ARRAY_NORMAL] = vertex_normals;
            arrays[Mesh::ARRAY_TEX_UV] = uvs;
            arrays[Mesh::ARRAY_INDEX] = indices;

            m_mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

            m_mesh->surface_set_material(0, m_material);
        }
    }
} //namespace Voxel
