#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    INVALID_ACTION = -1,
    TREAD,            // 0
    COILBLADE,        // 1
    SWIFTARROW,       // 2
    ALIGNMENT_LANCE,  // 3
    WRATHSPARK,       // 4
    GROUNDRAISE,      // 5
    BLOODDRAWING,     // 6
    WISPSPARKS,       // 7
    BONEDUST,         // 8
    RESPIRIT,         // 9
    SNOWMOTES,        // 10
    ETERNALSHACLES,   // 11
    ALTAR,            // 12
    END_TURN,         // 13
    COMBINE_ACTIONS,  // 24
    NETHERSWAP,       // 15
    LOS_ACTION,       // 16
    DETONATION,       // 17
    DEBUG_KILL,       // 18
    BONESPARKS,       // 19
    SUNDIVE,          // 20
    METEORSHATTER,    // 21
    ARMORCORE,        // 22
    IMMOLATION,       // 23
    ICEPOLE,          // 24
    OBLIVION,         // 25
    HOARFROST,        // 26
    RAPID_GROWTH,     // 27
    SUBLIMESTRUCTURE, // 28
    BLESSING          // 29
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif