#include "handlers.h"

using namespace godot;

void register_handlers()
{
    // TODO USE ENUMS FOR ACTIONS(
    Action::register_action("wrathspark", check_cell_taken, cast_wrathspark);
    Action::register_action("groundraise", check_cell_free, cast_groundraise);
}

void register_combinations()
{
    Action::register_combination("a", "b", "c");
}

bool check_cell_free(CastInfo &cast)
{
    return cast.surface->is_position_available(cast.position);
}

bool check_cell_taken(CastInfo &cast)
{
    return !check_cell_free(cast);
}

void cast_wrathspark(CastInfo &cast)
{
    Ref<SurfaceElement> target = cast.surface->get_element(cast.position);
    target->hit(4);
}

void cast_groundraise(CastInfo &cast)
{
    Ref<SurfaceElement> ground_element = memnew(SurfaceElement);
    cast.surface->place_element(cast.position, ground_element);
}
