#ifndef UNIT_H
#define UNIT_H

#include <unordered_map>
#include <unordered_set>

#include "surface_element.h"
#include "unit_subscriber.h"
#include "action_identifier.h"
#include "clone_context.h"

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
        int max_health = 0;
        int speed = 0;
        int attack = 0;
        int defence = 0;
        bool stunned = false;
        bool armored = false;
    };

    class Unit : public SurfaceElement
    {
        GDCLASS(Unit, SurfaceElement)

    private:
        std::unordered_map<UnitSubscriberIdentifier, UnitSubscriber *> subscribers;
        std::unordered_set<ActionIdentifier> deck;
        std::unordered_set<ActionIdentifier> hand;
        StatModifiers stat_modifiers;
        Faction faction = UNDEFINED;
        int base_max_health = 20;
        int base_speed = 0;
        int base_attack = 0;
        int base_defence = 0;
        int health = 20;

    protected:
        static void _bind_methods();

    public:
        Unit();
        ~Unit();
        Ref<SurfaceElement> clone() const override;
        void clone_subscribers_to(CloneContext &clone_context) const;

        int hit(int damage) override;
        int hit(int damage, bool trigger_on_hit);
        int heal(int damage);

        void set_health(const int p_health);
        int get_health() const;

        void set_base_max_health(const int p_max_health);
        int get_base_max_health() const;
        int get_max_health() const;

        void set_base_speed(const int p_speed);
        int get_base_speed() const;
        int get_speed() const;

        void set_base_attack(const int p_attack);
        int get_base_attack() const;
        int get_attack() const;

        void set_base_defence(const int p_defence);
        int get_base_defence() const;
        int get_defence() const;

        void set_faction(const Faction p_faction);
        Faction get_faction() const;

        bool is_unit() const override;
        bool is_dead() const override;

        void add_subscriber(UnitSubscriber *subscriber);
        bool has_subscriber(UnitSubscriberIdentifier id) const;
        void remove_subscriber(UnitSubscriberIdentifier id);
        int get_subscriber_duration(UnitSubscriberIdentifier id) const;
        UnitSubscriber *get_subscriber(UnitSubscriberIdentifier id);
        void trigger_on_start_turn_subscribers();
        void trigger_on_hit_subscribers(int damage);

        void reset_stat_modifiers();
        StatModifiers &get_stat_modifiers();
        TypedArray<int> get_subscriber_ids() const;
        std::vector<UnitSubscriberIdentifier> get_subscriber_ids_vec() const;

        void set_deck(const TypedArray<int> &p_deck);
        TypedArray<int> get_deck() const;

        void set_hand(const TypedArray<int> &p_hand);
        TypedArray<int> get_hand() const;
        std::unordered_set<ActionIdentifier> get_hand_set() const;

        bool is_in_deck(const ActionIdentifier id) const;
        bool is_in_hand(const ActionIdentifier id) const;
        void remove_from_hand(const ActionIdentifier id);
        void add_to_hand(const ActionIdentifier id);
        std::vector<ActionIdentifier> get_refill_candidates() const;
        void refill_hand();
        void refill_hand(ActionIdentifier refilled_action);
    };

    Unit *as_unit_ptr(Ref<SurfaceElement> element);
}

#endif