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
	{ 2, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 3, 255, ANI_FRONT, ANI_GO, 0 },
};


static const AniBlock ABLOCK29[2] = {
	{ 8, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 9, 4, ANI_FRONT, ANI_WAIT, 0 },
};

void Room39::entry() {
	if (!_G(spieler).R41Einbruch) {
		if (_G(spieler).R39HowardDa) {
			_G(det)->showStaticSpr(10);
			if (!_G(spieler).R39HowardWach)
				_G(det)->startDetail(1, 255, ANI_FRONT);
			else
				_G(det)->set_static_ani(5, -1);

			_G(atds)->del_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATA);
		}
	} else {
		_G(atds)->set_steuer_bit(62, ATS_AKTIV_BIT, ATS_DATA);
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
				hideCur();
				_G(spieler).R39HowardWach = true;
				_G(spieler).R39ScriptOk = true;
				autoMove(3, P_CHEWY);
				_G(spieler)._personHide[P_CHEWY] = true;
				_G(det)->startDetail(6, 255, ANI_FRONT);
				startAadWait(170);
				_G(det)->stop_detail(6);
				startSetailWait(7, 1, ANI_FRONT);
				_G(spieler)._personHide[P_CHEWY] = false;

				delInventory(_G(spieler).AkInvent);
				_G(det)->stop_detail(1);
				startAniBlock(2, ABLOCK33);
				start_spz(CH_TALK6, 255, ANI_FRONT, P_CHEWY);
				startAadWait(167);
				_G(det)->stop_detail(3);
				startSetailWait(4, 1, ANI_FRONT);
				_G(det)->set_static_ani(5, -1);
				_G(atds)->set_ats_str(62, 1, ATS_DATA);
				startAadWait(169);
				showCur();

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
			start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
			startAadWait(dia_nr);
			action_flag = true;
		}

		showCur();
	}

	return action_flag;
}

void Room39::talk_howard() {
	if (_G(spieler).R39HowardWach) {
		autoMove(3, P_CHEWY);
		_G(spieler).PersonGlobalDia[P_HOWARD] = 10012;
		_G(spieler).PersonDiaRoom[P_HOWARD] = true;
		calc_person_dia(P_HOWARD);

		if (_G(spieler).R41HowardDiaOK) {
			ok();
		}
	} else {
		start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
		startAadWait(168);
	}
}

void Room39::ok() {
	_G(spieler).R41Einbruch = true;

	Room43::night_small();
	_G(spieler)._personRoomNr[P_HOWARD] = 27;
	_G(obj)->show_sib(SIB_SURIMY_R27);
	_G(obj)->show_sib(SIB_ZEITUNG_R27);
	_G(obj)->calc_rsi_flip_flop(SIB_SURIMY_R27);
	_G(obj)->calc_rsi_flip_flop(SIB_ZEITUNG_R27);
	invent_2_slot(BRIEF_INV);

	switchRoom(27);
	startAadWait(192);
	_G(menu_item) = CUR_WALK;
	cursorChoice(_G(menu_item));
}

int16 Room39::use_tv() {
	int16 dia_nr = -1;
	int16 ani_nr = -1;
	int16 action_flag = false;

	hideCur();
	autoMove(2, P_CHEWY);
	int16 cls_flag = false;

	if (is_cur_inventar(ZAPPER_INV)) {
		_G(maus_links_click) = false;
		
		_G(spieler).R39TvOn = true;
		if (_G(spieler).R39TvKanal >= 5)
			_G(spieler).R39TvKanal = -1;

		_G(flags).NoPalAfterFlc = true;
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		flic_cut(FCUT_042);
		++_G(spieler).R39TvKanal;
		_G(flags).NoPalAfterFlc = true;

		if (_G(spieler).R39TvKanal == 2)
			flic_cut(FCUT_036);
		else if (_G(spieler).R39TvKanal == 5)
			flic_cut(FCUT_033);

		look_tv(false);
		set_tv();
		cls_flag = true;

		if (!_G(spieler).R39TranslatorUsed) {
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
			_G(spieler).R39TranslatorUsed = true;
			_G(flags).NoPalAfterFlc = true;
			flic_cut(FCUT_041);
			_G(spieler).R39TvKanal = 0;
			_G(spieler).R39ClintNews = 0;
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			_G(out)->setPalette(_G(pal));
			_G(flags).NoPalAfterFlc = true;
			flic_cut(TV_FLIC[0]);

			_G(out)->cls();
			_G(out)->setPointer(nullptr);
			_G(out)->cls();
			_G(out)->setPalette(_G(pal));
			set_tv();
			start_spz(CH_TRANS, 255, ANI_FRONT, P_CHEWY);
			startAadWait(80);

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
			if (_G(spieler).R39TranslatorUsed) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
				startAadWait(98);
				_G(spieler)._personHide[P_CHEWY] = true;
				startAniBlock(2, ABLOCK29);
				_G(spieler)._personHide[P_CHEWY] = false;
				ani_nr = CH_TALK5;
				dia_nr = 99;
				_G(atds)->set_ats_str(RECORDER_INV, _G(spieler).R39TvKanal + 1, INV_ATS_DATA);
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
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(out)->setPalette(_G(pal));
		_G(flags).NoPalAfterFlc = false;
	}

	if (dia_nr != -1) {
		if (ani_nr != -1)
			start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
		startAadWait(dia_nr);
		action_flag = true;
	}

	showCur();
	return action_flag;
}

void Room39::look_tv(bool cls_mode) {
	if (_G(spieler).R39TvOn) {
		if (!_G(flags).AutoAniPlay) {
			_G(flags).AutoAniPlay = true;
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
				_G(out)->setPointer(nullptr);
				_G(out)->cls();
				_G(out)->setPalette(_G(pal));
				_G(flags).NoPalAfterFlc = true;
			}

			if (_G(spieler).R39TranslatorUsed) {
				if (dia_nr != 85)
					start_aad(dia_nr, -1);
			}

			flic_cut(flic_nr);

			if (cls_mode) {
				_G(out)->cls();
				_G(out)->setPointer(nullptr);
				_G(out)->cls();
				_G(out)->setPalette(_G(pal));
				_G(flags).NoPalAfterFlc = false;

				if (_G(spieler).R39TranslatorUsed && dia_nr == 85)
					startAadWait(dia_nr);
			}
		}

		_G(maus_links_click) = false;
		_G(flags).AutoAniPlay = false;
	}
}

void Room39::set_tv() {
	for (int16 i = 0; i < 6; i++)
		_G(det)->hideStaticSpr(i + 4);

	if (_G(spieler).R39TvOn) {
		if (_G(spieler).R39TvKanal == 2) {
			_G(det)->startDetail(0, 255, ANI_FRONT);
		} else {
			_G(det)->stop_detail(0);
			_G(det)->showStaticSpr(_G(spieler).R39TvKanal + 4);
		}

		if (_G(spieler).R39TranslatorUsed) {
			_G(atds)->set_ats_str(229, TXT_MARK_LOOK, 2 + _G(spieler).R39TvKanal, ATS_DATA);
		} else {
			_G(atds)->set_ats_str(229, TXT_MARK_LOOK, 1, ATS_DATA);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
