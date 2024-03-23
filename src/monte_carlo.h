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
    float score = 0;
    Node *parent;
    std::vector<Node *> children;
    // Cast information
    const ActionIdentifier action;
    Ref<Surface> surface;
    Ref<Unit> caster;
    const Vector2i target;

    Node(
        const ActionIdentifier p_action,
        Ref<Surface> p_surface,
        Ref<Unit> p_caster,
        const Vector2i p_target,
        Node *p_parent = nullptr)
        : parent(p_parent), action(p_action), surface(p_surface), caster(p_caster), target(p_target){};

    ~Node();
    bool is_leaf() const;
};

class ActuionCastNode : public Node
{
};

class RandomResultNode : public Node
{
};

using SelectionFunction = float (*)(Node *, Faction);
using ActionPerformerFunction = void (*)(Ref<Unit>, Ref<Surface>);
float ucb(Node *node, Faction root_faction);

class MonteCarloTreeSearch
{

protected:
    Node *root;
    const int max_simulation_depth;
    const SelectionFunction selection_func;
    const ActionPerformerFunction performer_func;

public:
    MonteCarloTreeSearch(
        Node *p_root,
        const int p_max_sim_depth,
        SelectionFunction p_select,
        ActionPerformerFunction p_performer)
        : root(p_root),
          max_simulation_depth(p_max_sim_depth),
          selection_func(p_select),
          performer_func(p_performer){};
    ~MonteCarloTreeSearch();

    virtual float calculate_surface_score(Ref<Surface> surface) const;
    Node *select(Node *node) const;
    Node *expand(Node *node);
    float simulate(Node *node);
    void backpropagate(Node *node, const float score);
    void run(const int iterations);
};

void serialize_unit(std::vector<float> &vec, Ref<Unit> unit);
std::vector<float> serialize_cast(CastInfo cast);
std::vector<float> serialize_state(Ref<Surface> surface);
void draw_tree(Node *node, int depth, Faction root_faction);
void serialize_tree_to_stream(const Node *node, std::ofstream &to_file, const int visits_threshold);

class WinPredictorTreeSearch : public MonteCarloTreeSearch
{

public:
    WinPredictorTreeSearch(
        Node *p_root,
        const int p_max_sim_depth,
        SelectionFunction p_select,
        ActionPerformerFunction p_performer)
        : MonteCarloTreeSearch(p_root, p_max_sim_depth, p_select, p_performer){};

    float calculate_surface_score(Ref<Surface> surface) const override;
};

#endif