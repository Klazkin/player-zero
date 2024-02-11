#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "status.h"
#include "surface_element.h"
#include "destructible_element.h"

void register_handlers();
void register_combinations();

bool check_always_allow(const CastInfo &cast);
bool check_cell_free(const CastInfo &cast);
bool check_cell_taken(const CastInfo &cast);
bool check_unit_only(const CastInfo &cast);
bool check_self_cast(const CastInfo &cast);
bool check_not_self_cast(const CastInfo &cast);
bool check_is_direction_valid(const CastInfo &cast);
bool check_cast_distance(const CastInfo &cast, int max_distance);
bool check_line_of_sight(const CastInfo &cast);

void cast_nothing(const CastInfo &cast);
void cast_wrathspark(const CastInfo &cast);
void cast_groundraise(const CastInfo &cast);
void cast_tread(const CastInfo &cast);
void cast_swap(const CastInfo &cast);
void cast_detonate(const CastInfo &cast);
void cast_shacle(const CastInfo &cast);

void multicaster(const CastInfo &cast,
                 ActionCheckType local_checker,
                 ActionCastType local_caster,
                 const PackedVector2Array &points,
                 bool is_rotatable = true);

PackedVector2Array generate_coilblade_points();

#endif
