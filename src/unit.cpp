#include "unit.h"
#include <algorithm>
#include <random>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Unit::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_base_max_health"), &Unit::get_base_max_health);
    ClassDB::bind_method(D_METHOD("set_base_max_health", "p_max_health"), &Unit::set_base_max_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "base_max_health"), "set_base_max_health", "get_base_max_health");
    ClassDB::bind_method(D_METHOD("get_max_health"), &Unit::get_max_health);

    ClassDB::bind_method(D_METHOD("get_health"), &Unit::get_health);
    ClassDB::bind_method(D_METHOD("set_health", "p_health"), &Unit::set_health);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "health"), "set_health", "get_health");

    ClassDB::bind_method(D_METHOD("get_base_speed"), &Unit::get_base_speed);
    ClassDB::bind_method(D_METHOD("set_base_speed", "p_speed"), &Unit::set_base_speed);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "base_speed"), "set_base_speed", "get_base_speed");
    ClassDB::bind_method(D_METHOD("get_speed"), &Unit::get_speed);

    ClassDB::bind_method(D_METHOD("get_faction"), &Unit::get_faction);
    ClassDB::bind_method(D_METHOD("set_faction", "p_faction"), &Unit::set_faction);
    ClassDB::add_property("Unit", PropertyInfo(Variant::INT, "faction"), "set_faction", "get_faction");

    ClassDB::bind_method(D_METHOD("get_subscriber_ids"), &Unit::get_subscriber_ids);
    ClassDB::bind_method(D_METHOD("is_dead"), &Unit::is_dead);

    ClassDB::bind_method(D_METHOD("set_deck", "p_deck"), &Unit::set_deck);
    ClassDB::bind_method(D_METHOD("get_deck"), &Unit::get_deck);
    ClassDB::bind_method(D_METHOD("set_hand", "p_hand"), &Unit::set_hand);
    ClassDB::bind_method(D_METHOD("get_hand"), &Unit::get_hand);
    ClassDB::bind_method(D_METHOD("is_in_hand"), &Unit::is_in_hand);

    ADD_SIGNAL(MethodInfo("health_changed", PropertyInfo(Variant::INT, "new_health")));
    ADD_SIGNAL(MethodInfo("max_health_changed", PropertyInfo(Variant::INT, "new_max_health")));
    ADD_SIGNAL(MethodInfo("speed_changed", PropertyInfo(Variant::INT, "new_speed"))); // TODO add signals for stat modifiers
    ADD_SIGNAL(MethodInfo("subscriber_applied", PropertyInfo(Variant::INT, "subscriber")));
    ADD_SIGNAL(MethodInfo("subscriber_removed", PropertyInfo(Variant::INT, "subscriber")));
    ADD_SIGNAL(MethodInfo("action_added_to_hand", PropertyInfo(Variant::INT, "action")));
    ADD_SIGNAL(MethodInfo("action_removed_from_hand", PropertyInfo(Variant::INT, "action")));
    ADD_SIGNAL(MethodInfo("healed", PropertyInfo(Variant::INT, "damage")));

    BIND_ENUM_CONSTANT(UNDEFINED);
    BIND_ENUM_CONSTANT(PLAYER);
    BIND_ENUM_CONSTANT(MONSTER);
}

Unit::Unit()
{
}

Unit::~Unit()
{
    for (auto key_value_pair : subscribers)
    {
        delete key_value_pair.second;
    }
}

Ref<SurfaceElement> Unit::clone() const
{
    Ref<Unit> clone = memnew(Unit());

    clone->base_max_health = base_max_health;
    clone->health = health;
    clone->base_speed = base_speed;
    clone->faction = faction;

    clone->deck = deck;                     // copies
    clone->hand = hand;                     // copies
    clone->stat_modifiers = stat_modifiers; // copies

    // clone->subscribers = subscribers; // Do not clone directly, use clone_subscribers_to instead.
    return clone;
}

void Unit::clone_subscribers_to(CloneContext &clone_context) const
{
    for (auto key_value_pair : subscribers)
    {
        key_value_pair.second->clone_to(clone_context);
    }
}

int Unit::hit(int damage)
{
    return hit(damage, true);
}

int Unit::hit(int damage, bool trigger_on_hit)
{
    if (is_dead())
        return 0; // todo same as bellow

    if (stat_modifiers.armored)
    {
        stat_modifiers.armored = false;
        damage = 0;
    }

    damage = std::max(0, damage - stat_modifiers.defence);
    set_health(get_health() - damage);

    if (trigger_on_hit)
        trigger_on_hit_subscribers(damage); // todo here it should be final applied damage also? Implement damage instance struct..

    emit_signal("hurt", damage);

    if (is_dead())
        trigger_death();

    return 1; // TODO return damage change
}

int Unit::heal(int damage)
{
    if (is_dead())
    {
        return 0; // cannot heal dead unit
    }

    set_health(get_health() + damage);
    emit_signal("healed", damage);
    return 1; // TODO return heal change
}

void Unit::set_base_max_health(const int p_max_health)
{
    base_max_health = p_max_health;
    emit_signal("max_health_changed", p_max_health);
}

int Unit::get_base_max_health() const
{
    return base_max_health;
}

int Unit::get_max_health() const
{
    return base_max_health + stat_modifiers.max_health;
}

void Unit::set_health(const int p_health)
{
    health = std::clamp(p_health, 0, base_max_health);
    emit_signal("health_changed", p_health);
}

int Unit::get_health() const
{
    return health;
}

