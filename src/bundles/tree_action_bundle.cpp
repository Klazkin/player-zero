#include "tree_action_bundle.h"

TreeActionBundle::~TreeActionBundle()
{
    delete root;
}

bool TreeActionBundle::is_finished() const
{
    return node->get_action() == END_TURN || caster->is_dead();
}

void TreeActionBundle::cast_next()
{
    if (is_finished())
    {
        return;
    }

    if (node->is_leaf())
    {
        std::cout << "Critical: TreeActionBundle leaf node.\n";
    }

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

    CastInfo ci = {best_child->get_action(), surface, caster, best_child->get_target()};
    if (Action::_is_castable(ci))
    {
        Action::_cast_action(ci);

        if (ci.action == BLOODDRAWING) // locate the correct branch for Blooddrawing
        {
            Node *correct_branch = nullptr;

            for (auto branch : best_child->children)
            {
                if (branch->get_action() != INVALID_ACTION)
                {
                    std::cout << "Not a proper blooddrawing node";
                }

                if (caster->get_hand_set() == branch->caster->get_hand_set())
                {
                    correct_branch = branch;
                    break;
                }
            }
            if (correct_branch == nullptr)
            {
                std::cout << "BLOODDRAWING BRANCH NOT FOUND";
                return;
            }

            best_child = correct_branch;
        }
    }

    node = best_child;
    return;
}

Node *TreeActionBundle::get_root() const
{
    return root;
}
