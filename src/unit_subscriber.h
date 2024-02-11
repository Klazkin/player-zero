enum UnitSubscriberIdentifier
{
    INVALID_SUB = -1,
    AMROR,
    STATUS_BURN,
    STATUS_SLOW,
    STATUS_COUNTDOWN,
    STATUS_SHACLES
};

class UnitSubscriber
{
protected:
    UnitSubscriberIdentifier sub_id = INVALID_SUB;

public:
    UnitSubscriber();
    ~UnitSubscriber();

    UnitSubscriberIdentifier get_id() const;

    virtual void on_turn_start();
    virtual void on_hit(int damage);
};
