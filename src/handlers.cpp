#include "handlers.h"
#include <set>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

using namespace godot;

void register_handlers()
{
    Action::register_action(
        COMBINE_ACTIONS,
        check_action_combination,
        cast_combine_actions,
        gen_action_combinations);

    Action::register_action(
        WRATHSPARK,
        [](const CastInfo &c)
        { return check_cell_taken(c) && check_cast_distance(c, 5); },
        cast_wrathspark,
        gen_all_units_with_checker);

    Action::register_action(
        GROUNDRAISE,
        check_cell_free,
        cast_groundraise,
        gen_closest_free_cast);

    Action::register_action(
        TREAD,
        check_cell_free,
        cast_tread,
        gen_tread_cast);

    Action::register_action(
        NETHERSWAP,
        check_cell_taken,
        cast_swap,
        gen_all_other_units_cast);

    Action::register_action(
        COILBLADE,
        check_is_direction_valid,
        [](const CastInfo &c)
        {
            multicaster(
                c,
                check_cell_taken,
                cast_wrathspark,
                {Vector2i(1, 0), Vector2i(2, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)});
        },
        gen_4direction_cast);

    Action::register_action(
        LOS_ACTION,
        check_line_of_sight,
        cast_nothing);

    Action::register_action(
        DETONATION,
        check_unit_only,
        cast_detonate,
        gen_all_other_units_cast);

    Action::register_action(
        ETERNALSHACLES,
        [](const CastInfo &c)
        { return check_not_self_cast(c) && check_unit_only(c); },
        cast_shacle,
        gen_all_other_units_cast);

    Action::register_action(
        DEBUG_KILL,
        check_cell_taken,
        cast_debug_kill,
        gen_all_units_cast);

    Action::register_action(
        WISPSPARKS,
        [](const CastInfo &c)
        { return check_line_of_sight(c) && check_cell_taken(c) && check_cast_distance(c, 4); },
        cast_wispsparks,
        gen_all_units_with_checker);

    Action::register_action(
        BONEDUST,
        check_always_allow,
        cast_bonedust,
        gen_simple_cast);

    Action::register_action(
        BONESPARKS,
        check_always_allow,
        cast_bonesparks,
        gen_simple_cast);

    Action::register_action(
        ALTAR,
        check_cell_free,
        cast_altar,
        gen_closest_free_cast);
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

bool check_action_combination(const CastInfo &cast)
{
    ActionIdentifier action1 = (ActionIdentifier)cast.target.x;
    ActionIdentifier action2 = (ActionIdentifier)cast.target.y;
    return Action::has_combination(action1, action2) && cast.caster->is_unit();
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
    int *link_counter = new int(2); // keeps track of number of connections between the links

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

void cast_altar(const CastInfo &cast)
{
    Ref<SurfaceElement> altar = memnew(DestructibleElement); // TODO check safety
    cast.surface->place_element(cast.target, altar);
}

void cast_combine_actions(const CastInfo &cast)
{
    ActionIdentifier action1 = (ActionIdentifier)cast.target.x;
    ActionIdentifier action2 = (ActionIdentifier)cast.target.y;
    ActionIdentifier result = Action::get_combination(action1, action2);

    if (result == INVALID_ACTION) // redundant check
    {
        return;
    }

    Unit *ucaster = as_unit_ptr(cast.caster);

    ucaster->remove_from_hand(action1);
    ucaster->remove_from_hand(action2);
    ucaster->add_to_hand(result);
}

void multicaster(
    const CastInfo &cast,
    ActionCheckType local_checker,
    ActionCastType local_caster,
    const std::vector<Vector2i> &points,
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

    for (Vector2i point : points)
    {
        Vector2i loc_point = cast.caster->get_position() + lambda_rotate(point);
        const CastInfo loc_cast = {cast.action, cast.surface, cast.caster, loc_point};
        if (local_checker(loc_cast))
            local_caster(loc_cast);
    }
}

std::vector<CastInfo> gen_simple_cast(const CastInfo &initial_info)
{
    return std::vector<CastInfo>({initial_info});
}

std::vector<CastInfo> gen_self_cast(const CastInfo &initial_info)
{
    return std::vector<CastInfo>({
        {initial_info.action, initial_info.surface, initial_info.caster, initial_info.caster->get_position()},
    });
}

std::vector<CastInfo> gen_closest_free_cast(const CastInfo &initial_info) // todo expand bfs style
{
    std::vector<CastInfo> ret;
    for (auto neighbor : initial_info.surface->get_free_neighbors(initial_info.caster->get_position()))
    {
        ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, neighbor});
    }

    return ret;
}

