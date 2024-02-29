#include "actor.h"
#include "action.h"

#include <algorithm>
#include <chrono>
#include <godot_cpp/variant/utility_functions.hpp>

void Actor::_bind_methods()
{
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_decision_tree", "caster", "surface"), &Actor::get_actions_from_decision_tree);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_mcts", "caster", "surface", "iterations", "max_rollout_turns"), &Actor::get_actions_from_mcts);
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

    while (!caster->is_dead()) // if caster kill himself after cast, cant continue to cast
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
        CastInfo random_cast = candidate_casts[cast_pt];
        if (!Action::_is_castable(random_cast))
        {
            UtilityFunctions::printerr("Uncastable action generated in random actor");
            UtilityFunctions::printerr(random_cast.action);
            UtilityFunctions::printerr(random_cast.target);

            std::cout << caster.is_null() << caster->is_dead() << (caster == surface->turn_get_current_unit()) << "\n";
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

    float rollout(Node *node)
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

    void run()
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "mcts run started\n";
        for (int i = 0; i < iterations; ++i)
        {
            Node *selectedNode = select_leaf(root); // select node that child of root
            Node *expandedNode = expand(selectedNode);
            float score = rollout(expandedNode);
            backpropagate(expandedNode, score);
        }
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start);
        std::cout << "mcts run finished in " << duration.count() << "ms \n";
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
};

Ref<ActionBundle> Actor::get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface, int interations, int max_rollout_turns)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);
    Ref<Surface> surface_clone = surface->clone();
    Ref<Unit> caster_clone = surface_clone->get_element(caster->get_position());
    Node *root = new Node(END_TURN, surface_clone, caster_clone, Vector2i(0, 0));
    MonteCarloTreeSearch mcts(root, interations, max_rollout_turns);
    mcts.run();

    Node *node = root;
    // mcts.draw_tree(node, 5);
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

    delete root;
    return ab;
}
