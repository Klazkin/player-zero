#include "monte_carlo.h"
#include "status.h"

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

float ucb(Node *node, Faction root_faction)
{
    if (node->visits == 0)
    {
        return std::numeric_limits<float>::infinity();
    }

    if (node->parent == nullptr)
    {
        return 0.5;
    }

    // float score_multiplier = node->caster->get_faction() == root_faction ? 1.0 : -1.0;
    float exploit = node->score / node->visits;
    float explore = std::sqrt(2 * std::log(node->parent->visits) / node->visits);
    if (node->caster->get_faction() != root_faction)
    {
        exploit = 1 - exploit;
    }

    return exploit + explore;
}

MonteCarloTreeSearch::~MonteCarloTreeSearch()
{
}

float MonteCarloTreeSearch::calculate_surface_score(Ref<Surface> surface) const
{
    if (surface->get_winner() == root->caster->get_faction())
    {
        return 1;
    }
    if (surface->get_winner() != UNDEFINED)
    {
        return 0;
    }

    return 0.5;
}

Node *MonteCarloTreeSearch::select(Node *node) const
{
    Node *selected = node;
    while (!selected->is_leaf())
    {
        float best_selected = -std::numeric_limits<float>::infinity();
        for (auto child : selected->children)
        {
            float child_selection_score = selection_func(child, root->caster->get_faction());
            if (child_selection_score > best_selected)
            {
                best_selected = child_selection_score;
                selected = child;
            }
        }
    }
    return selected;
}

Node *MonteCarloTreeSearch::expand(Node *node)
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

float MonteCarloTreeSearch::simulate(Node *node)
{
    if (node == nullptr)
    {
        return 0.5;
    }

    if (node->caster == nullptr) // caster is nullptr if parent turn_get_current_unit is nullptr, i.e. no units left on surface
    {
        return 0.5;
    }

    Ref<Surface> rollout_surface = node->surface->clone();

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

        performer_func(current_unit, rollout_surface);
    }

    return calculate_surface_score(rollout_surface);
}

void MonteCarloTreeSearch::backpropagate(Node *node, const float score)
{
    while (node != nullptr)
    {
        node->score += score;
        node->visits++;
        node = node->parent;
    }
}

void MonteCarloTreeSearch::run(const int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        Node *selectedNode = select(root);
        Node *expandedNode = expand(selectedNode);
        float score = simulate(expandedNode);
        backpropagate(expandedNode, score);
    }
}

const auto action_pool = {
    COMBINE_ACTIONS,
    WRATHSPARK,
    GROUNDRAISE,
    // BLOODDRAWING,
    TREAD,
    COILBLADE,
    ETERNALSHACLES,
    // ALTAR,
    NETHERSWAP,
    WISPSPARKS,
    BONEDUST,
    BONESPARKS,
    RESPIRIT,
    // SNOWMOTES,
    SUNDIVE,
    METEORSHATTER,
    ARMORCORE,
    IMMOLATION,
    // ICEPOLE,
    // OBLIVION,
    HOARFROST,
    RAPID_GROWTH,
}; // 22

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

void serialize_unit(std::vector<float> &vec, Ref<Unit> unit)
{
    vec.push_back(unit->get_health());
    vec.push_back(unit->get_max_health());
    vec.push_back(unit->get_speed());

    for (ActionIdentifier action : action_pool)
        vec.push_back(unit->is_in_deck(action) ? 1 : 0);

    for (ActionIdentifier action : action_pool)
        vec.push_back(unit->is_in_hand(action) ? 1 : 0);

    for (UnitSubscriberIdentifier status : status_pool)
    {
        if (status == STATUS_CORE_ARMOR && unit->has_subscriber(STATUS_CORE_ARMOR))
        {
            auto ca_status = (CoreArmor *)unit->get_subscriber(STATUS_CORE_ARMOR);
            if (!ca_status->get_is_active())
            {
                vec.push_back(0);
                continue;
            }
        }

        vec.push_back(unit->get_subscriber_duration(status));
    }
}

std::vector<float> serialize_cast(CastInfo cast)
{
    std::vector<float> ret;

    Vector2i cast_offset = cast.action == END_TURN ? Vector2i(0, 0) : (cast.target - cast.caster->get_position());
    Ref<Unit> opponent = nullptr;

    for (auto u : cast.surface->get_only_units_vec())
    {
        if (u == cast.caster)
        {
            continue;
        }

        if (opponent == nullptr)
        {
            opponent = u;
            continue;
        }

        std::cout << "SC: More than one opponennt during serialization.\n";
    }

    if (opponent == nullptr)
    {
        std::cout << "SC: Zero opponents in serialization.\n";
        return ret;
    }

    Vector2i opponent_offset = (opponent->get_position() - cast.caster->get_position());
    ret.push_back(cast.action == END_TURN);

    for (ActionIdentifier action : action_pool)
        ret.push_back(cast.action == action);

    ret.push_back(cast_offset.x);
    ret.push_back(cast_offset.y);
    serialize_unit(ret, cast.caster);
    ret.push_back(opponent_offset.x);
    ret.push_back(opponent_offset.y);
    serialize_unit(ret, opponent);
    return ret;
}

std::vector<float> serialize_state(Ref<Surface> surface)
{
    std::vector<float> ret;
    Ref<Unit> player = nullptr;
    Ref<Unit> monster = nullptr;

    for (auto u : surface->get_only_units_vec())
    {
        if (u->get_faction() == PLAYER)
        {
            player = u;
        }

        if (u->get_faction() == MONSTER)
        {
            monster = u;
        }
    }

    if (player == nullptr || monster == nullptr)
    {
        std::cout << "unable to serialize state, one of the factions is missing\n";
        return ret;
    }

    ret.push_back(player->get_position().x);
    ret.push_back(player->get_position().y);
    serialize_unit(ret, player);

    ret.push_back(monster->get_position().x);
    ret.push_back(monster->get_position().y);
    serialize_unit(ret, monster);
    return ret;
}

void serialize_node_to_stream(Node *node, std::ofstream &to_file, float score)
{

    Vector2i cast_offset = (node->target - node->caster->get_position());
    // float score_multiplier = (node->caster->get_faction() == root->caster->get_faction()) ? 1.0 : -1.0; // invert score for enemy

    Ref<Unit> parent_node_caster = nullptr;
    for (auto u : node->parent->surface->get_only_units_vec()) // todo replace this with a less bad workaround
    {
        if (u->get_faction() == node->caster->get_faction())
        {
            parent_node_caster = u;
            break;
        }
    }

    if (parent_node_caster == nullptr)
    {
        std::cout << "parent_node_caster caster not found???\n";
        return;
    }

    CastInfo cast_with_parent_state = {
        node->action,
        node->parent->surface,
        parent_node_caster,
        node->target,
    };
    auto data_vector = serialize_cast(cast_with_parent_state);

    if (data_vector.size() != 114)
    {
        std::cout << "Serialize Node: vector wrong size.\n";
        return;
    }

    for (auto x : data_vector)
    {
        to_file << x << ',';
    }

    to_file
        << node->visits << ','
        << score << '\n';
}

void serialize_tree_to_stream(const Node *node, std::ofstream &to_file, const int visits_threshold)
{
    if (node->visits <= visits_threshold)
    {
        return;
    }

    int max_visits = -1;

    for (auto n : node->children)
    {
        max_visits = std::max(n->visits, max_visits);
    }

    for (auto n : node->children)
    {

        serialize_node_to_stream(n, to_file, ((float)n->visits / (float)max_visits));
        serialize_tree_to_stream(n, to_file, visits_threshold);
    }
}

void draw_tree(Node *node, int depth, Faction root_faction)
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
            draw_tree(child, depth - 1, root_faction);
            is_first = false;
        }
        std::cout << ")";
    }

    std::cout << node->action << "/"
              << node->target.x << "/"
              << node->target.y << "/"
              << node->visits << "/"
              << node->score << "/"
              << ucb(node, root_faction) << "/"
              << (node->caster == nullptr
                      ? UNDEFINED
                      : node->caster->get_faction());
    if (node->parent == nullptr) // is root.
    {
        std::cout << ";\n";
    }
}

static WinPredictor *model = nullptr;

float WinPredictorTreeSearch::calculate_surface_score(Ref<Surface> surface) const
{
    if (surface->get_remaining_factions_count() <= 1)
    {
        return MonteCarloTreeSearch::calculate_surface_score(surface);
    }

    std::vector<float> surface_state_vec = serialize_state(surface);
    std::array<float, 96> arr;
    std::copy_n(surface_state_vec.begin(), 96, arr.begin());

    // TODO remove the jank below
    if (model == nullptr)
    {
        std::cout << "NULL MODEL";
        model = new WinPredictor();
        // return 0;
    }

    if (model == nullptr)
    {
        std::cout << "Still bad";
        return 0;
    }

    std::array<float, 2> result = model->predict(arr);
    float player_winrate = 0.5 * (result[0] - result[1] + 1.0);

    if (root->caster->get_faction() != PLAYER)
    {
        return 1.0 - player_winrate;
    }
    return player_winrate;
}
