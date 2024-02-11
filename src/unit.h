#ifndef UNIT_H
#define UNIT_H

#include <unordered_map>
#include <unordered_set>

#include "surface_element.h"
#include "unit_subscriber.h"
#include "action_identifier.h"

enum Faction
{
    UNDEFINED = -1,
    PLAYER,
    MONSTER
};

VARIANT_ENUM_CAST(Faction);

namespace godot
{

    struct StatModifiers
    {
        int speed = 0;
        int max_health = 0;
    };

    class Unit : public SurfaceElement
    {
        GDCLASS(Unit, SurfaceElement)

    private:
        std::unordered_map<UnitSubscriberIdentifier, UnitSubscriber *> subscribers;
        // std::unordered_set<ActionIdentifier> action_pool;
        // std::vector<ActionIdentifier> action_hand;
        StatModifiers stat_modifiers;
        Faction faction = UNDEFINED;
        int health;
        int max_health;
        int speed;

    protected:
        static void _bind_methods();

    public:
        Unit();
        ~Unit();

        int hit(int damage) override;

        void set_health(const int p_health); // todo make a distinction between base stats and modified
        int get_health() const;

        void set_max_health(const int p_max_health);
        int get_max_health() const;

        void set_speed(const int p_speed);
        int get_speed() const;

        void set_faction(const Faction p_faction);
        Faction get_faction() const;

        bool is_unit() const override;
        bool is_dead() const override;

        void add_subscriber(UnitSubscriber *subscriber);
        bool has_subscriber(UnitSubscriberIdentifier id) const;
        void remove_subscriber(UnitSubscriberIdentifier id);
        void trigger_on_start_turn_subscribers();
        void trigger_on_hit_subscribers(int damage);

        void reset_stat_modifiers();
        StatModifiers get_stat_modifiers();

        // void add_action_to_pool(){};
        // void add_action_to_hand(){};
        // void get_pool();
        // void get_hand();
    };
}

#endif