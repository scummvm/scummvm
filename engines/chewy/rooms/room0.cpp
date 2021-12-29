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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/rooms/room0.h"
#include "chewy/rooms/rooms.h"

namespace Chewy {
namespace Rooms {

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
#define VOR 0
#define RUECK 1
#define SCHLAUCH1 11
#define SCHLAUCH2 38
#define SCHLAUCH3 59
#define KOPF1 39
#define KOPF2 46
#define KOPF3 48

void Room0::entry() {
	if (is_cur_inventar(0) || _G(spieler).R0KissenWurf ||
			obj->check_inventar(0))
		det->hide_static_spr(6);

	if (!flags.LoadGame) {
		set_person_pos(150, 100, 0, 1);
		_G(cur_hide_flag) = 0;
		hide_cur();
		_G(timer_nr)[0] = room->set_timer(255, 3);

		while (!ani_timer[_G(timer_nr)[0]].TimeFlag && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
#ifdef TODO_REENABLE
		start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(2, -1);
#endif
		show_cur();
	}
}

bool Room0::timer(int16 t_nr, int16 ani_nr) {
	switch (ani_nr) {
	case 1:
		if (timer_action_ctr > 0) {
			uhr->reset_timer(t_nr, 0);
			--timer_action_ctr;
		} else if (!is_chewy_busy()) {
			if (!_G(spieler).R0FueterLab)
				timer_action_ctr = 2;

			flags.AutoAniPlay = true;
			if (!_G(spieler).R0SlimeUsed) {
				start_aad_wait(42, -1);
				auto_move(5, 0);
				set_person_spr(0, 0);

				if (_G(spieler).R0FueterLab < 3) {
					start_spz(2, 255, false, 0);
					if (_G(spieler).R0FueterLab)
						start_aad_wait(618, -1);
					else
						start_aad_wait(43, -1);
				}

				eyeAnim();
			} else if (!_G(spieler).R0KissenWurf) {
				start_aad_wait(42, -1);
				start_spz(2, 255, false, 0);

				if (_G(spieler).R0FueterLab < 3) {
					start_aad_wait(43, -1);
					++_G(spieler).R0FueterLab;
				}

				auto_move(3, 0);
				set_person_pos(191, 120, P_CHEWY, P_LEFT);
			}

			if (!_G(spieler).R0KissenWurf)
				fuett_ani();

			uhr->reset_timer(t_nr, 0);
			flags.AutoAniPlay = false;
		}
		break;

	default:
		break;
	}

	return false;
}

bool Room0::getPillow() {
	if (!_G(spieler).inv_cur) {
		hide_cur();
		flags.AutoAniPlay = true;
		auto_move(1, 0);
		start_spz_wait(13, 1, 0, 0);
		invent_2_slot(0);
		menu_item = 0;
		cursor_wahl(0);
		atds->set_steuer_bit(174, 1, 1);
		det->hide_static_spr(6);

		flags.AutoAniPlay = false;
		show_cur();
		return true;
	}

	return false;
}

bool Room0::pullSlime() {
	if (!_G(spieler).inv_cur) {
		hide_cur();
		auto_move(2, 0);
		_G(spieler).PersonHide[0] = 1;
		start_detail_wait(3, 1, 0);
		set_person_pos(222, 106, 0, 0);
		_G(spieler).PersonHide[0] = 0;
		invent_2_slot(1);
		menu_item = 0;
		cursor_wahl(0);
		atds->set_steuer_bit(175, 1, 1);

		flags.AutoAniPlay = false;
		show_cur();
		return true;
	}

	return false;
}

void Room0::eyeAnim() {
	if (!_G(spieler).R0SlimeUsed) {
		// Start the eye animation
		eyeStart(EYE_START);
		if (!_G(spieler).R0SlimeUsed)
			eyeWait();

		if (_G(spieler).R0SlimeUsed) {
			start_aad(124);
			ch_schleim_auge();
			auge_schleim_back();
			auto_move(FUETTER_POS, P_CHEWY);
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		} else {
			eyeShoot();
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
			eyeStart(EYE_END);
		}
	}
}

void Room0::eyeStart(EyeMode mode) {
	ani_detail_info *adi;
	bool ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	if (mode == EYE_START)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 38;

	if (mode == EYE_START) {
		ani_klappe_delay();
	}

	ende = false;
	flags.AniUserAction = true;

	if (mode == EYE_START) {
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
		calc_auge_click(3);
		out->back2screen(workpage);
		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			if (mode == EYE_START) {
				++adi->ani_count;
				if (adi->ani_count > 38)
					ende = true;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					ende = true;
			}
		}
	}

	clear_prog_ani();
	flags.AniUserAction = false;

	if (mode == EYE_END) {
		det->start_detail(KLAPPE_DETAIL, 1, RUECK);
		while (det->get_ani_status(KLAPPE_DETAIL)) {
			set_ani_screen();
			EVENTS_UPDATE;
			SHOULD_QUIT_RETURN;
		}
	}
}

void Room0::eyeWait() {
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
		calc_auge_click(2);
		out->back2screen(workpage);

		if (adi->delay_count > 0) {
			--adi->delay_count;
		} else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}

