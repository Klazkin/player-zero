#ifndef UNIT_SUBSCRIBER_H
#define UNIT_SUBSCRIBER_H

#include "clone_context.h"

enum UnitSubscriberIdentifier
{
    INVALID_SUB = -1,
    SURFACE_KILL,
    AMROR,
    STATUS_BURN,
    STATUS_SLOW,
    STATUS_COUNTDOWN,
    STATUS_SHACLES,
    STATUS_DUSTED,
    STATUS_SPIRITING,
    STATUS_IMMOLATION,
    STATUS_CORE_ARMOR,
    STATUS_HOARFROST_ARMOR
};

class UnitSubscriber
{
protected:
    UnitSubscriberIdentifier sub_id = INVALID_SUB;

public:
    UnitSubscriber();
    virtual ~UnitSubscriber();
    virtual void clone_to(CloneContext &clone_context) const; // int clone_context todo its a map of ref to ref

    UnitSubscriberIdentifier get_id() const;

    virtual void on_death();
    virtual void on_turn_start();
    virtual void on_hit(int damage);
};

#endif