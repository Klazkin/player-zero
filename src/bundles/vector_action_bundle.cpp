#include "vector_action_bundle.h"
#include <godot_cpp/variant/utility_functions.hpp>

bool VectorActionBundle::is_finished() const
{
    return cast_counter >= casts.size();
}

void VectorActionBundle::cast_next()
{
    if (is_finished())
    {
        return;
    }

    CastInfo ci = casts.at(cast_counter++);
    if (Action::_is_castable(ci))
    {
        Action::_cast_action(ci);
        if (ci.caster->is_dead()) // case when sudoku after casting action
        {
            cast_counter = casts.size();
            return;
        }

        if (ci.action != END_TURN && cast_counter >= casts.size())
        {
            UtilityFunctions::printerr("LAST ACTION IN VectorActionBundle WAS NOT END TURN!!");
        }
    }
    else
    {
        warn_not_castable(ci);
    }
}

void VectorActionBundle::push_back_cast(const CastInfo &cast)
{
    casts.push_back(cast);
}

void VectorActionBundle::save_data(const String &path, const int visits_threshold)
{
    // does nothing
}
