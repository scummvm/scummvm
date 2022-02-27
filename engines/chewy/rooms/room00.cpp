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
#include "chewy/globals.h"
#include "chewy/main.h"
#include "chewy/room.h"
#include "chewy/rooms/room00.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

#define FLAP_SPRITE 5
#define SCHLEIM_DETAIL 2
#define CH_ZIEHT_SCHLEIM 3
#define CH_BLITZ 8
#define TUER_DETAIL 9
#define FLAP_DETAIL 10
#define SCHLAUCH_DETAIL 11
#define CH_WIRFT_SCHLEIM 12
#define CH_NACH_FUETTERN 13
#define CH_WIRFT_KISSEN 14
#define FUETTER_SCHLAUCH 15
#define STERNE_STEHEN 16
#define SCHLAUCH1 11
#define SCHLAUCH2 38
#define SCHLAUCH3 59
#define KOPF1 39
#define KOPF2 46
#define KOPF3 48

void Room0::entry() {
	if (is_cur_inventar(0) || _G(spieler).R0PillowThrow || _G(obj)->checkInventory(0))
		_G(det)->hideStaticSpr(6);

	if (!_G(flags).LoadGame) {
		set_person_pos(150, 100, P_CHEWY, P_RIGHT);
		_G(cur_hide_flag) = 0;
		hideCur();
		_G(timer_nr)[0] = _G(room)->set_timer(255, 3);

		while (!_G(ani_timer)[_G(timer_nr)[0]].TimeFlag && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}

		start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
		start_aad_wait(2, -1);

		showCur();
	}
}

bool Room0::timer(int16 t_nr, int16 ani_nr) {
	bool retval = false;
	if (ani_nr == 1) {
		if (_G(timer_action_ctr) > 0) {
			_G(uhr)->reset_timer(t_nr, 0);
			--_G(timer_action_ctr);
		} else if (!is_chewy_busy()) {
			if (!_G(spieler).R0FueterLab)
				_G(timer_action_ctr) = 2;

			_G(flags).AutoAniPlay = true;
			if (!_G(spieler).R0SlimeUsed) {
				start_aad_wait(42, -1);
				autoMove(5, P_CHEWY);
				set_person_spr(P_LEFT, P_CHEWY);

				if (_G(spieler).R0FueterLab < 3) {
					start_spz(CH_TALK3, 255, false, P_CHEWY);
					if (_G(spieler).R0FueterLab)
						start_aad_wait(618, -1);
					else
						start_aad_wait(43, -1);

					++_G(spieler).R0FueterLab;
				}

				eyeAnim();
			} else if (!_G(spieler).R0PillowThrow) {
				start_aad_wait(42, -1);
				start_spz(CH_TALK3, 255, false, P_CHEWY);

				if (_G(spieler).R0FueterLab < 3) {
					start_aad_wait(43, -1);
					++_G(spieler).R0FueterLab;
				}

				autoMove(3, P_CHEWY);
				set_person_pos(191, 120, P_CHEWY, P_LEFT);
			}

			if (!_G(spieler).R0PillowThrow)
				feederAni();

			_G(uhr)->reset_timer(t_nr, 0);
			_G(flags).AutoAniPlay = false;
		}
	} else if (t_nr == 3)
		retval = true;

	return retval;
}

