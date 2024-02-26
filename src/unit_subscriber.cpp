#include "unit_subscriber.h"
#include <godot_cpp/variant/utility_functions.hpp>

UnitSubscriber::UnitSubscriber()
{
}

UnitSubscriber::~UnitSubscriber()
{
}

void UnitSubscriber::clone_to(CloneContext &clone_context) const
{
    godot::UtilityFunctions::printerr("*UnitSubscriber::clone()");
}

UnitSubscriberIdentifier UnitSubscriber::get_id() const
{
    return sub_id;
}

void UnitSubscriber::on_death()
{
}

void UnitSubscriber::on_turn_start()
{
}

void UnitSubscriber::on_hit(int damage)
{
}
