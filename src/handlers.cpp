#include "handlers.h"
#include <set>
#include <godot_cpp/variant/utility_functions.hpp>
#include <algorithm>

using namespace godot;

void register_handlers()
{
    Action::register_action(
        END_TURN,
        check_always_allow,
        cast_end_trun,
        gen_self_cast);

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
        [](const CastInfo &c)
        { return check_cell_free(c) && check_cast_distance(c, 1); },
        cast_groundraise,
        gen_closest_free_cast);

    Action::register_action(
        TREAD,
        check_cell_free,
        cast_tread,
        gen_tread_cast);

    Action::register_action(
        NETHERSWAP,
        [](const CastInfo &c)
        { return check_cell_taken(c) && check_not_self_cast(c); },
        cast_swap,
        gen_all_other_units_cast);

    Action::register_action(
        COILBLADE,
        check_is_adjacent,
        [](const CastInfo &c)
        {
            multicaster(
                c,
                check_cell_taken,
                cast_coilblade_singular,
                {Vector2i(1, 0), Vector2i(2, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)});
        },
        gen_adjacent);

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
        gen_self_cast);

    Action::register_action(
        BONESPARKS,
        check_always_allow,
        cast_bonesparks,
        gen_self_cast);

    Action::register_action(
        ALTAR,
        check_cell_free,
        cast_altar,
        gen_closest_free_cast);

    Action::register_action(
        RESPIRIT,
        check_ally_unit_only,
        cast_respirit,
        gen_all_units_with_checker);

    Action::register_action(
        RAPID_GROWTH,
        check_self_cast,
        cast_rapid_growth,
        gen_self_cast);

    Action::register_action(
        IMMOLATION,
        check_ally_unit_only,
        cast_immolation,
        gen_all_units_with_checker);

    Action::register_action(
        ARMORCORE,
        check_ally_unit_only,
        cast_armorcore,
        gen_all_units_with_checker);

    Action::register_action(
        SUNDIVE,
        check_free_near_unit,
        cast_sundive,
        gen_free_near_every_unit);

    Action::register_action(
        BLOODDRAWING,
        check_self_cast,
        cast_blooddrawing,
        gen_self_cast);

    Action::register_action(
        METEORSHATTER,
        check_cell_taken,
        cast_meteorshatter,
        gen_all_elements_cast);

    Action::register_action(
        HOARFROST,
        check_ally_unit_only,
        cast_hoarfrost,
        gen_all_units_with_checker);

    Action::register_action(
        SWIFTARROW,
        [](const CastInfo &c)
        { return check_line_of_sight(c) && check_cell_taken(c) && check_cast_distance(c, 15); },
        cast_swiftarrow,
        gen_all_elements_with_checker);

    Action::register_action(
        ALIGNMENT_LANCE,
        check_is_adjacent,
        [](const CastInfo &c)
        {
            multicaster(
                c,
                check_cell_taken,
                cast_coilblade_singular,
                {Vector2i(1, 0), Vector2i(2, 0), Vector2i(3, 0)});
        },
        gen_adjacent);

    Action::register_action( // TODO implement along with altar
        BLESSING,
        check_always_allow,
        cast_blessing,
        gen_self_cast);
}

void register_combinations()
{
    Action::register_combination(BONEDUST, WISPSPARKS, BONESPARKS);

    Action::register_combination(BLOODDRAWING, GROUNDRAISE, ALTAR);
    Action::register_combination(BLOODDRAWING, RESPIRIT, ETERNALSHACLES);
    Action::register_combination(BLOODDRAWING, TREAD, NETHERSWAP);
    Action::register_combination(RESPIRIT, TREAD, RAPID_GROWTH);
    Action::register_combination(WRATHSPARK, BLOODDRAWING, IMMOLATION);
    Action::register_combination(RESPIRIT, GROUNDRAISE, ARMORCORE);
    Action::register_combination(WRATHSPARK, TREAD, SUNDIVE);
    Action::register_combination(WRATHSPARK, GROUNDRAISE, METEORSHATTER);
    Action::register_combination(TREAD, GROUNDRAISE, HOARFROST);
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
    return !check_cell_free(cast) && cast.surface->is_within(cast.target);
}

bool check_unit_only(const CastInfo &cast)
{
    return check_cell_taken(cast) && cast.surface->get_element(cast.target)->is_unit();
}

bool check_ally_unit_only(const CastInfo &cast)
{
    return check_unit_only(cast) &&
           as_unit_ptr(cast.surface->get_element(cast.target))->get_faction() == cast.caster->get_faction();
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
    ActionIdentifier result = Action::get_combination(
        (ActionIdentifier)cast.target.x,
        (ActionIdentifier)cast.target.y);

    if (result == INVALID_ACTION)
        return false;

    if (cast.caster->is_in_hand(result))
        return false;

    return true;
}

bool check_free_near_unit(const CastInfo &cast)
{
    if (!check_cell_free(cast))
    {
        return false;
    }

    for (auto v : {Vector2i(0, 1), Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, -1)})
    {
        if (check_unit_only({cast.action, cast.surface, cast.caster, cast.target + v}))
        {
            return true;
        }
    }

    return false;
}

