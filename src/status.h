#ifndef STATUS_H
#define STATUS_H

#include "action.h"

class BaseStatus
{
private:
    int duration;

public:
    BaseStatus(const int p_duration);
    ~BaseStatus();

    virtual void cast(const CastInfo &cast) const;
    virtual void decrease_duration();
    virtual int get_duration() const;
};

#endif