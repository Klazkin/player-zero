#include "actor.h"
#include "action.h"
#include "monte_carlo.h"
#include "bundles/tree_action_bundle.h"
#include "bundles/vector_action_bundle.h"

#include <algorithm>
#include <chrono>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>

void Actor::_bind_methods()
{
    // ClassDB::bind_static_method("Actor", D_METHOD("init_model"), &Actor::init_model);
    ClassDB::bind_static_method("Actor", D_METHOD("append_winner_to_file", "data_path", "winner"), &Actor::append_winner_to_file);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_decision_tree", "caster", "surface"), &Actor::get_actions_from_decision_tree);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_mcts", "caster", "surface", "iterations", "max_rollout_turns", "data_path"), &Actor::get_actions_from_mcts);
    ClassDB::bind_static_method("Actor", D_METHOD("perfrom_random_actions_for_turn", "caster", "surface"), &Actor::perfrom_random_actions_for_turn);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_model", "caster", "surface"), &Actor::get_actions_from_model);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_random", "caster", "surface"), &Actor::get_actions_from_random);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_wpmcts", "caster", "surface", "iterations", "max_rollout_turns"), &Actor::get_actions_from_wpmcts);
}

Ref<ActionBundle> Actor::get_actions_from_decision_tree(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<VectorActionBundle> ab = memnew(VectorActionBundle);

    auto units = surface->get_only_units_vec();
    Ref<Unit> target = nullptr;

    for (auto u : units)
    {
        if (u->get_faction() == caster->get_faction())
        {
            continue;
        }

        if ((u->get_position() - caster->get_position()).length_squared() >= 8 * 8)
        {
            continue;
        }

        if (target != nullptr && u->get_health() > target->get_health())
        {
            continue;
        }

        target = u;
    }

    if (target == nullptr)
    {
        UtilityFunctions::print("DTA: Returning ab, nothing found near.");
        ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
        return ab;
    }

    if ((target->get_position() - caster->get_position()).length_squared() >= 4 * 4)
    {
        UtilityFunctions::print("DTA: Adding tread cast");
        PackedVector2Array path = surface->get_shortest_path(caster->get_position(), target->get_position(), true);
        UtilityFunctions::print(path);
        Vector2i tread_target = path[path.size() - 4]; // TODO may cause issues
        ab->push_back_cast({TREAD, surface, caster, tread_target});
    }

    if (target->has_subscriber(STATUS_DUSTED) && caster->is_in_hand(WISPSPARKS) && caster->is_in_hand(BONEDUST))
    {
        UtilityFunctions::print("DTA: Adding bonesparks cast");
        ab->push_back_cast(Action::get_combination_cast(surface, caster, WISPSPARKS, BONEDUST));
        ab->push_back_cast({BONESPARKS, surface, caster, target->get_position()});
        ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
        return ab;
    }

    CastInfo ci = {WISPSPARKS, surface, caster, target->get_position()};

    if (caster->is_in_hand(WISPSPARKS) && check_line_of_sight(ci))
    {
        UtilityFunctions::print("DTA: Adding wispsparks cast");
        ab->push_back_cast(ci);
    }

    if (caster->is_in_hand(BONEDUST) && !target->has_subscriber(STATUS_DUSTED))
    {
        UtilityFunctions::print("DTA: Adding bonedust cast");
        ab->push_back_cast({BONEDUST, surface, caster, target->get_position()});
    }

    ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
    return ab;
}

void Actor::append_winner_to_file(const String &data_path, const Faction winner)
{
    std::fstream file(data_path.ascii(), std::ios::in | std::ios::out);
    std::vector<std::string> lines;
    std::string line;

    if (!file)
    {
        std::cerr << "Could not open file " << data_path.ascii() << std::endl;
        return;
    }

    while (std::getline(file, line))
    {
        lines.push_back(line);
    }

    file.clear();
    file.seekp(0, std::ios::beg);

    for (const auto &modified_line : lines)
    {
        file << modified_line << (winner == PLAYER) << ',' << (winner == MONSTER) << std::endl;
    }

    file.close();
}

