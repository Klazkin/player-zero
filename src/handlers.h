#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "surface_element.h"

void register_handlers();
void register_combinations();

bool check_always_allow(const CastInfo &cast);
bool check_cell_free(const CastInfo &cast);
bool check_cell_taken(const CastInfo &cast);
bool check_unit_only(const CastInfo &cast);
bool check_self_cast(const CastInfo &cast);
bool check_is_direction_valid(const CastInfo &cast);

void cast_wrathspark(const CastInfo &cast);
void cast_groundraise(const CastInfo &cast);
void cast_tread(const CastInfo &cast);
void cast_swap(const CastInfo &cast);
void multicaster(
    ActionCheckType p_local_checker,
    ActionCastType p_local_caster,
    const PackedVector2Array &p_points,
    bool p_is_rotatable,
    const CastInfo &cast);

PackedVector2Array generate_coilblade_points();

#endif
