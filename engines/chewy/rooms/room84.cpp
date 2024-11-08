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
#include "chewy/rooms/room84.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

bool Room84::_flag;

void Room84::entry() {
	_G(gameState).ScrollxStep = 2;
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).R84GoonsPresent = !_G(gameState).R88UsedMonkey;
	_flag = false;
	_G(det)->playSound(0, 0);

	if (_G(gameState).R88UsedMonkey) {
		_G(det)->del_static_ani(3);
		_G(det)->stopDetail(0);
		_G(gameState).room_e_obj[124].Attribut = EXIT_TOP;
		_G(atds)->setControlBit(478, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(479, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(485, 1, ATS_DATA);

		_G(det)->showStaticSpr(7);
		_G(atds)->delControlBit(481, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(482, ATS_ACTIVE_BIT);
	}

	if (_G(gameState).flags32_10) {
		_G(det)->startDetail(7, 255, false);
		_G(atds)->delControlBit(504, ATS_ACTIVE_BIT);
		_G(gameState).room_e_obj[124].Attribut = 255;
		_G(atds)->set_all_ats_str(485, 2, ATS_DATA);
	}

	if (_G(gameState).flags32_40) {
		_G(atds)->setControlBit(481, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(482, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(485, 1, ATS_DATA);		
		_G(gameState).room_e_obj[124].Attribut = EXIT_TOP;
	}

	if (_G(flags).LoadGame) {
		; // Nothing. It avoids a deeper level of if, and we need the _G(SetUpScreenFunc) at the end
	} else if (_G(gameState).flags30_1 || _G(gameState).flags31_8) {
		setPersonPos(569, 135, P_CHEWY, P_LEFT);
		setPersonPos(489, 113, P_HOWARD, P_RIGHT);
		setPersonPos(523, 110, P_NICHELLE, P_RIGHT);
		_G(gameState).flags30_1 = false;
		_G(gameState).flags31_8 = false;
		_G(mouseLeftClick) = false;
		_G(gameState).scrollx = 319;

		if (_G(gameState).flags32_10) {
			setPersonPos(347, 130, P_CHEWY, P_RIGHT);
			setPersonPos(408, 113, P_HOWARD, P_LEFT);
			_G(gameState).scrollx = 250;
			if (_G(gameState).flags32_40) {
				startAadWait(481);
				_G(gameState).room_e_obj[124].Attribut = EXIT_TOP;
			}
		}
	} else if (_G(gameState).flags32_20) {
		hideCur();
		setPersonPos(347, 130, P_CHEWY, P_RIGHT);
		setPersonPos(408, 113, P_HOWARD, P_LEFT);
		_G(gameState).scrollx = 250;
		_G(gameState).flags32_20 = false;
		_G(gameState).room_e_obj[124].Attribut = 255;
		_G(gameState).room_e_obj[125].Attribut = 255;
		_G(atds)->set_all_ats_str(485, 2, ATS_DATA);
		startAadWait(477);
		showCur();
	} else {
		setPersonPos(263, 136, P_CHEWY, P_LEFT);
		setPersonPos(238, 113, P_HOWARD, P_RIGHT);
		setPersonPos(294, 110, P_NICHELLE, P_RIGHT);
		_G(gameState).flags30_1 = false;
		_G(gameState).scrollx = 156;
	}
	
	_G(SetUpScreenFunc) = setup_func;
}

void Room84::xit(int16 eib_nr) {
	_G(gameState).ScrollxStep = 1;
	if (eib_nr == 125) {
		_G(gameState).R79Val[P_CHEWY] = 1;
		_G(gameState).R79Val[P_HOWARD] = 1;
		_G(gameState).R79Val[P_NICHELLE] = 1;
		_G(gameState)._personRoomNr[P_HOWARD] = 80;
		_G(gameState)._personRoomNr[P_NICHELLE] = 80;
	} else {
		if (_G(gameState)._personRoomNr[P_HOWARD] == 84)
			_G(gameState)._personRoomNr[P_HOWARD] = 85;
		if (_G(gameState)._personRoomNr[P_NICHELLE] == 84)
			_G(gameState)._personRoomNr[P_NICHELLE] = 85;
	}
}

void Room84::setup_func() {
	calc_person_look();

	if (_G(gameState).R84GoonsPresent && !_flag) {
		_flag = true;
		_G(gameState).flags30_80 = true;
		hideCur();
		autoMove(4, P_CHEWY);
		_G(flags).NoScroll = true;
		setPersonSpr(P_LEFT, P_CHEWY);
		auto_scroll(150, 0);
		_G(det)->del_static_ani(3);
		_G(det)->set_static_ani(4, -1);
		startAadWait(455);
		_G(det)->del_static_ani(4);
		start_spz(62, 1, false, P_HOWARD);
		startSetAILWait(5, 1, ANI_FRONT);
		_G(det)->set_static_ani(3, -1);
		startAadWait(456);
		_G(flags).NoScroll = false;
		showCur();
	}

	const int posX = _G(moveState)[P_CHEWY].Xypos[0];
	int howDestX, nicDestX;
	if (posX > 610) {
		howDestX = nicDestX = 610;
	} else if (posX < 280) {
		howDestX = 238;
		nicDestX = 294;
	} else if (posX < 380) {
		howDestX = 408;
		nicDestX = 454;
	} else {
		howDestX = 489;
		nicDestX = 523;
	}

	goAutoXy(howDestX, 113, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, 110, P_NICHELLE, ANI_GO);
}

void Room84::talk1() {
	autoMove(4, P_CHEWY);
	_G(flags).NoScroll = true;
	setPersonSpr(P_LEFT, P_CHEWY);
	auto_scroll(150, 0);
	startDialogCloseupWait(22);
	_G(flags).NoScroll = false;
}

void Room84::talk2() {
	startAadWait(478);
}

int Room84::proc4() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(5, P_CHEWY);

	if (_G(gameState).flags32_10) {
		_G(det)->stopDetail(7);
		startSetAILWait(8, 1, ANI_FRONT);
	} else {
		_G(gameState)._personRoomNr[P_NICHELLE] = 88;
	}

	_G(gameState)._personRoomNr[P_HOWARD] = 88;
	switchRoom(88);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
	showCur();

	return 1;
}

} // namespace Rooms
} // namespace Chewy
