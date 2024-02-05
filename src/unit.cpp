#include "unit.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Unit::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_max_health"), &Unit::get_max_health);
    ClassDB::bind_method(D_METHOD("set_max_health", "p_max_health"), &Unit::set_max_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "max_health"), "set_max_health", "get_max_health");

    ClassDB::bind_method(D_METHOD("get_health"), &Unit::get_health);
    ClassDB::bind_method(D_METHOD("set_health", "p_health"), &Unit::set_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "health"), "set_health", "get_health");

    ClassDB::bind_method(D_METHOD("get_speed"), &Unit::get_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Unit::set_speed);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "speed"), "set_speed", "get_speed");

    ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "new_health")));
    ADD_SIGNAL(MethodInfo("max_health_changed", PropertyInfo(Variant::INT, "new_max_health")));
    ADD_SIGNAL(MethodInfo("speed_changed", PropertyInfo(Variant::INT, "new_speed")));
}

Unit::Unit()
{
    max_health = 20;
    health = max_health;
    speed = 1;
}

Unit::~Unit()
{
    // Add your cleanup here.
}

int Unit::hit(int damage)
{
    set_health(get_health() - damage);
    emit_signal("hurt", damage);
    return 1; // TODO return damage change
}

void Unit::set_max_health(const int p_max_health)
{
    max_health = p_max_health;
    emit_signal("max_health_changed", p_max_health);
}

int Unit::get_max_health() const
{
    return max_health;
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

void Unit::set_speed(const int p_speed)
{
    speed = p_speed;
    emit_signal("speed_changed", p_speed);
}

int Unit::get_speed() const
{
    return speed;
}

bool Unit::is_unit() const
{
    return true;
}

bool godot::Unit::is_dead() const
{
    return health <= 0;
}