bool check_is_adjacent(const CastInfo &cast)
{
    return (cast.target - cast.caster->get_position()).length_squared() == 1 && cast.surface->is_within(cast.target);
}

void cast_nothing(const CastInfo &cast)
{
    UtilityFunctions::print("DEBUG CAST: " + String(cast.target));
}

void cast_wrathspark(const CastInfo &cast)
{
    Ref<SurfaceElement> target_element = cast.surface->get_element(cast.target);
    target_element->hit(3 + cast.caster->get_attack());
    if (target_element->is_unit())
    {
        Unit *target_unit = as_unit_ptr(target_element);
        target_unit->add_subscriber(new BurnStatus(target_unit, 3));
        target_unit->get_stat_modifiers().defence -= 1;
    }
}

void cast_groundraise(const CastInfo &cast)
{
    Ref<SurfaceElement> ground_element = memnew(DestructibleElement);
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
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    int *link_counter = new int(2); // keeps track of number of connections between the links

    ShaclesParent *parent = new ShaclesParent(link_counter, *cast.caster, target_unit, 3);
    ShaclesChild *child = new ShaclesChild(link_counter, target_unit, 3);

    cast.caster->add_subscriber(parent);
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
    target_element->hit(3 + cast.caster->get_attack());
}

void cast_bonedust(const CastInfo &cast)
{
    for (auto potential_target : cast.surface->get_only_units_vec())
    {
        // ignore alies
        if (potential_target->get_faction() == cast.caster->get_faction())
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
        if (potential_target->get_faction() == cast.caster->get_faction())
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
    Ref<Unit> altar = memnew(Unit);
    altar->set_base_speed(-10);
    altar->set_health(5);
    altar->set_faction(cast.caster->get_faction());
    // altar->set_deck([ END_TURN, BLESSING ]);
    cast.surface->place_element(cast.target, altar);
}

void cast_combine_actions(const CastInfo &cast)
{
    ActionIdentifier action1 = (ActionIdentifier)cast.target.x;
    ActionIdentifier action2 = (ActionIdentifier)cast.target.y;
    ActionIdentifier result = Action::get_combination(action1, action2);

    if (result == INVALID_ACTION) // redundant check done already by check function
    {
        return;
    }

    cast.caster->remove_from_hand(action1);
    cast.caster->remove_from_hand(action2);
    cast.caster->add_to_hand(result);
}

void cast_end_trun(const CastInfo &cast)
{
    cast.surface->end_current_units_turn();
}

void cast_respirit(const CastInfo &cast)
{
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    target_unit->add_subscriber(new Spiriting(target_unit, 4));
}

void cast_rapid_growth(const CastInfo &cast)
{
    cast.caster->heal(5);
}

void cast_immolation(const CastInfo &cast)
{
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    int borrowed_hp = std::max(1, target_unit->get_health() / 2);

    target_unit->hit(borrowed_hp);
    target_unit->add_subscriber(new Immolation(borrowed_hp, target_unit, 4));
    StatModifiers &sm = target_unit->get_stat_modifiers();

    sm.speed += 1;
    sm.attack += 4;
    sm.defence -= 1;
}

void cast_armorcore(const CastInfo &cast)
{
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    target_unit->add_subscriber(new CoreArmor(target_unit, 3));

    StatModifiers &sm = target_unit->get_stat_modifiers();

    sm.speed -= 1;
    sm.armored = true;
}

void cast_sundive(const CastInfo &cast)
{
    cast.surface->lift_element(cast.caster->get_position());
    cast.surface->place_element(cast.target, cast.caster);

    for (auto v : {Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)})
    {
        CastInfo ci = {cast.action,
                       cast.surface,
                       cast.caster,
                       cast.target + v};

        if (check_cell_taken(ci))
        {
            cast_wrathspark(ci);
        }
    }
}

void cast_blooddrawing(const CastInfo &cast)
{
    cast.caster->hit(3);
    if (cast.surface->get_random_events_enabled())
    {
        cast.caster->add_to_hand(BLOODDRAWING);
        cast.caster->refill_hand();
        cast.caster->remove_from_hand(BLOODDRAWING);
    }
}

void cast_meteorshatter(const CastInfo &cast)
{
    Ref<SurfaceElement> target = cast.surface->get_element(cast.target);
    const int RADIUS = 3;
    target->hit(5 + cast.caster->get_attack());

    for (auto key_val_pair : cast.surface->get_element_positions())
    {
        if ((key_val_pair.first - cast.target).length_squared() <= RADIUS * RADIUS)
        {
            key_val_pair.second->hit(10);
        }
    }
}

void cast_hoarfrost(const CastInfo &cast)
{
    Unit *target_unit = as_unit_ptr(cast.surface->get_element(cast.target));
    target_unit->add_subscriber(new HoarfrostArmor(target_unit, 3));
    target_unit->get_stat_modifiers().defence += 2;
}

void cast_coilblade_singular(const CastInfo &cast)
{
    cast.surface->get_element(cast.target)->hit(4 + cast.caster->get_attack());
}

