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
#include "chewy/rooms/room47.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room47::entry() {
	hide_person();
	setPersonPos(40, 170, P_CHEWY, P_RIGHT);
	_G(SetUpScreenFunc) = set_detail;
	_G(flags).MainInput = false;
}

void Room47::xit() {
	_G(flags).MainInput = true;
	show_person();
	setPersonPos(114, 102, P_CHEWY, P_LEFT);
	_G(spieler_mi)[P_HOWARD].Mode = true;
}

int16 Room47::use_button(int16 txt_nr) {
	int16 k_nr = 0;
	int16 action_ret = false;

	if (!_G(cur)->usingInventoryCursors()) {
		action_ret = true;

		switch (txt_nr) {
		case 286:
			k_nr = 0;
			break;

		case 292:
			k_nr = 1;
			break;

		case 293:
			k_nr = 2;
			break;

		default:
			break;
		}

		g_engine->_sound->playSound(0, 0);
		g_engine->_sound->playSound(0);
		++_G(gameState).R47Schloss[k_nr];

		if (_G(gameState).R47Schloss[k_nr] > 9)
			_G(gameState).R47Schloss[k_nr] = 0;

		if (_G(gameState).R47Schloss[0] == 7 &&
				_G(gameState).R47Schloss[1] == 6 &&
				_G(gameState).R47Schloss[2] == 2)
			_G(gameState).R47SchlossOk = true;
		else
			_G(gameState).R47SchlossOk = false;
	}

	return action_ret;
}

void Room47::set_detail() {
	for (int16 i = 0; i < 10; i++)
		_G(det)->hideStaticSpr(i);

	for (int16 i = 0; i < 3; i++) {
		_G(det)->showStaticSpr(_G(gameState).R47Schloss[i]);
		_G(det)->setStaticPos(_G(gameState).R47Schloss[i], 124 + i * 30, 96 - i, false, true);
		_G(det)->plot_static_details(0, 0, _G(gameState).R47Schloss[i], _G(gameState).R47Schloss[i]);
	}

	for (int16 i = 0; i < 10; i++)
		_G(det)->hideStaticSpr(i);
}

} // namespace Rooms
} // namespace Chewy
