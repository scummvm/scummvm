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
#include "chewy/rooms/room57.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room57::entry() {
	_G(zoom_horizont) = 180;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 4;
	_G(SetUpScreenFunc) = setup_func;
	_G(gameState).ZoomXy[P_HOWARD][0] = 46;
	_G(gameState).ZoomXy[P_HOWARD][1] = 86;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	if (_G(gameState).R57StudioAuf)
		_G(det)->hideStaticSpr(4);
	
	if (!_G(flags).LoadGame &&_G(gameState).R48TaxiEntry) {
		hideCur();
		_G(gameState).R48TaxiEntry = false;
		_G(gameState).scrollx = 0;
		_G(gameState).scrolly = 0;
		setPersonPos(4, 144, P_HOWARD, P_LEFT);
		setPersonPos(40, 160, P_CHEWY, P_RIGHT);
		autoMove(2, P_CHEWY);
		_G(mouseLeftClick) = false;
		showCur();
	}
}

void Room57::xit(int16 eib_nr) {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 57) {
		_G(spieler_mi)[P_HOWARD].Mode = false;
		if (eib_nr == 91)
			_G(gameState)._personRoomNr[P_HOWARD] = 62;
	}
}

void Room57::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 57) {
		calc_person_look();
		const int16 ch_y = _G(moveState)[P_CHEWY].Xypos[1];
		int16 x, y;
		if (ch_y < 145) {
			x = 176;
			y = 126;
		} else {
			x = 176;
			y = 142;
		}
		goAutoXy(x, y, P_HOWARD, ANI_GO);
	}
}

int16 Room57::use_taxi() {
	int16 action_ret = false;
	if (!_G(cur)->usingInventoryCursor()) {
		action_ret = true;
		hideCur();
		autoMove(3, P_CHEWY);
		g_engine->_sound->playSound(3, 0);
		g_engine->_sound->playSound(3);
		_G(det)->showStaticSpr(7);
		goAutoXy(16, 160, P_CHEWY, ANI_WAIT);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(gameState).R48TaxiPerson[P_CHEWY] = true;
		if (_G(gameState)._personRoomNr[P_HOWARD] == 57) {
			_G(SetUpScreenFunc) = nullptr;
			goAutoXy(11, 144, P_HOWARD, ANI_WAIT);
			_G(gameState)._personHide[P_HOWARD] = true;
			_G(gameState).R48TaxiPerson[P_HOWARD] = true;
			_G(gameState)._personRoomNr[P_HOWARD] = 48;
		}
		_G(det)->hideStaticSpr(7);
		g_engine->_sound->playSound(3, 1);
		g_engine->_sound->playSound(3);
		_G(room)->set_timer_status(3, TIMER_STOP);
		_G(det)->del_static_ani(3);
		startSetAILWait(5, 1, ANI_FRONT);
		g_engine->_sound->stopSound(0);
		switchRoom(48);
	}
	return action_ret;
}

int16 Room57::use_pfoertner() {
	int16 action_ret = false;
	_G(room)->set_timer_status(1, TIMER_STOP);
	_G(det)->del_static_ani(1);
	_G(det)->set_static_ani(3, -1);
	hideCur();
	autoMove(1, P_CHEWY);
	if (isCurInventory(CUTMAG_INV)) {
		action_ret = true;
		if (_G(gameState).flags37_10)
			startAadWait(596);
		else {
			_G(gameState).flags37_10 = true;
			startAadWait(339);
			new_invent_2_cur(BESTELL_INV);
		}
	} else if (isCurInventory(JMKOST_INV)) {
		action_ret = true;
		startAadWait(340);
	} else if (isCurInventory(EINLAD_INV)) {
		action_ret = true;
		_G(SetUpScreenFunc) = nullptr;
		goAutoXy(132, 130, P_HOWARD, ANI_WAIT);
		if (_G(gameState).R56AbfahrtOk) {
			startAadWait(341);
			goAutoXy(176, 130, P_HOWARD, ANI_WAIT);
			delInventory(_G(cur)->getInventoryCursor());
			_G(gameState).R57StudioAuf = true;
			_G(gameState).room_e_obj[91].Attribut = EXIT_TOP;
			_G(det)->hideStaticSpr(4);
			startSetAILWait(6, 1, ANI_WAIT);
			g_engine->_sound->stopSound(0);
			_G(atds)->setControlBit(358, ATS_ACTIVE_BIT);
		} else {
			startAadWait(349);
			goAutoXy(176, 130, P_HOWARD, ANI_WAIT);
		}
		_G(SetUpScreenFunc) = setup_func;
	}
	showCur();
	_G(room)->set_timer_status(1, TIMER_START);
	_G(det)->set_static_ani(1, -1);
	return action_ret;
}

void Room57::talk_pfoertner() {
	hideCur();
	autoMove(1, P_CHEWY);
	_G(room)->set_timer_status(1, TIMER_STOP);
	_G(det)->del_static_ani(1);
	_G(det)->set_static_ani(3, -1);
	int16 aad_nr;
	if (!_G(gameState).R57StudioAuf) {
		aad_nr = 338;
	} else
		aad_nr = 342;
	startAadWait(aad_nr);
	_G(room)->set_timer_status(1, TIMER_START);
	_G(det)->set_static_ani(1, -1);
	showCur();
}

} // namespace Rooms
} // namespace Chewy
