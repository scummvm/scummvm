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
#include "chewy/rooms/room43.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room43::night_small() {
	hide_person();
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	switchRoom(43);
	g_engine->_sound->stopMusic();
	_G(det)->playSound(0, 0);
	hideCur();
	_G(flags).NoScroll = true;
	_G(gameState).ScrollxStep = 1;
	startAadWait(188);
	auto_scroll(320, 0);
	startAadWait(189);
	auto_scroll(0, 0);
	startAadWait(190);
	_G(gameState).ScrollxStep = 16;
	auto_scroll(192, 0);

	for (int16 i = 0; i < 3; i++) {
		if (i)
			_G(gameState).ScrollxStep = 16 / i;
		auto_scroll(128 + i * 16, 0);
		auto_scroll(192, 0);
	}

	_G(gameState).scrollx = 194;
	startAadWait(191);
	flic_cut(FCUT_058);
	_G(det)->stopSound(0);
	_G(flags).NoScroll = false;
	_G(gameState).ScrollxStep = 1;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	showCur();
	show_person();
}

void Room43::catch_pg() {
	hide_person();
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	switchRoom(43);
	g_engine->_sound->stopMusic();
	_G(det)->playSound(0, 0);
	hideCur();
	_G(flags).NoScroll = true;
	_G(gameState).ScrollxStep = 1;
	startAadWait(234);
	auto_scroll(194, 0);
	startAadWait(235);
	_G(gameState).R43GetPgLady = true;
	flic_cut(FCUT_058);

	_G(det)->stopSound(0);
	register_cutscene(14);
	
	_G(flags).NoScroll = false;
	_G(gameState).ScrollxStep = 1;
	_G(gameState).scrollx = 0;
	_G(gameState).scrolly = 0;
	showCur();
	show_person();
}

} // namespace Rooms
} // namespace Chewy
