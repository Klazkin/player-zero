#ifndef ACTOR_H
#define ACTOR_H

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <handlers.h>

using namespace godot;

class ActionBundle : public RefCounted
{
    GDCLASS(ActionBundle, RefCounted);

private:
    std::vector<CastInfo> casts;
    int cast_counter = 0;

protected:
    static void _bind_methods();

public:
    ActionBundle();
    ~ActionBundle();

    bool cast_until_finished();
    std::vector<CastInfo> get_casts() const;
    void push_back_cast(const CastInfo &cast);
};

class Actor : public Object // Abstract class
{
    GDCLASS(Actor, Object);

protected:
    static void _bind_methods();

public:
    static Ref<ActionBundle> get_actions_from_decision_tree(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_random(Ref<Unit> caster, Ref<Surface> surface);
    static Ref<ActionBundle> get_actions_from_mcts(Ref<Unit> caster, Ref<Surface> surface); // Monte Carlo tree search
};

#endif