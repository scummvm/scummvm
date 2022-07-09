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

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room33.h"

namespace Chewy {
namespace Rooms {

static const MovLine SURIMY_MPKT[2] = {
	{ {  65, 115, 150 }, 1, 6 },
	{ { 300, 151, 150 }, 1, 6 }
};


void Room33::entry() {
	if (_G(gameState).R33MunterGet)
		_G(det)->hideStaticSpr(3);
	surimy_go();
}

void Room33::surimy_go() {
	if (!_G(gameState).R39ScriptOk && _G(gameState).R25SurimyLauf) {
		if (_G(gameState).R33SurimyGo >= 4) {
			hideCur();
			_G(gameState).R33SurimyGo = 0;
			_G(det)->load_taf_seq(39, 8, nullptr);
			_G(auto_obj) = 1;
			_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
			_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
			_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
			_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
			_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
			_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(gameState).DelaySpeed + 2;
			_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;
			init_auto_obj(SURIMY_OBJ, &SURIMY_TAF19_PHASES[0][0], 2, (const MovLine *)SURIMY_MPKT);
			start_spz(CH_TALK5, 255, ANI_FRONT, P_CHEWY);
			startAadWait(158);
			wait_auto_obj(SURIMY_OBJ);
			_G(auto_obj) = 0;
			showCur();
		} else {
			++_G(gameState).R33SurimyGo;
		}
	}
}

void Room33::look_schublade() {
	if (!_G(gameState).R33SchubFirst) {
		_G(gameState).R33SchubFirst = true;
		hideCur();
		autoMove(1, P_CHEWY);
		_G(atds)->set_ats_str(210, TXT_MARK_NAME, 1, ATS_DATA);
		_G(atds)->set_ats_str(210, TXT_MARK_USE, 1, ATS_DATA);
		showCur();
	}
}

int16 Room33::use_schublade() {
	int16 action_flag = false;

	if (_G(gameState).R33SchubFirst && !_G(cur)->usingInventoryCursor() && !_G(gameState).R33Messer) {
		hideCur();
		action_flag = true;
		_G(gameState).R33Messer = true;

		_G(atds)->set_ats_str(210, TXT_MARK_NAME, 0, ATS_DATA);
		_G(atds)->set_ats_str(210, TXT_MARK_LOOK, 1, ATS_DATA);
		_G(atds)->set_ats_str(210, TXT_MARK_USE, 2, ATS_DATA);
		start_spz(CH_LGET_O, 1, ANI_FRONT, P_CHEWY);
		invent_2_slot(KNIFE_INV);
		showCur();
	}

	return action_flag;
}

void Room33::use_maschine() {
	int16 dia_nr;
	int16 ani_nr;
	hideCur();

	if (!_G(gameState).R33MunterOk) {
		autoMove(4, P_CHEWY);

		bool hocker = false;
		if (_G(cur)->usingInventoryCursor()) {
			bool action = true;

			switch (_G(cur)->getInventoryCursor()) {
			case MILCH_INV:
				_G(gameState).R33Munter[3] = true;
				invent_2_slot(MILCH_LEER_INV);
				break;

			case K_FLEISCH_INV:
				_G(gameState).R33Munter[1] = true;
				break;

			case EIER_INV:
				_G(gameState).R33Munter[2] = true;
				break;

			case KAFFEE_INV:
				_G(gameState).R33Munter[0] = true;
				break;

			default:
				action = false;
				break;

			}

			if (action) {
				_G(gameState)._personHide[P_CHEWY] = true;
				startSetAILWait(0, 1, ANI_FRONT);
				_G(gameState)._personHide[P_CHEWY] = false;
				setPersonPos(128, 65, P_CHEWY, P_LEFT);
				start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
				hocker = true;
				delInventory(_G(cur)->getInventoryCursor());
				ani_nr = CH_TALK12;

				if (calc_muntermacher()) {
					_G(gameState).R33MunterOk = true;
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

		start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
		startAadWait(dia_nr);

		if (_G(gameState).R33MunterOk) {
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(2, 1, ANI_FRONT);
			flic_cut(FCUT_031);
			_G(gameState)._personHide[P_CHEWY] = false;
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			startAadWait(71);
			_G(atds)->delControlBit(225, ATS_ACTIVE_BIT);
		}

		if (hocker) {
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(1, 1, ANI_FRONT);
			setPersonPos(64, 100, P_CHEWY, P_LEFT);
		}

		_G(gameState)._personHide[P_CHEWY] = false;
	} else {
		startAadWait(70);
	}

	showCur();
}

bool Room33::calc_muntermacher() {
	bool ret = true;

	for (int16 i = 0; i < 4; i++) {
		if (_G(gameState).R33Munter[i] == false)
			ret = false;
	}

	return ret;
}

int16 Room33::get_munter() {
	int16 action_flag = false;

	if (!_G(cur)->usingInventoryCursor() && !_G(gameState).R33MunterGet && _G(gameState).R33MunterOk) {
		action_flag = true;
		hideCur();
		_G(gameState).R33MunterGet = true;
		autoMove(4, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		invent_2_slot(MUNTER_INV);
		_G(atds)->setControlBit(225, ATS_ACTIVE_BIT);
		_G(det)->hideStaticSpr(3);
		start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
		startAadWait(72);
		showCur();
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
