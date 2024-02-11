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

Ref<ActionBundle> Actor::get_actions_from_decision_tree(Ref<Unit> puppet, Ref<Surface> surface)
{
    Ref<ActionBundle> ab = memnew(ActionBundle);

    auto units = surface->get_only_units_vec();
    Ref<Unit> target = nullptr;

    for (auto u : units)
    {
        if (u->get_faction() == puppet->get_faction())
        {
            continue;
        }

        if ((u->get_position() - puppet->get_position()).length_squared() >= 8 * 8)
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
        UtilityFunctions::print("returning ab, nothing found near");
        return ab;
    }

    if ((target->get_position() - puppet->get_position()).length_squared() >= 4 * 4)
    {
        UtilityFunctions::print("Adding tread cast");
        PackedVector2Array path = surface->get_shortest_path(puppet->get_position(), target->get_position(), true);
        UtilityFunctions::print(path);
        Vector2i tread_target = path[path.size() - 5]; // TODO may cause issues
        ab->push_back_cast({TREAD, surface, puppet, tread_target});
    }

    if (target->has_subscriber(STATUS_DUSTED))
    {
        UtilityFunctions::print("Adding bonesparks cast");
        ab->push_back_cast({BONESPARKS, surface, puppet, target->get_position()});
    }
    else
    {
        CastInfo ci = {WISPSPARKS, surface, puppet, target->get_position()};
        if (check_line_of_sight(ci))
        {
            UtilityFunctions::print("Adding wispsparks cast");
            ab->push_back_cast(ci);
        }

        UtilityFunctions::print("Adding bonedust cast");
        ab->push_back_cast({BONEDUST, surface, puppet, target->get_position()});
    };

    return ab;
}
