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

#include "chewy/defines.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/episode3.h"

namespace Chewy {

int16 r45_delay;
int16 r50Wasser;
int16 r56koch_flug;
int16 r62Delay;
int16 r62TalkAni;
int16 r64TalkAni;
int16 r63ChewyAni;
int16 r63Schalter;
int16 r63RunDia;
int16 r65tmp_scrollx;
int16 r65tmp_scrolly;
int16 r65tmp_ch_x;
int16 r65tmp_ch_y;
int16 r65tmp_ho_x;
int16 r65tmp_ho_y;

void r45_entry(int16 eib_nr) {
	int16 ch_x, ch_y;
	int16 ho_x, ho_y;
	spieler.ScrollxStep = 2;
	SetUpScreenFunc = r45_setup_func;
	r45_delay = 0;
	zoom_horizont = 150;
	flags.ZoomMov = true;
	zoom_mov_fak = 4;
	spieler.ZoomXy[P_HOWARD][0] = 80;
	spieler.ZoomXy[P_HOWARD][1] = 70;
	if (spieler.PersonRoomNr[P_HOWARD] == 46) {
		spieler_mi[P_HOWARD].Mode = true;
		spieler.PersonRoomNr[P_HOWARD] = 45;
	}

	spieler_mi[P_HOWARD].Mode = true;
	if (!flags.LoadGame) {
		if (spieler.R48TaxiEntry) {
			ch_x = 67;
			ch_y = 146;
			ho_x = 43;
			ho_y = 129;

			spieler.R48TaxiEntry = false;
			maus_links_click = false;
		} else {
			if (eib_nr == 72) {
				ch_x = 259;
				ch_y = 146;
				ho_x = 304;
				ho_y = 130;
				spieler.scrollx = 130;
			} else {
				ch_x = 68;
				ch_y = 132;
				ho_x = 45;
				ho_y = 124;
			}
		}
		set_person_pos(ch_x, ch_y, P_CHEWY, P_LEFT);
		set_person_pos(ho_x, ho_y, P_HOWARD, P_LEFT);
	}

}

void r45_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	if (spieler.PersonRoomNr[P_HOWARD] == 45) {
		spieler_mi[P_HOWARD].Mode = false;
		if (eib_nr == 87) {
			spieler.PersonRoomNr[P_HOWARD] = 40;
		}
	}
	if (eib_nr == 87) {
		flags.NoPalAfterFlc = true;
		flic_cut(FCUT_073, FLC_MODE);
	}
}

#define R45_MAX_PERSON 9

int16 r45_pinfo[R45_MAX_PERSON][4] = {
	{0, 4, 1490, 1500},
	{0, 1, 180, 190},
	{0, 1, 40, 60},
	{0, 1, 40, 150},
	{0, 1, 40, 230},
	{0, 1, 40, 340},
	{0, 1, 49, 50},
	{0, 1, 5, 90},
	{0, 1, 7, 190},
};

void r45_setup_func() {
	int16 i;
	int16 x;
	int16 y;
	int16 ch_x;

	if (menu_display == 0) {
		if (!r45_delay) {
			r45_delay = spieler.DelaySpeed / 2;
			for (i = 0; i < R45_MAX_PERSON; i++) {
				if (r45_pinfo[i][0] == 1) {
					y = Adi[3 + i].y;
					x = Adi[3 + i].x;
					if (i < 6) {
						x -= r45_pinfo[i][1];
						if (x < -30) {
							r45_pinfo[i][0] = 0;
							det->stop_detail(3 + i);
						}
					} else {
						x += r45_pinfo[i][1];
						if (x > 540) {
							r45_pinfo[i][0] = 0;
							det->stop_detail(3 + i);
						}
					}
					det->set_detail_pos(3 + i, x, y);
				} else {
					++r45_pinfo[i][2];
					if (r45_pinfo[i][2] >= r45_pinfo[i][3]) {
						r45_pinfo[i][2] = 0;
						r45_pinfo[i][0] = 1;
						y = Adi[3 + i].y;
						if (i < 6) {
							x = 500;
						} else {
							x = 0;
						}
						det->set_detail_pos(3 + i, x, y);
						det->start_detail(3 + i, 255, ANI_VOR);
					}
				}
			}
		} else
			--r45_delay;
		if (spieler.PersonRoomNr[P_HOWARD] == 45 && HowardMov != 2) {
			calc_person_look();
			x = spieler_vector[P_HOWARD].Xypos[0];
			y = 64;
			ch_x = spieler_vector[P_CHEWY].Xypos[0];
			if (ch_x < 95) {
				x = 18;
				y = 130;
			} else if (ch_x > 240) {
				x = 304;
				y = 130;
			} else {
				x = 176;
				y = 130;
			}
			if (HowardMov && flags.ExitMov) {
				x = 56;
				y = 122;
			}
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 r45_use_taxi() {
	int16 action_ret = false;
	hide_cur();
	auto_move(1, P_CHEWY);
	if (!spieler.inv_cur) {
		if (spieler.ChewyAni == CHEWY_PUMPKIN) {
			action_ret = true;
			r45_talk_taxi(254);
		} else if (!spieler.R45TaxiOk) {
			action_ret = true;
			r45_talk_taxi(260);
		} else if (spieler.R45TaxiOk) {
			action_ret = true;
			r45_taxi_mov();
		}
	} else if (is_cur_inventar(RING_INV)) {
		action_ret = true;
		del_inventar(RING_INV);
		r45_talk_taxi(256);
		spieler.R45TaxiOk = true;
		r45_taxi_mov();
	} else if (is_cur_inventar(UHR_INV)) {
		action_ret = true;
		r45_talk_taxi(400);
	}

	show_cur();
	return action_ret;
}

void r45_talk_taxi(int16 aad_nr) {
	room->set_timer_status(12, TIMER_STOP);
	det->del_static_ani(12);
	start_detail_wait(13, 1, ANI_VOR);
	det->set_static_ani(14, -1);
	start_aad_wait(aad_nr, -1);
	det->del_static_ani(14);
	start_detail_wait(13, 1, ANI_RUECK);
	det->set_static_ani(12, -1);
	room->set_timer_status(12, TIMER_START);
}

void r45_taxi_mov() {
	HowardMov = 2;
	room->set_timer_status(12, TIMER_STOP);
	det->del_static_ani(12);
	det->show_static_spr(11);
	auto_move(3, P_CHEWY);
	spieler.PersonHide[P_CHEWY] = true;
	spieler.R48TaxiPerson[P_CHEWY] = true;
	if (spieler.PersonRoomNr[P_HOWARD] == 45) {
		go_auto_xy(93, 127, P_HOWARD, ANI_WAIT);
		spieler.PersonHide[P_HOWARD] = true;
		spieler.R48TaxiPerson[P_HOWARD] = true;
		spieler.PersonRoomNr[P_HOWARD] = 48;
	}
	det->hide_static_spr(11);
	start_detail_wait(15, 1, ANI_VOR);

	switch_room(48);
}

int16 r45_use_boy() {
	int16 action_ret = false;
	hide_cur();
	auto_move(2, P_CHEWY);
	if (!spieler.R45MagOk) {
		if (is_cur_inventar(DOLLAR175_INV)) {
			action_ret = true;
			new_invent_2_cur(CUTMAG_INV);
			atds->set_ats_str(DOLLAR175_INV, 1, INV_ATS_DATEI);
			start_aad_wait(258, -1);
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			start_detail_wait(1, 1, ANI_VOR);
			room->set_timer_status(0, TIMER_START);
			det->set_static_ani(0, -1);
			spieler.R45MagOk = true;
		}
	} else
		start_aad_wait(259, -1);
	show_cur();
	return action_ret;
}

void r45_talk_boy() {
	int16 aad_nr;
	hide_cur();
	if (!spieler.R45MagOk) {
		auto_move(2, P_CHEWY);
		aad_nr = 257;
	} else
		aad_nr = 259;
	start_aad_wait(aad_nr, -1);
	show_cur();
}

void r46_entry(int16 eib_nr) {
	if (spieler.PersonRoomNr[P_HOWARD] == 45) {
		spieler.PersonRoomNr[P_HOWARD] = 46;
		SetUpScreenFunc = r46setup_func;
		if (eib_nr == 79) {
			set_person_pos(140, 90, P_HOWARD, P_LEFT);
			auto_move(3, P_CHEWY);
			set_person_spr(P_LEFT, P_CHEWY);
		}
	}
	if (spieler.PersonRoomNr[P_HOWARD] == 46) {
		spieler_mi[P_HOWARD].Mode = true;
		SetUpScreenFunc = r46setup_func;
	}
	if (spieler.R47SchlossOk && !spieler.R46GetLeder)
		r46_bodo();
}

void r46_exit() {
	if (spieler.PersonRoomNr[P_HOWARD] == 46) {
		spieler_mi[P_HOWARD].Mode = false;
	}
}

void r46setup_func() {
	int16 x;
	int16 y;
	int16 ch_x;
	calc_person_look();
	x = spieler_vector[P_HOWARD].Xypos[0];
	y = 64;
	ch_x = spieler_vector[P_CHEWY].Xypos[0];
	if (ch_x > 160) {
		x = 260;
	} else
		x = 120;
	if (HowardMov && flags.ExitMov) {
		x = 160;
		y = 200;
	}
	go_auto_xy(x, y, P_HOWARD, ANI_GO);
}

void r46_bodo() {
	hide_cur();
	det->show_static_spr(0);
	person_end_phase[P_CHEWY] = P_RIGHT;
	start_aad_wait(240, -1);
	spieler_mi[P_CHEWY].Mode = true;
	go_auto_xy(42, 76, P_CHEWY, ANI_WAIT);
	start_aad_wait(250, -1);
	auto_move(1, P_CHEWY);
	set_person_spr(P_RIGHT, P_CHEWY);
	start_aad_wait(251, -1);
	auto_move(2, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = false;
	start_detail_wait(0, 1, ANI_VOR);
	det->start_detail(5, 255, ANI_VOR);
	start_aad_wait(241, -1);
	det->stop_detail(5);
	start_detail_wait(2, 1, ANI_VOR);
	start_detail_wait(3, 1, ANI_VOR);
	det->start_detail(4, 255, ANI_VOR);
	start_aad_wait(242, -1);
	det->stop_detail(4);
	start_detail_wait(6, 1, ANI_VOR);
	det->show_static_spr(3);
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(1, 1, ANI_VOR);
	flic_cut(FCUT_065, FLC_MODE);
	flic_cut(FCUT_066, FLC_MODE);
	out->setze_zeiger(0);
	out->cls();
	spieler.PersonHide[P_CHEWY] = false;
	det->hide_static_spr(0);
	det->hide_static_spr(3);
	load_chewy_taf(CHEWY_ROCKER);
	set_person_pos(213, 118, P_CHEWY, P_RIGHT);
	start_aad_wait(243, -1);
	invent_2_slot(UHR_INV);
	invent_2_slot(RING_INV);
	spieler.R46GetLeder = true;
	show_cur();
}

void r46_kloppe() {
	int16 i;
	int16 delay;
	for (i = 0; i < 4; i++) {
		mem->file->select_pool_item(Ci.Handle, FCUT_065);

		flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
		out->setze_zeiger(0);
		out->cls();
		start_aad(244 + i, -1);
		delay = spieler.DelaySpeed * 50;
		atds->print_aad(0, 0);
		while (in->get_switch_code() == 0 && delay) {
			out->skip_frame(1);
			--delay;
		}
	}
	ailsnd->fade_out(0);
	out->cls();
	while (ailsnd->music_playing());
}

int16 r46_use_schloss() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		if (!spieler.R46GetLeder) {
			auto_move(1, P_CHEWY);
			switch_room(47);
		} else
			start_aad_wait(252, -1);
	}
	return action_ret;
}

void r47_entry() {
	hide_person();
	set_person_pos(40, 170, P_CHEWY, P_RIGHT);
	SetUpScreenFunc = r47_set_detail;
}

void r47_exit() {
	show_person();
	set_person_pos(114, 102, P_CHEWY, P_LEFT);
	spieler_mi[P_HOWARD].Mode = true;
}

int16 r47_use_knopf(int16 txt_nr) {
	int16 k_nr = 0;
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		switch (txt_nr) {
		case 286:
			k_nr = 0;
			break;

		case 292:
			k_nr = 1;
			break;

		case 293:
			k_nr = 2;
			break;

		}
		++spieler.R47Schloss[k_nr];
		if (spieler.R47Schloss[k_nr] > 9)
			spieler.R47Schloss[k_nr] = 0;

		if (spieler.R47Schloss[0] == 7 &&
		        spieler.R47Schloss[1] == 6 &&
		        spieler.R47Schloss[2] == 2)
			spieler.R47SchlossOk = true;
		else
			spieler.R47SchlossOk = false;
	}
	return action_ret;
}

void r47_set_detail() {
	int16 i;
	for (i = 0; i < 10; i++)
		det->hide_static_spr(i);
	for (i = 0; i < 3; i++) {
		det->show_static_spr(spieler.R47Schloss[i]);
		det->set_static_pos(spieler.R47Schloss[i], 124 + i * 30, 96 - i * 1, false, true);
		det->plot_static_details(0, 0, spieler.R47Schloss[i], spieler.R47Schloss[i]);
	}
	for (i = 0; i < 10; i++)
		det->hide_static_spr(i);
}

void r48_entry() {
	maus_links_click = false;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	show_cur();
	r48calc_pic();
	SetUpScreenFunc = r48setup_func;
	timer_nr[0] = room->set_timer(255, 20);

}

void r48calc_pic() {
	int16 i;
	for (i = 0; i < 2; i++) {
		atds->set_steuer_bit(312 + i, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.R48Auswahl[3 + i] = 0;
	}
	spieler.R48Auswahl[0] = 1;
	spieler.R48Auswahl[1] = 1;
	spieler.R48Auswahl[2] = 1;

	if (obj->check_inventar(VCARD_INV)) {
		spieler.R48Auswahl[3] = 1;
		atds->del_steuer_bit(312, ATS_AKTIV_BIT, ATS_DATEI);
		det->show_static_spr(4);
	}
	if (obj->check_inventar(KAPPE_INV)) {
		spieler.R48Auswahl[4] = 1;
		atds->del_steuer_bit(313, ATS_AKTIV_BIT, ATS_DATEI);
		det->show_static_spr(4);
	}
}

void r48_frage() {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		hide_cur();
		start_detail_wait(1, 1, ANI_VOR);
		det->show_static_spr(6);
		start_detail_wait(1, 1, ANI_RUECK);
		det->hide_static_spr(6);
		uhr->reset_timer(timer_nr[0], 0);
		show_cur();
		flags.AutoAniPlay = false;
	}
}

void r48setup_func() {
	int16 idx;
	int16 r_nr;
	int16 i;
	for (i = 0; i < 5; i++)
		det->hide_static_spr(1 + i);
	if (flags.ShowAtsInvTxt) {
		if (menu_display == 0) {
			if (menu_item != CUR_USE) {
				menu_item = CUR_USE;
			}
			cur_2_inventory();
			cursor_wahl(CUR_ZEIGE);
			idx = det->maus_vector(minfo.x, minfo.y);
			if (idx != -1) {
				if (spieler.R48Auswahl[idx]) {
					det->show_static_spr(1 + idx);
					if (maus_links_click) {
						switch (idx) {
						case 0:
							r_nr = 45;
							break;

						case 1:
							r_nr = 49;
							break;

						case 2:
							r_nr = 54;
							break;

						case 3:
							r_nr = 57;
							break;

						case 4:
							r_nr = 56;
							break;

						default:
							r_nr = -1;
							break;

						}
						if (r_nr != -1) {
							SetUpScreenFunc = 0;
							det->hide_static_spr(1 + idx);
							hide_cur();
							room->set_timer_status(255, TIMER_STOP);
							room->set_timer_status(0, TIMER_STOP);
							det->stop_detail(0);
							det->del_static_ani(0);
							start_detail_wait(2, 1, ANI_VOR);
							menu_item = CUR_WALK;
							cursor_wahl(menu_item);
							show_cur();
							spieler.R48TaxiEntry = true;
							maus_links_click = false;
							set_up_screen(DO_SETUP);
							for (i = 0; i < MAX_PERSON; i++) {
								if (spieler.R48TaxiPerson[i]) {
									spieler.PersonHide[i] = false;
									spieler.R48TaxiPerson[i] = false;
								}
							}
							if (spieler.PersonRoomNr[P_HOWARD] == 48) {
								spieler.PersonRoomNr[P_HOWARD] = r_nr;
							}
							switch_room(r_nr);
						}
					}
				}
			}
		}
	}
}

void r49_entry(int16 eib_nr) {
	zoom_horizont = 110;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	spieler.ScrollxStep = 2;
	SetUpScreenFunc = r49setup_func;
	if (!spieler.R49BoyWeg)
		timer_nr[0] = room->set_timer(255, 2);
	else {
		det->del_static_ani(0);
		det->hide_static_spr(5);
	}

	spieler.ZoomXy[P_HOWARD][0] = 30;
	spieler.ZoomXy[P_HOWARD][1] = 30;
	spieler_mi[P_HOWARD].Mode = true;
	if (!flags.LoadGame) {
		if (spieler.R48TaxiEntry) {
			spieler.R48TaxiEntry = false;
			set_person_pos(527, 76, P_HOWARD, P_LEFT);
			set_person_pos(491, 98, P_CHEWY, P_LEFT);
			maus_links_click = false;
			spieler.scrollx = 320;
			spieler.scrolly = 0;
			det->start_detail(5, 1, ANI_VOR);
		} else if (eib_nr == 83)
			set_person_pos(377, 78, P_HOWARD, P_LEFT);
	}

}

void r49_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	if (spieler.PersonRoomNr[P_HOWARD] == 49) {
		spieler_mi[P_HOWARD].Mode = false;
		if (eib_nr == 80)
			spieler.PersonRoomNr[P_HOWARD] = 50;
		else if (eib_nr == 81) {
			SetUpScreenFunc = 0;
			start_aad_wait(268, -1);
		}
	}
}

