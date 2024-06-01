#ifndef VECTOR_ACTION_BUNDLE_H
#define VECTOR_ACTION_BUNDLE_H

#include "action_bundle.h"

class VectorActionBundle : public ActionBundle
{

protected:
    std::vector<CastInfo> casts;
    int cast_counter = 0;

public:
    VectorActionBundle(){};
    ~VectorActionBundle(){};

    bool is_finished() const override;
    void cast_next() override;
    void push_back_cast(const CastInfo &cast);
    void save_data(const String &path, const int visits_threshold) override;
};

#endif