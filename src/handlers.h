#ifndef HANDLERS_H
#define HANDLERS_H

#include "action.h"
#include "status.h"
#include "surface_element.h"
#include "destructible_element.h"
#include <vector>

using namespace std;

void register_handlers();
void register_combinations();

bool check_always_allow(const CastInfo &cast);
bool check_cell_free(const CastInfo &cast);
bool check_cell_taken(const CastInfo &cast);
bool check_unit_only(const CastInfo &cast);
bool check_ally_unit_only(const CastInfo &cast);
bool check_self_cast(const CastInfo &cast);
bool check_not_self_cast(const CastInfo &cast);
bool check_is_direction_valid(const CastInfo &cast);
bool check_cast_distance(const CastInfo &cast, int max_distance);
bool check_line_of_sight(const CastInfo &cast);
bool check_action_combination(const CastInfo &cast);
bool check_free_near_unit(const CastInfo &cast);

void cast_nothing(const CastInfo &cast);
void cast_wrathspark(const CastInfo &cast);
void cast_groundraise(const CastInfo &cast);
void cast_tread(const CastInfo &cast);
void cast_swap(const CastInfo &cast);
void cast_detonate(const CastInfo &cast);
void cast_shacle(const CastInfo &cast);
void cast_debug_kill(const CastInfo &cast);
void cast_wispsparks(const CastInfo &cast);
void cast_bonedust(const CastInfo &cast);
void cast_bonesparks(const CastInfo &cast);
void cast_altar(const CastInfo &cast);
void cast_combine_actions(const CastInfo &cast);
void cast_end_trun(const CastInfo &cast);
void cast_respirit(const CastInfo &cast);
void cast_rapid_growth(const CastInfo &cast);
void cast_immolation(const CastInfo &cast);
void cast_armorcore(const CastInfo &cast);
void cast_sundive(const CastInfo &cast);
void cast_blooddrawing(const CastInfo &cast);
void cast_meteorshatter(const CastInfo &cast);
void cast_hoarfrost(const CastInfo &cast);
void cast_coilblade_singular(const CastInfo &cast);

void multicaster(const CastInfo &cast,
                 ActionCheckType local_checker,
                 ActionCastType local_caster,
                 const vector<Vector2i> &points,
                 bool is_rotatable = true);

std::vector<CastInfo> gen_self_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_closest_free_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_all_elements_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_all_units_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_all_other_units_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_all_units_with_checker(const CastInfo &initial_info);
std::vector<CastInfo> gen_4direction_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_tread_cast(const CastInfo &initial_info);
std::vector<CastInfo> gen_action_combinations(const CastInfo &initial_info);
std::vector<CastInfo> gen_free_near_every_unit(const CastInfo &initial_info);

#endif
