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
    ClassDB::bind_static_method("Actor", D_METHOD("get_actions_from_decision_tree", "puppet", "surface"), &Actor::get_actions_from_decision_tree);
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
        Action::combine(caster, BONEDUST, WISPSPARKS);
        UtilityFunctions::print("DTA: Adding bonesparks cast");
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
