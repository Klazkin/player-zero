#include "surface.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Surface::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_position_available", "p_pos"), &Surface::is_position_available);
    ClassDB::bind_method(D_METHOD("place_element", "p_pos", "p_element"), &Surface::place_element);
    ClassDB::bind_method(D_METHOD("move_element", "p_pos_from", "p_pos_to"), &Surface::move_element);
    ClassDB::bind_method(D_METHOD("get_element", "p_pos"), &Surface::get_element);
    ClassDB::bind_method(D_METHOD("lift_element", "p_pos"), &Surface::lift_element);
    ClassDB::bind_method(D_METHOD("get_only_units"), &Surface::get_only_units);
    ClassDB::bind_method(D_METHOD("get_shortest_path", "path_start", "path_end"), &Surface::get_shortest_path);
    ClassDB::bind_method(D_METHOD("get_ray_collision", "ray_start", "ray_end"), &Surface::get_ray_collision);
}

Surface::Surface()
{
    std::map<Vector2i, Ref<SurfaceElement>> element_positions;
}

Surface::~Surface()
{
}

PackedVector3Array godot::Surface::get_shortest_path(const Vector2i path_start, const Vector2i path_end)
{
    return PackedVector3Array();
}

/*
 * Based on an extended Bresenham's line algorithm.
 */
Vector2i godot::Surface::get_ray_collision(const Vector2i ray_start, const Vector2i ray_end)
{
    int x1 = ray_start.x, y1 = ray_start.y, x2 = ray_end.x, y2 = ray_end.y;

    int dx = abs(x1 - x2); // difference
    int dy = abs(y1 - y2);

    int tx = (x1 < x2) ? 1 : -1; // direction of change (positive or negative)
    int ty = (y1 < y2) ? 1 : -1;

    if (dx >= dy) // x increases faster than y (easy slope)
    {
        int pk = 2 * dy - dx;
        for (int x = x1, y = y1; x != x2 + tx; x += tx)
        {
            if (x != x1 && !is_position_available(Vector2i(x, y)))
                return Vector2i(x, y);

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
            if (y != y1 && !is_position_available(Vector2i(x, y)))
                return Vector2i(x, y);

            if (pk >= 0)
            {
                x += tx;
                pk -= 2 * dy;
            }
            pk += 2 * dx;
        }
    }

    return ray_end;
}

bool Surface::is_position_available(const Vector2i &p_pos) const
{
    return element_positions.count(p_pos) == 0;
}

void Surface::place_element(const Vector2i &p_pos, const Ref<SurfaceElement> p_element)
{
    if (!p_element.is_valid() ||
        p_element->get_is_on_surface() ||
        !is_position_available(p_pos))
    {
        UtilityFunctions::print("Did not place");
        UtilityFunctions::print(!p_element.is_valid());
        UtilityFunctions::print(p_element.is_null());
        UtilityFunctions::print(p_element->get_is_on_surface());
        UtilityFunctions::print(element_positions.count(p_pos) > 0);
        return;
    }
    p_element->set_is_on_surface(true);
    p_element->set_position(p_pos);
    element_positions[p_pos] = p_element;
}

bool Surface::move_element(const Vector2i &p_pos_from, const Vector2i &p_pos_to)
{
    Ref<SurfaceElement> element = get_element(p_pos_from);
    if (element.is_null() || !is_position_available(p_pos_to))
    {
        return false;
    }

    element_positions.erase(p_pos_from);
    element_positions[p_pos_to] = element;
    return true;
}

Ref<SurfaceElement> Surface::get_element(const Vector2i &p_pos) const
{
    if (is_position_available(p_pos))
    {
        Ref<SurfaceElement> empty;
        return empty;
    }

    return element_positions.at(p_pos);
}

Ref<SurfaceElement> Surface::lift_element(const Vector2i &p_pos)
{
    Ref<SurfaceElement> element = get_element(p_pos);

    if (element.is_valid())
    {
        element_positions.erase(p_pos);
        element->set_is_on_surface(false);
    }

    return element;
}

TypedArray<Unit> Surface::get_only_units() const
{
    TypedArray<Unit> arr;

    for (auto pair : element_positions) // TODO check if the map contians empty cells
    {
        if (pair.second->_is_unit())
        {
            arr.append(pair.second);
        }
    }

    return arr;
}