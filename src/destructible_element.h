#ifndef DESTRUCTIBLE_ELEMENT_H
#define DESTRUCTIBLE_ELEMENT_H

#include <godot_cpp/classes/ref_counted.hpp>
#include "surface_element.h"

using namespace godot;

class DestructibleElement : public SurfaceElement
{
    GDCLASS(DestructibleElement, SurfaceElement)

private:
    int health;

protected:
    static void _bind_methods();

public:
    DestructibleElement();
    ~DestructibleElement();

    int hit(int damage) override;
    int get_health();

    bool is_dead() const override;
};

#endif