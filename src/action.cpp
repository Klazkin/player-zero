#include "action.h"

using namespace godot;

CombinationKey::CombinationKey(const ActionIdentifier p_action1, const ActionIdentifier p_action2)
    : action1(p_action1), action2(p_action2)
{
}

std::size_t CombinationKey::Hash::operator()(const CombinationKey &c) const
{
    return std::hash<int>()(c.action1) ^ std::hash<int>()(c.action2);
}

bool CombinationKey::operator==(const CombinationKey &other) const
{
    return (action1 == other.action1 && action2 == other.action2) ||
           (action1 == other.action2 && action2 == other.action1);
}

std::unordered_map<ActionIdentifier, ActionFunctions> Action::function_registry;
std::unordered_map<CombinationKey, ActionIdentifier, CombinationKey::Hash> Action::combination_registry;

void Action::_bind_methods()
{
    ClassDB::bind_static_method("Action", D_METHOD("is_castable", "action", "surface", "caster", "position"), &Action::is_castable);
    ClassDB::bind_static_method("Action", D_METHOD("cast_action", "action", "surface", "caster", "position"), &Action::cast_action);
    ClassDB::bind_static_method("Action", D_METHOD("has_combination", "action1", "action2"), &Action::has_combination);
    ClassDB::bind_static_method("Action", D_METHOD("get_combination", "action1", "action2"), &Action::get_combination);
    ClassDB::bind_static_method("Action", D_METHOD("combine", "caster", "action1", "action2"), &Action::combine);

    BIND_ENUM_CONSTANT(INVALID_ACTION)
    BIND_ENUM_CONSTANT(IDLE)
    BIND_ENUM_CONSTANT(WRATHSPARK);
    BIND_ENUM_CONSTANT(GROUNDRAISE);
    BIND_ENUM_CONSTANT(BLOODDRAWING);
    BIND_ENUM_CONSTANT(TREAD);
    BIND_ENUM_CONSTANT(COILBLADE);
    BIND_ENUM_CONSTANT(ETERNALSHACLES);
    BIND_ENUM_CONSTANT(ALTAR);
    BIND_ENUM_CONSTANT(NETHERSWAP);
    BIND_ENUM_CONSTANT(LOS_ACTION);
    BIND_ENUM_CONSTANT(DETONATION);
    BIND_ENUM_CONSTANT(DEBUG_KILL);
    BIND_ENUM_CONSTANT(WISPSPARKS);
    BIND_ENUM_CONSTANT(BONEDUST);
    BIND_ENUM_CONSTANT(BONESPARKS);
}

bool Action::_is_castable(const CastInfo &cast_info)
{
    if (!is_action_registered(cast_info.action))
    {
        return false;
    }

    if (cast_info.surface->turn_get_current_unit() != cast_info.caster)
    {
        return false;
    }

    if (cast_info.caster->is_unit() && !as_unit_ptr(cast_info.caster)->is_in_hand(cast_info.action))
    {
        return false;
    }

    return function_registry[cast_info.action].check_func(cast_info);
}

void Action::_cast_action(const CastInfo &cast_info)
{
    if (cast_info.caster->is_unit())
    {
        as_unit_ptr(cast_info.caster)->remove_from_hand(cast_info.action);
    }

    cast_info.surface->emit_action_cast(cast_info.action, cast_info.caster, cast_info.target);
    function_registry[cast_info.action].cast_func(cast_info);
}

bool Action::has_combination(const ActionIdentifier action1, const ActionIdentifier action2)
{
    return combination_registry.count(CombinationKey(action1, action2)) > 0;
}

ActionIdentifier Action::get_combination(const ActionIdentifier action1, const ActionIdentifier action2)
{
    if (has_combination(action1, action2))
    {
        return combination_registry[CombinationKey(action1, action2)];
    }
    return INVALID_ACTION;
}

void Action::combine(const Ref<Unit> caster, const ActionIdentifier action1, const ActionIdentifier action2)
{
    ActionIdentifier result = get_combination(action1, action2);
    if (result == INVALID_ACTION)
    {
        return;
    }

    caster->remove_from_hand(action1);
    caster->remove_from_hand(action2);
    caster->add_to_hand(result);
}

void Action::register_action(ActionIdentifier action, ActionCheckType check_func, ActionCastType cast_func)
{
    function_registry[action] = {check_func, cast_func};
}

bool Action::is_action_registered(ActionIdentifier action)
{
    return function_registry.count(action) > 0;
}

void Action::register_combination(ActionIdentifier action1, ActionIdentifier action2, ActionIdentifier action_result)
{
    combination_registry[CombinationKey(action1, action2)] = action_result;
}

bool Action::is_castable(const ActionIdentifier action, Ref<Surface> surface, Ref<SurfaceElement> caster, const Vector2i &target)
{
    return _is_castable({action, surface, caster, target});
}

void Action::cast_action(const ActionIdentifier action, Ref<Surface> surface, Ref<SurfaceElement> caster, const Vector2i &target)
{
    _cast_action({action, surface, caster, target});
}
