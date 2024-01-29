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

std::unordered_map<std::string, ActionFunctions> Action::function_registry;
std::unordered_map<CombinationKey, std::string, CombinationKey::Hash> Action::combination_registry;

void Action::_bind_methods()
{
    ClassDB::bind_static_method("Action", D_METHOD("is_castable", "action_name", "surface", "caster", "position"), &Action::is_castable);
    ClassDB::bind_static_method("Action", D_METHOD("cast_action", "action_name", "surface", "caster", "position"), &Action::cast_action);
    ClassDB::bind_static_method("Action", D_METHOD("has_combination", "action1", "action2"), &Action::has_combination);
    ClassDB::bind_static_method("Action", D_METHOD("get_combination", "action1", "action2"), &Action::get_combination);

    BIND_ENUM_CONSTANT(IDLE)
    BIND_ENUM_CONSTANT(WRATHSPARK);
    BIND_ENUM_CONSTANT(GROUNDRAISE);
    BIND_ENUM_CONSTANT(BLOODDRAWING);
    BIND_ENUM_CONSTANT(TREAD);
    BIND_ENUM_CONSTANT(COILBLADE);
    BIND_ENUM_CONSTANT(ETERNALSHACLES);
    BIND_ENUM_CONSTANT(ALTAR);
    BIND_ENUM_CONSTANT(NETHERSWAP);
}

void Action::register_action(std::string action_name, ActionCheckType check_func, ActionCastType cast_func)
{
    ActionFunctions af;
    af.check_func = check_func;
    af.cast_func = cast_func;
    function_registry[action_name] = af;
}

bool Action::is_action_registered(std::string action_name)
{
    return function_registry.count(action_name) > 0;
}

bool Action::_is_castable(CastInfo cast_info)
{
    // todo search action registered
    return function_registry[cast_info.action_name].check_func(cast_info);
}

void Action::_cast_action(CastInfo cast_info)
{
    function_registry[cast_info.action_name].cast_func(cast_info);
}

void Action::register_combination(std::string action1, std::string action2, std::string action_result)
{
    combination_registry[CombinationKey(action1, action2)] = action_result;
}

bool Action::is_castable(const String &action_name, Ref<Surface> surface, Ref<Unit> caster, const Vector2i &position)
{
    CastInfo cast;
    cast.action_name = "wrathspark"; // unimplemented
    cast.surface = surface;
    cast.caster = caster;
    cast.position = position;

    return _is_castable(cast);
}

bool Action::cast_action(const String &action_name, Ref<Surface> surface, Ref<Unit> caster, const Vector2i &position)
{
    CastInfo cast;
    cast.action_name = "wrathspark"; // unimplemented
    cast.surface = surface;
    cast.caster = caster;
    cast.position = position;

    _cast_action(cast);
    return false; // Todo determine if it should be a void
}

bool Action::has_combination(const String &action1, const String &action2)
{
    return false;
}

String Action::get_combination(const String &action1, const String &action2)
{
    return String(); // Todo implement
}

bool Action::_has_combination(std::string action1, std::string action2)
{
    return combination_registry.count(CombinationKey(action1, action2)) > 0;
}

std::string Action::_get_combination(std::string action1, std::string action2)
{
    if (_has_combination(action1, action2))
    {
        return combination_registry[CombinationKey(action1, action2)];
    }
    return "";
}
