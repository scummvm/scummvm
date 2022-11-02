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
#include "chewy/rooms/room62.h"

namespace Chewy {
namespace Rooms {

void Room62::entry() {
	_G(SetUpScreenFunc) = setup_func;
	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(gameState).R62LauraTransformation = 0;
	_G(gameState).room_e_obj[91].Attribut = 255;
	if (!_G(gameState).R62FirstEntry) {
		_G(gameState).R62TShow = false;
		hideCur();
		_G(gameState).R62FirstEntry = true;
		setPersonPos(49, 113, P_HOWARD, P_RIGHT);
		_G(det)->startDetail(6, 255, ANI_FRONT);
		startAadWait(345);
		_G(det)->stopDetail(6);
		_G(det)->set_static_ani(5, -1);
		goAutoXy(199, 59, P_HOWARD, ANI_WAIT);
		setPersonSpr(0, P_HOWARD);
		_G(det)->del_static_ani(5);
		_G(det)->startDetail(5, 255, ANI_FRONT);
		startAadWait(346);
		_G(det)->stopDetail(5);
		_G(det)->startDetail(6, 255, ANI_FRONT);
		startAadWait(347);
		_G(det)->stopDetail(6);
		startSetAILWait(7, 1, ANI_FRONT);
		_G(det)->set_static_ani(5, -1);
		_G(det)->startDetail(0, 255, ANI_FRONT);
		startAadWait(348);
		_G(det)->stopDetail(0);
		_G(det)->del_static_ani(0);
		_G(det)->startDetail(1, 1, ANI_FRONT);
		autoMove(0, P_CHEWY);
		_G(spieler_mi)[P_CHEWY].Mode = true;
		goAutoXy(160, 240, P_CHEWY, ANI_WAIT);
		_G(spieler_mi)[P_CHEWY].Mode = false;
		_G(det)->del_static_ani(5);
		startSetAILWait(7, 1, ANI_FRONT);
		showCur();
		_G(gameState).R64Moni1Ani = 3;
		_G(gameState).R64Moni2Ani = 4;
		setPersonPos(187, 43, P_CHEWY, P_RIGHT);
		_G(gameState).R62TShow = true;
		switchRoom(64);
	} else {
		_G(det)->del_static_ani(0);
		_G(gameState)._personHide[P_HOWARD] = true;
		_G(det)->set_static_ani(4, -1);
		_G(det)->set_static_ani(8, -1);
		_G(r62Delay) = 0;
		_G(r62TalkAni) = 8;

	}
}

void Room62::setup_func() {
	if (_G(r62Delay) <= 0 && _G(gameState).R62TShow) {
		_G(r62Delay) = (_G(gameState).DelaySpeed + 1) * 60;
		_G(det)->stopDetail(_G(r62TalkAni));
		if (_G(r62TalkAni) == 4)
			_G(r62TalkAni) = 8;
		else
			_G(r62TalkAni) = 4;
		_G(det)->startDetail(_G(r62TalkAni), 255, ANI_FRONT);
	} else
		--_G(r62Delay);
}

int16 Room62::use_laura() {
	int16 action_ret = false;
	if (isCurInventory(GERAET_INV)) {
		action_ret = true;
		hideCur();
		autoMove(2, P_CHEWY);
		auto_scroll(0, 0);
		_G(SetUpScreenFunc) = nullptr;
		_G(det)->del_static_ani(8);
		_G(det)->stopDetail(8);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(det)->startDetail(2, 255, ANI_FRONT);
		_G(det)->startDetail(6, 255, ANI_FRONT);
		startAadWait(399);
		_G(gameState)._personHide[P_CHEWY] = false;
		flic_cut(FCUT_077);
		showCur();
		_G(gameState).R64Moni1Ani = 0;
		_G(gameState).R64Moni2Ani = 0;
		_G(gameState).R62TShow = false;
		_G(gameState).R62LauraTransformation = true;
		switchRoom(63);
	}
	return action_ret;
}

} // namespace Rooms
} // namespace Chewy