void r49_calc_boy_ani() {
	if (!spieler.R49BoyAniCount) {
		spieler.R49BoyAniCount = 3;
		spieler.R49BoyAni = 1;
		det->del_static_ani(0);
	} else {
		--spieler.R49BoyAniCount;
		spieler.R49BoyAni = 0;
		det->del_static_ani(1);
	}
	det->set_static_ani(spieler.R49BoyAni, -1);
	det->start_detail(spieler.R49BoyAni, 1, ANI_VOR);
	uhr->reset_timer(timer_nr[0], 0);
}

void r49_calc_boy() {
	if (!spieler.R49WegFrei) {
		hide_cur();
		room->set_timer_status(255, TIMER_STOP);
		uhr->reset_timer(timer_nr[0], 0);
		stop_person(P_CHEWY);
		stop_person(P_HOWARD);
		person_end_phase[P_CHEWY] = P_LEFT;
		det->stop_detail(spieler.R49BoyAni);

		det->del_static_ani(spieler.R49BoyAni);
		det->set_static_ani(2, -1);
		SetUpScreenFunc = 0;
		start_aad_wait(262, -1);
		SetUpScreenFunc = r49setup_func;
		SetUpScreenFunc = 0;
		auto_move(3, P_CHEWY);
		go_auto_xy(374, 79, P_HOWARD, ANI_WAIT);
		set_person_spr(P_LEFT, P_HOWARD);
		det->del_static_ani(2);
		start_detail_wait(3, 1, ANI_VOR);
		start_detail_wait(4, 1, ANI_RUECK);
		SetUpScreenFunc = r49setup_func;
		det->set_static_ani(spieler.R49BoyAni, -1);
		room->set_timer_status(255, TIMER_START);
		show_cur();
	}
}

int16 r49_use_boy() {
	int16 tmp;
	int16 action_ret = false;
	if (is_cur_inventar(SPARK_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		del_inventar(spieler.AkInvent);
		r49_talk_boy(265);
		spieler.R49WegFrei = true;
		spieler.room_e_obj[80].Attribut = AUSGANG_OBEN;
		show_cur();
	} else if (is_cur_inventar(CIGAR_INV)) {
		action_ret = true;
		hide_cur();
		del_inventar(spieler.AkInvent);
		r49_talk_boy(263);
		SetUpScreenFunc = 0;
		auto_move(5, P_CHEWY);
		tmp = room->room_info->ZoomFak;
		room->set_zoom(0);
		go_auto_xy(416, 79, P_HOWARD, ANI_WAIT);
		set_person_spr(P_LEFT, P_HOWARD);
		flags.NoScroll = true;
		auto_scroll(164, 0);
		flic_cut(FCUT_067, FLC_MODE);
		room->set_timer_status(255, TIMER_STOP);
		uhr->reset_timer(timer_nr[0], 0);
		det->del_static_ani(spieler.R49BoyAni);
		det->stop_detail(spieler.R49BoyAni);
		flags.NoScroll = false;

		set_person_spr(P_RIGHT, P_CHEWY);
		start_aad_wait(264, -1);
		room->set_zoom(tmp);
		obj->add_inventar(GUM_INV, &room_blk);
		inventory_2_cur(GUM_INV);
		atds->set_steuer_bit(318, ATS_AKTIV_BIT, ATS_DATEI);
		SetUpScreenFunc = r49setup_func;
		spieler.R49BoyWeg = true;
		show_cur();
	}
	return action_ret;
}

void r49_talk_boy() {
	if (!spieler.R49BoyWeg) {
		auto_move(3, P_CHEWY);
		r49_talk_boy(266 + spieler.R49WegFrei);
	}
}

void r49_talk_boy(int16 aad_nr) {
	if (!spieler.R49BoyWeg) {
		room->set_timer_status(255, TIMER_STOP);
		uhr->reset_timer(timer_nr[0], 0);
		det->stop_detail(spieler.R49BoyAni);

		det->set_static_ani(spieler.R49BoyAni, -1);
		det->set_static_ani(2, -1);
		SetUpScreenFunc = 0;
		stop_person(P_HOWARD);
		start_aad_wait(aad_nr, -1);
		SetUpScreenFunc = r49setup_func;
		det->del_static_ani(2);
		det->set_static_ani(spieler.R49BoyAni, -1);
		room->set_timer_status(255, TIMER_START);
	}
}

void r49_look_hotel() {
	if (spieler.PersonRoomNr[P_HOWARD] == 49) {

		SetUpScreenFunc = 0;
		stop_person(P_HOWARD);
		start_aad_wait(261, -1);
		SetUpScreenFunc = r49setup_func;
	}
}

int16 r49_use_taxi() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		hide_cur();
		start_ani_block(2, ablock34);
		det->show_static_spr(7);
		auto_move(2, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		spieler.R48TaxiPerson[P_CHEWY] = true;
		if (spieler.PersonRoomNr[P_HOWARD] == 49) {
			go_auto_xy(507, 74, P_HOWARD, ANI_WAIT);
			go_auto_xy(490, 58, P_HOWARD, ANI_WAIT);
			spieler.PersonHide[P_HOWARD] = true;
			spieler.R48TaxiPerson[P_HOWARD] = true;
			spieler.PersonRoomNr[P_HOWARD] = 48;
		}
		det->hide_static_spr(7);
		start_detail_wait(5, 1, ANI_VOR);
		switch_room(48);
	}
	return action_ret;
}

void r49setup_func() {
	int16 x, y;
	int16 ch_x;
	if (spieler.PersonRoomNr[P_HOWARD] == 49) {
		calc_person_look();
		x = spieler_vector[P_HOWARD].Xypos[0];
		y = 64;
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		if (ch_x < 130) {
			x = 40;
			y = 97;
		} else if (ch_x < 312) {
			x = 221;
			y = 94;
		} else if (ch_x < 445) {
			x = 342;
			y = 93;
		} else {
			x = 536;
			y = 90;
		}

		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

void r50_entry(int16 eib_nr) {
	spieler.ScrollxStep = 2;
	if (spieler.R50Zigarre) {
		r50_stop_cigar();
	} else
		det->start_detail(0, 255, ANI_VOR);
	if (!spieler.R50KeyOK) {
		timer_nr[0] = room->set_timer(1, 8);
		r50Wasser = false;
	} else
		det->show_static_spr(4);
	SetUpScreenFunc = r50setup_func;
	if (spieler.PersonRoomNr[P_HOWARD] == 50) {
		spieler_mi[P_HOWARD].Mode = true;
		if (!flags.LoadGame) {
			if (eib_nr == 85)
				set_person_pos(298, 56, P_HOWARD, P_RIGHT);
			else
				set_person_pos(1, 64, P_HOWARD, P_RIGHT);
		}
	}
}

void r50_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	if (spieler.PersonRoomNr[P_HOWARD] == 50) {
		if (eib_nr == 83)
			spieler.PersonRoomNr[P_HOWARD] = 49;
		else
			spieler.PersonRoomNr[P_HOWARD] = 51;
		spieler_mi[P_HOWARD].Mode = false;
	}
}

void r50_stop_cigar() {
	room->set_timer_status(0, TIMER_STOP);
	det->del_static_ani(0);
	atds->set_ats_str(328, 1, ATS_DATEI);
	det->stop_detail(0);
	spieler.R50Zigarre = true;
}

void r50_calc_wasser() {
	if (!r50Wasser) {
		r50_stop_page();
		det->start_detail(4, 1, ANI_VOR);
		r50Wasser = true;
	}
}

void r50_stop_page() {
	room->set_timer_status(3, TIMER_STOP);
	det->del_static_ani(3);
	det->stop_detail(3);
	det->stop_detail(4);
}

void r50_go_page() {
	det->set_static_ani(3, -1);
	room->set_timer_status(3, TIMER_START);
}

void r50_calc_treppe() {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!spieler.R50KeyOK) {
			hide_cur();
			stop_person(P_CHEWY);
			room->set_timer_status(1, TIMER_STOP);
			r50Wasser = false;
			r50_stop_page();
			det->set_static_ani(5, -1);
			start_aad_wait(272, -1);
			auto_move(3, P_CHEWY);
			r50_aad_page(273, 5);
		}
		flags.AutoAniPlay = false;
	}
}

