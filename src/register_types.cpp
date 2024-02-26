#include "register_types.h"

#include "unit.h"
#include "surface_element.h"
#include "surface.h"
#include "action.h"
#include "handlers.h"
#include "ortbinding.h"
#include "destructible_element.h"
#include "action_bundle.h"
#include "actor.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_example_module(ModuleInitializationLevel p_level)
{
    if (p_level != GDEXTENSION_INITIALIZATION_CORE)
        return;

    register_handlers();
    register_combinations();

    ClassDB::register_class<SurfaceElement>();
    ClassDB::register_class<Unit>();
    ClassDB::register_class<Surface>();
    ClassDB::register_abstract_class<Action>();
    ClassDB::register_class<ORTBinding>();
    ClassDB::register_class<DestructibleElement>();
    ClassDB::register_class<ActionBundle>();
    ClassDB::register_abstract_class<Actor>();
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
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_CORE);

        return init_obj.init();
    }
}