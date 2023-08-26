#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "surface_element.h"

static void register_handlers();

namespace Wrathspark
{
    static bool check(CastInfo cast_info);
    static void cast(CastInfo cast_info);
}

#endif