int16 r50_use_gutschein() {
	int16 action_ret = false;
	if (is_cur_inventar(HOTEL_INV)) {
		action_ret = true;
		if (!spieler.R50KeyOK) {
			hide_cur();
			auto_move(3, P_CHEWY);
			room->set_timer_status(1, TIMER_STOP);
			r50Wasser = false;
			r50_stop_page();
			del_inventar(spieler.AkInvent);
			start_ani_block(2, ablock36);
			r50_aad_page(274, 8);
		} else
			start_aad_wait(276, -1);
	}
	return action_ret;
}

int16 r50_use_gum() {
	int16 action_ret = false;
	if (is_cur_inventar(GUM_INV)) {
		action_ret = true;
		hide_cur();
		spieler.R50KeyOK = true;
		room->set_timer_status(1, TIMER_STOP);
		r50Wasser = false;
		r50_stop_page();
		start_detail_wait(6, 1, ANI_VOR);
		det->set_static_ani(5, -1);
		auto_move(3, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(75, 92, P_CHEWY, ANI_WAIT);
		SetUpScreenFunc = 0;
		go_auto_xy(112, 57, P_HOWARD, ANI_WAIT);
		set_person_spr(P_LEFT, P_HOWARD);
		del_inventar(spieler.AkInvent);
		hide_person();
		start_detail_wait(2, 1, ANI_VOR);
		det->show_static_spr(4);
		start_detail_wait(2, 1, ANI_RUECK);
		show_person();
		set_person_spr(P_LEFT, P_CHEWY);
		start_aad_wait(275, -1);
		det->del_static_ani(5);
		start_detail_wait(9, 1, ANI_VOR);
		det->del_static_ani(5);
		det->set_static_ani(10, -1);
		start_aad_wait(277, -1);
		SetUpScreenFunc = r50setup_func;
		auto_move(3, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = true;
		det->del_static_ani(10);
		r50_go_page();
		obj->add_inventar(KEY_INV, &room_blk);
		inventory_2_cur(KEY_INV);
		atds->set_ats_str(323, 1, ATS_DATEI);
		atds->set_ats_str(327, 1, ATS_DATEI);
		spieler.room_e_obj[84].Attribut = AUSGANG_OBEN;
		show_cur();
	}
	return action_ret;
}

void r50_talk_page() {
	start_aad_wait(276, -1);
}

void r50_aad_page(int16 aad_nr, int16 ani_nr) {
	det->set_static_ani(ani_nr, -1);
	start_aad_wait(aad_nr, -1);
	det->del_static_ani(ani_nr);
	start_detail_wait(6, 1, ANI_RUECK);
	r50_go_page();
	if (!spieler.R50KeyOK)
		room->set_timer_status(1, TIMER_START);
	show_cur();
}

void r50setup_func() {
	int16 x, y;
	int16 ch_x;
	if (r50Wasser && !det->get_ani_status(4)) {
		r50Wasser = false;
		r50_go_page();
	}
	if (spieler.PersonRoomNr[P_HOWARD] == 50) {
		calc_person_look();
		x = spieler_vector[P_HOWARD].Xypos[0];
		y = 64;
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		if (ch_x < 72) {
			x = 1;
			y = 64;
		} else if (ch_x < 275 && spieler_vector[P_CHEWY].Xypos[1] > 72) {
			x = 143;
			y = 57;
		} else {
			x = 162;
			y = 115;
		}
		if (HowardMov && flags.ExitMov) {
			SetUpScreenFunc = 0;
			HowardMov = 0;
			auto_move(4, P_HOWARD);
		} else
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

void r51_entry() {
	int16 i;
	zoom_horizont = 140;
	flags.ZoomMov = true;
	zoom_mov_fak = 4;
	for (i = 0; i < 2; i++)
		det->start_detail(3 + i, 1, ANI_VOR);
	if (spieler.PersonRoomNr[P_HOWARD] == 51) {
		spieler.ZoomXy[P_HOWARD][0] = 40;
		spieler.ZoomXy[P_HOWARD][1] = 30;
		if (!flags.LoadGame) {
			set_person_pos(88, 93, P_HOWARD, P_RIGHT);
		}
		if (!spieler.R51FirstEntry) {
			hide_cur();
			spieler.R51FirstEntry = true;
			set_person_spr(P_LEFT, P_CHEWY);
			start_aad_wait(283, -1);
			show_cur();
		}
		SetUpScreenFunc = r51setup_func;
		spieler_mi[P_HOWARD].Mode = true;
	}
}

void r51_exit(int16 eib_nr) {
	if (spieler.PersonRoomNr[P_HOWARD] == 51) {
		if (eib_nr == 85) {
			spieler.PersonRoomNr[P_HOWARD] = 50;
		} else
			spieler.PersonRoomNr[P_HOWARD] = 52;
		spieler_mi[P_HOWARD].Mode = false;
	}
}

void r51setup_func() {
	int16 x, y;
	int16 ch_y;
	if (spieler.PersonRoomNr[P_HOWARD] == 51) {
		calc_person_look();
		x = spieler_vector[P_HOWARD].Xypos[0];
		y = 64;
		ch_y = spieler_vector[P_CHEWY].Xypos[1];
		if (ch_y < 129) {
			x = 56;
			y = 106;
		} else {
			x = 31;
			y = 118;
		}
		if (HowardMov && flags.ExitMov) {
			SetUpScreenFunc = 0;
			HowardMov = 0;
			auto_move(9, P_HOWARD);
		} else
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

int16 r51_use_door(int16 txt_nr) {
	int16 action_ret = false;
	if (is_cur_inventar(KEY_INV)) {
		hide_cur();
		action_ret = true;
		switch (txt_nr) {
		case 329:
			auto_move(8, P_CHEWY);
			SetUpScreenFunc = 0;
			det->show_static_spr(0);
			if (!spieler.R51HotelRoom) {
				auto_move(11, P_HOWARD);
				set_person_spr(P_LEFT, P_HOWARD);
				spieler.R51HotelRoom = true;
				spieler.room_e_obj[86].Attribut = AUSGANG_LINKS;
				start_aad_wait(285, -1);
				atds->set_ats_str(329, 1, ATS_DATEI);
				SetUpScreenFunc = r51setup_func;
			} else {
				show_cur();
				switch_room(52);
			}
			break;

		case 330:
			auto_move(9, P_CHEWY);
			if (!spieler.R51KillerWeg) {
				det->show_static_spr(1);
				start_detail_wait(2, 1, ANI_VOR);
				det->start_detail(5, 255, ANI_VOR);
				if (!spieler.R52HotDogOk) {
					start_aad_wait(287, -1);
					auto_move(12, P_CHEWY);
					det->stop_detail(5);
					start_ani_block(5, ablock37);
					det->hide_static_spr(1);
					start_aad_wait(284, -1);
				} else {
					spieler.R51KillerWeg = true;
					start_aad_wait(290, -1);
					out->ausblenden(1);
					out->setze_zeiger(0);
					out->cls();
					out->einblenden(pal, 0);
					flags.NoPalAfterFlc = true;
					flc->set_flic_user_function(r51_cut_serv);
					det->show_static_spr(16);
					flic_cut(FCUT_068, FLC_MODE);
					flc->remove_flic_user_function();
					det->hide_static_spr(16);
					flags.NoPalAfterFlc = false;
					det->stop_detail(5);
					obj->show_sib(SIB_AUSRUEST_R52);
					obj->calc_rsi_flip_flop(SIB_AUSRUEST_R52);
					det->hide_static_spr(1);

					fx_blend = BLEND3;
					set_up_screen(DO_SETUP);
					start_aad_wait(291, -1);
				}
			} else
				start_aad_wait(401, -1);
			break;

		case 331:
			auto_move(10, P_CHEWY);
			break;

		case 332:
			auto_move(6, P_CHEWY);
			start_aad_wait(286, -1);
			break;

		case 333:
			auto_move(4, P_CHEWY);
			switch (spieler.R51DoorCount) {
			case 0:
				det->show_static_spr(3);
				start_aad_wait(278, -1);
				start_detail_frame(0, 1, ANI_VOR, 3);
				start_spz(HO_BRILL_JMP, 1, ANI_VOR, P_HOWARD);
				wait_detail(0);

				det->show_static_spr(14);
				start_aad_wait(279, -1);
				++spieler.R51DoorCount;
				obj->show_sib(SIB_FLASCHE_R51);
				obj->calc_rsi_flip_flop(SIB_FLASCHE_R51);
				det->hide_static_spr(3);
				break;

			case 1:
				det->show_static_spr(3);
				start_aad_wait(280, -1);
				start_detail_wait(1, 1, ANI_VOR);
				++spieler.R51DoorCount;
				obj->show_sib(SIB_KAPPE_R51);
				obj->calc_rsi_flip_flop(SIB_KAPPE_R51);
				det->hide_static_spr(3);
				det->show_static_spr(15);
				break;

			default :
				start_aad_wait(281, -1);
				if (spieler.PersonRoomNr[P_HOWARD] == 51)
					start_aad_wait(282, -1);
				break;

			}
			break;

		case 334:
			auto_move(7, P_CHEWY);
			break;

		}
		show_cur();
	}
	return action_ret;
}

int16 r51_cut_serv(int16 frame) {
	det->plot_static_details(0, 0, 16, 16);
	return 0;
}

void r51_timer_action(int16 t_nr, int16 obj_nr) {
	// TODO
}

void r52_entry() {
	SetUpScreenFunc = r52setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	if (spieler.R52HotDogOk && !spieler.R52KakerWeg)
		r52_plot_armee(0);
	if (spieler.R52KakerWeg)
		det->stop_detail(0);
	if (!flags.LoadGame) {
		det->show_static_spr(4);
		spieler.R52TuerAuf = true;
		set_person_pos(20, 50, P_HOWARD, P_LEFT);
		set_person_pos(35, 74, P_CHEWY, P_RIGHT);
		auto_move(2, P_CHEWY);
		spieler.R52TuerAuf = false;
		det->hide_static_spr(4);
		check_shad(2, 1);
	}
}

void r52_exit() {
	if (spieler.PersonRoomNr[P_HOWARD] == 52) {
		spieler.PersonRoomNr[P_HOWARD] = 51;
		spieler_mi[P_HOWARD].Mode = false;
	}
}

int16 r52_use_hot_dog() {
	int16 i;
	int16 action_ret = false;
	hide_cur();
	if (is_cur_inventar(BURGER_INV)) {
		action_ret = true;
		auto_move(3, P_CHEWY);
		start_spz_wait(CH_ROCK_GET1, 1, ANI_VOR, P_CHEWY);
		det->show_static_spr(0);
		del_inventar(spieler.AkInvent);
		auto_move(4, P_CHEWY);
		spieler.R52HotDogOk = true;
		r52_plot_armee(20);
		atds->set_ats_str(341, 1, ATS_DATEI);
		auto_move(2, P_CHEWY);
		set_person_spr(P_LEFT, P_CHEWY);
		start_aad_wait(288, -1);
	} else if (is_cur_inventar(KILLER_INV)) {
		action_ret = true;
		auto_move(5, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(7, 1, ANI_VOR);
		det->start_detail(8, 255, ANI_VOR);
		for (i = 0; i < 5; i++) {
			wait_show_screen(20);
			det->stop_detail(2 + i);
		}
		det->stop_detail(0);
		det->stop_detail(8);
		start_detail_wait(7, 1, ANI_RUECK);
		spieler.PersonHide[P_CHEWY] = false;
		atds->set_steuer_bit(341, ATS_AKTIV_BIT, ATS_DATEI);
		start_aad_wait(303, -1);
		atds->set_ats_str(KILLER_INV, 1, INV_ATS_DATEI);
		spieler.R52KakerWeg = true;
	}
	show_cur();
	return action_ret;
}

void r52_plot_armee(int16 frame) {
	int16 i;
	for (i = 0; i < 5; i++) {
		wait_show_screen(frame);
		det->start_detail(2 + i, 255, ANI_VOR);
	}
}

void r52_kaker_platt() {
	if (!spieler.R52KakerJoke &&
	        spieler.R52HotDogOk &&
	        !spieler.R52KakerWeg &&
	        !flags.ExitMov) {
		spieler.R52KakerJoke = true;
		stop_person(P_CHEWY);
		start_aad_wait(289, -1);
	}
}

void r52setup_func() {
	int16 x, y;
	int16 ch_y;
	if (spieler.PersonRoomNr[P_HOWARD] == 52) {
		calc_person_look();
		x = 1;
		ch_y = spieler_vector[P_CHEWY].Xypos[1];
		if (ch_y < 97) {
			y = 44;
		} else {
			y = 87;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

void r53_entry() {
	hide_cur();
	obj->hide_sib(SIB_VISIT_R53);
	atds->del_steuer_bit(319, ATS_AKTIV_BIT, ATS_DATEI);
	start_detail_wait(0, 1, ANI_VOR);
	det->set_static_ani(1, -1);
	timer_nr[0] = room->set_timer(1, 7);
	show_cur();
}

void r53_man_go() {
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	start_detail_wait(5, 1, ANI_VOR);
	atds->set_steuer_bit(319, ATS_AKTIV_BIT, ATS_DATEI);
	if (!spieler.R53Visit)
		obj->show_sib(SIB_VISIT_R53);
}

void r53_talk_man() {
	hide_cur();
	auto_move(2, P_CHEWY);
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	start_detail_wait(2, 1, ANI_VOR);
	det->set_static_ani(3, -1);
	start_aad_wait(269 + (int16)spieler.R53Kostuem, -1);
	det->del_static_ani(3);
	start_detail_wait(4, 1, ANI_VOR);
	r53_man_go();
	show_cur();
}

int16 r53_use_man() {
	int16 action_ret = false;
	if (is_cur_inventar(BESTELL_INV)) {
		action_ret = true;
		hide_cur();
		spieler.R53Kostuem = true;
		del_inventar(spieler.AkInvent);
		auto_move(2, P_CHEWY);
		start_aad_wait(271, -1);
		room->set_timer_status(1, TIMER_STOP);
		det->del_static_ani(1);
		start_ani_block(7, ablock35);
		obj->add_inventar(JMKOST_INV, &room_blk);
		inventory_2_cur(JMKOST_INV);
		atds->set_steuer_bit(319, ATS_AKTIV_BIT, ATS_DATEI);
		show_cur();
	}
	return action_ret;
}

void r54_entry(int16 eib_nr) {
	spieler.ScrollxStep = 2;
	SetUpScreenFunc = r54_setup_func;
	zoom_horizont = 106;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	spieler.ZoomXy[P_HOWARD][0] = 30;
	spieler.ZoomXy[P_HOWARD][1] = 66;
	if (spieler.R54FputzerWeg)
		det->show_static_spr(9);
	spieler_mi[P_HOWARD].Mode = true;
	if (!flags.LoadGame) {
		if (spieler.R48TaxiEntry) {
			spieler.R48TaxiEntry = false;
			if (spieler.PersonRoomNr[P_HOWARD] == 54) {
				spieler.R54HowardVorne = false;
				set_person_pos(290, 61, P_HOWARD, P_RIGHT);
			}
			set_person_pos(300, 80, P_CHEWY, P_LEFT);
			maus_links_click = false;
			spieler.scrollx = 134;
			spieler.scrolly = 0;
		} else if (spieler.R55Location) {
			r54_aufzug_ab();
		} else if (eib_nr == 90 || spieler.R55ExitDia) {
			if (spieler.PersonRoomNr[P_HOWARD] == 54) {
				spieler.R54HowardVorne = false;
				set_person_pos(212, 61, P_HOWARD, P_RIGHT);
			}
			spieler.scrollx = 66;
			set_person_pos(241, 85, P_CHEWY, P_LEFT);
		}
		if (spieler.R55ExitDia) {
			start_aad_wait(spieler.R55ExitDia, -1);
		}
		if (spieler.R55R54First) {
			spieler.R55R54First = false;
			r54_calc_auto_taxi();
		}
	}
}

void r54_exit(int16 eib_nr) {
	spieler.ScrollxStep = 1;
	if (eib_nr == 89 && spieler.PersonRoomNr[P_HOWARD] == 54) {
		spieler.PersonRoomNr[P_HOWARD] = 55;
		spieler_mi[P_HOWARD].Mode = false;
	}
}

void r54_calc_auto_taxi() {
	int16 r_nr;
	r_nr = 0;
	if (spieler.R56AbfahrtOk) {
		r_nr = 57;
	} else if (obj->check_inventar(KAPPE_INV)) {
		r_nr = 56;
	}
	if (r_nr) {
		hide_cur();
		auto_move(7, P_CHEWY);
		spieler.R54HowardVorne = false;
		go_auto_xy(290, 61, P_HOWARD, ANI_WAIT);
		spieler.PersonRoomNr[P_HOWARD] = r_nr;
		maus_links_click = false;
		show_cur();
		spieler.R48TaxiEntry = true;
		switch_room(r_nr);
	}
}

void r54_setup_func() {
	int16 x, y;
	int16 ch_x;
	if (spieler.PersonRoomNr[P_HOWARD] == 54) {
		calc_person_look();
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		if (spieler.R54HowardVorne != 255) {
			if (spieler.R54HowardVorne) {
				if (ch_x < 218) {
					x = 150;
					y = 113;
				} else {
					x = 287;
					y = 115;
				}
			} else {
				y = 61;
				if (ch_x < 167) {
					x = 122;
				} else if (ch_x < 251) {
					x = 212;
				} else {
					x = 290;
				}
			}
			go_auto_xy(x, y, P_HOWARD, ANI_GO);
		}
	}
}

int16 r54_use_schalter() {
	int16 aad_nr;
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		hide_cur();
		action_ret = true;
		if (!spieler.R54FputzerWeg) {
			auto_move(1, P_CHEWY);
			spieler.R54HowardVorne = 255;
			go_auto_xy(127, 76, P_HOWARD, ANI_WAIT);
			go_auto_xy(14, 72, P_HOWARD, ANI_GO);
			start_spz_wait(CH_ROCK_GET2, 1, ANI_VOR, P_CHEWY);
			det->show_static_spr(0);
			auto_move(2, P_CHEWY);
			start_detail_wait(1, 1, ANI_VOR);
			det->start_detail(3, 255, ANI_VOR);
			start_aad_wait(292 + spieler.R54LiftCount, -1);
			det->stop_detail(3);
			++spieler.R54LiftCount;
			if (spieler.R54LiftCount < 3) {
				start_detail_wait(2, 1, ANI_VOR);
				det->hide_static_spr(0);
				start_detail_wait(1, 1, ANI_RUECK);
				aad_nr = 295;
			} else {
				start_detail_wait(5, 1, ANI_VOR);
				det->show_static_spr(9);
				start_detail_wait(4, 1, ANI_VOR);
				aad_nr = 296;
				spieler.R54FputzerWeg = true;
				atds->del_steuer_bit(345, ATS_AKTIV_BIT, ATS_DATEI);
				atds->set_ats_str(349, 1, ATS_DATEI);
				atds->set_ats_str(345, 1, ATS_DATEI);
			}
			start_aad_wait(aad_nr, -1);
			spieler.R54HowardVorne = false;
		} else
			start_aad_wait(297, -1);
		show_cur();
	}
	return action_ret;
}

void r54_talk_verkauf() {
	hide_cur();
	if (!spieler.R54HotDogOk) {
		if (spieler.AkInvent == DOLLAR175_INV)
			del_inventar(DOLLAR175_INV);
		else
			del_invent_slot(DOLLAR175_INV);
		spieler.R54HotDogOk = true;
		auto_move(3, P_CHEWY);
		spieler.R54HowardVorne = true;
		start_aad_wait(299, -1);
		room->set_timer_status(6, TIMER_STOP);
		det->del_static_ani(6);
		start_detail_wait(7, 1, ANI_VOR);
		det->start_detail(8, 255, ANI_VOR);
		start_aad_wait(310, -1);
		det->stop_detail(8);
		start_detail_wait(9, 1, ANI_VOR);
		start_detail_wait(10, 1, ANI_VOR);
		det->start_detail(11, 255, ANI_VOR);
		start_aad_wait(311, -1);
		det->stop_detail(11);
		room->set_timer_status(6, TIMER_START);
		det->set_static_ani(6, -1);
		auto_move(4, P_CHEWY);
		start_aad_wait(312, -1);
		obj->add_inventar(BURGER_INV, &room_blk);
		inventory_2_cur(BURGER_INV);
	} else
		start_aad_wait(313, -1);
	show_cur();
}

int16 r54_use_zelle() {
	int16 action_ret = false;
	hide_cur();
	if (spieler.inv_cur) {
		if (is_cur_inventar(JMKOST_INV)) {
			action_ret = true;
			if (!spieler.R54Schild) {
				start_aad_wait(318, -1);
			} else {
				auto_move(5, P_CHEWY);
				spieler.R54HowardVorne = true;
				SetUpScreenFunc = 0;
				auto_scroll(176, 0);
				go_auto_xy(239, 101, P_HOWARD, ANI_WAIT);
				flc->set_flic_user_function(r54_cut_serv);
				flic_cut(FCUT_069, FLC_MODE);
				flc->remove_flic_user_function();
				del_inventar(spieler.AkInvent);
				invent_2_slot(LEDER_INV);
				load_chewy_taf(CHEWY_JMANS);
				zoom_horizont = 90;
				set_person_pos(283, 93, P_CHEWY, P_LEFT);
				set_person_pos(238, 99, P_HOWARD, P_RIGHT);
				start_aad_wait(315, -1);
				go_auto_xy(241, 74, P_CHEWY, ANI_WAIT);
				start_aad_wait(316, -1);

				spieler_mi[P_HOWARD].Mode = true;
				switch_room(55);
			}
		}
	} else {
		action_ret = true;
		auto_move(6, P_CHEWY);
		spieler.R54HowardVorne = true;
		start_aad_wait(319, -1);
	}
	show_cur();
	return action_ret;
}

int16 r54_cut_serv(int16 frame) {
	det->plot_static_details(176, 0, 9, 9);
	return 0;
}

int16 r54_use_azug() {
	int16 ch_y;
	int16 ay;
	int16 delay;
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		hide_cur();
		if (!spieler.R54LiftOk) {
			if (spieler.R54FputzerWeg) {
				auto_move(8, P_CHEWY);
				spieler.R54HowardVorne = false;
				start_aad_wait(298, -1);
				spieler.PersonHide[P_CHEWY] = true;
				det->show_static_spr(12);
				spieler.R55Location = true;
				SetUpScreenFunc = 0;
				go_auto_xy(91, 62, P_HOWARD, ANI_WAIT);
				ch_y = 68;
				ay = 0;
				delay = 0;
				while (ch_y > -48) {
					det->set_static_pos(12, 125, ch_y, false, false);
					det->set_static_pos(9, 122, ay, false, false);
					if (!delay) {
						ch_y -= 3;
						ay -= 3;
						delay = spieler.DelaySpeed / 2;
					} else
						--delay;
					set_up_screen(DO_SETUP);
				}
				switch_room(55);
			}
		} else
			start_aad_wait(314, -1);
		show_cur();
	}
	return action_ret;
}

void r54_aufzug_ab() {
	int16 ch_y;
	int16 ay;
	int16 delay;
	set_person_pos(91, 62, P_HOWARD, P_RIGHT);
	set_person_pos(99, 82, P_CHEWY, P_RIGHT);
	spieler.scrollx = 0;
	SetUpScreenFunc = r54_setup_func;
	ch_y = -40;
	ay = -108;
	delay = 0;
	det->show_static_spr(12);
	while (ch_y < 68 && !SHOULD_QUIT) {
		det->set_static_pos(12, 125, ch_y, false, false);
		det->set_static_pos(9, 122, ay, false, false);
		if (!delay) {
			ch_y += 3;
			ay += 3;
			delay = spieler.DelaySpeed / 2;
		} else
			--delay;
		set_up_screen(DO_SETUP);
	}
	det->hide_static_spr(12);
	set_person_pos(99, 82, P_CHEWY, P_RIGHT);
	spieler.PersonHide[P_CHEWY] = false;
	maus_links_click = false;
	stop_person(P_CHEWY);
	spieler.R55Location = false;
}

short r54_use_taxi() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		hide_cur();
		auto_move(7, P_CHEWY);
		spieler.R48TaxiPerson[P_CHEWY] = true;
		if (spieler.PersonRoomNr[P_HOWARD] == 54) {
			if (spieler.R54HowardVorne) {
				spieler.R54HowardVorne = false;
				go_auto_xy(290, 61, P_HOWARD, ANI_WAIT);
			}
			spieler.PersonHide[P_HOWARD] = true;
			spieler.R48TaxiPerson[P_HOWARD] = true;
			spieler.PersonRoomNr[P_HOWARD] = 48;
		}
		spieler.PersonHide[P_CHEWY] = true;
		switch_room(48);
	}
	return action_ret;
}

void r55_entry() {
	spieler.ScrollxStep = 2;
	SetUpScreenFunc = r55_setup_func;
	zoom_horizont = 140;
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	spieler.R55ExitDia = 0;
	spieler.ZoomXy[P_HOWARD][0] = 20;
	spieler.ZoomXy[P_HOWARD][1] = 22;
	if (!spieler.R54FputzerWeg)
		det->start_detail(6, 255, ANI_VOR);

	if ((spieler.R55RaumOk) || (!spieler.R55RaumOk && spieler.R55ScriptWeg))
		det->hide_static_spr(0);
	if (!spieler.R55SekWeg) {
		det->start_detail(8, 255, ANI_VOR);
		det->start_detail(9, 255, ANI_VOR);
		spieler.R55Entry ^= 1;
		timer_nr[0] = room->set_timer(19 + (int16)spieler.R55Entry, 10);
		det->set_static_ani(19 + (int16)spieler.R55Entry, -1);
	}
	spieler_mi[P_HOWARD].Mode = true;
	if (spieler.R55Job) {
		r55_rock2mans();
		spieler.PersonRoomNr[P_HOWARD] = 54;
	}
	if (spieler.ChewyAni == CHEWY_JMANS) {
		set_person_pos(51, 75, P_CHEWY, P_RIGHT);
		room->set_zoom(10);
	}
	if (spieler.R55EscScriptOk && !spieler.R55RaumOk)
		det->show_static_spr(0);
	if (!flags.LoadGame) {
		if (spieler.R55Location) {
			spieler.scrollx = 136;
			set_person_pos(400, 82, P_CHEWY, P_RIGHT);
			start_detail_wait(3, 1, ANI_VOR);
			timer_nr[0] = room->set_timer(4, 10);
			det->set_static_ani(4, -1);
			atds->del_steuer_bit(340, ATS_AKTIV_BIT, ATS_DATEI);
		} else {
			spieler.scrollx = 0;
			if (spieler.PersonRoomNr[P_HOWARD] == 55) {
				set_person_pos(21, 77, P_HOWARD, P_RIGHT);
			}
			det->show_static_spr(8);
			auto_move(1, P_CHEWY);
			hide_cur();
			det->hide_static_spr(8);
			if (spieler.R55SekWeg && !spieler.R55Job) {

				set_person_spr(P_LEFT, P_CHEWY);
				if (spieler.ChewyAni == CHEWY_ROCKER) {
					r55_verleger_mov(0);
					r55_talk_line();
					spieler.R54Schild = true;
				} else {
					r55_verleger_mov(1);
					r55_get_job();
				}
			}
			show_cur();
		}
	}
}

void r55_exit(int16 eib_nr) {
	if (eib_nr == 90) {
		r55_mans2rock();
		if (spieler.PersonRoomNr[P_HOWARD] == 55) {
			spieler.PersonRoomNr[P_HOWARD] = 54;
			spieler_mi[P_HOWARD].Mode = false;
		}
	}
	spieler.ScrollxStep = 1;
}

int16 r55_use_stapel1() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		if (!spieler.R55ScriptWeg) {
			action_ret = true;
			spieler.R55ScriptWeg = true;
			auto_move(4, P_CHEWY);
			det->hide_static_spr(0);
			auto_move(5, P_CHEWY);
			det->show_static_spr(1);
			atds->set_ats_str(354, 1, ATS_DATEI);
			atds->set_ats_str(355, 1, ATS_DATEI);
		} else if (spieler.R55EscScriptOk && !spieler.R55RaumOk) {
			action_ret = true;
			start_aad_wait(333, -1);
		}
	} else if (is_cur_inventar(MANUSKRIPT_INV)) {
		action_ret = true;
		if (spieler.R55ScriptWeg) {
			auto_move(4, P_CHEWY);
			spieler.R55EscScriptOk = true;
			det->show_static_spr(0);
			del_inventar(spieler.AkInvent);
			atds->set_ats_str(354, 2, ATS_DATEI);
		} else
			start_aad_wait(326, -1);
	}
	return action_ret;
}

int16 r55_use_stapel2() {
	int16 action_ret = false;
	if (is_cur_inventar(MANUSKRIPT_INV)) {
		action_ret = true;
		start_aad_wait(327, -1);
	}
	return action_ret;
}

int16 r55_use_telefon() {
	int16 i;
	int16 delay;
	int16 tmp_delay;
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		if (spieler.R55EscScriptOk) {
			if (!spieler.R55RaumOk) {
				hide_cur();
				spieler.R55RaumOk = true;
				auto_move(6, P_CHEWY);
				spieler.PersonHide[P_CHEWY] = true;
				start_detail_wait(10, 1, ANI_VOR);
				det->start_detail(11, 255, ANI_VOR);
				start_aad_wait(329, -1);
				det->stop_detail(11);
				start_detail_wait(10, 1, ANI_RUECK);
				spieler.PersonHide[P_CHEWY] = false;
				auto_move(7, P_CHEWY);
				flags.NoScroll = true;
				auto_scroll(0, 0);
				start_aad_wait(330, -1);
				det->show_static_spr(8);
				start_detail_wait(0, 1, ANI_VOR);
				det->start_detail(1, 255, ANI_VOR);
				start_aad_wait(331, -1);
				det->stop_detail(1);
				start_detail_wait(2, 1, ANI_VOR);
				det->hide_static_spr(0);
				det->hide_static_spr(8);
				atds->set_ats_str(354, 1, ATS_DATEI);
				flic_cut(FCUT_071, FLC_MODE);
				spieler.PersonRoomNr[P_HOWARD] = 55;
				set_person_pos(178, 75, P_CHEWY, P_LEFT);
				set_person_pos(66, 73, P_HOWARD, P_RIGHT);
				SetUpScreenFunc = 0;
				det->set_static_ani(18, -1);
				start_aad_wait(334, -1);
				start_aad_wait(335, -1);
				start_aad_wait(336, -1);
				tmp_delay = spieler.DelaySpeed;
				for (i = 0; i < 7; i++) {
					delay = tmp_delay;
					if (!spieler.scrollx)
						spieler.scrollx = 8;
					else
						spieler.scrollx = 0;
					while (delay) {
						out->skip_frame(1);
						--delay;
					}
					set_up_screen(DO_SETUP);
				}
				flic_cut(FCUT_072, FLC_MODE);
				flags.NoScroll = false;
				invent_2_slot(SACKGELD_INV);
				invent_2_slot(EINLAD_INV);
				del_invent_slot(LEDER_INV);
				load_chewy_taf(CHEWY_ROCKER);
				spieler.PersonRoomNr[P_HOWARD] = 54;
				spieler_mi[P_HOWARD].Mode = false;
				show_cur();
				spieler.R55R54First = true;
				spieler.R55ExitDia = 337;
				spieler.room_e_obj[89].Attribut = 255;
				switch_room(54);
			} else {
				start_aad_wait(332, -1);
			}
		} else {
			start_aad_wait(328, -1);
		}
	}
	return action_ret;
}

void r55_get_job() {
	int16 tmp_scrollx;
	int16 tmp_scrolly;
	int16 r_nr;
	spieler.PersonHide[P_CHEWY] = true;
	tmp_scrollx = spieler.scrollx;
	tmp_scrolly = spieler.scrolly;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	switch_room(61);
	show_cur();
	start_ads_wait(15);
	spieler.PersonHide[P_CHEWY] = false;
	flags.LoadGame = true;
	spieler.scrollx = tmp_scrollx;
	spieler.scrolly = tmp_scrolly;
	if (spieler.R55Job) {
		r_nr = 55;
		atds->del_steuer_bit(357, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(354, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(355, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		r_nr = 54;
		r55_mans2rock();
	}
	spieler.PersonRoomNr[P_HOWARD] = r_nr;
	spieler_mi[P_HOWARD].Mode = true;
	if (r_nr == 54)
		flags.LoadGame = false;
	set_person_pos(118, 96, P_CHEWY, P_LEFT);
	switch_room(r_nr);
	flags.LoadGame = false;

}

void r55_mans2rock() {
	if (spieler.ChewyAni == CHEWY_JMANS) {
		spieler.R55ExitDia = 317;
		load_chewy_taf(CHEWY_ROCKER);
		del_invent_slot(LEDER_INV);
		invent_2_slot(JMKOST_INV);
	}
}

void r55_rock2mans() {
	if (spieler.ChewyAni == CHEWY_ROCKER) {
		load_chewy_taf(CHEWY_JMANS);
		del_invent_slot(JMKOST_INV);
		invent_2_slot(LEDER_INV);
	}
}

void r55_verleger_mov(int16 mode) {
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(11);
	start_detail_wait(13, 1, ANI_VOR);
	start_detail_wait(14, 1, ANI_VOR);
	if (spieler.ChewyAni == CHEWY_JMANS)
		start_spz(CH_JM_TITS, 1, ANI_VOR, P_CHEWY);
	if (mode)
		start_detail_wait(15, 1, ANI_VOR);
	det->set_static_ani(16, -1);
}

void r55_strasse(int16 mode) {
	if (!spieler.inv_cur || mode) {
		room->set_timer_status(4, TIMER_STOP);
		det->del_static_ani(4);
		det->stop_detail(4);
		atds->set_steuer_bit(340, ATS_AKTIV_BIT, ATS_DATEI);
		start_detail_wait(3, 1, ANI_RUECK);

		spieler.scrollx = 0;
		switch_room(54);
	}
}

int16 r55_use_kammeraus() {
	int16 action_ret = false;
	if (spieler.R55Location) {
		if (is_cur_inventar(KILLER_INV)) {
			action_ret = true;
			if (spieler.R52KakerWeg) {
				if (!spieler.R55SekWeg) {
					hide_cur();
					room->set_timer_status(4, TIMER_STOP);
					det->del_static_ani(4);
					det->stop_detail(4);
					start_detail_wait(5, 1, ANI_VOR);
					det->show_static_spr(10);
					flc->set_flic_user_function(r55_cut_serv);
					flic_cut(FCUT_070, FLC_MODE);
					flc->remove_flic_user_function();
					room->set_timer_status(19 + (int16)spieler.R55Entry, TIMER_STOP);
					det->stop_detail(19 + (int16)spieler.R55Entry);
					det->del_static_ani(19 + (int16)spieler.R55Entry);
					det->hide_static_spr(10);
					spieler.R55ExitDia = 322;
					spieler.R55SekWeg = true;
					atds->set_ats_str(KILLER_INV, 1, INV_ATS_DATEI);
					atds->set_steuer_bit(352, ATS_AKTIV_BIT, ATS_DATEI);
					r55_strasse(1);
					show_cur();
				}
			} else {
				start_aad_wait(325, -1);
			}
		}
	}
	return action_ret;
}

int16 r55_cut_serv(int16 frame) {
	if (frame < 10)
		det->plot_static_details(136, 0, 10, 10);
	return 0;
}

void r55_setup_func() {
	int16 x, y;
	int16 ch_x;
	if (spieler.PersonRoomNr[P_HOWARD] == 55) {
		calc_person_look();
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		if (ch_x < 100) {
			x = 62;
			y = 101;
		} else if (ch_x < 187) {
			x = 139;
			y = 119;
		} else if (ch_x < 276) {
			x = 235;
			y = 119;
		} else {
			x = 318;
			y = 110;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

void r55_talk_line() {
	int16 aad_nr = 0;
	if (spieler.R55Location == 0) {
		if (spieler.ChewyAni == CHEWY_ROCKER) {
			hide_cur();
			if (!spieler.R55SekWeg) {
				auto_move(2, P_CHEWY);
				flags.NoScroll = true;
				auto_scroll(136, 0);
				aad_nr = 320;
				spieler.R55ExitDia = 321;
				room->set_timer_status(19 + (int16)spieler.R55Entry, TIMER_STOP);
				det->stop_detail(19 + (int16)spieler.R55Entry);
				det->del_static_ani(19 + (int16)spieler.R55Entry);
				start_detail_wait(22, 1, ANI_VOR);
				det->set_static_ani(21, -1);
			} else if (!spieler.R55RaumOk) {
				auto_move(3, P_CHEWY);
				aad_nr = 323;
				spieler.R55ExitDia = 324;
			}
			start_aad_wait(aad_nr, -1);
			if (spieler.R55SekWeg) {
				det->del_static_ani(16);
				start_detail_wait(14, 1, ANI_VOR);
			} else {
				det->del_static_ani(21);
				start_detail_wait(22, 1, ANI_VOR);
			}
			flags.NoScroll = false;
			spieler.PersonRoomNr[P_HOWARD] = 54;
			spieler_mi[P_HOWARD].Mode = false;
			switch_room(54);
			show_cur();
		}
	}
}

void r56_entry() {
	spieler.ScrollxStep = 2;
	flags.ZoomMov = true;
	zoom_mov_fak = 4;
	spieler_mi[P_HOWARD].Mode = true;
	timer_nr[0] = room->set_timer(255, 25);
	if (!flags.LoadGame) {
		if (spieler.R48TaxiEntry) {
			hide_cur();
			spieler.R48TaxiEntry = false;
			spieler.scrollx = 0;
			spieler.scrolly = 0;
			spieler.PersonHide[P_CHEWY] = true;
			spieler.PersonHide[P_HOWARD] = true;
			det->hide_static_spr(2);
			zoom_horizont = 0;
			set_person_pos(-6, 16, P_HOWARD, P_RIGHT);
			set_person_pos(3, 42, P_CHEWY, P_RIGHT);
			start_detail_wait(7, 1, ANI_RUECK);
			start_detail_wait(8, 1, ANI_VOR);
			start_detail_wait(7, 1, ANI_VOR);
			set_up_screen(DO_SETUP);
			if (!spieler.R56GetTabak) {
				flic_cut(FCUT_074, FLC_MODE);
				det->show_static_spr(2);
			}
			room->set_zoom(23);
			spieler.ZoomXy[P_HOWARD][0] = 17;
			spieler.ZoomXy[P_HOWARD][1] = 37;
			spieler.PersonHide[P_CHEWY] = false;
			spieler.PersonHide[P_HOWARD] = false;
			SetUpScreenFunc = r56_setup_func;
			spieler_mi[P_CHEWY].Mode = true;
			auto_move(1, P_CHEWY);
			spieler_mi[P_CHEWY].Mode = false;
			maus_links_click = false;
			show_cur();
		} else if (spieler.R62Flucht) {
			maus_links_click = false;
			spieler.ZoomXy[P_HOWARD][0] = 40;
			spieler.ZoomXy[P_HOWARD][1] = 86;
			zoom_horizont = 114;
			room->set_zoom(70);
			spieler.R62Flucht = false;
			set_person_pos(308, 97, P_HOWARD, P_RIGHT);
			set_person_pos(429, 146, P_CHEWY, P_LEFT);
			spieler.scrollx = 262;
			spieler.PersonHide[P_HOWARD] = false;
			det->show_static_spr(9);
			det->show_static_spr(8);
			room->set_timer_status(0, TIMER_STOP);
			det->del_static_ani(0);
			det->set_static_ani(3, -1);
			maus_links_click = false;
			atds->stop_aad();
			start_aad_wait(306, -1);
			flic_cut(FCUT_076, FLC_MODE);
		}
	}
	SetUpScreenFunc = r56_setup_func;
	spieler.ZoomXy[P_HOWARD][0] = 40;
	spieler.ZoomXy[P_HOWARD][1] = 86;
	zoom_horizont = 114;
	room->set_zoom(70);
}

void r56_exit() {
	spieler_mi[P_HOWARD].Mode = false;
	spieler.ScrollxStep = 1;
}

int16 r56_use_taxi() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		hide_cur();
		auto_move(1, P_CHEWY);
		start_detail_wait(7, 1, ANI_RUECK);
		det->start_detail(8, 1, ANI_VOR);
		zoom_horizont = 0;
		room->set_zoom(23);
		spieler.ZoomXy[P_HOWARD][0] = 17;
		spieler.ZoomXy[P_HOWARD][1] = 37;
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(3, 42, P_CHEWY, ANI_WAIT);
		spieler_mi[P_CHEWY].Mode = false;
		spieler.PersonHide[P_CHEWY] = true;
		spieler.R48TaxiPerson[P_CHEWY] = true;
		if (spieler.PersonRoomNr[P_HOWARD] == 56) {
			spieler.PersonHide[P_HOWARD] = true;
			spieler.R48TaxiPerson[P_HOWARD] = true;
			spieler.PersonRoomNr[P_HOWARD] = 48;
		}
		show_cur();
		switch_room(48);
	}
	return action_ret;
}

void r56_talk_man() {
	auto_move(3, P_CHEWY);
	if (!spieler.R56AbfahrtOk) {
		start_ads_wait(16);
	} else if (!spieler.R62Flucht) {
		start_aad_wait(343, -1);
	}
}

int16 r56_use_man() {
	int16 action_ret = false;
	if (is_cur_inventar(FLASCHE_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		start_spz_wait(CH_ROCK_GET2, 1, ANI_VOR, P_CHEWY);
		room->set_timer_status(0, TIMER_STOP);
		det->del_static_ani(0);

		if (!spieler.R56WhiskyMix) {
			start_detail_wait(4, 1, ANI_VOR);

			det->set_static_ani(5, -1);
			start_aad_wait(304, -1);
			det->del_static_ani(5);
			room->set_timer_status(0, TIMER_START);
			det->set_static_ani(0, -1);
		} else {
			del_inventar(spieler.AkInvent);
			spieler.R56AbfahrtOk = true;
			start_detail_wait(6, 1, ANI_VOR);

			det->set_static_ani(1, -1);
			start_aad_wait(305, -1);
			det->del_static_ani(1);
		}
		room->set_timer_status(0, TIMER_START);
		det->set_static_ani(0, -1);
		show_cur();
	}
	return action_ret;
}

int16 r56_use_kneipe() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		if (!atds->get_steuer_bit(362, ATS_AKTIV_BIT, ATS_DATEI)) {
			hide_cur();
			action_ret = true;
			if (!spieler.R56Kneipe) {
				SetUpScreenFunc = 0;
				auto_move(4, P_CHEWY);
				spieler.PersonHide[P_CHEWY] = true;
				go_auto_xy(160, 58, P_HOWARD, ANI_WAIT);
				spieler.PersonHide[P_HOWARD] = true;
				spieler.R56Kneipe = true;
				flags.NoScroll = true;
				auto_scroll(0, 0);
				start_detail_wait(12, 3, ANI_VOR);
				flic_cut(FCUT_075, FLC_MODE);
				det->start_detail(10, 6, ANI_VOR);
				start_aad_wait(307, -1);
				det->stop_detail(10);
				out->ausblenden(0);
				set_up_screen(DO_SETUP);
				spieler.PersonHide[P_CHEWY] = false;
				spieler.PersonHide[P_HOWARD] = false;
				spieler.scrollx = 0;
				set_person_pos(23, 70, P_HOWARD, P_RIGHT);
				set_person_pos(50, 81, P_CHEWY, P_LEFT);

				fx_blend = BLEND3;
				start_aad_wait(308, -1);
				SetUpScreenFunc = r56_setup_func;
				flags.NoScroll = false;

				if (obj->check_inventar(SACKGELD_INV)) {
					del_invent_slot(SACKGELD_INV);
					start_aad_wait(309, -1);
				}
			} else {
				start_aad_wait(344, -1);
			}
			show_cur();
		}
	}
	return action_ret;
}

void r56_start_flug() {
	if (!r56koch_flug) {
		r56koch_flug = 12;
		det->start_detail(r56koch_flug, 1, ANI_VOR);
	}
}

void r56_setup_func() {
	int16 x, y;
	int16 ch_x;
	int16 ch_y;
	switch (r56koch_flug) {
	case 12:
		if (det->get_ani_status(12) == false) {
			det->start_detail(11, 1, ANI_VOR);
			r56koch_flug = 11;
		}
		break;

	case 11:
		if (det->get_ani_status(11) == false) {
			det->start_detail(10, 1, ANI_VOR);
			r56koch_flug = 10;
		}
		break;

	case 10:
		if (det->get_ani_status(10) == false) {
			r56koch_flug = 0;

			uhr->reset_timer(timer_nr[0], 0);
		}
		break;

	}
	if (spieler.PersonRoomNr[P_HOWARD] == 56) {
		calc_person_look();
		ch_x = spieler_vector[P_CHEWY].Xypos[0];
		ch_y = spieler_vector[P_CHEWY].Xypos[1];
		if (ch_x < 196) {
			x = 23;
			y = 70;
		} else if (ch_x < 283) {
			x = 115;
			y = 61;
		} else if (ch_y < 115) {
			x = 254;
			y = 66;
		} else {
			x = 308;
			y = 97;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
	if (!atds->get_steuer_bit(362, ATS_AKTIV_BIT, ATS_DATEI)) {
		if (menu_item == CUR_WALK) {
			if (minfo.x + spieler.scrollx >= 157 &&
			        minfo.x + spieler.scrollx <= 204 &&
			        minfo.y >= 28 && minfo.y <= 89) {
				cursor_wahl(CUR_AUSGANG_OBEN);
			} else
				cursor_wahl(CUR_WALK);
		}
	}
}

void r57_entry() {
	zoom_horizont = 180;
	flags.ZoomMov = true;
	zoom_mov_fak = 4;
	SetUpScreenFunc = r57_setup_func;
	spieler.ZoomXy[P_HOWARD][0] = 46;
	spieler.ZoomXy[P_HOWARD][1] = 86;
	spieler_mi[P_HOWARD].Mode = true;
	if (spieler.R57StudioAuf)
		det->hide_static_spr(4);
	if (!flags.LoadGame) {
		if (spieler.R48TaxiEntry) {
			hide_cur();
			spieler.R48TaxiEntry = false;
			spieler.scrollx = 0;
			spieler.scrolly = 0;
			set_person_pos(4, 144, P_HOWARD, P_LEFT);
			set_person_pos(40, 160, P_CHEWY, P_RIGHT);
			auto_move(2, P_CHEWY);
			maus_links_click = false;
			show_cur();
		}
	}
}

void r57_exit(int16 eib_nr) {
	if (spieler.PersonRoomNr[P_HOWARD] == 57) {
		spieler_mi[P_HOWARD].Mode = false;
		if (eib_nr == 91)
			spieler.PersonRoomNr[P_HOWARD] = 62;
	}
}

void r57_setup_func() {
	int16 x, y;
	int16 ch_y;
	if (spieler.PersonRoomNr[P_HOWARD] == 57) {
		calc_person_look();
		ch_y = spieler_vector[P_CHEWY].Xypos[1];
		if (ch_y < 145) {
			x = 176;
			y = 126;
		} else {
			x = 176;
			y = 142;
		}
		go_auto_xy(x, y, P_HOWARD, ANI_GO);
	}
}

int16 r57_use_taxi() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		hide_cur();
		auto_move(3, P_CHEWY);
		det->show_static_spr(7);
		go_auto_xy(16, 160, P_CHEWY, ANI_WAIT);
		spieler.PersonHide[P_CHEWY] = true;
		spieler.R48TaxiPerson[P_CHEWY] = true;
		if (spieler.PersonRoomNr[P_HOWARD] == 57) {
			go_auto_xy(11, 144, P_HOWARD, ANI_WAIT);
			spieler.PersonHide[P_HOWARD] = true;
			spieler.R48TaxiPerson[P_HOWARD] = true;
			spieler.PersonRoomNr[P_HOWARD] = 48;
		}
		det->hide_static_spr(7);
		room->set_timer_status(3, TIMER_STOP);
		det->del_static_ani(3);
		start_detail_wait(5, 1, ANI_VOR);
		switch_room(48);
	}
	return action_ret;
}

int16 r57_use_pfoertner() {
	int16 action_ret = false;
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	det->set_static_ani(3, -1);
	hide_cur();
	auto_move(1, P_CHEWY);
	if (is_cur_inventar(CUTMAG_INV)) {
		action_ret = true;
		start_aad_wait(339, -1);
		new_invent_2_cur(BESTELL_INV);
	} else if (is_cur_inventar(JMKOST_INV)) {
		action_ret = true;
		start_aad_wait(340, -1);
	} else if (is_cur_inventar(EINLAD_INV)) {
		action_ret = true;
		SetUpScreenFunc = 0;
		go_auto_xy(132, 130, P_HOWARD, ANI_WAIT);
		if (spieler.R56AbfahrtOk) {
			start_aad_wait(341, -1);
			go_auto_xy(176, 130, P_HOWARD, ANI_WAIT);
			del_inventar(spieler.AkInvent);
			spieler.R57StudioAuf = true;
			spieler.room_e_obj[91].Attribut = AUSGANG_OBEN;
			det->hide_static_spr(4);
			start_detail_wait(6, 1, ANI_WAIT);
			atds->set_steuer_bit(358, ATS_AKTIV_BIT, ATS_DATEI);
		} else {
			start_aad_wait(349, -1);
			go_auto_xy(176, 130, P_HOWARD, ANI_WAIT);
		}
		SetUpScreenFunc = r57_setup_func;
	}
	show_cur();
	room->set_timer_status(1, TIMER_START);
	det->set_static_ani(1, -1);
	return action_ret;
}

void r57_talk_pfoertner() {
	int16 aad_nr;
	hide_cur();
	auto_move(1, P_CHEWY);
	room->set_timer_status(1, TIMER_STOP);
	det->del_static_ani(1);
	det->set_static_ani(3, -1);
	if (!spieler.R57StudioAuf) {
		aad_nr = 338;
	} else
		aad_nr = 342;
	start_aad_wait(aad_nr, -1);
	room->set_timer_status(1, TIMER_START);
	det->set_static_ani(1, -1);
	show_cur();
}

void r58_entry() {
	SetUpScreenFunc = r58setup_func;
}

void r58_exit() {
	int16 tmp;
	spieler.scrollx = spieler.R58TmpSx;
	spieler.scrolly = spieler.R58TmpSy;
	spieler_vector[P_CHEWY].Xypos[0] = spieler.R58TmpX;
	spieler_vector[P_CHEWY].Xypos[1] = spieler.R58TmpY;
	spieler_mi[P_CHEWY].XyzStart[0] = spieler.R58TmpX;
	spieler_mi[P_CHEWY].XyzStart[1] = spieler.R58TmpY;
	flags.MainInput = true;
	flags.LoadGame = true;
	show_person();
	set_person_rnr();
	tmp = spieler.R58TmpRoom1;
	spieler.PersonRoomNr[P_CHEWY] = spieler.R58TmpRoom;

	switch_room(tmp);

}

void r58_look_cut_mag(int16 r_nr) {
	spieler.R58TmpRoom = r_nr;
	spieler.R58TmpRoom1 = spieler.PersonRoomNr[P_CHEWY];;

	spieler.R58TmpSx = spieler.scrollx;
	spieler.R58TmpSy = spieler.scrolly;
	spieler.R58TmpX = spieler_vector[P_CHEWY].Xypos[0];
	spieler.R58TmpY = spieler_vector[P_CHEWY].Xypos[1];
	spieler_vector[P_CHEWY].Xypos[0] = 160;
	spieler_vector[P_CHEWY].Xypos[1] = 100;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	flags.MainInput = false;
	hide_person();
	switch_room(r_nr);
}

void r58setup_func() {
	if (menu_item != CUR_LOOK) {
		menu_item = CUR_LOOK;
		cursor_wahl(menu_item);
	}
}

void r59_look_poster() {
	if (!spieler.R59PosterWeg) {

		atds->set_ats_str(302, 1, ATS_DATEI);
		atds->set_ats_str(301, 1, ATS_DATEI);
		spieler.R59PosterWeg = true;
		invent_2_slot(SPARK_INV);
		switch_room(60);
	}
}

void r62_entry() {
	SetUpScreenFunc = r62_setup_func;
	spieler_mi[P_HOWARD].Mode = true;
	spieler.R62LauraVerwandlung = 0;
	spieler.room_e_obj[91].Attribut = 255;
	if (!spieler.R62FirstEntry) {
		spieler.R62TShow = false;
		cur_hide_flag = false;
		hide_cur();
		spieler.R62FirstEntry = true;
		set_person_pos(49, 113, P_HOWARD, P_RIGHT);
		det->start_detail(6, 255, ANI_VOR);
		start_aad_wait(345, -1);
		det->stop_detail(6);
		det->set_static_ani(5, -1);
		go_auto_xy(199, 59, P_HOWARD, ANI_WAIT);
		set_person_spr(P_LEFT, P_HOWARD);
		det->del_static_ani(5);
		det->start_detail(5, 255, ANI_VOR);
		start_aad_wait(346, -1);
		det->stop_detail(5);
		det->start_detail(6, 255, ANI_VOR);
		start_aad_wait(347, -1);
		det->stop_detail(6);
		start_detail_wait(7, 1, ANI_VOR);
		det->set_static_ani(5, -1);
		det->start_detail(0, 255, ANI_VOR);
		start_aad_wait(348, -1);
		det->stop_detail(0);
		det->del_static_ani(0);
		det->start_detail(1, 1, ANI_VOR);
		auto_move(0, P_CHEWY);
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(160, 240, P_CHEWY, ANI_WAIT);
		spieler_mi[P_CHEWY].Mode = false;
		det->del_static_ani(5);
		start_detail_wait(7, 1, ANI_VOR);
		show_cur();
		spieler.R64Moni1Ani = 3;
		spieler.R64Moni2Ani = 4;
		set_person_pos(187, 43, P_CHEWY, P_RIGHT);
		spieler.R62TShow = true;
		switch_room(64);
	} else {
		det->del_static_ani(0);
		spieler.PersonHide[P_HOWARD] = true;
		det->set_static_ani(4, -1);
		det->set_static_ani(8, -1);
		r62Delay = 0;
		r62TalkAni = 8;

	}
}

void r62_setup_func() {
	if (r62Delay <= 0 && spieler.R62TShow) {
		r62Delay = (spieler.DelaySpeed + 1) * 60;
		det->stop_detail(r62TalkAni);
		if (r62TalkAni == 4)
			r62TalkAni = 8;
		else
			r62TalkAni = 4;
		det->start_detail(r62TalkAni, 255, ANI_VOR);
	} else
		--r62Delay;
}

int16 r62_use_laura() {
	int16 action_ret = false;
	if (is_cur_inventar(GERAET_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(2, P_CHEWY);
		auto_scroll(0, 0);
		SetUpScreenFunc = 0;
		det->del_static_ani(8);
		det->stop_detail(8);
		spieler.PersonHide[P_CHEWY] = true;
		det->start_detail(2, 255, ANI_VOR);
		det->start_detail(6, 255, ANI_VOR);
		start_aad_wait(399, -1);
		spieler.PersonHide[P_CHEWY] = false;
		flic_cut(FCUT_077, FLC_MODE);
		show_cur();
		spieler.R64Moni1Ani = 0;
		spieler.R64Moni2Ani = 0;
		spieler.R62TShow = false;
		spieler.R62LauraVerwandlung = true;
		switch_room(63);
	}
	return action_ret;
}

void r63_entry() {
	zoom_horizont = 76;
	spieler.ScrollxStep = 2;
	r63Schalter = false;
	r63RunDia = 0;
	if (spieler.R63FxMannWeg)
		det->del_static_ani(5);
	if (!spieler.R63Uhr) {
		cur_hide_flag = false;
		hide_cur();
		det->start_detail(12, 255, ANI_VOR);
		det->start_detail(10, 255, ANI_VOR);
		det->start_detail(18, 255, ANI_VOR);
		auto_move(4, P_CHEWY);
		det->stop_detail(10);
		det->start_detail(11, 255, ANI_VOR);
		auto_scroll(58, 0);
		start_aad_wait(355, -1);
		det->stop_detail(11);
		det->start_detail(10, 255, ANI_VOR);
		show_cur();
	} else if (!spieler.R63Feuer) {
		det->show_static_spr(10);
		det->show_static_spr(12);
	} else if (spieler.R62LauraVerwandlung) {
		SetUpScreenFunc = r63_setup_func;
		cur_2_inventory();
		spieler.scrollx = 176;
		set_person_pos(424, 78, P_CHEWY, P_LEFT);
		spieler.PersonHide[P_CHEWY] = true;
		spieler.room_e_obj[95].Attribut = 255;
		r63ChewyAni = 0;
		det->start_detail(0, 1, ANI_VOR);
	}
}

void r63_setup_func() {
	if (spieler.R62LauraVerwandlung) {
		switch (r63ChewyAni) {
		case 0:
			if (det->get_ani_status(0) == false) {
				if (r63RunDia < 4)
					++r63RunDia;
				start_aad(370 + r63RunDia);
				det->start_detail(1, 1, ANI_VOR);
				r63ChewyAni = 1;
			}
			break;

		case 1:
			if (det->get_ani_status(1) == false) {
				spieler.ScrollxStep = 4;
				set_person_pos(0, 0, P_CHEWY, P_RIGHT);
				det->start_detail(22, 1, ANI_VOR);
				det->start_detail(2, 1, ANI_VOR);
				atds->stop_aad();
				r63ChewyAni = 2;
			}
			break;

		case 2:
			if (det->get_ani_status(2) == false) {
				det->start_detail(3, 1, ANI_VOR);
				r63ChewyAni = 3;
			}
			break;

		case 3:
			if (det->get_ani_status(3) == false) {
				if (!flags.AutoAniPlay) {
					flags.AutoAniPlay = true;

					spieler.ScrollxStep = 16;
					spieler.scrollx -= spieler.scrollx % 16;
					flags.NoScroll = true;
					auto_scroll(176, 0);
					set_person_pos(424, 78, P_CHEWY, P_LEFT);
					flags.NoScroll = false;
					spieler.ScrollxStep = 4;
					if (!r63Schalter) {
						det->start_detail(0, 1, ANI_VOR);
						r63ChewyAni = 0;
					} else
						r63_bork_platt();
					flags.AutoAniPlay = false;
				}
			}
			break;

		}
	}
}

void r63_bork_platt() {
	spieler.R62LauraVerwandlung = false;
	r63Schalter = false;
	spieler.room_e_obj[95].Attribut = AUSGANG_OBEN;
	flic_cut(FCUT_081, FLC_MODE);
	flic_cut(FCUT_082, FLC_MODE);
	spieler.PersonHide[P_CHEWY] = false;
	check_shad(4, 1);
	spieler_mi[P_CHEWY].Mode = true;
	auto_move(6, P_CHEWY);
	spieler_mi[P_CHEWY].Mode = false;
	start_aad_wait(370, -1);
	start_detail_wait(4, 1, ANI_VOR);
	det->show_static_spr(13);
	start_aad_wait(361, -1);
	out->cls();
	flags.NoPalAfterFlc = true;
	flic_cut(FCUT_079, FLC_MODE);
	fx_blend = BLEND3;
	show_cur();
	flags.MainInput = true;
	spieler.R62Flucht = true;
	spieler.PersonRoomNr[P_HOWARD] = 56;
	switch_room(56);
}

void r63_talk_hunter() {
	hide_cur();
	auto_move(3, P_CHEWY);
	if (spieler.R63Uhr)
		det->hide_static_spr(10);
	det->start_detail(10, 255, ANI_VOR);
	start_aad_wait(356, -1);
	if (spieler.R63Uhr) {
		det->show_static_spr(10);
		det->stop_detail(10);
	}
	show_cur();
}

void r63_talk_regie() {
	hide_cur();
	auto_move(3, P_CHEWY);
	if (spieler.R63Uhr)
		det->hide_static_spr(12);
	det->stop_detail(18);
	det->start_detail(19, 255, ANI_VOR);
	start_aad_wait(357, -1);
	det->stop_detail(19);
	if (spieler.R63Uhr) {
		det->show_static_spr(12);
	} else {
		det->start_detail(18, 255, ANI_VOR);
	}
	show_cur();
}

void r63_talk_fx_man() {
	hide_cur();
	auto_move(1, P_CHEWY);
	start_aad_wait(358, -1);
	show_cur();
}

int16 r63_use_fx_man() {
	int16 action_ret = false;
	if (is_cur_inventar(MASKE_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(1, P_CHEWY);
		del_inventar(spieler.AkInvent);
		start_aad_wait(359, -1);
		det->del_static_ani(5);
		start_detail_wait(6, 1, ANI_VOR);
		det->start_detail(7, 255, ANI_VOR);
		start_aad_wait(362, -1);
		det->stop_detail(7);
		start_detail_wait(8, 1, ANI_VOR);
		spieler.R63FxMannWeg = true;
		atds->set_steuer_bit(384, ATS_AKTIV_BIT, ATS_DATEI);
		show_cur();
	}
	return action_ret;
}

int16 r63_use_schalter() {
	int16 action_ret = false;
	if (!spieler.inv_cur) {
		action_ret = true;
		if (spieler.R63FxMannWeg) {
			if (spieler.R62LauraVerwandlung) {
				r63Schalter = true;
				hide_cur();
				flags.MainInput = false;
			} else {
				hide_cur();
				auto_move(1, P_CHEWY);
				start_spz_wait(CH_ROCK_GET2, 1, ANI_VOR, P_CHEWY);
				det->show_static_spr(2);
				set_person_spr(P_LEFT, P_CHEWY);
				start_detail_wait(21, 1, ANI_VOR);
				det->show_static_spr(14);
				wait_show_screen(18);
				det->hide_static_spr(14);
				start_detail_wait(24, 1, ANI_VOR);
				det->hide_static_spr(2);
				start_aad_wait(364, -1);
				atds->set_ats_str(385, 1, ATS_DATEI);
				show_cur();
			}
		} else {
			hide_cur();
			start_aad_wait(363, -1);
			show_cur();
		}
	}
	return action_ret;
}

void r63_talk_girl() {
	auto_move(2, P_CHEWY);
	det->stop_detail(12);
	start_detail_wait(13, 1, ANI_VOR);
	det->set_static_ani(14, -1);
	start_ads_wait(17);
	det->del_static_ani(14);
	det->start_detail(12, 255, ANI_VOR);
}

int16 r63_use_girl() {
	int16 action_ret = false;
	if (is_cur_inventar(UHR_INV)) {
		action_ret = true;
		hide_cur();
		auto_move(2, P_CHEWY);
		del_inventar(spieler.AkInvent);
		det->stop_detail(12);
		start_detail_wait(13, 1, ANI_VOR);
		det->set_static_ani(14, -1);
		start_aad_wait(365, -1);
		det->del_static_ani(14);
		det->start_detail(15, 255, ANI_VOR);
		start_aad_wait(360, -1);
		det->stop_detail(15);
		start_detail_wait(16, 1, ANI_VOR);
		spieler.R63Uhr = true;
		det->stop_detail(10);
		det->stop_detail(18);
		det->show_static_spr(10);
		det->show_static_spr(12);
		atds->set_ats_str(381, 1, ATS_DATEI);
		atds->set_ats_str(382, 1, ATS_DATEI);
		atds->set_steuer_bit(380, ATS_AKTIV_BIT, ATS_DATEI);
		start_aad_wait(367, -1);
		show_cur();
	}
	return action_ret;
}

int16 r63_use_aschenbecher() {
	int16 action_ret = false;
	if (is_cur_inventar(ASCHE_INV)) {
		action_ret = true;
		cur_hide_flag = false;
		hide_cur();
		if (spieler.R63Uhr) {
			if (spieler.R63FxMannWeg) {
				auto_move(5, P_CHEWY);
				del_inventar(spieler.AkInvent);
				flags.NoScroll = true;
				auto_scroll(70, 0);
				auto_move(1, P_CHEWY);
				spieler.PersonHide[P_CHEWY] = true;
				det->start_detail(20, 255, ANI_VOR);
				det->start_detail(10, 255, ANI_VOR);
				det->start_detail(18, 255, ANI_VOR);
				det->hide_static_spr(10);
				det->hide_static_spr(12);
				start_aad_wait(368, -1);
				flic_cut(FCUT_080, FLC_MODE);
				atds->set_steuer_bit(381, ATS_AKTIV_BIT, ATS_DATEI);
				atds->set_steuer_bit(382, ATS_AKTIV_BIT, ATS_DATEI);
				atds->set_ats_str(383, 1, ATS_DATEI);
				spieler.R63Feuer = true;
				spieler.PersonHide[P_CHEWY] = false;
				spieler.scrollx = 0;
				set_person_pos(187, 42, P_CHEWY, P_RIGHT);
				switch_room(64);
				flags.NoScroll = false;
			} else
				start_aad_wait(369, -1);
		} else
			start_aad_wait(366, -1);
		show_cur();
	}
	return action_ret;
}

void r64_entry() {
	SetUpScreenFunc = r64_setup_func;
	r62Delay = 0;
	r64TalkAni = spieler.R64Moni1Ani;
	if (!spieler.R64ManWeg) {
		timer_nr[0] = room->set_timer(1, 10);
		det->set_static_ani(1, -1);
		r64_chewy_entry();
		if (spieler.R63Feuer) {
			flc->set_flic_user_function(r64_cut_sev);
			flic_cut(FCUT_078, FLC_MODE);
			flc->remove_flic_user_function();
			spieler.R64ManWeg = true;
			det->del_static_ani(1);
			room->set_timer_status(1, TIMER_STOP);
			atds->set_steuer_bit(376, ATS_AKTIV_BIT, ATS_DATEI);
			spieler.R64Moni1Ani = 5;
			r64_calc_monitor();
			start_aad_wait(354, -1);
		}
	} else
		r64_chewy_entry();
}

int16 r64_cut_sev(int16 frame) {
	int16 spr_nr;
	int16 x, y;
	spr_nr = chewy_ph[spieler_vector[P_CHEWY].Phase * 8 + spieler_vector[P_CHEWY].PhNr];
	x = spieler_mi[P_CHEWY].XyzStart[0] + chewy_kor[spr_nr * 2] - spieler.scrollx;
	y = spieler_mi[P_CHEWY].XyzStart[1] + chewy_kor[spr_nr * 2 + 1] - spieler.scrolly;
	calc_zoom(spieler_mi[P_CHEWY].XyzStart[1], (int16)room->room_info->ZoomFak,
	          (int16)room->room_info->ZoomFak, &spieler_vector[P_CHEWY]);
	out->scale_set(chewy->image[spr_nr], x, y,
	                spieler_vector[P_CHEWY].Xzoom,
	                spieler_vector[P_CHEWY].Yzoom,
	                scr_width);
	return 0;
}

void r64_chewy_entry() {
	r64_calc_monitor();
	if (!flags.LoadGame) {
		hide_cur();
		det->show_static_spr(3);
		auto_move(2, P_CHEWY);
		det->hide_static_spr(3);
		show_cur();
	}
}

void r64_calc_monitor() {
	int16 str_nr = 0;
	switch (spieler.R64Moni1Ani) {
	case 3:
		str_nr = 0;
		det->set_static_ani(3, -1);
		break;

	case 5:
		str_nr = 1;
		det->start_detail(5, 255, ANI_VOR);
		break;

	case 0:
		str_nr = 2;
		break;

	}
	atds->set_ats_str(373, str_nr, ATS_DATEI);
	switch (spieler.R64Moni2Ani) {
	case 4:
		str_nr = 0;
		det->set_static_ani(4, -1);
		break;

	case 0:
		str_nr = 1;
		break;

	}
	atds->set_ats_str(374, str_nr, ATS_DATEI);
}

void r64_setup_func() {
	if (r62Delay <= 0 && spieler.R62TShow) {
		r62Delay = (spieler.DelaySpeed + 1) * 60;
		if (r64TalkAni == 3 || r64TalkAni == 4)
			det->stop_detail(r64TalkAni);
		if (r64TalkAni == 4)
			r64TalkAni = spieler.R64Moni1Ani;
		else
			r64TalkAni = spieler.R64Moni2Ani;
		if (r64TalkAni != 0)
			det->start_detail(r64TalkAni, 255, ANI_VOR);
	} else
		--r62Delay;
}

void r64_talk_man() {
	r64_talk_man(350);
}

void r64_talk_man(int16 aad_nr) {
	if (!spieler.R64ManWeg) {
		hide_cur();
		auto_move(4, P_CHEWY);
		room->set_timer_status(1, TIMER_STOP);
		det->del_static_ani(1);
		det->stop_detail(1);
		det->set_static_ani(2, -1);
		start_aad_wait(aad_nr, -1);
		det->del_static_ani(2);
		room->set_timer_status(1, TIMER_START);
		det->set_static_ani(1, -1);
		show_cur();
	}
}

int16 r64_use_tasche() {
	int16 aad_nr;
	int16 action_ret = false;
	hide_cur();
	aad_nr = -1;
	if (!spieler.inv_cur) {
		if (spieler.R64ManWeg) {
			if (!atds->get_steuer_bit(375, ATS_AKTIV_BIT, ATS_DATEI)) {
				auto_move(3, P_CHEWY);
				start_spz_wait(CH_ROCK_GET1, 1, ANI_VOR, P_CHEWY);
				aad_nr = 353;
				new_invent_2_cur(GERAET_INV);
				atds->set_steuer_bit(375, ATS_AKTIV_BIT, ATS_DATEI);
			}
		} else {
			auto_move(3, P_CHEWY);
			aad_nr = 352;
		}
		if (aad_nr != -1) {
			start_aad_wait(aad_nr, -1);
			action_ret = true;
		}
	}
	show_cur();
	return action_ret;
}

void r65_entry() {
	hide_person();
	r65tmp_scrollx = spieler.scrollx;
	r65tmp_scrolly = spieler.scrolly;
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	r65tmp_ch_x = spieler_vector[P_CHEWY].Xypos[0];;
	r65tmp_ch_y = spieler_vector[P_CHEWY].Xypos[1];;
	r65tmp_ho_x = spieler_vector[P_HOWARD].Xypos[0];;
	r65tmp_ho_y = spieler_vector[P_HOWARD].Xypos[1];;
	spieler_vector[P_CHEWY].Xypos[0] = 10;
	spieler_vector[P_CHEWY].Xypos[1] = 80;
	spieler_vector[P_HOWARD].Xypos[0] = 150;
	spieler_vector[P_HOWARD].Xypos[1] = 10;
	if (spieler.PersonDia[P_HOWARD] < 10000) {
		cur_hide_flag = false;
		hide_cur();
		start_aad_wait(spieler.PersonDia[P_HOWARD], -1);
		show_cur();
	} else {
		start_ads_wait(spieler.PersonDia[P_HOWARD] - 10000);
	}
	flags.LoadGame = true;
	show_person();
	switch_room(spieler.PersonDiaTmpRoom[P_HOWARD]);
}

void r65_exit() {
	spieler.scrollx = r65tmp_scrollx;
	spieler.scrolly = r65tmp_scrolly;
	set_person_pos(r65tmp_ch_x, r65tmp_ch_y, P_CHEWY, -1);
	set_person_pos(r65tmp_ho_x, r65tmp_ho_y, P_HOWARD, -1);
	room_blk.AadLoad = true;
	room_blk.AtsLoad = true;
	maus_links_click = false;
	set_person_rnr();
}

void r65_atds_string_start(int16 dia_nr, int16 str_nr,
                           int16 person_nr, int16 mode) {
	int16 ani_nr;
	if (!spieler.PersonDiaRoom[person_nr]) {
		switch (person_nr) {
		case 0:
			if (mode == AAD_STR_START) {
				switch (spieler.ChewyAni) {
				case CHEWY_NORMAL:
					ani_nr = CH_TALK3;
					break;

				case CHEWY_PUMPKIN:
					ani_nr = CH_PUMP_TALK;
					break;

				case CHEWY_ROCKER:
					ani_nr = CH_ROCK_TALK1;
					break;

				case CHEWY_JMANS:
					ani_nr = CH_JM_TALK;
					break;

				default:
					ani_nr = -1;
					break;

				}
				if (ani_nr != -1)
					start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
			} else {
				stop_spz();
			}
			break;

		case 3:
			if (mode == AAD_STR_START) {
				start_spz(NI_TALK_L, 255, ANI_VOR, P_NICHELLE);
			} else {
				stop_spz();
			}
			break;

		}
	} else if (mode == AAD_STR_START) {
		det->start_detail(person_nr, 255, ANI_VOR);
	} else {
			det->stop_detail(person_nr);
	}
}

} // namespace Chewy
