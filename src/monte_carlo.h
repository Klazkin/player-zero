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
    Node *parent = nullptr;
    std::vector<Node *> children;
    Ref<Surface> surface;
    Ref<Unit> caster;

    Node(Ref<Surface> p_surface, Ref<Unit> p_caster)
        : surface(p_surface), caster(p_caster){};

    ~Node();
    bool is_leaf() const;
    virtual ActionIdentifier get_action() const;
    virtual Vector2i get_target() const;
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
    const ActionIdentifier refilled_action;

    RandomHandRefillNode(Ref<Surface> p_surface, Ref<Unit> p_caster, ActionIdentifier p_refilled);
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