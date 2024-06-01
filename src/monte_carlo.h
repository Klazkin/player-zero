#ifndef MONTE_CARLO_H
#define MONTE_CARLO_H

#include <vector>
#include <iostream>
#include <fstream>

#include "action.h"
#include "ortbinding.h"

using namespace godot;

class Node
{

public:
    int visits = 0;
    float score = 0.0; // action value
    float policy = 0.0;
    Node *parent = nullptr;
    std::vector<Node *> children;
    Ref<Surface> surface;
    Ref<Unit> caster;

    Node(Ref<Surface> p_surface, Ref<Unit> p_caster)
        : surface(p_surface), caster(p_caster){};

    ~Node();
    bool is_leaf() const;
    bool is_terminal() const;
    virtual ActionIdentifier get_action() const;
    virtual Vector2i get_target() const;
    void add_child(Node *child);
};

class ActionCastNode : public Node
{

private:
    const ActionIdentifier action;
    const Vector2i target;

public:
    ActionCastNode(
        const ActionIdentifier p_action,
        Ref<Surface> p_surface,
        Ref<Unit> p_caster,
        const Vector2i p_target)
        : action(p_action), target(p_target), Node(p_surface, p_caster){};

    ActionIdentifier get_action() const override;
    Vector2i get_target() const override;
};

class RandomHandRefillNode : public Node
{
public:
    RandomHandRefillNode(Ref<Surface> p_surface, Ref<Unit> p_caster, ActionIdentifier p_refilled);
};

using ActionPerformerFunction = void (*)(Ref<Unit>, Ref<Surface>);
float ucb(Node *node, Faction root_faction);

class MonteCarloTreeSearch
{

protected:
    Node *root;
    const int max_simulation_depth;
    const ActionPerformerFunction performer_func;

public:
    MonteCarloTreeSearch(
        Node *p_root,
        const int p_max_sim_depth,
        ActionPerformerFunction p_performer)
        : root(p_root),
          max_simulation_depth(p_max_sim_depth),
          performer_func(p_performer){};
    ~MonteCarloTreeSearch();

    virtual float calculate_surface_score(Ref<Surface> surface) const;
    virtual float selection_policy(Node *node, Faction root_faction) const; // default UCB
    virtual Node *select(Node *node) const;
    virtual Node *expand(Node *node);
    virtual float simulate(Node *node);
    virtual void backpropagate(Node *node, const float score);
    void run(const int iterations);
    Node *expand_random(Node *node, bool from_blooddrawing, const Ref<Unit> next_caster);
    Node *expand_auto(Node *node, const Ref<Unit> next_caster);
};

void draw_tree(Node *node, int depth, Faction root_faction, std::ofstream &fstream);

class PlayerZeroTreeSearch : public MonteCarloTreeSearch
{

private:
    float cpuct;
    std::string model_file;

public:
    PlayerZeroTreeSearch(Node *p_root, float p_cpuct, std::string p_model_file)
        : cpuct(p_cpuct),
          model_file(p_model_file),
          MonteCarloTreeSearch(p_root, -1, nullptr){};

    float selection_policy(Node *node, Faction root_faction) const override;
    Node *expand(Node *node) override;
    Node *expand_random(Node *node, bool from_blooddrawing);
    float simulate(Node *node) override;
    void backpropagate(Node *node, const float score) override;

    static void serialize_node(std::ofstream &to_file, Node *node, const Faction root_faction, const bool propagate, const int visits_threshold);
    void test_for_multiple_factions(Node *node);
};

// TODO remove from header file
void populate_board_array(Ref<Surface> surface, const Vector2i &current_caster_position, std::array<float, PZ_NUM_BOARD> &arr);

#endif