	flags.AniUserAction = false;
	clear_prog_ani();
}

void Room0::calc_auge_click(int16 ani_nr) {
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
				del_inventar(_G(spieler).AkInvent);
				_G(spieler).R0SlimeUsed = true;
			} else if (is_cur_inventar(KISSEN_INV)) {

				start_ats_wait(172, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void Room0::eyeShoot() {
	ani_detail_info *adi;
	bool ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	adi->ani_count = 47;

	ende = false;
	det->start_detail(CH_BLITZ, 1, VOR);

	while (!ende) {
		clear_prog_ani();
		_G(spieler).PersonHide[P_CHEWY] = true;
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
				ende = true;
		}

		set_up_screen(DO_SETUP);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}
	}

	det->start_detail(STERNE_STEHEN, 255, VOR);
	clear_prog_ani();
	spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
	spr_info[0].ZEbene = 190;
	spr_info[1] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
	spr_info[1].ZEbene = 191;
	spr_info[2] = det->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF2, ANI_HIDE);
	spr_info[2].ZEbene = 192;

	wait_show_screen(30);
	clear_prog_ani();
	set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
}

void Room0::auge_schleim_back() {
	ani_detail_info *adi;
	bool ende;

	adi = det->get_ani_detail(SCHLAUCH_DETAIL);
	adi->ani_count = 53;

	ende = false;
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
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
			if (adi->ani_count == 77)
				ende = true;
		}
	}

	flags.AniUserAction = false;
	clear_prog_ani();
}

void Room0::ch_schleim_auge() {
	ani_detail_info *adi;

	adi = det->get_ani_detail(CH_WIRFT_SCHLEIM);
	adi->ani_count = adi->start_ani;
	if (adi->load_flag) {
		det->load_taf_seq(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, 0);
	}

	while (adi->ani_count < adi->end_ani && !SHOULD_QUIT) {
		clear_prog_ani();
		_G(spieler).PersonHide[P_CHEWY] = true;
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
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}
	}

	if (adi->load_flag) {
		det->del_taf_tbl(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, 0);
	}

	clear_prog_ani();
	_G(spieler).PersonHide[P_CHEWY] = false;
}

