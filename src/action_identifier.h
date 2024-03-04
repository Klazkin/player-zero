#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    END_TURN = -3,
    COMBINE_ACTIONS = -2,
    INVALID_ACTION = -1,
    IDLE,           // 0
    WRATHSPARK,     // 1 fire
    GROUNDRAISE,    // 2 ground
    BLOODDRAWING,   // 3 blood, lose health, replenish actions immediately
    TREAD,          // 4
    COILBLADE,      // 5
    ETERNALSHACLES, // 6 shared damage  RESPIRIT + BLOOD
    ALTAR,          // summon altar which buffs allies
    NETHERSWAP,
    LOS_ACTION,
    DETONATION,
    DEBUG_KILL,
    WISPSPARKS,
    BONEDUST,
    BONESPARKS,       // TODO ADD THEM IN BINDINGS
    RESPIRIT,         // X Slow heal over duration of turns
    SNOWMOTES,        //   apply a debuff preventing opponents around you from moving, you cannot move either
    SUNDIVE,          // X teleport (WRATHSPARK + TREAD) -> teleport to neighbor with damage
    METEORSHATTER,    // X deal damage,  if it dies -> damage all around
    ARMORCORE,        // X apply armor status effect, negate 1 damage instance, lowers speed
    IMMOLATION,       // X reduce health by 50%, apply a temp buff, replenish health at the end of the buff
    ICEPOLE,          //   massive single damage instance, pierces resistances
    OBLIVION,         //   deal damage to a nearby targets for missing health of caster
    HOARFROST,        // P target recieves less damage (defence boost), (and attacker gets hit)
    RAPID_GROWTH,     // X immediate heal
    SUBLIMESTRUCTURE, //
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif