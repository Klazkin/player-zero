#ifndef ACTION_IDENTIFIER_H
#define ACTION_IDENTIFIER_H

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

enum ActionIdentifier
{
    COMBINE_ACTIONS = -2,
    INVALID_ACTION = -1,
    IDLE,
    WRATHSPARK,
    GROUNDRAISE,
    BLOODDRAWING,
    TREAD,
    COILBLADE,
    ETERNALSHACLES,
    ALTAR,
    NETHERSWAP,
    LOS_ACTION,
    DETONATION,
    DEBUG_KILL,
    WISPSPARKS,
    BONEDUST,
    BONESPARKS
};

VARIANT_ENUM_CAST(ActionIdentifier);

#endif