std::vector<CastInfo> gen_all_elements_cast(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;

    for (auto key_val_pair : initial_info.surface->get_element_positions())
    {
        ret.push_back(
            {initial_info.action, initial_info.surface, initial_info.caster, key_val_pair.first});
    }

    return ret;
}

std::vector<CastInfo> gen_all_units_cast(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;

    for (auto unit : initial_info.surface->get_only_units_vec())
    {
        ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, unit->get_position()});
    }

    return ret;
}

std::vector<CastInfo> gen_all_other_units_cast(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;
    for (auto unit : initial_info.surface->get_only_units_vec())
    {
        if (unit == initial_info.caster)
        {
            continue;
        }

        ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, unit->get_position()});
    }
    return ret;
}

std::vector<CastInfo> gen_all_units_with_checker(const CastInfo &initial_info)
{
    ActionCheckType checker = Action::get_action_checker(initial_info.action);
    std::vector<CastInfo> ret;
    for (auto unit : initial_info.surface->get_only_units_vec())
    {
        CastInfo cast = {initial_info.action,
                         initial_info.surface,
                         initial_info.caster,
                         unit->get_position()};

        if (!checker(cast))
        {
            continue;
        }

        ret.push_back(cast);
    }
    return ret;
}

std::vector<CastInfo> gen_4direction_cast(const CastInfo &initial_info)
{
    return std::vector<CastInfo>({
        {initial_info.action, initial_info.surface, initial_info.caster, Vector2i(1, 0)},
        {initial_info.action, initial_info.surface, initial_info.caster, Vector2i(0, 1)},
        {initial_info.action, initial_info.surface, initial_info.caster, Vector2i(-1, 0)},
        {initial_info.action, initial_info.surface, initial_info.caster, Vector2i(0, -1)},
    });
}

std::vector<CastInfo> gen_tread_cast(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;
    const int MAX_WALK = 4;
    const int MAX_SEARCH_DIST = 16;

    auto units = initial_info.surface->get_only_units_vec();

    for (auto u : units)
    {
        if (u == initial_info.caster)
        {
            continue; // exclude self
        }

        if ((u->get_position() - initial_info.caster->get_position()).length_squared() >= MAX_SEARCH_DIST * MAX_SEARCH_DIST)
        {
            continue; // exclude untis too far away.
        }

        PackedVector2Array path = initial_info.surface->get_shortest_path(
            initial_info.caster->get_position(),
            u->get_position(),
            true);

        if (path.size() < 2)
        {
            continue;
        }

        // walk towards unit for MAX_WALK distance
        Vector2i tread_target = path[std::max(1LL, path.size() - MAX_WALK)];
        ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, tread_target});
    }

    return ret;
}

std::vector<CastInfo> gen_action_combinations(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;
    std::unordered_set<ActionIdentifier> hand = as_unit_ptr(initial_info.caster)->get_hand_set();

    for (auto it1 = hand.begin(); it1 != hand.end(); ++it1)
    {
        for (auto it2 = std::next(it1); it2 != hand.end(); ++it2)
        {
            ActionIdentifier action1 = *it1;
            ActionIdentifier action2 = *it2;

            if (Action::has_combination(action1, action2))
            {
                ret.push_back({initial_info.action,
                               initial_info.surface,
                               initial_info.caster,
                               Vector2i((int)action1, (int)action2)});
            }
        }
    }

    return ret;
}