void cast_swiftarrow(const CastInfo &cast)
{
    Ref<SurfaceElement> target = cast.surface->get_element(cast.target);
    int damage_bonus = (cast.caster->get_position() - cast.target).length() / 2;
    target->hit(1 + damage_bonus + cast.caster->get_attack());
}

void cast_blessing(const CastInfo &cast)
{
    const int RADIUS = 3;

    for (auto u : cast.surface->get_only_units_vec())
    {
        if (u != cast.caster &&
            u->get_faction() == cast.caster->get_faction() &&
            (u->get_position() - cast.caster->get_position()).length_squared() <= RADIUS * RADIUS)
        {
            u->heal(2);
        }
    }
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
        auto offset = (cast.target - cast.caster->get_position());
        if (offset == Vector2i(0, 1))
            lambda_rotate = [](Vector2i p) -> Vector2i
            { return Vector2i(p.y, p.x); };

        else if (offset == Vector2i(-1, 0))
            lambda_rotate = [](Vector2i p) -> Vector2i
            { return Vector2i(-p.x, -p.y); };

        else if (offset == Vector2i(0, -1))
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

std::vector<CastInfo> gen_self_cast(const CastInfo &initial_info)
{
    return std::vector<CastInfo>({
        {initial_info.action, initial_info.surface, initial_info.caster, initial_info.caster->get_position()},
    });
}

std::vector<CastInfo> gen_closest_free_cast(const CastInfo &initial_info)
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

std::vector<CastInfo> gen_all_elements_with_checker(const CastInfo &initial_info)
{
    ActionCheckType checker = Action::get_action_checker(initial_info.action);
    std::vector<CastInfo> ret;
    for (auto key_val_pair : initial_info.surface->get_element_positions())
    {
        CastInfo cast = {initial_info.action,
                         initial_info.surface,
                         initial_info.caster,
                         key_val_pair.first};

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

        if (!initial_info.surface->is_position_available(tread_target))
        {
            std::cout << "Generated invalid tread position \n"
                      << "\ttread_target " << tread_target.x << " " << tread_target.y << "\n"
                      << "\tunit  " << u->get_position().x << " " << u->get_position().y << "\n"
                      << "\tcaster  " << initial_info.caster->get_position().x << " " << initial_info.caster->get_position().y << "\n";

            for (auto p : path)
            {
                std::cout << p.x << ":" << p.y << "\n";
            }
        }

        // for (auto n : initial_info.surface->get_free_neighbors(initial_info.caster->get_position()))
        // {
        //     if (n == tread_target)
        //         continue;
        //     ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, n});
        // }

        // // walk towards unit for distance between 1 and MAX_WALK distance
        // for (int i = 1; i <= std::min((long long)MAX_WALK, path.size() - 2); i++)
        // {
        //     Vector2i tread_target = path[std::max(1LL, path.size() - i)];
        //     ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, tread_target});
        // }
    }

    return ret;
}

std::vector<CastInfo> gen_action_combinations(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;
    std::unordered_set<ActionIdentifier> hand = initial_info.caster->get_hand_set();

    for (auto it1 = hand.begin(); it1 != hand.end(); ++it1)
    {
        for (auto it2 = std::next(it1); it2 != hand.end(); ++it2)
        {
            ActionIdentifier action1 = *it1;
            ActionIdentifier action2 = *it2;
            ActionIdentifier result = Action::get_combination(action1, action2);

            if (result == INVALID_ACTION || initial_info.caster->is_in_hand(result))
            {
                continue;
            }

            ret.push_back({initial_info.action,
                           initial_info.surface,
                           initial_info.caster,
                           Vector2i((int)action1, (int)action2)});
        }
    }

    return ret;
}

std::vector<CastInfo> gen_free_near_every_unit(const CastInfo &initial_info)
{
    std::unordered_set<Vector2i, VectorHasher> candidates_set;

    for (auto u : initial_info.surface->get_only_units_vec())
    {
        for (auto n : {Vector2i(0, 1), Vector2i(1, 0), Vector2i(0, -1), Vector2i(-1, 0)})
        {
            Vector2i target = u->get_position() + n;
            if (initial_info.surface->is_position_available(target))
            {
                candidates_set.insert(target);
            }
        }
    }

    std::vector<CastInfo> candidates_vec;
    candidates_vec.reserve(candidates_set.size());

    for (auto v : candidates_set)
    {
        candidates_vec.push_back({initial_info.action, initial_info.surface, initial_info.caster, v});
    }

    return candidates_vec;
}

std::vector<CastInfo> gen_adjacent(const CastInfo &initial_info)
{
    std::vector<CastInfo> ret;

    for (auto n : {Vector2i(0, 1), Vector2i(1, 0), Vector2i(0, -1), Vector2i(-1, 0)})
    {
        Vector2i pos = n + initial_info.caster->get_position();
        if (initial_info.surface->is_within(pos))
        {
            ret.push_back({initial_info.action, initial_info.surface, initial_info.caster, pos});
        }
    }

    return ret;
}
