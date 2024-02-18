#include "surface.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include "surface_kill_subscriber.h"

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
    ClassDB::bind_method(D_METHOD("turn_generate"), &Surface::turn_generate);
    ClassDB::bind_method(D_METHOD("turn_get_order"), &Surface::turn_get_order);
    ClassDB::bind_method(D_METHOD("turn_get_current_unit"), &Surface::turn_get_current_unit);
    ClassDB::bind_method(D_METHOD("turn_next"), &Surface::turn_next);

    ADD_SIGNAL(MethodInfo("turn_started", PropertyInfo(Variant::OBJECT, "unit")));
    ADD_SIGNAL(MethodInfo("turn_ended", PropertyInfo(Variant::OBJECT, "unit")));
    ADD_SIGNAL(MethodInfo("action_cast",
                          PropertyInfo(Variant::INT, "action"),
                          PropertyInfo(Variant::OBJECT, "caster"),
                          PropertyInfo(Variant::VECTOR2I, "target")));
}

Surface::Surface()
{
    std::unordered_map<Vector2i, Ref<SurfaceElement>, VectorHasher> element_positions;
    std::vector<Ref<Unit>> unit_order;
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

PackedVector2Array Surface::get_shortest_path(const Vector2i path_start, const Vector2i path_end, const bool to_neighbor) const
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

        // Special case for "to_neighbor", when we do not want to reach the target, but merely close enough
        // TODO generalise this to some "minimum distance_reached" param
        if (to_neighbor && (current - path_end).length_squared() == 1)
        {
            came_from[path_end] = current;
            break;
        }

        for (Vector2i next : get_free_neighbors(current))
        {
            int new_cost = cost_so_far[current] + 1;
            if (new_cost > 256) // prevent infinite iteration in cases of infinite maps
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
    p_element->set_death_subscriber(new SurfaceKillSubscriber(this, *p_element)); // TODO causes movement to be slow
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
        if (pair.second->is_unit())
        {
            arr.append(pair.second);
        }
    }

    return arr;
}

std::vector<Ref<Unit>> Surface::get_only_units_vec() const
{
    std::vector<Ref<Unit>> ret;

    for (auto pair : element_positions)
    {
        if (pair.second->is_unit())
        {
            ret.push_back(pair.second);
        }
    }

    return ret;
}

bool unit_speed_compare(Ref<Unit> u1, Ref<Unit> u2)
{
    return u1->get_speed() < u2->get_speed(); // TODO handle speed ties
}

void Surface::turn_generate()
{
    if (!unit_order.empty())
    {
        return;
    }
    for (auto pair : element_positions)
    {
        if (pair.second->is_unit())
        {
            unit_order.push_back(pair.second);
        }
    }

    std::sort(unit_order.begin(), unit_order.end(), unit_speed_compare);
    _start_current_units_turn(); // first units turn is begun automatically, maybe defer to dif. func?
}

void Surface::_start_current_units_turn()
{
    Ref<Unit> cur_unit = turn_get_current_unit();
    if (cur_unit.is_valid())
    {
        emit_signal("turn_started", cur_unit);
        cur_unit->reset_stat_modifiers();
        cur_unit->trigger_on_start_turn_subscribers();
    }
}

void Surface::emit_action_cast(const int action, const Ref<SurfaceElement> caster, const Vector2i target)
{
    emit_signal("action_cast", action, caster, target); // todo redundant? use action bundle instead maybe
    //... or not because the casts come from player also
}

TypedArray<Unit> Surface::turn_get_order() const
{
    TypedArray<Unit> arr;
    for (auto u : unit_order)
    {
        arr.append(u);
    }
    return arr;
}

Ref<Unit> Surface::turn_get_current_unit() const
{
    if (unit_order.empty())
    {
        return nullptr;
    }
    return unit_order.back();
}

void Surface::turn_next()
{
    emit_signal("turn_ended", turn_get_current_unit());
    unit_order.pop_back();
    _start_current_units_turn();
}
