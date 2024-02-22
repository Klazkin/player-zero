#ifndef SURFACE_ELEMENT_H
#define SURFACE_ELEMENT_H

#include <godot_cpp/classes/ref_counted.hpp>
#include "unit_subscriber.h"

using namespace godot;

enum Direction
{
    EAST = 0,
    NORTH = 1,
    WEST = 2,
    SOUTH = 3
};

VARIANT_ENUM_CAST(Direction);

class SurfaceElement : public RefCounted
{
    GDCLASS(SurfaceElement, RefCounted)

private:
    UnitSubscriber *death_sub = nullptr;
    bool is_on_surface = false;
    Vector2i position; // initial position undefined

protected:
    static void _bind_methods();

public:
    SurfaceElement();
    virtual ~SurfaceElement();
    virtual Ref<SurfaceElement> clone() const;

    virtual int hit(int damage);
    virtual void trigger_death();

    bool get_is_on_surface() const;
    void set_is_on_surface(const int p_is_on_surface);
    Vector2i get_position() const;
    void set_position(const Vector2i p_position);
    void set_death_subscriber(UnitSubscriber *p_death_sub);

    virtual bool is_unit() const; //  bad method that made me write a bunch of bad code
    virtual bool is_dead() const;
};

#endif