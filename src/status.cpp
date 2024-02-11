#include "status.h"
#include <godot_cpp/variant/utility_functions.hpp>

Status::Status(UnitSubscriberIdentifier p_sub_id, Unit *p_target_ptr, const int p_duration)
{
    UtilityFunctions::print("status init");
    target_ptr = p_target_ptr;
    duration = p_duration;
    sub_id = p_sub_id;
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

Shacles::Shacles(Unit *caster_ptr, Unit *p_target_ptr, const int p_duration) : Status(STATUS_SHACLES, caster_ptr, p_duration)
{
    UtilityFunctions::print("init shacles init");
    shacle_target_ptr = p_target_ptr; // TODO may cause issues if one of them is dead
}

void Shacles::on_hit(int damage)
{
    UtilityFunctions::print("shacle damage transfer");
    shacle_target_ptr->hit(damage); // damage transfer
}
