#pragma once

#include "godot_cpp/classes/random_number_generator.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/rid.hpp>

namespace Voxel
{
    class Chunk : public godot::Node3D
    {
        GDCLASS(Chunk, Node3D)

    public:
        Chunk() = default;
        ~Chunk() override = default;

        void _ready() override;
        void _exit_tree() override;

    protected:
        static void _bind_methods();

        void _notification(int p_what);

    private:
        void build_dummy_chunk_mesh();
        void ensure_instance();
        void sync_instance_transform();

        godot::Ref<godot::StandardMaterial3D> get_material() const;
        void set_material(const godot::Ref<godot::StandardMaterial3D> &material);

    private:
        godot::Ref<godot::StandardMaterial3D> m_material;
        godot::Ref<godot::RandomNumberGenerator> m_rng;
        godot::Ref<godot::ArrayMesh> m_mesh;
        godot::RID m_instance_rid;
    };
} //namespace Voxel
