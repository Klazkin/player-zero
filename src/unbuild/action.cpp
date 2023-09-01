#include "action.h"

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

bool ActionRegistry::_is_castable(std::string action_name, CastInfo cast_info)
{
    return function_registry[action_name].check_func(cast_info);
}

void ActionRegistry::_cast_action(std::string action_name, CastInfo cast_info)
{
    function_registry[action_name].cast_func(cast_info);
}

void ActionRegistry::register_combination(std::string action1, std::string action2, std::string action_result)
{
    combination_registry[CombinationKey(action1, action2)] = action_result;
}

bool ActionRegistry::has_combination(std::string action1, std::string action2)
{
    return combination_registry.count(CombinationKey(action1, action2)) > 0;
}

std::string ActionRegistry::get_combination(std::string action1, std::string action2)
{
    if (has_combination(action1, action2))
    {
        return combination_registry[CombinationKey(action1, action2)];
    }
    return "";
}