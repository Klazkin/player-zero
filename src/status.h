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
    ~Status();

public:
    virtual void decrease_duration();
    virtual int get_duration() const;

    void on_turn_start() override;
    void on_hit(int damage) override;
    void on_death() override;
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

class ShaclesParent : public Status
{

private:
    Unit *shacle_target_ptr = nullptr;
    int *link_counter = 0;

public:
    ShaclesParent(int *p_link_counter, Unit *caster_ptr, Unit *p_target_ptr, const int p_duration);
    ~ShaclesParent();

    void on_hit(int damage) override;
};

class ShaclesChild : public Status
{

private:
    int *link_counter = 0;

public:
    ShaclesChild(int *p_link_counter, Unit *p_target_ptr, const int p_duration);
    ~ShaclesChild();
};

class Dusted : public Status
{

public:
    Dusted(Unit *p_target_ptr, const int p_duration);

    void on_turn_start() override;
};

#endif