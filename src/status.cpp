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

void Status::trigger()
{
    decrease_duration();
    if (get_duration() <= 0)
    {
        target_ptr->remove_subscriber(get_id());
    }
}

BurnStatus::BurnStatus(Unit *p_target_ptr, const int p_duration) : Status(STATUS_BURN, p_target_ptr, p_duration) {}

void BurnStatus::trigger()
{
    target_ptr->hit(5);
    Status::trigger();
}