#include "tree_action_bundle.h"
#include <godot_cpp/variant/utility_functions.hpp>

TreeActionBundle::~TreeActionBundle()
{
    delete root;
}

bool TreeActionBundle::is_finished() const // TODO simplify this check
{
    if (current_node == nullptr || caster.is_null())
    {
        if (surface->get_remaining_factions_count() < 2)
        {
            return true;
        }

        std::cout << "is_finished() check failed. " << (current_node == nullptr) << " " << caster.is_null() << " \n";
        return true;
    }

    // TODO last or check in case our action was the winning action.
    return forced_to_finish || current_node->get_action() == END_TURN || caster->is_dead() || surface->get_winner() != UNDEFINED;
}

void TreeActionBundle::cast_next()
{
    // std::cout << "TAB: cast_next()\n";
    if (is_finished())
    {
        return;
    }

    if (current_node->is_leaf())
    {
        if (current_node->visits > 0)
        {
            std::cout << "Critical: TreeActionBundle leaf node! v:" << current_node->visits << "\n";

            std::ofstream fstream;
            fstream.open(std::string("tree_7345_" + std::to_string(UtilityFunctions::randi()) + ".graph"), std::ios::app);
            draw_tree(get_root(), 8, get_root()->caster->get_faction(), fstream);
            fstream.close();
        }

        // exit(7345);
        Action::_cast_action({END_TURN, surface, caster, caster->get_position()});
        forced_to_finish = true;
        return;
    }

    Node *best_child = nullptr;
    int tot_visits = current_node->visits;

    if (is_probabilistic && tot_visits > 0)
    {
        float chosen_point = UtilityFunctions::randf();
        float range_start = 0;

        for (size_t i = 0; i < current_node->children.size(); i++)
        {
            Node *child = current_node->children[i];
            float range_end = range_start + ((float)child->visits) / tot_visits;
            if (i == current_node->children.size() - 1)
            {
                range_end = 1.0;
            }
            if (chosen_point <= range_end)
            {
                best_child = child;
                break;
            }
            range_start = range_end;
        }

        if (best_child == nullptr)
        {
            std::cout << "Probabilistic algorithm failed, (range_start = " << range_start
                      << ", chosen_point = " << chosen_point
                      << ", tot_vistis = " << tot_visits
                      << ", 0.visits = " << current_node->children[0]->visits
                      << ") falling back on deterministic\n";

            range_start = 0;
            for (auto child : current_node->children)
            {
                float range_end = range_start + (float)child->visits / tot_visits;
                std::cout << "range: " << range_start << " - " << range_end << "\n";
                range_start = range_end;
            }
        }
    }

    if (best_child == nullptr) // Deterministic, also failsafe if probabilistic couldn't find best child for whatever reason
    {
        float best_score = -std::numeric_limits<float>::infinity();
        for (auto child : current_node->children)
        {
            if (child->visits == 0)
            {
                // std::cout << "UNEXPLORED CHILD FOUND IN TAB.\n";
                continue;
            }
            float slection_score = child->visits;
            if (slection_score > best_score)
            {
                best_score = slection_score;
                best_child = child;
            }
        }
    }

    if (best_child == nullptr)
    {
        std::cout << "NON OF THE CHILDREN WERE EXPLORED, CRITICAL.\n";
        Action::_cast_action({END_TURN, surface, caster, caster->get_position()});
        forced_to_finish = true;
        return;
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
                    std::cout << "Not a proper blooddrawing node\n";
                    std::ofstream fstream;
                    fstream.open(std::string("tree_87006_" + std::to_string(UtilityFunctions::randi()) + ".graph"), std::ios::app);
                    draw_tree(get_root(), 10, get_root()->caster->get_faction(), fstream);
                    fstream.close();
                    exit(87006);
                }

                if (caster->get_hand_set() == branch->caster->get_hand_set())
                {
                    correct_branch = branch;
                }
            }
            if (correct_branch == nullptr && !is_finished())
            {
                std::cout << "CRITICAL BLOODDRAWING BRANCH MISSING AND NOT LAST TURN.\n";

                std::ofstream fstream;
                fstream.open(std::string("tree_87007_" + std::to_string(UtilityFunctions::randi()) + ".graph"), std::ios::app);
                draw_tree(get_root(), 10, get_root()->caster->get_faction(), fstream);
                fstream.close();

                Action::_cast_action({END_TURN, surface, caster, caster->get_position()});
                forced_to_finish = true;
                return;
            }
            best_child = correct_branch;
        }
    }

    current_node = best_child;
    return;
}

Node *TreeActionBundle::get_root() const
{
    return root;
}

void TreeActionBundle::save_data(const String &path, const int visits_threshold)
{
    ofstream data_file_stream;
    data_file_stream.open(std::string(path.ascii()), std::ios::app);
    PlayerZeroTreeSearch::serialize_node(data_file_stream, root, root->caster->get_faction(), true, visits_threshold);
    data_file_stream.close();
}
