#include "handlers.h"

void register_handlers()
{
    ActionRegistry::register_action("wrathspark", Wrathspark::check, Wrathspark::cast);
}

bool Wrathspark::check(CastInfo cast_info)
{
    godot::SurfaceElement resident = cast_info.surface.get_occupation(cast_info.position);

    return false;
}

void Wrathspark::cast(CastInfo cast_info)
{
    // godot::Unit resident = cast_info.surface.get_occupation(cast_info.position);
}
