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
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/episode1.h"

namespace Chewy {

#define VOR 0
#define RUECK 1
#define SCHLAUCH1 11
#define SCHLAUCH2 38
#define SCHLAUCH3 59
#define KOPF1 39
#define KOPF2 46
#define KOPF3 48
#define KLAPPE_SPRITE 5
#define SCHLEIM_DETAIL 2
#define CH_ZIEHT_SCHLEIM 3
#define CH_BLITZ 8
#define TUER_DETAIL 9
#define KLAPPE_DETAIL 10
#define SCHLAUCH_DETAIL 11
#define CH_WIRFT_SCHLEIM 12
#define CH_NACH_FUETTERN 13
#define CH_WIRFT_KISSEN 14
#define FUETTER_SCHLAUCH 15
#define STERNE_STEHEN 16
#define ANI_HIDE 0
#define ANI_SHOW 1
int16 e_streifen;

void load_chewy_taf(int16 taf_nr) {
	taf_dateiheader *tafheader;
	const char *fname_;
	if (AkChewyTaf != taf_nr) {
		if (chewy) {
			free((char *)chewy);
			chewy = nullptr;
		}
		spieler_mi[P_CHEWY].HotY = CH_HOT_Y;
		switch (taf_nr) {
		case CHEWY_NORMAL:
			fname_ = CHEWY_TAF;
			chewy_ph_anz = chewy_phasen_anz;
			chewy_ph = (uint8 *)chewy_phasen;
			break;

		case CHEWY_BORK:
			fname_ = CHEWY_BO_TAF;
			chewy_ph_anz = chewy_bo_phasen_anz;
			chewy_ph = (uint8 *)chewy_bo_phasen;
			break;

		case CHEWY_MINI:
			fname_ = CHEWY_MI_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_PUMPKIN:
			fname_ = CHEWY_PUMP_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_ROCKER:
			fname_ = CHEWY_ROCK_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_ro_phasen;
			break;

		case CHEWY_JMANS:
			fname_ = CHEWY_JMAN_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_jm_phasen;
			spieler_mi[P_CHEWY].HotY = 68;
			break;

		default:
			fname_ = NULL;
			break;

		}
		if (fname_ != NULL) {
			spieler.ChewyAni = taf_nr;
			AkChewyTaf = taf_nr;
			chewy = mem->taf_adr(fname_);
			ERROR
			mem->file->get_tafinfo(fname_, &tafheader);
			if (!modul) {
				chewy_kor = chewy->korrektur;
			} else {
				error();
			}
		}
	}
}

void r0_entry() {
	set_person_pos(150, 100, P_CHEWY, P_RIGHT);
	cur_hide_flag = false;
	hide_cur();
	spieler_vector[P_CHEWY].Delay = 5;
	spieler.DelaySpeed = 5;
	auto_move(1, P_CHEWY);
	auto_move(7, P_CHEWY);
	auto_move(1, P_CHEWY);
	auto_move(7, P_CHEWY);
	auto_move(9, P_CHEWY);
	start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
	start_aad_wait(2, -1);
	show_cur();
}

void r0_auge_start(int16 mode) {
	ani_detail_info *adi;
	int16 ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	if (!mode)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 38;

	if (!mode) {

		r0_ani_klappe_delay();
	}
	ende = 0;
	flags.AniUserAction = true;
	if (!mode) {

		det->enable_sound(KLAPPE_DETAIL, 0);
		det->disable_sound(KLAPPE_DETAIL, 1);
		det->enable_sound(SCHLAUCH_DETAIL, 0);
		det->disable_sound(SCHLAUCH_DETAIL, 2);
	} else {

		det->disable_sound(KLAPPE_DETAIL, 0);
		det->enable_sound(KLAPPE_DETAIL, 1);
		det->disable_sound(SCHLAUCH_DETAIL, 0);
		det->enable_sound(SCHLAUCH_DETAIL, 2);
	}
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		if ((adi->ani_count > 11) && (adi->ani_count < 19)) {
			spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH1, ANI_HIDE);
			spr_info[1].ZEbene = 191;
		}
		if (adi->ani_count == 38) {
			spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF1, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		}
		spr_info[3] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		spr_info[3].ZEbene = 193;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		r0_calc_auge_click(3);
		out->back2screen(workpage);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			if (!mode) {
				++adi->ani_count;
				if (adi->ani_count > 38)
					ende = 1;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					ende = 1;
			}
		}
	}
	clear_prog_ani();
	flags.AniUserAction = false;
	if (mode) {
		det->start_detail(KLAPPE_DETAIL, 1, RUECK);
		while (det->get_ani_status(KLAPPE_DETAIL))
			set_ani_screen();
	}
}

void r0_auge_wait() {
	ani_detail_info *adi;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	adi->ani_count = 39;
	adi->delay_count = 15;

	flags.AniUserAction = true;
	while (adi->ani_count < 46) {
		clear_prog_ani();

		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		spr_info[2].ZEbene = 192;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		r0_calc_auge_click(2);
		out->back2screen(workpage);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			++adi->ani_count;
		}
	}
	flags.AniUserAction = false;
	clear_prog_ani();
}

void r0_calc_auge_click(int16 ani_nr) {
	int16 anz;
	int16 x, y;
	int16 i;

	if (mouse_on_prog_ani() == ani_nr) {
		if (minfo.button != 1 && kbinfo.key_code != ENTER) {
			char *str_ = atds->ats_get_txt(172, TXT_MARK_NAME, &anz, ATS_DATEI);
			if (str_ != 0) {
				out->set_fontadr(font8x8);
				out->set_vorschub(fvorx8x8, fvory8x8);
				x = minfo.x;
				y = minfo.y;
				calc_txt_xy(&x, &y, str_, anz);
				for (i = 0; i < anz; i++)
					print_shad(x, y + i * 10, 255, 300, 0, scr_width, txt->str_pos((char *)str_, i));
			}
		} else if (minfo.button == 1 || kbinfo.key_code == ENTER) {
			if (is_cur_inventar(SCHLEIM_INV)) {
				del_inventar(spieler.AkInvent);
				spieler.R0SchleimWurf = 1;
			} else if (is_cur_inventar(KISSEN_INV)) {

				start_ats_wait(172, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void r0_auge_shoot() {
	ani_detail_info *adi;
	int16 ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	adi->ani_count = 47;

	ende = 0;
	det->start_detail(CH_BLITZ, 1, VOR);
	while (!ende) {
		clear_prog_ani();
		spieler.PersonHide[P_CHEWY] = true;
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		if (adi->ani_count < 53) {
			spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		} else {
			spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, 47, ANI_HIDE);
			spr_info[2].ZEbene = 192;
			if (!det->get_ani_status(CH_BLITZ))
				ende = 1;
		}
		set_up_screen(DO_SETUP);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			++adi->ani_count;
		}
	}
	det->start_detail(STERNE_STEHEN, 255, VOR);
	clear_prog_ani();
	spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
	spr_info[0].ZEbene = 190;
	spr_info[1] =
	    det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
	spr_info[1].ZEbene = 191;
	spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF2, ANI_HIDE);
	spr_info[2].ZEbene = 192;
	wait_show_screen(30);
	clear_prog_ani();
	set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
	spieler.PersonHide[P_CHEWY] = false;
}

void r0_auge_schleim_back() {
	ani_detail_info *adi;
	int16 ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	adi->ani_count = 53;

	ende = 0;
	flags.AniUserAction = true;
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		if ((adi->ani_count > 52) && (adi->ani_count < 59)) {
			spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
			spr_info[1].ZEbene = 191;
		}
		if (adi->ani_count == 61) {
			spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH3, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		}
		spr_info[3] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		spr_info[3].ZEbene = 193;
		set_ani_screen();
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			++adi->ani_count;
			if (adi->ani_count == 77)
				ende = 1;
		}
	}
	flags.AniUserAction = false;
	clear_prog_ani();
}

void r0_ch_schleim_auge() {
	ani_detail_info *adi;

	adi = det->get_ani_detail(CH_WIRFT_SCHLEIM);
	adi->ani_count = adi->start_ani;
	if (adi->load_flag) {
		det->load_taf_seq(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, 0);
	}

	while (adi->ani_count < adi->end_ani) {
		clear_prog_ani();
		spieler.PersonHide[P_CHEWY] = true;
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF2, ANI_HIDE);
		spr_info[2].ZEbene = 192;
		spr_info[3] = det->plot_detail_sprite(0, 0, CH_WIRFT_SCHLEIM, adi->ani_count, ANI_HIDE);
		spr_info[3].ZEbene = 193;
		set_up_screen(DO_SETUP);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			++adi->ani_count;
		}
	}
	if (adi->load_flag) {
		det->del_taf_tbl(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, 0);
	}
	clear_prog_ani();
	spieler.PersonHide[P_CHEWY] = false;
}

void r0_fuetter_start(int16 mode) {
	ani_detail_info *adi;
	int16 ende;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	if (!mode)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 135;

	if (!mode) {

		r0_ani_klappe_delay();
		det->enable_sound(KLAPPE_DETAIL, 0);
		det->disable_sound(KLAPPE_DETAIL, 1);
		det->enable_sound(FUETTER_SCHLAUCH, 0);
		det->disable_sound(FUETTER_SCHLAUCH, 2);
	} else {

		det->disable_sound(KLAPPE_DETAIL, 0);
		det->enable_sound(KLAPPE_DETAIL, 1);
		det->disable_sound(FUETTER_SCHLAUCH, 0);
		det->enable_sound(FUETTER_SCHLAUCH, 2);
	}
	ende = 0;
	if (spieler.R0SchleimWurf)
		flags.AniUserAction = true;
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		if (flags.AniUserAction)
			get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		if (!mode)
			r0_calc_kissen_click(1);
		out->back2screen(workpage);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			if (!mode) {
				++adi->ani_count;
				if (adi->ani_count > 135)
					ende = 1;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					ende = 1;
			}
		}
	}
	clear_prog_ani();
	flags.AniUserAction = false;
	if (mode) {
		det->start_detail(KLAPPE_DETAIL, 1, RUECK);
		while (det->get_ani_status(KLAPPE_DETAIL))
			set_ani_screen();

	}
}

void r0_kissen_wurf() {
	int16 i;

	for (i = 0; i < 30 && !spieler.R0KissenWurf; i++) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 136, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		r0_calc_kissen_click(1);
		out->back2screen(workpage);
	}
	clear_prog_ani();
}

void r0_calc_kissen_click(int16 ani_nr) {
	int16 anz;
	int16 x, y;
	int16 i;

	if (mouse_on_prog_ani() == ani_nr) {
		if (minfo.button != 1 && kbinfo.key_code != ENTER) {
			char *str_ = atds->ats_get_txt(173, TXT_MARK_NAME, &anz, ATS_DATEI);
			if (str_ != 0) {
				out->set_fontadr(font8x8);
				out->set_vorschub(fvorx8x8, fvory8x8);
				x = minfo.x;
				y = minfo.y;
				calc_txt_xy(&x, &y, str_, anz);
				for (i = 0; i < anz; i++)
					print_shad(x, y + i * 10, 255, 300, 0, scr_width, txt->str_pos((char *)str_, i));
			}
		} else if (minfo.button == 1 || kbinfo.key_code == ENTER) {
			if (is_cur_inventar(KISSEN_INV) && spieler.R0SchleimWurf) {
				del_inventar(spieler.AkInvent);
				spieler.R0KissenWurf = 1;
			} else if (is_cur_inventar(SCHLEIM_INV)) {
				start_ats_wait(173, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void r0_ch_fuetter() {
	ani_detail_info *adi;
	int16 i;
	int16 ende;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	adi->ani_count = 136;

	i = 152;
	ende = 0;

	if (spieler.R0SchleimWurf)
		flags.AniUserAction = true;
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		if (adi->ani_count == 136) {
			spieler.PersonHide[P_CHEWY] = true;
			if (!spieler.R0SchleimWurf)
				det->stop_detail(16);
		}
		if (adi->ani_count > 138) {
			spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
			spr_info[1].ZEbene = 191;
		}
		if (adi->ani_count > 141) {
			spr_info[2] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, i, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		}
		if (adi->ani_count == 138) {
			spr_info[3] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 139, ANI_HIDE);
			spr_info[3].ZEbene = 193;
		}
		spr_info[4] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		spr_info[4].ZEbene = 194;
		set_ani_screen();
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			if (adi->ani_count > 141)
				++i;
			++adi->ani_count;
			if (adi->ani_count == 152)
				ende = 1;
		}
	}

	adi->ani_count = 138;

	ende = 0;
	det->start_detail(CH_NACH_FUETTERN, 2, VOR);
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		if (adi->ani_count > 138) {
			spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
			spr_info[1].ZEbene = 191;
		}
		if (adi->ani_count == 138) {
			spr_info[2] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 139, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		}
		spr_info[3] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		spr_info[3].ZEbene = 193;
		set_ani_screen();
		if (!det->get_ani_status(CH_NACH_FUETTERN))
			ende = 1;
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + spieler.DelaySpeed;
			if (adi->ani_count > 135)
				--adi->ani_count;
		}
	}
	spieler.PersonHide[P_CHEWY] = false;
	flags.AniUserAction = false;
	clear_prog_ani();
}

void r0_ch_kissen() {
	ani_detail_info *adi;
	int16 ende, mode;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	adi->ani_count = 161;

	ende = 0;
	spieler.PersonHide[P_CHEWY] = true;
	det->start_detail(CH_WIRFT_KISSEN, 1, VOR);
	mode = 0;
	while (!ende) {
		clear_prog_ani();
		if (!det->get_ani_status(CH_WIRFT_KISSEN)) {
			mode = 1;
			spieler.PersonHide[P_CHEWY] = false;
			set_person_pos(228 - CH_HOT_MOV_X, 143 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		}
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		if (mode) {
			spr_info[2] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
			spr_info[2].ZEbene = 192;
		}
		set_up_screen(DO_SETUP);
		if (mode) {
			if (adi->delay_count > 0)
				--adi->delay_count;
			else {
				adi->delay_count = adi->delay + spieler.DelaySpeed;
				--adi->ani_count;
				if (adi->ani_count == 151)
					ende = 1;
			}
		}
	}
	clear_prog_ani();
}

void r0_ani_klappe_delay() {
	int16 i;
	det->start_detail(KLAPPE_DETAIL, 1, VOR);
	while (det->get_ani_status(KLAPPE_DETAIL)) {
		set_ani_screen();

	}
	flags.AniUserAction = true;
	for (i = 0; i < 25; i++) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		set_ani_screen();
	}
	flags.AniUserAction = false;
	clear_prog_ani();
}

