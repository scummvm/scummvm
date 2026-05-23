/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MADS_DRAGONSPHERE_MADS_CONV_H
#define MADS_DRAGONSPHERE_MADS_CONV_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Dragonsphere {

// Conversation node IDs are added here as each conversation is ported.
// Each conversation gets its own enum block named after its .CON file.

enum {
	conv002_counter_only = 0,
	conv002_banter_random = 1,
	conv002_flirt_rand = 2
};

enum {
	conv003_replies_b_b = 2,
	conv003_exit_b_b = 8,
	conv003_replies_defeat = 5
};

enum {
	conv004_seen_only = 0,
	conv004_resolved_only = 1,
	conv004_am_only = 2,
	conv004_wait_only = 3,
	conv004_thanks_only = 4,
	conv004_nay_only = 5,
	conv004_queen_only = 6,
	conv004_fathers_only = 7,
	conv004_last_only = 8,
	conv004_exit_b_b = 10
};

enum {
	conv005_exit_b_b = 16
};

enum {
	conv006_wares_greeting   = 0,
	conv006_choices_chat     = 2,
	conv006_choices_sorry    = 3,
	conv006_exit_b_b         = 5,
	conv006_how_only         = 6,
	conv006_thanks_only      = 7,
	conv006_wise_only        = 8,
};

enum {
	conv008_query_wish       =  3,
	conv008_query_respect    =  4,
	conv008_query_norespect  =  5,
	conv008_query_humans     =  6,
	conv008_query_why_annoy  =  7,
	conv008_query_king       =  8,
	conv008_query_where      =  9,
	conv008_banter_only      = 11
};

enum {
	conv009_hello1a_only     = 1,
	conv009_hello2a_only     = 3,
	conv009_choices_trip     = 4,
	conv009_choices_never    = 5,
	conv009_choices_someday  = 6,
	conv009_exiter_leave     = 8
};

enum {
	conv010_hold_only  =  3,
	conv010_leave_b_b  =  6,
	conv010_fix_b_b    = 22,
	conv010_die_b_b    = 31
};

enum {
	conv012_gash_b_b   = 14
};

enum {
	conv014_heehee_only      = 0,
	conv014_query_wish       = 3,
	conv014_query_respect    = 4,
	conv014_query_humans     = 6,
	conv014_query_why_annoy  = 7,
	conv014_banter_only      = 8
};

enum {
	conv015_seeher_show      =  0,
	conv015_seeher_who       =  1,
	conv015_seeher_comeout   =  2,
	conv015_answer_go        =  6,
	conv015_react_gohome     =  9,
	conv015_gift3_b_b        = 19,
	conv015_kiss_b_b         = 27,
	conv015_exit_b_b         = 32
};

enum {
	conv017_exit_b_b    = 4,
	conv016_exit_b_b    = 5,
};

enum {
	conv018_exit_b_b    = 4
};

enum {
	conv019_exit_b_b    = 4
};

enum {
	conv020_exit_b_b    = 4
};

enum {
	conv021_exit_b_b    = 4
};

enum {
	conv022_exit_b_b    = 4
};

enum {
	conv023_exit_b_b    = 6
};

enum {
	conv026_last_one     = 10,
	conv026_exit_b_b     = 17
};

enum {
	conv027_hello_only       =  0,
	conv027_seven_only       =  2,
	conv027_stopper_monk     =  4,
	conv027_explain_one      =  6,
	conv027_next_one         =  7,
	conv027_last_one         =  8,
	conv027_quickie_b_b      = 12,
	conv027_quickie_only     = 13,
	conv027_hem_only         = 14,
	conv027_one_only         = 15,
	conv027_two_only         = 16,
	conv027_four_only        = 19,
	conv027_inter2_only      = 21,
	conv027_amulet_only      = 22,
	conv027_inter2_b_b       = 24,
	conv027_six_only         = 25,
	conv027_amulet_b_b       = 27,
	conv027_eight_only       = 28,
	conv027_kiss_b_b         = 30,
	conv027_nogood_hidewhat  = 31,
	conv027_exit_b_b         = 34,
	conv027_restart          = 35,
	conv027_restart_only     = 35,
	conv027_ten_only         = 36
};

enum {
	conv028_didnot_innocent = 9,
	conv028_exit_b_b        = 26
};

enum {
	conv031_hello_only   =  0,
	conv031_whoare_only  =  1,
	conv031_wellwell_b_b =  3,
	conv031_wellwell     =  4,
	conv031_exit_b_b     = 11
};

enum {
	conv032_pid_dies_b_b   =  5,
	conv032_pain_b_b       = 14,
	conv032_pain           = 15,
	conv032_pid_death_b_b  = 20
};

enum {
	conv033_six_only     =  4,
	conv033_exit_b_b     =  7
};

