#ifndef UNIT_SUBSCRIBER_H
#define UNIT_SUBSCRIBER_H

enum UnitSubscriberIdentifier
{
    INVALID_SUB = -1,
    SURFACE_KILL,
    AMROR,
    STATUS_BURN,
    STATUS_SLOW,
    STATUS_COUNTDOWN,
    STATUS_SHACLES,
};

class UnitSubscriber
{
protected:
    UnitSubscriberIdentifier sub_id = INVALID_SUB;

public:
    UnitSubscriber();
    virtual ~UnitSubscriber();

    UnitSubscriberIdentifier get_id() const;

    virtual void on_death();
    virtual void on_turn_start();
    virtual void on_hit(int damage);
};

#endif