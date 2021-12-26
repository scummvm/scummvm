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

#ifndef CHEWY_EPISODE1_H
#define CHEWY_EPISODE1_H

namespace Chewy {

void switch_room(int16 nr);

void r2_jump_out_r1(int16 nr);

void r3_terminal();

void r3_init_sonde();

void r3_sonde_knarre();

void r3_sonde_aufnahme();

int16 r4_sonde_comp();

void r5_knopf();

void r6_entry();

void r6_init_robo();

void r6_init_robo1();

void r6_bola_knopf();

int16 r6_cut_serv1(int16 frame);
int16 r6_cut_serv2(int16 frame);

void r7_haken(int16 sib_nr);

void r7_klingel();

void r8_entry();

void r8_start_folter();

void r8_stop_folter();

void r8_hole_kohle();

void r8_start_verbrennen();

bool r8_gips_wurf();

void r8_open_gdoor();

void r8_talk_nimoy();

void r9_entry();

void r9_gtuer();

void r9_surimy();

void r9_surimy_ani();

void r10_entry();
void r10_get_surimy();

void r11_entry();

int16 r11_scanner();

void r11_terminal();

void r11_bork_zwinkert();

void r11_talk_debug();

void r11_get_card();

void r11_put_card();

void r12_entry();

void r12_init_bork();

void r12_talk_bork();

void r12_bork_talk_not_ok();

int16 r12_use_terminal();

int16 r12_use_linke_rohr();

int16 r12_chewy_trans();

int16 r12_proc1();

void r11_chewy_bo_use();

void r12_bork_ok();

int16 r12_cut_serv(int16 frame);

void r13_entry();

void r13_exit();

void r13_talk_bork();

void r13_jmp_band();

void r13_jmp_boden();

int16 r13_monitor_knopf();

void r14_entry();

void r14_eremit_feuer(int16 t_nr, int16 ani_nr);

int16 r14_use_schrott();

int16 r14_use_schleim();

int16 r14_use_gleiter();

void r14_talk_eremit();

void r14_feuer();

void r16_entry();

int16 r16_use_gleiter();

void r17_entry();

void r17_xit();

void r17_calc_seil();

void r17_plot_seil();

int16 r17_use_seil();

void r17_kletter_down();

void r17_kletter_up();

void r17_door_kommando(int16 mode);

void r17_close_door();

int16 r17_energie_hebel();

int16 r17_get_oel();

void r18_entry();

void r18_init_borks();

void r18_timer_action(int16 t_nr);

int16 r18_calc_surimy();

int16 r18_calc_schalter();

void r18_monitor();

int16 r18_go_cyberspace();

int16 r18_sonden_moni();
int16 r18_use_cart_moni();

void r19_entry();

void r21_entry();

void r21_calc_laser();

void r21_init_spinne();

void r21_restart_spinne2();

void r21_e_streifen();

void r21_chewy_kolli();

void r21_salto();

void r21_use_gitter_energie();

int16 r21_use_fenster();
void r21setup_func();

void r22_entry();

void r22_bork(int16 t_nr);

int16 r22_chewy_amboss();

void r22_bork_walk1();

void r22_bork_walk2();

void r22_get_bork();

int16 r22_malen();

void r23_cockpit();

int16 r23_start_gleiter();

void r23_use_cartridge();

void r23_get_cartridge();

void r24_entry();

void r24_exit();

void r24_use_hebel(int16 txt_nr);

void r24_calc_hebel_spr();

void r24_calc_animation(int16 kristall_nr);

} // namespace Chewy

#endif
