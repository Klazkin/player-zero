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
    void trigger() override;
};

class BurnStatus : public Status
{

public:
    BurnStatus(Unit *p_target_ptr, const int p_duration);

    void trigger() override;
};
#endif