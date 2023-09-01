#ifndef ACTION_H
#define ACTION_H

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <unordered_map>
#include <string>
#include <functional>

#include "surface.h"
#include "unit.h"

using ActionCheckType = bool (*)(CastInfo);
using ActionCastType = void (*)(CastInfo);

struct CastInfo
{
    std::string action_name;
    godot::Surface surface;
    godot::Unit caster;
    godot::Vector2i position;
};

struct ActionFunctions
{
    ActionCheckType check_func;
    ActionCastType cast_func;
};

struct CombinationKey
{

    std::string action1;
    std::string action2;

    CombinationKey(const std::string &pa1, const std::string &pa2);

    struct Hash
    {
        std::size_t operator()(const CombinationKey &c) const;
    };

    bool operator==(const CombinationKey &other) const;
};

class ActionRegistry
{
private:
    static std::unordered_map<std::string, ActionFunctions> function_registry;
    static std::unordered_map<CombinationKey, std::string, CombinationKey::Hash> combination_registry;

public:
    static void register_action(std::string action_name, ActionCheckType check_func, ActionCastType cast_func);
    static bool is_action_registered(std::string action_name);

    static bool _is_castable(std::string action_name, CastInfo cast_info); // use ActionCheckType
    static void _cast_action(std::string action_name, CastInfo cast_info); // use ActionCastType

    static void register_combination(std::string action1, std::string action2, std::string action_result);
    static bool has_combination(std::string action1, std::string action2);
    static std::string get_combination(std::string action1, std::string action2);
};

#endif