#include "surface_element.h"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SurfaceElement::_bind_methods()
{
    BIND_ENUM_CONSTANT(EAST);
    BIND_ENUM_CONSTANT(NORTH);
    BIND_ENUM_CONSTANT(SOUTH);
    BIND_ENUM_CONSTANT(WEST);

    ClassDB::bind_method(D_METHOD("get_position"), &SurfaceElement::get_position);
    ClassDB::bind_method(D_METHOD("get_is_on_surface"), &SurfaceElement::get_is_on_surface);

    ADD_SIGNAL(MethodInfo("hurt", PropertyInfo(Variant::INT, "damage")));
    ADD_SIGNAL(MethodInfo("position_changed", PropertyInfo(Variant::VECTOR2I, "new_pos")));
    ADD_SIGNAL(MethodInfo("killed"));
}

SurfaceElement::SurfaceElement()
{
}

SurfaceElement::~SurfaceElement()
{
    UtilityFunctions::print("~SurfaceElement()");
    delete death_sub;
}

Ref<SurfaceElement> SurfaceElement::clone() const
{
    UtilityFunctions::print("Called clone on SurfaceElement");
    Ref<SurfaceElement> clone = memnew(SurfaceElement());
    return clone;
}

int SurfaceElement::hit(int damage)
{
    emit_signal("hurt", damage);
    return 0;
}

void SurfaceElement::trigger_death()
{
    death_sub->on_death();
    emit_signal("killed");
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
    emit_signal("position_changed", p_postion);
    position = p_postion;
}

void SurfaceElement::set_death_subscriber(UnitSubscriber *p_death_sub)
{
    if (death_sub != nullptr)
    {
        delete death_sub;
    }
    death_sub = p_death_sub;
}

bool SurfaceElement::is_unit() const
{
    return false;
}

bool SurfaceElement::is_dead() const // TODO Depricated
{
    return false;
}
