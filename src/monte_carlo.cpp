#include "monte_carlo.h"
#include "status.h"
#include "actor.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <string>
#include <fstream>
#include <godot_cpp/variant/utility_functions.hpp>

Node::~Node()
{
    for (auto child : children)
    {
        delete child;
    }
}

bool Node::is_leaf() const
{
    return children.size() == 0;
}

bool Node::is_terminal() const
{
    return surface->get_remaining_factions_count() <= 1;
}

ActionIdentifier ActionCastNode::get_action() const
{
    return action;
}

Vector2i ActionCastNode::get_target() const
{
    return target;
}

ActionIdentifier Node::get_action() const
{
    return INVALID_ACTION;
}

RandomHandRefillNode::RandomHandRefillNode(Ref<Surface> p_surface, Ref<Unit> p_caster, ActionIdentifier p_refilled) : Node(p_surface, p_caster)
{
}

Vector2i Node::get_target() const
{
    return Vector2i();
}

void Node::add_child(Node *child)
{
    if (child->parent != nullptr)
    {
        std::cerr << "attempting to add an already connected node\n";
        return;
    }

    child->parent = this;
    children.push_back(child);
}

float ucb(Node *node, Faction root_faction)
{
    if (node->visits == 0)
    {
        return std::numeric_limits<float>::infinity();
    }

    if (node->parent == nullptr) // root node
    {
        return 0.0;
    }

    float score_multipler = node->caster->get_faction() == root_faction ? 1.0 : -1.0;
    // float exploit = node->score * score_multipler / node->visits;
    float exploit = node->score * score_multipler;
    float explore = std::sqrt(2 * std::log(node->parent->visits) / node->visits);
    return exploit + explore;
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
}

float MonteCarloTreeSearch::calculate_surface_score(Ref<Surface> surface) const
{
    if (surface->get_winner() == root->caster->get_faction())
    {
        return 1.0;
    }
    if (surface->get_winner() != UNDEFINED)
    {
        return -1.0;
    }

    return 0.0;
}

float MonteCarloTreeSearch::selection_policy(Node *node, Faction root_faction) const
{
    return ucb(node, root_faction);
}

Node *MonteCarloTreeSearch::select(Node *node) const
{
    Node *selected = node;
    while (!selected->is_leaf())
    {
        float best_selected = -std::numeric_limits<float>::infinity();
        for (auto child : selected->children)
        {
            float child_selection_metric;
            if (child->get_action() == INVALID_ACTION) // if is a random node
            {
                child_selection_metric = -child->visits; // pick with least visits
            }
            else
            {
                child_selection_metric = selection_policy(child, root->caster->get_faction());
            }

            if (child_selection_metric > best_selected)
            {
                best_selected = child_selection_metric;
                selected = child;
            }
        }
    }
    return selected;
}

