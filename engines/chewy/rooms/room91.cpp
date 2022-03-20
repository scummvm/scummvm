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
#include "chewy/rooms/room91.h"

namespace Chewy {
namespace Rooms {

int16 Room91::_click;

void Room91::entry() {
	_G(zoom_horizont) = 110;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).ScrollxStep = 2;
	_G(SetUpScreenFunc) = setup_func;
	_click = 0;
	_G(gameState).ZoomXy[P_HOWARD][0] = _G(gameState).ZoomXy[P_HOWARD][1] = 30;
	_G(spieler_mi)[P_HOWARD].Mode = true;

	if (_G(flags).LoadGame)
		return;

	_G(gameState).scrollx = 320;
	hideCur();

	if (_G(gameState).flags34_1) {
		setPersonPos(499, 106, P_CHEWY, P_RIGHT);
		setPersonPos(536, 90, P_HOWARD, P_RIGHT);
		if (!_G(gameState).flags34_2) {
			_G(gameState).flags34_2 = true;
			startAadWait(503);
		}
	} else {
		_G(flags).MainInput = false;
		_G(flags).NoScroll = true;
		_G(gameState).flags34_1 = true;
		setPersonPos(326, 99, P_CHEWY, P_RIGHT);
		setPersonPos(312, 75, P_HOWARD, P_RIGHT);
		_G(gameState).SVal3 = 0;
		_G(gameState).flags34_4 = true;
		startAadWait(502);
		_G(gameState)._personHide[P_CHEWY] = true;
		_G(det)->startDetail(0, 255, false);

		for (int i = 0; i < 3; ++i) {
			_G(timer_nr)[i] = _G(room)->set_timer(5 + i, 3 + (2 * i));
		}
		_G(det)->startDetail(5, 2, false);
	}

	showCur();
}

void Room91::xit(int16 eib_nr) {
	_G(flags).MainInput = true;
	_G(gameState).ScrollxStep = 1;
	_G(gameState).scrollx = 0;

	if (eib_nr == 136)
		_G(gameState)._personRoomNr[P_HOWARD] = 90;

	_G(gameState).flags34_4 = false;
	_G(flags).NoScroll = false;
}

void Room91::setup_func() {
	if (!_G(gameState).flags34_4) {
		calc_person_look();
		const int xyPos = _G(spieler_vector)[0].Xypos[0];
		if (xyPos < 130)
			goAutoXy(40, 97, P_HOWARD, ANI_GO);
		else if (xyPos < 312)
			goAutoXy(221, 94, P_HOWARD, ANI_GO);
		else if (xyPos < 445)
			goAutoXy(342, 93, P_HOWARD, ANI_GO);
		else
			goAutoXy(536, 90, P_HOWARD, ANI_GO);
	} else {
		if (_G(menu_display))
			return;
		
		_G(menu_item) = CUR_USE;
		cur_2_inventory();
		cursorChoice(CUR_22);

		if (_G(mouseLeftClick) == 0 || _click)
			return;

		const int oldClick = _click;
		_G(mouseLeftClick) = oldClick;
		_click = 1;
		const int aniNr = 1 + (g_events->_mousePos.y <= 100 ? 1 : 0);
		hideCur();
		_G(det)->stop_detail(0);
		startSetAILWait(aniNr, 1, ANI_FRONT);
		_click = oldClick;
		_G(det)->startDetail(0, 255, false);
		_G(det)->startDetail(aniNr + 2, 1, false);
		_G(det)->startDetail(7, 1, false);
		start_spz_wait(62, 1, false, P_HOWARD);
		_G(gameState).SVal3 += 1;
		showCur();
		if (_G(gameState).SVal3 == 4) {
			_G(det)->stop_detail(0);
			_G(gameState)._personHide[P_CHEWY] = false;
			hideCur();
			autoMove(1, P_CHEWY);
			stop_spz();
			startAadWait(505);
			_G(gameState)._personHide[P_HOWARD] = true;
			startSetAILWait(9, 1, ANI_FRONT);
			_G(gameState)._personHide[P_HOWARD] = false;
			_G(gameState)._personRoomNr[P_HOWARD] = 50;
			_G(gameState).flags34_4 = false;
			showCur();
			switchRoom(50);
		}
	}
}

} // namespace Rooms
} // namespace Chewy
