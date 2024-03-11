#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    END_TURN = -3,
    COMBINE_ACTIONS = -2,
    INVALID_ACTION = -1,
    IDLE,             // 0
    WRATHSPARK,       // 1 fire
    GROUNDRAISE,      // 2 ground
    BLOODDRAWING,     // 3 blood, lose health, replenish actions immediately
    TREAD,            // 4
    COILBLADE,        // 5
    ETERNALSHACLES,   // 6 shared damage  RESPIRIT + BLOOD
    ALTAR,            // 7 summon altar which buffs allies
    NETHERSWAP,       // 8
    LOS_ACTION,       // 9
    DETONATION,       // 10
    DEBUG_KILL,       // 11
    WISPSPARKS,       // 12
    BONEDUST,         // 13
    BONESPARKS,       // 14 TODO ADD THEM IN BINDINGS
    RESPIRIT,         // 15 X Slow heal over duration of turns
    SNOWMOTES,        // 16  apply a debuff preventing opponents around you from moving, you cannot move either
    SUNDIVE,          // 17 X teleport (WRATHSPARK + TREAD) -> teleport to neighbor with damage
    METEORSHATTER,    // 18 X deal damage,  if it dies -> damage all around
    ARMORCORE,        // 19 X apply armor status effect, negate 1 damage instance, lowers speed
    IMMOLATION,       // 20 X reduce health by 50%, apply a temp buff, replenish health at the end of the buff
    ICEPOLE,          // 21 massive single damage instance, pierces resistances
    OBLIVION,         // 22 deal damage to a nearby targets for missing health of caster
    HOARFROST,        // 23 P target recieves less damage (defence boost), (and attacker gets hit)
    RAPID_GROWTH,     // 24 X immediate heal
    SUBLIMESTRUCTURE, // 25
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif