#include "hpp/core/voxelgdcpp.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "hpp/tools/log.hpp"

void VoxelGDCPP::initialize()
{
    auto root = std::filesystem::current_path();
    Tools::Log::begin("VoxelGDCPP", root, true, Tools::Log::Level::Debug);

    Tools::Log::debug() << "Root: " << root.generic_string();
    Tools::Log::warn() << "Test";
    Tools::Log::error() << "Test";

    auto num = 5;
}

void VoxelGDCPP::test() { Tools::Log::debug() << "Test"; }

void VoxelGDCPP::_bind_methods()
{
    godot::ClassDB::bind_method(godot::D_METHOD("initialize"),
            &VoxelGDCPP::initialize);
    godot::ClassDB::bind_method(godot::D_METHOD("test"), &VoxelGDCPP::test);
}
