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

void DreamGenContext::turnpathon() {
	turnpathon(al);
}

void DreamGenContext::turnpathon(uint8 param) {
	findormake(param, 0xff, data.byte(kRoomnum) + 100);
	PathNode *roomsPaths = getroomspaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0xff;
}

void DreamGenContext::turnpathoff() {
	turnpathoff(al);
}

void DreamGenContext::turnpathoff(uint8 param) {
	findormake(param, 0x00, data.byte(kRoomnum) + 100);
	PathNode *roomsPaths = getroomspaths()->nodes;
	if (param == 0xff)
		return;
	roomsPaths[param].on = 0x00;
}

void DreamGenContext::turnanypathon(uint8 param, uint8 room) {
	findormake(param, 0xff, room + 100);
	PathNode *paths = (PathNode *)segRef(data.word(kReels)).ptr(kPathdata + 144 * room, 0);
	paths[param].on = 0xff;
}


void DreamGenContext::turnanypathon() {
	turnanypathon(al, ah);
}

void DreamGenContext::turnanypathoff(uint8 param, uint8 room) {
	findormake(param, 0x00, room + 100);
	PathNode *paths = (PathNode *)segRef(data.word(kReels)).ptr(kPathdata + 144 * room, 0);
	paths[param].on = 0x00;
}

void DreamGenContext::turnanypathoff() {
	turnanypathoff(al, ah);
}

RoomPaths *DreamGenContext::getroomspaths() {
	void *result = segRef(data.word(kReels)).ptr(data.byte(kRoomnum) * 144, 144);
	return (RoomPaths *)result;
}

void DreamGenContext::autosetwalk() {
	al = data.byte(kManspath);
	if (data.byte(kFinaldest) == al)
		return;
	const RoomPaths *roomsPaths = getroomspaths();
	checkdest(roomsPaths);
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

void DreamGenContext::checkdest(const RoomPaths *roomsPaths) {
	const PathSegment *segments = roomsPaths->segments;
	ah = data.byte(kManspath) << 4;
	al = data.byte(kDestination);
	uint8 destination = data.byte(kDestination);
	for (size_t i = 0; i < 24; ++i) {
		dh = segments[i].b0 & 0xf0;
		dl = segments[i].b0 & 0x0f;
		if (ax == dx) {
			data.byte(kDestination) = segments[i].b1 & 0x0f;
			return;
		}
		dl = (segments[i].b0 & 0xf0) >> 4;
		dh = (segments[i].b0 & 0x0f) << 4;
		if (ax == dx) {
			destination = segments[i].b1 & 0x0f;
		}
	}
	data.byte(kDestination) = destination;
}

void DreamGenContext::findxyfrompath() {
	const PathNode *roomsPaths = getroomspaths()->nodes;
	data.byte(kRyanx) = roomsPaths[data.byte(kManspath)].x - 12;
	data.byte(kRyany) = roomsPaths[data.byte(kManspath)].y - 12;
}

void DreamGenContext::checkifpathison() {
	flags._z = checkifpathison(al);
}

bool DreamGenContext::checkifpathison(uint8 index) {
	RoomPaths *roomsPaths = getroomspaths();
	uint8 pathOn = roomsPaths->nodes[index].on;
	return pathOn == 0xff;
}

} /*namespace dreamgen */