void r0_auge_ani() {
	if (!spieler.R0SchleimWurf) {
		r0_auge_start(0);
		if (!spieler.R0SchleimWurf)
			r0_auge_wait();
		if (spieler.R0SchleimWurf) {
			start_aad(124);
			r0_ch_schleim_auge();
			r0_auge_schleim_back();
			auto_move(FUETTER_POS, P_CHEWY);
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		} else {
			r0_auge_shoot();
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
			r0_auge_start(1);
		}
	}
}

void r0_fuett_ani() {
	int16 action;
	action = false;
	r0_fuetter_start(0);
	if (spieler.R0SchleimWurf) {
		r0_kissen_wurf();
		if (spieler.R0KissenWurf) {
			r0_ch_kissen();
			r0_fuetter_start(1);
			auto_move(VERSTECK_POS, P_CHEWY);
			set_up_screen(DO_SETUP);
			out->cls();
			flic_cut(FCUT_001, CFO_MODE);

			spieler.PersonRoomNr[P_CHEWY] = 1;
			room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
			ERROR
			set_person_pos(Rdi->AutoMov[4].X - CH_HOT_MOV_X,
			               Rdi->AutoMov[4].Y - CH_HOT_MOV_Y, P_CHEWY, P_RIGHT);
			spieler_vector[P_CHEWY].DelayCount = 0;

			check_shad(4, 0);
			fx_blende = 1;
			set_up_screen(DO_SETUP);
		} else
			action = true;
	} else
		action = true;
	if (action) {
		r0_ch_fuetter();
		start_spz(CH_EKEL, 3, ANI_VOR, P_CHEWY);
		start_aad(55);
		r0_fuetter_start(1);
	}
}

#define HAND_NORMAL 68
#define HAND_CLICK 69
#define RAHMEN_ROT 70
#define RAHMEN_GELB 71
int16 r4_sonde_comp() {
	int16 ende;

	int16 spr_nr;
	int16 cur_x;

	int16 cur_pos[3][2] = { {  83, 106 },
	                        { 136, 103 },
	                        { 188, 101 }
	                      };

	int16 console[3][4] = { {  82, 158, 143, 199 },
	                        { 150, 159, 194, 193 },
	                        { 201, 154, 262, 193 }
	                      };

	cur_2_inventory();
	spieler.PersonHide[P_CHEWY] = true;
	cur_display = false;
	switch_room(4);
	cur_display = true;
	ende = 0;
	curblk.sprite = room_blk.DetImage;
	cur_x = 1;
	spr_nr = RAHMEN_ROT;
	cur->move(160, 160);

	start_aad(46);
	while (!ende) {
		maus_action();
		if (maus_links_click) {
			switch (in->maus_vector(minfo.x + 17, minfo.y + 7, (int16 *)console, 3)) {
			case 0:
				if (cur_x > 0)
					--cur_x;
				else
					cur_x = 2;
				det->play_sound(0, 1);
				break;

			case 1:
				ende = 1;
				spr_nr = RAHMEN_GELB;
				det->play_sound(0, 0);
				break;

			case 2:
				if (cur_x < 2)
					++cur_x;
				else
					cur_x = 0;
				det->play_sound(0, 2);
				break;

			}
		}
		spr_info[0].Image = room_blk.DetImage[spr_nr];

		spr_info[0].ZEbene = 0;
		spr_info[0].X = cur_pos[cur_x][0];
		spr_info[0].Y = cur_pos[cur_x][1];
		if (minfo.button == 1 || kbinfo.key_code == ENTER) {
			curani.ani_anf = HAND_CLICK;
			curani.ani_end = HAND_CLICK;
		} else {
			curani.ani_anf = HAND_NORMAL;
			curani.ani_end = HAND_NORMAL;
		}
		cursor_wahl(CUR_USER);
		spieler.CurHoehe = 16;
		in->rectangle(0, 123, 320 - spieler.CurBreite, 194);
		if (minfo.y < 124) {
			minfo.y = 123;
		}
		set_up_screen(DO_SETUP);
	}
	g_events->delay(500);

	clear_prog_ani();
	spieler.PersonHide[P_CHEWY] = false;
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);
	set_person_pos(118 - CH_HOT_MOV_X,
	               164 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
	spieler_vector[P_CHEWY].DelayCount = 0;
	maus_links_click = false;
	minfo.button = 0;
	spieler.PersonRoomNr[P_CHEWY] = 3;
	room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
	ERROR
	fx_blende = 1;
	atds->stop_aad();
	return (cur_x);
}

void switch_room(int16 nr) {
	fx_blende = 1;
	exit_room(-1);
	spieler.PersonRoomNr[P_CHEWY] = nr;
	room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
	ERROR

	enter_room(-1);
	set_up_screen(DO_SETUP);
}

void r3_terminal() {
	show_cur();
	auto_obj = 0;
	switch (r4_sonde_comp()) {
	case 0:
		r3_sonde_aufnahme();
		spr_info[0].Image = room_blk.DetImage[120];
		spr_info[0].X = 250;
		spr_info[0].Y = 2;
		spr_info[0].ZEbene = 0;

		if (!spieler.R2KabelBork) {
			det->stop_detail(5);
			start_ani_block(2, ablock3);
			if (spieler.R2FussSchleim) {
				spieler.R2FussSchleim = 0;

				spieler.room_s_obj[SIB_SCHLEIM].ZustandFlipFlop = 1;
			}
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_wait(8, 1, ANI_VOR);
			set_up_screen(DO_SETUP);
			det->stop_detail(6);
			clear_prog_ani();
			switch_room(1);
			start_ani_block(2, ablock0);
			set_person_pos(92, 131, P_CHEWY, P_LEFT);
			spieler.PersonHide[P_CHEWY] = false;
		} else if (!spieler.R2FussSchleim) {
			start_ani_block(2, ablock2);
			r2_jump_out_r1(9);
			start_aad_wait(45, -1);
		} else {
			spieler.PersonHide[P_CHEWY] = true;
			start_ani_block(2, ablock1);
			spieler.PersonRoomNr[P_CHEWY] = 5;
			clear_prog_ani();
			auto_obj = 0;
			room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
			ERROR
			fx_blende = 1;
			start_ani_block(3, ablock5);
			set_person_pos(91, 107, P_CHEWY, P_LEFT);
			spieler.PersonHide[P_CHEWY] = false;
		}
		clear_prog_ani();
		break;

	case 1:
		auto_obj = 1;
		maus_links_click = 0;
		minfo.button = 0;
		stop_person(P_CHEWY);
		start_aad_wait(51, -1);
		set_up_screen(DO_SETUP);
		break;

	case 2:
		r3_sonde_knarre();
		break;

	}
}

MovLine sonde_mpkt[3] = {
	{ {  13, 45,  75 }, 1, 2 },
	{ { 237, 52, 160 }, 1, 2 },
	{ {   4, 83, 180 }, 0, 2 }
};
int16 sonde_phasen[4][2] = {
	{ 120, 120 },
	{ 118, 118 },
	{ 141, 141 },
	{ 119, 119 }
};

void r3_init_sonde() {
#define SONDE_OBJ 0
#define SONDE_OBJ1 1
	auto_obj = 1;
	mov_phasen[SONDE_OBJ].AtsText = 24;
	mov_phasen[SONDE_OBJ].Lines = 3;
	mov_phasen[SONDE_OBJ].Repeat = 255;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = 1;
	init_auto_obj(SONDE_OBJ, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine *)sonde_mpkt);
}

#define KOPF_SCHUSS 0
#define SONDE_ANI 2
#define SONDE_SHOOT 3
#define SONDE_RET 4
#define SONDE_RAUCH 5
#define SONDE_REIN 6
#define SONDE_GREIF 7
#define SONDE_GREIF1 8

#define SONDE_SPR_R 118
#define SONDE_SPR_L 120

void r3_sonde_knarre() {
	MovLine sonde_mpkt1[3] = {
		{ { 237,  52, 160 }, 0, 2 },
		{ {  13,  45,  75 }, 0, 4 },
		{ {   4, 100, 180 }, 1, 2 }
	};
	MovLine sonde_mpkt2[2] = {
		{ {   4, 100, 180 }, 1, 3 },
		{ {  13,  45,  75 }, 1, 3 }
	};
	room_detail_info *rdi;
	int16 tmp;
	int16 ende;
	hide_cur();
	rdi = det->get_room_detail_info();

	det->load_taf_seq(162, 17, 0);
	tmp = zoom_horizont;
	zoom_horizont = 100;
	auto_obj = 1;
	mov_phasen[SONDE_OBJ].Lines = 3;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = 1;
	init_auto_obj(SONDE_OBJ, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine
	              *)sonde_mpkt1);
	flags.AniUserAction = false;
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();

	det->start_detail(SONDE_SHOOT, 1, ANI_VOR);
	ende = 0;
	while (!ende) {
		clear_prog_ani();
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 170) {
			spieler.PersonHide[P_CHEWY] = true;
			det->start_detail(KOPF_SCHUSS, 1, ANI_VOR);
		}
		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count >= 13 &&
		        rdi->Ainfo[KOPF_SCHUSS].ani_count <= 21) {
			spr_info[1] = det->plot_detail_sprite(0, 0, KOPF_SCHUSS, 21, ANI_HIDE);
			spr_info[1].ZEbene = 190;
		}
		else if (rdi->Ainfo[KOPF_SCHUSS].ani_count > 21 &&
		         det->get_ani_status(SONDE_RET) == 0) {

			spr_info[2] = det->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
			spr_info[2].ZEbene = 190;
		}
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 178) {
			det->start_detail(SONDE_RET, 3, ANI_RUECK);
		}
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count == 28) {
			ende = 1;
		}
		set_ani_screen();
	}

	clear_prog_ani();
	det->start_detail(SONDE_RAUCH, 3, ANI_VOR);
	ende = 0;
	while (det->get_ani_status(SONDE_RAUCH)) {

		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;

		spr_info[1] = det->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
		spr_info[1].ZEbene = 190;
		++ende;
		if (ende == 3)
			spieler.PersonHide[P_CHEWY] = false;
		set_ani_screen();
	}
	start_aad_wait(53, -1);

	clear_prog_ani();
	det->start_detail(SONDE_REIN, 1, ANI_RUECK);
	while (det->get_ani_status(SONDE_REIN)) {

		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;
		set_ani_screen();
	}
	clear_prog_ani();

	mov_phasen[SONDE_OBJ].Lines = 2;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 28;
	init_auto_obj(SONDE_OBJ, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine
	              *)sonde_mpkt2);
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();
	det->del_taf_tbl(162, 17, 0);
	zoom_horizont = tmp;
	r3_init_sonde();

	show_cur();
}

void r3_sonde_aufnahme() {
	int16 tmp;
	int16 i;
	int16 spr_nr;
	int16 anistart;
	room_detail_info *rdi;

	MovLine sonde_mpkt1[2] = {
		{ { 237,  52, 160 }, 0, 2 },
		{ { 144, 100, 180 }, 0, 2 }
	};
	MovLine sonde_mpkt_[4][2] = {
		{ { { 144, 100, 180 }, 0, 2 },
		  { { 110, 100, 180 }, 0, 2 } },

		{ { { 110, 101, 180 }, 0, 3 },
		  { { -55,  50, 180 }, 0, 3 } },

		{ { { 310,  20, 180 }, 0, 3 },
		  { { -55,  20, 180 }, 0, 3 } },

		{ { { 310,   2, 180 }, 0, 3 },
		  { { 250,   2, 180 }, 0, 3 } }
	};
	flags.AniUserAction = false;
	hide_cur();
	rdi = det->get_room_detail_info();
	tmp = zoom_horizont;
	zoom_horizont = 100;
	anistart = false;
	auto_obj = 1;
	mov_phasen[SONDE_OBJ].Lines = 2;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = 1;
	init_auto_obj(SONDE_OBJ, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine *)sonde_mpkt1);
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();

	det->start_detail(SONDE_GREIF, 1, ANI_VOR);
	while (det->get_ani_status(SONDE_GREIF)) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_GREIF, SONDE_SPR_L, ANI_HIDE);
		spr_info[0].ZEbene = 146;
		set_ani_screen();
	}

	clear_prog_ani();
	auto_obj = 2;

	spr_nr = 140;
	for (i = 0; i < 4; i++) {

		mov_phasen [SONDE_OBJ].Lines = 2;
		mov_phasen [SONDE_OBJ].Repeat = 1;
		mov_phasen [SONDE_OBJ].ZoomFak = 0;
		auto_mov_obj [SONDE_OBJ].Id = AUTO_OBJ0;
		auto_mov_obj [SONDE_OBJ].Mode = 1;
		auto_mov_vector[SONDE_OBJ].Delay = spieler.DelaySpeed;
		init_auto_obj(SONDE_OBJ, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ].Lines, sonde_mpkt_[i]);
		mov_phasen [SONDE_OBJ1].Lines = 2;
		mov_phasen [SONDE_OBJ1].Repeat = 1;
		mov_phasen [SONDE_OBJ1].ZoomFak = 0;
		auto_mov_obj [SONDE_OBJ1].Id = AUTO_OBJ1;
		auto_mov_obj [SONDE_OBJ1].Mode = 1;
		auto_mov_vector[SONDE_OBJ1].Delay = spieler.DelaySpeed;
		init_auto_obj(SONDE_OBJ1, &sonde_phasen[0][0], mov_phasen[SONDE_OBJ1].Lines, sonde_mpkt_[i]);
		mov_phasen [SONDE_OBJ1].Phase[0][0] = spr_nr;
		mov_phasen [SONDE_OBJ1].Phase[0][1] = spr_nr;
		while (mov_phasen[SONDE_OBJ].Repeat != -1) {

			if (i == 2 || i == 1) {

				if (mouse_auto_obj(SONDE_OBJ, 50, 100)) {
					if (minfo.button == 1 || kbinfo.key_code == ENTER) {
						if (is_cur_inventar(SPINAT_INV)) {
							ssi[0].X = 120;
							ssi[0].Y = 100;
							if (spieler.PersonRoomNr[P_CHEWY] == 3)

								start_aad(50);
							else

								start_aad(44);
							del_inventar(spieler.AkInvent);
							spieler.R2FussSchleim = 1;
							mov_phasen[SONDE_OBJ1].Phase[0][0] = 142;
							mov_phasen[SONDE_OBJ1].Phase[0][1] = 149;
							auto_mov_vector[SONDE_OBJ1].PhAnz = 8;
							anistart = true;
						}
					}
				}
			}
			set_ani_screen();
			if (anistart) {
				if (auto_mov_vector[SONDE_OBJ1].PhNr == 7) {
					anistart = false;
					spr_nr = 149;
					mov_phasen[SONDE_OBJ1].Phase[0][0] = spr_nr;
					mov_phasen[SONDE_OBJ1].Phase[0][1] = spr_nr;
					auto_mov_vector[SONDE_OBJ1].PhAnz = 1;
					auto_mov_vector[SONDE_OBJ1].PhNr = 0;
				}
			}
		}
		switch (i) {
		case 0:
			show_cur();
			start_aad(52);
			flags.AniUserAction = true;
			spr_nr = 141;
			spieler.PersonHide[P_CHEWY] = true;
			det->load_taf_seq(142, 8, 0);
			break;

		case 1:
			switch_room(1);
			break;

		case 2:
			det->del_taf_tbl(142, 7, 0);
			flags.AniUserAction = false;
			switch_room(2);
			break;

		}
	}
	flags.AniUserAction = false;
	zoom_horizont = tmp;
	auto_obj = 0;
}

void r2_jump_out_r1(int16 nr) {
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(nr, 1, ANI_VOR);
	set_up_screen(DO_SETUP);
	det->stop_detail(6);
	set_person_pos(32, 127, P_CHEWY, P_LEFT);
	spieler.PersonHide[P_CHEWY] = false;
	clear_prog_ani();
	switch_room(1);
	check_shad(2, 1);
}

void r5_knopf() {
	int16 str_nr;
	if (spieler.R5Terminal) {
		if (spieler.R5Tuer == false) {
			start_detail_wait(9, 1, ANI_VOR);
			spieler.room_e_obj[6].Attribut = AUSGANG_OBEN;
			str_nr = 1;
		} else {
			start_detail_wait(9, 1, ANI_RUECK);
			spieler.room_e_obj[6].Attribut = 255;
			str_nr = 0;
		}
		atds->set_ats_str(29, str_nr, ATS_DATEI);
		spieler.R5Tuer ^= 1;
		obj->calc_rsi_flip_flop(SIB_TUERE_R5);
	} else
		start_aad_wait(1, -1);
}

void r6_entry() {
	zoom_horizont = 80;
	flags.ZoomMov = true;
	zoom_mov_fak = 2;
	if (spieler.R6BolaSchild) {
		if (spieler.R6RaumBetreten < 3) {
			det->start_detail(7, 255, ANI_VOR);
			atds->del_steuer_bit(44, ATS_AKTIV_BIT, ATS_DATEI);
			if (!flags.LoadGame)
				++spieler.R6RaumBetreten;
			if (spieler.R6RaumBetreten == 3) {
				det->stop_detail(7);
				r6_init_robo();
				wait_auto_obj(0);
				spieler.R6BolaOk = true;
				obj->show_sib(SIB_BOLA_KNOPF_R6);
				obj->hide_sib(SIB_BOLA_R6);
				atds->set_steuer_bit(44, ATS_AKTIV_BIT, ATS_DATEI);
			}
		}
	}
}

int16 r6_robo_phasen[4][2] = {
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 },
	{ 86, 86 }
};

MovLine r6_robo_mpkt[3] = {
	{ { 168,  71, 180 }, 1,  1 },
	{ { 180,  71, 100 }, 1,  2 },
	{ {  60, 210, 110 }, 1, 16 }
};

void r6_init_robo() {
#define ROBO_OBJ 0
	auto_obj = 1;
	mov_phasen[ROBO_OBJ].AtsText = 44;
	mov_phasen[ROBO_OBJ].Lines = 3;
	mov_phasen[ROBO_OBJ].Repeat = 1;
	mov_phasen[ROBO_OBJ].ZoomFak = 0;
	auto_mov_obj[ROBO_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[ROBO_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[ROBO_OBJ].Mode = 1;
	init_auto_obj(ROBO_OBJ, &r6_robo_phasen[0][0], mov_phasen[ROBO_OBJ].Lines, (MovLine
	              *)r6_robo_mpkt);
}

void r6_bola_knopf() {
	int16 tmp;
	if (!spieler.R6BolaBecher) {
		det->hide_static_spr(0);
		start_detail_wait(0, 1, ANI_VOR);
		if (spieler.R6BolaOk) {
			spieler.R6BolaBecher = true;
			det->show_static_spr(0);
			start_ani_block(2, ablock7);
			obj->calc_rsi_flip_flop(SIB_BOLA_FLECK_R6);
			wait_detail(2);
			obj->calc_rsi_flip_flop(SIB_BOLA_SCHACHT);
			atds->del_steuer_bit(42, ATS_AKTIV_BIT, ATS_DATEI);
			atds->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATEI);
			obj->calc_rsi_flip_flop(SIB_BOLA_KNOPF_R6);
			obj->hide_sib(SIB_BOLA_KNOPF_R6);
		} else {
			spieler.PersonHide[P_CHEWY] = true;
			start_ani_block(3, ablock6);
			while (det->get_ani_status(3)) {
				if (!det->get_ani_status(14)) {
					set_person_pos(220, 89, P_CHEWY, P_LEFT);
					spieler.PersonHide[P_CHEWY] = false;
				}
				set_up_screen(DO_SETUP);
			}
			det->show_static_spr(0);
			++spieler.R6BolaJoke;
			if (spieler.R6BolaJoke < 3)
				tmp = 3;
			else
				tmp = 4;
			start_aad_wait(tmp, -1);
		}
		obj->calc_rsi_flip_flop(SIB_BOLA_KNOPF_R6);
	}
}

void r7_haken(int16 sib_nr) {
	int16 dia_nr;
	del_inventar(8);

	spieler.AkInvent = -1;
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);
	if (spieler.R7RHaken) {
		spieler.R7SeilOk = true;
		if (sib_nr == SIB_LHAKEN_R7) {
			obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			obj->calc_rsi_flip_flop(SIB_RHAKEN_R7);
		}
		atds->del_steuer_bit(56, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATEI);
		dia_nr = 9;
	} else {
		spieler.R7SeilLeft = true;
		dia_nr = 48;
	}
	atds->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATEI);
	start_aad(dia_nr);
}

