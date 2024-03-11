#ifndef ACTOR_H
#define ACTOR_H

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <action_bundle.h>

using namespace godot;

class Actor : public Object // Abstract class
{
    GDCLASS(Actor, Object);

protected:
    static void _bind_methods();

public:
    static void perfrom_random_actions_for_turn(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_decision_tree(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface, int interations, int max_rollout_turns, const String &data_path);
};

#endif