void Unit::set_base_speed(const int p_speed)
{
    base_speed = p_speed;
    emit_signal("speed_changed", p_speed);
}

int Unit::get_base_speed() const
{
    return base_speed;
}

int Unit::get_speed() const
{
    return base_speed + stat_modifiers.speed;
}

void Unit::set_faction(const Faction p_faction)
{
    faction = p_faction;
}

Faction Unit::get_faction() const
{
    return faction;
}

bool Unit::is_unit() const
{
    return true;
}

bool Unit::is_dead() const
{
    return health <= 0;
}

void Unit::add_subscriber(UnitSubscriber *subscriber)
{
    if (has_subscriber(subscriber->get_id()))
    {
        remove_subscriber(subscriber->get_id());
    }

    subscribers[subscriber->get_id()] = subscriber;
    emit_signal("subscriber_applied", subscriber->get_id());
}

bool Unit::has_subscriber(UnitSubscriberIdentifier id) const
{
    return subscribers.count(id) > 0;
}

void Unit::remove_subscriber(UnitSubscriberIdentifier id)
{
    delete subscribers[id];
    subscribers.erase(id);
    emit_signal("subscriber_removed", id);
}

int Unit::get_subscriber_duration(UnitSubscriberIdentifier id) const
{
    if (!has_subscriber(id))
        return 0;

    return subscribers.at(id)->get_duration();
}

UnitSubscriber *Unit::get_subscriber(UnitSubscriberIdentifier id) // todo really really really bad
{
    return subscribers.at(id);
}

void Unit::trigger_on_start_turn_subscribers()
{
    auto copy = subscribers;
    for (auto key_value_pair : copy) // todo linked lists?
    {
        key_value_pair.second->on_turn_start();
    }
}

void Unit::trigger_on_hit_subscribers(int damage)
{
    std::vector<UnitSubscriberIdentifier> ids;

    for (auto key_value_pair : subscribers) // todo linked lists?
    {
        // key_value_pair.second->on_hit(damage);
        ids.push_back(key_value_pair.first);
    }

    for (auto id : ids) // due to turn ending in the middle of trigger on hit, some subsribers are removed and the copy now has invalid pointers
    {
        if (has_subscriber(id))
        {
            subscribers[id]->on_hit(damage);
        }
    }
}

void Unit::reset_stat_modifiers()
{
    stat_modifiers = StatModifiers();
}

StatModifiers &Unit::get_stat_modifiers()
{
    return stat_modifiers;
}

TypedArray<int> Unit::get_subscriber_ids() const
{
    TypedArray<int> arr;
    for (auto key_value_pair : subscribers)
    {
        arr.append(key_value_pair.second->get_id());
    }

    return arr;
}

std::vector<UnitSubscriberIdentifier> Unit::get_subscriber_ids_vec() const
{
    std::vector<UnitSubscriberIdentifier> arr;

    for (auto key_value_pair : subscribers)
    {
        arr.push_back(key_value_pair.second->get_id());
    }

    return arr;
}

void Unit::set_deck(const TypedArray<int> &p_deck)
{
    deck.clear();
    for (int i = 0; i < p_deck.size(); i++)
    {
        deck.insert((ActionIdentifier)(int)p_deck[i]);
    }
}

TypedArray<int> Unit::get_deck() const
{
    TypedArray<int> arr;
    for (const auto action : deck)
    {
        arr.append(action);
    }

    return arr;
}

void Unit::set_hand(const TypedArray<int> &p_hand)
{
    hand.clear();
    for (int i = 0; i < p_hand.size(); i++)
    {
        ActionIdentifier action = (ActionIdentifier)(int)p_hand[i];
        add_to_hand(action);
    }
}

TypedArray<int> Unit::get_hand() const
{
    TypedArray<int> arr;
    for (const auto action : hand)
    {
        arr.append(action);
    }

    return arr;
}

std::unordered_set<ActionIdentifier> Unit::get_hand_set() const
{
    return hand;
}

bool Unit::is_in_deck(const ActionIdentifier id) const
{
    return deck.find(id) != deck.end();
}

bool Unit::is_in_hand(const ActionIdentifier id) const
{
    return hand.find(id) != hand.end();
}

void Unit::remove_from_hand(const ActionIdentifier id)
{
    hand.erase(id);
    emit_signal("action_removed_from_hand", id);
}

void godot::Unit::add_to_hand(const ActionIdentifier id)
{
    hand.insert(id);
    emit_signal("action_added_to_hand", id);
}

void Unit::refill_hand() // TODO use shuffle bag instead? https://docs.godotengine.org/en/stable/tutorials/math/random_number_generation.html
{
    if (is_in_deck(ActionIdentifier::TREAD) && !is_in_hand(ActionIdentifier::TREAD))
    {
        add_to_hand(ActionIdentifier::TREAD); // Free refill for tread
    }

    std::vector<ActionIdentifier> refill_candidates;

    std::copy_if(deck.begin(), deck.end(), std::back_inserter(refill_candidates),
                 [&](ActionIdentifier a)
                 { return hand.find(a) == hand.end(); });

    if (refill_candidates.size() == 0)
    {
        return;
    }

    int refill_i = UtilityFunctions::randi_range(0, refill_candidates.size() - 1);
    add_to_hand(refill_candidates[refill_i]);
}

Unit *godot::as_unit_ptr(Ref<SurfaceElement> element)
{
    // todo maybe use Ref<Unit> <== Ref<SurfaceElement> instead
    return Object::cast_to<Unit>(*element);
}