void r7_klingel() {
	if ((!spieler.R7BellCount) ||
	        (spieler.R7BellCount >= 2 && spieler.R7SeilLeft != 0 && !spieler.R7SeilOk)) {
		spieler.PersonHide[P_CHEWY] = true;
		start_aad(5, 0);
		start_ani_block(3, ablock25);

		det->show_static_spr(7);
		start_detail_wait(12, 1, ANI_VOR);
		start_detail_wait(11, 1, ANI_VOR);
		det->hide_static_spr(7);
		det->stop_detail(5);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		spieler.PersonHide[P_CHEWY] = false;
	}
	else if (spieler.R7BellCount == 1) {
		spieler.PersonHide[P_CHEWY] = true;
		start_aad(6, 0);
		start_ani_block(3, ablock25);
		det->show_static_spr(7);
		start_detail_wait(10, 1, ANI_VOR);
		det->start_detail(13, 1, ANI_VOR);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		det->stop_detail(5);
		spieler.PersonHide[P_CHEWY] = false;
		flags.NoScroll = true;
		auto_move(6, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		det->start_detail(0, 255, ANI_VOR);
		start_detail_wait(13, 1, ANI_VOR);
		flic_cut(FCUT_002, CFO_MODE);
		det->stop_detail(0);
		spieler.scrollx = 0;
		spieler.scrolly = 0;
		start_detail_frame(19, 1, ANI_VOR, 6);
		start_detail_frame(9, 1, ANI_VOR, 4);
		det->show_static_spr(9);
		wait_detail(9);
		det->hide_static_spr(9);
		obj->show_sib(SIB_SCHLOTT_R7);
		obj->calc_rsi_flip_flop(SIB_SCHLOTT_R7);
		set_person_pos(114, 138, P_CHEWY, -1);
		spieler.PersonHide[P_CHEWY] = false;
		flags.NoScroll = false;
		det->hide_static_spr(7);
	} else if (!spieler.R7SeilOk) {
		spieler.PersonHide[P_CHEWY] = true;
		start_aad(7, 0);
		start_ani_block(3, ablock25);
		det->show_static_spr(7);
		det->load_taf_seq(192, 74, 0);
		det->start_detail(14, 1, ANI_VOR);
		set_person_pos(95, 94, P_CHEWY, P_RIGHT);
		det->stop_detail(5);
		spieler.PersonHide[P_CHEWY] = false;
		flags.NoScroll = true;
		auto_move(6, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		det->start_detail(0, 255, ANI_VOR);
		wait_detail(14);
		start_ani_block(4, ablock10);
		det->hide_static_spr(7);
		det->stop_detail(0);
		set_person_pos(181, 130, P_CHEWY, P_RIGHT);
		spieler.PersonHide[P_CHEWY] = false;
		flags.NoScroll = false;
		det->del_taf_tbl(192, 74, 0);
	} else if (spieler.R7SeilOk && !spieler.R7BorkFlug) {
		spieler.R7BorkFlug = true;
		spieler.PersonHide[P_CHEWY] = true;
		start_aad(8, 0);
		start_detail_wait(1, 1, ANI_VOR);
		obj->set_rsi_flip_flop(SIB_TKNOPF2_R7, 255);
		obj->hide_sib(SIB_KLINGEL_R7);
		flags.NoPalAfterFlc = false;
		flic_cut(FCUT_003, CFO_MODE);
		set_person_pos(201, 117, P_CHEWY, P_LEFT);
		spieler.scrollx = 0;
		spieler.scrolly = 0;
		spieler.PersonHide[P_CHEWY] = false;
	}
	++spieler.R7BellCount;
}

void r8_entry() {
	spieler.R7ChewyFlug = true;
	if (!spieler.R8Folter)
		r8_start_folter();
	else
		r8_stop_folter();
	atds->set_ats_str(60, 0, ATS_DATEI);
}

void r8_start_folter() {
	atds->set_ats_str(67, 1, ATS_DATEI);

	det->stop_detail(19);

	det->start_detail(13, 255, ANI_VOR);
}

void r8_stop_folter() {
	atds->set_ats_str(67, 0, ATS_DATEI);
	det->start_detail(19, 255, ANI_VOR);

	det->stop_detail(13);

	spieler.R8Folter = true;
	obj->hide_sib(SIB_FOLTER_R8);
}

void r8_hole_kohle() {
	if (!spieler.R8Kohle) {
		hide_cur();
		spieler.R8Kohle = true;
		auto_move(4, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(12, 1, ANI_VOR);
		cur_2_inventory();
		invent_2_slot(KOHLE_HEISS_INV);
		spieler.PersonHide[P_CHEWY] = false;
		show_cur();
	}
}

void r8_start_verbrennen() {
	int16 ende;
	hide_cur();
	if (!spieler.inv_cur) {
		auto_move(3, P_CHEWY);
		start_aad(102, 0);
		spieler.PersonHide[P_CHEWY] = true;
		start_ani_block(2, ablock12);
		ende = 0;
		while (!ende && det->get_ani_status(9)) {

			set_up_screen(DO_SETUP);
			if (minfo.button == 1 || kbinfo.key_code == ENTER) {
				if (minfo.x > 146 && minfo.x < 208 &&
				        minfo.y > 107 && minfo.y < 155)
					ende = 1;
			}
		}
		det->stop_detail(9);
		set_person_pos(129, 246, P_CHEWY, P_RIGHT);
		start_ani_block(2, ablock13);
		atds->set_ats_str(60, TXT_MARK_LOOK, 1, ATS_DATEI);
		spieler.PersonHide[P_CHEWY] = false;
	}
	show_cur();
}

void r8_gips_wurf() {
	hide_cur();
	det->load_taf_seq(116, 30, 0);
	auto_move(2, P_CHEWY);
	maus_links_click = 0;
	spieler.PersonHide[P_CHEWY] = true;
	del_inventar(GIPS_EIMER_INV);
	start_detail_wait(4, 1, ANI_VOR);
	spieler.PersonHide[P_CHEWY] = false;
	start_detail_frame(5, 1, ANI_VOR, 16);
	start_detail_wait(6, 1, ANI_VOR);
	obj->show_sib(33);
	det->show_static_spr(14);
	wait_detail(5);
	spieler.R8GipsWurf = true;
	spieler.room_m_obj[MASKE_INV].ZEbene = 0;
	obj->set_inventar(MASKE_INV, 181, 251, 8, &room_blk);
	det->del_taf_tbl(116, 30, 0);
	auto_move(8, P_CHEWY);
	flags.AtsAction = false;
	menu_item = CUR_USE;
	look_invent_screen(INVENTAR_NORMAL, 178);
	flags.AtsAction = true;
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(20, 1, ANI_VOR);
	spieler.PersonHide[P_CHEWY] = false;
	invent_2_slot(MASKE_INV);
	cursor_wahl(menu_item);
	show_cur();
}

void r8_open_gdoor() {
	spieler.PersonHide[P_CHEWY] = true;
	det->show_static_spr(17);
	set_up_screen(DO_SETUP);
	start_detail_wait(7, 1, ANI_VOR);
	det->show_static_spr(15);
	det->hide_static_spr(17);
	spieler.PersonHide[P_CHEWY] = false;
	set_person_pos(204, 274, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(69, ATS_AKTIV_BIT, ATS_DATEI);
	obj->hide_sib(31);
	spieler.R8GTuer = true;
	spieler.room_e_obj[15].Attribut = AUSGANG_UNTEN;
}

void r8_talk_nimoy() {
	int16 tmp;
	auto_move(9, P_CHEWY);
	flags.NoScroll = true;
	auto_scroll(0, 120);
	if (spieler.R8Folter) {
		if (!spieler.R8GipsWurf)
			tmp = 1;
		else
			tmp = 2;
		if (!spieler.R8GTuer)
			load_ads_dia(tmp);
		else
			start_aad_wait(61, -1);
	} else
		load_ads_dia(6);
	flags.NoScroll = false;
}

void r9_entry() {
	spieler.R7ChewyFlug = false;

	if (!spieler.R9Gitter) {
		set_person_pos(138, 91, P_CHEWY, P_LEFT);
	} else
		det->show_static_spr(5);
	if (spieler.R9Surimy) {
		det->hide_static_spr(4);
		room->set_timer_status(7, TIMER_STOP);
	}
}

void r9_gtuer() {
	spieler.R9Gitter = true;
	det->show_static_spr(5);
	start_detail_wait(6, 1, ANI_VOR);
	set_person_pos(74, 93, P_CHEWY, P_LEFT);
	atds->del_steuer_bit(74, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(75, ATS_AKTIV_BIT, ATS_DATEI);
	atds->del_steuer_bit(76, ATS_AKTIV_BIT, ATS_DATEI);
	obj->show_sib(34);
	atds->set_ats_str(73, 1, ATS_DATEI);
}

#define SURIMY_OBJ 0
int16 r9_surimy_phasen[4][2] = {
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 },
	{ 91, 98 }
};

MovLine r9_surimy_mpkt[2] = {
	{ {  80, 170, 199 }, 2, 6 },
	{ { 210, 162, 199 }, 2, 6 }
};
MovLine r9_surimy_mpkt1[2] = {
	{ { 224, 158, 199 }, 2, 6 },
	{ { 330, 162, 199 }, 2, 6 }
};

void r9_surimy() {
	int16 tmp;
	spieler.R9Surimy = true;
	tmp = spieler_vector[P_CHEWY].Count;
	stop_person(P_CHEWY);
	atds->set_steuer_bit(75, ATS_AKTIV_BIT, ATS_DATEI);
	det->hide_static_spr(4);
	room->set_timer_status(7, TIMER_STOP);
	r9_surimy_ani();
	start_aad_wait(56, -1);
	spieler.room_e_obj[17].Attribut = AUSGANG_RECHTS;
	spieler_vector[P_CHEWY].Count = tmp;
	get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
	mov->continue_auto_go();
}

void r9_surimy_ani() {
	det->load_taf_seq(91, 8, 0);
	auto_obj = 1;
	mov_phasen[SURIMY_OBJ].AtsText = 0;
	mov_phasen[SURIMY_OBJ].Lines = 2;
	mov_phasen[SURIMY_OBJ].Repeat = 1;
	mov_phasen[SURIMY_OBJ].ZoomFak = 0;
	auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SURIMY_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[SURIMY_OBJ].Mode = 1;
	init_auto_obj(SURIMY_OBJ, &r9_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
	              *)r9_surimy_mpkt);
	wait_auto_obj(SURIMY_OBJ);
	start_detail_frame(0, 1, ANI_VOR, 15);
	det->start_detail(2, 1, ANI_VOR);
	wait_detail(0);
	start_detail_wait(1, 1, ANI_VOR);
	start_spz(CH_EKEL, 2, ANI_VOR, P_CHEWY);
	det->hide_static_spr(4);
	mov_phasen[SURIMY_OBJ].Repeat = 1;
	init_auto_obj(SURIMY_OBJ, &r9_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
	              *)r9_surimy_mpkt1);
	wait_auto_obj(SURIMY_OBJ);
	det->del_taf_tbl(91, 8, 0);
}

void r10_entry() {
	if (!spieler.R10Surimy) {
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
		out->setze_zeiger(0);
		fx->blende1(workptr, screen0, pal, 150, 0, 0);
		spieler.R10Surimy = true;
		flic_cut(FCUT_004, CFO_MODE);
		start_aad(101, 0);
		fx_blende = 0;
		set_person_pos(1, 130, P_CHEWY, P_RIGHT);
		auto_move(2, P_CHEWY);
	} else if (spieler.R10SurimyOk)
		room->set_timer_status(3, TIMER_STOP);
}

void r10_get_surimy() {
	auto_move(4, P_CHEWY);
	start_aad(104, 0);
	flc->set_custom_user_function(r6_cut_serv2);
	flic_cut(FCUT_006, CFO_MODE);
	flc->remove_custom_user_function();
	spieler.R10SurimyOk = true;
	room->set_timer_status(3, TIMER_STOP);
	atds->set_ats_str(77, TXT_MARK_LOOK, 1, ATS_DATEI);
	invent_2_slot(18);

	del_inventar(spieler.AkInvent);
}

void r11_entry() {
	zoom_horizont = 80;
	flags.ZoomMov = true;
	zoom_mov_fak = 2;

	if (spieler.R12ChewyBork) {
		if (!spieler.R11DoorRightB) {
			obj->calc_rsi_flip_flop(SIB_TKNOPF2_R11);
			spieler.R11DoorRightB = exit_flip_flop(5, 22, -1, 98, -1, -1,
			                                       AUSGANG_OBEN, -1, (int16)spieler.R11DoorRightB);
			obj->calc_all_static_detail();
		}

		obj->hide_sib(SIB_TKNOPF1_R11);
		obj->hide_sib(SIB_SCHLITZ_R11);
		obj->hide_sib(SIB_TKNOPF2_R11);
		obj->hide_sib(SIB_TKNOPF3_R11);
		spieler.room_e_obj[20].Attribut = 255;
		spieler.room_e_obj[21].Attribut = 255;
		atds->del_steuer_bit(121, ATS_AKTIV_BIT, ATS_DATEI);
		if (!flags.LoadGame) {
			start_ani_block(2, ablock17);
			auto_move(8, P_CHEWY);
			start_aad_wait(31, -1);
			det->stop_detail(9);
		}
		det->show_static_spr(8);
		if (!flags.LoadGame)
			auto_move(6, P_CHEWY);
		timer_nr[0] = room->set_timer(255, 10);
	}
}

void r11_bork_zwinkert() {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		det->hide_static_spr(8);
		start_ani_block(2, ablock18);
		uhr->reset_timer(timer_nr[0], 0);
		det->show_static_spr(8);
		flags.AutoAniPlay = false;
	}
}

void r11_talk_debug() {
	if (spieler.R12ChewyBork) {
		flags.AutoAniPlay = true;
		auto_move(8, P_CHEWY);
		start_ads_wait(5);
		flags.AutoAniPlay = false;

	}
}

void r11_chewy_bo_use() {
	if (spieler.R12ChewyBork) {
		flags.AutoAniPlay = true;
		stop_person(P_CHEWY);
		det->hide_static_spr(8);
		start_ani_block(2, ablock17);
		start_aad_wait(32, -1);
		det->stop_detail(9);
		det->show_static_spr(8);
		auto_move(6, P_CHEWY);
		flags.AutoAniPlay = false;
	}
}

int16 r11_scanner() {
	int16 action_flag = false;
	if (!spieler.R12ChewyBork) {
		auto_move(7, P_CHEWY);
		if (!spieler.R11CardOk) {
			action_flag = true;
			start_aad_wait(13, -1);
		} else {
			if (is_cur_inventar(BORK_INV)) {
				action_flag = true;
				flc->set_custom_user_function(r12_cut_serv);
				start_aad(105, 0);
				flic_cut(FCUT_011, CFO_MODE);
				flc->remove_custom_user_function();
				spieler.R11TerminalOk = true;
				cur_2_inventory();
				menu_item = CUR_TALK;
				cursor_wahl(menu_item);
				start_aad_wait(12, -1);
				load_ads_dia(3);
			} else if (!spieler.inv_cur) {
				if (!spieler.R11TerminalOk) {
					action_flag = true;
					flic_cut(FCUT_010, CFO_MODE);
					start_aad_wait(20, -1);
				} else if (spieler.R11TerminalOk) {
					action_flag = true;
					start_aad_wait(12, -1);
					menu_item = CUR_TALK;
					cursor_wahl(menu_item);
					load_ads_dia(3);
				}
			}
		}
	}
	return (action_flag);
}

void r11_get_card() {
	if (spieler.R11CardOk) {
		spieler.R11CardOk = false;
		obj->add_inventar(spieler.R11IdCardNr, &room_blk);

		spieler.AkInvent = spieler.R11IdCardNr;
		cursor_wahl(CUR_AK_INVENT);
		cursor_wahl(CUR_AK_INVENT);
		det->stop_detail(0);
		atds->set_ats_str(83, TXT_MARK_LOOK, 0, ATS_DATEI);
		atds->set_ats_str(84, TXT_MARK_LOOK, 0, ATS_DATEI);
	}
}

void r11_put_card() {
	if (is_cur_inventar(RED_CARD_INV) || is_cur_inventar(YEL_CARD_INV)) {
		spieler.R11IdCardNr = spieler.AkInvent;
		del_inventar(spieler.R11IdCardNr);
		det->start_detail(0, 255, ANI_VOR);
		atds->set_ats_str(83, TXT_MARK_LOOK, 1, ATS_DATEI);
		atds->set_ats_str(84, TXT_MARK_LOOK, 1, ATS_DATEI);
		spieler.R11CardOk = true;
		if (!spieler.R11TerminalOk)
			start_aad_wait(16, -1);
	}
}

void r12_entry() {
	int16 i;
	zoom_horizont = 150;
	timer_nr[1] = room->set_timer(255, 20);
	if (!spieler.R12Betreten) {
		spieler.R12Betreten = true;
		for (i = 7; i < 10; i++)
			det->show_static_spr(i);
		flags.NoScroll = true;
		auto_scroll(60, 0);
		flic_cut(FCUT_016, CFO_MODE);
		flags.NoScroll = false;
		for (i = 7; i < 10; i++)
			det->hide_static_spr(i);
		obj->show_sib(SIB_TALISMAN_R12);
		obj->calc_rsi_flip_flop(SIB_TALISMAN_R12);
		obj->calc_all_static_detail();
		auto_move(5, P_CHEWY);
		start_aad_wait(109, -1);
	} else {
		if (spieler.R12Talisman == true && !spieler.R12BorkInRohr)
			timer_nr[0] = room->set_timer(255, 20);
		else if (spieler.R12BorkInRohr && !spieler.R12RaumOk)
			det->show_static_spr(12);

	}
}

#define R12_BORK_OBJ 0
int16 r12_bork_phasen[4][2] = {
	{ 74, 79 },
	{ 80, 85 },
	{ 62, 67 },
	{ 68, 73 }
};

MovLine r12_bork_mpkt[5] = {
	{ { 207, 220, 199 }, 2, 6 },
	{ { 207, 145, 199 }, 2, 6 },
	{ {  30, 145, 199 }, 0, 6 },
	{ { 207, 145, 199 }, 1, 6 },
	{ { 207, 220, 199 }, 3, 6 }
};

MovLine r12_bork_mpkt1[2] = {
	{ { 207, 220, 199 }, 0, 6 },
	{ { 170, 145, 199 }, 0, 6 }
};

MovLine r12_bork_mpkt2[3] = {
	{ { 170, 145, 199 }, 1, 8 },
	{ { 180, 145, 120 }, 1, 8 },
	{ { 300,  80, 120 }, 1, 8 }
};

void r12_init_bork() {
	if (!auto_obj_status(R12_BORK_OBJ) &&
	        !spieler.R12BorkTalk) {

		if (!auto_obj)
			det->load_taf_seq(62, (85 - 62) + 1, 0);
		if (!flags.AutoAniPlay && !flags.ChAutoMov) {
			auto_obj = 1;
			mov_phasen[R12_BORK_OBJ].AtsText = 120;
			mov_phasen[R12_BORK_OBJ].Lines = 5;
			mov_phasen[R12_BORK_OBJ].Repeat = 1;
			mov_phasen[R12_BORK_OBJ].ZoomFak = (int16)room->room_info->ZoomFak + 20;
			auto_mov_obj[R12_BORK_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[R12_BORK_OBJ].Delay = spieler.DelaySpeed;
			auto_mov_obj[R12_BORK_OBJ].Mode = 1;
			init_auto_obj(R12_BORK_OBJ, &r12_bork_phasen[0][0], mov_phasen[R12_BORK_OBJ].Lines, (MovLine
			              *)r12_bork_mpkt);
			if (!spieler.R12TalismanOk) {
				hide_cur();
				auto_mov_vector[R12_BORK_OBJ].DelayCount = 1000;
				auto_move(5, P_CHEWY);
				auto_mov_vector[R12_BORK_OBJ].DelayCount = 0;
				if (spieler.R12BorkCount < 3) {
					++spieler.R12BorkCount;
					uhr->reset_timer(timer_nr[0], 0);
					wait_show_screen(10);
					start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
					start_aad_wait(14, -1);
				}
				wait_auto_obj(R12_BORK_OBJ);
				show_cur();
			} else {
				r12_bork_ok();
			}
		}
		uhr->reset_timer(timer_nr[0], 0);
	}
}

void r12_talk_bork() {
	if (!spieler.R12TalismanOk) {

		start_aad_wait(28, -1);

	}
}

void r12_bork_ok() {
	hide_cur();
	flags.MausLinks = true;
	auto_mov_vector[R12_BORK_OBJ].DelayCount = 1000;
	auto_move(5, P_CHEWY);
	auto_mov_vector[R12_BORK_OBJ].DelayCount = 0;
	spieler.R12BorkTalk = true;
	mov_phasen[R12_BORK_OBJ].Repeat = 1;
	mov_phasen[R12_BORK_OBJ].Lines = 2;
	init_auto_obj(R12_BORK_OBJ, &r12_bork_phasen[0][0], mov_phasen[R12_BORK_OBJ].Lines, (MovLine
	              *)r12_bork_mpkt1);
	wait_auto_obj(R12_BORK_OBJ);
	spieler.R12BorkInRohr = true;
	det->set_detail_pos(3, 170, 145);
	det->start_detail(3, 255, ANI_VOR);
	start_aad_wait(57, -1);
	det->stop_detail(3);
	mov_phasen[R12_BORK_OBJ].Repeat = 1;
	mov_phasen[R12_BORK_OBJ].Lines = 3;
	init_auto_obj(R12_BORK_OBJ, &r12_bork_phasen[0][0], mov_phasen[R12_BORK_OBJ].Lines, (MovLine
	              *)r12_bork_mpkt2);
	wait_auto_obj(R12_BORK_OBJ);
	det->hide_static_spr(10);
	start_detail_wait(4, 1, ANI_VOR);
	talk_hide_static = -1;
	det->show_static_spr(12);
	atds->set_ats_str(118, TXT_MARK_LOOK, 2, ATS_DATEI);
	obj->calc_rsi_flip_flop(SIB_ROEHRE_R12);
	flags.MausLinks = false;
	show_cur();
}

int16 r12_use_terminal() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		if (!spieler.R12ChewyBork) {
			auto_move(6, P_CHEWY);
			start_aad_wait(110, -1);
			if (spieler.R12BorkInRohr && !spieler.R12RaumOk) {
				start_aad_wait(112, -1);
				flags.NoScroll = true;
				auto_scroll(46, 0);
				flic_cut(FCUT_017, CFO_MODE);

				load_chewy_taf(CHEWY_BORK);
				flags.NoScroll = false;
				atds->set_ats_str(118, 0, ATS_DATEI);
				det->hide_static_spr(12);
				menu_item = CUR_WALK;
				cursor_wahl(menu_item);
				set_person_pos(108, 90, P_CHEWY, -1);
				spieler.R12ChewyBork = true;
				spieler.R12RaumOk = true;
				auto_move(4, P_CHEWY);
				start_aad_wait(113, 0);
			} else if (spieler.R12TalismanOk && !spieler.R12RaumOk) {
				spieler.R12TalismanOk = false;
				spieler.R12KetteLinks = true;
				uhr->disable_timer();
				obj->calc_rsi_flip_flop(SIB_L_ROEHRE_R12);
				obj->calc_rsi_flip_flop(SIB_ROEHRE_R12);
				obj->calc_all_static_detail();
				atds->set_ats_str(118, TXT_MARK_LOOK, 0, ATS_DATEI);
				atds->set_ats_str(117, TXT_MARK_LOOK, 1, ATS_DATEI);
				start_aad(111, 0);
			} else {
				spieler.R12TransOn = true;
				uhr->reset_timer(timer_nr[1], 0);
			}
		}
		else
			start_aad(114, 0);
	}
	return (action_flag);
}

