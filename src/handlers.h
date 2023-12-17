#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "surface_element.h"

void register_handlers();
void register_combinations();

namespace Wrathspark
{
    static bool check(CastInfo cast_info);
    static void cast(CastInfo cast_info);
}

#endif
