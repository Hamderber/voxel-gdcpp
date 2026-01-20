#pragma once

#include "block.hpp"
#include "constants.hpp"
#include "godot_cpp/variant/vector3.hpp"
#include "godot_cpp/variant/vector3i.hpp"
#include "hpp/tools/log_stream.hpp"
#include "hpp/tools/string.hpp"
#include "resource/pallet.hpp"
#include <cstdint>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace Voxel
{
    class World;

    class Chunk : public godot::Node3D
    {
        GDCLASS(Chunk, Node3D)
    public:
        Chunk() = default;
        ~Chunk() override = default;

        void _ready() override;
        void _exit_tree() override;

        void set_pallet(godot::Ref<Resource::Pallet> p_pallet) { m_pallet = p_pallet; }
        void set_world_position(World *pWorld, int x, int y)
        {
            m_pWorld = pWorld;

            m_pos = godot::Vector2i(x, y);
            set_position(godot::Vector3i(m_pos.x, 0, m_pos.y));
            Tools::Log::debug() << "Set chunk " << this << " to " << Tools::String::xy_to_string(x, y)
                                << " in world " << pWorld << ".";
        }
        const Block *get_block_at(godot::Vector3 p_pos);
        const Block *get_block_at(uint32_t x, uint32_t y, uint32_t z);
        inline size_t get_block_index_local(uint32_t x, uint32_t y, uint32_t z) const
        {
            return x +
                   static_cast<size_t>(z) * CHUNK_AXIS_LENGTH_U +
                   static_cast<size_t>(y) * (CHUNK_AXIS_LENGTH_U * CHUNK_AXIS_LENGTH_U);
        }
        const godot::Vector2i get_pos() { return m_pos; }

        void generate_blocks();

    protected:
        static void _bind_methods() {}
        void _notification(int p_what);

    private:
        void initialize_block_data();
        void remesh();
        void ensure_instance();
        void sync_instance_transform();

        bool m_isInitialized = false;

        World *m_pWorld;

        godot::Ref<Resource::Pallet> m_pallet;
        godot::Ref<godot::ArrayMesh> m_mesh;
        godot::RID m_instance_rid;

        godot::Vector2i m_pos;
        std::unique_ptr<Voxel::Block *[]> m_pBlocks;
    };
} //namespace Voxel