void Room0::fuetter_start(int16 mode) {
	ani_detail_info *adi;
	bool ende;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	if (!mode)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 135;

	if (!mode) {
		ani_klappe_delay();
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

	ende = false;
	if (_G(spieler).R0SlimeUsed)
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
			calc_kissen_click(1);

		out->back2screen(workpage);
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			if (!mode) {
				++adi->ani_count;
				if (adi->ani_count > 135)
					ende = true;
			} else {
				--adi->ani_count;
				if (adi->ani_count == adi->start_ani - 1)
					ende = true;
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

void Room0::kissen_wurf() {
	for (int16 i = 0; i < 30 && !_G(spieler).R0KissenWurf; i++) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		spr_info[1] = det->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 136, ANI_HIDE);
		spr_info[1].ZEbene = 191;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		cur->plot_cur();
		calc_kissen_click(1);
		out->back2screen(workpage);
	}

	clear_prog_ani();
}

void Room0::calc_kissen_click(int16 ani_nr) {
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
			if (is_cur_inventar(KISSEN_INV) && _G(spieler).R0SlimeUsed) {
				del_inventar(_G(spieler).AkInvent);
				_G(spieler).R0KissenWurf = 1;
			} else if (is_cur_inventar(SCHLEIM_INV)) {
				start_ats_wait(173, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void Room0::ch_fuetter() {
	ani_detail_info *adi;
	int16 i;
	bool ende;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	adi->ani_count = 136;

	i = 152;
	ende = false;

	if (_G(spieler).R0SlimeUsed)
		flags.AniUserAction = true;
	while (!ende) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, KLAPPE_DETAIL, KLAPPE_SPRITE, ANI_HIDE);
		spr_info[0].ZEbene = 190;
		if (adi->ani_count == 136) {
			_G(spieler).PersonHide[P_CHEWY] = true;
			if (!_G(spieler).R0SlimeUsed)
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
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			if (adi->ani_count > 141)
				++i;
			++adi->ani_count;
			if (adi->ani_count == 152)
				ende = true;
		}
	}

	adi->ani_count = 138;

	ende = false;
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
			ende = true;
		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			if (adi->ani_count > 135)
				--adi->ani_count;
		}
	}

	_G(spieler).PersonHide[P_CHEWY] = false;
	flags.AniUserAction = false;
	clear_prog_ani();
}

void Room0::ch_kissen() {
	ani_detail_info *adi;
	bool ende, mode;

	adi = det->get_ani_detail(FUETTER_SCHLAUCH);
	adi->ani_count = 161;

	ende = false;
	_G(spieler).PersonHide[P_CHEWY] = true;
	det->start_detail(CH_WIRFT_KISSEN, 1, VOR);
	mode = 0;

	while (!ende) {
		clear_prog_ani();
		if (!det->get_ani_status(CH_WIRFT_KISSEN)) {
			mode = 1;
			_G(spieler).PersonHide[P_CHEWY] = false;
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
				adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
				--adi->ani_count;
				if (adi->ani_count == 151)
					ende = true;
			}
		}
	}

	clear_prog_ani();
}

void Room0::ani_klappe_delay() {
	int16 i;
	det->start_detail(KLAPPE_DETAIL, 1, VOR);
	while (det->get_ani_status(KLAPPE_DETAIL) && !SHOULD_QUIT) {
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

void Room0::fuett_ani() {
	int16 action;
	action = false;
	fuetter_start(0);

	if (_G(spieler).R0SlimeUsed) {
		kissen_wurf();
		if (_G(spieler).R0KissenWurf) {
			ch_kissen();
			fuetter_start(1);
			auto_move(VERSTECK_POS, P_CHEWY);
			set_up_screen(DO_SETUP);
			out->cls();
			flic_cut(FCUT_001, CFO_MODE);

			test_intro(1);
			ERROR

			_G(spieler).PersonRoomNr[P_CHEWY] = 1;
			room->load_room(&room_blk, _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
			ERROR
			set_person_pos(Rdi->AutoMov[4].X - CH_HOT_MOV_X,
			               Rdi->AutoMov[4].Y - CH_HOT_MOV_Y, P_CHEWY, P_RIGHT);
			spieler_vector[P_CHEWY].DelayCount = 0;

			check_shad(4, 0);
			fx_blend = BLEND1;
			set_up_screen(DO_SETUP);
		} else {
			action = true;
		}
	} else {
		action = true;
	}

	if (action) {
		ch_fuetter();
		start_spz(CH_EKEL, 3, ANI_VOR, P_CHEWY);
		start_aad(55);
		fuetter_start(1);
	}
}

} // namespace Rooms
} // namespace Chewy
