#include "main.h"
#include <iostream>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/variant/vector2i.hpp>
#include "unit.h"
#include "ortbinding.h"
#include "register_types.h"

using namespace godot;

int main(int argc, char const *argv[])
{
    std::cout << "Starting Procedure." << std::endl;
    std::array<float, 12> input{};
    _predict(input);
    std::cout << "Procedure finished." << std::endl;
}
