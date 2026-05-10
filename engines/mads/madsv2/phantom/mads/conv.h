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

#ifndef MADS_PHANTOM_MADS_CONV_H
#define MADS_PHANTOM_MADS_CONV_H

#include "common/scummsys.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

enum {
	conv001_speech_talk = 0,
	conv001_continue_abc = 1,
	conv002_questions_one = 2,
	conv001_what_one = 4,
	conv001_yesno_yes = 8,
	conv001_everything_byebye = 10,
	conv001_everything_copycat = 12,
	conv001_speak_byebye = 18,
	conv001_saytwo_1 = 22,
	conv001_instructions_three = 24,
	conv001_point_two_abc = 27,
};

enum {
	conv002_sayone_abc = 1,
	conv002_answers_job = 5,
	conv002_answers_please = 6,
	conv002_answers_house = 8,
	conv002_answers_go_on = 9,
	conv002_answers_prison = 10,
	conv002_answers_building = 11,
	conv002_answers_catacombs = 12,
	conv002_interrogate_mishap = 15,
	conv002_interrogate_chandelier = 16,
	conv002_interrogate_phantom = 19,
	conv002_interrogate_giry = 22,
	conv002_nomore_first = 25,
	conv002_saytwo_abc = 26,
};

enum {
	conv003_bon_jour_hello = 0,
	conv003_first_howdy = 1,
	conv003_second_aloha = 2,
	conv003_name_i_am = 3,
	conv003_name_form = 4,
	conv003_name_oops = 5,
	conv003_adios_byebye = 6,
	conv003_nomore_byebye = 7,
	conv003_questions_three = 10,
	conv003_knowghost_see = 13,
	conv003_knowghost_look = 14,
	conv003_return_dreams = 15,
	conv003_return_she = 16,
	conv003_return_giry = 18,
	conv003_return_byebye = 23,
	conv003_yourself_byebye = 27
};

enum {
	conv004_bon_jour_hello = 0,
	conv004_who_who_are = 1,
	conv004_monsieur_again = 2,
	conv004_name_raoul = 3,
	conv004_name_dear = 4,
	conv004_name_sorry = 5,
	conv004_anything_take = 19,
	conv004_imgone_bye = 25,
	conv004_dontleave_help = 26,
	conv004_promises_continue = 27,
	conv004_promises_return = 28,
	conv004_promises_cant = 29,
	conv004_promises_safe = 30
};

enum {
	conv005_nowhere_nothing = 9,
	conv005_report_adieu = 12,
	conv005_lets_see_b_what_have_b = 14,
	conv005_anyone_b_sign_it_b = 17,
	conv005_crime_missing = 20,
	conv005_rumors_sandbag = 24,
	conv005_rumors_accident = 25,
	conv005_goman_abc = 35,
	conv005_murder_b_aa = 37,
	conv005_scream_help = 41,
	conv005_readbook_two_abc = 42,
	conv005_point_one_b_abc = 44
};

enum {
	conv007_richard_intro_b = 2,
	conv007_daaeb_intro_c = 3,
	conv007_where_pushed = 8,
	conv007_where_killed = 10,
	conv007_badfall_abc = 11,
	conv007_youraoul_abc = 12,
	conv007_kiss_abc = 13,
	conv007_afterkiss_abc = 14,
	conv007_delirious_abc = 16,
	conv007_long_abc = 20,
	conv007_worry_abc = 21,
	conv007_dashing_tuxedo = 22,
	conv007_dashing_london = 23,
	conv007_answers_abc = 24,
	conv007_final_goaway = 25,
	conv007_office_abc = 28,
	conv007_solo_alone = 30,
	conv007_pinch_wait_b_nothing_b = 32,
};

enum {
	conv008_things_two = 4,
	conv008_things_three = 5,
	conv008_actions_b_b = 7,
	conv008_actions_d_d = 13,
	conv008_actions_byebye = 15,
	conv008_tellabout_have = 17,
	conv008_tellabout_rumor = 19,
	conv008_later_final = 20,
	conv008_nomore_first = 21,
	conv008_var_actions_done = 24,
	conv008_var_christine_done = 26,
	conv008_christine_three = 27
};

enum {
	conv009_dialogue_one = 1,
	conv009_dialogue_paint = 2,
	conv009_dialogue_three = 10,
	conv009_lips_abc = 11,
	conv009_wink_abc = 12
};

