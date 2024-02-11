#include "status.h"
#include <godot_cpp/variant/utility_functions.hpp>

Status::Status(UnitSubscriberIdentifier p_sub_id, Unit *p_target_ptr, const int p_duration)
{
    UtilityFunctions::print("Status()");
    target_ptr = p_target_ptr;
    duration = p_duration;
    sub_id = p_sub_id;
}

Status::~Status()
{
    UtilityFunctions::print("~Status()");
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

void BurnStatus::on_turn_start()
{
    target_ptr->hit(5);
    Status::on_turn_start();
}

Countdown::Countdown(Unit *p_target_ptr, const int p_duration) : Status(STATUS_COUNTDOWN, p_target_ptr, p_duration) {}

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
    UtilityFunctions::print("~ShaclesParent()");
    (*link_counter)--;
    if (*link_counter <= 0)
    {
        UtilityFunctions::print("delete link_counter");
        delete link_counter;
    }
}

void ShaclesParent::on_hit(int damage)
{
    if (*link_counter == 2)
    {
        UtilityFunctions::print("shacle_target_ptr->hit(damage);");
        shacle_target_ptr->hit(damage); // damage transfer
    }
}

ShaclesChild::ShaclesChild(int *p_link_counter, Unit *p_target_ptr, const int p_duration) : Status(STATUS_SHACLES, p_target_ptr, p_duration)
{
    link_counter = p_link_counter;
}

ShaclesChild::~ShaclesChild()
{
    UtilityFunctions::print("~ShaclesChild()");
    (*link_counter)--;
    if (*link_counter <= 0)
    {
        UtilityFunctions::print("delete link_counter");
        delete link_counter;
    }
}