int16 r12_use_linke_rohr() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		if (!spieler.R12KetteLinks) {
			auto_move(7, P_CHEWY);
			start_aad_wait(29, -1);
		} else {
			spieler.R12KetteLinks = false;
			uhr->enable_timer();
			atds->set_ats_str(117, TXT_MARK_LOOK, 0, ATS_DATEI);
		}
	}
	return (action_flag);
}

int16 r12_chewy_trans() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (spieler.R12TransOn) {
			action_flag = true;
			flags.AutoAniPlay = true;
			auto_move(9, P_CHEWY);
			spieler.PersonHide[P_CHEWY] = true;
			start_ani_block(2, ablock16);
			set_person_pos(108, 82, P_CHEWY, P_RIGHT);
			spieler.PersonHide[P_CHEWY] = false;
			spieler.R12TransOn = false;
			flags.AutoAniPlay = false;
		}
	}
	return (action_flag);
}

void r13_entry() {
	if (!spieler.R12ChewyBork && !spieler.R13BorkOk) {
		out->cls();
		flic_cut(FCUT_013, CFO_MODE);
		set_person_pos(106, 65, P_CHEWY, P_RIGHT);
		switch_room(11);
		start_aad_wait(27, -1);
	} else {
		if (spieler.R13MonitorStatus)
			det->show_static_spr(11 - spieler.R13MonitorStatus);

		if (!spieler.R13BorkOk) {
			det->show_static_spr(12);
			obj->hide_sib(SIB_BANDKNOPF_R13);
			spieler.R13Bandlauf = 1;

			atds->set_ats_str(94, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(97, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(93, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);
		}
		if (spieler_vector[P_CHEWY].Xypos[0] > 290) {
			spieler.R13Band = true;
			atds->del_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
			spieler.room_e_obj[25].Attribut = 255;
		}
		if (spieler.R21GitterMuell)
			det->hide_static_spr(6);
	}
}

#define R13_BORK_OBJ 0
int16 r13_bork_phasen[4][2] = {
	{ 92, 97 },
	{ 86, 91 },
	{ 86, 91 },
	{ 68, 73 }
};

MovLine r13_bork_mpkt[3] = {
	{ { 168, 140, 162 }, 0, 6 },
	{ {  50, 140, 162 }, 0, 6 },
	{ {  50, 107, 130 }, 2, 6 }
};

void r13_talk_bork() {
	if (!spieler.R13BorkOk) {
		spieler.R13BorkOk = true;
		spieler.R12ChewyBork = false;
		det->show_static_spr(13);
		det->set_detail_pos(10, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
		det->
		set_static_pos(12, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1], 0, true)
		;
		spieler.PersonHide[P_CHEWY] = true;
		start_aad_wait(33, -1);
		det->stop_detail(9);
		det->load_taf_seq(86, (97 - 86) + 1, 0);
		spieler.PersonHide[P_CHEWY] = false;
		det->hide_static_spr(12);
		auto_obj = 1;
		mov_phasen[R13_BORK_OBJ].AtsText = 122;
		mov_phasen[R13_BORK_OBJ].Lines = 3;
		mov_phasen[R13_BORK_OBJ].Repeat = 1;
		mov_phasen[R13_BORK_OBJ].ZoomFak = 0;
		auto_mov_obj[R13_BORK_OBJ].Id = AUTO_OBJ0;
		auto_mov_vector[R13_BORK_OBJ].Delay = spieler.DelaySpeed;
		auto_mov_obj[R13_BORK_OBJ].Mode = 1;
		init_auto_obj(R13_BORK_OBJ, &r13_bork_phasen[0][0], mov_phasen[R13_BORK_OBJ].Lines, (MovLine *)r13_bork_mpkt);
		auto_move(9, P_CHEWY);
		wait_auto_obj(R13_BORK_OBJ);
		auto_move(11, P_CHEWY);
		flags.NoScroll = true;
		auto_scroll(41, 0);
		start_aad_wait(248, -1);
		flic_cut(FCUT_014, CFO_MODE);
		load_chewy_taf(CHEWY_NORMAL);
		auto_obj = 0;
		flags.NoScroll = false;
		atds->set_steuer_bit(122, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(92, ATS_AKTIV_BIT, ATS_DATEI);
		obj->show_sib(SIB_BANDKNOPF_R13);
		set_person_pos(153, 138, P_CHEWY, P_LEFT);

		start_aad_wait(34, -1);
		start_aad_wait(249, -1);

		obj->show_sib(SIB_TKNOPF1_R11);
		obj->show_sib(SIB_SCHLITZ_R11);
		obj->show_sib(SIB_TKNOPF2_R11);
		obj->show_sib(SIB_TKNOPF3_R11);
		if (spieler.R6DoorRightB)
			spieler.room_e_obj[20].Attribut = AUSGANG_LINKS;
		spieler.room_e_obj[21].Attribut = AUSGANG_OBEN;
	}
}

void r13_jmp_band() {
	if (!spieler.R13Band && !spieler.R12ChewyBork) {
		if (!spieler.R13Bandlauf) {
			obj->hide_sib(SIB_BANDKNOPF_R13);
			spieler.room_e_obj[25].Attribut = 255;
			atds->del_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
			spieler.R13Band = true;
			auto_move(3, P_CHEWY);
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_wait(8, 1, ANI_VOR);
			spieler.PersonHide[P_CHEWY] = false;
			set_person_pos(292, 98, P_CHEWY, P_RIGHT);
		} else {
			spieler.R13Surf = true;
			maus_links_click = false;
			auto_move(12, P_CHEWY);
			start_aad_wait(117, -1);
			flags.NoScroll = true;
			auto_scroll(76, 0);
			flic_cut(FCUT_015, CFO_MODE);
			flags.NoScroll = false;
			set_person_pos(195, 226, P_CHEWY, P_LEFT);

			spieler.R13Bandlauf = 0;

			atds->set_ats_str(94, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(97, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);

			atds->set_ats_str(93, TXT_MARK_LOOK, spieler.R13Bandlauf, ATS_DATEI);
			obj->calc_rsi_flip_flop(SIB_BANDKNOPF_R13);
			obj->hide_sib(SIB_BANDKNOPF_R13);
			switch_room(14);
			flic_cut(FCUT_018, CFO_MODE);
			spieler.scrollx = 92;
			spieler.scrolly = 120;
			spieler.PersonHide[P_CHEWY] = true;
			wait_show_screen(40);
			spieler.PersonHide[P_CHEWY] = false;
		}
	}
	else
		start_aad_wait(118, -1);
}

void r13_jmp_boden() {
	if (spieler.R13Band) {
		if (!spieler.R13Surf)
			obj->show_sib(SIB_BANDKNOPF_R13);
		spieler.room_e_obj[25].Attribut = AUSGANG_OBEN;
		atds->set_steuer_bit(100, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.R13Band = false;
		auto_move(5, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(7, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		set_person_pos(176, 138, P_CHEWY, P_LEFT);
	}
}

int16 r13_monitor_knopf() {
	int16 action_flag = false;
	if (!spieler.R13Band &&
	        !spieler.R12ChewyBork &&
	        !spieler.inv_cur) {
		action_flag = true;
		auto_move(8, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(6, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		if (spieler.R13MonitorStatus)
			det->hide_static_spr(11 - spieler.R13MonitorStatus);
		++spieler.R13MonitorStatus;
		if (spieler.R13MonitorStatus > 4)
			spieler.R13MonitorStatus = 0;
		else
			det->show_static_spr(11 - spieler.R13MonitorStatus);
		atds->set_ats_str(96, TXT_MARK_LOOK, spieler.R13MonitorStatus, ATS_DATEI);
	}
	return (action_flag);
}

void r14_entry() {
	spieler.R23GleiterExit = 14;
	zoom_horizont = 310;
	if (!flags.LoadGame) {
		if (spieler.R14GleiterAuf) {
			set_person_pos(381, 264, P_CHEWY, P_LEFT);
			det->show_static_spr(6);
			spieler.scrollx = 160;
			spieler.scrolly = 120;
		} else {
			spieler.scrollx = 92;
			spieler.scrolly = 17;
		}
	}
}

void r14_eremit_feuer(int16 t_nr, int16 ani_nr) {
	if (!flags.AutoAniPlay && !spieler.R14Feuer) {
		flags.AutoAniPlay = true;
		det->hide_static_spr(9);
		start_detail_wait(room->room_timer.ObjNr[ani_nr], 1, 0);
		uhr->reset_timer(t_nr, 0);
		det->show_static_spr(9);
		det->start_detail(7, 1, ANI_VOR);
		flags.AutoAniPlay = false;
	}
}

int16 r14_use_schrott() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		auto_move(3, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(12, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;

		if (!spieler.R14Waffe) {
			action_flag = true;
			spieler.R14Waffe = true;
			start_aad_wait(21, -1);
			invent_2_slot(BWAFFE_INV);
		}
	}
	return (action_flag);
}

int16 r14_use_gleiter() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		auto_move(4, P_CHEWY);
		if (!spieler.R14GleiterAuf) {
			spieler.R14GleiterAuf = true;
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_wait(10, 1, ANI_VOR);
			spieler.PersonHide[P_CHEWY] = false;
			det->show_static_spr(6);
			atds->set_ats_str(107, TXT_MARK_LOOK, 1, ATS_DATEI);
		} else {
			spieler.R23GleiterExit = 14;
			r23_cockpit();
		}
	}
	return (action_flag);
}

void r14_talk_eremit() {
	if (!spieler.R14Feuer) {
		auto_move(6, P_CHEWY);
		if (spieler.R14Translator) {
			flags.AutoAniPlay = true;
			load_ads_dia(0);
		} else {
			flags.AutoAniPlay = true;
			start_aad_wait(24, -1);
			flags.AutoAniPlay = false;
		}
	}
}

int16 r14_use_schleim() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		auto_move(2, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(11, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		if (!spieler.R14Sicherung) {
			action_flag = true;
			spieler.R14Sicherung = true;
			start_aad_wait(22, -1);
			invent_2_slot(SICHERUNG_INV);

		}
	}
	return (action_flag);
}

void r14_feuer() {
	int16 tmp;
	int16 waffe;
	waffe = false;
	tmp = spieler.AkInvent;
	spieler.R14Feuer = true;
	flags.AutoAniPlay = true;
	hide_cur();
	if (is_cur_inventar(BWAFFE_INV)) {
		auto_move(5, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_frame(8, 1, ANI_VOR, 11);
		start_detail_wait(9, 1, ANI_VOR);
		wait_detail(8);
		spieler.PersonHide[P_CHEWY] = false;
		waffe = true;
	} else {
		auto_move(7, P_CHEWY);
		det->hide_static_spr(9);
		start_detail_frame(2, 1, ANI_VOR, 9);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(13, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		wait_detail(2);
		start_detail_wait(5, 1, ANI_VOR);
		det->show_static_spr(9);
	}
	del_inventar(tmp);
	det->start_detail(6, 255, ANI_VOR);
	wait_show_screen(40);
	if (waffe)
		auto_move(7, P_CHEWY);
	start_aad_wait(26, -1);
	det->hide_static_spr(9);
	start_detail_wait(3, 1, ANI_VOR);
	det->show_static_spr(9);
	invent_2_slot(FLUXO_INV);
	atds->set_ats_str(105, TXT_MARK_LOOK, 1, ATS_DATEI);
	spieler.R14FluxoFlex = true;
	flags.AutoAniPlay = false;
	show_cur();
}

void r16_entry() {
	zoom_horizont = 140;
	if (!spieler.R16F5Exit) {
		det->show_static_spr(4);
		spieler.room_e_obj[32].Attribut = 255;
		atds->del_steuer_bit(124, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		det->hide_static_spr(4);
		spieler.room_e_obj[32].Attribut = AUSGANG_LINKS;
		atds->set_steuer_bit(124, ATS_AKTIV_BIT, ATS_DATEI);
	}
}

int16 r16_use_gleiter() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		auto_move(6, P_CHEWY);
		spieler.R23GleiterExit = 16;
		r23_cockpit();
	}
	return (action_flag);
}

void r17_entry() {
	r17_plot_seil();
	if (spieler.R17GitterWeg)
		det->hide_static_spr(5);
	if (spieler.R17DoorKommand)
		det->show_static_spr(7);
	if (spieler.R17Location == 1) {
		flags.ZoomMov = true;
		zoom_mov_fak = 3;
		room->set_zoom(25);
		zoom_horizont = 0;
		spieler.scrollx = 0;
		spieler.scrolly = 60;
		set_person_pos(242, 146, P_CHEWY, P_LEFT);
		r17_xit();
	} else if (spieler.R17Location == 3) {
		room->set_zoom(32);
		zoom_horizont = 399;
		r17_xit();
	} else {
		room->set_zoom(15);
		zoom_horizont = 0;
		spieler.room_e_obj[36].Attribut = 255;
		spieler.room_e_obj[38].Attribut = 255;
		if (spieler.R6DoorLeftF)
			spieler.room_e_obj[39].Attribut = AUSGANG_RECHTS;
		else
			spieler.room_e_obj[39].Attribut = 255;
		if (spieler.R18DoorBruecke)
			spieler.room_e_obj[35].Attribut = AUSGANG_LINKS;
		else
			spieler.room_e_obj[35].Attribut = 255;
	}
}

void r17_xit() {
	if (spieler.R17DoorKommand)
		spieler.room_e_obj[36].Attribut = AUSGANG_OBEN;
	else
		spieler.room_e_obj[36].Attribut = 255;
	spieler.room_e_obj[35].Attribut = 255;
	spieler.room_e_obj[39].Attribut = 255;
	spieler.room_e_obj[38].Attribut = AUSGANG_OBEN;
}

int16 r17_use_seil() {
	int16 action_flag = false;
	if (!flags.AutoAniPlay) {
		if (spieler.R17Location == 1) {
			if (is_cur_inventar(SEIL_INV)) {
				action_flag = true;
				del_inventar(spieler.AkInvent);
				flags.AutoAniPlay = true;
				auto_move(5, P_CHEWY);
				spieler.PersonHide[P_CHEWY] = true;
				start_detail_wait(10, 1, ANI_VOR);
				spieler.R17Seil = true;
				atds->del_steuer_bit(139, ATS_AKTIV_BIT, ATS_DATEI);
				r17_plot_seil();
				spieler.PersonHide[P_CHEWY] = false;
				flags.AutoAniPlay = false;
				start_aad(119, -1);
			}
		}
	}
	return (action_flag);
}

void r17_plot_seil() {
	int16 i;
	if (spieler.R17Seil) {
		for (i = 0; i < 3; i++)
			det->show_static_spr(8 + i);
	}
}

#define R17_CHEWY_OBJ 0
int16 r17_chewy_phasen[4][2] = {
	{ 0, 0 },
	{ 0, 0 },
	{ 141, 144 },
	{ 177, 177 }
};

MovLine r17_chewy_mpkt[2] = {
	{ { 241, 150, 0 }, 3, 8 },
	{ { 241, 350, 0 }, 3, 8 }
};

MovLine r17_chewy_mpkt1[2] = {
	{ { 243, 334, 0 }, 2, 6 },
	{ { 243, 150, 0 }, 2, 6 }
};

void r17_kletter_down() {
	auto_move(5, P_CHEWY);
	det->load_taf_seq(177, 1, 0);
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(14, 1, ANI_VOR);
	flags.ZoomMov = false;
	zoom_mov_fak = 1;
	spieler.ScrollyStep = 2;
	room->set_zoom(32);
	zoom_horizont = 399;
	auto_obj = 1;
	init_auto_obj(R17_CHEWY_OBJ, &r17_chewy_phasen[0][0], mov_phasen[R17_CHEWY_OBJ].Lines, (MovLine
	              *)r17_chewy_mpkt);
	set_person_pos(242, 350, P_CHEWY, P_LEFT);
}

void r17_kletter_up() {
	auto_move(6, P_CHEWY);
	det->load_taf_seq(141, 4, 0);
	spieler.PersonHide[P_CHEWY] = true;
	start_detail_wait(11, 1, ANI_VOR);
	flags.ZoomMov = true;
	zoom_mov_fak = 3;
	spieler.ScrollyStep = 1;
	room->set_zoom(25);
	zoom_horizont = 0;
	auto_obj = 1;
	init_auto_obj(R17_CHEWY_OBJ, &r17_chewy_phasen[0][0], mov_phasen[R17_CHEWY_OBJ].Lines, (MovLine
	              *)r17_chewy_mpkt1);
	set_person_pos(243, 146, P_CHEWY, P_LEFT);
}

void r17_calc_seil() {
	if (spieler.R17Seil) {
		if (spieler.R17Location != 2) {
			if (!flags.AutoAniPlay) {
				if (!spieler.inv_cur) {
					r17_close_door();
					flags.AutoAniPlay = true;
					mov_phasen[R17_CHEWY_OBJ].AtsText = 0;
					mov_phasen[R17_CHEWY_OBJ].Lines = 2;
					mov_phasen[R17_CHEWY_OBJ].Repeat = 1;
					mov_phasen[R17_CHEWY_OBJ].ZoomFak = 0;
					auto_mov_obj[R17_CHEWY_OBJ].Id = AUTO_OBJ0;
					auto_mov_vector[R17_CHEWY_OBJ].Delay = spieler.DelaySpeed;
					auto_mov_obj[R17_CHEWY_OBJ].Mode = 1;
					if (spieler.R17Location == 1) {
						r17_kletter_down();
						spieler.R17Location = 3;
					}
					else if (spieler.R17Location == 3) {
						r17_kletter_up();
						spieler.R17Location = 1;
					}
					menu_item = CUR_WALK;
					cursor_wahl(menu_item);
					wait_auto_obj(R17_CHEWY_OBJ);
					set_person_spr(P_LEFT, P_CHEWY);
					spieler.ScrollyStep = 1;
					spieler.PersonHide[P_CHEWY] = false;
					flags.AutoAniPlay = false;
					auto_obj = 0;
					r17_xit();
				}
			}
		}
	}
}

void r17_door_kommando(int16 mode) {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!mode) {
			if (!spieler.R17DoorKommand) {
				spieler.room_e_obj[36].Attribut = AUSGANG_OBEN;
				spieler.R17DoorKommand = true;
				start_detail_wait(4, 1, ANI_VOR);
				stop_person(P_CHEWY);
				det->show_static_spr(7);
			}
		} else {
			r17_close_door();
		}
		flags.AutoAniPlay = false;
		atds->set_ats_str(144, spieler.R17DoorKommand, ATS_DATEI);
	}
}

void r17_close_door() {
	if (spieler.R17DoorKommand) {
		spieler.room_e_obj[36].Attribut = 255;
		spieler.R17DoorKommand = false;
		det->hide_static_spr(7);
		det->start_detail(4, 1, ANI_RUECK);
	}
}

int16 r17_energie_hebel() {
	int16 dia_nr;
	int16 action_flag = false;
	auto_move(7, P_CHEWY);
	if (!spieler.R17HebelOk) {
		action_flag = true;
		if (is_cur_inventar(BECHER_VOLL_INV)) {
			spieler.R17HebelOk = true;
			dia_nr = 38;
		} else
			dia_nr = 37;
		start_aad_wait(dia_nr, -1);
	}
	else if (!spieler.inv_cur) {
		action_flag = true;
		obj->calc_rsi_flip_flop(SIB_HEBEL_R17);
		spieler.R17EnergieOut ^= 1;
		atds->set_ats_str(142, TXT_MARK_LOOK, spieler.R17EnergieOut, ATS_DATEI);
		det->play_sound(12, 0);
		if (spieler.R17EnergieOut)
			det->disable_sound(15, 0);
		else {
			det->enable_sound(15, 0);
			det->play_sound(15, 0);
		}
	}
	return (action_flag);
}

int16 r17_get_oel() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		auto_move(4, P_CHEWY);
		start_spz(CH_EKEL, 3, ANI_VOR, P_CHEWY);
		start_aad_wait(60, -1);
	} else if (is_cur_inventar(BECHER_LEER_INV)) {
		action_flag = true;
		r17_close_door();
		auto_move(4, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(13, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		del_inventar(spieler.AkInvent);
		obj->add_inventar(BECHER_VOLL_INV, &room_blk);
		inventory_2_cur(BECHER_VOLL_INV);
	}

	return (action_flag);
}

int16 bork_spr [5] = {15, 16, 17, 24, 25};
int16 bork_spr1[4] = {20, 21, 18, 19};

void r18_entry() {
	int16 i;
	spieler.R18MoniSwitch = 0;
	spieler.R18CartTerminal = 0;
	atds->set_ats_str(151, TXT_MARK_LOOK, 0, ATS_DATEI);
	spieler.ScrollxStep = 2;
	if (spieler.R18CartTerminal)
		det->show_static_spr(23);
	if (!spieler.R18SurimyWurf)
		r18_init_borks();
	else {
		for (i = 0; i < 5; i++)
			det->hide_static_spr(bork_spr[i]);
		for (i = 0; i < (4 - spieler.R18Krone); i++)
			det->show_static_spr(bork_spr1[i]);
	}
	if (spieler.R16F5Exit)
		det->hide_static_spr(19);
	if (spieler.R17EnergieOut) {
		det->stop_detail(0);
		atds->set_ats_str(150, TXT_MARK_LOOK, 1, ATS_DATEI);
	} else {
		atds->set_ats_str(150, TXT_MARK_LOOK, 0, ATS_DATEI);
	}
	if (!spieler.R18FirstEntry && !spieler.R18Gitter) {
		start_aad_wait(39, -1);
		spieler.R18FirstEntry = true;
	}
}

void r18_init_borks() {
	int16 i;
	for (i = 0; i < 5; i++)
		det->show_static_spr(bork_spr[i]);
	for (i = 0; i < 4; i++)
		det->hide_static_spr(bork_spr1[i]);
	timer_nr[0] = room->set_timer(255, 10);
	timer_nr[1] = room->set_timer(255, 15);
	spieler.scrollx = 276;
	spieler.scrolly = 0;
	flags.NoScroll = true;
}

void r18_timer_action(int16 t_nr) {

	if (!spieler.R18SurimyWurf) {
		if (!flags.AutoAniPlay) {
			flags.AutoAniPlay = true;
			if (t_nr == timer_nr[0]) {
				det->hide_static_spr(16);
				start_detail_wait(10, 1, ANI_VOR);
				det->show_static_spr(16);
				uhr->reset_timer(timer_nr[0], 10);
			}
			else if (t_nr == timer_nr[1]) {
				hide_cur();
				det->hide_static_spr(17);
				start_detail_wait(9, 1, ANI_VOR);
				det->show_static_spr(17);
				if (!spieler.R18SondeMoni) {
					spieler.R18SondeMoni = true;
					start_detail_wait(2, 1, ANI_VOR);
					det->show_static_spr(9);
					start_detail_wait(4, 1, ANI_VOR);
					det->show_static_spr(11);
					wait_show_screen(50);
					det->hide_static_spr(9);
					det->hide_static_spr(11);
				} else {
					r18_monitor();
				}
				show_cur();
				uhr->reset_timer(timer_nr[1], 15);
			}
			flags.AutoAniPlay = false;
		}
	}
}

void r18_monitor() {
	int16 nr;
	nr = 0;
	spieler.R18MoniSwitch ^= 1;
	if (spieler.R18MoniSwitch) {
		start_ani_block(2, ablock21);
		if (spieler.R17EnergieOut)
			nr = 2;
		else
			nr = 1;
	} else {
		det->stop_detail(23);
		atds->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATEI);
	}
	atds->set_ats_str(151, TXT_MARK_LOOK, nr, ATS_DATEI);
}

int16 r18_sonden_moni() {
	int16 i;
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		hide_cur();
		auto_move(8, P_CHEWY);
		start_detail_wait(3, 1, ANI_VOR);
		det->show_static_spr(10);
		start_ani_block(3, ablock20);
		for (i = 0; i < 3; i++)
			det->show_static_spr(i + 12);
		wait_show_screen(40);
		for (i = 0; i < 5; i++)
			det->hide_static_spr(i + 10);
		show_cur();
	}
	return (action_flag);
}

int16 r18_surimy_phasen[4][2] = {
	{ 245, 252 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

MovLine r18_surimy_mpkt[2] = {
	{ { 453, 170, 190 }, 0, 6 },
	{ { 392, 170, 190 }, 0, 6 }
};

MovLine r18_surimy_mpkt1[2] = {
	{ { 392, 170, 190 }, 0, 6 },
	{ { 143, 170, 190 }, 0, 6 }
};

MovLine r18_surimy_mpkt2[2] = {
	{ { 143, 170, 190 }, 0, 6 },
	{ {   0, 170, 190 }, 0, 6 }
};

MovLine r18_surimy_mpkt3[2] = {
	{ { 500, 100, 190 }, 0, 12 },
	{ { 392, 170, 190 }, 0, 12 }
};

int16 r18_calc_surimy() {
	int16 i;
	int16 action_flag = false;
	if (is_cur_inventar(SURIMY_INV)) {
		action_flag = true;
		hide_cur();
		del_inventar(spieler.AkInvent);
		spieler.R18SurimyWurf = true;
		det->load_taf_seq(245, 294 - 245 + 1, 0);
		det->load_taf_seq(116, 170 - 116 + 1, 0);
		auto_obj = 1;
		mov_phasen[SURIMY_OBJ].AtsText = 0;
		mov_phasen[SURIMY_OBJ].Lines = 2;
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		mov_phasen[SURIMY_OBJ].ZoomFak = 0;
		auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
		auto_mov_vector[SURIMY_OBJ].Delay = spieler.DelaySpeed;
		auto_mov_obj[SURIMY_OBJ].Mode = 1;
		if (spieler_vector[P_CHEWY].Xypos[1] < 150) {
			start_detail_frame(18, 1, ANI_VOR, 8);

			init_auto_obj(SURIMY_OBJ, &r18_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
			              *)r18_surimy_mpkt3);
			wait_detail(18);
		} else {
			auto_move(1, P_CHEWY);
			stop_person(P_CHEWY);
			spieler.PersonHide[P_CHEWY] = true;
			start_detail_frame(17, 1, ANI_VOR, 12);
			maus_links_click = false;

			init_auto_obj(SURIMY_OBJ, &r18_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
			              *)r18_surimy_mpkt);
			wait_detail(17);
		}
		spieler.PersonHide[P_CHEWY] = false;
		wait_auto_obj(SURIMY_OBJ);

		det->set_detail_pos(21, 392, 170);
		det->set_detail_pos(22, 447, 154);
		start_detail_frame(21, 1, ANI_VOR, 14);
		det->start_detail(22, 1, ANI_VOR);
		wait_detail(21);
		det->set_static_pos(26, 392, 170, false, true);
		det->show_static_spr(26);

		det->hide_static_spr(24);
		start_ani_block(2, ablock22);
		det->show_static_spr(20);

		for (i = 0; i < 3; i++)
			det->hide_static_spr(i + 15);
		start_ani_block(2, ablock23);
		det->show_static_spr(18);
		det->show_static_spr(19);

		det->hide_static_spr(26);
		flags.NoScroll = true;
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &r18_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
		              *)r18_surimy_mpkt1);
		auto_scroll(70, 0);
		wait_auto_obj(SURIMY_OBJ);

		det->set_detail_pos(21, 143, 170);
		det->set_detail_pos(22, 198, 154);
		start_detail_frame(21, 1, ANI_VOR, 14);
		det->start_detail(22, 1, ANI_VOR);
		wait_detail(21);
		det->set_static_pos(26, 143, 170, false, true);
		det->show_static_spr(26);

		det->hide_static_spr(25);
		start_ani_block(2, ablock24);
		det->show_static_spr(21);

		det->hide_static_spr(26);
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &r18_surimy_phasen[0][0], mov_phasen[SURIMY_OBJ].Lines, (MovLine
		              *)r18_surimy_mpkt2);
		auto_scroll(0, 0);
		wait_auto_obj(SURIMY_OBJ);
		spieler.ScrollxStep = 6;
		flags.NoScroll = false;
		auto_scroll(318, 0);
		spieler.ScrollxStep = 2;
		auto_obj = 0;

		atds->set_ats_str(153, 1, ATS_DATEI);

		atds->set_ats_str(149, TXT_MARK_LOOK, 1, ATS_DATEI);
		for (i = 0; i < 3; i++)
			atds->del_steuer_bit(158 + i, ATS_AKTIV_BIT, ATS_DATEI);

		show_cur();
	}
	return (action_flag);
}

int16 r18_calc_schalter() {
	int16 action_flag = false;
	if (!spieler.inv_cur && !spieler.R18Gitter) {
		action_flag = true;
		auto_move(6, P_CHEWY);

		r18_monitor();
	}
	return (action_flag);
}

short r18_use_cart_moni() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		auto_move(9, P_CHEWY);
		set_person_spr(P_LEFT, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		spieler.R18CartTerminal ^= 1;
		if (!spieler.R18CartTerminal) {
			atds->set_ats_str(157, TXT_MARK_LOOK, 0, ATS_DATEI);

			atds->set_ats_str(147, TXT_MARK_LOOK, 0, ATS_DATEI);
			det->hide_static_spr(23);
			start_detail_wait(20, 1, ANI_RUECK);
		} else {
			atds->set_ats_str(157, TXT_MARK_LOOK, 1, ATS_DATEI);
			atds->set_ats_str(147, TXT_MARK_LOOK, 1, ATS_DATEI);
			start_detail_wait(20, 1, ANI_VOR);
			det->show_static_spr(23);
			if (spieler.R18CartFach) {
				spieler.R18CartSave = true;
				atds->set_ats_str(CARTRIDGE_INV, TXT_MARK_LOOK, 1, INV_ATS_DATEI);
				start_aad_wait(120, -1);
			}
		}
	}
	return (action_flag);
}

