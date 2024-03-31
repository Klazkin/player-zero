#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    INVALID_ACTION = 1,
    TREAD,           // 0
    COILBLADE,       // 1
    SWIFTARROW,      // 2
    ALIGNMENT_LANCE, // 3
    WRATHSPARK,      // 4
    GROUNDRAISE,     // 5
    BLOODDRAWING,    // 6
    WISPSPARKS,      // 7
    BONEDUST,        // 8
    RESPIRIT,        // 9
    SNOWMOTES,       // 10
    ETERNALSHACLES,  // 11
    ALTAR,           // 12
    END_TURN,
    COMBINE_ACTIONS,
    NETHERSWAP,
    LOS_ACTION,
    DETONATION,
    DEBUG_KILL,
    BONESPARKS,
    SUNDIVE,
    METEORSHATTER,
    ARMORCORE,
    IMMOLATION,
    ICEPOLE,
    OBLIVION,
    HOARFROST,
    RAPID_GROWTH,
    SUBLIMESTRUCTURE,
    BLESSING
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif