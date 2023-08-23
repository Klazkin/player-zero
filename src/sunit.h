#ifndef SUNIT_H
#define SUNIT_H

#include <godot_cpp/classes/object.hpp>

namespace godot
{

    class SUnit : public Object
    {
        GDCLASS(SUnit, Object)

    private:
        int health;
        int max_health;
        int speed;

    protected:
        static void _bind_methods();

    public:
        SUnit();
        ~SUnit();

        void set_health(const int p_health);
        int get_health() const;

        void set_max_health(const int p_max_health);
        int get_max_health() const;

        void set_speed(const int p_speed);
        int get_speed() const;
    };

}

#endif