void Actor::perfrom_random_actions_for_turn(Ref<Unit> caster, Ref<Surface> surface)
{
    if (surface->turn_get_current_unit() != caster)
    {
        UtilityFunctions::printerr("Not current units turn.");
        return;
    }

    if (caster->is_dead())
    {
        UtilityFunctions::printerr("Caster dead at the start of random cast.");
        return;
    }

    while (!caster->is_dead()) // if caster kills themselves after cast, cant continue to cast
    {
        std::vector<CastInfo> candidate_casts;
        for (auto action : caster->get_hand_set())
        {
            std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface, caster, Vector2i()});
            candidate_casts.insert(candidate_casts.end(), action_casts.begin(), action_casts.end());
        }

        if (candidate_casts.size() == 0)
        {
            Action::_cast_action({END_TURN, surface, caster, Vector2i()});
            break;
        }

        int cast_pt = UtilityFunctions::randi_range(0, candidate_casts.size() - 1);

        if (cast_pt >= candidate_casts.size())
        {
            UtilityFunctions::printerr("Critical: Generated random int bigger than candidates list.");
        }

        CastInfo random_cast = candidate_casts[cast_pt];
        if (!Action::_is_castable(random_cast))
        {
            UtilityFunctions::printerr("Uncastable action generated in random actor");
            UtilityFunctions::printerr(random_cast.action);
            UtilityFunctions::printerr(random_cast.target);

            std::cout << caster.is_null() << caster->is_dead() << (caster != surface->turn_get_current_unit()) << "\n";
            continue;
        }
        Action::_cast_action(random_cast);

        if (random_cast.action == END_TURN)
        {
            break;
        }
    }
}

void write_vec_to_file(ofstream &to_file, const std::vector<float> &vec)
{

    if (vec.size() != 96)
    {
        std::cout << "Vec to file size incorrect.\n";
        return;
    }

    for (auto x : vec)
    {
        to_file << x << ',';
    }

    to_file << '\n';
}

Ref<ActionBundle> Actor::get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface, int interations, int max_rollout_turns, const String &data_path)
{
    Ref<TreeActionBundle> ab = memnew(TreeActionBundle(surface, caster));
    MonteCarloTreeSearch mcts(ab->get_root(), max_rollout_turns, ucb, perfrom_random_actions_for_turn);
    mcts.run(interations);
    draw_tree(ab->get_root(), 8, ab->get_root()->caster->get_faction());
    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_model(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);
    // Removed function ...
    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<VectorActionBundle> ab = memnew(VectorActionBundle);
    Ref<Unit> opponent = nullptr;

    for (auto u : surface->get_only_units_vec())
    {
        if (u == caster)
        {
            continue;
        }

        if (opponent == nullptr)
        {
            opponent = u;
            continue;
        }

        std::cout << "AFM: More than one oppponennt during opponent search.\n";
    }

    if (surface->turn_get_current_unit() != caster)
    {
        UtilityFunctions::printerr("AFM: Not current units turn.");
        return ab;
    }

    if (caster->is_dead())
    {
        UtilityFunctions::printerr("AFM: Caster dead at the start of random cast.");
        return ab;
    }

    Ref<Surface> surface_clone = surface->clone();
    Ref<Unit> caster_clone = surface_clone->get_element(caster->get_position());

    while (!caster_clone->is_dead()) // if caster kills themselves after cast, cant continue to cast
    {
        CastInfo random_cast = {END_TURN, surface_clone, caster_clone, Vector2()};
        std::vector<CastInfo> candidate_casts;
        for (auto action : caster_clone->get_hand_set())
        {
            std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface_clone, caster_clone, Vector2i()});
            candidate_casts.insert(candidate_casts.end(), action_casts.begin(), action_casts.end());
        }

        if (candidate_casts.size() != 0)
        {
            int cast_pt = UtilityFunctions::randi_range(0, candidate_casts.size() - 1);
            if (cast_pt >= candidate_casts.size())
            {
                UtilityFunctions::printerr("Critical: Generated random int bigger than candidates list.");
            }

            random_cast = candidate_casts[cast_pt];
        }

        if (!Action::_is_castable(random_cast))
        {
            std::cout << "Uncastable action in AFM clone (1) candidates.";
            std::cout << random_cast.action;
            std::cout << random_cast.target.x << " " << random_cast.target.y;
            std::cout << caster_clone.is_null() << caster_clone->is_dead() << (caster_clone != surface_clone->turn_get_current_unit()) << "\n";
            continue;
        }

        Action::_cast_action(random_cast);
        ab->push_back_cast({random_cast.action, surface, caster, random_cast.target}); // push cast to original

        if (random_cast.action == END_TURN)
        {
            break;
        }
    }

    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_wpmcts(Ref<Unit> caster, Ref<Surface> surface, int interations, int max_rollout_turns)
{
    Ref<TreeActionBundle> ab = memnew(TreeActionBundle(caster, surface));
    WinPredictorTreeSearch wpts(ab->get_root(), max_rollout_turns, ucb, perfrom_random_actions_for_turn);
    wpts.run(interations);
    // draw_tree(ab->get_root(), 8, ab->get_root()->caster->get_faction());
    return ab;
}