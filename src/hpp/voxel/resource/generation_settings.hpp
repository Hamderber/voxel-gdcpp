#pragma once

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/variant/string.hpp"
#include <cstdint>
#include <godot_cpp/core/class_db.hpp>

namespace Voxel::Resource
{
    class GenerationSettings : public godot::Resource
    {
        GDCLASS(GenerationSettings, godot::Resource);

    public:
        float get_frequency() const { return m_frequency; }
        void set_frequency(float v)
        {
            m_frequency = godot::CLAMP(v, 0.0001f, 10.f);
            emit_changed();
        }

        int32_t get_octaves() const { return m_octaves; }
        void set_octaves(int32_t v)
        {
            m_octaves = godot::CLAMP(v, 1, 12);
            emit_changed();
        }

    protected:
        static void _bind_methods()
        {
            godot::ClassDB::bind_method(godot::D_METHOD("get_frequency"), &GenerationSettings::get_frequency);
            godot::ClassDB::bind_method(godot::D_METHOD("set_frequency", "v"), &GenerationSettings::set_frequency);

            ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "frequency", godot::PROPERTY_HINT_RANGE, "0.0001,10,0.0001"),
                         "set_frequency", "get_frequency");

            godot::ClassDB::bind_method(godot::D_METHOD("get_octaves"), &GenerationSettings::get_octaves);
            godot::ClassDB::bind_method(godot::D_METHOD("set_octaves", "v"), &GenerationSettings::set_octaves);

            ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "octaves", godot::PROPERTY_HINT_RANGE, "1,12,1"),
                         "set_octaves", "get_octaves");
        }

    private:
        float m_frequency = 0.01f;
        int32_t m_octaves = 4;
    };
} //namespace Voxel::Resource
