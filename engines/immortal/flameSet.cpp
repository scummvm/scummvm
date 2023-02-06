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

void Room::flameInit() {
	flameFreeAll();
	_candleTmp = 0;
}

void Room::flameFreeAll() {
	_numFlames = 0;
	_numInRoom = 0;
}

void Room::flameDrawAll(uint16 vX, uint16 vY) {
	for (uint i = 0; i < _fset.size(); i++) {
		// For every flame in the room, add the sprite to the sprite table
		univAddSprite(vX, vY, _fset[i]._x, _fset[i]._y, g_immortal->_cycPtrs[g_immortal->_cycles[_fset[i]._c]._cycList]._sName, cycleGetFrame(_fset[i]._c), 0);
		if (cycleAdvance(_fset[i]._c) == true) {
			cycleFree(_fset[i]._c);
			_fset[i]._c = flameGetCyc(&_fset[i], 1);
		}
	}
}

bool Room::roomLighted() {
	// For testing purposes, we say it's always lit
	return true;

	// Very simple, just checks every torch and if any of them are lit, we say the room is lit
	for (uint i = 0; i < _fset.size(); i++) {
		if (_fset[i]._p != kFlameOff) {
			return true;
		}
	}
	return false;
}

void Room::lightTorch(uint8 x, uint8 y) {
	/* Checks every torch to see if it is:
	 * pattern == off, and inside the point x,y
	 * which is the fireball position. This is a
	 * little bit clever, because it saves cycles
	 * over checking x,y first, since you need to
	 * check both x,y and flame pattern. Neato.
	 */

	for (uint i = 0; i < _fset.size(); i++) {
		if (_fset[i]._p == kFlameOff) {
			if (Utilities::inside(kLightTorchX, x, y, _fset[i]._x + 16, _fset[i]._y + 8)) {
				_fset[i]._p = kFlameNormal;

			}
		}
	}
}

void Room::flameSetRoom(Common::Array<SFlame> &allFlames) {
	for (uint i = 0; i < allFlames.size(); i++) {
		Flame f;
		f._p = allFlames[i]._p;
		f._x = allFlames[i]._x;
		f._y = allFlames[i]._y;
		f._c = flameGetCyc(&f, (0 | _candleTmp));
		//debug("made flame, cyc = %d", f._c);
		_fset.push_back(f);
	}
	_candleTmp = 1;
}

int Room::flameGetCyc(Flame *f, int first) {
	/* I must say, although this is clever, it is the most
	 * convoluted way to do this I could imagine. Here's what it does:
	 * Get a random number between 0 and 255. Now reduce this number by the length
	 * of the array for the particular flame pattern until we are at less than 0.
	 * Now add back that same length. We are now at the length of the array
	 * minus a random amount between 0 and the length of the array.
	 * This gives us a random entry within the array to start at.
	 */
	CycID flamePatA[] = {kCycFNormal0, kCycFNormal1, kCycFNormal2,
	                     kCycFNormal0, kCycFNormal1, kCycFNormal2,
	                     kCycFNormal0, kCycFNormal1, kCycFNormal2,
	                     kCycFNormal0, kCycFNormal1, kCycFNormal2
	                    };
	CycID flamePatB[] = {kCycFCandleBurst,   kCycFCandleSway,    kCycFCandleJump,
	                     kCycFCandleLeap,    kCycFCandleFlicker,
	                     kCycFCandleFlicker, kCycFCandleFlicker, kCycFCandleFlicker
	                    };
	CycID flamePatC[] = {kCycFOff};
	CycID flamePatD[] = {kCycFFlicker0, kCycFFlicker1, kCycFFlicker2};

	int numFlameCycs[] = {12, 8, 1, 3};

	int r = getRandomNumber(255) & (kMaxFlameCycs - 1);

	do {
		r -= numFlameCycs[(int) f->_p];
	} while (r >= 0);

	r += numFlameCycs[(int) f->_p];

	// Why is this not indexed further? ie. LDA patternTable,x : STA $00 : LDA ($00),y instead of a branch tree?
	// Pretty sure CPX 3 times is more than a single LDA (dp),y
	switch (f->_p) {
	case 0:
		return cycleNew(flamePatA[r]);
	case 1:
		return cycleNew(flamePatB[r]);
	case 2:
		return cycleNew(flamePatC[r]);
	case 3:
		return cycleNew(flamePatD[r]);
	default:
		return 0;
	}
}

} // namespace Immortal
