/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CHEWY_EPISODE3_H
#define CHEWY_EPISODE3_H

namespace Chewy {

void switch_room(int16 nr);

void r45_entry(int16 eib_nr);

void r45_exit(int16 eib_nr);

void r45_setup_func();

int16 r45_use_taxi();

void r45_talk_taxi(int16 aad_nr);

void r45_taxi_mov();

void r45_talk_boy();

int16 r45_use_boy();

void r46_entry(int16 eib_nr);

void r46_bodo();

int16 r46_use_schloss();

void r46_kloppe();

void r46_exit();

void r46setup_func();

void r47_entry();

void r47_exit();

int16 r47_use_knopf(int16 txt_nr);

void r47_set_detail();

void r48_entry();

void r48calc_pic();

void r48_frage();
void r48setup_func();

void r49_calc_boy();

void r49_talk_boy();

void r49_talk_boy(int16 aad_nr);

void r49_look_hotel();

int16 r49_use_boy();

void r49_entry(int16 eib_nr);

void r49_exit(int16 eib_nr);
void r49setup_func();

int16 r49_use_taxi();

void r49_calc_boy_ani();

void r50_entry(int16 eib_nr);

void r50_exit(int16 eib_nr);

void r50_stop_cigar();

void r50_calc_wasser();

void r50_calc_treppe();

int16 r50_use_gutschein();

int16 r50_use_gum();

void r50_aad_page(int16 aad_nr, int16 ani_nr);

void r50_stop_page();
void r50setup_func();

void r50_talk_page();

void r51_entry();

void r51_exit(int16 eib_nr);

int16 r51_use_door(int16 txt_nr);
void r51setup_func();
int16 r51_cut_serv(int16 frame);

void r52_entry();

void r52_exit();
void r52setup_func();

int16 r52_use_hot_dog();

void r52_plot_armee(int16 frame);

void r52_kaker_platt();

void r53_entry();

void r53_man_go();

void r53_talk_man();

int16 r53_use_man();

void r54_entry(int16 eib_nr);

void r54_exit(int16 eib_nr);
void r54_setup_func();

int16 r54_use_schalter();

void r54_talk_verkauf();

int16 r54_use_zelle();

int16 r54_use_azug();

int16 r54_use_taxi();

void r54_aufzug_ab();
int16 r54_cut_serv(int16 frame);

void r54_calc_auto_taxi();

void r55_entry();

void r55_exit(int16 eib_nr);
void r55_setup_func();

void r55_strasse(int16 mode);

void r55_talk_line();

int16 r55_use_kammeraus();
int16 r55_cut_serv(int16 frame);

void r55_verleger_mov(int16 mode);

void r55_get_job();

void r55_mans2rock();

void r55_rock2mans();

int16 r55_use_stapel1();

int16 r55_use_stapel2();

int16 r55_use_telefon();

void r56_entry();

void r56_exit();
void r56_setup_func();

int16 r56_use_taxi();

void r56_talk_man();

int16 r56_use_man();

int16 r56_use_kneipe();

void r56_start_flug();

void r57_entry();

void r57_exit(int16 eib_nr);
void r57_setup_func();

int16 r57_use_taxi();

int16 r57_use_pfoertner();

void r57_talk_pfoertner();

void r58_entry();

void r58_exit();

void r58_look_cut_mag(int16 r_nr);
void r58setup_func();

void r59_look_poster();

void r62_entry();
void r62_setup_func();

int16 r62_use_laura();

void r63_entry();

void r63_talk_hunter();

void r63_talk_regie();

void r63_talk_fx_man();

int16 r63_use_fx_man();

int16 r63_use_schalter();

void r63_talk_girl();

int16 r63_use_girl();

int16 r63_use_aschenbecher();

void r63_bork_platt();
void r63_setup_func();

void r64_entry();

void r64_chewy_entry();

void r64_calc_monitor();
void r64_setup_func();

void r64_talk_man();

void r64_talk_man(int16 aad_nr);

int16 r64_use_tasche();
int16 r64_cut_sev(int16 frame);

void r65_entry();

void r65_exit();
void r65_atds_string_start(int16 dia_nr, int16 str_nr,
	int16 person_nr, int16 mode);

} // namespace Chewy

#endif
