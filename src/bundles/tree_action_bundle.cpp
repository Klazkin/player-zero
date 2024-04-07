#include "tree_action_bundle.h"

TreeActionBundle::~TreeActionBundle()
{
    delete root;
}

bool TreeActionBundle::is_finished() const
{
    return current_node->get_action() == END_TURN || caster->is_dead() || surface->get_winner() != UNDEFINED || forced_to_finish;
    // TODO last or check in case our action was the winning action.
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
        std::cout << "Critical: TreeActionBundle leaf node.\n";
        draw_tree(get_root(), 6, get_root()->caster->get_faction());
        exit(7345);
    }

    float best_score = -std::numeric_limits<float>::infinity();
    Node *best_child = nullptr;
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
                    draw_tree(get_root(), 10, get_root()->caster->get_faction());
                    exit(87006);
                }

                if (caster->get_hand_set() == branch->caster->get_hand_set())
                {
                    correct_branch = branch;
                }
            }
            if (correct_branch == nullptr)
            {
                std::cout << "BLOODDRAWING BRANCH NOT FOUND\n";
                if (is_finished())
                {
                    std::cout << "...But it does not matter, the case is finished.";
                }
                else
                {
                    std::cout << "CRITICAL BLOODDRAWING AND NOT LAST TURN.\n";
                }
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

void TreeActionBundle::save_data(const String &path)
{
    ofstream data_file_stream;
    data_file_stream.open(std::string(path.ascii()), std::ios::app);
    PlayerZeroTreeSearch::serialize_node(data_file_stream, root); // TODO Currently does not serailize the whole tree, ONLY THE ROOT.
    data_file_stream.close();
}
