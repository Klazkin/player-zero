#include "sunit.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void SUnit::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_max_health"), &SUnit::get_max_health);
    ClassDB::bind_method(D_METHOD("set_max_health", "p_max_health"), &SUnit::set_max_health);
    ClassDB::add_property("SUnit", PropertyInfo(Variant::INT, "max_health"), "set_max_health", "get_max_health");

    ClassDB::bind_method(D_METHOD("get_health"), &SUnit::get_health);
    ClassDB::bind_method(D_METHOD("set_health", "p_health"), &SUnit::set_health);
    ClassDB::add_property("SUnit", PropertyInfo(Variant::INT, "health"), "set_health", "get_health");

    ClassDB::bind_method(D_METHOD("get_speed"), &SUnit::get_speed);
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &SUnit::set_speed);
    ClassDB::add_property("SUnit", PropertyInfo(Variant::INT, "speed"), "set_speed", "get_speed");
}

SUnit::SUnit()
{
    max_health = 20;
    health = health;
    speed = 1;
}

SUnit::~SUnit()
{
    // Add your cleanup here.
}

void SUnit::set_max_health(const int p_max_health)
{
    max_health = p_max_health;
}

int SUnit::get_max_health() const
{
    return max_health;
}

void SUnit::set_health(const int p_health)
{
    health = p_health;
}

int SUnit::get_health() const
{
    return health;
}

void SUnit::set_speed(const int p_speed)
{
    speed = p_speed;
}

int SUnit::get_speed() const
{
    return speed;
}