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
        if (ci.caster->is_dead()) // case when sudoku after casting action
        {
            cast_counter = casts.size();
            return true;
        }

        if (ci.action == END_TURN)
        {
            return true;
        }

        if (cast_counter == casts.size())
        {
            UtilityFunctions::printerr("LAST ACTION IN ACTION_BUNDLE WAS NOT END TURN!!");
        }
    }
    else
    {
        UtilityFunctions::printerr("CRITICAL: Did not pass cast check");
        for (auto cj : casts)
        {
            std::cout << '\t' << cj.action << "\n";
        }

        UtilityFunctions::printerr(ci.action);
        UtilityFunctions::printerr(ci.caster->get_position());
        UtilityFunctions::printerr(ci.target);

        if (!as_unit_ptr(ci.caster)->is_in_hand(ci.action))
        {
            UtilityFunctions::printerr("Action not found in hand.");
        }

        if (ci.surface->get_winner() != UNDEFINED)
        {
            UtilityFunctions::printerr("Surface has a winner.");
        }

        if (ci.caster->is_dead())
        {
            UtilityFunctions::printerr("Caster is dead.");
        }

        if (ci.surface->turn_get_current_unit() != ci.caster)
        {
            UtilityFunctions::printerr("Caster is not current turn unit.");
        }

        int end_turn_c = 0;
        for (auto c : casts)
        {
            if (c.action == END_TURN)
                end_turn_c++;
        }

        if (end_turn_c > 1)
        {
            UtilityFunctions::printerr("More than 1 END_TURN cast.");
        }

        // Action::_cast_action({END_TURN, ci.surface, ci.caster, Vector2i()});
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