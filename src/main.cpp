#include "main.h"
#include <iostream>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include "unit.h"
#include "register_types.h"

using namespace godot;

int main(int argc, char const *argv[])
{
    std::cout << "Test" << std::endl;

    std::cout << "Simple function " << Math::rad_to_deg(10.0) << std::endl;

    Vector2i vec;
    vec.x = 50;
    vec.y = 12;

    vec = vec * 2;

    std::cout << "Vec function " << vec.length() << std::endl;

    // Unit unit;
    // initialize_example_module(MODULE_INITIALIZATION_LEVEL_CORE);

    ClassDB::register_class<Unit>();

    std::cout << "registered unit." << std::endl;

    auto unit = memnew(Unit);

    std::cout << "deleting unit." << std::endl;

    memdelete(unit);

    std::cout << "Closing." << std::endl;

    return 0;
}