bool Room0::getPillow() {
	bool retval = false;
	
	if (!_G(spieler).inv_cur) {
		hideCur();
		_G(flags).AutoAniPlay = true;
		autoMove(1, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		invent_2_slot(0);
		_G(menu_item) = CUR_WALK;
		cursorChoice(CUR_WALK);
		_G(atds)->set_steuer_bit(174, ATS_AKTIV_BIT, ATS_DATEI);
		_G(det)->hideStaticSpr(6);

		_G(flags).AutoAniPlay = false;
		showCur();
		retval = true;
	}

	return retval;
}

bool Room0::pullSlime() {
	bool retval = false;
	if (!_G(spieler).inv_cur) {
		hideCur();
		
		_G(flags).AutoAniPlay = true;
		autoMove(2, P_CHEWY);
		_G(spieler).PersonHide[P_CHEWY] = true;
		start_detail_wait(3, 1, ANI_FRONT);
		start_detail_wait(17, 2, ANI_FRONT);
		set_person_pos(222, 106, P_CHEWY, P_LEFT);
		_G(spieler).PersonHide[P_CHEWY] = false;
		invent_2_slot(1);
		_G(menu_item) = CUR_WALK;
		cursorChoice(CUR_WALK);
		_G(atds)->set_steuer_bit(175, ATS_AKTIV_BIT, ATS_DATEI);

		_G(flags).AutoAniPlay = false;
		showCur();
		retval = true;
	}

	return retval;
}

void Room0::eyeAnim() {
	if (!_G(spieler).R0SlimeUsed) {
		// Start the eye animation
		eyeStart(EYE_START);
		if (!_G(spieler).R0SlimeUsed)
			eyeWait();

		if (_G(spieler).R0SlimeUsed) {
			start_aad(124);
			checkSlimeEye();
			eyeSlimeBack();
			autoMove(FUETTER_POS, P_CHEWY);
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		} else {
			eyeShoot();
			set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
			eyeStart(EYE_END);
		}
	}
}

void Room0::eyeStart(EyeMode mode) {
	AniDetailInfo *adi = _G(det)->getAniDetail(SCHLAUCH_DETAIL);
	if (mode == EYE_START)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 38;

	if (mode == EYE_START) {
		trapDoorOpen();
	}

	bool ende = false;
	_G(flags).AniUserAction = true;

	if (mode == EYE_START) {
		g_engine->_sound->playSound(FLAP_DETAIL, 0);
		g_engine->_sound->stopSound(1);
		g_engine->_sound->playSound(SCHLAUCH_DETAIL, 0);
		g_engine->_sound->stopSound(2);
	} else {
		g_engine->_sound->stopSound(0);
		g_engine->_sound->playSound(FLAP_DETAIL, 1);
		g_engine->_sound->stopSound(0);
		g_engine->_sound->playSound(SCHLAUCH_DETAIL, 2);
	}

	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		if ((adi->ani_count > 11) && (adi->ani_count < 19)) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH1, ANI_HIDE);
			_G(spr_info)[1].ZEbene = 191;
		}
		
		if (adi->ani_count == 38) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF1, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3].ZEbene = 193;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		SHOULD_QUIT_RETURN;

		_G(cur)->plot_cur();
		calcEyeClick(3);
		_G(out)->back2screen(_G(workpage));

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
	_G(flags).AniUserAction = false;

	if (mode == EYE_END) {
		trapDoorClose();
	}
}

void Room0::eyeWait() {
	AniDetailInfo *adi = _G(det)->getAniDetail(SCHLAUCH_DETAIL);
	adi->ani_count = 39;
	adi->delay_count = 15;
	_G(flags).AniUserAction = true;

	while (adi->ani_count < 46) {
		clear_prog_ani();

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;
		_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[2].ZEbene = 192;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		_G(cur)->plot_cur();
		calcEyeClick(2);
		_G(out)->back2screen(_G(workpage));

		if (adi->delay_count > 0) {
			--adi->delay_count;
		} else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::calcEyeClick(int16 ani_nr) {
	if (mouse_on_prog_ani() == ani_nr) {
		if (_G(minfo).button != 1 && _G(kbinfo).key_code != Common::KEYCODE_RETURN) {
			int16 anz;
			char *str_ = _G(atds)->ats_get_txt(172, TXT_MARK_NAME, &anz, ATS_DATEI);
			if (str_ != 0) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = _G(minfo).x;
				int16 y = _G(minfo).y;
				calc_txt_xy(&x, &y, str_, anz);
				for (int16 i = 0; i < anz; i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), _G(txt)->str_pos((char *)str_, i));
			}
		} else if (_G(minfo).button == 1 || _G(kbinfo).key_code == Common::KEYCODE_RETURN) {
			if (is_cur_inventar(SLIME_INV)) {
				del_inventar(_G(spieler).AkInvent);
				_G(spieler).R0SlimeUsed = true;
			} else if (is_cur_inventar(PILLOW_INV)) {
				start_ats_wait(172, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void Room0::eyeShoot() {
	AniDetailInfo *adi = _G(det)->getAniDetail(SCHLAUCH_DETAIL);
	adi->ani_count = 47;

	bool ende = false;
	_G(det)->start_detail(CH_BLITZ, 1, ANI_FRONT);

	while (!ende) {
		clear_prog_ani();
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;

		if (adi->ani_count < 53) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		} else {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, 47, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
			if (!_G(det)->get_ani_status(CH_BLITZ))
				ende = true;
		}

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}
	}

	_G(det)->start_detail(STERNE_STEHEN, 255, ANI_FRONT);
	clear_prog_ani();
	_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
	_G(spr_info)[0].ZEbene = 190;
	_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
	_G(spr_info)[1].ZEbene = 191;
	_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF2, ANI_HIDE);
	_G(spr_info)[2].ZEbene = 192;

	wait_show_screen(30);
	clear_prog_ani();
	set_person_pos(199 - CH_HOT_MOV_X, 145 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
	_G(spieler).PersonHide[P_CHEWY] = false;
}

void Room0::eyeSlimeBack() {
	AniDetailInfo *adi = _G(det)->getAniDetail(SCHLAUCH_DETAIL);
	adi->ani_count = 53;

	bool ende = false;
	_G(flags).AniUserAction = true;

	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;

		if ((adi->ani_count > 52) && (adi->ani_count < 59)) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
			_G(spr_info)[1].ZEbene = 191;
		}
		if (adi->ani_count == 61) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH3, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3].ZEbene = 193;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
			if (adi->ani_count == 77)
				ende = true;
		}
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::checkSlimeEye() {
	AniDetailInfo *adi = _G(det)->getAniDetail(CH_WIRFT_SCHLEIM);
	adi->ani_count = adi->start_ani;
	if (adi->load_flag) {
		_G(det)->load_taf_seq(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, nullptr);
	}

	while (adi->ani_count < adi->end_ani && !SHOULD_QUIT) {
		clear_prog_ani();
		_G(spieler).PersonHide[P_CHEWY] = true;
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, SCHLAUCH2, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;
		_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SCHLAUCH_DETAIL, KOPF2, ANI_HIDE);
		_G(spr_info)[2].ZEbene = 192;
		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, CH_WIRFT_SCHLEIM, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3].ZEbene = 193;
		set_up_screen(DO_SETUP);

		if (adi->delay_count > 0)
			--adi->delay_count;
		else {
			adi->delay_count = adi->delay + _G(spieler).DelaySpeed;
			++adi->ani_count;
		}
	}

	if (adi->load_flag) {
		_G(det)->del_taf_tbl(adi->start_ani, (adi->end_ani - adi->start_ani) + 1, nullptr);
	}

	clear_prog_ani();
	_G(spieler).PersonHide[P_CHEWY] = false;
}

