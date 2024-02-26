#include <action_bundle.h>
#include <godot_cpp/variant/utility_functions.hpp>

void ActionBundle::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("cast_until_finished"), &ActionBundle::cast_until_finished);
}

ActionBundle::ActionBundle()
{
    std::vector<CastInfo> casts;
}

ActionBundle::~ActionBundle()
{
}

bool ActionBundle::cast_until_finished()
{
    if (cast_counter >= casts.size())
    {
        return true;
    }

    CastInfo ci = casts.at(cast_counter++);

    if (Action::_is_castable(ci))
    {
        Action::_cast_action(ci);

        if (ci.action != END_TURN && cast_counter == casts.size())
        {
            UtilityFunctions::printerr("LAST ACTION IN ACTION_BUNDLE WAS NOT END TURN!!");
        }
    }
    else
    {
        UtilityFunctions::printerr("Did not pass cast check");
        UtilityFunctions::printerr(ci.action);
        UtilityFunctions::printerr(ci.target);
    }

    return cast_counter >= casts.size();
}

std::vector<CastInfo> ActionBundle::get_casts() const
{
    return casts;
}

void ActionBundle::push_back_cast(const CastInfo &cast)
{
    casts.push_back(cast);
}