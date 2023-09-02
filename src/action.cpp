#include "action.h"
using namespace godot;

CombinationKey::CombinationKey(const std::string &pa1, const std::string &pa2)
    : action1(pa1), action2(pa2)
{
}

std::size_t CombinationKey::Hash::operator()(const CombinationKey &c) const
{
    return std::hash<std::string>()(c.action1) ^ (std::hash<std::string>()(c.action2));
}

bool CombinationKey::operator==(const CombinationKey &other) const
{
    return (action1 == other.action1 && action2 == other.action2) ||
           (action1 == other.action2 && action2 == other.action1);
}

std::unordered_map<std::string, ActionFunctions> ActionRegistry::function_registry;
std::unordered_map<CombinationKey, std::string, CombinationKey::Hash> ActionRegistry::combination_registry;

void godot::ActionRegistry::_bind_methods()
{
    ClassDB::bind_static_method("ActionRegistry", D_METHOD("is_castable", "action_name", "surface", "caster", "position"), &ActionRegistry::is_castable);
    ClassDB::bind_static_method("ActionRegistry", D_METHOD("cast_action", "action_name", "surface", "caster", "position"), &ActionRegistry::cast_action);
    ClassDB::bind_static_method("ActionRegistry", D_METHOD("has_combination", "action1", "action2"), &ActionRegistry::has_combination);
    ClassDB::bind_static_method("ActionRegistry", D_METHOD("get_combination", "action1", "action2"), &ActionRegistry::get_combination);
}

void ActionRegistry::register_action(std::string action_name, ActionCheckType check_func, ActionCastType cast_func)
{
    ActionFunctions af;
    af.cast_func = cast_func;
    af.check_func = check_func;
    function_registry[action_name] = af;
}

bool ActionRegistry::is_action_registered(std::string action_name)
{
    return function_registry.count(action_name) > 0;
}

bool ActionRegistry::_is_castable(CastInfo cast_info)
{
    return function_registry[cast_info.action_name].check_func(cast_info);
}

void ActionRegistry::_cast_action(CastInfo cast_info)
{
    function_registry[cast_info.action_name].cast_func(cast_info);
}

void ActionRegistry::register_combination(std::string action1, std::string action2, std::string action_result)
{
    combination_registry[CombinationKey(action1, action2)] = action_result;
}

bool godot::ActionRegistry::is_castable(const String &action_name, Surface *surface, Unit *caster, const Vector2i &position)
{
    return false;
}

bool godot::ActionRegistry::cast_action(const String &action_name, Surface *surface, Unit *caster, const Vector2i &position)
{
    return false;
}

bool godot::ActionRegistry::has_combination(const String &action1, const String &action2)
{
    return false;
}

String godot::ActionRegistry::get_combination(const String &action1, const String &action2)
{
    return String();
}

bool ActionRegistry::_has_combination(std::string action1, std::string action2)
{
    return combination_registry.count(CombinationKey(action1, action2)) > 0;
}

std::string ActionRegistry::_get_combination(std::string action1, std::string action2)
{
    if (_has_combination(action1, action2))
    {
        return combination_registry[CombinationKey(action1, action2)];
    }
    return "";
}
