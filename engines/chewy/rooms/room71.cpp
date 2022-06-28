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
#include "chewy/rooms/room71.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room71::_state;
int Room71::_delay;

void Room71::entry(int16 eib_nr) {
	g_engine->_sound->playSound(0, 0);
	g_engine->_sound->playSound(0, 1);
	g_engine->_sound->playSound(0);
	g_engine->_sound->playSound(0, 1, false);
	_G(gameState).ScrollxStep = 2;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_HOWARD][0] = 8;
	_G(gameState).ZoomXy[P_HOWARD][1] = 16;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 8;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 16;
	_G(zoom_horizont) = 0;
	_state = 0;
	_delay = 0;
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState).DiaAMov = 4;
	if (!_G(gameState).R71LeopardVined && !_G(gameState).flags28_4) {
		_G(timer_nr)[0] = _G(room)->set_timer(0, 10);
		_G(det)->set_static_ani(0, -1);
	} else if (_G(gameState).R71LeopardVined) {
		_G(det)->startDetail(2, 255, ANI_FRONT);
		_G(det)->showStaticSpr(8);
		_G(det)->hideStaticSpr(2);
		_G(det)->hideStaticSpr(3);
	} else if (_G(gameState).flags28_4) {
		_G(det)->showStaticSpr(5);
		_G(det)->hideStaticSpr(3);
	}

	if (_G(flags).LoadGame)
		return;

	switch (eib_nr) {
	case 110:
		setPersonPos(500, 67, P_CHEWY, P_LEFT);
		setPersonPos(568, 31, P_NICHELLE, P_LEFT);
		setPersonPos(538, 35, P_HOWARD, P_LEFT);
		break;
	case 114:
		setPersonPos(70, 48, P_CHEWY, P_RIGHT);
		setPersonPos(165, 16, P_NICHELLE, P_RIGHT);
		setPersonPos(120, 18, P_HOWARD, P_RIGHT);
		break;
	default:
		break;
	}
}

void Room71::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	if (_G(gameState)._personRoomNr[P_HOWARD] != 71)
		return;

	switch (eib_nr) {
	case 105:
		_G(gameState)._personRoomNr[P_HOWARD] = 74;
		_G(gameState)._personRoomNr[P_NICHELLE] = 74;
		break;
	case 106:
		_G(gameState)._personRoomNr[P_HOWARD] = 73;
		_G(gameState)._personRoomNr[P_NICHELLE] = 73;
	default:
		break;
	}
}

void Room71::setup_func() {
	calc_person_look();

	if (_state) {
		if (_delay)
			--_delay;
		else {
			_delay = (_G(gameState).DelaySpeed + 1) / 2;

			if (_state == 1) {
				_G(det)->setStaticPos(3, 244 + g_engine->_rnd.getRandomNumber(4), -1 * g_engine->_rnd.getRandomNumber(4), false, false);				
			} else {
				_G(det)->setStaticPos(5, 198 + g_engine->_rnd.getRandomNumber(3), -1 * g_engine->_rnd.getRandomNumber(3), false, false);
			}
		}

	} else {
		const int posX = _G(moveState)[P_CHEWY].Xypos[0];

		int howDestX, howDestY, nicDestX, nicDestY;
		if (posX < 40) {
			howDestX = 82;
			howDestY = 18;
			nicDestX = 127;
			nicDestY = 19;
		} else if (posX < 190) {
			howDestX = 229;
			howDestY = 22;
			nicDestX = 268;
			nicDestY = 22;
		} else if (posX < 290) {
			howDestX = 329;
			howDestY = 26;
			nicDestX = 368;
			nicDestY = 29;
		} else if (posX < 420) {
			howDestX = 434;
			howDestY = 36;
			nicDestX = 477;
			nicDestY = 32;
		} else {
			howDestX = 538;
			howDestY = 35;
			nicDestX = 568;
			nicDestY = 31;
		}

		goAutoXy(howDestX, howDestY, P_HOWARD, ANI_GO);
		goAutoXy(nicDestX, nicDestY, P_NICHELLE, ANI_GO);
	}
}

