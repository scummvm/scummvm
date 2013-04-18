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

void DreamGenContext::fillryan() {
	uint8 *inv = segRef(data.word(kBuffers)).ptr(kRyaninvlist, 60);
	findallryan(inv);
	inv += data.byte(kRyanpage) * 2 * 10;
	for (size_t i = 0; i < 2; ++i) {
		for (size_t j = 0; j < 5; ++j) {
			uint8 objIndex = *inv++;
			uint8 objType = *inv++;
			obtoinv(objIndex, objType, kInventx + j * kItempicsize, kInventy + i * kItempicsize);
		}
	}
	showryanpage();
}

void DreamGenContext::isitworn() {
	flags._z = isitworn((const DynObject *)es.ptr(bx, sizeof(DynObject)));
}

bool DreamGenContext::isitworn(const DynObject *object) {
	return (object->id[0] == 'W'-'A') && (object->id[1] == 'E'-'A');
}

void DreamGenContext::wornerror() {
	data.byte(kCommandtype) = 255;
	delpointer();
	printmessage(76, 21, 57, 240, false);
	worktoscreenm();
	hangonp(50);
	showpanel();
	showman();
	examicon();
	data.byte(kCommandtype) = 255;
	worktoscreenm();
}

void DreamGenContext::makeworn() {
	makeworn((DynObject *)es.ptr(bx, sizeof(DynObject)));
}

void DreamGenContext::makeworn(DynObject *object) {
	object->id[0] = 'W'-'A';
	object->id[1] = 'E'-'A';
}

void DreamGenContext::obtoinv() {
	obtoinv(al, ah, di, bx);
}

void DreamGenContext::obtoinv(uint8 index, uint8 flag, uint16 x, uint16 y) {
	Frame *icons1 = (Frame *)segRef(data.word(kIcons1)).ptr(0, 0);
	showframe(icons1, x - 2, y - 1, 10, 0);
	if (index == 0xff)
		return;

	Frame *extras = (Frame *)segRef(data.word(kExtras)).ptr(0, 0);
	Frame *frees = (Frame *)segRef(data.word(kFreeframes)).ptr(0, 0);
	Frame *frames = (flag == 4) ? extras : frees;
	showframe(frames, x + 18, y + 19, 3 * index + 1, 128);
	const DynObject *object = (const DynObject *)getanyaddir(index, flag);
	bool worn = isitworn(object);
	if (worn)
		showframe(icons1, x - 3, y - 2, 7, 0);
}

void DreamGenContext::obpicture() {
	if (data.byte(kObjecttype) == 1)
		return;
	Frame *frames;
	if (data.byte(kObjecttype) == 4)
		frames = (Frame *)segRef(data.word(kExtras)).ptr(0, 0);
	else
		frames = (Frame *)segRef(data.word(kFreeframes)).ptr(0, 0);
	uint8 frame = 3 * data.byte(kCommand) + 1;
	showframe(frames, 160, 68, frame, 0x80);
}

void DreamGenContext::obicons() {
	uint8 value1, value2;
	getanyad(&value1, &value2);
	if (value1 == 0xff) {
		showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 260, 1, 1, 0);
	} else {
		showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 210, 1, 4, 0);
	}
}

} /*namespace dreamgen */

