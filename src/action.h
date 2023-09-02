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

struct CastInfo
{
    std::string action_name;
    godot::Surface surface;
    godot::Unit caster;
    godot::Vector2i position;
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

using ActionCheckType = bool (*)(CastInfo);
using ActionCastType = void (*)(CastInfo);

struct ActionFunctions
{
    ActionCheckType check_func;
    ActionCastType cast_func;
};

namespace godot
{
    class ActionRegistry : public Object
    {
        GDCLASS(ActionRegistry, Object)

        private:
        static std::unordered_map<std::string, ActionFunctions> function_registry;
        static std::unordered_map<CombinationKey, std::string, CombinationKey::Hash> combination_registry;

        static bool _is_castable(CastInfo cast_info); // use ActionCheckType
        static void _cast_action(CastInfo cast_info); // use ActionCastType
        static bool _has_combination(std::string action1, std::string action2);
        static std::string _get_combination(std::string action1, std::string action2);

    protected:
        static void _bind_methods();

    public:
        static void register_action(std::string action_name, ActionCheckType check_func, ActionCastType cast_func);
        static bool is_action_registered(std::string action_name);
        static void register_combination(std::string action1, std::string action2, std::string action_result);

        // front-end functions
        static bool is_castable(const String &action_name, Surface *surface, Unit *caster, const Vector2i &position);
        static bool cast_action(const String &action_name, Surface *surface, Unit *caster, const Vector2i &position);
        static bool has_combination(const String &action1, const String &action2);
        static String get_combination(const String &action1, const String &action2);
    };
}

#endif
