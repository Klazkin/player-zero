#ifndef UNIT_H
#define UNIT_H

#include <unordered_map>
#include "surface_element.h"
#include "unit_subscriber.h"

namespace godot
{

    class Unit : public SurfaceElement
    {
        GDCLASS(Unit, SurfaceElement)

    private:
        std::unordered_map<UnitSubscriberIdentifier, UnitSubscriber *> subscribers;
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

        bool is_unit() const override;
        bool is_dead() const override;

        void add_subscriber(UnitSubscriber *subscriber);
        void remove_subscriber(UnitSubscriberIdentifier id);
        void trigger_all_subscribers();
    };

}

#endif