enum {
	conv010_hasthem_abc = 4,
	conv010_noyoudont_abc = 5,
	conv010_unlock_b_box_5 = 7,
	conv010_gethem_abc = 8,
	conv010_beginning_who = 9,
	conv010_beginning_you = 10,
	conv010_beginning_pardon = 11,
	conv010_nomore_abc = 13,
	conv010_byebye_first = 14,
	conv010_dialogue_three = 16,
	conv010_dialogue_one = 17,
	conv010_ghost_byebye = 19,
	conv010_trance_b_box_5 = 21,
	conv010_box_b_b = 23,
	conv010_box_d_d = 25,
	conv010_box_byebye = 40,
	conv010_ghost_interest = 45,
	conv010_bye_two_solong = 46
};

enum {
	conv011_has_b_ticket = 5,
	conv011_has_abc = 6,
	conv011_enter_b_a = 8,
	conv011_hasnot_abc = 9,
	conv011_boxoffice_abc = 10,
	conv011_enter_it_abc = 12,
	conv011_enjoy_abc = 13,
	conv011_five_nono = 14
};

enum {
	conv012_hello_one = 1,
	conv012_hello_four = 4,
	conv012_byebye_first = 6,
	conv012_questions_three = 7,
	conv012_questions_one = 8,
	conv012_seen_byebye = 10,
	conv012_seen_mask = 12,
	conv012_tell_byebye = 19,
	conv012_var_questions_done = 26,
	conv012_tell_knewhim = 29,
	conv012_nomore_first = 30
};

enum {
	conv013_intro_hello = 0,
	conv013_returned_abc = 1,
	conv013_again_abc = 2,
	conv013_dreams_romantic = 19,
	conv013_lovescene_openline = 21,
	conv013_practice_first = 27,
	conv013_biteme_b_b = 31
};

enum {
	conv014_second_angel = 1,
	conv014_eighth_inside = 7
};

enum {
	conv015_daae_b_b = 5,
	conv015_next_second = 7,
	conv015_professional_fourth = 9,
	conv015_damned_b_b = 11,
	conv015_exchange_ticket = 13,
	conv015_exchange_relief = 14,
	conv015_exchange_opera = 15
};

enum {
	conv016_buy_four = 4,
	conv016_take_b_b = 6,
	conv016_kind_five = 12,
	conv016_money_cash = 13,
	conv016_money_lend = 14
};

enum {
	conv017_ticket_block = 0,
	conv017_hasit_first = 3,
	conv017_hasnot_first = 4
};

enum {
	conv018_begin_first = 0,
	conv018_adieu_bye = 4,
	conv018_nomore_huh = 5
};

enum {
	conv019_talk_b_b = 2,
	conv019_exit_b_b = 12
};

enum {
	conv020_where_a = 2,
	conv020_exit_b_b = 5,
	conv020_dialogue_b_b = 8,
	conv020_bye_b_b = 14,
	conv020_story_b_b = 22
};

enum {
	conv021_fight_b_b = 3,
	conv021_ending_b_b = 8,
	conv021_outtahere_first = 10,
	conv021_outtahere_second = 11,
	conv021_outtahere_third = 12,
	conv021_hasit_b_b = 14,
	conv021_score_abc = 15,
	conv021_score_b_b = 17,
};

enum {
	conv022_second_next = 1,
	conv022_resolution_florent = 6,
	conv022_resolution_fall = 7,
	conv022_resolution_alone = 8,
	conv022_dead_strangled = 9,
	conv022_object_abc = 13,
	conv022_do_abc = 15,
	conv022_kiss_b_b = 17,
	conv022_kiss_abc = 18,
	conv022_notes_hadany = 19,
	conv022_notes_here = 20,
	conv022_notes_chandelier = 21,
	conv022_notes_audience = 22,
	conv022_look_abc = 23,
	conv022_notes_b_b = 25,
	conv022_leave_b_b = 29
};

enum {
	conv023_unhand_b_b = 3,
	conv023_okay_b_b = 6,
	conv023_okay_abc = 7,
	conv023_die_b_b = 12
};

enum {
	conv027_choices_one = 2,
	conv027_choices_two = 3,
	conv027_choices_three = 4,
	conv027_choices_four = 5,
	conv027_choices_five = 6,
	conv027_exit_a_a = 7
};

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
