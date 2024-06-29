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

#include "objects.h"

Darkseed::Objects::Objects() {
	_objectVar.resize(MAX_OBJECTS);
	_objectRunningCode.resize(MAX_OBJECTS);
	_moveObjectXY.resize(MAX_OBJECTS);
	_moveObjectRoom.resize(MAX_OBJECTS); // The original only allocates 42 entries here but writes 199 in the save file!
	reset();
}

void Darkseed::Objects::reset() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		_objectVar[i] = 0;
		_objectRunningCode[i] = 0;
		_moveObjectXY[i].x = 0; // TODO verify this is the correct reset state for these XY vars.
		_moveObjectXY[i].y = 0;
		_moveObjectRoom[i] = i < 42 ? 0xff : 0; // Hack for weird behaviour in original engine.
	}
	// Initial object state.
	setVar(52, 1);
	setVar(112, 0);
	setVar(62, 0);
}

void Darkseed::Objects::setVar(uint16 varIdx, int16 newValue) {
	if (varIdx >= MAX_OBJECTS) {
		error("setVar: Object Index out of range! %d", varIdx);
	}
	_objectVar[varIdx] = newValue;
}

int16 Darkseed::Objects::getVar(uint16 varIdx) {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}

Common::Point Darkseed::Objects::getMoveObjectPosition(uint8 objIdx) {
	if (objIdx >= MAX_OBJECTS) {
		error("getMoveObjectPosition: Object Index out of range! %d", objIdx);
	}
	return _moveObjectXY[objIdx];
}

int16 &Darkseed::Objects::operator[](uint16 varIdx) {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}
const int16 &Darkseed::Objects::operator[](uint16 varIdx) const {
	if (varIdx >= MAX_OBJECTS) {
		error("getVar: Object Index out of range! %d", varIdx);
	}
	return _objectVar[varIdx];
}

static constexpr uint16 eyeDescriptionsTbl[] = {
	0, 0, 0, 0,
	0, 0, 0, 513,
	0, 421, 0, 521,
	0, 0, 713, 0,
	791, 812, 0, 743,
	0, 0, 661, 166,
	0, 0, 0, 0,
	500, 0, 0, 423,
	425, 427, 418, 667,
	503, 505, 507, 509,
	511, 755, 652, 0,
	728, 0, 0, 43,
	0, 0, 0, 0,
	192, 0, 0, 0,
	0, 893, 845, 0,
	0, 452, 721, 0,
	483, 0, 466, 466,
	466, 0, 0, 705,
	0, 0, 0, 0,
	0, 0, 0, 829,
	552, 0, 0, 0,
	0, 0, 711, 608,
	610, 606, 604, 602,
	600, 598, 596, 594,
	592, 590, 588, 0,
	0, 732, 166, 0,
	0, 0, 0, 0,
	0, 0, 0, 241,
	231, 750, 815, 826,
	838, 794, 797, 806,
	802, 440, 448, 117,
	259, 271, 305, 90,
	161, 136, 376, 398,
	414, 474, 477, 480,
	0, 0, 999, 252,
	0, 0, 170, 182,
	212, 219, 284, 315,
	328, 337, 346, 356,
	515, 526, 533, 0,
	547, 561, 570, 575,
	613, 615, 620, 624,
	636, 638, 641, 643,
	645, 0, 673, 677,
	680, 683, 688, 717,
	726, 746, 0, 759,
	765, 780, 787, 818,
	822, 824, 0, 855,
	862, 0, 880, 887,
	891, 900, 0, 724,
	671, 321, 163
};

int Darkseed::Objects::getEyeDescriptionTosIdx(uint16 objNum) {
	if (objNum >= MAX_OBJECTS) {
		error("getEyeDescriptionTosIdx: Object Index out of range! %d", objNum);
	}
	return eyeDescriptionsTbl[objNum];
}

int Darkseed::Objects::getMoveObjectRoom(uint16 idx) {
	if (idx >= MAX_OBJECTS) {
		error("getMoveObjectRoom: index out of range.");
	}
	return _moveObjectRoom[idx];
}
