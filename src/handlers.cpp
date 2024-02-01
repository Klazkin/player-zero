#include "handlers.h"

using namespace godot;

void register_handlers()
{
    Action::register_action(WRATHSPARK, check_cell_taken, cast_wrathspark);
    Action::register_action(GROUNDRAISE, check_cell_free, cast_groundraise);
    Action::register_action(TREAD, check_cell_free, cast_tread);
    Action::register_action(NETHERSWAP, check_cell_taken, cast_swap);
}

void register_combinations()
{
    Action::register_combination(BLOODDRAWING, GROUNDRAISE, ALTAR);
    Action::register_combination(TREAD, BLOODDRAWING, NETHERSWAP);
}

bool cast_always(const CastInfo &cast)
{
    return true;
}

bool check_cell_free(const CastInfo &cast)
{
    return cast.surface->is_position_available(cast.target);
}

bool check_cell_taken(const CastInfo &cast)
{
    return !check_cell_free(cast);
}

bool check_unit_only(const CastInfo &cast)
{
    return check_cell_taken(cast) && cast.surface->get_element(cast.target)->_is_unit();
}

bool check_self_cast(const CastInfo &cast)
{
    return cast.caster == cast.surface->get_element(cast.target);
}

// bool check_cast_distance()
// {
//     return cast.position
// }

void cast_wrathspark(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);
    target_element->hit(4);
}

void cast_groundraise(const CastInfo &cast)
{
    Ref<SurfaceElement> ground_element = memnew(SurfaceElement);
    cast.surface->place_element(cast.target, ground_element);
}

void cast_tread(const CastInfo &cast)
{
    cast.surface->lift_element(cast.caster->get_position());
    cast.surface->place_element(cast.target, cast.caster);
}

void cast_swap(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);

    cast.surface->lift_element(cast.caster->get_position());
    cast.surface->lift_element(cast.target);
    cast.surface->place_element(cast.caster->get_position(), target_element);
    cast.surface->place_element(cast.target, cast.caster);
}

void cast_coil(const CastInfo &cast)
{
}
