#include "action_bundle.h"
#include <godot_cpp/variant/utility_functions.hpp>

void ActionBundle::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("is_finished"), &ActionBundle::is_finished);
    ClassDB::bind_method(D_METHOD("cast_next"), &ActionBundle::cast_next);
}

bool ActionBundle::is_finished() const
{
    return true;
}

void ActionBundle::cast_next()
{
    std::cout << "Attempting to cast on abstract ActionBundle.\n";
    if (is_finished())
    {
        return;
    }
}

void ActionBundle::warn_not_castable(const CastInfo &cast) const
{
    UtilityFunctions::printerr("CRITICAL: Did not pass cast check in action bundle.");
    UtilityFunctions::printerr(cast.action);
    UtilityFunctions::printerr(cast.caster->get_position());
    UtilityFunctions::printerr(cast.target);

    if (!as_unit_ptr(cast.caster)->is_in_hand(cast.action))
    {
        UtilityFunctions::printerr("Action not found in hand.");
    }

    if (cast.surface->get_winner() != UNDEFINED)
    {
        UtilityFunctions::printerr("Surface has a winner.");
    }

    if (cast.caster->is_dead())
    {
        UtilityFunctions::printerr("Caster is dead.");
    }

    if (cast.surface->turn_get_current_unit() != cast.caster)
    {
        UtilityFunctions::printerr("Caster is not current turn unit.");
    }
}