Node *MonteCarloTreeSearch::expand(Node *node)
{
    if (node->is_terminal())
    {
        return node;
    }

    Ref<Unit> next_caster = node->surface->turn_get_current_unit(); // should never be a dead unit!!!

    if (next_caster->is_dead() || next_caster.is_null() || !next_caster.is_valid())
    {
        std::cout << "Next caster is not valid or null (" << next_caster.is_null() << next_caster.is_valid() << ") \n";
        std::cout << "# of factions: " << node->surface->get_remaining_factions_count() << "\n";
        std::cout << "# of units: " << node->surface->get_only_units_vec().size() << "\n";
        std::cout << "Node: " << node->get_action() << " ";
        std::cout << "Parent: " << node->parent->get_action() << "\n";
        std::cout << "Node caster: " << node->caster->is_autonomous() << "\n";
        std::cout << "history of actions:" << "\n";
        std::cout << "NC addr: " << as_unit_ptr(next_caster) << "\n";
        std::cout << "NU addr: " << as_unit_ptr(nullptr) << "\n";
        std::cout << "COUNTER: " << node->surface->get_turn_counter() << "\n";
        std::cout << "TURN: ";

        auto turn_order = node->surface->turn_get_order();

        for (size_t i = 0; i < turn_order.size(); i++)
        {
            std::cout << as_unit_ptr(turn_order[i]) << ", ";
            /* code */
        }
        std::cout << "\n";

        turn_order = node->parent->surface->turn_get_order();

        std::cout << "COUNTER: " << node->parent->surface->get_turn_counter() << "\n";
        std::cout << "TURN: ";

        for (size_t i = 0; i < turn_order.size(); i++)
        {
            std::cout << as_unit_ptr(turn_order[i]) << ", ";
            /* code */
        }
        std::cout << "\n";

        Node *cur = node;
        // std::unordered_set<Node *> ps;

        while (cur != nullptr)
        {
            auto cpos = cur->caster->get_position();
            auto tpos = cur->get_target();

            std::cout << " A: " << cur->get_action()
                      << " C: " << cpos.x << "," << cpos.y
                      << " T: " << tpos.x << "," << tpos.y << "\n";

            // if (ps.count(cur) > 0)
            // {
            //     std::cout << "Cyclic graph.\n";
            //     break;
            // }
            // else
            // {
            //     ps.insert(cur);
            // }

            cur = cur->parent;
        }

        std::cout << "\n";
        return node;
    }

    if (next_caster->is_dead())
    {
        std::cout << "Next caster is dead. ";
        return node;
    }

    if (next_caster->is_autonomous())
    {
        return expand_auto(node, next_caster);
    }

    if (node->get_action() == END_TURN)
    {
        return expand_random(node, false, next_caster);
    }

    if (node->get_action() == BLOODDRAWING)
    {
        return expand_random(node, true, next_caster);
    }

    for (auto action : next_caster->get_hand_set())
    {
        for (auto original_cast : Action::generate_action_casts({action, node->surface, next_caster, Vector2i()}))
        {
            Ref<Surface> surface_clone = node->surface->clone();
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
                Node *new_node = new ActionCastNode(action, surface_clone, next_caster_clone, clone_cast.target);
                node->add_child(new_node);
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

float MonteCarloTreeSearch::simulate(Node *node)
{
    if (node == nullptr)
    {
        std::cerr << "null node in tree";
        return 0;
    }

    if (node->caster == nullptr) // caster is nullptr if parent turn_get_current_unit is nullptr, i.e. no units left on surface
    {
        std::cout << "null ptr caster";
        return 0;
    }

    Ref<Surface> rollout_surface = node->surface->clone();
    rollout_surface->set_random_events_enabled(true);

    int simulation_depth = 0;
    while (rollout_surface->get_remaining_factions_count() > 1 && simulation_depth++ < max_simulation_depth)
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

        // std::cout << "{";
        if (current_unit->is_autonomous())
        {
            Actor::perfrom_auto_actions_for_turn(current_unit, rollout_surface);
        }
        else
        {
            performer_func(current_unit, rollout_surface);
        }
        // std::cout << "}";
    }

    return calculate_surface_score(rollout_surface);
}

void MonteCarloTreeSearch::backpropagate(Node *node, const float score)
{
    while (node != nullptr)
    {
        node->score = (node->visits * node->score + score) / (node->visits + 1);
        // node->score += score;
        node->visits++;
        node = node->parent;
    }
}

void MonteCarloTreeSearch::run(const int iterations)
{

    // auto myid = this_thread::get_id();
    // std::stringstream ss;
    // ss << myid;
    // std::string thread_id_str = ss.str();
    // std::string filename = "R:\\SERB_log_" + thread_id_str + ".txt";
    // std::ofstream file(filename);

    for (int i = 0; i < iterations; ++i)
    {

        // file << 'S' << std::flush;
        Node *selected_node = select(root);
        // file << 'E' << std::flush;
        Node *expanded_node = expand(selected_node);
        // file << 'R' << std::flush;
        float score = simulate(expanded_node);
        // file << 'B' << std::flush;
        backpropagate(expanded_node, score);
    }

    // file << "F" << std::flush;
    // file.close();
}

Node *MonteCarloTreeSearch::expand_random(Node *node, bool from_blooddrawing, const Ref<Unit> next_caster)
{
    for (auto action : next_caster->get_refill_candidates())
    {
        if ((from_blooddrawing && action == BLOODDRAWING) || action == TREAD)
        {
            continue;
        }

        Ref<Surface> surface_clone = node->surface->clone();
        Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));
        next_caster_clone->refill_hand(action);

        Node *child = new RandomHandRefillNode(surface_clone, next_caster_clone, action);
        node->add_child(child);
    }

    if (node->is_leaf()) // nothing to refill, add empty refill node
    {
        Ref<Surface> surface_clone = node->surface->clone();
        Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));
        next_caster_clone->refill_hand(INVALID_ACTION);

        Node *child = new RandomHandRefillNode(surface_clone, next_caster_clone, INVALID_ACTION);
        node->add_child(child);
    }

    return node->children[0];
}

Node *MonteCarloTreeSearch::expand_auto(Node *node, const Ref<Unit> next_caster)
{
    Ref<Surface> surface_clone = node->surface->clone();
    Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));
    Ref<ActionBundle> ab = Actor::get_actions_from_decision_tree(next_caster_clone, surface_clone);

    next_caster_clone->refill_hand();
    while (!ab->is_finished())
    {
        ab->cast_next();
    }

    Node *next_node = new ActionCastNode(END_TURN, surface_clone, next_caster_clone, Vector2i());
    node->add_child(next_node);
    return next_node;
}

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

void populate_board_array(Ref<Surface> surface, const Vector2i &current_caster_position, std::array<float, PZ_NUM_BOARD> &arr)
{
    for (auto key_val_pair : surface->get_element_positions())
    {
        auto position = key_val_pair.first;
        auto element = key_val_pair.second;
        int arr_index = (position.x * 12 + position.y) * (18 + 30 * 2);

        if (!element->is_unit())
        {
            arr[arr_index] = 1.0;
            arr[arr_index + 4] = element->get_health();
            continue;
        }
        auto unit = as_unit_ptr(element);

        // faction [3]
        // arr[arr_index] = 0 for non-unit elements
        arr[arr_index + 1] = unit->get_faction() == PLAYER;
        arr[arr_index + 2] = unit->get_faction() == MONSTER;
        // is controlled [1]
        arr[arr_index + 3] = unit->get_position() == current_caster_position;
        // attributes [4]
        arr[arr_index + 4] = unit->get_health();
        arr[arr_index + 5] = unit->get_base_max_health();
        arr[arr_index + 6] = unit->get_base_speed();
        arr[arr_index + 7] = unit->get_base_attack();
        arr[arr_index + 8] = unit->get_base_defence();
        // statueses [9 - 18]
        int s = 1;
        for (auto status : status_pool)
        {
            if (status == STATUS_CORE_ARMOR && unit->has_subscriber(STATUS_CORE_ARMOR))
            {
                auto ca_status = (CoreArmor *)unit->get_subscriber(STATUS_CORE_ARMOR);
                if (!ca_status->get_is_active())
                {
                    continue;
                }
            }

            arr[arr_index + 8 + s] = unit->get_subscriber_duration(status);
            s++;
        }

        // hand
        for (size_t i = 0; i < 30; i++)
        {
            arr[arr_index + 18 + i] = unit->is_in_hand(static_cast<ActionIdentifier>(i));
        }

        // deck
        for (size_t i = 0; i < 30; i++)
        {
            arr[arr_index + 18 + 30 + i] = unit->is_in_deck(static_cast<ActionIdentifier>(i));
        }
    }
}

void draw_tree(Node *node, int depth, Faction root_faction, std::ofstream &fstream)
{
    if (!node->is_leaf() && depth > 0)
    {
        fstream << "(";
        bool is_first = true;
        for (auto child : node->children)
        {
            if (!is_first)
            {
                fstream << ",";
            }
            draw_tree(child, depth - 1, root_faction, fstream);
            is_first = false;
        }
        fstream << ")";
    }

    fstream << node->get_action() << "/"
            << node->get_target().x << "/"
            << node->get_target().y << "/"
            << node->visits << "/"
            << node->score << "/"
            << node->policy << "/"
            << (node->caster == nullptr
                    ? UNDEFINED
                    : node->caster->get_faction());
    if (node->parent == nullptr) // is root.
    {
        fstream << ";\n";
    }
}

/*
select:
    get all valid moves for current node (those that were generated by policy)
    find best action using upper confidence bound;

    s - state
    a - action
    cpuct - exploration temp = 1
    Q(s, a) = action value (0 at start)
    P(s, a) = policy raiting
    N = Times parent node was visited
    n = Times given node was visited
    EPS = 1e-8
    ---
    if visited ==> U(s, a) = Q(s, a) + cpuct * P(s, a) * sqrt(N) / (1 + n)
    else       ==> U(s, a) = cpuct * P(s, a) * sqrt(N + EPS)

    Search until reaches a leaf?
*/

const float EPS = 1e-8;

float PlayerZeroTreeSearch::selection_policy(Node *node, Faction root_faction) const
{
    if (node->visits == 0)
    {
        return cpuct * node->policy * std::sqrt(node->parent->visits + EPS);
    }
    float score_multipler = node->caster->get_faction() == root->caster->get_faction() ? 1.0 : -1.0;
    return score_multipler * node->score + cpuct * node->policy * std::sqrt(node->parent->visits) / (1 + node->visits);
}

/*
expand:
    if not terminal (is leaf)...
        predict value (winner) from -1 to 1, and policy             --- NN prediction happens here.
        get valid moves from generator
        mask policy and valid moves
        get the normalized policy
        (exception, all valid moves were maked)

    add child nodes for possible moves
    visited = 0?
*/

int get_cast_index(const CastInfo &ci)
{
    return (ci.target.x * 12 + ci.target.y) * 30 + ci.action;
}

Node *PlayerZeroTreeSearch::expand(Node *node)
{
    if (node->is_terminal())
    {
        node->score = calculate_surface_score(node->surface);
        return node;
    }

    if (node->get_action() == END_TURN)
    {
        return expand_random(node, false);
    }

    if (node->get_action() == BLOODDRAWING)
    {
        return expand_random(node, true);
    }

    Ref<Unit> next_caster = node->surface->turn_get_current_unit(); // should never be a dead unit!!!

    if (next_caster.is_null() || !next_caster.is_valid())
    {
        std::cout << "Next caster is not valid or null" << next_caster.is_null() << next_caster.is_valid() << "\n";
        return node;
    }

    if (next_caster->is_dead())
    {
        std::cout << "Next caster is dead. ";
        return node;
    }

    std::array<float, PZ_NUM_POLICY> mask = {};
    std::array<float, PZ_NUM_BOARD> board = {};
    PZPrediction prediction;
    std::vector<CastInfo> possible_casts;

    for (auto action : next_caster->get_hand_set())
    {
        for (auto ci : Action::generate_action_casts({action, node->surface, next_caster, Vector2i()}))
        {
            possible_casts.push_back(ci);
            mask[get_cast_index(ci)] = 1.0;
        }
    }

    populate_board_array(node->surface, next_caster->get_position(), board);
    PlayerZeroPredictor::get(model_file)->predict(board, mask, prediction);

    float policy_sum = 0;
    for (auto ci : possible_casts)
    {
        policy_sum += prediction.policy[get_cast_index(ci)];
    }

    if (std::abs(policy_sum - 1.0) > 0.1) // 0.1 is the allowed delta for error
    {
        std::cout << "Policy does not add up to 1.0; " << policy_sum << "\n";
        std::ofstream fstream;
        fstream.open(std::string("policy_sum_above_1.txt"), std::ios::app);
        fstream << "Policy sum: " << policy_sum << "\n";
        fstream << "N Casts: " << possible_casts.size() << "\n";
        fstream << "Casts: ";

        for (auto ci : possible_casts)
        {
            fstream << "(action " << ci.action << " at " << ci.target.x << ":" << ci.target.y << "), ";
        }
        fstream << "\n";

        serialize_node(fstream, node, root->caster->get_faction(), true, 500);
        fstream << "0\n0\n0\nBoard:\n";
        for (auto b : board)
        {
            fstream << b << ",";
        }
        fstream << "\nPolicy:\n";
        for (auto p : prediction.policy)
        {
            fstream << p << ",";
        }
        fstream.close();
        exit(10131);
    }

    if (prediction.value[0] > 1.01 || prediction.value[0] < -1.01)
    {
        std::cout << "Value outside of range; " << prediction.value[0] << "\n";
    }

    if (possible_casts.size() == 0)
    {
        std::cout << "Hand size: " << next_caster->get_hand_set().size() << "\n";
        std::cout << "Deck size: " << next_caster->get_deck().size() << "\n";
        std::cout << "Refill Candidates size: " << next_caster->get_refill_candidates().size() << "\n";
        std::cout << "Zero possible casts!\n";
        exit(10000);
        return node;
    }

    for (auto ci : possible_casts)
    {
        Ref<Surface> surface_clone = node->surface->clone();
        Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));

        CastInfo clone_cast = {
            ci.action,
            surface_clone,
            next_caster_clone,
            ci.target,
        };

        if (Action::_is_castable(clone_cast))
        {
            Action::_cast_action(clone_cast);
            Node *new_node = new ActionCastNode(ci.action, surface_clone, next_caster_clone, clone_cast.target);

            float action_policy = prediction.policy[get_cast_index(ci)];
            if (action_policy >= 1.01 || action_policy < 0.0)
            {
                std::cout << "action_policy of " << action_policy << "! sum: " << policy_sum << "\n";
                std::ofstream fstream;
                fstream.open(std::string("board_input_large_policy.txt"), std::ios::app);
                fstream << "Policy: " << action_policy << "\n";
                fstream << "Action: " << ci.action << " at " << ci.target.x << ":" << ci.target.y << "\n";
                serialize_node(fstream, node, root->caster->get_faction(), false, 0);
                fstream << "0\n0\n0\nBoard:\n";
                for (auto b : board)
                {
                    fstream << b << ",";
                }
                fstream << "\nPolicy:\n";
                for (auto p : prediction.policy)
                {
                    fstream << p << ",";
                }
                fstream.close();
                exit(10132);
            }
            new_node->policy = action_policy;
            node->add_child(new_node);
        }
        else
        {
            std::cerr << "Unable to cast during mcts, action: " << ci.action << "\n";
        }
    }

    float score_multipler = next_caster->get_faction() == root->caster->get_faction() ? 1.0 : -1.0;
    node->score = score_multipler * prediction.value[0];
    return node; // return expanded node
}

Node *PlayerZeroTreeSearch::expand_random(Node *node, bool from_blooddrawing)
{
    Ref<Unit> next_caster = node->surface->turn_get_current_unit();
    std::array<float, PZ_NUM_POLICY> empty_mask = {};
    std::array<float, PZ_NUM_BOARD> board = {};
    PZPrediction prediction;

    populate_board_array(node->surface, next_caster->get_position(), board);
    PlayerZeroPredictor::get(model_file)->predict(board, empty_mask, prediction);

    for (auto action : next_caster->get_refill_candidates())
    {
        if ((from_blooddrawing && action == BLOODDRAWING) || action == TREAD)
        {
            continue;
        }

        Ref<Surface> surface_clone = node->surface->clone();
        Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));
        next_caster_clone->refill_hand(action);

        Node *child = new RandomHandRefillNode(surface_clone, next_caster_clone, action);
        node->add_child(child);
    }

    if (node->is_leaf()) // nothing to refill, add empty refill node
    {
        Ref<Surface> surface_clone = node->surface->clone();
        Ref<Unit> next_caster_clone = as_unit_ptr(surface_clone->get_element(next_caster->get_position()));
        next_caster_clone->refill_hand(INVALID_ACTION);

        Node *child = new RandomHandRefillNode(surface_clone, next_caster_clone, INVALID_ACTION);
        node->add_child(child);
    }

    float score_multipler = next_caster->get_faction() == root->caster->get_faction() ? 1.0 : -1.0;
    node->score = score_multipler * prediction.value[0];
    return node;
}

/*
simulate:
    if game ended [TERMINAL] -> evaluate
    game did not end -> use the network prediction
*/

float PlayerZeroTreeSearch::simulate(Node *node)
{
    return node->score; // network prediction value (already multiplied by faction score_multiplier) or if terminal the evaluated score
}

/*
backpropagate:
    iterate times vistied
    update action value Q(s, a);
    v - (from the NN)
    ---

    if visited 0 times ==> Q(s, a) = v
    else               ==> Q(s, a) = (n * Q(S, a) + v) / (n + 1)
*/

void PlayerZeroTreeSearch::backpropagate(Node *node, const float score)
{
    while (node != nullptr)
    {
        if (node->visits > 0)
        {
            node->score = (node->visits * node->score + score) / (node->visits + 1);
        }
        node->visits++;
        node = node->parent;
    }
}

float clamp_score(float score)
{
    return std::clamp(score, -0.99999f, 0.99999f);
}

void PlayerZeroTreeSearch::serialize_node(std::ofstream &to_file, Node *node, const Faction root_faction, const bool propagate, const int visits_threshold)
{
    to_file << node->surface->get_element_positions().size() << '\n';

    for (auto key_val_pair : node->surface->get_element_positions())
    {
        auto position = key_val_pair.first;
        auto element = key_val_pair.second;
        to_file << position.x << ','
                << position.y << ',';
        if (!element->is_unit())
        {
            to_file << "1,0,0,0," << element->get_health() << '\n';
            continue;
        }

        auto unit = as_unit_ptr(element);
        to_file << (unit->get_faction() == UNDEFINED) << ','
                << (unit->get_faction() == PLAYER) << ','
                << (unit->get_faction() == MONSTER) << ','
                << (unit == *node->caster) << ','
                << unit->get_health() << ','
                << unit->get_base_max_health() << ','
                << unit->get_base_speed() << ','
                << unit->get_base_attack() << ','
                << unit->get_base_defence();

        for (auto status : status_pool)
        {
            to_file << ',';
            if (status == STATUS_CORE_ARMOR && unit->has_subscriber(STATUS_CORE_ARMOR))
            {
                auto ca_status = (CoreArmor *)unit->get_subscriber(STATUS_CORE_ARMOR);
                if (!ca_status->get_is_active())
                {
                    to_file << '0';
                    continue;
                }
            }

            to_file << unit->get_subscriber_duration(status);
        }

        // hand
        for (size_t i = 0; i < 30; i++)
        {
            to_file << ',' << unit->is_in_hand(static_cast<ActionIdentifier>(i));
        }

        // deck
        for (size_t i = 0; i < 30; i++)
        {
            to_file << ',' << unit->is_in_deck(static_cast<ActionIdentifier>(i));
        }

        to_file << '\n';
    };

    to_file << node->children.size() << '\n';

    float score_sum = 0;
    float score_multiplier = node->caster->get_faction() == root_faction ? 1.0 : -1.0;

    for (auto child : node->children)
    {
        score_sum += clamp_score(child->score) * score_multiplier + 1;
    }

    if (score_sum == 0)
    {
        std::cout << "ERR: exp_sum == 0\n";
    }

    for (auto child : node->children)
    {
        // see comment above to explain the clamp
        float policy = (clamp_score(child->score) * score_multiplier + 1) / score_sum;

        if (std::isnan(policy) || policy < 0 || policy > 1)
        {
            std::cout << "ERR: true_action_policy is invalid: ";
            std::cout << child->get_target().x << ','
                      << child->get_target().y << ','
                      << child->get_action() << ','
                      << child->policy << ','
                      << policy << ','
                      << child->visits << ','
                      << child->score << '\n';

            policy = 1.0 / node->children.size();
            std::cout << " failsafe policy: " << policy << "\n";
        }

        to_file << child->get_target().x << ','
                << child->get_target().y << ','
                << child->get_action() << ','
                << child->policy << ','
                << policy << ','
                << child->visits << ','
                << child->score << '\n';
    }

    if (!propagate)
    {
        return;
    }

    Node *best_child = nullptr;
    int best_visits = 0;

    for (auto child : node->children)
    {
        if (child->visits > best_visits)
        {
            best_visits = child->visits;
            best_child = child;
        }
    }

    if (best_child != nullptr && best_child->get_action() == BLOODDRAWING)
    {
        if (!best_child->is_leaf())
        {
            best_child = best_child->children[UtilityFunctions::randi_range(0, best_child->children.size() - 1)];
        }
        else
        {
            return;
        }
    }

    if (best_visits < visits_threshold ||
        best_child->is_terminal() ||
        best_child->get_action() == INVALID_ACTION ||
        best_child->get_action() == END_TURN ||
        best_child->caster->get_faction() != node->caster->get_faction())
    {
        return;
    }

    serialize_node(to_file, best_child, root_faction, true, visits_threshold);
}

void PlayerZeroTreeSearch::test_for_multiple_factions(Node *node)
{
    Faction first_child_faction = UNDEFINED;
    for (auto c : node->children)
    {
        if (first_child_faction == UNDEFINED)
        {
            first_child_faction = c->caster->get_faction();
            continue;
        }

        if (first_child_faction != c->caster->get_faction())
        {
            std::cout << "CRITICAL: Multiple factions share a branch!!";
            break;
        }

        test_for_multiple_factions(c);
    }
}
