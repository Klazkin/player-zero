#include "actor.h"
#include "action.h"
#include "ortbinding.h"

#include <algorithm>
#include <chrono>
#include <godot_cpp/variant/utility_functions.hpp>
#include <iostream>
#include <fstream>

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

class Node
{
public:
    int visits;
    float score;
    Ref<Unit> caster;
    Ref<Surface> surface;
    std::vector<Node *> children;
    Node *parent;

    const ActionIdentifier action;
    const Vector2i target;

    Node( // todo maybe attributes can be bunched up into cast info...
        const ActionIdentifier p_action,
        Ref<Surface> p_surface,
        Ref<Unit> p_caster,
        const Vector2i p_target)
        : caster(p_caster),
          visits(0),
          score(0),
          parent(nullptr),
          surface(p_surface),
          action(p_action),
          target(p_target)
    {
    }
    ~Node()
    {
        for (auto child : children)
        {
            delete child;
        }
    }

    bool is_leaf()
    {
        return children.size() == 0;
    }
};

const auto action_pool = {
    COMBINE_ACTIONS,
    WRATHSPARK,
    GROUNDRAISE,
    // BLOODDRAWING,
    TREAD,
    COILBLADE,
    ETERNALSHACLES,
    // ALTAR,
    NETHERSWAP,
    WISPSPARKS,
    BONEDUST,
    BONESPARKS,
    RESPIRIT,
    // SNOWMOTES,
    SUNDIVE,
    METEORSHATTER,
    ARMORCORE,
    IMMOLATION,
    // ICEPOLE,
    // OBLIVION,
    HOARFROST,
    RAPID_GROWTH,
}; // 22

const auto status_pool = {
    STATUS_BURN,
    STATUS_SLOW,
    STATUS_COUNTDOWN,
    STATUS_SHACLES,
    STATUS_DUSTED,
    STATUS_SPIRITING,
    STATUS_IMMOLATION,
    STATUS_CORE_ARMOR,
    STATUS_HOARFROST_ARMOR,
}; // 9

void serialize_unit(std::vector<float> &vec, Ref<Unit> unit)
{
    vec.push_back(unit->get_health());
    vec.push_back(unit->get_max_health());
    vec.push_back(unit->get_speed());

    for (ActionIdentifier action : action_pool)
        vec.push_back(unit->is_in_deck(action) ? 1 : 0);

    for (ActionIdentifier action : action_pool)
        vec.push_back(unit->is_in_hand(action) ? 1 : 0);

    for (UnitSubscriberIdentifier status : status_pool)
    {
        if (status == STATUS_CORE_ARMOR && unit->has_subscriber(STATUS_CORE_ARMOR))
        {
            auto ca_status = (CoreArmor *)unit->get_subscriber(STATUS_CORE_ARMOR);
            if (!ca_status->get_is_active())
            {
                vec.push_back(0);
                continue;
            }
        }

        vec.push_back(unit->get_subscriber_duration(status));
    }
}

std::vector<float> serialize_cast(CastInfo cast)
{
    std::vector<float> ret;

    Vector2i cast_offset = cast.action == END_TURN ? Vector2i(0, 0) : (cast.target - cast.caster->get_position());
    Ref<Unit> opponent = nullptr;

    for (auto u : cast.surface->get_only_units_vec())
    {
        if (u == cast.caster)
        {
            continue;
        }

        if (opponent == nullptr)
        {
            opponent = u;
            continue;
        }

        std::cout << "SC: More than one opponennt during serialization.\n";
    }

    if (opponent == nullptr)
    {
        std::cout << "SC: Zero opponents in serialization.\n";
        return ret;
    }

    Vector2i opponent_offset = (opponent->get_position() - cast.caster->get_position());
    ret.push_back(cast.action == END_TURN);

    for (ActionIdentifier action : action_pool)
        ret.push_back(cast.action == action);

    ret.push_back(cast_offset.x);
    ret.push_back(cast_offset.y);
    serialize_unit(ret, cast.caster);
    ret.push_back(opponent_offset.x);
    ret.push_back(opponent_offset.y);
    serialize_unit(ret, opponent);
    return ret;
}

