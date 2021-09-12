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

#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
//define FORBIDDEN_SYMBOL_EXCEPTION_fgetc
//#define FORBIDDEN_SYMBOL_EXCEPTION_fputc
//#define FORBIDDEN_SYMBOL_EXCEPTION_fread
//#define FORBIDDEN_SYMBOL_EXCEPTION_fwrite
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "common/system.h"

#include "chewy/defines.h"
#include "chewy/global.h"

#include "chewy/ani_dat.h"
#include "chewy/episode1.h"
#include "chewy/episode2.h"
#include "chewy/episode3.h"
#include "chewy/episode4.h"

#define STERNE_ANI 17
#define TUER_ZU_ANI 3
#define GITTER_BLITZEN 7

void play_scene_ani(int16 nr, int16 mode) {
#define ROOM_1_1 101
#define ROOM_1_2 102
#define ROOM_1_5 105
#define ROOM_2_3 203
#define ROOM_3_1 301
#define ROOM_8_17 817
#define ROOM_9_4 904
#define ROOM_18_20 1820
	int16 r_nr;
	int16 tmp;
	r_nr = spieler.PersonRoomNr[P_CHEWY] * 100 + nr;

	switch (r_nr) {

	case ROOM_2_3:
		start_aad(49);
		det->start_detail(GITTER_BLITZEN, 12, ANI_VOR);
		spieler.R2KabelBork = 1;
		del_inventar(spieler.AkInvent);
		atds->del_steuer_bit(11, ATS_COUNT_BIT, ATS_DATEI);
		atds->del_steuer_bit(11, ATS_ACTION_BIT, ATS_DATEI);
		atds->del_steuer_bit(19, ATS_COUNT_BIT, ATS_DATEI);
		atds->del_steuer_bit(25, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(8, ATS_COUNT_BIT, ATS_DATEI);
		atds->set_ats_str(11, 1, ATS_DATEI);

		break;

	case ROOM_8_17:
		start_aad(100, 0);
		det->start_detail(21, 4, ANI_VOR);
		break;

	case ROOM_18_20:
		del_inventar(spieler.AkInvent);
		break;

	}
	start_detail_wait(nr, 1, mode);

	switch (r_nr) {

	case ROOM_0_3:
		start_detail_wait(STERNE_ANI, 2, ANI_VOR);
		set_person_pos(222, 106, P_CHEWY, P_LEFT);
		break;

	case ROOM_2_3:
		start_aad_wait(47, -1);
		break;

	case ROOM_1_1:
		det->hide_static_spr(2);
		start_detail_wait(4, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		atds->del_steuer_bit(7, ATS_COUNT_BIT, ATS_DATEI);
		atds->ats_get_txt(7, TXT_MARK_LOOK, &tmp, ATS_DATEI);

		break;

	case ROOM_3_1:
		r3_terminal();
		break;

	case ROOM_9_4:
		r9_gtuer();
		break;
	}

	kbinfo.scan_code = 0;
}

void timer_action(int16 t_nr) {
	int16 default_flag;
	int16 ani_nr;
	int16 ok;
	default_flag = false;
	ani_nr = t_nr - room->room_timer.TimerStart;
	switch (spieler.PersonRoomNr[P_CHEWY]) {

	case 0:
		switch (ani_nr) {
		case 1:
			if (!flags.AutoAniPlay && !flags.AdsDialog) {
				ok = false;
				flags.AutoAniPlay = true;
				if (!spieler.R0SchleimWurf) {
					start_aad_wait(42, -1);
					auto_move(BLITZ_POS, P_CHEWY);
					set_person_spr(P_LEFT, P_CHEWY);
					if (spieler.R0FueterLab < 3) {
						start_spz(CH_TALK3, 255, 0, P_CHEWY);
						start_aad_wait(43, -1);
						++spieler.R0FueterLab;
					}
					r0_auge_ani();
					ok = true;
				} else {
					if (!spieler.R0KissenWurf) {
						start_aad_wait(42, -1);
						start_spz(CH_TALK3, 255, 0, P_CHEWY);
						if (spieler.R0FueterLab < 3) {
							start_aad_wait(43, -1);
							++spieler.R0FueterLab;
						}
						auto_move(FUETTER_POS, P_CHEWY);
						set_person_pos(199 - CH_HOT_MOV_X,
						               145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
					}
				}
				if (!spieler.R0KissenWurf) {
					r0_fuett_ani();
				}
				uhr->reset_timer(t_nr, 0);
				flags.AutoAniPlay = false;
			}
			break;
		default:
			default_flag = true;
			break;

		}
		break;

	case 11:
		if (t_nr == timer_nr[0])
			r11_bork_zwinkert();
		break;

	case 12:
		if (t_nr == timer_nr[0])
			r12_init_bork();
		else if (t_nr == timer_nr[1]) {
			if (spieler.R12TransOn) {
				spieler.R12TransOn = false;
				start_aad_wait(30, -1);
			}
		}
		break;

	case 14:
		switch (ani_nr) {
		case 0:
			r14_eremit_feuer(t_nr, ani_nr);
			break;
		}
		break;

	case 18:
		r18_timer_action(t_nr);
		break;

	case 21:
		if (t_nr == timer_nr[0]) {
			r21_restart_spinne2();
		} else if (t_nr == timer_nr[2])
			r21_chewy_kolli();
		break;

	case 22:
		if (!ani_nr && !flags.ExitMov) {
			r22_bork(t_nr);
		}
		break;

	case 40:
		if (t_nr == timer_nr[0])
			spieler.R40PoliceStart = true;
		default_flag = true;
		break;

	case 48:
		if (t_nr == timer_nr[0])
			r48_frage();
		else
			default_flag = true;
		break;

	case 49:
		if (t_nr == timer_nr[0])
			r49_calc_boy_ani();
		break;

	case 50:
		if (t_nr == timer_nr[0]) {
			r50_calc_wasser();
		}
		default_flag = true;
		break;

	case 56:
		if (t_nr == timer_nr[0])
			r56_start_flug();
		else
			default_flag = true;
		break;

	case 68:
		if (t_nr == timer_nr[0]) {
			r68_calc_diva();
		}
		default_flag = true;
		break;
	default:
		default_flag = true;
		break;

	}
	if (default_flag && flags.AutoAniPlay == false) {
		det->start_detail(room->room_timer.ObjNr[ani_nr], 1, 0);
		uhr->reset_timer(t_nr, 0);
	}
	kbinfo.scan_code = 0;
}

void check_ged_action(int16 index) {
#define KABELABDECKUNG 1
	int16 flag;
	index -= 50;
	index /= 4;
	if (!flags.GedAction) {
		flags.GedAction = true;
		flag = false;
		switch (spieler.PersonRoomNr[P_CHEWY]) {

		case 1:
			switch (index) {
			case 0:
				if (!spieler.R2KabelBork) {
					if (spieler.AkInvent == KABEL_INV) {
						flag = 1;
						del_inventar(spieler.AkInvent);
					} else if (obj->check_inventar(KABEL_INV)) {
						flag = 1;
						obj->del_obj_use(KABEL_INV);
						del_invent_slot(KABEL_INV);
					}
					if (flag) {
						start_aad_wait(54, -1);
						atds->set_ats_str(8, TXT_MARK_LOOK, 0, ATS_DATEI);
						spieler.room_s_obj[KABELABDECKUNG].ZustandFlipFlop = 2;
						obj->calc_rsi_flip_flop(KABELABDECKUNG);
						obj->calc_all_static_detail();
					}
				}
				break;

			}
			break;

		case 2:
			switch (index) {
			case 0:
				det->stop_detail(5);
				if (!spieler.R2KabelBork) {

					det->start_detail(6, 2, ANI_VOR);
				} else {
					start_ani_block(2, ablock4);
				}
				r2_jump_out_r1(9);
				break;

			}
			break;

		case 7:
			switch (index) {
			case 0:
				if (spieler.R7BorkFlug && spieler.R7ChewyFlug) {
					spieler.PersonHide[P_CHEWY] = true;
					start_detail_wait(20, 1, ANI_VOR);
					det->show_static_spr(10);
					wait_show_screen(20 * spieler.DelaySpeed);
					det->hide_static_spr(10);
					set_person_pos(180, 124, P_CHEWY, P_LEFT);
					spieler.PersonHide[P_CHEWY] = false;
					spieler.R7ChewyFlug = false;

				}
				break;

			}
			break;
		case 9:
			switch (index) {
			case 0:
				if (!spieler.R9Surimy)
					r9_surimy();
				break;

			}
			break;

		case 11:
			switch (index) {
			case 0:
				r11_chewy_bo_use();
				break;

			}
			break;

		case 13:
			switch (index) {
			case 2:
				if (spieler.R12ChewyBork) {
					stop_person(P_CHEWY);
					r13_talk_bork();
				}
				break;

			}
			break;

		case 17:
			switch (index) {
			case 0:
				r17_door_kommando(0);
				break;

			case 1:
				r17_door_kommando(1);
				break;

			}
			break;

		case 18:
			if (!index) {
				if (!spieler.R18SurimyWurf) {
					stop_person(P_CHEWY);
					auto_move(1, P_CHEWY);
					start_aad(40, 0);
				}
			}
			break;

		case 28:
			if (!index) {

				r28_get_pump();
			}
			break;

		case 37:
			if (!index) {
				r37_dog_bell();
			} else if (index == 1) {
				if (spieler.R37Kloppe && !spieler.R37Mes) {
					stop_person(P_CHEWY);
					spieler.R37Mes = true;
					start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
					start_aad_wait(142, -1);
				}
			}
			break;

		case 42:
			if (!index)
				r42_calc_xit();
			break;

		case 45:
		case 46:
			if (!index && flags.ExitMov)
				HowardMov = 1;
			break;

		case 49:
			if (!index)
				r49_calc_boy();
			break;

		case 50:
			if (!index)
				r50_calc_treppe();
			break;

		case 52:
			if (index == 1)
				r52_kaker_platt();
			break;

		case 55:
			if (!index)
				r55_talk_line();
			break;
		}
		flags.GedAction = false;
	}
	kbinfo.scan_code = 0;
}

int16 ged_user_func(int16 idx_nr) {
	switch (idx_nr) {
	case 40:
		switch (spieler.PersonRoomNr[P_CHEWY]) {

		case 8:
			if (spieler.R8GTuer)
				idx_nr = 0;
			break;

		case 9:
			if (!spieler.R9Gitter)
				idx_nr = 0;
			break;

		case 16:
			if (!spieler.R16F5Exit)
				idx_nr = 0;
			break;

		case 17:
			if (spieler.R17Location != 1)
				idx_nr = 0;
			break;

		case 21:
			if (!spieler.R21Laser2Weg)
				idx_nr = 0;
			break;

		case 31:
			if (!spieler.R31KlappeZu)
				idx_nr = 0;
			break;

		case 41:
			if (!spieler.R41LolaOk)
				idx_nr = 0;
			break;

		case 52:
			if (!spieler.R52LichtAn)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;
		}
		break;

	case 41:
		switch (spieler.PersonRoomNr[P_CHEWY]) {

		case 17:
			if (spieler.R17Location != 2)
				idx_nr = 0;
			break;

		case 21:
			if (!spieler.R21Laser1Weg) {
				idx_nr = 0;
			} else
				idx_nr = 3;
			break;

		case 37:
			if (!spieler.R37Kloppe) {

				idx_nr = 0;
			}
			break;

		case 52:
			if (!spieler.R52TuerAuf)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;
		}
		break;

	}
	return (idx_nr);
}

void enter_room(int16 eib_nr) {
	int16 i;

	if ((!modul) && (flags.InitSound))
		load_room_music(spieler.PersonRoomNr[P_CHEWY]);
	ERROR
	load_chewy_taf(spieler.ChewyAni);
	atds->stop_aad();
	atds->stop_ats();
	spieler.DiaAMov = -1;

	zoom_mov_fak = 1;
	flags.ZoomMov = true;
	for (i = 0; i < MAX_PERSON; i++) {
		zoom_mov_anpass(&spieler_vector[i], &spieler_mi[i]);
		spieler.ZoomXy[i][0] = 0;
		spieler.ZoomXy[i][1] = 0;
	}
	flags.ZoomMov = false;
	spieler.ZoomXy[P_CHEWY][ 0] = (int16)room->room_info->ZoomFak;
	spieler.ZoomXy[P_CHEWY][1] = (int16)room->room_info->ZoomFak;

	uhr->reset_timer(0, 0);
	flags.AutoAniPlay = false;
	SetUpScreenFunc = false;
	HowardMov = false;
	cur_hide_flag = false;
	switch (spieler.PersonRoomNr[P_CHEWY]) {

	case 0:
		r0_entry();
		break;

	case 3:
		r3_init_sonde();
		break;

	case 6:
		r6_entry();
		break;

	case 8:
		r8_entry();
		break;

	case 9:
		r9_entry();
		break;

	case 10:
		r10_entry();
		break;

	case 11:
		r11_entry();
		break;

	case 12:
		r12_entry();
		break;

	case 13:
		r13_entry();
		break;

	case 14:
		r14_entry();
		break;

	case 16:
		r16_entry();
		break;

	case 17:
		r17_entry();
		if (spieler.SoundSwitch) {
			if (!spieler.R17EnergieOut)
				det->play_sound(15, 0);
		}
		break;

	case 18:
		r18_entry();
		break;

	case 19:
		r19_entry();
		break;

	case 21:
		r21_entry();
		break;

	case 22:
		r22_entry();
		break;

	case 23:
		spieler.PersonHide[P_CHEWY] = true;
		set_person_pos(135, 69, P_CHEWY, -1);
		break;

	case 24:
		r24_entry();
		if (spieler.SoundSwitch)
			det->play_sound(17, 0);
		break;

	case 25:
		r25_entry();
		break;

	case 27:
		r27_entry();
		break;

	case 28:
		r28_entry(eib_nr);
		break;

	case 29:
		r29_entry();
		break;

	case 31:
		r31_entry();
		break;

	case 32:
		r32_entry();
		break;

	case 33:
		r33_entry();
		break;

	case 34:
		if (flags.LoadGame)
			r34_use_kuehlschrank();
		break;

	case 35:
		r35_entry();
		break;

	case 37:
		r37_entry();
		break;

	case 39:
		r39_entry();
		break;

	case 40:
		r40_entry(eib_nr);
		break;

	case 41:
		r41_entry();
		break;

	case 42:
		r42_entry();
		break;

	case 45:
		r45_entry(eib_nr);
		break;

	case 46:
		r46_entry(eib_nr);
		break;

	case 47:
		r47_entry();
		break;

	case 48:
		r48_entry();
		break;

	case 49:
		r49_entry(eib_nr);
		break;

	case 50:
		r50_entry(eib_nr);
		break;

	case 51:
		r51_entry();
		break;

	case 52:
		r52_entry();
		break;

	case 53:
		r53_entry();
		break;

	case 54:
		r54_entry(eib_nr);
		break;

	case 55:
		r55_entry();
		break;

	case 56:
		r56_entry();
		break;

	case 57:
		r57_entry();
		break;

	case 58:
	case 59:
	case 60:
		r58_entry();
		break;

	case 62:
		r62_entry();
		break;

	case 63:
		r63_entry();
		break;

	case 64:
		r64_entry();
		break;

	case 65:
		r65_entry();
		break;

	case 66:
		r66_entry(eib_nr);
		break;

	case 67:
		r67_entry();
		break;

	case 68:
		r68_entry();
		break;

	case 69:
		r69_entry(eib_nr);
		break;
	}
	flags.LoadGame = false;
}

void exit_room(int16 eib_nr) {
	int16 *xy;
	int16 x, y;
	int16 no_exit;
	no_exit = false;
	det->disable_room_sound();

	switch (spieler.PersonRoomNr[P_CHEWY]) {

	case 6:
		if (eib_nr == 8)
			spieler.R17Location = 2;
		break;

	case 11:
		atds->set_steuer_bit(121, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case 18:
		if (eib_nr == 40)
			spieler.R17Location = 2;
		else if (eib_nr == 41)
			spieler.R17Location = 3;
		spieler.ScrollxStep = 1;
		flags.NoScroll = false;
		spieler.R18Gitter = false;
		spieler.room_e_obj[50].Attribut = 255;
		spieler.room_e_obj[41].Attribut = AUSGANG_UNTEN;
		break;

	case 19:
		flags.NoScroll = false;
		break;

	case 21:
		if (eib_nr == 47)
			spieler.R17Location = 1;
		flags.NoEndPosMovObj = false;
		SetUpScreenFunc = 0;
		load_chewy_taf(CHEWY_NORMAL);

		break;

	case 22:
		break;

	case 23:
		spieler.PersonHide[P_CHEWY] = false;
		switch (spieler.R23GleiterExit) {
		case 16:
			set_person_pos(126, 110, P_CHEWY, P_RIGHT);
			break;

		case 25:
			spieler.R25GleiterExit = true;
			break;

		}
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		maus_links_click = false;
		break;

	case 24:
		r24_exit();
		break;

	case 27:
		r27_exit(eib_nr);
		break;

	case 28:
		r28_exit(eib_nr);
		break;

	case 29:
		r29_exit();
		break;

	case 34:
		flags.ChewyDontGo = false;
		break;

	case 41:
		r41_exit();
		break;

	case 45:
		r45_exit(eib_nr);
		break;

	case 46:
		r46_exit();
		break;

	case 47:
		r47_exit();
		break;

	case 49:
		r49_exit(eib_nr);
		break;

	case 52:
		r52_exit();
		break;

	case 54:
		r54_exit(eib_nr);
		break;

	case 56:
		r56_exit();
		break;

	case 57:
		r57_exit(eib_nr);
		break;

	case 64:
		if (spieler.R64Moni1Ani == 5)
			spieler.R64Moni1Ani = 3;
		break;

	case 65:
		r65_exit();
		break;

	default:
		no_exit = true;
		break;
	}

	x = -1;
	y = -1;
	switch (eib_nr) {

	case 0:
	case 3:
	case 18:
	case 30:
	case 35:
	case 48:
	case 51:
	case 70:
	case 83:
	case 93:
		x = -44;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 1:
	case 2:
	case 17:
	case 19:
	case 34:
	case 39:
	case 49:
	case 52:
		xy = (int16 *)ablage[room_blk.AkAblage];
		x = xy[0] + 30;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 10:
	case 41:
	case 15:
	case 58:
	case 73:
	case 77:
	case 78:
	case 92:
		xy = (int16 *)ablage[room_blk.AkAblage];
		x = spieler_vector[P_CHEWY].Xypos[0];
		y = xy[1] + 3;
		break;

	case 6:
	case 11:
	case 13:
	case 14:
	case 22:
	case 23:
	case 25:
	case 27:
	case 33:
	case 38:
	case 40:
	case 50:
	case 65:
	case 74:
		set_person_pos(spieler_vector[P_CHEWY].Xypos[0],
		               spieler_vector[P_CHEWY].Xypos[1], P_CHEWY, P_RIGHT);
		set_up_screen(DO_SETUP);
		break;

	case 8:
	case 9:
	case 12:
	case 16:
	case 20:
	case 21:
	case 24:
	case 32:
	case 36:
	case 71:
	case 96:
		set_person_pos(spieler_vector[P_CHEWY].Xypos[0],
		               spieler_vector[P_CHEWY].Xypos[1], P_CHEWY, P_LEFT);
		set_up_screen(DO_SETUP);
		break;

	case 62:
		x = 20;
		y = 80;
		break;

	case 75:
		x = 160;
		y = 200;
		det->show_static_spr(4);
		break;

	case 72:
		x = spieler_vector[P_CHEWY].Xypos[0];
		y = spieler_vector[P_CHEWY].Xypos[1] - 10;
		break;

	case 84:
		x = spieler_vector[P_CHEWY].Xypos[0] - 70;
		y = spieler_vector[P_CHEWY].Xypos[1] - 50;
		HowardMov = 1;
		break;

	case 85:
		x = spieler_vector[P_CHEWY].Xypos[0] + 70;
		y = spieler_vector[P_CHEWY].Xypos[1];
		HowardMov = 1;
		break;

	case 86:
		det->show_static_spr(0);
		x = spieler_vector[P_CHEWY].Xypos[0] - 44;
		y = spieler_vector[P_CHEWY].Xypos[1];
		HowardMov = 2;
		break;

	case 90:
		det->show_static_spr(8);
		x = spieler_vector[P_CHEWY].Xypos[0] - 60;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 94:
		det->show_static_spr(3);
		x = spieler_vector[P_CHEWY].Xypos[0] - 40;
		y = spieler_vector[P_CHEWY].Xypos[1] - 10;
		break;

	}
	if (x != -1 && y != -1) {
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(x, y, P_CHEWY, ANI_WAIT);
		spieler_mi[P_CHEWY].Mode = false;
	}

	if (no_exit) {
		switch (spieler.PersonRoomNr[P_CHEWY]) {

		case 40:
			r40_exit(eib_nr);
			break;

		case 42:
			if (spieler.PersonRoomNr[P_HOWARD] == 42)
				spieler.PersonRoomNr[P_HOWARD] = 40;
			break;

		case 50:
			r50_exit(eib_nr);
			break;

		case 51:
			r51_exit(eib_nr);
			break;

		case 55:
			r55_exit(eib_nr);
			break;

		case 66:
			r66_exit(eib_nr);
			break;

		case 67:
			r67_exit();
			break;

		case 68:
			r68_exit();
			break;

		case 69:
			r69_exit(eib_nr);
			break;
		}
	}
}

void flic_cut(int16 nr, int16 mode) {
	int16 i;
	out->setze_zeiger(0);
	det->disable_room_sound();
	ailsnd->end_sound();
	g_system->delayMillis(1000); // delay(50);
	Ci.Handle = (void *)fopen("CUT\\CUT.TAP\0", "rb");
	Ci.Fname = 0;
	if (Ci.Handle) {
		switch (nr) {
		case FCUT_001:
			ailsnd->stop_mod();
			CurrentSong = -1;
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			break;
		case FCUT_019:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
			ailsnd->stop_mod();
			CurrentSong = -1;
			nr = FCUT_019;
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, 21);
				out->cls();
				flc->custom_play(&Ci);
				if (!modul) {
					fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
					out->cls();
					flc->custom_play(&Ci);
				}
			}
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, 20);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, 23);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				fseek((FILE *)Ci.Handle, sizeof(ChunkHead), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			ERROR;
			ailsnd->fade_out(0);
			out->ausblenden(1);
			out->cls();
			while (ailsnd->music_playing());
			break;

		case FCUT_053:
			for (i = 0; i < 3; i++) {
				mem->file->select_pool_item(Ci.Handle, nr);

				flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			}
			break;

		case FCUT_058:

			nr = FCUT_058;
			mem->file->select_pool_item(Ci.Handle, nr);

			flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, FCUT_059);

				flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			}
			if (!spieler.R43GetPgLady) {
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_060);

					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
			} else {
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_061);

					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_062);

					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
			}
			ailsnd->fade_out(0);
			out->ausblenden(1);
			out->cls();
			while (ailsnd->music_playing());
			break;

		case FCUT_065:
			r46_kloppe();
			break;

		default:
			mem->file->select_pool_item(Ci.Handle, nr);
			if (!mode)
				flc->custom_play(&Ci);
			else if (mode == 1)
				flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			ERROR;
			break;

		}
		fclose((FILE *)Ci.Handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}
	ERROR;
	ailsnd->end_sound();
	g_system->delayMillis(1000); // delay(50);
	ailsnd->set_sound_mastervol(spieler.SoundVol);
	ailsnd->set_music_mastervol(spieler.MusicVol);
	load_room_music(spieler.PersonRoomNr[P_CHEWY]);
	ERROR
	room->load_sound();
	ERROR
	if (spieler.SoundSwitch != false)
		det->enable_room_sound();
	if (!flags.NoPalAfterFlc)
		out->set_palette(pal);
	atds->stop_aad();
	atds->stop_ats();
	uhr->reset_timer(0, 0);
	out->setze_zeiger(workptr);
	flags.NoPalAfterFlc = false;
}

