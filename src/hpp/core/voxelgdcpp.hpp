#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"

class VoxelGDCPP : public godot::Node
{
    GDCLASS(VoxelGDCPP, godot::Node)
protected:
    static void _bind_methods();

private:
    void initialize();
    void test();
};
