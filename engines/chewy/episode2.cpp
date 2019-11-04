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

#include "engines/chewy/defines.h"
#include "engines/chewy/global.h"
#include "engines/chewy/ani_dat.h"
#include "engines/chewy/episode2.h"

void calc_person_look() {
	int16 i;
	for (i = 1; i < MAX_PERSON; i++) {
		if (spieler_mi[i].Id != NO_MOV_OBJ) {

			if (spieler_vector[i].Xypos[0] > spieler_vector[P_CHEWY].Xypos[0])
				person_end_phase[i] = P_LEFT;
			else
				person_end_phase[i] = P_RIGHT;
		}
	}
}

void r25_entry() {
	if (!spieler.R29Schlauch2) {
		det->hide_static_spr(0);
		det->hide_static_spr(1);
	}
	if (!spieler.R25FirstEntry) {
		hide_cur();
		if (obj->check_inventar(TRANSLATOR_INV)) {
			obj->calc_rsi_flip_flop(SIB_TRANSLATOR_23);
			atds->set_ats_str(113, 0, ATS_DATEI);

			obj->del_inventar(TRANSLATOR_INV, &room_blk);
			spieler.inv_cur = false;
			menu_item = CUR_WALK;
			spieler.AkInvent = -1;
			cursor_wahl(menu_item);
			del_invent_slot(TRANSLATOR_INV);
		}
		spieler.R25FirstEntry = true;
		spieler.PersonHide[P_CHEWY] = true;
		flic_cut(FCUT_029, FLC_MODE);
		fx_blende = 0;
		set_person_pos(219, 141, P_CHEWY, P_RIGHT);
		spieler.PersonHide[P_CHEWY] = false;
		start_spz(CH_TALK11, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(64, -1);
		show_cur();
	} else if (spieler.R25GleiterExit) {
		set_person_pos(127, 122, P_CHEWY, P_LEFT);
		if (spieler.R25SurimyGo < 1) {
			++spieler.R25SurimyGo;
		} else {
			spieler.R25GleiterExit = false;
			r25_xit_gleiter();
		}
	}
}

int16 r25_gleiter_loesch() {
	int16 action_flag = false;
	if (!spieler.R25GleiteLoesch && spieler.R29Schlauch2) {
		if (!spieler.inv_cur) {
			action_flag = true;
			spieler.R25GleiteLoesch = true;
			auto_move(2, P_CHEWY);
			flic_cut(FCUT_030, FLC_MODE);
			obj->calc_rsi_flip_flop(SIB_SCHLAUCH_R25);
			atds->set_ats_str(219, 1, ATS_DATEI);
			atds->set_ats_str(187, 1, ATS_DATEI);
		}
	} else if (spieler.R25GleiteLoesch) {
		if (is_cur_inventar(MILCH_LEER_INV)) {
			action_flag = true;
			auto_move(2, P_CHEWY);
			start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
			del_inventar(spieler.AkInvent);
			obj->add_inventar(MILCH_WAS_INV, &room_blk);
			inventory_2_cur(MILCH_WAS_INV);
			start_aad_wait(253, -1);
		}
	}
	return (action_flag);
}

int16 r25_use_gleiter() {
	int16 action_flag = false;
	if (!spieler.inv_cur && spieler.R25GleiteLoesch) {
		action_flag = true;
		auto_move(3, P_CHEWY);
		spieler.R23GleiterExit = 25;
		r23_cockpit();
	}
	return (action_flag);
}

#define SURIMY_OBJ 0
int16 r25_surimy_phasen[4][2] = {
	{ 56, 63 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

MovLine r25_surimy_mpkt[2] = {
	{ { 100, 150, 150 }, 0, 6 },
	{ { -20, 150, 150 }, 0, 6 }
};

void r25_xit_gleiter() {
	if (!spieler.R25SurimyLauf) {
		spieler.R25SurimyLauf = true;
		det->load_taf_seq(56, 8, 0);
		auto_obj = 1;
		mov_phasen[SURIMY_OBJ].AtsText = 0;
		mov_phasen[SURIMY_OBJ].Lines = 2;
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		mov_phasen[SURIMY_OBJ].ZoomFak = 0;
		auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
		auto_mov_vector[SURIMY_OBJ].Delay = spieler.DelaySpeed + 2;
		auto_mov_obj[SURIMY_OBJ].Mode = 1;
		init_auto_obj(SURIMY_OBJ, &r25_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
		              *)r25_surimy_mpkt);
		fx_blende = 1;
		set_up_screen(DO_SETUP);
		start_spz(CH_TALK12, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(65, -1);
		fx_blende = 0;
		wait_auto_obj(SURIMY_OBJ);
		auto_obj = 0;
	}
}

void r27_entry() {
	if (spieler.PersonRoomNr[P_HOWARD] == 27) {
		timer_nr[0] = room->set_timer(0, 5);
		det->set_static_ani(0, -1);
		atds->del_steuer_bit(274, ATS_AKTIV_BIT, ATS_DATEI);
	} else
		atds->set_steuer_bit(274, ATS_AKTIV_BIT, ATS_DATEI);
	spieler.PersonHide[P_HOWARD] = true;
	spieler.ScrollxStep = 2;
}

void r27_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	hide_cur();
	if (spieler.PersonRoomNr[P_HOWARD] == 27) {
		if (eib_nr == 55) {
			start_aad_wait(175, -1);
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			start_detail_wait(2, 1, ANI_VOR);
			spieler.PersonRoomNr[P_HOWARD] = 28;
			spieler_mi[P_HOWARD].Id = HOWARD_OBJ;
		} else if ((spieler.R27HowardGed < 3) && (eib_nr != -1)) {
			++spieler.R27HowardGed;
			start_aad_wait(174, -1);
		}
	}
	spieler.PersonHide[P_HOWARD] = false;
	show_cur();
}

void r27_get_surimy() {
	obj->calc_all_static_detail();
	auto_move(4, P_CHEWY);
	if (spieler.PersonRoomNr[P_HOWARD] == 27) {
		start_aad_wait(171, -1);
	}
	obj->hide_sib(SIB_SURIMY_R27);
}

void r27_talk_howard() {
	auto_move(4, P_CHEWY);
	start_aad_wait(173, -1);
}

void r27_howard_ged() {
	if (spieler.R27HowardGed < 3) {
		++spieler.R27HowardGed;
	}
}

void r28_entry(int16 eib_nr) {
	zoom_horizont = 140;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	spieler.ScrollxStep = 2;
	if (spieler.R28RKuerbis)
		det->show_static_spr(6);
	if (spieler.R28Briefkasten) {
		det->show_static_spr(8);
		det->show_static_spr(9);
	} else
		det->show_static_spr(7);
	if (spieler.PersonRoomNr[P_HOWARD] == 28) {
		spieler.ZoomXy[P_HOWARD][0] = 40;
		spieler.ZoomXy[P_HOWARD][1] = 40;
		hide_cur();
		SetUpScreenFunc = r28setup_func;
		if (spieler.R40Wettbewerb) {

			spieler.scrollx = 320;
			spieler.PersonHide[P_CHEWY] = false;
			r28_haendler();
			spieler.R40Wettbewerb = false;
		} else {
			switch (eib_nr) {
			case 55:
				if (!spieler.R28EntryHaus) {

					spieler.R28EntryHaus = true;
					set_person_pos(232, 100, P_HOWARD, P_RIGHT);
					auto_move(3, P_HOWARD);
					go_auto_xy(320, spieler_vector[P_HOWARD].Xypos[1], P_HOWARD, ANI_WAIT);
					start_aad_wait(176, -1);
				} else if (spieler.R31SurFurz && !spieler.R28RKuerbis) {
					spieler.R28RKuerbis = true;
					spieler.ScrollxStep = 2;
					auto_move(6, P_CHEWY);
					wait_show_screen(10);
					start_aad_wait(194, -1);
					spieler.room_e_obj[62].Attribut = 255;
					atds->set_ats_str(208, 1, ATS_DATEI);
					flags.NoScroll = true;
					auto_scroll(0, 0);
					flc->set_flic_user_function(r28_cut_serv);
					flic_cut(FCUT_064, FLC_MODE);
					flc->remove_flic_user_function();
					det->show_static_spr(6);
					flags.NoScroll = false;
					if (!spieler.R40TeilKarte) {
						wait_show_screen(50);
						start_aad_wait(195, -1);
					} else {
						r28_haendler();
					}
					spieler.ScrollxStep = 1;
				}
				break;

			case 70:
			case 77:
				if (spieler.R28PostCar) {
					spieler.R28PostCar = false;
					out->setze_zeiger(0);
					out->cls();
					flic_cut(FCUT_063, FLC_MODE);
					spieler.R28ChewyPump = false;
					invent_2_slot(K_MASKE_INV);
					atds->set_ats_str(209, 0, ATS_DATEI);
					spieler.room_e_obj[69].Attribut = 255;
					load_chewy_taf(CHEWY_NORMAL);

					set_person_pos(480, 118, P_HOWARD, P_LEFT);
					set_person_pos(440, 146, P_CHEWY, P_RIGHT);
					fx_blende = 0;
					flags.ExitMov = false;
					spieler_mi[P_HOWARD].Mode = true;
					go_auto_xy(420, 113, P_HOWARD, ANI_WAIT);
					spieler_mi[P_HOWARD].Mode = false;
					set_person_pos(440, 146, P_CHEWY, P_LEFT);
					start_aad_wait(193, -1);
				} else {
					set_person_pos(480, 100, P_HOWARD, P_LEFT);
					go_auto_xy(420, 113, P_HOWARD, ANI_GO);
					flags.ExitMov = false;
					auto_move(6, P_CHEWY);
				}
				break;

			}
		}
		show_cur();
	}
}

void r28_exit(int16 eib_nr) {
	spieler.R28PostCar = false;
	spieler.ScrollxStep = 1;
	hide_cur();
	if (spieler.PersonRoomNr[P_HOWARD] == 28) {
		if (eib_nr == 69) {
			SetUpScreenFunc = 0;
			if (!spieler.R28ExitTown) {
				start_aad_wait(178, -1);
				spieler.R28ExitTown = true;
			}
			flags.ExitMov = false;
			auto_move(6, P_HOWARD);
			spieler.PersonRoomNr[P_HOWARD] = 40;
		}
	}
	show_cur();
}

void r28_haendler() {
	hide_cur();
	SetUpScreenFunc = 0;
	spieler.R28ChewyPump = true;
	del_inventar(K_MASKE_INV);
	atds->set_ats_str(209, 1, ATS_DATEI);
	spieler.room_e_obj[69].Attribut = AUSGANG_OBEN;
	load_chewy_taf(CHEWY_PUMPKIN);
	set_person_pos(480, 113, P_HOWARD, P_RIGHT);
	set_person_pos(490, 146, P_CHEWY, P_RIGHT);
	det->set_static_ani(2, -1);
	wait_show_screen(50);
	start_aad_wait(196, -1);
	det->del_static_ani(2);
	det->set_static_ani(3, -1);
	start_aad_wait(197, -1);
	det->del_static_ani(3);
	det->set_static_ani(4, -1);
	start_aad_wait(198, -1);
	SetUpScreenFunc = r28setup_func;
	auto_move(4, P_CHEWY);
	hide_cur();
	auto_move(3, P_CHEWY);
	set_person_spr(P_RIGHT, P_CHEWY);
	det->del_static_ani(4);
	wait_show_screen(28);
	start_aad_wait(199, -1);
	invent_2_slot(DOLLAR175_INV);
}

void r28setup_func() {
	int16 x;
	calc_person_look();
	if (spieler_vector[P_CHEWY].Xypos[0] > 350)
		x = 420;
	else
		x = 320;
	go_auto_xy(x, 113, P_HOWARD, ANI_GO);
}

void r28_use_surimy() {
	int16 dia_nr;
	int16 ani_nr;
	if (!spieler.R28SurimyCar) {
		hide_cur();
		flags.NoScroll = true;
		auto_scroll(0, 0);
		spieler.R28SurimyCar = true;
		start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
		flc->set_flic_user_function(r28_cut_serv2);
		flic_cut(FCUT_055, FLC_MODE);
		flc->remove_flic_user_function();
		flc->set_flic_user_function(r28_cut_serv);
		flic_cut(FCUT_056, FLC_MODE);
		flc->remove_flic_user_function();
		ani_nr = CH_TALK3;
		dia_nr = 140;
		atds->set_ats_str(205, 1, ATS_DATEI);
		atds->set_ats_str(222, 1, ATS_DATEI);
	} else {
		ani_nr = CH_TALK5;
		dia_nr = 139;
		obj->calc_rsi_flip_flop(SIB_AUTO_R28);
	}
	obj->calc_all_static_detail();
	start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(dia_nr, -1);
	flags.NoScroll = false;
	show_cur();
}

void r28_set_pump() {
	int16 tmp;
	hide_cur();
	if (spieler.PersonRoomNr[P_CHEWY] == 28) {
		if (!flags.AutoAniPlay) {
			flags.AutoAniPlay = true;
			if (spieler_vector[P_CHEWY].Xypos[0] < 380)
				auto_move(5, P_CHEWY);
			spieler.PersonHide[P_CHEWY] = true;
			if (person_end_phase[P_CHEWY] == P_RIGHT)
				tmp = 1;
			else
				tmp = 0;
			del_inventar(K_MASKE_INV);
			det->
			set_detail_pos(tmp, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
			if (spieler.R28PumpTxt1 < 3) {
				start_aad(137);
				++spieler.R28PumpTxt1;
			}
			start_detail_wait(tmp, 1, ANI_VOR);
			spieler.PersonHide[P_CHEWY] = false;

			load_chewy_taf(CHEWY_PUMPKIN);
			spieler.R28ChewyPump = true;
			if (spieler.R39TransMensch) {
				spieler.room_e_obj[69].Attribut = AUSGANG_OBEN;
			} else
				atds->set_ats_str(209, 1, ATS_DATEI);
			if (spieler.PersonRoomNr[P_HOWARD] == 28 && spieler.R28PumpTxt < 3) {
				stop_person(P_HOWARD);
				SetUpScreenFunc = 0;
				start_aad_wait(177, -1);
				SetUpScreenFunc = r28setup_func;
				++spieler.R28PumpTxt;
			}
			flags.AutoAniPlay = false;
		}
	} else {
		start_ats_wait(20, TXT_MARK_USE, 14, INV_USE_DEF);
	}
	show_cur();
}

void r28_get_pump() {
	int16 tmp;
	if (spieler.R28ChewyPump) {
		hide_cur();
		stop_person(P_CHEWY);
		spieler.R28ChewyPump = false;
		spieler.PersonHide[P_CHEWY] = true;
		if (person_end_phase[P_CHEWY] == P_RIGHT)
			tmp = 1;
		else
			tmp = 0;
		det->set_detail_pos(tmp, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
		start_detail_wait(tmp, 1, ANI_RUECK);
		invent_2_slot(K_MASKE_INV);
		atds->set_ats_str(209, 0, ATS_DATEI);
		spieler.room_e_obj[69].Attribut = 255;
		spieler.PersonHide[P_CHEWY] = false;
		load_chewy_taf(CHEWY_NORMAL);
		if (spieler.R28PumpTxt1 < 3) {
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(138, -1);
		}
		show_cur();
	}
}

int16 r28_use_breifkasten() {
	int16 action_flag = false;
	if (spieler.R28Briefkasten && !spieler.inv_cur) {
		action_flag = true;
		hide_cur();
		spieler.R28Briefkasten = false;
		auto_move(7, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		det->hide_static_spr(8);
		det->hide_static_spr(9);
		det->show_static_spr(7);
		auto_move(8, P_CHEWY);
		start_spz(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		start_aad_wait(179, -1);
		atds->set_ats_str(206, 0, ATS_DATEI);
		show_cur();
		invent_2_slot(MANUSKRIPT_INV);
		spieler.R28Manuskript = true;
	}
	return (action_flag);
}

int16 r28_cut_serv(int16 frame) {
	if (spieler.R28Briefkasten) {
		det->plot_static_details(0, 0, 8, 9);
	} else {
		det->plot_static_details(0, 0, 7, 7);
	}
	return (0);
}

int16 r28_cut_serv2(int16 frame) {
	if (frame < 23) {
		if (spieler.R28Briefkasten) {
			det->plot_static_details(0, 0, 8, 9);
		} else {
			det->plot_static_details(0, 0, 7, 7);
		}
	}
	return (0);
}

void r29_entry() {
	if (spieler.R29Schlauch1)
		det->show_static_spr(7);
	else if (spieler.R29Schlauch2) {
		det->show_static_spr(8);
		det->show_static_spr(10);
	}
	if (spieler.R29AutoSitz)
		det->show_static_spr(9);
}

void r29_exit() {
	if (obj->check_inventar(PUMPE_INV)) {
		del_inventar(PUMPE_INV);
	}
}

int16 r29_use_pumpe() {
	int16 action_flag = false;
	if (!spieler.R29Pumpe) {
		action_flag = true;
		if (is_cur_inventar(SCHLAUCH_INV)) {
			spieler.R29Pumpe = true;
			spieler.R29Schlauch1 = true;
			auto_move(1, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			det->show_static_spr(7);
			atds->del_steuer_bit(218, ATS_AKTIV_BIT, ATS_DATEI);
			del_inventar(SCHLAUCH_INV);
		} else if (!spieler.inv_cur)
			start_aad_wait(62, -1);
	}
	return (action_flag);
}

int16 r29_get_schlauch() {
	int16 action_flag = false;
	if (spieler.R29Schlauch1 && !spieler.inv_cur) {
		action_flag = true;
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
		new_invent_2_cur(PUMPE_INV);
	}
	return (action_flag);
}

void r29_use_schlauch() {
	if (is_cur_inventar(PUMPE_INV)) {
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
		det->hide_static_spr(7);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(4, 1, ANI_VOR);
		det->show_static_spr(8);
		det->show_static_spr(10);
		atds->del_steuer_bit(219, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(218, 1, ATS_DATEI);
		spieler.R29Schlauch1 = false;
		spieler.R29Schlauch2 = true;
		del_inventar(PUMPE_INV);
		set_person_pos(308, 105, P_CHEWY, P_RIGHT);
		spieler.PersonHide[P_CHEWY] = false;
	}
}

void r29_schlitz_sitz() {
	if (!spieler.R29AutoSitz) {
		spieler.R29AutoSitz = true;
		spieler.PersonHide[P_CHEWY] = true;
		det->hide_static_spr(4);
		start_aad(63);
		start_ani_block(4, ablock26);
		det->show_static_spr(9);
		while (flags.AdsDialog) {
			set_up_screen(DO_SETUP);
		}
		det->stop_detail(2);
		atds->del_steuer_bit(212, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.PersonHide[P_CHEWY] = false;
		kbinfo.scan_code = 0;
	}
}

int16 r29_zaun_sprung() {
	int16 action_flag = false;
	if (spieler.R29AutoSitz && !spieler.inv_cur) {
		action_flag = true;
		auto_move(3, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_frame(3, 1, ANI_VOR, 7);
		det->hide_static_spr(9);
		start_ani_block(5, ablock27);
		set_up_screen(DO_SETUP);
		switch_room(37);
		spieler.PersonHide[P_CHEWY] = false;
	}
	return (action_flag);
}

#define SURIMY_OBJ 0
int16 surimy_taf19_phasen[4][2] = {
	{ 0, 0 },
	{ 39, 46 },
	{ 0, 0 },
	{ 0, 0 }
};

MovLine r31_surimy_mpkt[2] = {
	{ {  79, 152, 150 }, 1, 6 },
	{ { 273, 220, 150 }, 1, 6 }
};

void r31_entry() {
	r31_calc_luke();
	r31_surimy_go();
}

void r31_surimy_go() {
	if (!spieler.R39ScriptOk) {
		if (spieler.R31SurimyGo >= 3) {
			hide_cur();
			spieler.R31SurimyGo = 0;
			det->load_taf_seq(39, 8, 0);
			auto_obj = 1;
			mov_phasen[SURIMY_OBJ].AtsText = 0;
			mov_phasen[SURIMY_OBJ].Lines = 2;
			mov_phasen[SURIMY_OBJ].Repeat = 1;
			mov_phasen[SURIMY_OBJ].ZoomFak = 0;
			auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[SURIMY_OBJ].Delay = spieler.DelaySpeed + 2;
			auto_mov_obj[SURIMY_OBJ].Mode = 1;
			init_auto_obj(SURIMY_OBJ, &surimy_taf19_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
			              *)r31_surimy_mpkt);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(157, -1);
			wait_auto_obj(SURIMY_OBJ);
			auto_obj = 0;
			show_cur();
		} else
			++spieler.R31SurimyGo;
	}
}

void r31_calc_luke() {
	int16 i;
	if (!spieler.R31KlappeZu) {
		for (i = 0; i < 3; i++)
			det->show_static_spr(5 + i);
		atds->set_ats_str(244, 1, ATS_DATEI);
		atds->del_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.room_e_obj[75].Attribut = AUSGANG_UNTEN;
	} else {
		for (i = 0; i < 3; i++)
			det->hide_static_spr(5 + i);
		atds->set_ats_str(244, 0, ATS_DATEI);
		atds->set_steuer_bit(245, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.room_e_obj[75].Attribut = 255;
	}
}

void r31_open_luke() {
	if (spieler.R31KlappeZu) {
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
		spieler.R31KlappeZu = false;
		r31_calc_luke();
	}
}

void r31_close_luke() {
	if (!spieler.R31KlappeZu) {
		auto_move(2, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		spieler.R31KlappeZu = true;
		r31_calc_luke();
	}
}

int16 r31_use_topf() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	int16 action_flag = false;

	hide_cur();
	if (spieler.inv_cur) {
		if (spieler.R31PflanzeWeg) {
			if (is_cur_inventar(K_KERNE_INV)) {
				spieler.R31KoernerDa = true;
				auto_move(1, P_CHEWY);
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				del_inventar(spieler.AkInvent);
				ani_nr = CH_TALK3;
				dia_nr = 150;
				atds->set_ats_str(242, 2, ATS_DATEI);
			} else if (is_cur_inventar(MILCH_WAS_INV)) {
				if (spieler.R31KoernerDa) {
					spieler.R31Wasser = true;
					auto_move(1, P_CHEWY);
					spieler.PersonHide[P_CHEWY] = true;
					start_ani_block(3, ablock30);
					spieler.PersonHide[P_CHEWY] = false;
					del_inventar(spieler.AkInvent);
					obj->add_inventar(MILCH_LEER_INV, &room_blk);
					inventory_2_cur(MILCH_LEER_INV);
					ani_nr = CH_TALK6;
					dia_nr = 151;
					atds->set_ats_str(242, 3, ATS_DATEI);
				} else {
					ani_nr = CH_TALK5;
					dia_nr = 152;
				}
			} else if (is_cur_inventar(SURIMY_INV)) {
				if (!spieler.R31SurFurz) {
					if (spieler.R31Wasser) {
						if (!spieler.R28SurimyCar) {
							ani_nr = CH_TALK5;
							dia_nr = 180;
						} else {
							r31_close_luke();
							auto_move(3, P_CHEWY);
							flic_cut(FCUT_046, FLC_MODE);
							spieler.R31SurFurz = true;
							ani_nr = CH_TALK6;
							dia_nr = 156;
							atds->set_ats_str(242, 4, ATS_DATEI);
							cur_2_inventory();
						}
					} else {
						ani_nr = CH_TALK5;
						dia_nr = 155;
					}
				}
			} else {
				ani_nr = CH_TALK5;
				dia_nr = 153;
			}
		} else {
			ani_nr = CH_TALK5;
			dia_nr = 154;
		}
	}
	if (dia_nr != -1) {
		start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}
	show_cur();
	return (action_flag);
}

void r32_entry() {
	if (spieler.R32HowardWeg)
		det->hide_static_spr(0);
	if (!spieler.R32Script && spieler.R32UseSchreib) {
		det->show_static_spr(5);
	}
}

int16 r32_use_howard() {
	int16 dia_nr = 0;
	int16 ani_nr = 0;
	int16 action_flag = false;
	hide_cur();
	if (is_cur_inventar(TRICHTER_INV)) {
		if (spieler.R33MunterGet) {
			auto_move(1, P_CHEWY);
			cur_2_inventory();
			obj->del_inventar(MUNTER_INV, &room_blk);
			del_invent_slot(MUNTER_INV);
			flic_cut(FCUT_043, FLC_MODE);
			atds->set_steuer_bit(230, ATS_AKTIV_BIT, ATS_DATEI);
			start_spz(CH_TALK12, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(75, -1);
			wait_show_screen(5);
			auto_move(5, P_CHEWY);
			wait_show_screen(10);
			start_spz(CH_TALK12, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(125, -1);
			wait_show_screen(10);
			det->hide_static_spr(0);
			start_detail_frame(0, 1, ANI_VOR, 9);
			start_detail_wait(1, 1, ANI_RUECK);
			det->show_static_spr(7);
			det->show_static_spr(6);
			wait_show_screen(20);
			det->hide_static_spr(7);
			start_detail_wait(1, 1, ANI_VOR);
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			ani_nr = CH_TALK3;
			dia_nr = 164;
			spieler.R32HowardWeg = true;
			spieler.R39HowardDa = true;
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 73;
		}
	} else if (is_cur_inventar(MUNTER_INV)) {
		ani_nr = CH_TALK12;
		dia_nr = 74;
	}
	if (dia_nr) {
		start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}
	show_cur();
	return (action_flag);
}

void r32_use_schreibmaschine() {
	int16 dia_nr = -1;
	int16 ani_nr = -1;

	hide_cur();
	if (spieler.R32HowardWeg) {
		if (spieler.inv_cur) {
			switch (spieler.AkInvent) {
			case PAPIER_INV:
				auto_move(2, P_CHEWY);
				spieler.R32PapierOk = true;
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				del_inventar(spieler.AkInvent);
				atds->set_ats_str(231, TXT_MARK_LOOK, 1, ATS_DATEI);
				ani_nr = CH_TALK3;
				dia_nr = 86;
				break;

			case CYB_KRONE_INV:
				if (!spieler.R32UseSchreib) {
					if (!spieler.R32PapierOk) {
						ani_nr = CH_TALK12;
						dia_nr = 87;
					} else {
						auto_move(3, P_CHEWY);
						spieler.R32UseSchreib = true;
						cur_2_inventory();
						flic_cut(FCUT_044, FLC_MODE);
						det->show_static_spr(5);
						atds->set_ats_str(203, 1, ATS_DATEI);
						ani_nr = CH_TALK3;
						dia_nr = 88;
						atds->set_ats_str(231, TXT_MARK_LOOK, 0, ATS_DATEI);
					}
				}
				break;

			default:
				ani_nr = CH_TALK12;
				dia_nr = 90;
				break;

			}
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 89;
		}
	} else {
		ani_nr = CH_TALK12;
		dia_nr = 92;
	}
	start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(dia_nr, -1);
	show_cur();
}

int16 r32_get_script() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (!spieler.R32Script && spieler.R32UseSchreib) {
			action_flag = true;
			spieler.R32Script = true;
			auto_move(4, P_CHEWY);
			invent_2_slot(MANUSKRIPT_INV);
			start_spz_wait(CH_LGET_U, 1, ANI_VOR, P_CHEWY);
			det->hide_static_spr(5);
			atds->set_ats_str(203, 0, ATS_DATEI);
			start_spz(CH_TALK3, 1, ANI_VOR, P_CHEWY);
			start_aad_wait(91, -1);
		}
	}
	return (action_flag);
}

void r33_entry() {
	if (spieler.R33MunterGet)
		det->hide_static_spr(3);
	r33_surimy_go();
}

MovLine r33_surimy_mpkt[2] = {
	{ {  65, 115, 150 }, 1, 6 },
	{ { 300, 151, 150 }, 1, 6 }
};

void r33_surimy_go() {
	if (!spieler.R39ScriptOk) {
		if (spieler.R33SurimyGo >= 4) {
			hide_cur();
			spieler.R33SurimyGo = 0;
			det->load_taf_seq(39, 8, 0);
			auto_obj = 1;
			mov_phasen[SURIMY_OBJ].AtsText = 0;
			mov_phasen[SURIMY_OBJ].Lines = 2;
			mov_phasen[SURIMY_OBJ].Repeat = 1;
			mov_phasen[SURIMY_OBJ].ZoomFak = 0;
			auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[SURIMY_OBJ].Delay = spieler.DelaySpeed + 2;
			auto_mov_obj[SURIMY_OBJ].Mode = 1;
			init_auto_obj(SURIMY_OBJ, &surimy_taf19_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
			              *)r33_surimy_mpkt);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(158, -1);
			wait_auto_obj(SURIMY_OBJ);
			auto_obj = 0;
			show_cur();
		} else
			++spieler.R33SurimyGo;
	}
}

void r33_look_schublade() {
	if (!spieler.R33SchubFirst) {
		spieler.R33SchubFirst = true;
		auto_move(1, P_CHEWY);
		atds->set_ats_str(210, TXT_MARK_NAME, 1, ATS_DATEI);
		atds->set_ats_str(210, TXT_MARK_USE, 1, ATS_DATEI);
	}
}

int16 r33_use_schublade() {
	int16 action_flag = false;
	if (spieler.R33SchubFirst && !spieler.inv_cur) {
		if (!spieler.R33Messer) {
			action_flag = true;
			spieler.R33Messer = true;
			atds->set_ats_str(210, TXT_MARK_NAME, 0, ATS_DATEI);
			atds->set_ats_str(210, TXT_MARK_LOOK, 1, ATS_DATEI);
			atds->set_ats_str(210, TXT_MARK_USE, 2, ATS_DATEI);
			start_spz(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			invent_2_slot(MESSER_INV);
		}
	}
	return (action_flag);
}

void r33_use_maschine() {
	int16 action;
	int16 dia_nr;
	int16 ani_nr;
	int16 hocker;
	action = true;
	hocker = false;
	hide_cur();
	if (!spieler.R33MunterOk) {
		auto_move(4, P_CHEWY);
		if (spieler.inv_cur) {
			switch (spieler.AkInvent) {
			case K_FLEISCH_INV:
				spieler.R33Munter[1] = true;
				break;

			case MILCH_INV:
				spieler.R33Munter[3] = true;
				invent_2_slot(MILCH_LEER_INV);
				break;

			case KAFFEE_INV:
				spieler.R33Munter[0] = true;
				break;

			case EIER_INV:
				spieler.R33Munter[2] = true;
				break;

			default:
				action = false;
				break;

			}
			if (action) {
				spieler.PersonHide[P_CHEWY] = true;
				start_detail_wait(0, 1, ANI_VOR);
				spieler.PersonHide[P_CHEWY] = false;
				set_person_pos(128, 65, P_CHEWY, P_LEFT);
				start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
				hocker = true;
				del_inventar(spieler.AkInvent);
				ani_nr = CH_TALK12;
				if (r33_calc_muntermacher()) {
					spieler.R33MunterOk = true;
					dia_nr = 69;
				} else {
					dia_nr = 67;
				}
			} else {
				ani_nr = CH_TALK4;
				dia_nr = 68;
			}
		} else {
			ani_nr = CH_TALK12;
			dia_nr = 66;
		}
		start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		if (spieler.R33MunterOk) {
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_wait(2, 1, ANI_VOR);
			flic_cut(FCUT_031, FLC_MODE);
			spieler.PersonHide[P_CHEWY] = false;
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(71, -1);
			atds->del_steuer_bit(225, ATS_AKTIV_BIT, ATS_DATEI);
		}
		if (hocker) {
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_wait(1, 1, ANI_VOR);
			set_person_pos(64, 100, P_CHEWY, P_LEFT);
		}
		spieler.PersonHide[P_CHEWY] = false;
	} else {
		start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(70, -1);
	}
	show_cur();
}

int16 r33_calc_muntermacher() {
	int16 ret;
	int16 i;
	ret = true;
	for (i = 0; i < 4; i++) {
		if (spieler.R33Munter[i] == false)
			ret = false;
	}
	return (ret);
}

int16 r33_get_munter() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (!spieler.R33MunterGet && spieler.R33MunterOk) {
			action_flag = true;
			spieler.R33MunterGet = true;
			auto_move(4, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			invent_2_slot(MUNTER_INV);
			atds->set_steuer_bit(225, ATS_AKTIV_BIT, ATS_DATEI);
			det->hide_static_spr(3);
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(72, -1);
		}
	}
	return (action_flag);
}

void r34_use_kuehlschrank() {
	if (!flags.LoadGame) {
		auto_move(3, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
	}
	spieler.PersonHide[P_CHEWY] = true;
	flags.ChewyDontGo = true;
	if (!flags.LoadGame) {
		switch_room(34);
	}
	set_person_pos(160, 70, P_CHEWY, -1);
}

void r34_xit_kuehlschrank() {
	spieler.PersonHide[P_CHEWY] = false;
	set_person_pos(54, 111, P_CHEWY, -1);
	switch_room(33);
	flags.ChewyDontGo = false;
	maus_links_click = false;
}

void r35_entry() {
	if (spieler.R35Schublade)
		det->show_static_spr(1);
}

int16 r35_schublade() {
	int16 action_flag = false;
	hide_cur();
	if (!spieler.inv_cur) {
		if (!spieler.R35Schublade) {
			action_flag = true;
			auto_move(3, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			det->show_static_spr(1);
			spieler.R35Schublade = true;
			atds->set_ats_str(234, 1, ATS_DATEI);
		} else if (!spieler.R35Falle) {
			action_flag = true;
			auto_move(3, P_CHEWY);
			spieler.R35Falle = true;
			spieler.PersonHide[P_CHEWY] = true;
			start_ani_block(2, ablock28);
			spieler.PersonHide[P_CHEWY] = false;
			set_person_pos(33, 90, P_CHEWY, P_LEFT);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(93, -1);
			spieler.PersonHide[P_CHEWY] = false;
			atds->set_ats_str(234, 2, ATS_DATEI);
		}
	}
	show_cur();
	return (action_flag);
}

int16 r35_use_cat() {
	int16 action_flag = false;
	hide_cur();
	if (is_cur_inventar(TRANSLATOR_INV)) {
		action_flag = true;
		auto_move(4, P_CHEWY);
		spieler.R35TransCat = true;
		start_spz_wait(CH_TRANS, 1, ANI_VOR, P_CHEWY);
		flic_cut(FCUT_045, FLC_MODE);
		start_spz(CH_TRANS, 1, ANI_VOR, P_CHEWY);
		start_aad_wait(94, -1);
	} else if (is_cur_inventar(PUTENKEULE_INV)) {
		action_flag = true;
		disable_timer();
		auto_move(4, P_CHEWY);
		spieler.R35CatEat = true;
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		del_inventar(spieler.AkInvent);
		det->stop_detail(0);
		det->del_static_ani(0);
		start_detail_wait(1, 1, ANI_VOR);
		det->start_detail(2, 1, ANI_VOR);
		start_detail_wait(3, 1, ANI_VOR);
		obj->show_sib(SIB_KNOCHEN_R35);
		obj->calc_rsi_flip_flop(SIB_KNOCHEN_R35);
		det->show_static_spr(7);
		atds->del_steuer_bit(237, ATS_AKTIV_BIT, ATS_DATEI);
		while (det->get_ani_status(2)) {
			set_up_screen(DO_SETUP);
		}
		det->set_static_ani(0, -1);
		enable_timer();
		start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(95, -1);
	}
	show_cur();
	return (action_flag);
}

void r35_talk_cat() {
	int16 dia_nr;
	auto_move(4, P_CHEWY);
	if (spieler.R35TransCat) {
		if (!spieler.R35CatEat) {
			dia_nr = 7;
		} else {
			dia_nr = 8;
		}
		spieler.PersonHide[P_CHEWY] = true;
		switch_room(36);
		start_ads_wait(dia_nr);
		spieler.PersonHide[P_CHEWY] = false;
		switch_room(35);
	} else {
		start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(96, -1);
	}
}

void r37_entry() {
	zoom_horizont = 100;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	SetUpScreenFunc = r37_setup_func;
	if (!flags.LoadGame) {
		spieler.scrollx = 124;
		set_person_pos(219, 66, P_CHEWY, P_RIGHT);
	}
	if (!spieler.R37Kloppe) {
		timer_nr[1] = room->set_timer(7, 5);
		det->set_static_ani(7, -1);
		if (!spieler.R37HundScham) {
			timer_nr[0] = room->set_timer(3, 4);
			det->set_static_ani(3, -1);
		}
	}
	if (spieler.R37Gebiss) {
		det->hide_static_spr(9);
		if (spieler.R37Kloppe) {
			det->hide_static_spr(8);
		} else if (spieler.R37HundScham)
			det->show_static_spr(0);
	}
}

void r37_setup_func() {
	if (maus_links_click &&
	        !spieler.R37Kloppe &&
	        menu_item == CUR_WALK) {
		if ((minfo.x + spieler.scrollx > 380 && minfo.y > 120) ||
		        (minfo.x + spieler.scrollx > 482)) {
			auto_move(7, P_CHEWY);
			maus_links_click = false;
		}
	}
}

short r37_use_wippe() {
	int16 action_flag = false;
	if (spieler.inv_cur) {
		action_flag = true;
		if (is_cur_inventar(H_FUTTER_INV)) {
			hide_cur();
			auto_move(0, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(129, 0);
			start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(159, -1);
			del_inventar(spieler.AkInvent);
			flc->set_flic_user_function(r37_cut_serv1);
			flic_cut(FCUT_047, FLC_MODE);
			flc->remove_flic_user_function();
			flags.NoScroll = false;
			show_cur();
			spieler.scrollx = 269;
			set_person_pos(388, 119, P_CHEWY, P_RIGHT);
			switch_room(29);
			maus_links_click = false;
		} else {
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(160, -1);
		}
	}
	return (action_flag);
}

int16 r37_cut_serv1(int16 frame) {
	int16 static_nr;
	int16 static_nr1;
	if (!spieler.R37Kloppe) {
		if (!spieler.R37Gebiss) {
			static_nr = 9;
			static_nr1 = 11;
			det->show_static_spr(11);
		} else {
			static_nr = 8;
			static_nr1 = 0;
		}
		det->plot_static_details(spieler.scrollx, spieler.scrolly, static_nr, static_nr);
		det->plot_static_details(spieler.scrollx, spieler.scrolly, static_nr1, static_nr1);
	}

	det->plot_static_details(spieler.scrollx, spieler.scrolly, 7, 7);
	det->plot_static_details(spieler.scrollx, spieler.scrolly, 14, 14);
	return (0);
}

int16 r37_cut_serv2(int16 frame) {
	int16 static_nr[] = {7, 14, 12, 10};
	short i;
	det->show_static_spr(12);
	det->show_static_spr(10);
	for (i = 0; i < 4; i++)
		det->plot_static_details(spieler.scrollx, spieler.scrolly, static_nr[i], static_nr[i]);
	return (0);
}

int16 r37_use_glas() {
	int16 action_flag = false;
	if (!spieler.R37Gebiss) {
		if (is_cur_inventar(ANGEL2_INV)) {
			action_flag = true;
			flags.NoScroll = true;
			hide_cur();
			auto_move(5, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(146, 0);
			start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(147, -1);
			del_inventar(spieler.AkInvent);
			flc->set_flic_user_function(r37_cut_serv2);
			flic_cut(FCUT_048, FLC_MODE);
			flc->remove_flic_user_function();
			flic_cut(FCUT_049, FLC_MODE);
			invent_2_slot(GEBISS_INV);
			det->hide_static_spr(9);
			atds->set_ats_str(250, 1, ATS_DATEI);
			atds->set_ats_str(256, 1, ATS_DATEI);
			atds->del_steuer_bit(251, ATS_AKTIV_BIT, ATS_DATEI);
			obj->show_sib(SIB_HFUTTER2_R37);
			spieler.R37Gebiss = true;
			start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(146, -1);
			show_cur();
			flags.NoScroll = false;
		} else {
			auto_move(4, P_CHEWY);
		}
	}
	return (action_flag);
}

void r37_dog_bell() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;

	hide_cur();
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!spieler.R37Gebiss) {
			stop_person(P_CHEWY);
			flags.ChAutoMov = false;
			set_person_spr(P_LEFT, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(178, 0);
			disable_timer();
			det->stop_detail(3);
			det->del_static_ani(3);
			start_detail_wait(5, 1, ANI_VOR);
			det->hide_static_spr(9);
			start_detail_wait(6, 1, ANI_VOR);
			spieler.PersonHide[P_CHEWY] = true;
			det->start_detail(11, 255, ANI_VOR);
			flic_cut(FCUT_050, FLC_MODE);
			start_detail_wait(6, 1, ANI_RUECK);
			det->stop_detail(11);
			set_person_pos(326, 85, P_CHEWY, P_LEFT);
			spieler.PersonHide[P_CHEWY] = false;
			det->show_static_spr(9);
			start_ani_block(3, ablock31);
			det->set_static_ani(3, -1);
			enable_timer();
			dia_nr = 149;
			ani_nr = CH_TALK12;
		} else if (!spieler.R37HundScham) {
			stop_person(P_CHEWY);
			set_person_spr(P_LEFT, P_CHEWY);
			flags.NoScroll = true;
			auto_scroll(178, 0);
			room->set_timer_status(3, TIMER_STOP);
			det->del_static_ani(3);
			det->stop_detail(3);
			start_detail_wait(4, 1, ANI_VOR);
			flic_cut(FCUT_051, FLC_MODE);
			spieler.scrollx = 104;
			flic_cut(FCUT_054, FLC_MODE);
			flic_cut(FCUT_054, FLC_MODE);
			det->show_static_spr(0);
			spieler.R37HundScham = true;
			dia_nr = 148;
			ani_nr = CH_TALK6;
		}
		flags.AutoAniPlay = false;
		if (dia_nr != -1) {
			start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(dia_nr, -1);
		}
	}
	flags.NoScroll = false;
	show_cur();
}

void r37_talk_hahn() {
	auto_move(7, P_CHEWY);
	if (!spieler.R37TransHahn) {
		start_aad_wait(145, -1);
	} else {
		r37_hahn_dia();
	}
}

void r37_use_hahn() {
	if (is_cur_inventar(TRANSLATOR_INV)) {
		hide_cur();
		auto_move(7, P_CHEWY);
		spieler.R37TransHahn = true;
		start_spz_wait(CH_TRANS, 1, ANI_VOR, P_CHEWY);
		flic_cut(FCUT_052, FLC_MODE);
		cur_2_inventory();
		menu_item = CUR_TALK;
		cursor_wahl(menu_item);
		show_cur();
		r37_hahn_dia();
	} else if (spieler.R37TransHahn) {
		if (is_cur_inventar(GEBISS_INV)) {
			spieler.R37Kloppe = true;
			hide_cur();
			auto_move(6, P_CHEWY);
			load_room_music(256);
			room->set_timer_status(7, TIMER_STOP);
			det->stop_detail(7);
			det->del_static_ani(7);
			det->start_detail(9, 1, ANI_VOR);
			start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
			del_inventar(GEBISS_INV);
			flags.NoScroll = true;
			auto_scroll(177, 0);
			while (det->get_ani_status(9)) {
				set_up_screen(DO_SETUP);
			}
			det->start_detail(4, 1, ANI_VOR);
			det->hide_static_spr(0);
			det->start_detail(10, 10, ANI_VOR);
			auto_move(8, P_CHEWY);
			flic_cut(FCUT_053, FLC_MODE);
			det->stop_detail(10);
			spieler.scrollx = 320;
			flags.NoScroll = false;
			atds->set_steuer_bit(251, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_steuer_bit(250, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_steuer_bit(256, ATS_AKTIV_BIT, ATS_DATEI);
			det->hide_static_spr(8);
			start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(141, -1);
			obj->add_inventar(EIER_INV, &room_blk);
			inventory_2_cur(EIER_INV);
			show_cur();
		}
	} else if (spieler.inv_cur) {
		start_aad_wait(143, -1);
	}
}

void r37_hahn_dia() {
	int16 tmp_scrollx;
	int16 tmp_scrolly;
	spieler.PersonHide[P_CHEWY] = true;
	tmp_scrollx = spieler.scrollx;
	tmp_scrolly = spieler.scrolly;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	switch_room(38);
	start_ads_wait(9);
	spieler.PersonHide[P_CHEWY] = false;
	flags.LoadGame = true;
	spieler.scrollx = tmp_scrollx;
	spieler.scrolly = tmp_scrolly;
	switch_room(37);
	flags.LoadGame = false;
}

uint8 tv_flic[] = {
	FCUT_039,
	FCUT_040,
	FCUT_035,
	FCUT_032,
	FCUT_037,
	FCUT_034
};

void r39_entry() {
	if (!spieler.R41Einbruch) {
		if (spieler.R39HowardDa) {
			det->show_static_spr(10);
			if (!spieler.R39HowardWach)
				det->start_detail(1, 255, ANI_VOR);
			else
				det->set_static_ani(5, -1);
			atds->del_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATEI);
		}
	} else
		atds->set_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATEI);
	r39_set_tv();
}

short r39_use_howard() {
	int16 dia_nr = -1;
	int16 ani_nr = 0;
	int16 action_flag = false;

	if (!spieler.R39HowardWach) {
		if (spieler.inv_cur) {
			if (is_cur_inventar(MANUSKRIPT_INV)) {
				hide_cur();
				spieler.R39HowardWach = true;
				spieler.R39ScriptOk = true;
				auto_move(3, P_CHEWY);
				spieler.PersonHide[P_CHEWY] = true;
				det->start_detail(6, 255, ANI_VOR);
				start_aad_wait(170, -1);
				det->stop_detail(6);
				start_detail_wait(7, 1, ANI_VOR);
				spieler.PersonHide[P_CHEWY] = false;
				del_inventar(spieler.AkInvent);
				det->stop_detail(1);
				start_ani_block(2, ablock33);
				start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(167, -1);
				det->stop_detail(3);
				start_detail_wait(4, 1, ANI_VOR);
				det->set_static_ani(5, -1);
				atds->set_ats_str(62, 1, ATS_DATEI);
				start_aad_wait(169, -1);
				show_cur();
				spieler.PersonGlobalDia[P_HOWARD] = 10012;
				spieler.PersonDiaRoom[P_HOWARD] = true;;
				calc_person_dia(P_HOWARD);
				if (spieler.R41HowardDiaOK) {
					r39_ok();
				}
				dia_nr = -1;
			} else {
				ani_nr = CH_TALK11;
				dia_nr = 166;
			}
		} else {
			ani_nr = CH_TALK5;
			dia_nr = 165;
		}
		if (dia_nr != -1) {
			start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(dia_nr, -1);
			action_flag = true;
		}
		show_cur();
	}
	return (action_flag);
}

void r39_talk_howard() {
	if (spieler.R39HowardWach) {
		auto_move(3, P_CHEWY);
		spieler.PersonGlobalDia[P_HOWARD] = 10012;
		spieler.PersonDiaRoom[P_HOWARD] = true;;
		calc_person_dia(P_HOWARD);
		if (spieler.R41HowardDiaOK) {
			r39_ok();
		}
	} else {
		start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(168, -1);
	}
}

void r39_ok() {
	spieler.R41Einbruch = true;

	r43_night_small();
	spieler.PersonRoomNr[P_HOWARD] = 27;
	obj->show_sib(SIB_SURIMY_R27);
	obj->show_sib(SIB_ZEITUNG_R27);
	obj->calc_rsi_flip_flop(SIB_SURIMY_R27);
	obj->calc_rsi_flip_flop(SIB_ZEITUNG_R27);
	invent_2_slot(BRIEF_INV);
	switch_room(27);
	start_aad_wait(192, -1);
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);
}

int16 r39_use_tv() {
	int16 cls_flag;
	int16 dia_nr = -1;
	int16 ani_nr = -1;
	int16 action_flag = false;

	auto_move(2, P_CHEWY);
	cls_flag = false;
	hide_cur();
	if (is_cur_inventar(ZAPPER_INV)) {
		spieler.R39TvOn = true;
		if (spieler.R39TvKanal >= 5)
			spieler.R39TvKanal = -1;
		flags.NoPalAfterFlc = true;
		out->setze_zeiger(0);
		out->cls();
		flic_cut(FCUT_042, FLC_MODE);
		++spieler.R39TvKanal;
		flags.NoPalAfterFlc = true;
		if (spieler.R39TvKanal == 2)
			flic_cut(FCUT_036, FLC_MODE);
		else if (spieler.R39TvKanal == 5)
			flic_cut(FCUT_033, FLC_MODE);
		r39_look_tv(0);
		r39_set_tv();
		cls_flag = true;
		if (!spieler.R39TransMensch) {
			ani_nr = CH_TALK11;
			dia_nr = 78;
		} else {
			if (!spieler.R39TvKanal && spieler.R39ClintNews < 3) {
				dia_nr = 79;
				ani_nr = -1;
			} else {
				dia_nr = 80 + spieler.R39TvKanal;
				ani_nr = -1;
			}
		}
	} else if (is_cur_inventar(TRANSLATOR_INV)) {
		if (spieler.R39TvOn) {
			start_spz_wait(CH_TRANS, 1, ANI_VOR, P_CHEWY);
			spieler.R39TransMensch = true;
			flags.NoPalAfterFlc = true;
			flic_cut(FCUT_041, FLC_MODE);
			spieler.R39TvKanal = 0;
			spieler.R39ClintNews = 0;
			out->setze_zeiger(0);
			out->cls();
			out->set_palette(pal);
			flags.NoPalAfterFlc = true;
			flic_cut(tv_flic[0], FLC_MODE);
			out->cls();
			out->setze_zeiger(0);
			out->cls();
			out->set_palette(pal);
			r39_set_tv();
			start_spz(CH_TRANS, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(80, -1);

			ani_nr = CH_TRANS;
			dia_nr = 77;
		} else {
			ani_nr = CH_TALK11;
			dia_nr = 76;
		}
		r39_set_tv();
	} else if (is_cur_inventar(RECORDER_INV)) {
		if (spieler.R39TvOn) {
			if (!spieler.R39TransMensch) {
				ani_nr = CH_TALK12;
				dia_nr = 97;
			} else {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(98, -1);
				spieler.PersonHide[P_CHEWY] = true;
				start_ani_block(2, ablock29);
				spieler.PersonHide[P_CHEWY] = false;
				ani_nr = CH_TALK5;
				dia_nr = 99;
				atds->set_ats_str(CASSETTE_INV, spieler.R39TvKanal + 1, INV_ATS_DATEI);
				spieler.R39TvRecord = spieler.R39TvKanal + 1;
			}
		} else {
			ani_nr = CH_TALK11;
			dia_nr = 76;
		}
	}
	if (cls_flag) {
		out->cls();
		out->setze_zeiger(0);
		out->cls();
		out->set_palette(pal);
		flags.NoPalAfterFlc = false;
	}
	if (dia_nr != -1) {
		if (ani_nr != -1)
			start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}
	show_cur();
	return (action_flag);
}

void r39_look_tv(int16 cls_mode) {
	int16 flic_nr;
	int16 dia_nr;
	if (spieler.R39TvOn) {
		if (!spieler.R39TvKanal && spieler.R39ClintNews < 3) {
			flic_nr = FCUT_038;
			++spieler.R39ClintNews;
			dia_nr = 79;
		} else {
			flic_nr = tv_flic[spieler.R39TvKanal];
			if (!spieler.R39TvKanal)
				spieler.R39ClintNews = 0;
			dia_nr = 80 + spieler.R39TvKanal;
		}
		if (cls_mode) {
			out->setze_zeiger(0);
			out->cls();
			out->set_palette(pal);
			flags.NoPalAfterFlc = true;
		}
		flic_cut(flic_nr, FLC_MODE);
		if (cls_mode) {
			out->cls();
			out->setze_zeiger(0);
			out->cls();
			out->set_palette(pal);
			flags.NoPalAfterFlc = false;
			if (spieler.R39TransMensch)
				start_aad_wait(dia_nr, -1);
		}
	}
}

void r39_set_tv() {
	int16 i;
	for (i = 0; i < 6; i++)
		det->hide_static_spr(i + 4);
	if (spieler.R39TvOn) {
		if (spieler.R39TvKanal == 2)
			det->start_detail(0, 255, ANI_VOR);
		else {
			det->stop_detail(0);
			det->show_static_spr(spieler.R39TvKanal + 4);
		}
		if (spieler.R39TransMensch) {
			atds->set_ats_str(229, TXT_MARK_LOOK, 2 + spieler.R39TvKanal, ATS_DATEI);
		} else {
			atds->set_ats_str(229, TXT_MARK_LOOK, 1, ATS_DATEI);
		}
	}
}

void r40_entry(int16 eib_nr) {

	zoom_horizont = 130;
	spieler.ScrollxStep = 2;
	if (spieler.R40Geld) {
		det->del_static_ani(6);
		room->set_timer_status(6, TIMER_STOP);
	}
	if (spieler.R40HaendlerOk) {
		det->del_static_ani(4);
		room->set_timer_status(4, TIMER_STOP);
	}
	if (spieler.R40PoliceWeg == false) {
		timer_nr[0] = room->set_timer(255, 10);
		atds->del_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		det->hide_static_spr(15);
	}
	spieler.R40PoliceAniStatus = 255;
	spieler.R40PoliceStart = 0;
	spieler_mi[P_HOWARD].Mode = true;
	if (spieler.PersonRoomNr[P_HOWARD] == 40) {
		spieler.ZoomXy[P_HOWARD][0] = 40;
		spieler.ZoomXy[P_HOWARD][1] = 40;
		if (!flags.LoadGame) {
			switch (eib_nr) {
			case 69:
				set_person_pos(30, 105, P_HOWARD, P_RIGHT);
				go_auto_xy(158, 99, P_HOWARD, ANI_GO);
				break;

			case 73:
			case 74:
			case 87:
				set_person_pos(158, 93, P_HOWARD, P_LEFT);
				break;

			}
		}
	}
	SetUpScreenFunc = r40_setup_func;
	if (spieler.R40TrainMove)
		r40_move_train(0);
}

void r40_exit(int16 eib_nr) {
	hide_cur();
	spieler.ScrollxStep = 1;
	spieler.R40PoliceAb = false;
	stop_spz();
	SetUpScreenFunc = 0;
	if (spieler.PersonRoomNr[P_HOWARD] == 40) {
		if (eib_nr == 70 || eib_nr == 77) {
			spieler.PersonRoomNr[P_HOWARD] = 28;
		} else if (eib_nr == 72) {
			if ((obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) &&
			        spieler.R42BriefOk && spieler.R28Manuskript) ||
			        (spieler.R40TrainOk)) {
				spieler.R40TrainOk = true;
				spieler.PersonRoomNr[P_HOWARD] = 45;
				spieler.room_e_obj[72].Exit = 45;
				obj->hide_sib(SIB_MUENZE_R40);
				uhr->disable_timer();
				out->ausblenden(0);
				hide_person();

				set_up_screen(DO_SETUP);
				out->einblenden(pal, 0);
				uhr->enable_timer();
				maus_links_click = false;
				start_aad_wait(238, -1);
				r40_move_train(1);
				flags.NoPalAfterFlc = true;
				flic_cut(FCUT_073, FLC_MODE);
				show_person();
			} else
				spieler.PersonRoomNr[P_HOWARD] = 42;
		}
	}
	spieler_mi[P_HOWARD].Mode = false;
	show_cur();
}

#define SPEED 3
void r40_move_train(int16 mode) {
	int16 lx, ax;
	int16 delay;
	spieler.R40TrainMove = false;
	hide_cur();
	auto_move(9, P_CHEWY);
	flags.NoScroll = true;
	auto_scroll(232, 0);
	if (!mode)
		start_aad_wait(206, -1);
	lx = -40;
	ax = lx - 190;
	det->start_detail(7, 20, ANI_VOR);
	det->show_static_spr(11);
	if (mode && spieler.ChewyAni == CHEWY_PUMPKIN)
		det->show_static_spr(12);
	delay = 0;
	while (ax < 560) {
		det->set_detail_pos(7, lx, 46);
		det->set_static_pos(11, ax, 62, false, false);
		if (mode && spieler.ChewyAni == CHEWY_PUMPKIN)
			det->set_static_pos(12, ax, 62, false, true);
		if (!delay) {
			lx += SPEED;
			ax += SPEED;
			delay = spieler.DelaySpeed / 2;
		} else
			--delay;
		set_up_screen(DO_SETUP);
	}
	det->stop_detail(7);
	det->hide_static_spr(11);
	det->hide_static_spr(12);
	if (!mode)
		start_aad_wait(207, -1);
	if (!mode)
		auto_scroll(180, 0);
	flags.NoScroll = false;
	show_cur();
}

#define POLICE_LEFT 8
#define POLICE_OFFEN 9
#define POLICE_WUERG 10
#define POLICE_RIGHT 11
#define POLICE_FLASCHE 16

void r40_setup_func() {
	int16 x, y;
	int16 sp_x;

	if (!spieler.R40HoUse && spieler.PersonRoomNr[P_HOWARD] == 40) {
		calc_person_look();
		x = spieler_vector[P_HOWARD].Xypos[0];
		y = spieler_vector[P_HOWARD].Xypos[1];
		sp_x = spieler_vector[P_CHEWY].Xypos[0];
		if (sp_x > 170 && sp_x < 255) {
			x = 248;
			y = 97;
		} else if (sp_x > 255 && sp_x < 350) {
			x = 310;
			y = 90;
		} else if (sp_x > 350) {
			x = 428;
			y = 90;
		} else if (sp_x < 170) {
			x = 165;
			y = 99;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}

	if (spieler.R40PoliceWeg == false) {
		if (spieler.R40PoliceStart) {
			spieler.R40PoliceStart = false;
			spieler.R40PoliceAniStatus = POLICE_LEFT;
			room->set_timer_status(255, TIMER_STOP);
			det->hide_static_spr(15);
			det->start_detail(POLICE_LEFT, 1, ANI_VOR);
			atds->set_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
		}
		switch (spieler.R40PoliceAniStatus) {
		case POLICE_LEFT:
			if (det->get_ani_status(POLICE_LEFT) == false) {
				det->start_detail(POLICE_OFFEN, 1, ANI_VOR);
				spieler.R40PoliceAniStatus = POLICE_OFFEN;
			}
			break;

		case POLICE_OFFEN:
			if (det->get_ani_status(POLICE_OFFEN) == false) {
				det->show_static_spr(0);
				det->start_detail(POLICE_FLASCHE, 1, ANI_VOR);
				spieler.R40PoliceAniStatus = POLICE_FLASCHE;
			}
			break;

		case POLICE_FLASCHE:
			if (det->get_ani_status(POLICE_FLASCHE) == false) {
				det->hide_static_spr(0);
				if (spieler.R40DuengerTele) {
					hide_cur();
					spieler.R40PoliceWeg = true;
					det->start_detail(17, 255, ANI_VOR);
					start_aad_wait(226, -1);
					det->stop_detail(17);
					spieler.R40HoUse = true;
					person_end_phase[P_CHEWY] = P_RIGHT;
					start_detail_wait(10, 1, ANI_VOR);
					person_end_phase[P_HOWARD] = P_RIGHT;
					start_aad_wait(224, -1);
					spieler.R40PoliceWeg = true;
					show_cur();
					flags.MausLinks = false;
					flags.MainInput = true;
					spieler.R40HoUse = false;
					atds->set_steuer_bit(276, ATS_AKTIV_BIT, ATS_DATEI);
				} else {
					det->start_detail(POLICE_RIGHT, 1, ANI_VOR);
					spieler.R40PoliceAniStatus = POLICE_RIGHT;
				}
			}
			break;

		case POLICE_RIGHT:
			if (det->get_ani_status(POLICE_RIGHT) == false) {
				det->show_static_spr(15);
				spieler.R40PoliceAniStatus = 255;
				room->set_timer_status(255, TIMER_START);
				uhr->reset_timer(timer_nr[0], 0);
				atds->del_steuer_bit(275, ATS_AKTIV_BIT, ATS_DATEI);
			}
			break;

		}
	}
}

int16 r40_use_mr_pumpkin() {
	int16 action_ret = false;
	hide_cur();
	if (spieler.inv_cur) {
		switch (spieler.AkInvent) {
		case CENT_INV:
			action_ret = true;
			auto_move(5, P_CHEWY);
			del_inventar(spieler.AkInvent);
			start_detail_wait(15, 1, ANI_VOR);
			start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(200, -1);
			break;

		case CASSETTE_INV:
			if (spieler.R39TvRecord == 6) {
				action_ret = true;
				if (spieler.R40PoliceWeg == false)
					r40_use_schalter(227);
				else {
					hide_cur();
					auto_move(8, P_CHEWY);
					start_spz_wait(CH_PUMP_GET1, 1, ANI_VOR, P_CHEWY);
					del_inventar(spieler.AkInvent);
					out->ausblenden(1);
					r43_catch_pg();
					del_invent_slot(LIKOER_INV);
					obj->add_inventar(LIKOER2_INV, &room_blk);
					inventory_2_cur(LIKOER2_INV);
					switch_room(40);
					start_aad_wait(236, -1);
				}
			} else
				start_aad_wait(228 + spieler.R39TvRecord, -1);
			break;

		}
	}
	show_cur();
	return (action_ret);
}

int16 r40_use_schalter(int16 aad_nr) {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (spieler.R40PoliceWeg == false) {
			action_flag = true;
			hide_cur();
			auto_move(8, P_CHEWY);
			if (spieler.R40PoliceAniStatus != 255) {
				start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(204, -1);
				while (spieler.R40PoliceAniStatus != 255)
					set_up_screen(DO_SETUP);
			}
			room->set_timer_status(255, TIMER_STOP);
			spieler.R40PoliceStart = false;
			stop_spz();
			start_spz_wait(CH_PUMP_GET1, 1, ANI_VOR, P_CHEWY);
			if (spieler.R40PoliceAb) {
				spieler.R40PoliceAb = false;
				stop_spz();
				go_auto_xy(308, 100, P_HOWARD, ANI_WAIT);
				spieler.R40HoUse = false;
			}
			det->hide_static_spr(15);
			start_detail_wait(12, 1, ANI_VOR);
			det->set_static_ani(14, -1);
			start_aad_wait(aad_nr, -1);
			det->del_static_ani(14);
			start_detail_wait(13, 1, ANI_VOR);
			det->show_static_spr(15);
			room->set_timer_status(255, TIMER_START);
			uhr->reset_timer(timer_nr[0], 0);
			show_cur();
		}
	}
	return (action_flag);
}

void r40_talk_police() {
	if (spieler.R40PoliceWeg == false &&
	        spieler.R40PoliceAniStatus == 255) {
		hide_cur();
		spieler.R40PoliceStart = false;
		room->set_timer_status(255, TIMER_STOP);
		auto_move(7, P_CHEWY);
		start_aad_wait(203, -1);
		room->set_timer_status(255, TIMER_START);
		uhr->reset_timer(timer_nr[0], 0);
		show_cur();
	}
}

void r40_talk_handler() {
	if (!spieler.R40HaendlerOk) {
		hide_cur();
		auto_move(6, P_CHEWY);
		det->del_static_ani(4);
		room->set_timer_status(4, TIMER_STOP);
		det->set_static_ani(3, -1);
		start_aad_wait(202, -1);
		det->set_static_ani(4, -1);
		det->del_static_ani(3);
		room->set_timer_status(4, TIMER_START);
		show_cur();
	}
}

int16 r40_use_haendler() {
	int16 action_flag = false;
	if (menu_item == CUR_HOWARD && !spieler.R40HaendlerOk) {
		action_flag = true;
		hide_cur();
		invent_2_slot(DUENGER_INV);
		spieler.R40HoUse = true;
		spieler.R40HaendlerOk = true;
		spieler.R40TeilKarte = true;
		spieler.R40DuengerMit = true;
		atds->set_steuer_bit(283, ATS_AKTIV_BIT, ATS_DATEI);
		auto_move(10, P_CHEWY);
		auto_move(11, P_HOWARD);
		start_aad_wait(208, -1);
		auto_move(6, P_HOWARD);
		flags.NoScroll = true;
		auto_scroll(270, 0);
		det->del_static_ani(4);
		room->set_timer_status(4, TIMER_STOP);
		det->set_static_ani(3, -1);
		start_aad_wait(209, -1);
		det->del_static_ani(3);
		det->set_static_ani(5, -1);
		start_aad_wait(213, -1);
		if (spieler.R28RKuerbis) {
			det->del_static_ani(5);
			det->set_static_ani(3, -1);
			start_aad_wait(211, -1);
			out->ausblenden(0);
			out->set_teilpalette(pal, 255, 1);
			start_aad_wait(212, -1);
			out->ausblenden(0);
			spieler.R40Wettbewerb = true;
			spieler.PersonRoomNr[P_HOWARD] = 28;
			flags.NoScroll = false;
			out->setze_zeiger(0);
			out->cls();
			switch_room(28);
			SetUpScreenFunc = r28setup_func;
		} else {
			auto_move(11, P_HOWARD);
			start_aad_wait(210, -1);
		}
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		show_cur();
		flags.NoScroll = false;
		spieler.R40HoUse = false;
		flags.MausLinks = false;
	}
	return (action_flag);
}

int16 r40_use_bmeister() {
	short action_flag = false;
	if (menu_item == CUR_HOWARD) {
		action_flag = true;
		hide_cur();
		spieler.R40HoUse = true;
		auto_move(9, P_CHEWY);
		auto_move(11, P_HOWARD);
		start_aad_wait(214, -1);
		r40_bmeister_dia(215);
		start_aad_wait(216, -1);
		spieler.R40HoUse = false;
		flags.NoScroll = false;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		show_cur();
	} else if (is_cur_inventar(LIKOER2_INV)) {
		action_flag = true;
		hide_cur();
		spieler.R40HoUse = true;
		new_invent_2_cur(HOTEL_INV);

		r40_bmeister_dia(237);

		show_cur();
	}
	return (action_flag);
}

void r40_bmeister_dia(int16 aad_nr) {
	auto_move(7, P_CHEWY);
	auto_move(12, P_HOWARD);
	flags.NoScroll = true;
	auto_scroll(206, 0);
	det->del_static_ani(0);
	room->set_timer_status(0, TIMER_STOP);
	det->stop_detail(0);
	start_detail_wait(1, 1, ANI_VOR);
	det->set_static_ani(2, -1);
	if (aad_nr == 237) {
		start_aad_wait(aad_nr, -1);
		flags.NoPalAfterFlc = true;
		flic_cut(FCUT_062, FLC_MODE);
		fx_blende = 3;
		start_aad_wait(375, -1);
	} else
		start_aad_wait(aad_nr, -1);
	flags.NoPalAfterFlc = false;
	room->set_timer_status(0, TIMER_START);
	det->set_static_ani(0, -1);
	det->del_static_ani(2);
	auto_move(11, P_HOWARD);
	auto_move(9, P_CHEWY);
}

void r40_use_police() {
	if (menu_item == CUR_HOWARD) {
		if (spieler.R40PoliceWeg == false &&
		        spieler.R40PoliceAniStatus == 255) {
			spieler.R40PoliceAb = true;
			hide_cur();
			spieler.R40PoliceStart = false;
			room->set_timer_status(255, TIMER_STOP);
			spieler.R40HoUse = true;
			auto_move(9, P_CHEWY);
			auto_move(11, P_HOWARD);
			start_aad_wait(217, -1);
			auto_move(8, P_CHEWY);
			go_auto_xy(300, 120, P_HOWARD, ANI_WAIT);
			person_end_phase[P_HOWARD] = P_RIGHT;
			start_aad_wait(218, -1);
			start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
			menu_item = CUR_WALK;
			cursor_wahl(menu_item);
			show_cur();
			flags.MausLinks = false;
		}
	} else
		start_aad_wait(225, -1);
}

int16 r40_use_tele() {
	int16 dia_nr = -1;
	int16 dia_nr1 = -1;
	int16 timer_wert;
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (spieler.R40PoliceWeg == false) {
			action_flag = true;
			hide_cur();
			if (!spieler.R40PoliceAb) {
				start_aad_wait(219, -1);
				spieler.R40HoUse = false;
			} else {
				auto_move(13, P_CHEWY);
				det->show_static_spr(0);
				if (!spieler.R40DuengerMit) {
					dia_nr = 220;
					dia_nr1 = 222;
				} else {
					dia_nr = 221;
					dia_nr1 = 223;
				}
				start_aad_wait(dia_nr, -1);
				auto_move(11, P_HOWARD);
				det->hide_static_spr(0);
				auto_move(9, P_CHEWY);
				start_aad_wait(dia_nr1, -1);
				spieler.R40HoUse = false;
				timer_wert = 0;
				if (dia_nr1 == 223) {
					if (is_cur_inventar(DUENGER_INV)) {
						del_inventar(spieler.AkInvent);
					} else {
						obj->del_inventar(DUENGER_INV, &room_blk);
						del_invent_slot(DUENGER_INV);
					}
					invent_2_slot(LIKOER_INV);
					auto_move(1, P_CHEWY);
					spieler.R40DuengerMit = false;
					spieler.R40DuengerTele = true;
					flags.MausLinks = true;
					flags.MainInput = false;
					timer_wert = 3;
				}
				spieler.R40PoliceAb = false;
				room->set_timer_status(255, TIMER_START);
				uhr->reset_timer(timer_nr[0], timer_wert);
			}
			if (dia_nr1 != 223)
				show_cur();
		}
	}
	return (action_flag);
}

void r41_entry() {
	hide_cur();

	if (!spieler.R41LolaOk) {
		if (!flags.LoadGame) {
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			start_ani_block(2, ablock32);
			room->set_timer_status(0, TIMER_START);
			det->set_static_ani(0, -1);
			start_aad_wait(127, -1);
		} else
			det->start_detail(6, 255, ANI_VOR);
	}
	if (spieler.R41Einbruch) {
		atds->del_steuer_bit(271, ATS_AKTIV_BIT, ATS_DATEI);
		timer_nr[0] = room->set_timer(7, 5);
		det->set_static_ani(7, -1);
		det->show_static_spr(6);
		det->show_static_spr(7);
	}
	spieler.PersonHide[P_HOWARD] = true;
	show_cur();
}

void r41_exit() {
	spieler.PersonHide[P_HOWARD] = false;
	if (spieler.PersonRoomNr[P_HOWARD] == 41)
		spieler.PersonRoomNr[P_HOWARD] = 40;
	if (!spieler.R41TrainCount) {
		spieler.R41TrainCount = 3;
		spieler.R40TrainMove = true;
	}
	--spieler.R41TrainCount;
}

void r41_talk_hoggy1() {
	r41_stop_hoggy();
	auto_move(1, P_CHEWY);
	if (!spieler.R41FirstTalk) {
		r41_first_talk();
	} else if (!spieler.R41Einbruch) {
		show_cur();
		start_ads_wait(11);

	} else if (spieler.R41Einbruch) {
		if (!spieler.R41BruchInfo) {
			spieler.R41BruchInfo = true;
			start_aad_wait(132, -1);
			start_aad_wait(128, -1);
		} else if (spieler.R31SurFurz &&
		         !spieler.R41KuerbisInfo) {
			spieler.R41KuerbisInfo = true;
			start_aad_wait(131, -1);
			auto_move(5, P_CHEWY);
			new_invent_2_cur(TICKET_INV);
		} else
			start_aad_wait(130, -1);
	} else
		start_aad_wait(130, -1);
	r41_start_hoggy();
}

void r41_talk_hoggy2() {
	r41_stop_hoggy();
	auto_move(2, P_CHEWY);
	if (!spieler.R41FirstTalk) {
		r41_first_talk();
	} else if (spieler.R41BruchInfo) {
		if (spieler.R31SurFurz &&
		        !spieler.R41KuerbisInfo) {
			spieler.R41KuerbisInfo = true;
			start_aad_wait(131, -1);
			auto_move(5, P_CHEWY);

			new_invent_2_cur(TICKET_INV);
		} else
			start_aad_wait(129, -1);
	} else {
		start_aad_wait(129, -1);
	}
	r41_start_hoggy();
}

void r41_first_talk() {
	spieler.R41FirstTalk = true;
	start_aad_wait(134, -1);
	atds->set_ats_str(266, TXT_MARK_NAME, 1, ATS_DATEI);
	atds->set_ats_str(265, TXT_MARK_NAME, 1, ATS_DATEI);
}

void r41_start_hoggy() {
	int16 i;
	show_cur();
	for (i = 0; i < 2; i++) {
		room->set_timer_status(i, TIMER_START);
		det->set_static_ani(i, -1);
		det->del_static_ani(i + 3);
	}
}

void r41_stop_hoggy() {
	int16 i;
	hide_cur();
	for (i = 0; i < 2; i++) {
		room->set_timer_status(i, TIMER_STOP);
		det->del_static_ani(i);
		det->stop_detail(i);
		det->set_static_ani(i + 3, -1);
	}
}

int16 r41_use_kasse() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		r41_stop_hoggy();
		auto_move(1, P_CHEWY);
		start_aad_wait(133, -1);
		r41_start_hoggy();
	}
	return (action_flag);
}

int16 r41_use_lola() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (!spieler.R41LolaOk && spieler.R41RepairInfo) {
			hide_cur();
			action_flag = true;
			spieler.R41LolaOk = true;
			auto_move(4, P_CHEWY);
			flic_cut(FCUT_057, FLC_MODE);
			set_person_pos(127, 112, P_CHEWY, P_LEFT);
			det->stop_detail(6);

			atds->set_ats_str(267, 1, ATS_DATEI);
			atds->hide_item(11, 0, 3);
			show_cur();
		}
	}
	return (action_flag);
}

int16 r41_use_brief() {
	int16 action_flag = false;
	if (is_cur_inventar(BRIEF_INV)) {
		action_flag = true;
		r41_stop_hoggy();
		auto_move(6, P_CHEWY);
		start_aad_wait(126, -1);
		r41_start_hoggy();
	} else if (is_cur_inventar(BRIEF2_INV)) {
		action_flag = true;
		auto_move(6, P_CHEWY);
		del_inventar(spieler.AkInvent);
		r41_stop_hoggy();
		start_aad_wait(186, -1);
		r41_start_hoggy();
		atds->set_ats_str(206, 1, ATS_DATEI);
		spieler.R28Briefkasten = true;
		spieler.R40TrainMove = true;
		spieler.R28PostCar = true;
		spieler.R42BriefOk = true;
	}
	return (action_flag);
}

void r41_sub_dia() {
	aad_wait(-1);
	start_aad_wait(161, -1);
	if (spieler.R41LolaOk) {
		start_aad_wait(163, -1);
		atds->hide_item(11, 0, 2);
		stop_ads_dialog();
		auto_move(5, P_CHEWY);
		new_invent_2_cur(PAPIER_INV);
	} else {
		start_aad_wait(162, -1);

	}
}

void r42_entry() {
	if (spieler.PersonRoomNr[P_HOWARD] == 42) {
		SetUpScreenFunc = r42setup_func;
		if (!flags.LoadGame) {
			det->stop_detail(0);
			timer_nr[0] = room->set_timer(8, 5);
			det->set_static_ani(8, -1);
			spieler.R42BeamterWach = true;
			SetUpScreenFunc = r42setup_func;

			set_person_pos(80, 43, P_HOWARD, P_LEFT);

			atds->set_ats_str(263, 1, ATS_DATEI);
			atds->set_ats_str(264, 1, ATS_DATEI);
		}
		if (obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) &&
		        !spieler.R42BriefOk)
			start_aad_wait(302, -1);
		if (obj->check_inventar(HOTEL_INV) && obj->check_inventar(TICKET_INV) &&
		        spieler.R42BriefOk)
			start_aad_wait(301, -1);
	}
}

void r42setup_func() {
	if (!spieler.R42HoToBeamter)
		calc_person_look();
}

void r42_calc_xit() {
	if (spieler.PersonRoomNr[P_HOWARD] == 42) {
		if (flags.ExitMov) {
			atds->set_ats_str(264, 1, ATS_DATEI);
			stop_spz();
			spieler.R42HoToBeamter = false;

		}
	}
}

void r42_get_kuerbis(int16 aad_nr) {
	hide_cur();
	auto_move(1, P_CHEWY);
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_RUECK);
	spieler.PersonHide[P_CHEWY] = false;
	start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(aad_nr, -1);
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(7, 1, ANI_VOR);
	spieler.PersonHide[P_CHEWY] = false;
	show_cur();
}

int16 r42_use_psack() {
	int16 action_flag = false;
	if (!spieler.R42BeamterWach && !spieler.inv_cur) {
		action_flag = true;
		r42_get_kuerbis(136);
	} else if (spieler.R42HoToBeamter &&
	           !spieler.inv_cur &&
	           !spieler.R42MarkeOk) {

		action_flag = true;
		auto_move(3, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		start_aad_wait(187, -1);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(9, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		new_invent_2_cur(BMARKE_INV);
		start_aad_wait(181, -1);
		spieler.R42MarkeOk = true;
		auto_move(4, P_CHEWY);
		start_aad_wait(185, -1);
		spieler.R42HoToBeamter = false;
	} else if (is_cur_inventar(BRIEF2_INV)) {
		action_flag = true;
		auto_move(3, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(10, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		del_inventar(spieler.AkInvent);
		start_aad_wait(183, -1);
		obj->calc_rsi_flip_flop(SIB_BKASTEN_R28);
		atds->set_ats_str(206, 1, ATS_DATEI);
		spieler.R28Briefkasten = true;
		spieler.R40TrainMove = true;
		spieler.R28PostCar = true;
		spieler.R42BriefOk = true;
	} else if (is_cur_inventar(BRIEF_INV)) {
		action_flag = true;
		start_aad_wait(182, -1);
	}
	return (action_flag);
}

int16 r42_use_beamter() {
	int16 action_flag = false;
	if (menu_item == CUR_HOWARD) {
		action_flag = true;
		atds->set_ats_str(264, 2, ATS_DATEI);
		hide_cur();
		auto_move(4, P_CHEWY);
		start_aad_wait(184, -1);
		spieler.R42HoToBeamter = true;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		show_cur();
		start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
		flags.MausLinks = false;
	}
	return (action_flag);
}

void r42_talk_beamter() {
	int16 dia_nr;
	auto_move(1, P_CHEWY);
	if (!spieler.R42BeamterWach) {
		dia_nr = 10;
	} else if (!spieler.R42MarkeOk) {
		dia_nr = 13;
	} else {
		dia_nr = 14;
	}
	start_ads_wait(dia_nr);
}

void r42_dia_beamter(int16 str_end_nr) {
	hide_cur();
	if (str_end_nr != 4) {
		while (atds->aad_get_status() != -1)
			set_up_screen(DO_SETUP);
		spieler.PersonHide[P_CHEWY] = true;
		flags.NoDiaBox = true;
		switch (str_end_nr) {
		case 1:
			start_detail_wait(3, 4, ANI_VOR);
			break;

		case 2:
			start_detail_wait(4, 4, ANI_VOR);
			break;

		case 3:
			start_detail_wait(5, 2, ANI_VOR);
			break;

		}
	} else {
		while (atds->aad_get_status() < 2) {
			set_up_screen(DO_SETUP);
		}
		det->stop_detail(0);
		start_detail_wait(1, 1, ANI_VOR);
		det->start_detail(2, 255, ANI_VOR);
		while (atds->aad_get_status() != -1) {
			set_up_screen(DO_SETUP);
		}
		det->stop_detail(2);
		det->start_detail(0, 255, ANI_VOR);
	}
	spieler.PersonHide[P_CHEWY] = true;
	det->start_detail(6, 255, ANI_VOR);
	start_aad_wait(135, -1);
	det->stop_detail(6);
	flags.NoDiaBox = false;
	spieler.PersonHide[P_CHEWY] = false;
	show_cur();
}

void r43_night_small() {
	int16 i;
	hide_person();
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	switch_room(43);
	ailsnd->stop_mod();
	hide_cur();
	flags.NoScroll = true;
	spieler.ScrollxStep = 1;
	start_aad_wait(188, -1);
	auto_scroll(320, 0);
	start_aad_wait(189, -1);
	auto_scroll(0, 0);
	start_aad_wait(190, -1);
	spieler.ScrollxStep = 16;
	auto_scroll(192, 0);
	for (i = 0; i < 3; i++) {
		if (i)
			spieler.ScrollxStep = 16 / i;
		auto_scroll(128 + i * 16, 0);
		auto_scroll(192, 0);
	}
	spieler.scrollx = 194;
	start_aad_wait(191, -1);
	load_room_music(255);
	flic_cut(FCUT_058, FLC_MODE);
	flags.NoScroll = false;
	spieler.ScrollxStep = 1;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	show_cur();
	show_person();
}

void r43_catch_pg() {
	hide_person();
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	switch_room(43);
	ailsnd->stop_mod();
	hide_cur();
	flags.NoScroll = true;
	spieler.ScrollxStep = 1;
	start_aad_wait(234, -1);
	auto_scroll(194, 0);
	start_aad_wait(235, -1);
	load_room_music(255);
	spieler.R43GetPgLady = true;
	flic_cut(FCUT_058, FLC_MODE);
	flags.NoScroll = false;
	spieler.ScrollxStep = 1;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	show_cur();
	show_person();
}

void r44_look_news() {
	int16 tmpsx, tmpsy;
	int16 tmproom;
	tmproom = spieler.PersonRoomNr[P_CHEWY];
	tmpsx = spieler.scrollx;
	tmpsy = spieler.scrolly;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	hide_person();
	fx_blende = 1;
	spieler.PersonRoomNr[P_CHEWY] = 44;
	room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
	ERROR
	start_aad_wait(172, -1);
	spieler.scrollx = tmpsx;
	spieler.scrolly = tmpsy;
	switch_room(tmproom);

	show_person();
}
