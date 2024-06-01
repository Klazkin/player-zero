#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    INVALID_ACTION = -1,
    TREAD,           // 0 X
    COILBLADE,       // 1 X
    SWIFTARROW,      // 2 X
    ALIGNMENT_LANCE, // 3 X
    DAGGERS,         // 4 X
    WRATHSPARK,      // 5 X
    GROUNDRAISE,     // 6 X
    BLOODDRAWING,    // 7 X
    RESPIRIT,        // 8 X
    COLDUST,         // 9 X
    END_TURN,        // 10 X
    COMBINE_ACTIONS, // 11 X
    SENTRY,          // 12 X
    OBLIVION,        // 13 X
    BLOCK,           // 14 X
    METEORSHATTER,   // 15 X
    ALTAR,           // 16 X
    ARMORCORE,       // 17 X
    BARRAGE,         // 18 X
    IMMOLATION,      // 19 X
    FLARE,           // 20 X
    COLDSPARK,       // 21 X
    SUNDIVE,         // 22 X
    ETERNAL_SHACLES, // 23 X
    LEACH,           // 24 X
    NETHERSWAP,      // 25 X
    HOARFROST,       // 26 X
    RAPID_GROWTH,    // 27 X
    BLESSING,        // 28 X
    SENTRY_STRIKE,   // 29 X
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif