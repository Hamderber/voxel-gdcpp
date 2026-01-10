#pragma once

#include "godot_cpp/classes/material.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"
#include "hpp/tools/log.hpp"
#include <sstream>
#include <string_view>

namespace Tools
{
    class Material
    {
    public:
        static godot::Ref<godot::StandardMaterial3D> get_default_material()
        {
            if (!m_material.is_valid())
            {
                godot::Ref<godot::StandardMaterial3D> mat3d;

                mat3d.instantiate();
                mat3d->set_albedo(godot::Color(1.f, 0.f, 0.86f));
                mat3d->set_name("UNKNOWN_MATERIAL");

                m_material = mat3d;

                Tools::Log::debug("Created default UNKNOWN_MATERIAL.");
            }

            return m_material;
        }

        static void EnsureDefaultMaterial(godot::Ref<godot::StandardMaterial3D> &p_material, std::string_view p_objName = "an object")
        {
            if (p_material.is_valid())
                return;

            p_material = get_default_material();

            std::stringstream ss;
            ss << "Set material to default for " << p_objName << ".";
            Tools::Log::debug(ss.str());
        }

    private:
        static godot::Ref<godot::StandardMaterial3D> m_material;
    };
} //namespace Tools
