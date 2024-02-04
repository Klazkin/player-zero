#include "surface.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

std::size_t VectorHasher::operator()(const Vector2i vec) const
{
    return std::hash<int>()(vec.x) ^ std::hash<int>()(vec.y + 1);
}

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
    std::unordered_map<Vector2i, Ref<SurfaceElement>, VectorHasher> element_positions;
}

Surface::~Surface()
{
}

std::vector<Vector2i> Surface::get_free_neighbors(const Vector2i pos) const
{
    std::vector<Vector2i> neighbors;
    for (Vector2i offset : {Vector2i(1, 0), Vector2i(0, 1), Vector2i(-1, 0), Vector2i(0, -1)})
    {
        if (is_position_available(pos + offset))
        {
            neighbors.push_back(pos + offset);
        }
    }

    return neighbors;
}

PackedVector2Array Surface::get_shortest_path(const Vector2i path_start, const Vector2i path_end) const
{
    std::unordered_map<Vector2i, int, VectorHasher> cost_so_far;
    std::unordered_map<Vector2i, Vector2i, VectorHasher> came_from;

    auto manhattan_dist = [&](Vector2i a, Vector2i b) -> bool
    {
        int a_priority = abs(path_end.x - a.x) + abs(path_end.y - a.y) + cost_so_far.at(a);
        int b_priority = abs(path_end.x - b.x) + abs(path_end.y - b.y) + cost_so_far.at(b);

        /*
        Note that the Compare parameter is defined such that it returns true if
        its first argument comes before its second argument in a weak ordering.
        But because the priority queue outputs largest elements first,
        the elements that "come before" are actually output last. That is,
        the front of the queue contains the "last" element according to the
        weak ordering imposed by Compare.
        */
        return a_priority >= b_priority;
    };
    std::priority_queue<Vector2i, std::vector<Vector2i>, decltype(manhattan_dist)> frontier(manhattan_dist);
    frontier.push(path_start);

    cost_so_far[path_start] = 0;
    came_from[path_start] = path_start;
    bool max_cost_reached = false;

    while (!frontier.empty())
    {
        Vector2i current = frontier.top();
        frontier.pop();

        if (current == path_end || max_cost_reached)
            break;

        for (Vector2i next : get_free_neighbors(current))
        {
            UtilityFunctions::print("    Next:   " + String(next));
            int new_cost = cost_so_far[current] + 1;

            if (new_cost > 256) // prevent infinite iteration in cases of large infinite maps
            {
                max_cost_reached = true;
                break;
            }

            if (cost_so_far.count(next) == 0 || new_cost < cost_so_far[next])
            {
                cost_so_far[next] = new_cost;
                frontier.push(next);
                came_from[next] = current;
            }
        }
    }

    PackedVector2Array path;
    auto recon_pos = path_end;
    while (recon_pos != path_start)
    {
        path.append(recon_pos);
        Vector2i next_pos = came_from[recon_pos];

        if (recon_pos != next_pos)
            recon_pos = next_pos;
        else
            break;
    }

    return path;
}

/*
 * Based on an extended Bresenham's line algorithm.
 */
Vector2i Surface::get_ray_collision(const Vector2i ray_start, const Vector2i ray_end) const
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