enum {
	conv034_five_b_b    =  4,
	conv034_seven_only  =  7,
	conv034_eight_b_b   =  9,
	conv034_eight_only  = 10,
	conv034_nine_only   = 11,
	conv034_final_only  = 15,
	conv034_exit_a_a    = 16,
	conv034_exit_b_b    = 17
};

enum {
	conv035_exit_b_b    =  3
};

enum {
	conv036_question2_b_b    =  7,
	conv036_question3_b_b    = 15,
	conv036_die_b_b          = 24,
	conv036_ok_b_b           = 26
};

enum {
	conv037_next_b_b         =  2,
	conv037_exit_b_b         =  5
};

enum {
	conv038_third_yes        =  2,
	conv038_third_idont      =  4,
	conv038_third_blab       =  5,
	conv038_knowledge_yes    = 12,
	conv038_knowledge_no     = 13,
	conv038_knowledge_idont  = 14,
	conv038_fourth_nound     = 21
};

enum {
	conv039_greeting_only    =  0,
	conv039_choices_b_b      =  2,
	conv039_gift_b_b         = 19,
	conv039_pre_addon_b_b    = 30,
	conv039_exit_b_b         = 36,
	conv039_exit_d_d         = 38
};

enum {
	conv040_enter_b_b        =  4,
	conv040_repeater         =  7,
	conv040_leave_b_b        =  8
};

enum {
	conv041_indance          = 17,
	conv041_postdanc         = 21,
	conv041_postgame         = 24,

	conv041_danceyn_no       = 15,
	conv041_to_game_b_b      = 17,
	conv041_to_game_d_d      = 19,
	conv041_dance_b_b        = 24,
	conv041_postdance_b_b    = 35,
	conv041_postdanc_only    = 36,
	conv041_leaving          = 41,
	conv041_give_stuff_a_a   = 42,
	conv041_give_stuff_b_b   = 43,
	conv041_give_stuff_f_f   = 45,
	conv041_give_stuff_c_c   = 46,
	conv041_give_stuff_d_d   = 47,
	conv041_exit_b_b         = 64,
	conv041_exit_d_d         = 66,
	conv041_exit_f_f         = 68
};

enum {
	conv042_pre_poem         =  0,
	conv042_one_first        =  1,
	conv042_one_second       =  2,
	conv042_one_third        =  3,
	conv042_one_restart      =  4,
	conv042_one_abort        =  5,

	conv042_two_first        =  6,
	conv042_two_second       =  7,
	conv042_two_third        =  8,
	conv042_two_restart      =  9,
	conv042_two_abort        = 10,

	conv042_three_first      = 11,
	conv042_three_second     = 12,
	conv042_three_third      = 13,
	conv042_three_restart    = 14,
	conv042_three_abort      = 15,

	conv042_four_first       = 16,
	conv042_four_second      = 17,
	conv042_four_third       = 18,
	conv042_four_restart     = 19,
	conv042_four_abort       = 20,

	conv042_five_first       = 21,
	conv042_five_second      = 22,
	conv042_five_third       = 23,
	conv042_five_restart     = 24,
	conv042_five_abort       = 25,

	conv042_six_first        = 26,
	conv042_six_second       = 27,
	conv042_six_third        = 28,
	conv042_six_restart      = 29,
	conv042_six_abort        = 30,

	conv042_seven_first      = 31,
	conv042_seven_second     = 32,
	conv042_seven_third      = 33,
	conv042_seven_restart    = 34,
	conv042_seven_abort      = 35,

	conv042_eight_first      = 36,
	conv042_eight_second     = 37,
	conv042_eight_third      = 38,
	conv042_eight_restart    = 39,
	conv042_eight_abort      = 40,

	conv042_nine_first       = 41,
	conv042_nine_second      = 42,
	conv042_nine_third       = 43,
	conv042_nine_restart     = 44,
	conv042_nine_abort       = 45,

	conv042_ten_first        = 46,
	conv042_ten_second       = 47,
	conv042_ten_third        = 48,
	conv042_ten_restart      = 49,
	conv042_ten_abort        = 50,

	conv042_eleven_first     = 51,
	conv042_eleven_second    = 52,
	conv042_eleven_third     = 53,
	conv042_eleven_restart   = 54,
	conv042_eleven_abort     = 55,

	conv042_twelve_first     = 56,
	conv042_twelve_second    = 57,
	conv042_twelve_third     = 58,
	conv042_twelve_restart   = 59,
	conv042_twelve_abort     = 60,

	conv042_thirteen_first   = 61,
	conv042_thirteen_second  = 62,
	conv042_thirteen_third   = 63,
	conv042_thirteen_restart = 64,
	conv042_thirteen_abort   = 65,

	conv042_fourteen_first   = 66,
	conv042_fourteen_second  = 67,
	conv042_fourteen_third   = 68,
	conv042_fourteen_restart = 69,
	conv042_fourteen_abort   = 70,

	conv042_easy_14_first    = 71,
	conv042_easy_14_second   = 72,
	conv042_easy_14_third    = 73,
	conv042_easy_14_restart  = 74,
	conv042_easy_14_abort    = 75,

	conv042_fifteen_first    = 76,
	conv042_fifteen_second   = 77,
	conv042_fifteen_third    = 78,
	conv042_fifteen_restart  = 79,
	conv042_fifteen_abort    = 80,

	conv042_sixteen_first    = 81,
	conv042_sixteen_second   = 82,
	conv042_sixteen_third    = 83,
	conv042_sixteen_restart  = 84,
	conv042_sixteen_abort    = 85,

	conv042_seventeen_first  = 86,
	conv042_seventeen_second = 87,
	conv042_seventeen_third  = 88,
	conv042_seventeen_restart= 89,
	conv042_seventeen_abort  = 90,

	conv042_exit_b_b         = 93,
	conv042_later_b_b        = 94,
	conv042_whoops_b_b       = 96,
};

enum {
	conv043_greeting         =  0,
	conv043_restart          =  4,
	conv043_homenow          = 14,
	conv043_rise_b_b         = 17,
	conv043_rise_d_d         = 19,
	conv043_exit_b_b         = 21,
	conv043_exit_d_d         = 23
};

enum {
	conv044_repeater         =  0,
	conv044_greet            =  2,
	conv044_leave_b_b        =  7,
	conv044_enter_b_b        =  9
};

enum {
	conv045_egaming_only     =  4,
	conv045_hgaming_only     =  5,
	conv045_to_game_b_b      =  9,
	conv045_giver            = 11,
	conv045_exit_b_b         = 11,
	conv045_give_stuff_a_a   = 14,
	conv045_give_stuff_b_b   = 15,
	conv045_moredrink_b_b    = 20,
	conv045_passout_b_b      = 23,
	conv045_passout_e_e      = 26,
	conv045_revive           = 28,
	conv045_timer_b_b        = 29
};

enum {
	conv046_advicelp         =  1,
	conv046_advicelp_bynow   =  2,
	conv046_advicelp_seeya   =  4,
	conv046_exit_b_b         =  6
};

enum {
	conv047_protect          =  0,
	conv047_kingsay_escort   =  3,
	conv047_kingsay_why      =  4,
	conv047_exit_b_b         =  7,
	conv047_exit_k_k         =  9,
	conv047_myguys_whydirt   = 10,
	conv047_postbribe        = 12,
	conv047_exit_i_i         = 15,
	conv047_prebribe_giver   = 17,
	conv047_give_b_b         = 20,
	conv047_reaction2_only   = 23,
	conv047_offer_never      = 24,
	conv047_askback          = 24,
	conv047_offer_boon       = 25,
	conv047_offer_wasnt      = 26,
	conv047_boon2_only       = 27,
	conv047_boon3_suppose    = 29,
	conv047_usesame          = 29,
	conv047_take_back_b_b    = 31,
	conv047_heythere         = 32,
	conv047_collat2_only     = 33,
	conv047_exit_d_d         = 35,
	conv047_askback_heythere = 36,
	conv047_exit_f_f         = 40,
	conv047_kill_b_b         = 42,
	conv047_give_d_d         = 45,
	conv047_heythere_only    = 46,
	conv047_take_back_d_d    = 48
};

enum {
	conv049_recheck          =  1,
	conv049_exit_b_b         = 37,
	conv049_polyquiz_b_b     = 33
};

enum {
	conv050_exit_b_b         =  8,
	conv050_branch           =  9,
	conv050_answers_yes      = 16,
	conv050_answers_rulesask = 18,
	conv050_exit_d_d         = 20
};

enum {
	conv051_nopass           = 0,
	conv051_exit_b_b         = 3
};

enum {
	conv052_nopass           = 0,
	conv052_exit_b_b         = 5
};

enum {
	conv053_story_b_b        =  3,
	conv053_focus_b_b        =  9,
	conv053_thanks_b_b       = 12,
	conv053_thanks_only      = 13,
	conv053_restart_only     = 14
};

enum {
	conv054_restart          =  1,
	conv054_exit_b_b         = 10,
	conv054_show_drink_b_b   = 21,
	conv054_death_b_b        = 25,
	conv054_take_swig_b_b    = 29,
	conv054_offer_b_b        = 33
};

enum {
	conv055_greet_only       =  0,
	conv055_restart          =  8,
	conv055_exit_b_b         = 13
};

enum {
	conv056_ralphre2_permit  =  2
};

enum {
	conv057_startquiz_only   =  0,
	conv057_startquiz        =  0,
	conv057_newquiz          = 25,
	conv057_exit_b_b         = 29,
	conv057_respons_b_b      = 33,
	conv057_restart          = 39,
	conv057_exit_d_d         = 51,
	conv057_exit_f_f         = 59,
	conv057_exit_h_h         = 61
};

} // namespace Dragonsphere
} // namespace MADSV2
} // namespace MADS

#endif
