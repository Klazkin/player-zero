#ifndef RANDOM_ACTION_BUNDLE_H
#define RANDOM_ACTION_BUNDLE_H

#include "action_bundle.h"

class RandomActionBundle : public ActionBundle
{

private:
    Ref<Surface> surface;
    Ref<Unit> caster;
    bool is_forced_to_finish = false;

public:
    RandomActionBundle(Ref<Surface> p_surface, Ref<Unit> p_caster) : surface(p_surface), caster(p_caster){};
    ~RandomActionBundle(){};

    bool is_finished() const override;
    void cast_next() override;
};

#endif