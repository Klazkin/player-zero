#include "surface.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Surface::_bind_methods()
{
}

Surface::Surface()
{
    // std::unordered_map<Vector2i, SurfaceElement> occupations;
}

Surface::~Surface()
{
}

void Surface::set_pathfinding_provider(AbstractPathfindingProvider *p_provider)
{
    // pathfinding_provider = p_provider;
}

AbstractPathfindingProvider Surface::get_pathfinding_provider() const
{
    return pathfinding_provider;
}

void Surface::set_collision_provider(AbstractCollisionProvider *p_provider)
{
    // collision_provider = p_provider;
}

AbstractCollisionProvider Surface::get_collision_provider() const
{
    return collision_provider;
}

void Surface::occupy(const Vector2i p_pos, const SurfaceElement p_element)
{
}

void Surface::reoccupy(const Vector2i p_pos, const SurfaceElement p_element)
{
}

SurfaceElement Surface::get_occupation(const Vector2i p_pos) const
{
    return SurfaceElement();
}

SurfaceElement Surface::clear_occupation(const Vector2i p_pos) const
{
    return SurfaceElement();
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

LosCheckResult *AbstractCollisionProvider::getLosCheck(const Vector2i &from, const Vector2i &to)
{
    return nullptr;
}

void AbstractPathfindingProvider::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("getPath", "from", "to"), &AbstractPathfindingProvider::getPath);
}

void AbstractPathfindingProvider::getPath(const Vector2i &from, const Vector2i &to)
{
}
