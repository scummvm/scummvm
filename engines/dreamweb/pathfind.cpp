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

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamBase::turnPathOn(uint8 param) {
	findOrMake(param, 0xff, data.byte(kRoomnum) + 100);
	PathNode *roomsPaths = getRoomsPaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0xff;
}

void DreamBase::turnPathOff(uint8 param) {
	findOrMake(param, 0x00, data.byte(kRoomnum) + 100);
	PathNode *roomsPaths = getRoomsPaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0x00;
}

void DreamBase::turnAnyPathOn(uint8 param, uint8 room) {
	findOrMake(param, 0xff, room + 100);
	PathNode *paths = (PathNode *)getSegment(data.word(kReels)).ptr(kPathdata + 144 * room, 0);
	paths[param].on = 0xff;
}

void DreamBase::turnAnyPathOff(uint8 param, uint8 room) {
	findOrMake(param, 0x00, room + 100);
	PathNode *paths = (PathNode *)getSegment(data.word(kReels)).ptr(kPathdata + 144 * room, 0);
	paths[param].on = 0x00;
}

RoomPaths *DreamBase::getRoomsPaths() {
	void *result = getSegment(data.word(kReels)).ptr(data.byte(kRoomnum) * 144, 144);
	return (RoomPaths *)result;
}

void DreamBase::faceRightWay() {
	PathNode *paths = getRoomsPaths()->nodes;
	uint8 dir = paths[data.byte(kManspath)].dir;
	data.byte(kTurntoface) = dir;
	data.byte(kLeavedirection) = dir;
}

void DreamBase::setWalk() {
	if (data.byte(kLinepointer) != 254) {
		// Already walking
		data.byte(kFinaldest) = data.byte(kPointerspath);
	} else if (data.byte(kPointerspath) == data.byte(kManspath)) {
		// Can't walk
		faceRightWay();
	} else if (data.byte(kWatchmode) == 1) {
		// Holding reel
		data.byte(kDestafterhold) = data.byte(kPointerspath);
		data.byte(kWatchmode) = 2;
	} else if (data.byte(kWatchmode) == 2) {
		// Can't walk
	} else {
		data.byte(kDestination) = data.byte(kPointerspath);
		data.byte(kFinaldest) = data.byte(kPointerspath);
		if (data.word(kMousebutton) != 2 || data.word(kCommandtype) == 3) {
			autoSetWalk();
		} else {
			data.byte(kWalkandexam) = 1;
			data.byte(kWalkexamtype) = data.byte(kCommandtype);
			data.byte(kWalkexamnum) = data.byte(kCommand);
			autoSetWalk();
		}
	}
}

void DreamBase::autoSetWalk() {
	if (data.byte(kFinaldest) == data.byte(kManspath))
		return;
	const RoomPaths *roomsPaths = getRoomsPaths();
	checkDest(roomsPaths);
	data.word(kLinestartx) = roomsPaths->nodes[data.byte(kManspath)].x - 12;
	data.word(kLinestarty) = roomsPaths->nodes[data.byte(kManspath)].y - 12;
	data.word(kLineendx) = roomsPaths->nodes[data.byte(kDestination)].x - 12;
	data.word(kLineendy) = roomsPaths->nodes[data.byte(kDestination)].y - 12;
	bresenhams();
	if (data.byte(kLinedirection) != 0) {
		data.byte(kLinepointer) = data.byte(kLinelength) - 1;
		data.byte(kLinedirection) = 1;
		return;
	}
	data.byte(kLinepointer) = 0;
}

void DreamBase::checkDest(const RoomPaths *roomsPaths) {
	const PathSegment *segments = roomsPaths->segments;
	const uint8 tmp = data.byte(kManspath) << 4;
	uint8 destination = data.byte(kDestination);
	for (size_t i = 0; i < 24; ++i) {
		if ((segments[i].b0 & 0xf0) == tmp &&
		    (segments[i].b0 & 0x0f) == data.byte(kDestination)) {
			data.byte(kDestination) = segments[i].b1 & 0x0f;
			return;
		}

		if (((segments[i].b0 & 0x0f) << 4) == tmp &&
		    ((segments[i].b0 & 0xf0) >> 4) == data.byte(kDestination)) {
			destination = segments[i].b1 & 0x0f;
		}
	}
	data.byte(kDestination) = destination;
}

void DreamBase::findXYFromPath() {
	const PathNode *roomsPaths = getRoomsPaths()->nodes;
	data.byte(kRyanx) = roomsPaths[data.byte(kManspath)].x - 12;
	data.byte(kRyany) = roomsPaths[data.byte(kManspath)].y - 12;
}

