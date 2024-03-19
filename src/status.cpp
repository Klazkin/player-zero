#include "status.h"
#include <godot_cpp/variant/utility_functions.hpp>

Status::Status(UnitSubscriberIdentifier p_sub_id, Unit *p_target_ptr, const int p_duration)
{
    target_ptr = p_target_ptr;
    duration = p_duration;
    sub_id = p_sub_id;
}

Status::~Status()
{
}

void Status::decrease_duration()
{
    if (duration > 0)
        duration--;
}

int Status::get_duration() const
{
    return duration;
}

void Status::on_turn_start()
{
    decrease_duration();
    if (get_duration() <= 0)
    {
        target_ptr->remove_subscriber(get_id());
    }
}

void Status::on_hit(int damage)
{
}

void Status::on_death()
{
}

BurnStatus::BurnStatus(Unit *p_target_ptr, const int p_duration) : Status(STATUS_BURN, p_target_ptr, p_duration) {}

void BurnStatus::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new BurnStatus(cloned_owner, duration));
}

void BurnStatus::on_turn_start()
{
    target_ptr->get_stat_modifiers().defence -= 1;
    target_ptr->hit(1);
    Status::on_turn_start();
}

Countdown::Countdown(Unit *p_target_ptr, const int p_duration) : Status(STATUS_COUNTDOWN, p_target_ptr, p_duration) {}

void Countdown::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new Countdown(cloned_owner, duration));
}

void Countdown::on_turn_start()
{
    if (get_duration() == 1)
        target_ptr->hit(15);
    Status::on_turn_start();
}

ShaclesParent::ShaclesParent(int *p_link_counter, Unit *caster_ptr, Unit *p_target_ptr, const int p_duration) : Status(STATUS_SHACLES, caster_ptr, p_duration)
{
    link_counter = p_link_counter;
    shacle_target_ptr = p_target_ptr;
}

ShaclesParent::~ShaclesParent()
{
    (*link_counter)--;
    if (*link_counter <= 0)
    {
        delete link_counter;
    }
}

void ShaclesParent::clone_to(CloneContext &clone_context) const
{
    if ((*link_counter) < 2) // link broken, dummy subscriber
    {
        // UnitSubscriber *clone = new UnitSubscriber();
        // cloned_owner->add_subscriber(clone);
        return;
    }

    // link unbroken, instanciate new shacles status on both units.
    Unit *caster_clone = as_unit_ptr(clone_context[target_ptr]);
    Unit *target_clone = as_unit_ptr(clone_context[shacle_target_ptr]);

    if (target_clone == nullptr) // Todo investigate why this check is needed
    {
        return;
    }

    int *counter_clone = new int(2);
    caster_clone->add_subscriber(new ShaclesParent(counter_clone, caster_clone, target_clone, duration));
    target_clone->add_subscriber(new ShaclesChild(counter_clone, target_clone, duration));
}

void ShaclesParent::on_hit(int damage)
{
    if (*link_counter == 2)
    {
        shacle_target_ptr->hit(damage, false); // damage transfer (Without triggering other on_hits..)
        // Above is "false" so that if two units are shacled twice in both directions we do not create an infinite loop.
    }
}

ShaclesChild::ShaclesChild(int *p_link_counter, Unit *p_target_ptr, const int p_duration) : Status(STATUS_SHACLES, p_target_ptr, p_duration)
{
    link_counter = p_link_counter;
}

ShaclesChild::~ShaclesChild()
{
    (*link_counter)--;
    if (*link_counter <= 0)
    {
        delete link_counter;
    }
}

void ShaclesChild::clone_to(CloneContext &clone_context) const
{
}

Dusted::Dusted(Unit *p_target_ptr, const int p_duration) : Status(STATUS_DUSTED, p_target_ptr, p_duration) {}

void Dusted::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new Dusted(cloned_owner, duration));
}

void Dusted::on_turn_start()
{
    StatModifiers &sm = target_ptr->get_stat_modifiers();
    sm.speed -= 2;
    Status::on_turn_start();
}

Spiriting::Spiriting(Unit *p_target_ptr, const int p_duration) : Status(STATUS_SPIRITING, p_target_ptr, p_duration) {}

void Spiriting::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new Spiriting(cloned_owner, duration));
};

void Spiriting::on_turn_start()
{
    target_ptr->heal(2);
    Status::on_turn_start();
};

Immolation::Immolation(const int p_borrowed_hp, Unit *p_target_ptr, const int p_duration) : Status(STATUS_IMMOLATION, p_target_ptr, p_duration)
{
    borrowed_hp = p_borrowed_hp;
}

void Immolation::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new Immolation(borrowed_hp, cloned_owner, duration));
};

void Immolation::on_turn_start()
{
    StatModifiers &sm = target_ptr->get_stat_modifiers();
    sm.speed += 1;
    sm.damage += 4;
    sm.defence -= 1;

    if (get_duration() == 1)
        target_ptr->heal(borrowed_hp);

    Status::on_turn_start();
};

CoreArmor::CoreArmor(Unit *p_target_ptr, const int p_duration) : Status(STATUS_CORE_ARMOR, p_target_ptr, p_duration) {}
void CoreArmor::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    auto copy = new CoreArmor(cloned_owner, duration);
    copy->is_active = is_active;
    cloned_owner->add_subscriber(copy);
};

void CoreArmor::on_turn_start()
{
    if (is_active)
    {
        StatModifiers &sm = target_ptr->get_stat_modifiers();
        sm.speed -= 1;
        sm.armored = true;
    }

    Status::on_turn_start();
};

void CoreArmor::on_hit(int damage)
{
    is_active = false;
}
bool CoreArmor::get_is_active() const
{
    return is_active;
};

HoarfrostArmor::HoarfrostArmor(Unit *p_target_ptr, const int p_duration) : Status(STATUS_HOARFROST_ARMOR, p_target_ptr, p_duration) {}
void HoarfrostArmor::clone_to(CloneContext &clone_context) const
{
    Unit *cloned_owner = as_unit_ptr(clone_context[target_ptr]);
    cloned_owner->add_subscriber(new HoarfrostArmor(cloned_owner, duration));
};

void HoarfrostArmor::on_turn_start()
{
    target_ptr->get_stat_modifiers().defence += 2;
    Status::on_turn_start();
};