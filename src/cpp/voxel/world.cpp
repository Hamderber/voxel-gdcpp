#include "hpp/voxel/world.hpp"
#include "godot_cpp/classes/timer.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/callable.hpp"
#include "hpp/tools/log.hpp"
#include "hpp/tools/material.hpp"

using namespace godot;

namespace Voxel
{
    void World::_ready()
    {
        build_debounce_timer();
        Tools::Material::EnsureDefaultMaterial(m_material, "World");
        default_generation_settings();
        subscribe_to_signals();
    }

    void World::_exit_tree() {}

    void World::_bind_methods()
    {
        godot::ClassDB::bind_method(godot::D_METHOD("get_view_distance"), &World::get_render_distance);
        godot::ClassDB::bind_method(godot::D_METHOD("set_view_distance", "v"), &World::set_render_distance);

        std::stringstream ss;
        ss << "1," << MAX_RENDER_DISTANCE << ",suffix:Chunks";
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "view_distance", godot::PROPERTY_HINT_RANGE, ss.str().c_str()),
                     "set_view_distance", "get_view_distance");

        godot::ClassDB::bind_method(godot::D_METHOD("get_seed"), &World::get_seed);
        godot::ClassDB::bind_method(godot::D_METHOD("set_seed", "s"), &World::set_seed);
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "seed"), "set_seed", "get_seed");

        godot::ClassDB::bind_method(godot::D_METHOD("get_material"), &World::get_material);
        godot::ClassDB::bind_method(godot::D_METHOD("set_material", "m"), &World::set_material);
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "voxel_material", godot::PROPERTY_HINT_RESOURCE_TYPE, "material"),
                     "set_material", "get_material");

        godot::ClassDB::bind_method(godot::D_METHOD("get_settings"), &World::get_settings);
        godot::ClassDB::bind_method(godot::D_METHOD("set_settings", "g"), &World::set_settings);
        ADD_PROPERTY(godot::PropertyInfo(godot::Variant::OBJECT, "settings", godot::PROPERTY_HINT_RESOURCE_TYPE, "GenerationSettings"),
                     "set_settings", "get_settings");

        godot::ClassDB::bind_method(godot::D_METHOD("request_rebuild"), &World::request_rebuild);
        godot::ClassDB::bind_method(godot::D_METHOD("rebuild"), &World::rebuild);
        godot::ClassDB::bind_method(godot::D_METHOD("rebuild_debounce_timer"), &World::rebuild_debounce_timer);
    }

    void World::build_debounce_timer()
    {
        m_pDebounceTimer = memnew(Timer);
        m_pDebounceTimer->set_wait_time(DEBOUNCE_DELAY);
        m_pDebounceTimer->set_one_shot(true);
        add_child(m_pDebounceTimer);

        // The resource being built itself signals the request_rebuild which would crash the editor if the timer isn't
        // fully in the scene tree yet. (This was a hard bug do find, do not change this without triple checking!!)
        call_deferred("rebuild_debounce_timer");
    }

    void World::rebuild_debounce_timer()
    {
        m_pDebounceTimer->stop();
        m_pDebounceTimer->start();
    }

    void World::default_generation_settings()
    {
        if (m_generationSettings.is_valid())
            return;

        m_generationSettings.instantiate();

        Tools::Log::debug("Created default world generation settings.");
    }

    void World::subscribe_to_signals()
    {
        m_pDebounceTimer->connect("timeout", Callable(this, "rebuild"));
        m_generationSettings->connect("changed", godot::Callable(this, "request_rebuild"));

        Tools::Log::debug("World subscribed to signal(s).");
    }

    void World::request_rebuild()
    {
        if (!is_inside_tree())
            return;

        if (!m_pDebounceTimer)
        {
            rebuild_debounce_timer();
        }
        else
        {
            m_pDebounceTimer->stop();
            m_pDebounceTimer->set_wait_time(DEBOUNCE_DELAY);
            m_pDebounceTimer->start();
        }

        Tools::Log::debug("World rebuild requested!");
    }

    void World::rebuild()
    {
        // TODO: Un-dirty
        m_dirty = true;
        Tools::Log::debug("World rebuild executed!");
    }
} //namespace Voxel
