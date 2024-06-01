#include "actor.h"
#include "action.h"
#include "monte_carlo.h"
#include "bundles/tree_action_bundle.h"
#include "bundles/vector_action_bundle.h"
#include "bundles/random_action_bundle.h"

#include <algorithm>
#include <chrono>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>

void Actor::_bind_methods()
{
    ClassDB::bind_static_method("Actor", D_METHOD("append_winner_to_file", "data_path", "winner"), &Actor::append_winner_to_file);
    ClassDB::bind_static_method("Actor", D_METHOD("write_winner_at_the_end_of_file", "data_path", "winner"), &Actor::write_winner_at_the_end_of_file);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_decision_tree", "caster", "surface"), &Actor::get_actions_from_decision_tree);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_mcts", "caster", "surface", "iterations", "max_rollout_turns"), &Actor::get_actions_from_mcts);
    ClassDB::bind_static_method("Actor", D_METHOD("perfrom_random_actions_for_turn", "caster", "surface"), &Actor::perfrom_random_actions_for_turn);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_random", "caster", "surface"), &Actor::get_actions_from_random);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_pzts", "caster", "surface", "iterations", "model_file"), &Actor::get_actions_from_pzts);
    ClassDB::bind_static_method("Actor", D_METHOD("reload_pzts_model"), &Actor::reload_pzts_model);
}

Ref<ActionBundle> Actor::get_actions_from_decision_tree(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<VectorActionBundle> ab = memnew(VectorActionBundle);

    if (caster->is_in_hand(BLESSING))
    {
        ab->push_back_cast({BLESSING, surface, caster, Vector2i()});
    }

    if (caster->is_in_hand(SENTRY_STRIKE))
    {
        std::vector<CastInfo> candidates = gen_all_units_with_checker({SENTRY_STRIKE, surface, caster, Vector2i()});
        Ref<SurfaceElement> best_target = nullptr;

        for (auto ci : candidates)
        {
            Ref<SurfaceElement> target = surface->get_element(ci.target);

            if (best_target == nullptr)
            {
                best_target = target;
                continue;
            }

            if (target->get_health() < best_target->get_health())
            {
                best_target = target;
                continue;
            }
        }

        if (best_target != nullptr)
        {
            ab->push_back_cast({SENTRY_STRIKE, surface, caster, best_target->get_position()});
        }
    }

    ab->push_back_cast({END_TURN, surface, caster, Vector2i()});
    return ab;
}

void Actor::append_winner_to_file(const String &path, const Faction winner)
{
    std::fstream file(path.ascii(), std::ios::in | std::ios::out);
    std::vector<std::string> lines;
    std::string line;

    if (!file)
    {
        std::cerr << "Could not open file " << path.ascii() << std::endl;
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

void Actor::write_winner_at_the_end_of_file(const String &path, const Faction winner)
{
    ofstream data_file_stream;
    data_file_stream.open(std::string(path.ascii()), std::ios::app);
    data_file_stream << "0\n0\n" // double zeroes mark the end of the file
                     << winner << "\n";
    data_file_stream.close();
}

void Actor::perfrom_random_actions_for_turn(Ref<Unit> caster, Ref<Surface> surface)
{
    if (caster.is_null())
    {
        UtilityFunctions::printerr("Caster is null.");
        return;
    }

    if (caster->is_dead())
    {
        UtilityFunctions::printerr("Caster dead at the start of random cast.");
        return;
    }

    if (surface.is_null() || !surface.is_valid())
    {
        UtilityFunctions::printerr("Invalid surface.");
        return;
    }

    if (surface->turn_get_current_unit() != caster)
    {
        UtilityFunctions::printerr("Not current units turn.");
        return;
    }

    // std::cout << "x";
    while (!caster->is_dead()) // if caster kills themselves after cast, cant continue to cast
    {
        std::vector<CastInfo> candidate_casts;
        for (auto action : caster->get_hand_set())
        {
            std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface, caster, caster->get_position()});
            candidate_casts.insert(candidate_casts.end(), action_casts.begin(), action_casts.end());
        }

        if (candidate_casts.size() == 0)
        {
            Action::_cast_action({END_TURN, surface, caster, Vector2i()});
            break;
        }

        CastInfo random_cast = candidate_casts[UtilityFunctions::randi_range(0, candidate_casts.size() - 1)];
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

void Actor::reload_pzts_model()
{
    PlayerZeroPredictor::unload_model("player_zero.onnx");
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

Ref<ActionBundle> Actor::get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface, int iterations, int max_rollout_turns)
{
    Ref<TreeActionBundle> ab = memnew(TreeActionBundle(surface, caster));
    MonteCarloTreeSearch mcts(ab->get_root(), max_rollout_turns, perfrom_random_actions_for_turn);
    mcts.run(iterations);

    // ofstream data_file_stream;
    // data_file_stream.open(std::string("last_action.graph"), std::ios::app);
    // draw_tree(ab->get_root(), 10, ab->get_root()->caster->get_faction(), data_file_stream);
    // data_file_stream.close();

    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<RandomActionBundle> ab = memnew(RandomActionBundle(surface, caster));
    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_pzts(Ref<Unit> caster, Ref<Surface> surface, int iterations, const String &model_file)
{
    Ref<TreeActionBundle> ab = memnew(TreeActionBundle(surface, caster));
    // ab->is_probabilistic = true; // TODO make configurable

    PlayerZeroTreeSearch pzts(ab->get_root(), 1.0, model_file.ascii().get_data());
    pzts.run(iterations);

    // ofstream data_file_stream;
    // data_file_stream.open(std::string("last_action.graph"), std::ios::app);
    // draw_tree(ab->get_root(), 8, ab->get_root()->caster->get_faction(), data_file_stream);
    // data_file_stream.close();

    // // Verifitcation code for #93
    // std::array<float, PZ_NUM_BOARD> board_arr = {};
    // populate_board_array(surface, caster->get_position(), board_arr);
    // ofstream board_fs;
    // board_fs.open(std::string("board_fs.txt"), std::ios::app);
    // for (float b : board_arr)
    // {
    //     board_fs << b << " ";
    // }
    // board_fs << "\n";
    // board_fs.close();

    // ofstream node_fs;
    // node_fs.open(std::string("node_fs.txt"), std::ios::app);
    // pzts.serialize_node(node_fs, ab->get_root(), ab->get_root()->caster->get_faction(), false);
    // node_fs << "\n";
    // node_fs.close();

    return ab;
}

void Actor::perfrom_auto_actions_for_turn(Ref<Unit> caster, Ref<Surface> surface)
{
    if (caster.is_null())
    {
        UtilityFunctions::printerr("Caster is null.");
        return;
    }

    if (caster->is_dead())
    {
        UtilityFunctions::printerr("Caster dead at the start of random cast.");
        return;
    }

    if (surface.is_null() || !surface.is_valid())
    {
        UtilityFunctions::printerr("Invalid surface.");
        return;
    }

    if (surface->turn_get_current_unit() != caster)
    {
        UtilityFunctions::printerr("Not current units turn.");
        return;
    }

    Ref<ActionBundle> ab = get_actions_from_decision_tree(caster, surface);

    while (!ab->is_finished())
    {
        ab->cast_next();
    }
}