std::vector<float> serialize_state(Ref<Surface> surface)
{
    std::vector<float> ret;
    Ref<Unit> player = nullptr;
    Ref<Unit> monster = nullptr;

    for (auto u : surface->get_only_units_vec())
    {
        if (u->get_faction() == PLAYER)
        {
            player = u;
        }

        if (u->get_faction() == MONSTER)
        {
            monster = u;
        }
    }

    if (player == nullptr || monster == nullptr)
    {
        std::cout << "unable to serialize state, one of the factions is missing\n";
        return ret;
    }

    ret.push_back(player->get_position().x);
    ret.push_back(player->get_position().y);
    serialize_unit(ret, player);

    ret.push_back(monster->get_position().x);
    ret.push_back(monster->get_position().y);
    serialize_unit(ret, monster);
    return ret;
}

static WinPredictor *model = nullptr;

class MonteCarloTreeSearch
{
public:
    Node *root;
    int iterations;
    int max_rollout_turns;

    MonteCarloTreeSearch(Node *root, int iterations, int max_rollout_turns) : root(root), iterations(iterations), max_rollout_turns(max_rollout_turns) {}

    float ucb(Node *node)
    {
        if (node->visits == 0)
        {
            return std::numeric_limits<float>::infinity();
        }

        if (node->parent == nullptr)
        {
            return 0;
        }

        float score_multiplier = node->caster->get_faction() == root->caster->get_faction() ? 1.0 : -1.0;
        float exploit = node->score / node->visits;
        float explore = std::sqrt(2 * std::log(node->parent->visits) / node->visits);
        return (exploit * score_multiplier) + explore;
    }

    Node *select_leaf(Node *node)
    {
        Node *selected = node;
        while (!selected->is_leaf())
        {
            float best_ucb = -std::numeric_limits<float>::infinity();
            for (auto child : selected->children)
            {
                float child_ucb = ucb(child);
                if (child_ucb > best_ucb)
                {
                    best_ucb = child_ucb;
                    selected = child;
                }
            }
        }
        return selected;
    }

    float eval_surface_score(Faction faction, Ref<Surface> surface)
    {
        if (surface->get_winner() == faction)
        {
            return 1;
        }
        if (surface->get_winner() != UNDEFINED)
        {
            return -1;
        }

        return 0;
    }

