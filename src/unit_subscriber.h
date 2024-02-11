enum UnitSubscriberIdentifier
{
    INVALID_SUB = -1,
    AMROR,
    STATUS_BURN,
    STATUS_SLOW,
};

class UnitSubscriber
{
protected:
    UnitSubscriberIdentifier sub_id = INVALID_SUB;

public:
    UnitSubscriber();
    ~UnitSubscriber();

    UnitSubscriberIdentifier get_id() const;

    virtual void trigger();
};
