#include "hpp/tools/material.hpp"
#include "godot_cpp/classes/ref.hpp"
#include "godot_cpp/classes/standard_material3d.hpp"

namespace Tools
{
    godot::Ref<godot::StandardMaterial3D> Material::m_material = nullptr;
} //namespace Tools