    Node *expand(Node *node)
    {
        if (node->visits == 0) // node is unvisited
        {
            return node;
        }

        if (node->surface->get_remaining_factions_count() <= 1 || node->caster == nullptr) // terminal node
        {
            return node;
        }

        Ref<Unit> next_caster = node->surface->turn_get_current_unit(); // should never be a dead unit!!!

        if (next_caster.is_null() || !next_caster.is_valid())
        {
            std::cout << "Next clone is not valid or null" << next_caster.is_null() << next_caster.is_valid() << "\n";
            return node;
        }

        if (next_caster->is_dead())
        {
            std::cout << "Next clone is dead. ";
            return node;
        }

        for (auto action : next_caster->get_hand_set())
        {
            for (auto original_cast : Action::generate_action_casts({action, node->surface, next_caster, Vector2i()}))
            {
                Ref<Surface> surface_clone = node->surface->clone(); // todo check how handling of REF to PTR conversion goes
                Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));

                if (next_caster_clone.is_null() || !next_caster_clone.is_valid())
                {
                    std::cout << "Caster clone is not valid or null " << next_caster_clone.is_null() << next_caster_clone.is_valid() << "\n";
                    std::cout << surface_clone->get_only_units_vec().size() << "\n";
                    std::cout << surface_clone->get_winner() << "\n";
                }

                CastInfo clone_cast = {
                    action,
                    surface_clone,
                    next_caster_clone,
                    original_cast.target,
                };

                if (Action::_is_castable(clone_cast))
                {
                    Action::_cast_action(clone_cast);
                    Node *newNode = new Node(action, surface_clone, next_caster_clone, clone_cast.target);
                    newNode->parent = node;
                    node->children.push_back(newNode);
                }
                else
                {
                    std::cerr << "Unable to cast during mcts, action: " << action << "\n";
                }
            }
        }

        if (node->is_leaf())
        {
            std::cerr << "Node remained as leaf after expansion, something is going wrong\n";
            return node;
        }

        return node->children[0];
    }

    float rollout(Node *node, const bool use_model)
    {
        if (node == nullptr)
        {
            return 0;
        }

        if (node->caster == nullptr) // caster is nullptr if parent turn_get_current_unit is nullptr, i.e. no units left on surface
        {
            return 0; // eval_surface_score(UNDEFINED, node->surface);
        }

        Ref<Surface> rollout_surface = node->surface->clone();
        int rollout_turns = 0;
        while (rollout_surface->get_remaining_factions_count() > 1 && rollout_turns++ < max_rollout_turns)
        {
            Ref<Unit> current_unit = rollout_surface->turn_get_current_unit();
            if (current_unit == nullptr) // case if no units left on surface
            {
                break;
            }

            if (current_unit->is_dead()) // case if no units left on surface
            {
                std::cout << "Critical: dead unit in rollout\n";
                break;
            }

            Actor::perfrom_random_actions_for_turn(current_unit, rollout_surface); // todo research other rollout directors
        }

        if (use_model && rollout_surface->get_remaining_factions_count() > 1)
        {

            std::vector<float> surface_state_vec = serialize_state(rollout_surface);
            std::array<float, 96> arr;
            std::copy_n(surface_state_vec.begin(), 96, arr.begin());

            if (model == nullptr)
            {
                std::cout << "NULL MODEL";
                model = new WinPredictor();
                // return 0;
            }

            if (model == nullptr)
            {
                std::cout << "Still bad";
                return 0;
            }

            std::array<float, 2> result = model->predict(arr);
            float multiplier = root->caster->get_faction() == PLAYER ? 1.0 : -1.0;
            // std::cout << "prediction:" << result[0] << " " << result[1] << "\n";
            return multiplier * (result[0] - result[1]);
        }

        return eval_surface_score(root->caster->get_faction(), rollout_surface);
    }

    void backpropagate(Node *node, float score)
    {
        while (node != nullptr)
        {
            node->score += score;
            node->visits++;
            node = node->parent;
        }
    }

    void run(bool const rollout_use_model = false)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << (rollout_use_model ? "wp_" : "") << "mcts run started\n";
        for (int i = 0; i < iterations; ++i)
        {
            Node *selectedNode = select_leaf(root); // select node that child of root
            Node *expandedNode = expand(selectedNode);
            float score = rollout(expandedNode, rollout_use_model);
            backpropagate(expandedNode, score);
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << (rollout_use_model ? "wp_" : "") << "mcts run finished in " << duration.count() << "ms \n";
    }

    void draw_tree(Node *node, int depth)
    {
        if (!node->is_leaf() && depth > 0)
        {
            std::cout << "(";
            bool is_first = true;
            for (auto child : node->children)
            {
                if (!is_first)
                {
                    std::cout << ",";
                }
                draw_tree(child, depth - 1);
                is_first = false;
            }
            std::cout << ")";
        }

        std::cout << node->action << "/"
                  << node->target.x << "/"
                  << node->target.y << "/"
                  << node->visits << "/"
                  << node->score << "/"
                  << ucb(node) << "/"
                  << (node->caster == nullptr
                          ? UNDEFINED
                          : node->caster->get_faction());
        if (node->parent == nullptr) // is root.
        {
            std::cout << ";\n";
        }
    }

    void serialize_tree_to_stream(const Node *node, ofstream &to_file, const int visits_threshold)
    {
        if (node->visits <= visits_threshold)
        {
            return;
        }

        int max_visits = -1;

        for (auto n : node->children)
        {
            max_visits = std::max(n->visits, max_visits);
        }

        for (auto n : node->children)
        {
            serialize_node_to_stream(n, to_file, ((float)n->visits / (float)max_visits));
            serialize_tree_to_stream(n, to_file, visits_threshold);
        }
    }

    void serialize_node_to_stream(Node *node, ofstream &to_file, float score)
    {

        Vector2i cast_offset = (node->target - node->caster->get_position());
        // float score_multiplier = (node->caster->get_faction() == root->caster->get_faction()) ? 1.0 : -1.0; // invert score for enemy

        Ref<Unit> parent_node_caster = nullptr;
        for (auto u : node->parent->surface->get_only_units_vec()) // todo replace this with a less bad workaround
        {
            if (u->get_faction() == node->caster->get_faction())
            {
                parent_node_caster = u;
                break;
            }
        }

        if (parent_node_caster == nullptr)
        {
            std::cout << "parent_node_caster caster not found???\n";
            return;
        }

        CastInfo cast_with_parent_state = {
            node->action,
            node->parent->surface,
            parent_node_caster,
            node->target,
        };
        auto data_vector = serialize_cast(cast_with_parent_state);

        if (data_vector.size() != 114)
        {
            std::cout << "Serialize Node: vector wrong size.\n";
            return;
        }

        for (auto x : data_vector)
        {
            to_file << x << ',';
        }

        to_file
            << node->visits << ','
            << score << '\n';
    }
};

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
    MonteCarloTreeSearch mcts(root, interations, max_rollout_turns);
    mcts.run();

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
    MonteCarloTreeSearch mcts(root, interations, max_rollout_turns);
    mcts.run(true);

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