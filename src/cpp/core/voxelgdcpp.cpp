#include "hpp/core/voxelgdcpp.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "hpp/tools/log.hpp"
#include "hpp/tools/log_stream.hpp"
#include <filesystem>

void VoxelGDCPP::initialize()
{
    auto root = std::filesystem::current_path();
    Tools::Log::begin("VoxelGDCPP", root.string(), true, Tools::Log::Level::Debug);

    Tools::Log::debug() << "Root: " << root.generic_string();
}

void VoxelGDCPP::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("initialize"), &VoxelGDCPP::initialize);
}
