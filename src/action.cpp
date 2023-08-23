#include "action.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

void Action::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_name"), &Action::get_name);
    ClassDB::bind_method(D_METHOD("set_name", "p_name"), &Action::set_name);
    ClassDB::add_property("Action", PropertyInfo(Variant::STRING, "max_name"), "set_name", "get_name");
}

Action::Action()
{
    name = String("Hello World");
}

Action::~Action()
{
    // Add your cleanup here.
}

void Action::set_name(const String p_name)
{
    name = p_name;
}

String Action::get_name() const
{
    return name;
}
