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
#include "chewy/room.h"
#include "chewy/rooms/room39.h"
#include "chewy/rooms/room43.h"

namespace Chewy {
namespace Rooms {

static const uint8 TV_FLIC[] = {
	FCUT_039,
	FCUT_040,
	FCUT_035,
	FCUT_032,
	FCUT_037,
	FCUT_034
};

static const AniBlock ABLOCK33[2] = {
	{ 2, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 3, 255, ANI_VOR, ANI_GO, 0 },
};


static const AniBlock ABLOCK29[2] = {
	{ 8, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 9, 4, ANI_VOR, ANI_WAIT, 0 },
};

bool Room39::_flag;

void Room39::entry() {
	if (!_G(spieler).R41Einbruch) {
		if (_G(spieler).R39HowardDa) {
			det->show_static_spr(10);
			if (!_G(spieler).R39HowardWach)
				det->start_detail(1, 255, ANI_VOR);
			else
				det->set_static_ani(5, -1);

			atds->del_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATEI);
		}
	} else {
		atds->set_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATEI);
	}

	set_tv();
}

short Room39::use_howard() {
	int16 ani_nr = 0;
	int16 action_flag = false;

	if (!_G(spieler).R39HowardWach) {
		int16 dia_nr;
		if (_G(spieler).inv_cur) {
			if (is_cur_inventar(MANUSKRIPT_INV)) {
				hide_cur();
				_G(spieler).R39HowardWach = true;
				_G(spieler).R39ScriptOk = true;
				auto_move(3, P_CHEWY);
				_G(spieler).PersonHide[P_CHEWY] = true;
				det->start_detail(6, 255, ANI_VOR);
				start_aad_wait(170, -1);
				det->stop_detail(6);
				start_detail_wait(7, 1, ANI_VOR);
				_G(spieler).PersonHide[P_CHEWY] = false;

				del_inventar(_G(spieler).AkInvent);
				det->stop_detail(1);
				start_ani_block(2, ABLOCK33);
				start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(167, -1);
				det->stop_detail(3);
				start_detail_wait(4, 1, ANI_VOR);
				det->set_static_ani(5, -1);
				atds->set_ats_str(62, 1, ATS_DATEI);
				start_aad_wait(169, -1);
				show_cur();

				_G(spieler).PersonGlobalDia[P_HOWARD] = 10012;
				_G(spieler).PersonDiaRoom[P_HOWARD] = true;;
				calc_person_dia(P_HOWARD);

				if (_G(spieler).R41HowardDiaOK) {
					ok();
				}

				dia_nr = -1;
				action_flag = true;
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

	return action_flag;
}

void Room39::talk_howard() {
	if (_G(spieler).R39HowardWach) {
		auto_move(3, P_CHEWY);
		_G(spieler).PersonGlobalDia[P_HOWARD] = 10012;
		_G(spieler).PersonDiaRoom[P_HOWARD] = true;
		calc_person_dia(P_HOWARD);

		if (_G(spieler).R41HowardDiaOK) {
			ok();
		}
	} else {
		start_spz(CH_TALK5, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(168, -1);
	}
}

void Room39::ok() {
	_G(spieler).R41Einbruch = true;

	Room43::night_small();
	_G(spieler).PersonRoomNr[P_HOWARD] = 27;
	obj->show_sib(SIB_SURIMY_R27);
	obj->show_sib(SIB_ZEITUNG_R27);
	obj->calc_rsi_flip_flop(SIB_SURIMY_R27);
	obj->calc_rsi_flip_flop(SIB_ZEITUNG_R27);
	invent_2_slot(BRIEF_INV);

	switch_room(27);
	start_aad_wait(192, -1);
	_G(menu_item) = CUR_WALK;
	cursor_wahl(_G(menu_item));
}

int16 Room39::use_tv() {
	int16 dia_nr = -1;
	int16 ani_nr = -1;
	int16 action_flag = false;

	hide_cur();
	auto_move(2, P_CHEWY);
	int16 cls_flag = false;

	if (is_cur_inventar(ZAPPER_INV)) {
		_G(maus_links_click) = false;
		
		_G(spieler).R39TvOn = true;
		if (_G(spieler).R39TvKanal >= 5)
			_G(spieler).R39TvKanal = -1;

		flags.NoPalAfterFlc = true;
		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();
		flic_cut(FCUT_042, CFO_MODE);
		++_G(spieler).R39TvKanal;
		flags.NoPalAfterFlc = true;

		if (_G(spieler).R39TvKanal == 2)
			flic_cut(FCUT_036, CFO_MODE);
		else if (_G(spieler).R39TvKanal == 5)
			flic_cut(FCUT_033, CFO_MODE);

		look_tv(false);
		set_tv();
		cls_flag = true;

		if (!_G(spieler).R39TransMensch) {
			ani_nr = CH_TALK11;
			dia_nr = 78;
		} else if (!_G(spieler).R39TvKanal && _G(spieler).R39ClintNews < 3) {
			dia_nr = -1;
			ani_nr = -1;
		} else {
			if (80 + _G(spieler).R39TvKanal != 85)
				dia_nr = -1;
			else
				dia_nr = 85;
			
			ani_nr = -1;
		}

		action_flag = true;
	} else if (is_cur_inventar(TRANSLATOR_INV) && _G(spieler).ChewyAni != CHEWY_ROCKER) {
		action_flag = true;
		if (_G(spieler).R39TvOn) {
			start_spz_wait(CH_TRANS, 1, false, P_CHEWY);
			_G(spieler).R39TransMensch = true;
			flags.NoPalAfterFlc = true;
			flic_cut(FCUT_041, CFO_MODE);
			_G(spieler).R39TvKanal = 0;
			_G(spieler).R39ClintNews = 0;
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			_G(out)->set_palette(_G(pal));
			flags.NoPalAfterFlc = true;
			flic_cut(TV_FLIC[0], CFO_MODE);

			_G(out)->cls();
			_G(out)->setze_zeiger(nullptr);
			_G(out)->cls();
			_G(out)->set_palette(_G(pal));
			set_tv();
			start_spz(CH_TRANS, 255, ANI_VOR, P_CHEWY);
			start_aad_wait(80, -1);

			ani_nr = CH_TRANS;
			dia_nr = 77;
		} else {
			ani_nr = CH_TALK11;
			dia_nr = 76;
		}

		set_tv();

	} else if (is_cur_inventar(RECORDER_INV) && _G(spieler).ChewyAni != CHEWY_ROCKER) {
		action_flag = true;
		if (_G(spieler).R39TvOn) {
			if (_G(spieler).R39TransMensch) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
				start_aad_wait(98, -1);
				_G(spieler).PersonHide[P_CHEWY] = true;
				start_ani_block(2, ABLOCK29);
				_G(spieler).PersonHide[P_CHEWY] = false;
				ani_nr = CH_TALK5;
				dia_nr = 99;
				atds->set_ats_str(RECORDER_INV, _G(spieler).R39TvKanal + 1, INV_ATS_DATEI);
				_G(spieler).R39TvRecord = _G(spieler).R39TvKanal + 1;
			} else {
				ani_nr = CH_TALK12;
				dia_nr = 97;
			}
		} else {
			ani_nr = CH_TALK11;
			dia_nr = 76;
		}
	}

	if (cls_flag) {
		_G(out)->cls();
		_G(out)->setze_zeiger(nullptr);
		_G(out)->cls();
		_G(out)->set_palette(_G(pal));
		flags.NoPalAfterFlc = false;
	}

	if (dia_nr != -1) {
		if (ani_nr != -1)
			start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(dia_nr, -1);
		action_flag = true;
	}

	show_cur();
	return action_flag;
}

void Room39::look_tv(bool cls_mode) {
	_flag = false;

	if (_G(spieler).R39TvOn) {
		if (!flags.AutoAniPlay) {
			flags.AutoAniPlay = true;
			int16 flic_nr;
			int16 dia_nr;
			if (!_G(spieler).R39TvKanal && _G(spieler).R39ClintNews < 3) {
				flic_nr = FCUT_038;
				++_G(spieler).R39ClintNews;
				dia_nr = 79;
			} else {
				flic_nr = TV_FLIC[_G(spieler).R39TvKanal];
				if (!_G(spieler).R39TvKanal)
					_G(spieler).R39ClintNews = 0;
				dia_nr = 80 + _G(spieler).R39TvKanal;
			}

			if (cls_mode) {
				_G(out)->setze_zeiger(nullptr);
				_G(out)->cls();
				_G(out)->set_palette(_G(pal));
				flags.NoPalAfterFlc = true;
			}

			if (_G(spieler).R39TransMensch) {
				if (dia_nr != 85)
					start_aad(dia_nr, -1);
				else
					_flag = true;
			}

			flc->set_custom_user_function(setup_func);
			flic_cut(flic_nr, CFO_MODE);
			flc->remove_custom_user_function();

			if (cls_mode) {
				_G(out)->cls();
				_G(out)->setze_zeiger(nullptr);
				_G(out)->cls();
				_G(out)->set_palette(_G(pal));
				flags.NoPalAfterFlc = false;

				if (_G(spieler).R39TransMensch && dia_nr == 85)
					start_aad_wait(dia_nr, -1);
			}
		}

		_G(maus_links_click) = false;
		flags.AutoAniPlay = false;
	}
}

void Room39::set_tv() {
	for (int16 i = 0; i < 6; i++)
		det->hide_static_spr(i + 4);

	if (_G(spieler).R39TvOn) {
		if (_G(spieler).R39TvKanal == 2) {
			det->start_detail(0, 255, ANI_VOR);
		} else {
			det->stop_detail(0);
			det->show_static_spr(_G(spieler).R39TvKanal + 4);
		}

		if (_G(spieler).R39TransMensch) {
			atds->set_ats_str(229, TXT_MARK_LOOK, 2 + _G(spieler).R39TvKanal, ATS_DATEI);
		} else {
			atds->set_ats_str(229, TXT_MARK_LOOK, 1, ATS_DATEI);
		}
	}
}

int16 Room39::setup_func(int16 frame) {
	int16 action_ret = _G(in)->get_switch_code() == Common::KEYCODE_ESCAPE ? -1 : 0;

	if (_flag) {
		if (frame == 121)
			start_aad(599, -1);

		switch (frame) {
		case 247:
			start_aad(600, -1);
			break;
		case 267:
			start_aad(601, 0);
			break;
		case 297:
			_G(in)->_hotkey = 1;
			break;
		case 171:
		case 266:
		case 370:
			atds->stop_aad();
			break;
		default:
			break;
		}
	}

	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);

	if (!_flag && atds->aad_get_status() == -1)
		action_ret = -1;

	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
