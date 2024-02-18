#include "unit.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Unit::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_base_max_health"), &Unit::get_base_max_health);
    ClassDB::bind_method(D_METHOD("set_base_max_health", "p_max_health"), &Unit::set_base_max_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "base_max_health"), "set_base_max_health", "get_base_max_health");
    ClassDB::bind_method(D_METHOD("get_max_health"), &Unit::get_max_health);

    ClassDB::bind_method(D_METHOD("get_health"), &Unit::get_health);
    ClassDB::bind_method(D_METHOD("set_health", "p_health"), &Unit::set_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "health"), "set_health", "get_health");

    ClassDB::bind_method(D_METHOD("get_base_speed"), &Unit::get_base_speed);
    ClassDB::bind_method(D_METHOD("set_base_speed", "p_speed"), &Unit::set_base_speed);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "base_speed"), "set_base_speed", "get_base_speed");
    ClassDB::bind_method(D_METHOD("get_speed"), &Unit::get_speed);

    ClassDB::bind_method(D_METHOD("get_faction"), &Unit::get_faction);
    ClassDB::bind_method(D_METHOD("set_faction", "p_faction"), &Unit::set_faction);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "faction"), "set_faction", "get_faction");

    ClassDB::bind_method(D_METHOD("get_subscriber_ids"), &Unit::get_subscriber_ids);
    ClassDB::bind_method(D_METHOD("is_dead"), &Unit::is_dead);

    ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "new_health")));
    ADD_SIGNAL(MethodInfo("max_health_changed", PropertyInfo(Variant::INT, "new_max_health")));
    ADD_SIGNAL(MethodInfo("speed_changed", PropertyInfo(Variant::INT, "new_speed"))); // TODO add signals for stat modifiers
    ADD_SIGNAL(MethodInfo("subscriber_applied", PropertyInfo(Variant::INT, "subscriber")));
    ADD_SIGNAL(MethodInfo("subscriber_removed", PropertyInfo(Variant::INT, "subscriber")));

    BIND_ENUM_CONSTANT(UNDEFINED);
    BIND_ENUM_CONSTANT(PLAYER);
    BIND_ENUM_CONSTANT(MONSTER);
}

Unit::Unit()
{
    std::unordered_map<UnitSubscriberIdentifier, UnitSubscriber> subscribers;
    std::unordered_set<ActionIdentifier> action_pool;
    std::vector<ActionIdentifier> action_hand;
    base_max_health = 20;
    health = base_max_health;
    base_speed = 1;
}

Unit::~Unit()
{
    UtilityFunctions::print("~Unit()");
    for (auto key_value_pair : subscribers)
    {
        delete key_value_pair.second;
    }
}

int Unit::hit(int damage)
{
    set_health(get_health() - damage);
    trigger_on_hit_subscribers(damage); // todo here it should be final applied damage also
    emit_signal("hurt", damage);
    if (is_dead())
    {
        trigger_death();
    }
    return 1; // TODO return damage change
}

void Unit::set_base_max_health(const int p_max_health)
{
    base_max_health = p_max_health;
    emit_signal("max_health_changed", p_max_health);
}

int Unit::get_base_max_health() const
{
    return base_max_health;
}

int Unit::get_max_health() const
{
    return base_max_health + stat_modifiers.max_health;
}

void Unit::set_health(const int p_health)
{
    health = p_health;
    emit_signal("health_changed", p_health);
}

int Unit::get_health() const
{
    return health;
}

void Unit::set_base_speed(const int p_speed)
{
    base_speed = p_speed;
    emit_signal("speed_changed", p_speed);
}

int Unit::get_base_speed() const
{
    return base_speed;
}

int Unit::get_speed() const
{
    return base_speed + stat_modifiers.speed;
}

void godot::Unit::set_faction(const Faction p_faction)
{
    faction = p_faction;
}

Faction godot::Unit::get_faction() const
{
    return faction;
}

bool Unit::is_unit() const
{
    return true;
}

bool Unit::is_dead() const
{
    return health <= 0;
}

void Unit::add_subscriber(UnitSubscriber *subscriber)
{
    if (has_subscriber(subscriber->get_id()))
    {
        remove_subscriber(subscriber->get_id());
    }

    subscribers[subscriber->get_id()] = subscriber;
    emit_signal("subscriber_applied", subscriber->get_id());
}

bool Unit::has_subscriber(UnitSubscriberIdentifier id) const
{
    return subscribers.count(id) > 0;
}

void Unit::remove_subscriber(UnitSubscriberIdentifier id)
{
    delete subscribers[id];
    subscribers.erase(id);
    emit_signal("subscriber_removed", id);
}

void Unit::trigger_on_start_turn_subscribers()
{
    for (auto key_value_pair : subscribers)
    {
        key_value_pair.second->on_turn_start();
    }
}

void Unit::trigger_on_hit_subscribers(int damage)
{
    for (auto key_value_pair : subscribers)
    {
        key_value_pair.second->on_hit(damage);
    }
}

void godot::Unit::reset_stat_modifiers()
{
    stat_modifiers.speed = 0;
    stat_modifiers.max_health = 0;
}

StatModifiers &Unit::get_stat_modifiers()
{
    return stat_modifiers;
}

TypedArray<int> Unit::get_subscriber_ids() const
{
    TypedArray<int> arr;
    for (auto key_value_pair : subscribers)
    {
        arr.append(key_value_pair.second->get_id());
    }

    return arr;
}
