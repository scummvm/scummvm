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
#include "engines/util.h"
#include "graphics/surface.h"
#include "common/config-manager.h"

namespace DreamGen {

void DreamGenContext::turnpathonCPP(uint8 param) {
	al = param;
	push(es);
	push(bx);
	turnpathon();
	bx = pop();
	es = pop();
}

void DreamGenContext::turnpathoffCPP(uint8 param) {
	al = param;
	push(es);
	push(bx);
	turnpathoff();
	bx = pop();
	es = pop();
}

void DreamGenContext::getroomspaths() {
	es = data.word(kReels);
	bx = data.byte(kRoomnum) * 144;
}

uint8 *DreamGenContext::getroomspathsCPP() {
	void *result = segRef(data.word(kReels)).ptr(data.byte(kRoomnum) * 144, 144);
	return (uint8 *)result;
}

void DreamGenContext::autosetwalk() {
	al = data.byte(kManspath);
	if (data.byte(kFinaldest) == al)
		return;
	const uint8 *roomsPaths = getroomspathsCPP();
	checkdest(roomsPaths);
	data.word(kLinestartx) = roomsPaths[data.byte(kManspath) * 8 + 0] - 12;
	data.word(kLinestarty) = roomsPaths[data.byte(kManspath) * 8 + 1] - 12;
	data.word(kLineendx) = roomsPaths[data.byte(kDestination) * 8 + 0] - 12;
	data.word(kLineendy) = roomsPaths[data.byte(kDestination) * 8 + 1] - 12;
	bresenhams();
	if (data.byte(kLinedirection) != 0) {
		data.byte(kLinepointer) = data.byte(kLinelength) - 1;
		data.byte(kLinedirection) = 1;
		return;
	}
	data.byte(kLinepointer) = 0;
}

void DreamGenContext::checkdest(const uint8 *roomsPaths) {
	const uint8 *p = roomsPaths + 12 * 8;
	ah = data.byte(kManspath) << 4;
	al = data.byte(kDestination);
	uint8 destination = data.byte(kDestination);
	for (size_t i = 0; i < 24; ++i) {
		dh = p[0] & 0xf0;
		dl = p[0] & 0x0f;
		if (ax == dx) {
			data.byte(kDestination) = p[1] & 0x0f;
			return;
		}
		dl = (p[0] & 0xf0) >> 4;
		dh = (p[0] & 0x0f) << 4;
		if (ax == dx) {
			destination = p[1] & 0x0f;
		}
		p += 2;
	}
	data.byte(kDestination) = destination;
}

void DreamGenContext::findxyfrompath() {
	const uint8 *roomsPaths = getroomspathsCPP();
	data.byte(kRyanx) = roomsPaths[data.byte(kManspath) * 8 + 0] - 12;
	data.byte(kRyany) = roomsPaths[data.byte(kManspath) * 8 + 1] - 12;
}

} /*namespace dreamgen */

