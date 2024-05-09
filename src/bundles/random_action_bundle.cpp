#include "random_action_bundle.h"
#include <godot_cpp/variant/utility_functions.hpp>

bool RandomActionBundle::is_finished() const
{
    return is_forced_to_finish || caster->is_dead() || surface->get_winner() != UNDEFINED;
}

void RandomActionBundle::cast_next()
{
    if (is_finished())
    {
        return;
    }

    std::vector<CastInfo> candidate_casts;
    for (auto action : caster->get_hand_set())
    {
        std::vector<CastInfo> action_casts = Action::generate_action_casts({action, surface, caster, caster->get_position()});
        candidate_casts.insert(candidate_casts.end(), action_casts.begin(), action_casts.end());
    }

    if (candidate_casts.size() == 0)
    {
        Action::_cast_action({END_TURN, surface, caster, Vector2i()});
        is_forced_to_finish = true;
        return;
    }

    CastInfo random_cast = candidate_casts[UtilityFunctions::randi_range(0, candidate_casts.size() - 1)];
    if (Action::_is_castable(random_cast))
    {
        Action::_cast_action(random_cast);
        if (random_cast.action == END_TURN)
        {
            is_forced_to_finish = true;
            return;
        }
    }
    else
    {
        warn_not_castable(random_cast);
    }
}
