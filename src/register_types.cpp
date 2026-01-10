#include "hpp/core/voxelgdcpp.hpp"
#include "hpp/voxel/chunk.hpp"
#include "hpp/voxel/resource/generation_settings.hpp"
#include "hpp/voxel/world.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdextension_types(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    GDREGISTER_CLASS(VoxelGDCPP)

    // GDREGISTER_CLASS(Voxel::Chunk)
    ClassDB::register_class<Voxel::Chunk>();
    // GDREGISTER_CLASS(Voxel::World)
    ClassDB::register_class<Voxel::World>();
    // GDREGISTER_CLASS(Voxel::Resource::GenerationSettings)
    ClassDB::register_class<Voxel::Resource::GenerationSettings>();
}

void uninitialize_gdextension_types(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C"
{
    // Initialization
    GDExtensionBool GDE_EXPORT voxelgdcppext_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                                                  GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
    {
        GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
        init_obj.register_initializer(initialize_gdextension_types);
        init_obj.register_terminator(uninitialize_gdextension_types);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