void Room0::feederStart(int16 mode) {
	AniDetailInfo *adi = _G(det)->getAniDetail(FUETTER_SCHLAUCH);
	if (!mode)
		adi->ani_count = adi->start_ani;
	else
		adi->ani_count = 135;

	if (!mode) {
		trapDoorOpen();
		g_engine->_sound->playSound(FLAP_DETAIL, 0);
		g_engine->_sound->stopSound(1);
		g_engine->_sound->playSound(FUETTER_SCHLAUCH, 0);
		g_engine->_sound->stopSound(2);
	} else {
		g_engine->_sound->stopSound(0);
		g_engine->_sound->playSound(FLAP_DETAIL, 1);
		g_engine->_sound->stopSound(0);
		g_engine->_sound->playSound(FUETTER_SCHLAUCH, 2);
	}

	bool ende = false;
	if (_G(spieler).R0SlimeUsed)
		_G(flags).AniUserAction = true;

	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;

		if (_G(flags).AniUserAction)
			get_user_key(NO_SETUP);

		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;

		set_up_screen(NO_SETUP);
		_G(cur)->plot_cur();
		if (!mode)
			calcPillowClick(1);

		_G(out)->back2screen(_G(workpage));
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
	_G(flags).AniUserAction = false;

	if (mode) {
		_G(det)->start_detail(FLAP_DETAIL, 1, ANI_BACK);
		while (_G(det)->get_ani_status(FLAP_DETAIL))
			set_ani_screen();

	}
}

void Room0::feederExtend() {
	for (int16 i = 0; i < 30 && !_G(spieler).R0PillowThrow; i++) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 136, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;
		get_user_key(NO_SETUP);
		set_up_screen(NO_SETUP);
		_G(cur)->plot_cur();
		calcPillowClick(1);
		_G(out)->back2screen(_G(workpage));
	}

	clear_prog_ani();
}

void Room0::calcPillowClick(int16 ani_nr) {
	if (mouse_on_prog_ani() == ani_nr) {
		if (_G(minfo).button != 1 && _G(kbinfo).key_code != Common::KEYCODE_RETURN) {
			int16 anz;
			char *str_ = _G(atds)->ats_get_txt(173, TXT_MARK_NAME, &anz, ATS_DATEI);
			if (str_ != nullptr) {
				_G(fontMgr)->setFont(_G(font8));
				int16 x = _G(minfo).x;
				int16 y = _G(minfo).y;
				calc_txt_xy(&x, &y, str_, anz);
				for (int16 i = 0; i < anz; i++)
					printShadowed(x, y + i * 10, 255, 300, 0, _G(scr_width), _G(txt)->str_pos((char *)str_, i));
			}
		} else if (_G(minfo).button == 1 || _G(kbinfo).key_code == Common::KEYCODE_RETURN) {
			if (is_cur_inventar(PILLOW_INV) && _G(spieler).R0SlimeUsed) {
				del_inventar(_G(spieler).AkInvent);
				_G(spieler).R0PillowThrow = true;
			} else if (is_cur_inventar(SLIME_INV)) {
				start_ats_wait(173, TXT_MARK_WALK, 14, ATS_DATEI);
			}
		}
	}
}

