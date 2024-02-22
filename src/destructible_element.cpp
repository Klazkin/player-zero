#include "destructible_element.h"
#include <godot_cpp/variant/utility_functions.hpp>

void DestructibleElement::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_health"), &DestructibleElement::get_health);
}

DestructibleElement::DestructibleElement()
{
    health = 3;
}

DestructibleElement::~DestructibleElement()
{
}

Ref<SurfaceElement> DestructibleElement::clone() const
{
    UtilityFunctions::print("Called clone on DestructibleElement");
    Ref<DestructibleElement> clone = memnew(DestructibleElement());
    return clone;
}

int DestructibleElement::hit(int damage)
{
    health -= damage;
    emit_signal("hurt", damage);
    if (is_dead())
    {
        trigger_death();
    }

    return damage;
}

int DestructibleElement::get_health()
{
    return 0;
}

bool DestructibleElement::is_dead() const
{
    return health <= 0;
}
