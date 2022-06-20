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
#include "chewy/room.h"
#include "chewy/rooms/room14.h"
#include "chewy/rooms/room23.h"

namespace Chewy {
namespace Rooms {

void Room14::entry() {
	_G(gameState).R23GleiterExit = 14;
	_G(zoom_horizont) = 310;
	_G(gameState).ScrollxStep = 2;

	if (!_G(gameState).R14Feuer)
		_G(obj)->hide_sib(46);

	if (!_G(flags).LoadGame) {
		if (_G(gameState).R14GleiterAuf) {
			setPersonPos(381, 264, P_CHEWY, P_LEFT);
			_G(det)->showStaticSpr(6);
			_G(gameState).scrollx = 160;
			_G(gameState).scrolly = 120;
		} else {
			_G(gameState).scrollx = 92;
			_G(gameState).scrolly = 17;
		}
	}
}

bool Room14::timer(int16 t_nr, int16 ani_nr) {
	if (ani_nr) 
		eremit_feuer(t_nr, ani_nr);

	return false;
}

void Room14::eremit_feuer(int16 t_nr, int16 ani_nr) {
	if (!_G(flags).AutoAniPlay && !_G(gameState).R14Feuer) {
		_G(flags).AutoAniPlay = true;
		_G(det)->hideStaticSpr(9);
		startSetAILWait(_G(room)->_roomTimer._objNr[ani_nr], 1, ANI_FRONT);
		_G(uhr)->resetTimer(t_nr, 0);
		_G(det)->showStaticSpr(9);
		_G(det)->startDetail(7, 1, ANI_FRONT);
		_G(flags).AutoAniPlay = false;
	}
}

int16 Room14::use_schrott() {
	int16 action_flag = false;

	if (!_G(gameState).inv_cur) {
		autoMove(3, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(12, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;

		if (!_G(gameState).R14Waffe) {
			action_flag = true;
			_G(gameState).R14Waffe = true;
			startAadWait(21);
			invent_2_slot(BWAFFE_INV);
		}
	}
	return action_flag;
}

int16 Room14::use_gleiter() {
	int16 action_flag = false;

	if (!_G(gameState).inv_cur) {
		action_flag = true;
		autoMove(4, P_CHEWY);

		if (!_G(gameState).R14GleiterAuf) {
			_G(gameState).R14GleiterAuf = true;
			_G(gameState)._personHide[P_CHEWY] = true;
			startSetAILWait(10, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = false;
			_G(det)->showStaticSpr(6);
			_G(atds)->set_ats_str(107, TXT_MARK_LOOK, 1, ATS_DATA);
		} else {
			_G(gameState).R23GleiterExit = 14;
			Room23::cockpit();
		}
	}

	return action_flag;
}

void Room14::talk_eremit()  {
	if (!_G(gameState).R14Feuer) {
		autoMove(6, P_CHEWY);
		_G(flags).AutoAniPlay = true;

		if (_G(gameState).R14Translator) {
			loadDialogCloseup(0);
			_G(obj)->show_sib(46);
		} else {
			hideCur();
			startAadWait(24);
			showCur();
			_G(flags).AutoAniPlay = false;
		}
	}
}

int16 Room14::use_schleim() {
	int16 action_flag = false;

	if (!_G(gameState).inv_cur) {
		autoMove(2, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(11, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;

		if (!_G(gameState).R14Sicherung) {
			action_flag = true;
			_G(gameState).R14Sicherung = true;
			startAadWait(22);
			invent_2_slot(SICHERUNG_INV);
		}
	}

	return action_flag;
}

void Room14::feuer() {
	int16 waffe = false;
	int16 tmp = _G(gameState).AkInvent;
	_G(gameState).R14Feuer = true;
	_G(cur_hide_flag) = false;
	_G(flags).AutoAniPlay = true;
	hideCur();

	if (isCurInventory(BWAFFE_INV)) {
		autoMove(5, P_CHEWY);
		waffe = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		startDetailFrame(8, 1, ANI_FRONT, 11);
		startSetAILWait(9, 1, ANI_FRONT);
		waitDetail(8);
		_G(gameState)._personHide[P_CHEWY] = false;
	} else {
		autoMove(7, P_CHEWY);
		_G(det)->hideStaticSpr(9);
		startDetailFrame(2, 1, ANI_FRONT, 9);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(13, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		waitDetail(2);
		startSetAILWait(5, 1, ANI_FRONT);
		_G(det)->showStaticSpr(9);
	}

	delInventory(tmp);
	_G(det)->startDetail(6, 255, ANI_FRONT);
	waitShowScreen(40);

	if (waffe)
		autoMove(7, P_CHEWY);

	startAadWait(26);
	_G(det)->hideStaticSpr(9);
	startSetAILWait(3, 1, ANI_FRONT);
	_G(det)->showStaticSpr(9);
	invent_2_slot(FLUXO_INV);
	_G(atds)->set_ats_str(105, TXT_MARK_LOOK, 1, ATS_DATA);
	_G(gameState).R14FluxoFlex = true;
	_G(flags).AutoAniPlay = false;
	showCur();
}

} // namespace Rooms
} // namespace Chewy
