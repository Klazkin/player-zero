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
    float score = 0; // action value
    float policy = 0;
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
};

void serialize_unit(std::vector<float> &vec, Ref<Unit> unit);
std::vector<float> serialize_cast(CastInfo cast);
std::vector<float> serialize_state(Ref<Surface> surface);
void draw_tree(Node *node, int depth, Faction root_faction, std::ofstream &fstream);
void serialize_tree_to_stream(const Node *node, std::ofstream &to_file, const int visits_threshold);

class WinPredictorTreeSearch : public MonteCarloTreeSearch
{

public:
    WinPredictorTreeSearch(
        Node *p_root,
        const int p_max_sim_depth,
        ActionPerformerFunction p_performer)
        : MonteCarloTreeSearch(p_root, p_max_sim_depth, p_performer){};

    float calculate_surface_score(Ref<Surface> surface) const override;
};

void populate_board_array(Ref<Surface> surface, const Vector2i &current_caster_position, std::array<float, PZ_NUM_BOARD> &board);

class PlayerZeroTreeSearch : public MonteCarloTreeSearch
{

private:
    float cpuct;

public:
    PlayerZeroTreeSearch(
        Node *p_root,
        float p_cpuct)
        : cpuct(p_cpuct),
          MonteCarloTreeSearch(p_root, -1, nullptr){};

    float selection_policy(Node *node, Faction root_faction) const override;
    float calculate_surface_score(Ref<Surface> surface) const override;
    Node *expand(Node *node) override;
    Node *expand_random(Node *node, bool from_blooddrawing);
    float simulate(Node *node) override;
    void backpropagate(Node *node, const float score) override;

    static void serialize_node(std::ofstream &to_file, Node *node);
    void serialize_tree(std::ofstream &to_file);
    void test_for_multiple_factions(Node *node);
};

#endif