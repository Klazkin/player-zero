#include "surface_element.h"

using namespace godot;

void SurfaceElement::_bind_methods()
{
}

SurfaceElement::SurfaceElement()
{
    is_on_surface = false;
}

SurfaceElement::~SurfaceElement()
{
}

int SurfaceElement::hit(int damage) const
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

bool SurfaceElement::_is_unit() const
{
    return false;
}

void SurfaceElement::kill()
{
}
