#ifndef ACITON_BUNDLE_H
#define ACITON_BUNDLE_H

#include <handlers.h>
#include <godot_cpp/core/class_db.hpp>

class ActionBundle : public RefCounted
{
    GDCLASS(ActionBundle, RefCounted);

protected:
    std::vector<CastInfo> casts;
    int cast_counter = 0;
    static void _bind_methods();

public:
    ActionBundle();
    ~ActionBundle();

    virtual bool cast_until_finished();
    std::vector<CastInfo> get_casts() const;
    void push_back_cast(const CastInfo &cast);
};

#endif