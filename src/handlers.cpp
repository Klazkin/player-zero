#include "handlers.h"

using namespace godot;

void register_handlers()
{
    Action::register_action(WRATHSPARK, check_cell_taken, cast_wrathspark);
    Action::register_action(GROUNDRAISE, check_cell_free, cast_groundraise);
}

void register_combinations()
{
    Action::register_combination(BLOODDRAWING, GROUNDRAISE, ALTAR);
    Action::register_combination(TREAD, BLOODDRAWING, NETHERSWAP);
}

bool check_cell_free(const CastInfo &cast)
{
    return cast.surface->is_position_available(cast.position);
}

bool check_cell_taken(const CastInfo &cast)
{
    return !check_cell_free(cast);
}

bool check_self_cast(const CastInfo &cast)
{
    return cast.caster == cast.surface->get_element(cast.position);
}

void cast_wrathspark(const CastInfo &cast)
{
    Ref<SurfaceElement> target = cast.surface->get_element(cast.position);
    target->hit(4);
}

void cast_groundraise(const CastInfo &cast)
{
    Ref<SurfaceElement> ground_element = memnew(SurfaceElement);
    cast.surface->place_element(cast.position, ground_element);
}
