#include "handlers.h"
#include <set>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void register_handlers()
{
    Action::register_action(WRATHSPARK, check_cell_taken, cast_wrathspark);
    Action::register_action(GROUNDRAISE, check_cell_free, cast_groundraise);
    Action::register_action(TREAD, check_cell_free, cast_tread);
    Action::register_action(NETHERSWAP, check_cell_taken, cast_swap);
    Action::register_action(COILBLADE, check_is_direction_valid, cast_coildblade);
}

void register_combinations()
{
    Action::register_combination(BLOODDRAWING, GROUNDRAISE, ALTAR);
    Action::register_combination(TREAD, BLOODDRAWING, NETHERSWAP);
}

bool check_always_allow(const CastInfo &cast)
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

bool check_is_direction_valid(const CastInfo &cast)
{
    const std::set<Vector2i> valids({Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)});
    return valids.count(cast.target) > 0;
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

void cast_coildblade(const CastInfo &cast)
// const PackedVector2Array points,
// const ActionCheckType check_func,
// const ActionCastType cast_func
// const bool rotate)
{
    PackedVector2Array points;
    points.append(Point2i(1, 0));
    points.append(Point2i(2, 0));
    points.append(Point2i(0, 1));
    points.append(Point2i(-1, 0));
    points.append(Point2i(0, -1));

    // TEMPORARY UGLY SOLUTION
    Direction dir = EAST;
    if (cast.target == Vector2i(0, 1))
        dir = NORTH;
    if (cast.target == Vector2i(-1, 0))
        dir = WEST;
    if (cast.target == Vector2i(0, -1))
        dir = SOUTH;

    UtilityFunctions::print("Direction is " + dir);

    for (Point2i point : points)
    {
        // Local cast
        Point2i loc_point = cast.caster->get_position() + point;

        switch (dir)
        {
        case (EAST):
            break;

        case (NORTH):
            loc_point = Vector2i(loc_point.y, loc_point.x);
            break;

        case (WEST):
            loc_point = Vector2i(-loc_point.x, -loc_point.y);
            break;

        case (SOUTH):
            loc_point = Vector2i(-loc_point.y, -loc_point.x);
            break;
        }

        CastInfo loc_cast = {COILBLADE, cast.surface, cast.caster, loc_point};
        if (check_cell_taken(loc_cast))
            cast_wrathspark(loc_cast);
    }
}