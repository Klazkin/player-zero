#ifndef STATUS_H
#define STATUS_H

#include "unit.h"
#include <list>
#include <unordered_map>

class Status : public UnitSubscriber
{
protected:
    int duration;
    Unit *target_ptr = nullptr;

    Status(UnitSubscriberIdentifier p_sub_id, Unit *p_target_ptr, const int p_duration);

public:
    virtual void decrease_duration();
    virtual int get_duration() const;
    void on_turn_start() override;
};

class BurnStatus : public Status
{

public:
    BurnStatus(Unit *p_target_ptr, const int p_duration);

    void on_turn_start() override;
};

class Countdown : public Status
{

public:
    Countdown(Unit *p_target_ptr, const int p_duration);

    void on_turn_start() override;
};

class Shacles : public Status
{

private:
    Unit *shacle_target_ptr = nullptr;

public:
    Shacles(Unit *caster_ptr, Unit *p_target_ptr, const int p_duration);

    void on_hit(int damage) override;
};

#endif