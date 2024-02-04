#include "main.h"
#include <iostream>
// #include <godot_cpp/core/class_db.hpp>
// #include <godot_cpp/core/math.hpp>
// #include <godot_cpp/variant/vector2i.hpp>
// #include <godot_cpp/variant/vector2i.hpp>
// #include "unit.h"
// #include "ortbinding.h"
// #include "register_types.h"

// using namespace godot;
using namespace std;

int main(int argc, char const *argv[])
{
    bresenham(23, 57, -10, -2);
    printf("\n");
}

void bresenham(int x1, int y1, int x2, int y2)
{
    int dx = abs(x1 - x2); // difference
    int dy = abs(y1 - y2);

    int tx = (x1 < x2) ? 1 : -1; // direction of change (positive or negative)
    int ty = (y1 < y2) ? 1 : -1;

    if (dx >= dy) //  dx > dy -> x increases faster (easy slope)
    {
        int pk = 2 * dy - dx;
        for (int x = x1, y = y1; x != x2 + tx; x += tx)
        {
            cout << "(" << x << "," << y << "), ";

            if (pk >= 0)
            {
                y += ty;
                pk -= 2 * dx;
            }
            pk += 2 * dy;
        }
    }
    else // y increases faster (steep slope)
    {
        int pk = 2 * dx - dy;
        for (int x = x1, y = y1; y != y2 + ty; y += ty)
        {
            cout << "(" << x << "," << y << "), ";

            if (pk >= 0)
            {
                x += tx;
                pk -= 2 * dy;
            }
            pk += 2 * dx;
        }
    }
}