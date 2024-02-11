#include "destructible_element.h"

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
