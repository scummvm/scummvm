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

#ifndef CHEWY_EPISODE2_H
#define CHEWY_EPISODE2_H

namespace Chewy {

void switch_room(int16 nr);
void r23_cockpit();

void r25_entry();

int16 r25_gleiter_loesch();

int16 r25_use_gleiter();

void r25_xit_gleiter();

void r26_entry();

void r27_entry();

void r27_get_surimy();

void r27_talk_howard();

void r27_exit(int16 eib_nr);

void r28_entry(int16 eib_nr);

void r28_use_surimy();

void r28_set_pump();

void r28_get_pump();

void r28setup_func();

void r28_exit(int16 eib_nr);

int16 r28_use_breifkasten();

void r28_haendler();

int16 r28_cut_serv(int16 frame);

int16 r28_cut_serv2(int16 frame);

void r29_entry();

void r29_exit();

int16 r29_use_pumpe();

int16 r29_get_schlauch();

bool r29_use_schlauch();

void r29_schlitz_sitz();

int16 r29_zaun_sprung();

void r31_entry();

void r31_calc_luke();

void r31_open_luke();

void r31_close_luke();

int16 r31_use_topf();

void r31_surimy_go();

void r32_entry();

int16 r32_use_howard();

void r32_use_schreibmaschine();

int16 r32_get_script();

void r33_entry();

void r33_look_schublade();

int16 r33_use_schublade();

void r33_use_maschine();

int16 r33_calc_muntermacher();

short r33_get_munter();

void r33_surimy_go();

bool r34_use_kuehlschrank();

void r34_xit_kuehlschrank();

void r35_entry();

int16 r35_schublade();

int16 r35_use_cat();

void r35_talk_cat();

void r37_entry();

int16 r37_use_wippe();

int16 r37_cut_serv1(int16 frame);

int16 r37_cut_serv2(int16 frame);

void r37_dog_bell();

int16 r37_use_glas();

void r37_talk_hahn();

void r37_use_hahn();

void r37_hahn_dia();
void r37_setup_func();

int16 r39_use_tv();

void r39_look_tv(int16 cls_mode);

void r39_set_tv();

void r39_entry();

void r39_entry();

int16 r39_use_howard();

void r39_talk_howard();

void r39_ok();

void r40_entry(int16 eib_nr);

void r40_exit(int16 eib_nr);

int16 r40_use_mr_pumpkin();

void r40_talk_handler();

int16 r40_use_haendler();

void r40_talk_police();

int16 r40_use_schalter(int16 aad_nr);

void r40_setup_func();

void r40_move_train(int16 mode);

int16 r40_use_bmeister();

bool r40_use_police();

int16 r40_use_tele();

void r41_entry();

void r41_exit();

void r41_talk_hoggy1();

void r41_talk_hoggy2();

void r41_first_talk();

int16 r41_use_kasse();

void r41_start_hoggy();

void r41_stop_hoggy();

int16 r41_use_lola();

int16 r41_use_brief();

void r41_sub_dia();

void r40_bmeister_dia(int16 aad_nr);

void r42_entry();

int16 r42_use_psack();

void r42_talk_beamter();

void r42_dia_beamter(int16 str_end_nr);

void r42_calc_xit();

int16 r42_use_beamter();

void r42setup_func();

void r42_get_kuerbis(int16 aad_nr);

void r43_night_small();

void r43_catch_pg();

void r44_look_news();

} // namespace Chewy

#endif
