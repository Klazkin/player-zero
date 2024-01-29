#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "surface_element.h"

void register_handlers();
void register_combinations();

/*
GroundRaise
BloodDrawing
Tread
CoilBlade
Wrathspark
EternalShacles
Altar
NetherSwap
*/

bool check_cell_free(CastInfo &cast);
bool check_cell_taken(CastInfo &cast);

void cast_wrathspark(CastInfo &cast);
void cast_groundraise(CastInfo &cast);

#endif
