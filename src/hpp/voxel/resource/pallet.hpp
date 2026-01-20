#pragma once

#include "godot_cpp/classes/resource.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include <godot_cpp/core/class_db.hpp>

namespace Voxel::Resource
{
    class Pallet : public godot::Resource
    {
        GDCLASS(Pallet, godot::Resource);

    public:
        enum MaterialType
        {
            TYPE_UNKNOWN = 0,
            TYPE_GENERIC,
            TYPE_GLASS,
            TYPE_METAL,
            TYPE_COUNT
        };

        godot::Ref<godot::StandardMaterial3D> get_material(int p_type) const;

        godot::Ref<godot::StandardMaterial3D> get_unknown_material() const { return get_material(TYPE_UNKNOWN); }
        void set_unknown_material(godot::Ref<godot::StandardMaterial3D> m)
        {
            m_materials[TYPE_UNKNOWN] = m;
            emit_changed();
        }

        godot::Ref<godot::StandardMaterial3D> get_generic_material() const { return get_material(TYPE_GENERIC); }
        void set_generic_material(godot::Ref<godot::StandardMaterial3D> m)
        {
            m_materials[TYPE_GENERIC] = m;
            emit_changed();
        }

        godot::Ref<godot::StandardMaterial3D> get_glass_material() const { return get_material(TYPE_GLASS); }
        void set_glass_material(godot::Ref<godot::StandardMaterial3D> m)
        {
            m_materials[TYPE_GLASS] = m;
            emit_changed();
        }

        godot::Ref<godot::StandardMaterial3D> get_metal_material() const { return get_material(TYPE_METAL); }
        void set_metal_material(godot::Ref<godot::StandardMaterial3D> m)
        {
            m_materials[TYPE_METAL] = m;
            emit_changed();
        }

        void default_pallet();

    protected:
        static void _bind_methods();

    private:
        godot::Ref<godot::StandardMaterial3D> m_materials[TYPE_COUNT];
    };
} //namespace Voxel::Resource