uint16 exit_flip_flop(int16 ani_nr, int16 eib_nr1, int16 eib_nr2,
                        int16 ats_nr1, int16 ats_nr2, int16 sib_nr,
                        int16 spr_nr1, int16 spr_nr2, int16 flag) {
	if (ani_nr != -1)
		det->start_detail(ani_nr, 1, flag);
	flag ^= 1;
	if (ats_nr1 != -1)
		atds->set_ats_str(ats_nr1, flag, ATS_DATEI);
	if (ats_nr2 != -1)
		atds->set_ats_str(ats_nr2, flag, ATS_DATEI);
	if (flag) {
		if (eib_nr1 != -1)
			spieler.room_e_obj[eib_nr1].Attribut = spr_nr1;
		if (eib_nr2 != -1)
			spieler.room_e_obj[eib_nr2].Attribut = spr_nr2;
	} else {
		if (eib_nr1 != -1)
			spieler.room_e_obj[eib_nr1].Attribut = 255;
		if (eib_nr2 != -1)
			spieler.room_e_obj[eib_nr1].Attribut = 255;
	}
	if (sib_nr != -1)
		obj->calc_rsi_flip_flop(sib_nr);
	return ((uint16)flag);
}

int16 sib_event_no_inv(int16 sib_nr) {
	int16 ret;
	ret = true;
	switch (sib_nr) {

	case SIB_MONOKEL:
		spieler.R0Monokel = true;
		obj->hide_sib(SIB_MONOKEL);
		if (spieler.R0GBuch)
			atds->del_steuer_bit(12, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_GBUCH:
		spieler.R0GBuch = true;
		obj->hide_sib(SIB_GBUCH);
		if (spieler.R0Monokel)
			atds->del_steuer_bit(12, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_KABEL_R1:
		atds->set_ats_str(8, TXT_MARK_LOOK, 1, ATS_DATEI);
		break;

	case SIB_TERMINAL_R5:
		if (spieler.R5Terminal) {
			spieler.R5Terminal = 0;
			det->stop_detail(6);
			atds->set_ats_str(27, TXT_MARK_LOOK, 0, ATS_DATEI);
			atds->set_ats_str(30, TXT_MARK_LOOK, 0, ATS_DATEI);
		}
		break;

	case SIB_KNOPF_R5:
		r5_knopf();
		break;

	case SIB_BOLA_KNOPF_R6:
		r6_bola_knopf();
		break;

	case SIB_SEIL:
		obj->hide_sib(SIB_SEIL);
		break;

	case SIB_BOLA_SCHACHT:
		atds->set_ats_str(41, TXT_MARK_LOOK, 0, ATS_DATEI);
		break;

	case SIB_TKNOPF1_R7:
		spieler.R6DoorLeftB = exit_flip_flop(3, 12, 9, 49, 35, SIB_TKNOPF2_R6,
		                                     AUSGANG_OBEN, AUSGANG_LINKS,
		                                     (int16)spieler.R6DoorLeftB);
		break;

	case SIB_TKNOPF2_R7:
		spieler.R7DoorRight = exit_flip_flop(4, 13, 14, 53, 68, SIB_TUER_R8,
		                                     AUSGANG_RECHTS, AUSGANG_RECHTS,
		                                     (int16)spieler.R7DoorRight);
		break;

	case SIB_HEBEL_R7:
		spieler.R7Hebel ^= 1;
		atds->set_ats_str(50, spieler.R7Hebel, ATS_DATEI);
		break;

	case SIB_GIPS_R7:
		spieler.R7RHaken = true;
		obj->show_sib(SIB_RHAKEN_R7);
		obj->hide_sib(SIB_GIPS_R7);
		break;

	case SIB_KLINGEL_R7:
		r7_klingel();
		break;

	case SIB_SCHLOTT_R7:
		obj->hide_sib(SIB_SCHLOTT_R7);
		break;

	case SIB_LHAKEN_R7:
		if (spieler.R7SeilLeft) {
			if (spieler.R7RHaken) {
				spieler.R7SeilOk = true;
				auto_move(4, P_CHEWY);
				obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
				obj->calc_rsi_flip_flop(SIB_RHAKEN_R7);
				atds->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATEI);
				atds->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATEI);
				atds->del_steuer_bit(56, ATS_AKTIV_BIT, ATS_DATEI);
				start_aad(9);
			} else {
				obj->set_rsi_flip_flop(SIB_LHAKEN_R7, 2);
				obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			}
		}
		break;

	case SIB_FOLTER_R8:
		r8_stop_folter();

		break;

	case SIB_TUER_R8:
		spieler.R7DoorRight = exit_flip_flop(-1, 14, 13, 68, 53, SIB_TKNOPF2_R7,
		                                     AUSGANG_RECHTS, AUSGANG_RECHTS,
		                                     (int16)spieler.R7DoorRight);
		break;

	case SIB_DEE_PAINT_R9:
		obj->hide_sib(SIB_DEE_PAINT_R9);
		break;

	case SIB_SCHLITZ_R11:
		r11_get_card();
		break;

	case SIB_TALISMAN_R12:
		spieler.R12Talisman = true;
		obj->hide_sib(SIB_TALISMAN_R12);
		timer_nr[0] = room->set_timer(255, 20);
		break;

	case SIB_BANDKNOPF_R13:
		spieler.R13Bandlauf ^= 1;

		atds->set_ats_str(94, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

		atds->set_ats_str(97, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

		atds->set_ats_str(93, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);
		break;

	case SIB_GITTER_R16:
		atds->set_ats_str(125, 1, ATS_DATEI);
		spieler.room_e_obj[33].Attribut = AUSGANG_OBEN;
		break;

	case SIB_CART1_R18:
		atds->set_steuer_bit(155, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_CART_FACH_R18:
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		spieler.R18CartFach = 0;
		cur_2_inventory();
		break;

	case SIB_TUERKNOPF_R18:
		if (spieler.R18DoorBruecke) {
			det->disable_sound(19, 0);
			det->enable_sound(19, 1);
		} else {
			det->enable_sound(19, 0);
			det->disable_sound(19, 1);
		}

		if (!spieler.R6DoorLeftF) {
			spieler.R6DoorLeftF = exit_flip_flop(-1, 8, -1, 33, -1, SIB_TKNOPF1_R6,
			                                     AUSGANG_LINKS, -1,
			                                     (int16)spieler.R6DoorLeftF);
		}
		spieler.R18DoorBruecke = exit_flip_flop(19, 40, 35, 148, -1, -1,
		                                        AUSGANG_OBEN, AUSGANG_LINKS,
		                                        (int16)spieler.R18DoorBruecke);
		break;

	case SIB_SCHALTER1_R21:
		det->start_detail(0, 1, spieler.R21Hebel1);
		spieler.R21Hebel1 ^= 1;
		r21_calc_laser();
		atds->set_ats_str(126, TXT_MARK_LOOK, spieler.R21Hebel1, ATS_DATEI);
		break;

	case SIB_SCHALTER2_R21:
		det->start_detail(1, 1, spieler.R21Hebel2);
		spieler.R21Hebel2 ^= 1;
		r21_calc_laser();
		atds->set_ats_str(127, TXT_MARK_LOOK, spieler.R21Hebel2, ATS_DATEI);
		break;

	case SIB_SCHALTER3_R21:
		det->start_detail(2, 1, spieler.R21Hebel3);
		spieler.R21Hebel3 ^= 1;
		r21_calc_laser();
		atds->set_ats_str(128, TXT_MARK_LOOK, spieler.R21Hebel3, ATS_DATEI);
		break;

	case SIB_SEIL_R21:
		atds->set_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_GITTER1_R21:
		r21_use_gitter_energie();
		break;

	case SIB_CARTRIDGE_R23:
		r23_get_cartridge();
		break;

	case SIB_FLUXO_R23:
		spieler.R23FluxoFlex = false;
		atds->set_ats_str(112, 0, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_TRANSLATOR_23:
		atds->set_ats_str(113, 1, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_SURIMY_R27:
		r27_get_surimy();
		break;

	case SIB_TOPF_R31:
		obj->hide_sib(SIB_TOPF_R31);
		atds->set_ats_str(242, 1, ATS_DATEI);
		spieler.R31PflanzeWeg = true;
		break;

	case SIB_SCHLAUCH_R26:
	case SIB_TRICHTER_R26:
	case SIB_ANGEL0_R26:
	case SIB_MILCH_R27:
	case SIB_KAFFEE_R33:
	case SIB_ZAPPER_R39:
	case SIB_KNOCHEN_R35:
	case SIB_KUERBIS_R37:
	case SIB_ZEITUNG_R27:
	case SIB_FLASCHE_R51:
	case SIB_KAPPE_R51:
	case SIB_AUSRUEST_R51:
	case SIB_PIRANHA:
	case SIB_SCHALL:
	case SIB_ARTEFAKT:
		obj->hide_sib(sib_nr);
		break;

	case SIB_PUTE_R34:
		atds->set_ats_str(226, 1, ATS_DATEI);
		break;

	case SIB_RADIO_R35:
		obj->hide_sib(sib_nr);
		invent_2_slot(CASSETTE_INV);
		break;

	case SIB_MUENZE_R40:
		obj->hide_sib(sib_nr);
		det->del_static_ani(6);
		room->set_timer_status(6, TIMER_STOP);
		spieler.R40Geld = true;
		start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(201, -1);
		break;

	case SIB_CIGAR_R50:
		r50_stop_cigar();
		break;

	case SIB_LAMPE_R52:
		atds->del_steuer_bit(338, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.R52LichtAn ^= 1;
		check_shad(2 * (spieler.R52LichtAn + 1), 1);
		break;

	case SIB_VISIT_R53:
		obj->hide_sib(sib_nr);
		spieler.R53Visit = true;
		break;

	case SIB_KAUTABAK_R56:
		obj->hide_sib(sib_nr);
		spieler.R56GetTabak = true;
		break;

	case SIB_ASCHE_R64:
		det->stop_detail(0);
		obj->hide_sib(sib_nr);
		r64_talk_man(351);
		break;
	default:
		ret = false;
		break;

	}
	return (ret);
}

void sib_event_inv(int16 sib_nr) {
	switch (sib_nr) {

	case SIB_TERMINAL_R5:
		if (!spieler.R5Terminal) {
			spieler.R5Terminal = 1;
			cur_2_inventory();
			del_inventar(RED_CARD_INV);

			start_aad(103, -1);
			det->start_detail(6, 255, ANI_VOR);
			atds->set_ats_str(27, TXT_MARK_LOOK, 1, ATS_DATEI);
			atds->set_ats_str(30, TXT_MARK_LOOK, 1, ATS_DATEI);
		}
		break;

	case SIB_TKNOPF1_R6:
		spieler.R6DoorLeftF = exit_flip_flop(4, 8, -1, 33, -1, -1,
		                                     AUSGANG_LINKS, -1,
		                                     (int16)spieler.R6DoorLeftF);
		break;

	case SIB_TKNOPF2_R6:
		spieler.R6DoorLeftB = exit_flip_flop(5, 9, 12, 35, 49, SIB_TKNOPF1_R7,
		                                     AUSGANG_LINKS, AUSGANG_OBEN,
		                                     (int16)spieler.R6DoorLeftB);
		break;

	case SIB_TKNOPF3_R6:
		spieler.R6DoorRightB = exit_flip_flop(6, 11, 20, 37, 99, SIB_TKNOPF3_R11,
		                                      AUSGANG_RECHTS, AUSGANG_LINKS,
		                                      (int16)spieler.R6DoorRightB);
		break;

	case SIB_BOLA_KNOPF_R6:
		del_inventar(spieler.AkInvent);
		spieler.R6BolaSchild = true;
		det->show_static_spr(2);
		obj->calc_rsi_flip_flop(SIB_BOLA_KNOPF_R6);
		obj->hide_sib(SIB_BOLA_KNOPF_R6);
		obj->show_sib(SIB_BOLA_R6);
		break;

	case SIB_RHAKEN_R7:
	case SIB_LHAKEN_R7:
		r7_haken(sib_nr);
		break;

	case SIB_GTUER:
		r8_open_gdoor();
		break;

	case SIB_SURIMY_R10:
		r10_get_surimy();
		break;

	case SIB_SCHLITZ_R11:
		r11_put_card();
		break;

	case SIB_TKNOPF1_R11:
		spieler.R11DoorRightF = exit_flip_flop(1, 23, -1, 87, -1, -1,
		                                       AUSGANG_OBEN, -1, (int16)spieler.R11DoorRightF);
		break;

	case SIB_TKNOPF2_R11:
		spieler.R11DoorRightB = exit_flip_flop(5, 22, -1, 98, -1, -1,
		                                       AUSGANG_OBEN, -1, (int16)spieler.R11DoorRightB);
		break;

	case SIB_TKNOPF3_R11:
		spieler.R6DoorRightB = exit_flip_flop(6, 20, 11, 99, 37, SIB_TKNOPF3_R6,
		                                      AUSGANG_LINKS, AUSGANG_RECHTS,
		                                      (int16)spieler.R6DoorRightB);
		break;

	case SIB_ROEHRE_R12:
		spieler.R12TalismanOk = true;
		del_inventar(spieler.AkInvent);
		atds->set_ats_str(118, TXT_MARK_LOOK, 1, ATS_DATEI);
		start_aad(115, 0);
		break;

	case SIB_FEUER_R14:
		r14_feuer();
		break;

	case SIB_CART_FACH_R18:
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		spieler.R18CartFach = 1;
		del_inventar(spieler.AkInvent);
		if (spieler.R18CartTerminal == true) {
			spieler.R18CartSave = true;
			start_aad_wait(120, -1);
		} else
			start_aad_wait(121, -1);
		break;

	case SIB_PAINT_R22:
		disable_timer();
		r22_malen();
		enable_timer();
		break;

	case SIB_GITTER2_R21:
		spieler.R21GitterMuell = exit_flip_flop(-1, 46, 27, 132, 90, -1,
		                                        AUSGANG_RECHTS, AUSGANG_RECHTS,
		                                        (int16)spieler.R21GitterMuell);
		atds->set_ats_str(90, TXT_MARK_USE, 2, ATS_DATEI);
		break;

	case SIB_CARTRIDGE_R23:
		r23_use_cartridge();
		if (spieler.R18CartSave == true)
			atds->set_ats_str(111, 2, ATS_DATEI);
		else
			atds->set_ats_str(111, 1, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_FLUXO_R23:
		spieler.R23FluxoFlex = true;
		del_inventar(spieler.AkInvent);
		atds->set_ats_str(112, 1, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_TRANSLATOR_23:
		del_inventar(spieler.AkInvent);
		atds->set_ats_str(113, 0, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_AUTO_R28:
		r28_use_surimy();
		break;

	case SIB_AUTO_SITZ:
		r29_schlitz_sitz();
		break;
	}
}
