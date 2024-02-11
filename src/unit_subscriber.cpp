#include "unit_subscriber.h"
#include <godot_cpp/variant/utility_functions.hpp>

UnitSubscriber::UnitSubscriber()
{
    godot::UtilityFunctions::print("usub init");
}

UnitSubscriber::~UnitSubscriber()
{
    godot::UtilityFunctions::print("usub decons");
}

UnitSubscriberIdentifier UnitSubscriber::get_id() const
{
    return sub_id;
}

void UnitSubscriber::trigger()
{
    godot::UtilityFunctions::print("Trigged generic usub");
}