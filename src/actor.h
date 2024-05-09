#ifndef ACTOR_H
#define ACTOR_H

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "bundles/action_bundle.h"
#include "ortbinding.h"

using namespace godot;

class Actor : public Object // Abstract class
{
    GDCLASS(Actor, Object);

protected:
    static void _bind_methods();

public:
    static void append_winner_to_file(const String &path, const Faction winner);
    static void write_winner_at_the_end_of_file(const String &path, const Faction winner);
    static void perfrom_random_actions_for_turn(Ref<Unit> caster, Ref<Surface> surface);
    static void reload_pzts_model();
    static Ref<ActionBundle> get_actions_from_decision_tree(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface, int iterations, int max_rollout_turns);
    static Ref<ActionBundle> get_actions_from_model(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_wpts(Ref<Unit> caster, Ref<Surface> surface, int iterations, int max_rollout_turns);
    static Ref<ActionBundle> get_actions_from_pzts(Ref<Unit> caster, Ref<Surface> surface, int iterations, const String &model_file);
};

#endif