void Room0::checkFeed() {
	AniDetailInfo *adi = _G(det)->getAniDetail(FUETTER_SCHLAUCH);
	adi->ani_count = 136;

	int16 i = 152;
	bool ende = false;

	if (_G(spieler).R0SlimeUsed)
		_G(flags).AniUserAction = true;

	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;

		if (adi->ani_count == 136) {
			_G(spieler).PersonHide[P_CHEWY] = true;
			if (!_G(spieler).R0SlimeUsed)
				_G(det)->stop_detail(16);
		}

		if (adi->ani_count > 138) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
			_G(spr_info)[1].ZEbene = 191;
		}
		
		if (adi->ani_count > 141) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, i, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		}

		if (adi->ani_count == 138) {
			_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 139, ANI_HIDE);
			_G(spr_info)[3].ZEbene = 193;
		}

		_G(spr_info)[4] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		_G(spr_info)[4].ZEbene = 194;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

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
	_G(det)->start_detail(CH_NACH_FUETTERN, 2, ANI_FRONT);

	ende = false;
	while (!ende) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;

		if (adi->ani_count > 138) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
			_G(spr_info)[1].ZEbene = 191;
		}

		if (adi->ani_count == 138) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 139, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		}

		_G(spr_info)[3] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
		_G(spr_info)[3].ZEbene = 193;
		set_ani_screen();
		SHOULD_QUIT_RETURN;

		if (!_G(det)->get_ani_status(CH_NACH_FUETTERN))
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
	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::checkPillow() {
	AniDetailInfo *adi = _G(det)->getAniDetail(FUETTER_SCHLAUCH);
	adi->ani_count = 161;

	bool ende = false;
	_G(spieler).PersonHide[P_CHEWY] = true;
	_G(det)->start_detail(CH_WIRFT_KISSEN, 1, ANI_FRONT);
	bool mode = false;

	while (!ende) {
		clear_prog_ani();
		if (!_G(det)->get_ani_status(CH_WIRFT_KISSEN)) {
			mode = true;
			_G(spieler).PersonHide[P_CHEWY] = false;
			set_person_pos(228 - CH_HOT_MOV_X, 143 - CH_HOT_MOV_Y, P_CHEWY, P_LEFT);
		}

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;
		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, 138, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 191;

		if (mode) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, FUETTER_SCHLAUCH, adi->ani_count, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 192;
		}

		set_up_screen(DO_SETUP);
		SHOULD_QUIT_RETURN;

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

void Room0::trapDoorOpen() {
	_G(det)->start_detail(FLAP_DETAIL, 1, ANI_FRONT);
	while (_G(det)->get_ani_status(FLAP_DETAIL)) {
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = true;
	for (int16 i = 0; i < 25; i++) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, FLAP_DETAIL, FLAP_SPRITE, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 190;

		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(flags).AniUserAction = false;
	clear_prog_ani();
}

void Room0::trapDoorClose() {
	_G(det)->start_detail(FLAP_DETAIL, 1, ANI_BACK);

	while (_G(det)->get_ani_status(FLAP_DETAIL)) {
		set_ani_screen();
		EVENTS_UPDATE;
		SHOULD_QUIT_RETURN;
	}
}

void Room0::feederAni() {
	int16 action = false;
	feederStart(0);

	if (_G(spieler).R0SlimeUsed) {
		feederExtend();
		if (_G(spieler).R0PillowThrow) {
			checkPillow();
			feederStart(1);
			autoMove(VERSTECK_POS, P_CHEWY);
			set_up_screen(DO_SETUP);
			_G(out)->cls();
			flic_cut(FCUT_000);

			register_cutscene(1);

			_G(spieler).PersonRoomNr[P_CHEWY] = 1;
			_G(room)->load_room(&_G(room_blk), _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
			set_person_pos(_G(Rdi)->AutoMov[4].X - CH_HOT_MOV_X,
			               _G(Rdi)->AutoMov[4].Y - CH_HOT_MOV_Y, P_CHEWY, P_RIGHT);
			_G(spieler_vector)[P_CHEWY].DelayCount = 0;

			check_shad(4, 0);
			_G(fx_blend) = BLEND1;
			set_up_screen(DO_SETUP);
		} else {
			action = true;
		}
	} else {
		action = true;
	}

	if (action) {
		checkFeed();
		start_spz(CH_EKEL, 3, ANI_FRONT, P_CHEWY);
		start_aad(55);
		feederStart(1);
	}
}

} // namespace Rooms
} // namespace Chewy
