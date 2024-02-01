#ifndef UNIT_H
#define UNIT_H

#include "surface_element.h"

namespace godot
{

    class Unit : public SurfaceElement
    {
        GDCLASS(Unit, SurfaceElement)

    private:
        int health;
        int max_health;
        int speed;

    protected:
        static void _bind_methods();

    public:
        Unit();
        ~Unit();

        int hit(int damage) override;

        void set_health(const int p_health);
        int get_health() const;

        void set_max_health(const int p_max_health);
        int get_max_health() const;

        void set_speed(const int p_speed);
        int get_speed() const;

        bool _is_unit() const override;
    };

}

#endif