#include "main.h"
#include <iostream>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/memory.hpp>
#include "unit.h"
#include "register_types.h"

using namespace godot;

int main(int argc, char const *argv[])
{

    std::cout << "Test" << std::endl;

    ClassDB::register_class<Unit>();

    std::cout << "Registered" << std::endl;

    // Assuming `Unit` has a default constructor
    Unit *unit = memnew(Unit());

    // Print some information if needed
    std::cout << "Unit Health: " << unit->get_max_health() << std::endl;

    // Delete the allocated object
    memdelete(unit);

    std::cout << "Closing." << std::endl;

    return 0;
}