int Room71::proc1() {
	if (_G(cur)->usingInventoryCursors()) {
		_G(flags).NoScroll = false;
		return 0;
	}

	hideCur();
	autoMove(2, P_CHEWY);
	_G(flags).NoScroll = true;
	auto_scroll(256, 0);
	_delay = 0;
	if (_G(menu_item) == CUR_HOWARD) {
		proc4();
		if (_G(gameState).flags28_4) {
			if (_G(gameState).R71Val2)
				proc2();
			else
				_G(gameState).R71Val1 = 1;
		}
	} else if (_G(menu_item) == CUR_NICHELLE) {
		proc3();
		if (_G(gameState).flags28_4) {
			if (_G(gameState).R71Val1)
				proc2();
			else
				_G(gameState).R71Val2 = 1;
		}
	} else {
		_G(gameState)._personHide[P_CHEWY] = true;
		proc5(3);
		_G(gameState)._personHide[P_CHEWY] = false;
	}

	if (!_G(gameState).R71LeopardVined)
		startAadWait(432);

	showCur();
	_G(flags).NoScroll = false;
	return 1;
}

void Room71::proc2() {
	startAadWait(432);
	startAadWait(433);
	_G(SetUpScreenFunc) = nullptr;
	goAutoXy(518, 35, P_HOWARD, ANI_GO);
	goAutoXy(568, 36, P_NICHELLE, ANI_WAIT);
	_G(SetUpScreenFunc) = setup_func;
	_G(flags).NoScroll = true;
	auto_scroll(284, 0);
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(gameState)._personHide[P_NICHELLE] = true;
	_G(gameState)._personHide[P_HOWARD] = true;
	proc5(5);
	_G(det)->hideStaticSpr(2);
	_G(det)->startDetail(5, 255, false);
	auto_scroll(160, 0);
	_G(det)->hideStaticSpr(5);
	startSetAILWait(1, 1, ANI_FRONT);
	_G(det)->startDetail(2, 255, false);
	_G(det)->stop_detail(5);
	_G(det)->showStaticSpr(8);
	_G(atds)->setControlBit(442, ATS_ACTIVE_BIT);
	_G(atds)->setControlBit(446, ATS_ACTIVE_BIT);
	_G(gameState)._personHide[P_HOWARD] = false;
	_G(gameState)._personHide[P_NICHELLE] = false;
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(gameState).R71LeopardVined = true;
	_G(flags).NoScroll = false;
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	startAadWait(434);
}

void Room71::proc3() {
	startAadWait(431);
	_G(SetUpScreenFunc) = nullptr;
	goAutoXy(510, 34, P_NICHELLE, ANI_WAIT);
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(gameState)._personHide[P_NICHELLE] = true;
	proc5(6);
	_G(gameState)._personHide[P_NICHELLE] = false;
	_G(gameState)._personHide[P_CHEWY] = false;
}

void Room71::proc4() {
	startAadWait(430);
	_G(SetUpScreenFunc) = nullptr;
	goAutoXy(518, 35, P_HOWARD, ANI_WAIT);
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(gameState)._personHide[P_HOWARD] = true;
	proc5(4);
	_G(gameState)._personHide[P_HOWARD] = false;
	_G(gameState)._personHide[P_CHEWY] = false;
}

void Room71::proc5(int16 val) {
	_state = 1 + (_G(gameState).flags28_4 ? 1 : 0);
	_G(det)->hideStaticSpr(2);
	startSetAILWait(val, 5, ANI_FRONT);
	_G(det)->showStaticSpr(2);
	_state = 0;
}

int Room71::proc6() {
	if (_G(cur)->usingInventoryCursors())
		return 0;

	hideCur();
	autoMove(3, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->hideStaticSpr(3);
	_G(det)->showStaticSpr(4);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(244, 44, P_CHEWY, ANI_WAIT);
	start_spz_wait(14, 1, false, P_CHEWY);
	_G(det)->hideStaticSpr(4);
	_G(det)->showStaticSpr(5);
	_G(gameState).flags28_4 = true;
	_G(room)->set_timer_status(0, TIMER_STOP);
	_G(det)->del_static_ani(0);
	_G(atds)->setControlBit(443, ATS_ACTIVE_BIT);
	autoMove(3, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	
	showCur();
	return 1;
}

void Room71::proc7() {
	hideCur();
	autoMove(4, P_CHEWY);
	startAadWait(429);
	_G(SetUpScreenFunc) = nullptr;
	setPersonSpr(P_LEFT, P_CHEWY);
	goAutoXy(323, 28, P_NICHELLE, ANI_WAIT);
	_G(flags).NoScroll = true;
	auto_scroll(200, 0);
	_G(gameState)._personHide[P_NICHELLE] = true;
	_G(det)->startDetail(7, 255, false);
	startAadWait(624);
	_G(det)->stop_detail(7);
	_G(gameState)._personHide[P_NICHELLE] = false;
	_G(SetUpScreenFunc) = setup_func;
	_G(flags).NoScroll = false;
	startAadWait(432);

	showCur();
}

} // namespace Rooms
} // namespace Chewy
