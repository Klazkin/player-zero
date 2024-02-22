#include "actor.h"
#include "action.h"

#include <algorithm>
#include <godot_cpp/variant/utility_functions.hpp>

void ActionBundle::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("cast_until_finished"), &ActionBundle::cast_until_finished);
}

ActionBundle::ActionBundle()
{
    std::vector<CastInfo> casts;
    UtilityFunctions::print("ActionBundle()");
}

ActionBundle::~ActionBundle()
{
    UtilityFunctions::print("~ActionBundle()");
}

bool ActionBundle::cast_until_finished()
{
    if (cast_counter >= casts.size())
    {
        return true;
    }

    CastInfo ci = casts.at(cast_counter++);

    if (Action::_is_castable(ci))
    {
        Action::_cast_action(ci);
    }
    else
    {
        UtilityFunctions::print("Did not pass cast check");
    }

    return cast_counter >= casts.size();
}

std::vector<CastInfo> ActionBundle::get_casts() const
{
    return casts;
}

void ActionBundle::push_back_cast(const CastInfo &cast)
{
    casts.push_back(cast);
}

void Actor::_bind_methods()
{
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_decision_tree", "caster", "surface"), &Actor::get_actions_from_decision_tree);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_random", "caster", "surface"), &Actor::get_actions_from_random);
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_mcts", "caster", "surface"), &Actor::get_actions_from_mcts);
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

    return ab;
}

Ref<ActionBundle> Actor::get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);
    std::vector<CastInfo> casts;

    /// determine random casting order

    for (auto action : caster->get_hand_set())
    {
        std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface, caster, Vector2i(0, 0)});
        casts.insert(casts.end(), action_casts.begin(), action_casts.end());
    }

    if (casts.size() == 0)
    {
        return ab;
    }

    // select a random one.
    CastInfo chosen_random = casts[std::rand() / ((RAND_MAX + 1u) / (casts.size()))];
    if (Action::_is_castable(chosen_random))
    {
        ab->push_back_cast(chosen_random);
    }
    else
    {
        UtilityFunctions::printerr("UNCASTABLE ACTION GENERATED IN RANDOM ACTOR");
        UtilityFunctions::printerr(chosen_random.action);
        UtilityFunctions::printerr(chosen_random.target);
    }

    return ab;
}

const int MCTS_SIMS_PER_PLAY = 3000;

Ref<ActionBundle> Actor::get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface)
{

    Ref<ActionBundle> ab = memnew(ActionBundle);

    for (auto action : caster->get_hand_set())
    {
    }

    return ab;
}
