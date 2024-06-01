#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    INVALID_ACTION = -1,
    TREAD,           // 0
    COILBLADE,       // 1
    SWIFTARROW,      // 2
    ALIGNMENT_LANCE, // 3
    DAGGERS,         // 4
    WRATHSPARK,      // 5
    GROUNDRAISE,     // 6
    BLOODDRAWING,    // 7
    RESPIRIT,        // 8
    COLDUST,         // 9
    END_TURN,        // 10
    COMBINE_ACTIONS, // 11
    SENTRY,          // 12
    OBLIVION,        // 13
    BLOCK,           // 14
    METEORSHATTER,   // 15
    ALTAR,           // 16
    ARMORCORE,       // 17
    BARRAGE,         // 18
    IMMOLATION,      // 19
    FLARE,           // 20
    COLDSPARK,       // 21
    SUNDIVE,         // 22
    ETERNAL_SHACLES, // 23
    LEACH,           // 24
    NETHERSWAP,      // 25
    HOARFROST,       // 26
    RAPID_GROWTH,    // 27
    BLESSING,        // 28
    SENTRY_STRIKE,   // 29
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif