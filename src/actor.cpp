#include "actor.h"
#include "action.h"
#include "monte_carlo.h"

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
    Ref<ActionBundle> ab = memnew(ActionBundle);

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
    Ref<ActionBundle> ab = memnew(ActionBundle);
    Ref<Surface> surface_clone = surface->clone();
    Ref<Unit> caster_clone = surface_clone->get_element(caster->get_position());
    Node *root = new Node(END_TURN, surface_clone, caster_clone, Vector2i(0, 0));
    MonteCarloTreeSearch mcts(root, max_rollout_turns, ucb, perfrom_random_actions_for_turn);
    mcts.run(interations);

    ofstream data_file_stream;
    if (!data_path.is_empty())
    {
        data_file_stream.open(std::string(data_path.ascii()), std::ios::app);
    }
    Node *node = root;
    // mcts.draw_tree(node, 10);
    while (!node->is_leaf())
    {
        float best_score = -std::numeric_limits<float>::infinity();
        Node *best_child = nullptr;
        for (auto child : node->children)
        {
            float child_score = child->visits; // TODO try different scorings
            if (child_score > best_score)
            {
                best_score = child_score;
                best_child = child;
            }
        }

        CastInfo ci = {best_child->action, surface, caster, best_child->target};
        ab->push_back_cast(ci);
        node = best_child;

        if (!data_path.is_empty())
        {
            write_vec_to_file(data_file_stream, serialize_state(node->parent->surface));
        }

        if (node->action == END_TURN)
        {
            break;
        }
    }

    if (node->is_leaf())
    { // Due to a bug where MCTS tree stops generating after a victory, it does not add END_TURN action cast after winning.
        ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
    }

    // mcts.serialize_tree_to_stream(mcts.root, data_file_stream, 2500);
    data_file_stream.close();

    delete root;
    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_model(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);
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

    // generate clones...
    //
    // generate cast_candidates
    // turn surface into vector...
    // apply normalization...
    // fetch from model...

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
        std::vector<CastInfo> candidate_casts;
        for (auto action : caster_clone->get_hand_set())
        {
            std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface_clone, caster_clone, Vector2i()});
            candidate_casts.insert(candidate_casts.end(), action_casts.begin(), action_casts.end());
        }

        float best_exploit = -1;
        CastInfo best_cast = {END_TURN, surface_clone, caster_clone, Vector2i()};
        // cast evaluation, get explot
        for (auto candidate_cast : candidate_casts)
        {
            Ref<Surface> surface_clone_clone = surface_clone->clone();
            Ref<Unit> caster_clone_clone = surface_clone_clone->get_element(caster_clone->get_position());

            CastInfo candidate_cast_clone = {candidate_cast.action, surface_clone_clone, caster_clone_clone, candidate_cast.target};

            // if (candidate_cast.action == END_TURN)
            // {
            //     continue;
            // }

            if (Action::_is_castable(candidate_cast_clone))
            {
                Action::_cast_action(candidate_cast_clone);
            }
            else
            {
                std::cout << "Uncastable action in AFM clone clone (2) candidates.\n";
                continue;
            }

            // candidate cast should be cast on a clone surface, then the clone surfacae must be turned into vector.
            std::vector<float> vec_cast = serialize_cast(candidate_cast_clone); // must be called after cast!

            if (vec_cast.size() != 114)
            {
                std::cout << "Serialized candidate cast clone wrong size (" << vec_cast.size() << ").\n";
                continue;
            }

            std::array<float, 114> arr;
            std::copy_n(vec_cast.begin(), 114, arr.begin());

            float exploit = ORTBinding::dueler_predict(arr); // use candidate_cast_clone

            std::cout
                << "Exploit for action " << candidate_cast_clone.action
                << " at " << candidate_cast_clone.target.x << ";" << candidate_cast_clone.target.y
                << " is " << exploit << "\n";

            if (exploit > best_exploit)
            {
                best_exploit = exploit;
                best_cast = candidate_cast;
            }
        }

        if (!Action::_is_castable(best_cast))
        {
            std::cout << "Uncastable action in AFM clone (1) candidates.";
            std::cout << best_cast.action;
            std::cout << best_cast.target.x << " " << best_cast.target.y;
            std::cout << caster_clone.is_null() << caster_clone->is_dead() << (caster_clone != surface_clone->turn_get_current_unit()) << "\n";
            continue;
        }

        Action::_cast_action(best_cast);
        ab->push_back_cast({best_cast.action, surface, caster, best_cast.target}); // push cast to original

        if (best_cast.action == END_TURN)
        {
            break;
        }
    }

    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);
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
    Ref<ActionBundle> ab = memnew(ActionBundle);
    Ref<Surface> surface_clone = surface->clone();
    Ref<Unit> caster_clone = surface_clone->get_element(caster->get_position());
    Node *root = new Node(END_TURN, surface_clone, caster_clone, Vector2i(0, 0));
    WinPredictorTreeSearch wpts(root, max_rollout_turns, ucb, perfrom_random_actions_for_turn);
    wpts.run(interations);

    Node *node = root;
    // mcts.draw_tree(node, 10);
    while (!node->is_leaf())
    {
        float best_score = -std::numeric_limits<float>::infinity();
        Node *best_child = nullptr;
        for (auto child : node->children)
        {
            float child_score = child->visits; // TODO try different scorings
            if (child_score > best_score)
            {
                best_score = child_score;
                best_child = child;
            }
        }

        CastInfo ci = {best_child->action, surface, caster, best_child->target};
        ab->push_back_cast(ci);
        node = best_child;

        if (node->action == END_TURN)
        {
            break;
        }
    }

    if (node->is_leaf())
    { // Due to a bug where MCTS tree stops generating after a victory, it does not add END_TURN action cast after winning.
        ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
    }

    delete root;
    return ab;
}