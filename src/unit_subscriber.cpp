#include "unit_subscriber.h"
#include <godot_cpp/variant/utility_functions.hpp>

UnitSubscriber::UnitSubscriber()
{
    godot::UtilityFunctions::print("UnitSubscriber()");
}

UnitSubscriber::~UnitSubscriber()
{
    godot::UtilityFunctions::print("~UnitSubscriber()");
}

UnitSubscriberIdentifier UnitSubscriber::get_id() const
{
    return sub_id;
}

void UnitSubscriber::on_death()
{
    godot::UtilityFunctions::print("UnitSubscriber::on_death()");
}

void UnitSubscriber::on_turn_start()
{
    godot::UtilityFunctions::print("UnitSubscriber::on_turn_start()");
}

void UnitSubscriber::on_hit(int damage)
{
    godot::UtilityFunctions::print("UnitSubscriber::on_hit()");
}
