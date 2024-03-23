#ifndef ACITON_BUNDLE_H
#define ACITON_BUNDLE_H

#include <handlers.h>
#include <godot_cpp/core/class_db.hpp>

class ActionBundle : public RefCounted
{
    GDCLASS(ActionBundle, RefCounted);

protected:
    static void _bind_methods();

public:
    ActionBundle(){};
    ~ActionBundle(){};

    virtual bool is_finished() const;
    virtual void cast_next();

    void warn_not_castable(const CastInfo &cast) const;
};

#endif