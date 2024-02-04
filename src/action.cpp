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
    ClassDB::bind_static_method("Action", D_METHOD("is_castable", "action_name", "surface", "caster", "position"), &Action::is_castable);
    ClassDB::bind_static_method("Action", D_METHOD("cast_action", "action_name", "surface", "caster", "position"), &Action::cast_action);
    ClassDB::bind_static_method("Action", D_METHOD("has_combination", "action1", "action2"), &Action::has_combination);
    ClassDB::bind_static_method("Action", D_METHOD("get_combination", "action1", "action2"), &Action::get_combination);

    BIND_ENUM_CONSTANT(INVALID)
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
}

bool Action::_is_castable(const CastInfo &cast_info)
{
    if (!is_action_registered(cast_info.action))
    {
        return false;
    }

    return function_registry[cast_info.action].check_func(cast_info);
}

void Action::_cast_action(const CastInfo &cast_info)
{
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
    return INVALID;
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

bool Action::is_castable(const ActionIdentifier action, Ref<Surface> surface, Ref<Unit> caster, const Vector2i &target)
{
    return _is_castable({action, surface, caster, target});
}

void Action::cast_action(const ActionIdentifier action, Ref<Surface> surface, Ref<Unit> caster, const Vector2i &target)
{
    _cast_action({action, surface, caster, target});
}
