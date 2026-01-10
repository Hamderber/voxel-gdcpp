#pragma once

#include "godot_cpp/classes/material.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "resource/generation_settings.hpp"
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <hpp/tools/log.hpp>

namespace Voxel
{
    namespace Resource
    {
        class GenerationSettings;
    }

    class World : public godot::Node3D
    {
        GDCLASS(World, godot::Node3D)

        static const int32_t MAX_RENDER_DISTANCE = 64;

    public:
        World() = default;
        ~World() override = default;

        void _ready() override;
        void _exit_tree() override;

        int32_t get_render_distance() const { return m_renderDistance; }
        void set_render_distance(int32_t v)
        {
            m_renderDistance = godot::MAX(v, 1);
            request_rebuild();
        }

        int64_t get_seed() const { return m_seed; }
        void set_seed(int64_t s)
        {
            m_seed = s;
            request_rebuild();
        }

        godot::Ref<godot::Material> get_material() const { return m_material; }
        void set_material(const godot::Ref<godot::Material> &m)
        {
            m_material = m;
            request_rebuild();
        }

        godot::Ref<Resource::GenerationSettings> get_settings() const { return m_generationSettings; }
        void set_settings(const godot::Ref<Resource::GenerationSettings> &g)
        {
            m_generationSettings = g;
            request_rebuild();
        }

    protected:
        static void _bind_methods();

    private:
        void build_debounce_timer();
        void rebuild_debounce_timer();
        void default_generation_settings();
        void subscribe_to_signals();

        void request_rebuild();
        void rebuild();

        godot::Timer *m_pDebounceTimer;
        const double DEBOUNCE_DELAY = 1.5;

        int32_t m_renderDistance = 6;
        int32_t m_chunkSize = 16;
        int64_t m_seed = 8675309;
        godot::Ref<godot::StandardMaterial3D> m_material;
        godot::Ref<Resource::GenerationSettings> m_generationSettings;
        bool m_dirty = true;
    };
} //namespace Voxel
