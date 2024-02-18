#include "handlers.h"
#include <set>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

using namespace godot;

void register_handlers()
{
    Action::register_action(
        WRATHSPARK,
        [](const CastInfo &c)
        { return check_cell_taken(c) && check_cast_distance(c, 5); },
        cast_wrathspark);

    Action::register_action(
        GROUNDRAISE,
        check_cell_free,
        cast_groundraise);

    Action::register_action(
        TREAD,
        check_cell_free,
        cast_tread);

    Action::register_action(
        NETHERSWAP,
        check_cell_taken,
        cast_swap);

    Action::register_action(
        COILBLADE,
        check_is_direction_valid,
        [](const CastInfo &c)
        { multicaster(c, check_cell_taken, cast_wrathspark, generate_coilblade_points()); });

    Action::register_action(
        LOS_ACTION,
        check_line_of_sight,
        cast_nothing);

    Action::register_action(
        DETONATION,
        check_unit_only,
        cast_detonate);

    Action::register_action(
        ETERNALSHACLES,
        [](const CastInfo &c)
        { return check_not_self_cast(c) && check_cell_taken(c); },
        cast_shacle);

    Action::register_action(
        DEBUG_KILL,
        check_cell_taken,
        cast_debug_kill);

    Action::register_action(
        WISPSPARKS,
        [](const CastInfo &c)
        { return check_line_of_sight(c) && check_cell_taken(c) && check_cast_distance(c, 4); },
        cast_wispsparks);

    Action::register_action(
        BONEDUST,
        check_always_allow,
        cast_bonedust);

    Action::register_action(
        BONESPARKS,
        check_always_allow,
        cast_bonesparks);
}

void register_combinations()
{
    Action::register_combination(BLOODDRAWING, GROUNDRAISE, ALTAR);
    Action::register_combination(TREAD, BLOODDRAWING, NETHERSWAP);
    Action::register_combination(BONEDUST, WISPSPARKS, BONESPARKS);
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
    return check_cell_taken(cast) && cast.surface->get_element(cast.target)->is_unit();
}

bool check_self_cast(const CastInfo &cast)
{
    return cast.caster->get_position() == cast.target;
}

bool check_not_self_cast(const CastInfo &cast)
{
    return !check_self_cast(cast);
}

bool check_is_direction_valid(const CastInfo &cast)
{
    const std::set<Vector2i> valids({Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)});
    return valids.count(cast.target) > 0;
}

bool check_cast_distance(const CastInfo &cast, int max_distance)
{
    return (cast.target - cast.caster->get_position()).length_squared() <= max_distance * max_distance;
}

bool check_line_of_sight(const CastInfo &cast)
{
    return cast.target == cast.surface->get_ray_collision( // returns first collider
                              cast.caster->get_position(), // cast from
                              cast.target);                // cast to
}

void cast_nothing(const CastInfo &cast)
{
    UtilityFunctions::print("DEBUG CAST: " + String(cast.target));
}

void cast_wrathspark(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);
    target_element->hit(4);

    if (target_element->is_unit())
    {
        Unit *target_unit = as_unit_ptr(target_element);
        target_unit->add_subscriber(new BurnStatus(target_unit, 5));
    }
}

void cast_groundraise(const CastInfo &cast)
{
    Ref<SurfaceElement> ground_element = memnew(DestructibleElement); // TODO check safety
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

void cast_detonate(const CastInfo &cast)
{
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    target_unit->add_subscriber(new Countdown(target_unit, 2));
}

void cast_shacle(const CastInfo &cast)
{
    Unit *caster_unit = as_unit_ptr(cast.caster);
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    int *link_counter = new int(2);

    ShaclesParent *parent = new ShaclesParent(link_counter, caster_unit, target_unit, 3);
    ShaclesChild *child = new ShaclesChild(link_counter, target_unit, 3);

    caster_unit->add_subscriber(parent);
    target_unit->add_subscriber(child);
}

void cast_debug_kill(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);
    target_element->hit(99999);
}

void cast_wispsparks(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);
    target_element->hit(2);
}

void cast_bonedust(const CastInfo &cast)
{
    for (auto potential_target : cast.surface->get_only_units_vec())
    {
        // ignore alies
        if (cast.caster->is_unit() && potential_target->get_faction() == as_unit_ptr(cast.caster)->get_faction())
        {
            continue;
        }

        if ((potential_target->get_position() - cast.caster->get_position()).length_squared() <= 4 * 4)
        {
            potential_target->add_subscriber(new Dusted(*potential_target, 2));
            StatModifiers &sm = potential_target->get_stat_modifiers();
            sm.speed -= 2;
        }
    }
}

void cast_bonesparks(const CastInfo &cast)
{
    for (auto potential_target : cast.surface->get_only_units_vec())
    {
        // ignore alies
        if (cast.caster->is_unit() && potential_target->get_faction() == as_unit_ptr(cast.caster)->get_faction())
        {
            continue;
        }

        if ((potential_target->get_position() - cast.caster->get_position()).length_squared() <= 4 * 4 &&
            potential_target->has_subscriber(STATUS_DUSTED))
        {
            potential_target->hit(10);
        }
    }
}

void multicaster(
    const CastInfo &cast,
    ActionCheckType local_checker,
    ActionCastType local_caster,
    const PackedVector2Array &points,
    bool is_rotatable)
{
    using RotatorFunc = Vector2i (*)(Vector2i);
    RotatorFunc lambda_rotate = [](Vector2i p)
    { return p; };

    if (is_rotatable)
    {
        if (cast.target == Vector2i(0, 1))
            lambda_rotate = [](Vector2i p) -> Vector2i
            { return Vector2i(p.y, p.x); };

        else if (cast.target == Vector2i(-1, 0))
            lambda_rotate = [](Vector2i p) -> Vector2i
            { return Vector2i(-p.x, -p.y); };

        else if (cast.target == Vector2i(0, -1))
            lambda_rotate = [](Vector2i p) -> Vector2i
            { return Vector2i(-p.y, -p.x); };
    }

    for (Point2i point : points)
    {
        Point2i loc_point = lambda_rotate(cast.caster->get_position() + point);
        CastInfo loc_cast = {COILBLADE, cast.surface, cast.caster, loc_point};
        if (local_checker(loc_cast))
            local_caster(loc_cast);
    }
}

Unit *as_unit_ptr(Ref<SurfaceElement> element)
{
    return Object::cast_to<Unit>(*element);
}

PackedVector2Array generate_coilblade_points()
{ // TODO make a registry for patterns
    PackedVector2Array points;
    points.append(Point2i(1, 0));
    points.append(Point2i(2, 0));
    points.append(Point2i(0, 1));
    points.append(Point2i(-1, 0));
    points.append(Point2i(0, -1));
    return points;
}
