#ifndef ACTION_H
#define ACTION_H

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include <unordered_map>
#include <string>
#include <functional>

#include "surface.h"
#include "unit.h"
#include "action_identifier.h"

using namespace godot;

struct CastInfo
{
    ActionIdentifier action; // TODO try making const?
    Ref<Surface> surface;
    Ref<SurfaceElement> caster;
    Vector2i target;
};

struct CombinationKey
{
    const ActionIdentifier action1;
    const ActionIdentifier action2;

    CombinationKey(const ActionIdentifier p_action1, const ActionIdentifier p_action2);

    struct Hash
    {
        std::size_t operator()(const CombinationKey &c) const;
    };

    bool operator==(const CombinationKey &other) const;
};

using ActionCheckType = bool (*)(const CastInfo &);
using ActionCastType = void (*)(const CastInfo &);

struct ActionFunctions
{
    ActionCheckType check_func; // Todo ask Siim about const ActionCheckType &check_func pattern instead
    ActionCastType cast_func;
};

class Action : public Object // Abstract class
{
    GDCLASS(Action, Object);

private:
    static std::unordered_map<ActionIdentifier, ActionFunctions> function_registry;
    static std::unordered_map<CombinationKey, ActionIdentifier, CombinationKey::Hash> combination_registry;

protected:
    static void _bind_methods();

public:
    static bool _is_castable(const CastInfo &cast_info); // use ActionCheckType
    static void _cast_action(const CastInfo &cast_info); // use ActionCastType

    static void register_action(ActionIdentifier action, ActionCheckType check_func, ActionCastType cast_func);
    static bool is_action_registered(ActionIdentifier action);
    static void register_combination(ActionIdentifier action1, ActionIdentifier action2, ActionIdentifier action_result);

    // gd extension functions
    static bool is_castable(const ActionIdentifier action, Ref<Surface> surface, Ref<SurfaceElement> caster, const Vector2i &target);
    static void cast_action(const ActionIdentifier action, Ref<Surface> surface, Ref<SurfaceElement> caster, const Vector2i &target);
    static bool has_combination(const ActionIdentifier action1, const ActionIdentifier action2);
    static ActionIdentifier get_combination(const ActionIdentifier action1, const ActionIdentifier action2);
};

#endif
