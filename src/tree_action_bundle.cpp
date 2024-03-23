#include "tree_action_bundle.h"
#include <godot_cpp/variant/utility_functions.hpp>

TreeActionBundle::~TreeActionBundle()
{
    delete root;
}

bool TreeActionBundle::cast_until_finished()
{
    std::cout << "Getting casts from TAB:\n"
              << caster.is_valid() << surface.is_valid() << " " << root << " " << node << "\n";

    if (node == nullptr)
    {
        node = root;
    }

    if (node->is_leaf())
    {
        std::cout << "TreeActionBundle leaf node\n";
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
        if (ci.caster->is_dead()) // case when sudoku after casting action
        {
            cast_counter = casts.size(); // todo uneeded in tree cast
            return true;
        }

        if (ci.action == END_TURN)
        {
            return true;
        }

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
                return false;
            }

            best_child = correct_branch;
        }
    }
    else
    {
        UtilityFunctions::printerr("CRITICAL: Did not pass cast check in tree action bundle");
        for (auto cj : casts)
        {
            std::cout << '\t' << cj.action << "\n";
        }

        UtilityFunctions::printerr(ci.action);
        UtilityFunctions::printerr(ci.caster->get_position());
        UtilityFunctions::printerr(ci.target);

        if (!as_unit_ptr(ci.caster)->is_in_hand(ci.action))
        {
            UtilityFunctions::printerr("Action not found in hand.");
        }

        if (ci.surface->get_winner() != UNDEFINED)
        {
            UtilityFunctions::printerr("Surface has a winner.");
        }

        if (ci.caster->is_dead())
        {
            UtilityFunctions::printerr("Caster is dead.");
        }

        if (ci.surface->turn_get_current_unit() != ci.caster)
        {
            UtilityFunctions::printerr("Caster is not current turn unit.");
        }
        // Action::_cast_action({END_TURN, ci.surface, ci.caster, Vector2i()});
    }

    node = best_child;
    return false;
}
