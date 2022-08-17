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

/* [Alternate Name: Torch Processing]
 * --- What is a FlameSet ---
 * A FlameSet is a list of torches in a given room.
 * The level has X amount of flames total, and each
 * room has N amount of torches, making up a FlameSet.
 * There are 3 instances of torches in the game.
 * First is in the story record, second is in the level torches,
 * and lastly is the in-room torch. This is done so that
 * the in-room torch can be lit up by a fireball and then
 * stay permanantly lit for the next time the player enters the room.
 */

#include "immortal/room.h"

namespace Immortal {

void Room::flameSetRoom(Common::Array<SFlame> allFlames) {
	for (int i = 0; i < allFlames.size(); i++) {
		Flame f;
		f._p = allFlames[i]._p;
		f._x = allFlames[i]._x;
		f._y = allFlames[i]._y;
		f._c = flameGetCyc(0 | _candleTmp);
		_candleTmp = 1;
		_fset.push_back(f);
	}
}

void Room::flameDrawAll() {

}

bool Room::roomLighted() {
	// Very simple, just checks every torch and if any of them are lit, we say the room is lit
	for (int i = 0; i < _fset.size(); i++) {
		if (_fset[i]._p != kFlameOff) {
			return true;
		}
	}
	return false;
}

void Room::lightTorch(int x, int y) {
	/* Checks every torch to see if it is:
	 * pattern == off, and inside the point x,y
	 * which is the fireball position. This is a
	 * little bit clever, because it saves cycles
	 * over checking x,y first, since you need to
	 * check both x,y and flame pattern. Neato.
	 */

	for (int i = 0; i < _fset.size(); i++) {
		if (_fset[i]._p == kFlameOff) {
			if (Immortal::Util::inside(x, y, kLightTorchX, _fset[i]._x + 16, _fset[i]._y + 8)) {
				_fset[i]._p = kFlameNormal;

			}
		}
	}
}

Cyc Room::flameGetCyc(int first) {
	return kCycNone;
}

} // namespace immortal





















