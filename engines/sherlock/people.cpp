/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sherlock/people.h"

namespace Sherlock {

// Characer animation sequences
static const uint8 CHARACTER_SEQUENCES[MAX_HOLMES_SEQUENCE][MAX_FRAME] = {
	{ 29, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Right
	{ 22, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down 
	{ 29, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Left
	{ 15, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up
	{ 42, 1, 2, 3, 4, 5, 0 },			// Goto Stand Right
	{ 47, 1, 2, 3, 4, 5, 0 },			// Goto Stand Down
	{ 42, 1, 2, 3, 4, 5, 0 },			// Goto Stand Left
	{ 36, 1, 0 },						// Goto Stand Up
	{ 8, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up Right
	{ 1, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down Right
	{ 8, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Up Left
	{ 1, 1, 2, 3, 4, 5, 6, 7, 0 },		// Walk Down Left
	{ 37, 1, 2, 3, 4, 5, 0 },			// Goto Stand Up Right
	{ 37, 1, 2, 3, 4, 5, 0 },			// Goto Stand Up Left
	{ 52, 1, 2, 3, 4, 0 },				// Goto Stand Down Right
	{ 52, 1, 2, 3, 4, 0 }				// Goto Stand Down Left
};


People::People(SherlockEngine *vm) : _vm(vm) {
	_walkLoaded = false;
}

People::~People() {
	if (_walkLoaded)
		delete _data[PLAYER]._images;
}

void People::reset() {
	Sprite &p = _data[PLAYER];

	p._description = "Sherlock Holmes!";
	p._type = CHARACTER;
	p._position = Common::Point(10000, 11000);
	p._sequenceNumber = STOP_DOWNRIGHT;
	p._sequences = &CHARACTER_SEQUENCES;
	p._imageFrame = nullptr;
	p._frameNumber = 1;
	p._movement = Common::Point(0, 0);
	p._oldPosition = Common::Point(0, 0);
	p._oldSize = Common::Point(0, 0);
	p._misc = 0;
	p._walkCount = 0;
	p._pickUp = "";
	p._allow = 0;
	p._noShapeSize = Common::Point(0, 0);
	p._goto = Common::Point(0, 0);
	p._status = 0;
}

bool People::loadWalk() {
	if (_walkLoaded) {
		return false;
	} else {
		_data[PLAYER]._images = new ImageFile("walk.vgs");
		_data[PLAYER].setImageFrame();
		_walkLoaded = true;

		return true;
	}
}

} // End of namespace Sherlock