bool DreamBase::checkIfPathIsOn(uint8 index) {
	RoomPaths *roomsPaths = getRoomsPaths();
	uint8 pathOn = roomsPaths->nodes[index].on;
	return pathOn == 0xff;
}

void DreamBase::bresenhams() {
	workoutFrames();
	Common::Point *lineData = &_lineData[0];
	int16 startX = (int16)data.word(kLinestartx);
	int16 startY = (int16)data.word(kLinestarty);
	int16 endX = (int16)data.word(kLineendx);
	int16 endY = (int16)data.word(kLineendy);

	if (endX == startX) {
		uint16 deltaY;
		int8 y;
		if (endY < startY) {
			deltaY = startY - endY;
			y = (int8)endY;
			data.byte(kLinedirection) = 1;
		} else {
			deltaY = endY - startY;
			y = (int8)startY;
			data.byte(kLinedirection) = 0;
		}
		++deltaY;
		int8 x = (int8)startX;
		data.byte(kLinelength) = deltaY;
		for (; deltaY; --deltaY) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++y;
		}
		return;
	}
	uint16 deltaX;
	if (endX < startX) {
		deltaX = startX - endX;
		SWAP(startX, endX);
		SWAP(startY, endY);
		data.word(kLinestartx) = (uint16)startX;
		data.word(kLinestarty) = (uint16)startY;
		data.word(kLineendx) = (uint16)endX;
		data.word(kLineendy) = (uint16)endY;
		data.byte(kLinedirection) = 1;
	} else {
		deltaX = endX - startX;
		data.byte(kLinedirection) = 0;
	}

	int16 increment;
	if (endY == startY) {
		int8 x = (int8)startX;
		int8 y = (int8)startY;
		++deltaX;
		data.byte(kLinelength) = deltaX;
		for (; deltaX; --deltaX) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++x;
		}
		return;
	}
	uint16 deltaY;
	if (startY > endY) {
		deltaY = startY - endY;
		increment = -1;
	} else {
		deltaY = endY - startY;
		increment = 1;
	}

	uint16 delta1, delta2;
	byte lineRoutine;

	if (deltaY > deltaX) {
		lineRoutine = 1;
		delta1 = deltaY;
		delta2 = deltaX;
	} else {
		lineRoutine = 0;
		delta1 = deltaX;
		delta2 = deltaY;
	}

	uint16 increment1 = delta2 * 2;
	uint16 increment2 = delta2 * 2 - delta1 * 2;
	int16 remainder = delta2 * 2 - delta1;
	++delta1;
	int8 x = (int8)startX;
	int8 y = (int8)startY;
	data.byte(kLinelength) = delta1;
	if (lineRoutine != 1) {
		for (; delta1; --delta1) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			++x;
			if (remainder < 0) {
				remainder += increment1;
			} else {
				remainder += increment2;
				y += increment;
			}
		}
	} else {
		for (; delta1; --delta1) {
			lineData->x = x;
			lineData->y = y;
			++lineData;
			y += increment;
			if (remainder < 0) {
				remainder += increment1;
			} else {
				remainder += increment2;
				++x;
			}
		}
	}
}

void DreamBase::workoutFrames() {
	byte tmp;
	int diffx, diffy;

	// We have to use signed arithmetic here because these values can
	// be slightly negative when walking off-screen
	int lineStartX = (int16)data.word(kLinestartx);
	int lineStartY = (int16)data.word(kLinestarty);
	int lineEndX = (int16)data.word(kLineendx);
	int lineEndY = (int16)data.word(kLineendy);


	diffx = ABS(lineStartX - lineEndX);
	diffy = ABS(lineStartY - lineEndY);

	if (diffx < diffy) {
		tmp = 2;
		if (diffx >= (diffy >> 1))
			tmp = 1;
	} else {
		// tendstohoriz
		tmp = 0;
		if (diffy >= (diffx >> 1))
			tmp = 1;
	}

	if (lineStartX >= lineEndX) {
		// isinleft
		if (lineStartY < lineEndY) {
			if (tmp != 1)
				tmp ^= 2;
			tmp += 4;
		} else {
			// topleft
			tmp += 6;
		}
	} else {
		// isinright
		if (lineStartY < lineEndY) {
			tmp += 2;
		} else {
			// botright
			if (tmp != 1)
				tmp ^= 2;
		}
	}

	data.byte(kTurntoface) = tmp & 7;
	data.byte(kTurndirection) = 0;
}

} // End of namespace DreamGen
