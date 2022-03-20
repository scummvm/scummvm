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
#include "chewy/rooms/room93.h"

namespace Chewy {
namespace Rooms {

void Room93::entry() {
	if (_G(flags).LoadGame)
		return;

	_G(gameState).scrollx = 0;
	hide_person();
	startSetAILWait(3, 0, ANI_GO);
	_G(det)->set_static_ani(0, -1);
	startAadWait(616);
	_G(det)->del_static_ani(0);
	startSetAILWait(3, 1, ANI_FRONT);
	_G(det)->set_static_ani(1, -1);
	startAdsWait(27);

	if (!_G(gameState).flags37_40) {
		_G(det)->del_static_ani(1);
		hideCur();
		startSetAILWait(3, 1, ANI_GO);
		_G(det)->set_static_ani(0, -1);
		startAadWait(549);
		_G(det)->del_static_ani(0);
		startSetAILWait(3, 1, ANI_FRONT);
		startSetAILWait(6, 1, ANI_FRONT);
		_G(det)->set_static_ani(7, -1);
		startAadWait(550);
		_G(det)->del_static_ani(7);
		startSetAILWait(6, 1, ANI_GO);
		startSetAILWait(2, 1, ANI_FRONT);
		setupScreen(DO_SETUP);
		showCur();
	}

	_G(gameState).flags35_40 = true;
	show_person();
	switchRoom(94);
}

void Room93::xit() {
	_G(gameState)._personRoomNr[P_HOWARD] = 94;
	_G(gameState).scrollx = _G(gameState).r94Scrollx;
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);
}

} // namespace Rooms
} // namespace Chewy
