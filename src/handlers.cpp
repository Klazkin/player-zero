#include "handlers.h"

void register_handlers()
{
    godot::ActionRegistry::register_action("wrathspark", Wrathspark::check, Wrathspark::cast);
}

bool Wrathspark::check(CastInfo cast_info)
{
    return cast_info.surface.get_occupation(cast_info.position) != nullptr;
}

void Wrathspark::cast(CastInfo cast_info)
{
    godot::SurfaceElement resident = *cast_info.surface.get_occupation(cast_info.position); // null unsafe
    resident.hit(4);
}
