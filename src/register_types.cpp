#include "register_types.h"

#include "unit.h"
// #include "action.h"
#include "surface_element.h"
#include "surface.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

// TODO TEST FUNCTIONS WITH CUSTOM CLASSES

using namespace godot;

void initialize_example_module(ModuleInitializationLevel p_level)
{
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS)
    {
        ClassDB::register_class<SurfaceElement>();
        ClassDB::register_class<Unit>();
        ClassDB::register_class<LosCheckResult>();
        ClassDB::register_abstract_class<AbstractCollisionProvider>();
        ClassDB::register_abstract_class<AbstractPathfindingProvider>();
        // ClassDB::register_class<Surface>();
    }
}

void uninitialize_example_module(ModuleInitializationLevel p_level)
{
}

extern "C"
{
    // Initialization.
    GDExtensionBool GDE_EXPORT example_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_example_module);
        init_obj.register_terminator(uninitialize_example_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);

        return init_obj.init();
    }
}