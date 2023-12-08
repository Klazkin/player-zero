#include "surface.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Surface::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_pathfinding_provider", "p_provider"), &Surface::set_pathfinding_provider);
    ClassDB::bind_method(D_METHOD("get_pathfinding_provider"), &Surface::get_pathfinding_provider);
    ClassDB::add_property("Surface", PropertyInfo(Variant::NIL, "pathfinding_provider"), "set_pathfinding_provider", "get_pathfinding_provider");

    ClassDB::bind_method(D_METHOD("set_collision_provider", "p_provider"), &Surface::set_collision_provider);
    ClassDB::bind_method(D_METHOD("get_collision_provider"), &Surface::get_collision_provider);
    ClassDB::add_property("Surface", PropertyInfo(Variant::NIL, "collision_provider"), "set_collision_provider", "get_collision_provider");

    ClassDB::bind_method(D_METHOD("place", "p_pos", "p_element"), &Surface::place);
    ClassDB::bind_method(D_METHOD("move", "p_pos", "p_element"), &Surface::move);
    ClassDB::bind_method(D_METHOD("get_occupation", "p_pos"), &Surface::get_occupation);
    ClassDB::bind_method(D_METHOD("clear_occupation", "p_pos"), &Surface::clear_occupation);
    ClassDB::bind_method(D_METHOD("get_all_units"), &Surface::get_all_units);
}

Surface::Surface()
{
    std::map<Vector2i, SurfaceElement *> occupations;
    pathfinding_provider = nullptr;
    collision_provider = nullptr;
}

Surface::~Surface()
{
}

void Surface::set_pathfinding_provider(AbstractPathfindingProvider *p_provider)
{
    pathfinding_provider = p_provider;
}

AbstractPathfindingProvider *Surface::get_pathfinding_provider() const
{
    return pathfinding_provider;
}

void Surface::set_collision_provider(AbstractCollisionProvider *p_provider)
{
    collision_provider = p_provider;
}

AbstractCollisionProvider *Surface::get_collision_provider() const
{
    return collision_provider;
}

void Surface::place(const Vector2i &p_pos, SurfaceElement *p_element)
{
    if (occupations.count(p_pos) > 0)
        return;

    occupations[p_pos] = p_element;
}

void Surface::move(const Vector2i &p_pos, SurfaceElement *p_element)
{
}

SurfaceElement *Surface::get_occupation(const Vector2i &p_pos) const
{
    return nullptr;
}

SurfaceElement *Surface::clear_occupation(const Vector2i &p_pos) const
{
    return nullptr;
}

TypedArray<Unit> Surface::get_all_units() const
{
    return TypedArray<Unit>();
}

void LosCheckResult::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_collision_object", "p_object"), &LosCheckResult::set_collision_object);
    ClassDB::bind_method(D_METHOD("get_collision_object"), &LosCheckResult::get_collision_object);
    ClassDB::add_property("LosCheckResult", PropertyInfo(Variant::NIL, "collision_object"), "set_collision_object", "get_collision_object");

    ClassDB::bind_method(D_METHOD("set_collision_position", "p_pos"), &LosCheckResult::set_collision_position);
    ClassDB::bind_method(D_METHOD("get_collision_position"), &LosCheckResult::get_collision_position);
    ClassDB::add_property("LosCheckResult", PropertyInfo(Variant::VECTOR2I, "collision_position"), "set_collision_position", "get_collision_position");
}

LosCheckResult::LosCheckResult()
{
    element = nullptr;
}

LosCheckResult::~LosCheckResult()
{
}

void LosCheckResult::set_collision_position(const Vector2i &p_pos)
{
    collided_at = p_pos;
}

void LosCheckResult::set_collision_object(SurfaceElement *p_element)
{
    element = p_element;
}

Vector2i LosCheckResult::get_collision_position() const
{
    return collided_at;
}

SurfaceElement *LosCheckResult::get_collision_object() const
{
    return element;
}

void AbstractCollisionProvider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getLosCheck", "from", "to"), &AbstractCollisionProvider::getLosCheck);
}

LosCheckResult *AbstractCollisionProvider::getLosCheck(const Vector2i &from, const Vector2i &to) // TODO ACTUALLY ABSTRACT METHOD
{
    return nullptr;
}

void AbstractPathfindingProvider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getPath", "from", "to"), &AbstractPathfindingProvider::getPath);
}

TypedArray<Vector2i> AbstractPathfindingProvider::getPath(const Vector2i &from, const Vector2i &to) // TODO ACTUALLY ABSTRACT METHOD
{
    TypedArray<Vector2i> arr;

    arr.resize(2);
    arr[0] = Vector2i(1, 2);
    arr[1] = Vector2i(2, 3);

    return arr;
}
