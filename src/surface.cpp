#include "surface.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Surface::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_pathfinding_provider", "p_provider"), &Surface::set_pathfinding_provider);
    ClassDB::bind_method(D_METHOD("get_pathfinding_provider"), &Surface::get_pathfinding_provider);
    ClassDB::add_property("Surface", PropertyInfo(Variant::NIL, "pathfinding_provider"), "set_pathfinding_provider", "get_pathfinding_provider");

    ClassDB::bind_method(D_METHOD("set_collision_provider", "p_provider"), &Surface::set_collision_provider);
    ClassDB::bind_method(D_METHOD("get_collision_provider"), &Surface::get_collision_provider);
    ClassDB::add_property("Surface", PropertyInfo(Variant::NIL, "collision_provider"), "set_collision_provider", "get_collision_provider");

    ClassDB::bind_method(D_METHOD("is_position_available", "p_pos"), &Surface::is_position_available);
    ClassDB::bind_method(D_METHOD("place_element", "p_pos", "p_element"), &Surface::place_element);
    ClassDB::bind_method(D_METHOD("move_element", "p_pos_from", "p_pos_to"), &Surface::move_element);
    ClassDB::bind_method(D_METHOD("get_element", "p_pos"), &Surface::get_element);
    ClassDB::bind_method(D_METHOD("lift_element", "p_pos"), &Surface::lift_element);
    ClassDB::bind_method(D_METHOD("get_only_units"), &Surface::get_only_units);
}

Surface::Surface()
{
    std::map<Vector2i, Ref<SurfaceElement>> element_positions;
    Ref<PathfindingProvider> pathfinding_provider;
    Ref<CollisionProvider> collision_provider;
}

Surface::~Surface()
{
}

void Surface::set_pathfinding_provider(const Ref<PathfindingProvider> p_provider)
{
    pathfinding_provider = p_provider;
}

Ref<PathfindingProvider> Surface::get_pathfinding_provider() const
{
    return pathfinding_provider;
}

void Surface::set_collision_provider(const Ref<CollisionProvider> p_provider)
{
    collision_provider = p_provider;
}
Ref<CollisionProvider> Surface::get_collision_provider() const
{
    return collision_provider;
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

void LosCheckResult::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_collision_object", "p_object"), &LosCheckResult::set_collision_element);
    ClassDB::bind_method(D_METHOD("get_collision_object"), &LosCheckResult::get_collision_element);
    ClassDB::add_property("LosCheckResult", PropertyInfo(Variant::NIL, "collision_object"), "set_collision_object", "get_collision_object");

    ClassDB::bind_method(D_METHOD("set_collision_position", "p_pos"), &LosCheckResult::set_collision_position);
    ClassDB::bind_method(D_METHOD("get_collision_position"), &LosCheckResult::get_collision_position);
    ClassDB::add_property("LosCheckResult", PropertyInfo(Variant::VECTOR2I, "collision_position"), "set_collision_position", "get_collision_position");
}

LosCheckResult::LosCheckResult()
{
    Ref<SurfaceElement> element;
}

LosCheckResult::~LosCheckResult()
{
}

void LosCheckResult::set_collision_position(const Vector2i &p_pos)
{
    collided_at = p_pos;
}

void LosCheckResult::set_collision_element(Ref<SurfaceElement> p_element)
{
    element = p_element;
}

Vector2i LosCheckResult::get_collision_position() const
{
    return collided_at;
}

Ref<SurfaceElement> LosCheckResult::get_collision_element() const // TODO in future may also include level geometry as a potential collider
{
    return element;
}

void CollisionProvider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getLosCheck", "from", "to"), &CollisionProvider::getLosCheck);
}

Ref<LosCheckResult> CollisionProvider::getLosCheck(const Vector2i &from, const Vector2i &to)
{
    return nullptr;
}

void PathfindingProvider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getPath", "from", "to"), &PathfindingProvider::getPath);
}

TypedArray<Vector2i> PathfindingProvider::getPath(const Vector2i &from, const Vector2i &to)
{
    TypedArray<Vector2i> arr;

    arr.resize(2);
    arr[0] = Vector2i(1, 2);
    arr[1] = Vector2i(2, 3);

    return arr;
}
