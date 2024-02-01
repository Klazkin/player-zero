#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "surface_element.h"

void register_handlers();
void register_combinations();

bool cast_always(const CastInfo &cast);
bool check_cell_free(const CastInfo &cast);
bool check_cell_taken(const CastInfo &cast);
bool check_unit_only(const CastInfo &cast);
bool check_self_cast(const CastInfo &cast);

void cast_wrathspark(const CastInfo &cast);
void cast_groundraise(const CastInfo &cast);
void cast_tread(const CastInfo &cast);
void cast_swap(const CastInfo &cast);
void cast_coil(const CastInfo &cast);

#endif
