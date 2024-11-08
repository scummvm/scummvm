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
#include "chewy/rooms/room52.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room52::entry() {
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (_G(gameState).R52HotDogOk && !_G(gameState).R52KakerWeg) {
		plot_armee(0);
		_G(det)->playSound(0, 0);
	} else {
		_G(det)->stopSound(0);
	}

	if (_G(gameState).R52KakerWeg)
		_G(det)->stopDetail(0);

	if (!_G(flags).LoadGame) {
		_G(det)->showStaticSpr(4);
		_G(gameState).R52TuerAuf = true;
		setPersonPos(20, 50, P_HOWARD, P_LEFT);
		setPersonPos(35, 74, P_CHEWY, P_RIGHT);
		autoMove(2, P_CHEWY);
		_G(gameState).R52TuerAuf = false;
		_G(det)->hideStaticSpr(4);
		setShadowPalette(2, true);
	}
}

void Room52::xit() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 52) {
		_G(gameState)._personRoomNr[P_HOWARD] = 51;
		_G(spieler_mi)[P_HOWARD].Mode = false;
	}
}

void Room52::gedAction(int index) {
	if (index == 1)
		kaker_platt();
}

int16 Room52::use_hot_dog() {
	int16 action_ret = false;
	hideCur();

	if (isCurInventory(BURGER_INV)) {
		action_ret = true;
		autoMove(3, P_CHEWY);
		start_spz_wait(CH_ROCK_GET1, 1, false, P_CHEWY);
		_G(det)->showStaticSpr(0);
		delInventory(_G(cur)->getInventoryCursor());
		autoMove(4, P_CHEWY);
		_G(gameState).R52HotDogOk = true;
		plot_armee(20);
		_G(det)->playSound(0, 0);
		_G(atds)->set_all_ats_str(341, 1, ATS_DATA);
		autoMove(2, P_CHEWY);
		setPersonSpr(P_LEFT, P_CHEWY);
		startAadWait(288);

	} else if (isCurInventory(KILLER_INV)) {
		action_ret = true;
		autoMove(5, P_CHEWY);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(7, 1, ANI_FRONT);
		_G(det)->playSound(7, 0);
		_G(det)->startDetail(8, 255, ANI_FRONT);

		for (int16 i = 0; i < 5; i++) {
			waitShowScreen(20);
			_G(det)->stopDetail(2 + i);
		}

		_G(det)->stopSound(0);
		_G(det)->stopDetail(0);
		_G(det)->stopDetail(8);
		startSetAILWait(7, 1, ANI_BACK);
		_G(det)->stopSound(0);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(atds)->setControlBit(341, ATS_ACTIVE_BIT);
		startAadWait(303);
		_G(atds)->set_all_ats_str(KILLER_INV, 1, INV_ATS_DATA);
		_G(gameState).R52KakerWeg = true;
	}

	showCur();
	return action_ret;
}

void Room52::plot_armee(int16 frame) {
	for (int16 i = 0; i < 5; i++) {
		waitShowScreen(frame);
		_G(det)->startDetail(2 + i, 255, ANI_FRONT);
	}
}

void Room52::kaker_platt() {
	if (!_G(gameState).R52KakerJoke && _G(gameState).R52HotDogOk && !_G(gameState).R52KakerWeg && !_G(flags).ExitMov) {
		_G(gameState).R52KakerJoke = true;
		stopPerson(P_CHEWY);
		hideCur();
		startAadWait(289);
		showCur();
	}
}

void Room52::setup_func() {
	if (_G(gameState)._personRoomNr[P_HOWARD] == 52) {
		calc_person_look();
		const int16 y = (_G(moveState)[P_CHEWY].Xypos[1] < 97) ? 44 : 87;
		goAutoXy(1, y, P_HOWARD, ANI_GO);
	}
}

} // namespace Rooms
} // namespace Chewy