int16 r18_go_cyberspace() {
	int16 action_flag = false;
	if (!spieler.inv_cur && !spieler.R18Gitter) {
		action_flag = true;
		auto_move(7, P_CHEWY);
		cur_2_inventory();
		switch_room(24);
	}
	return (action_flag);
}

void r19_entry() {
	zoom_horizont = 250;
	flags.NoScroll = true;
	spieler.scrollx = 0;
	spieler.scrolly = 24;
}

void r21_entry() {
	load_chewy_taf(CHEWY_MINI);
	r21_calc_laser();
	r21_init_spinne();
	timer_nr[2] = room->set_timer(255, 1);
	flags.NoEndPosMovObj = true;
	SetUpScreenFunc = r21setup_func;
}

void r21_calc_laser() {

	if (spieler.R21Hebel1 && !spieler.R21Hebel2 && spieler.R21Hebel3) {
		spieler.R21Laser1Weg = true;
		det->stop_detail(3);
		atds->set_steuer_bit(134, ATS_AKTIV_BIT, ATS_DATEI);
		atds->del_steuer_bit(133, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		spieler.R21Laser1Weg = false;
		det->start_detail(3, 255, ANI_VOR);
		atds->del_steuer_bit(134, ATS_AKTIV_BIT, ATS_DATEI);
		atds->set_steuer_bit(133, ATS_AKTIV_BIT, ATS_DATEI);
	}

	if (!spieler.R21Hebel1 && spieler.R21Hebel2 && !spieler.R21Hebel3) {
		if (obj->check_inventar(SEIL_INV) == false) {
			obj->show_sib(SIB_SEIL_R21);
			atds->del_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		}
		spieler.R21Laser2Weg = true;
		det->stop_detail(4);
		atds->set_steuer_bit(135, ATS_AKTIV_BIT, ATS_DATEI);
	} else {
		obj->hide_sib(SIB_SEIL_R21);
		atds->set_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		spieler.R21Laser2Weg = false;
		det->start_detail(4, 255, ANI_VOR);
		atds->del_steuer_bit(135, ATS_AKTIV_BIT, ATS_DATEI);
	}
}

#define R21_SPINNE1_OBJ 0
#define R21_SPINNE2_OBJ 1
#define R21_ENERGIE_OBJ 2
int16 r21_spinne_phasen[4][2] = {
	{ 58, 65 },
	{ 50, 57 },
	{ 42, 49 },
	{ 66, 89 }
};

MovLine r21_spinne_mpkt[3] = {
	{ { 550, 275, 0 }, 0, 2 },
	{ { 396, 275, 0 }, 0, 2 },
	{ { 530, 275, 0 }, 1, 2 }
};

MovLine r21_spinne_mpkt1[2] = {
	{ { 104,  50, 0 }, 3, 3 },
	{ { 104, 430, 0 }, 3, 3 }
};

MovLine r21_energie_mpkt[2] = {
	{ { 115, 140, 0 }, 2, 1 },
	{ { 115, 190, 0 }, 2, 1 }
};

void r21_init_spinne() {
	det->load_taf_seq(42, (89 - 42) + 1, 0);
	auto_obj = 2;

	mov_phasen[R21_SPINNE1_OBJ].AtsText = 130;
	mov_phasen[R21_SPINNE1_OBJ].Lines = 3;
	mov_phasen[R21_SPINNE1_OBJ].Repeat = 255;
	mov_phasen[R21_SPINNE1_OBJ].ZoomFak = 0;
	auto_mov_obj[R21_SPINNE1_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[R21_SPINNE1_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[R21_SPINNE1_OBJ].Mode = 1;
	init_auto_obj(R21_SPINNE1_OBJ, &r21_spinne_phasen[0][0], mov_phasen[R21_SPINNE1_OBJ].Lines, (MovLine *)r21_spinne_mpkt);

	mov_phasen[R21_SPINNE2_OBJ].AtsText = 130;
	mov_phasen[R21_SPINNE2_OBJ].Lines = 2;
	mov_phasen[R21_SPINNE2_OBJ].Repeat = 1;
	mov_phasen[R21_SPINNE2_OBJ].ZoomFak = 0;
	auto_mov_obj[R21_SPINNE2_OBJ].Id = AUTO_OBJ1;
	auto_mov_vector[R21_SPINNE2_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[R21_SPINNE2_OBJ].Mode = 1;
	init_auto_obj(R21_SPINNE2_OBJ, &r21_spinne_phasen[0][0], mov_phasen[R21_SPINNE2_OBJ].Lines, (MovLine *)r21_spinne_mpkt1);
	timer_nr[0] = room->set_timer(255, 21);
	e_streifen = false;

}

void r21_restart_spinne2() {
	mov_phasen[R21_SPINNE2_OBJ].Repeat = 1;
	init_auto_obj(R21_SPINNE2_OBJ, &r21_spinne_phasen[0][0], mov_phasen[R21_SPINNE2_OBJ].Lines, (MovLine *)r21_spinne_mpkt1);
	uhr->reset_timer(timer_nr[0], 0);
	e_streifen = false;

}

void r21_e_streifen() {
	auto_obj = 3;
	mov_phasen[R21_ENERGIE_OBJ].AtsText = 0;
	mov_phasen[R21_ENERGIE_OBJ].Lines = 2;
	mov_phasen[R21_ENERGIE_OBJ].Repeat = 1;
	mov_phasen[R21_ENERGIE_OBJ].ZoomFak = 0;
	auto_mov_obj[R21_ENERGIE_OBJ].Id = AUTO_OBJ2;
	auto_mov_vector[R21_ENERGIE_OBJ].Delay = spieler.DelaySpeed;
	auto_mov_obj[R21_ENERGIE_OBJ].Mode = 1;
	init_auto_obj(R21_ENERGIE_OBJ, &r21_spinne_phasen[0][0], mov_phasen[R21_ENERGIE_OBJ].Lines, (MovLine *)r21_energie_mpkt);

}

void r21setup_func() {
	if (auto_mov_vector[R21_SPINNE2_OBJ].Xypos[1] >= 190) {
		if (!e_streifen) {
			e_streifen = true;
			r21_e_streifen();
		}
	}
}

void r21_chewy_kolli() {
	int16 spr_nr;
	int16 *Cxy;
	int16 xoff;
	int16 yoff;
	int16 *xy;
	int16 kolli;
	int16 i;
	int16 ani_nr;
	int16 tmp;
	kolli = false;

	for (i = 0; i < 3 && !kolli; i++) {
		spr_nr = mov_phasen[i].Phase[auto_mov_vector[i].Phase][0] +
		         auto_mov_vector[i].PhNr;
		xy = (int16 *)room_blk.DetImage[spr_nr];
		Cxy = room_blk.DetKorrekt + (spr_nr << 1);
		xoff = xy[0];
		yoff = xy[1];
		if (i == 2)
			xoff += 10;
		xoff += auto_mov_vector[i].Xzoom;
		yoff += auto_mov_vector[i].Yzoom;
		if ((!i && spieler_vector[P_CHEWY].Xypos[0] < 516) ||
		        (i == 1 && spieler_vector[P_CHEWY].Xypos[1] > 70) ||
		        (i == 2)) {
			if (spieler_vector[P_CHEWY].Xypos[0] + 12 >= auto_mov_vector[i].Xypos[0] + Cxy[0] &&
			        spieler_vector[P_CHEWY].Xypos[0] + 12 <= auto_mov_vector[i].Xypos[0] + xoff + Cxy[0] &&
			        spieler_vector[P_CHEWY].Xypos[1] + 12 >= auto_mov_vector[i].Xypos[1] + Cxy[1] &&
			        spieler_vector[P_CHEWY].Xypos[1] + 12 <= auto_mov_vector[i].Xypos[1] + yoff + Cxy[1] &&
			        mov_phasen[i].Start == 1)
				kolli = true;
		}
	}
	if (kolli) {
		if (!flags.AutoAniPlay) {
			tmp = spieler_vector[P_CHEWY].Count;
			stop_person(P_CHEWY);
			flags.AutoAniPlay = true;
			spieler.PersonHide[P_CHEWY] = true;
			if (spieler_vector[P_CHEWY].Xyvo[0] < 0)
				ani_nr = 10;
			else
				ani_nr = 11;
			det->
			set_detail_pos(ani_nr, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
			start_detail_wait(ani_nr, 1, ANI_VOR);
			spieler.PersonHide[P_CHEWY] = false;
			flags.AutoAniPlay = false;
			spieler_vector[P_CHEWY].Count = tmp;
			get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
			mov->continue_auto_go();
		}
	}
}

void r21_salto() {
	int16 i;
	if (!spieler.inv_cur) {
		if (atds->get_ats_str(134, TXT_MARK_USE, ATS_DATEI) == 8) {
			if (!spieler.R21Salto) {
				if (!flags.AutoAniPlay) {
					spieler.R21Salto = true;
					flags.AutoAniPlay = true;
					spieler.PersonHide[P_CHEWY] = true;
					for (i = 0; i < 3; i++)
						det->
						set_detail_pos(12 + i, spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]
						              );
					start_ani_block(3, ablock19);
					spieler.PersonHide[P_CHEWY] = false;
					start_aad_wait(36, -1);
					flags.AutoAniPlay = false;
				}
			}
		}
	}
}

void r21_use_gitter_energie() {
	spieler.R21GitterEnergie = exit_flip_flop(-1, 47, -1, 131, 138, -1,
	                           AUSGANG_UNTEN, AUSGANG_OBEN,
	                           (int16)spieler.R21GitterEnergie);
	auto_obj = 0;
	spieler.R17Location = 1;
	spieler.PersonHide[P_CHEWY] = true;
	switch_room(17);
	det->hide_static_spr(5);
	start_detail_wait(9, 1, ANI_VOR);
	spieler.R17GitterWeg = true;
	spieler.PersonHide[P_CHEWY] = false;
}

int16 r21_use_fenster() {
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		if (!flags.AutoAniPlay && spieler.R21Laser1Weg) {
			action_flag = true;
			flags.AutoAniPlay = true;
			spieler.R18Gitter = true;
			auto_move(13, P_CHEWY);
			set_person_pos(541, 66, P_CHEWY, P_LEFT);
			switch_room(18);
			if (!spieler.R18FirstEntry) {
				start_aad_wait(39, -1);
				spieler.R18FirstEntry = true;
			}
			spieler.room_e_obj[50].Attribut = AUSGANG_OBEN;
			spieler.room_e_obj[41].Attribut = 255;
			flags.AutoAniPlay = false;
		}
	}
	return (action_flag);
}

void r22_entry() {
	if (!spieler.R22BorkPlatt) {
		det->load_taf_seq(36, (56 - 36) + 1, 0);
		room->set_timer(255, 15);
	} else if (spieler.R22ChewyPlatt && !spieler.R22GetBork)
		det->show_static_spr(4);
}

int16 r22_chewy_amboss() {
	int16 action_flag = false;
	if (!spieler.R22ChewyPlatt && !spieler.inv_cur && !flags.AutoAniPlay) {
		action_flag = true;
		flags.AutoAniPlay = true;
		auto_move(5, P_CHEWY);
		spieler.PersonHide[P_CHEWY] = true;
		start_detail_wait(1, 1, ANI_VOR);
		spieler.PersonHide[P_CHEWY] = false;
		auto_move(2, P_CHEWY);
		flags.NoPalAfterFlc = false;
		flic_cut(FCUT_007, CFO_MODE);
		spieler.R22ChewyPlatt = true;
		atds->set_ats_str(79, 1, ATS_DATEI);
		flags.AutoAniPlay = false;
	}
	return (action_flag);
}

#define R22_BORK_OBJ 0
int16 r22_bork_phasen[4][2] = {
	{ 51, 56 },
	{ 36, 41 },
	{ 36, 41 },
	{ 36, 41 }
};

MovLine r22_bork_mpkt[2] = {
	{ {-30, 125, 170} , 1, 6 },
	{ { 155, 125, 170 }, 1, 6 },
};

MovLine r22_bork_mpkt1[2] = {
	{ { 155, 125, 170}, 2, 6 },
	{ { -30, 125, 170}, 0, 6 }
};

MovLine r22_bork_mpkt2[2] = {
	{ { -30, 125, 170 }, 1, 6 },
	{ {  90, 125, 170 }, 1, 6 }
};

void r22_bork(int16 t_nr) {
	if (!flags.AutoAniPlay) {
		flags.AutoAniPlay = true;
		if (!spieler.R22BorkPlatt) {
			hide_cur();
			start_spz(CH_TALK2, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(10, -1);
			auto_move(3, P_CHEWY);
			auto_obj = 1;
			mov_phasen[R22_BORK_OBJ].AtsText = 0;
			mov_phasen[R22_BORK_OBJ].Lines = 2;
			mov_phasen[R22_BORK_OBJ].Repeat = 1;
			mov_phasen[R22_BORK_OBJ].ZoomFak = 0;
			auto_mov_obj[R22_BORK_OBJ].Id = AUTO_OBJ0;
			auto_mov_vector[R22_BORK_OBJ].Delay = spieler.DelaySpeed;
			auto_mov_obj[R22_BORK_OBJ].Mode = 1;
			if (!spieler.R22Paint) {
				r22_bork_walk1();
			} else {
				spieler.R22ChewyPlatt = true;
				r22_bork_walk2();
			}
			show_cur();
		}
		uhr->reset_timer(t_nr, 0);
		flags.AutoAniPlay = false;
	}
}

void r22_bork_walk1() {

	init_auto_obj(R22_BORK_OBJ, &r22_bork_phasen[0][0], mov_phasen[R22_BORK_OBJ].Lines, (MovLine
	              *)r22_bork_mpkt);
	wait_auto_obj(R22_BORK_OBJ);

	start_detail_wait(2, 1, ANI_VOR);

	mov_phasen[R22_BORK_OBJ ].Repeat = 1;
	init_auto_obj(R22_BORK_OBJ, &r22_bork_phasen[0][0], mov_phasen[R22_BORK_OBJ].Lines, (MovLine
	              *)r22_bork_mpkt1);
	wait_auto_obj(R22_BORK_OBJ);
}

void r22_bork_walk2() {

	init_auto_obj(R22_BORK_OBJ, &r22_bork_phasen[0][0], mov_phasen[R22_BORK_OBJ].Lines, (MovLine
	              *)r22_bork_mpkt2);
	wait_auto_obj(R22_BORK_OBJ);

	flic_cut(FCUT_009, CFO_MODE);
	det->show_static_spr(4);
	atds->del_steuer_bit(81, ATS_AKTIV_BIT, ATS_DATEI);
	spieler.R22BorkPlatt = true;
	atds->set_steuer_bit(79, ATS_AKTIV_BIT, ATS_DATEI);
}

void r22_get_bork() {
	if (!spieler.R22GetBork && spieler.R22BorkPlatt) {
		auto_move(4, P_CHEWY);
		det->hide_static_spr(4);
		spieler.PersonHide[P_CHEWY] = true;
		start_ani_block(2, ablock14);
		set_person_pos(171, 120, P_CHEWY, P_LEFT);
		start_aad_wait(11, -1);
		det->stop_detail(3);
		spieler.PersonHide[P_CHEWY] = false;
		atds->set_steuer_bit(81, ATS_AKTIV_BIT, ATS_DATEI);
		invent_2_slot(BORK_INV);

		spieler.R22GetBork = true;
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
	}
}

int16 r22_malen() {
	int16 action_flag = false;
	if (!flags.AutoAniPlay && is_cur_inventar(17)) {
		action_flag = true;
		flags.AutoAniPlay = true;
		auto_move(8, P_CHEWY);
		flic_cut(FCUT_008, CFO_MODE);
		atds->set_ats_str(82, TXT_MARK_LOOK, 1, ATS_DATEI);
		spieler.R22Paint = true;
		obj->calc_rsi_flip_flop(SIB_PAINT_R22);
		obj->hide_sib(SIB_PAINT_R22);
		del_inventar(spieler.AkInvent);
		obj->calc_all_static_detail();
		flags.AutoAniPlay = false;
	}
	return (action_flag);
}

void r23_cockpit() {
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	maus_links_click = false;
	switch_room(23);
	if (spieler.R23Cartridge)
		det->show_static_spr(3);
	else
		det->hide_static_spr(3);
}

int16 r23_start_gleiter() {
	int16 i;
	int16 start_ok;
	int16 action_flag = false;
	if (!spieler.inv_cur) {
		action_flag = true;
		start_ok = false;
		if (!spieler.R23FluxoFlex)
			start_aad_wait(23, -1);
		else {
			start_ok = true;
			if (spieler.R23GleiterExit == 16) {
				if (!spieler.R16F5Exit) {
					start_ok = false;
					start_aad_wait(35, -1);
				} else if ((!spieler.R23Cartridge) || (!spieler.R18CartSave)) {

					start_ok = false;
					start_aad_wait(41, -1);
				} else if (!spieler.R17EnergieOut) {

					start_ok = false;
					start_aad_wait(300, -1);
				}
			}
			if (start_ok) {
				hide_cur();
				start_ani_block(4, ablock15);
				wait_show_screen(30);
				for (i = 0; i < 4; i++)
					det->stop_detail(i);
				if (spieler.R23GleiterExit == 14) {
					out->setze_zeiger(0);
					out->cls();
					flic_cut(FCUT_012, CFO_MODE);
					out->cls();
					spieler.R23GleiterExit = 16;
					set_person_pos(126, 110, P_CHEWY, P_RIGHT);
					switch_room(spieler.R23GleiterExit);
					start_spz(CH_WONDER1, 1, ANI_VOR, P_CHEWY);
					while (flags.SpzAni)
						set_up_screen(DO_SETUP);
					start_spz(CH_TALK2, 255, ANI_VOR, P_CHEWY);
					spieler.DelaySpeed = 10;
					start_aad_wait(59, -1);
					stop_person(P_CHEWY);
					maus_links_click = false;
				} else if (spieler.R23GleiterExit == 16) {
					out->setze_zeiger(0);
					out->cls();
					flic_cut(FCUT_019, CFO_MODE);
					spieler.R23GleiterExit = 25;

					switch_room(spieler.R23GleiterExit);
				}

				show_cur();
			}
		}
	}
	return (action_flag);
}

void r23_use_cartridge() {
	del_inventar(spieler.AkInvent);
	atds->set_ats_str(111, 1, ATS_DATEI);
	atds->del_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATEI);
	spieler.R23Cartridge = true;
	if (spieler.R18CartSave) {
		start_detail_wait(4, 1, ANI_VOR);
		det->show_static_spr(3);
	}
	menu_item_vorwahl = CUR_USE;
}

void r23_get_cartridge() {
	atds->set_ats_str(111, 0, ATS_DATEI);
	atds->set_steuer_bit(171, ATS_AKTIV_BIT, ATS_DATEI);
	if (spieler.R18CartSave) {
		spieler.R23Cartridge = false;
		det->hide_static_spr(3);
	}
}

uint8 kristall_spr[3][3] = { {14, 20, 13},
	{20, 13, 14},
	{13, 14, 20},
};

void r24_entry() {
	int16 i;
	flags.MainInput = false;
	spieler.PersonHide[P_CHEWY] = true;
	set_person_pos(0, 0, P_CHEWY, -1);
	spieler.scrollx = 0;
	spieler.scrolly = 0;
	curblk.sprite = room_blk.DetImage;
	curani.ani_anf = 7;
	curani.ani_end = 10;
	menu_item = CUR_USER;
	cursor_wahl(CUR_USER);
	if (spieler.R16F5Exit)
		det->show_static_spr(10);
	else
		det->hide_static_spr(10);
	r24_calc_hebel_spr();
	r24_calc_animation(255);
	for (i = 0; i < 3; i++) {
		if (kristall_spr[i][spieler.R24Hebel[i]] == 20)
			det->start_detail(5 + i * 4, 255, ANI_RUECK);
	}
}

void r24_exit() {
	spieler.PersonHide[P_CHEWY] = false;
	menu_item = CUR_WALK;
	cursor_wahl(menu_item);
	set_person_pos(263, 144, P_CHEWY, -1);
	spieler.scrollx = 88;
	maus_links_click = false;
	flags.MainInput = true;
}

void r24_use_hebel(int16 txt_nr) {
	if (!spieler.R24Hebel[txt_nr - 161] ||
	        spieler.R24Hebel[txt_nr - 161] == 2) {
		spieler.R24Hebel[txt_nr - 161] = 1;
		spieler.R24HebelDir[txt_nr - 161] ^= 1;
	} else {
		if (spieler.R24HebelDir[txt_nr - 161])
			spieler.R24Hebel[txt_nr - 161] = 0;
		else
			spieler.R24Hebel[txt_nr - 161] = 2;
	}
	r24_calc_hebel_spr();
	r24_calc_animation(txt_nr - 161);

	if (spieler.R24Hebel[0] == 1 &&
	        spieler.R24Hebel[1] == 0 &&
	        spieler.R24Hebel[2] == 2) {
		spieler.R16F5Exit = true;
		det->enable_sound(1, 0);
		det->disable_sound(1, 1);
		det->start_detail(1, 1, ANI_VOR);
		det->show_static_spr(10);
		atds->set_ats_str(164, TXT_MARK_NAME, 1, ATS_DATEI);
	}
	else if (spieler.R16F5Exit) {
		det->hide_static_spr(10);
		det->enable_sound(1, 1);
		det->disable_sound(1, 0);
		det->start_detail(1, 1, ANI_RUECK);
		spieler.R16F5Exit = false;
		atds->set_ats_str(164, TXT_MARK_NAME, 0, ATS_DATEI);
	}
}

void r24_calc_hebel_spr() {
	int16 i;
	int16 j;
	if (!spieler.R24FirstEntry) {
		spieler.R24FirstEntry = true;
		spieler.R24Hebel[0] = 2;
		spieler.R24HebelDir[0] = 0;
		spieler.R24Hebel[1] = 1;
		spieler.R24HebelDir[1] = 0;
		spieler.R24Hebel[2] = 0;
		spieler.R24HebelDir[2] = 1;
	}
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			det->hide_static_spr(1 + j + i * 3);
		det->show_static_spr(1 + spieler.R24Hebel[i] + i * 3);
		atds->set_ats_str(166 + i, TXT_MARK_NAME, spieler.R24Hebel[i], ATS_DATEI);
	}
}

void r24_calc_animation(int16 kristall_nr) {
	int16 i;
	int16 ani_nr;
	if (kristall_nr != 255) {
		hide_cur();

		if (kristall_spr[kristall_nr][spieler.R24Hebel[kristall_nr]] == 20) {
			if (spieler.R24KristallLast[kristall_nr] == 13) {
				ani_nr = 7;
			} else {
				ani_nr = 8;
			}
			det->enable_sound(ani_nr + kristall_nr * 4, 0);
			det->disable_sound(5 + ani_nr + kristall_nr * 4, 0);
			det->hide_static_spr(spieler.R24KristallLast[kristall_nr] + kristall_nr * 2);
			start_detail_wait(ani_nr + kristall_nr * 4, 1, ANI_RUECK);
			start_detail_wait(6 + kristall_nr * 4, 1, ANI_RUECK);
			det->start_detail(5 + kristall_nr * 4, 255, ANI_RUECK);
		}

		else if (spieler.R24KristallLast[kristall_nr] == 20) {

			if (kristall_spr[kristall_nr][spieler.R24Hebel[kristall_nr]] == 13) {
				ani_nr = 7;
			} else {
				ani_nr = 8;
			}
			det->disable_sound(ani_nr + kristall_nr * 4, 0);
			det->enable_sound(5 + ani_nr + kristall_nr * 4, 0);
			det->stop_detail(5 + kristall_nr * 4);
			start_detail_wait(6 + kristall_nr * 4, 1, ANI_VOR);
			start_detail_wait(ani_nr + kristall_nr * 4, 1, ANI_VOR);
		}
		show_cur();
	}

	for (i = 0; i < 6; i++)
		det->hide_static_spr(13 + i);
	for (i = 0; i < 3; i++) {
		det->show_static_spr(kristall_spr[i][spieler.R24Hebel[i]] + i * 2);
		spieler.R24KristallLast[i] = kristall_spr[i][spieler.R24Hebel[i]];
	}
}

int16 r6_cut_serv1(int16 frame) {
	atds->print_aad(spieler.scrollx, spieler.scrolly);
	if (frame == 44)
		start_aad(108, 0);
	return (0);
}

int16 r6_cut_serv2(int16 frame) {
	atds->print_aad(spieler.scrollx, spieler.scrolly);
	if (frame == 31)
		start_aad(107, 0);
	return (0);
}

int16 r12_cut_serv(int16 frame) {
	atds->print_aad(spieler.scrollx, spieler.scrolly);
	if (frame == 43)
		start_aad(106, 0);
	return (0);
}

} // namespace Chewy
