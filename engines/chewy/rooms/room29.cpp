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
#include "chewy/rooms/room29.h"

namespace Chewy {
namespace Rooms {

static const AniBlock ABLOCK26[3] = {
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 0, 1, ANI_BACK, ANI_WAIT, 0 },
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK27[5] = {
	{ 0, 1, ANI_BACK, ANI_WAIT, 0 },
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 0, 1, ANI_BACK, ANI_WAIT, 0 },
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 0, 1, ANI_BACK, ANI_WAIT, 0 },
};


void Room29::entry() {
	if (_G(gameState).R29Schlauch1) {
		_G(det)->showStaticSpr(7);
	} else if (_G(gameState).R29WaterHose) {
		_G(det)->showStaticSpr(8);
		_G(det)->showStaticSpr(10);
	}

	if (_G(gameState).R29AutoSitz) {
		if (_G(gameState).ChewyAni == CHEWY_ROCKER)
			_G(atds)->setControlBit(212, ATS_ACTIVE_BIT);
		else
			_G(det)->showStaticSpr(9);
	}

	_G(gameState).ScrollxStep = 2;
}

void Room29::xit() {
	if (_G(obj)->checkInventory(PUMPE_INV))
		delInventory(PUMPE_INV);
}

int16 Room29::use_pumpe() {
	int16 action_flag = false;

	if (!_G(gameState).R29Pumpe) {
		hideCur();

		if (isCurInventory(SCHLAUCH_INV)) {
			action_flag = true;
			_G(gameState).R29Pumpe = true;
			_G(gameState).R29Schlauch1 = true;
			autoMove(1, P_CHEWY);
			start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
			_G(det)->showStaticSpr(7);
			_G(atds)->delControlBit(218, ATS_ACTIVE_BIT);
			delInventory(SCHLAUCH_INV);
		} else if (!_G(cur)->usingInventoryCursor()) {
			action_flag = true;
			startAadWait(62);
		}

		showCur();
	}

	return action_flag;
}

int16 Room29::getWaterHose() {
	int16 action_flag = false;
	if (_G(gameState).R29Schlauch1 && !_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		hideCur();

		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		new_invent_2_cur(PUMPE_INV);

		showCur();
	}
	return action_flag;
}

bool Room29::useWaterHose() {
	bool result = false;

	if (isCurInventory(PUMPE_INV)) {
		result = true;
		hideCur();

		autoMove(2, P_CHEWY);
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		_G(det)->hideStaticSpr(7);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(4, 1, ANI_FRONT);
		_G(det)->showStaticSpr(8);
		_G(det)->showStaticSpr(10);
		_G(atds)->delControlBit(219, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(218, 1, ATS_DATA);

		_G(gameState).R29Schlauch1 = false;
		_G(gameState).R29WaterHose = true;
		delInventory(PUMPE_INV);
		setPersonPos(308, 105, P_CHEWY, P_RIGHT);
		_G(gameState)._personHide[P_CHEWY] = false;
		showCur();
	}

	return result;
}

void Room29::schlitz_sitz() {
	if (!_G(gameState).R29AutoSitz) {
		hideCur();
		_G(gameState).R29AutoSitz = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(det)->hideStaticSpr(4);
		_G(det)->showStaticSpr(11);
		startAniBlock(3, ABLOCK26);
		_G(det)->showStaticSpr(9);
		_G(det)->hideStaticSpr(11);
		_G(det)->startDetail(2, 255, ANI_FRONT);
		startAadWait(63);

		_G(det)->stopDetail(2);
		_G(atds)->delControlBit(212, ATS_ACTIVE_BIT);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(mouseLeftClick) = false;
		g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;

		showCur();
	}
}

int16 Room29::zaun_sprung() {
	int16 action_flag = false;

	if (_G(gameState).R29AutoSitz && !_G(cur)->usingInventoryCursor()) {
		hideCur();
		
		action_flag = true;
		autoMove(3, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startDetailFrame(3, 1, ANI_FRONT, 7);
		_G(det)->hideStaticSpr(9);
		startAniBlock(5, ABLOCK27);
		setupScreen(DO_SETUP);

		showCur();
		switchRoom(37);
		_G(gameState)._personHide[P_CHEWY] = false;

	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
