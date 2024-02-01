#include "surface_element.h"

using namespace godot;

void SurfaceElement::_bind_methods()
{
    BIND_ENUM_CONSTANT(EAST);
    BIND_ENUM_CONSTANT(NORTH);
    BIND_ENUM_CONSTANT(SOUTH);
    BIND_ENUM_CONSTANT(WEST);

    ClassDB::bind_method(D_METHOD("get_position"), &SurfaceElement::get_position);
    ClassDB::bind_method(D_METHOD("get_is_on_surface"), &SurfaceElement::get_is_on_surface);
}

SurfaceElement::SurfaceElement()
{
    is_on_surface = false;
    position; // initial position undefined
}

SurfaceElement::~SurfaceElement()
{
}

int SurfaceElement::hit(int damage)
{
    return 0;
}

bool SurfaceElement::get_is_on_surface() const
{
    return is_on_surface;
}

void SurfaceElement::set_is_on_surface(const int p_is_on_surface)
{
    is_on_surface = p_is_on_surface;
}

Vector2i SurfaceElement::get_position() const
{
    return position;
}

void SurfaceElement::set_position(const Vector2i p_postion)
{
    position = p_postion;
}

bool SurfaceElement::_is_unit() const
{
    return false;
}

void SurfaceElement::kill()
{
}
