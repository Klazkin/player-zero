#include "handlers.h"

using namespace godot;

void register_handlers()
{
    ActionRegistry::register_action("wrathspark", Wrathspark::check, Wrathspark::cast);
}

void register_combinations()
{
    ActionRegistry::register_combination("act", "act", "act");
}

bool Wrathspark::check(CastInfo cast_info)
{
    return cast_info.surface.get_occupation(cast_info.position) != nullptr;
}

void Wrathspark::cast(CastInfo cast_info)
{
    SurfaceElement resident = *cast_info.surface.get_occupation(cast_info.position); // null unsafe
    resident.hit(4);
}
