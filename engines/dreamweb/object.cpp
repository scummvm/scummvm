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
	if (value1 != 0xff) {
		// can open it
		showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 210, 1, 4, 0);
	}

	showframe((Frame *)segRef(data.word(kIcons2)).ptr(0, 0), 260, 1, 1, 0);
}

void DreamGenContext::examineob(bool examineAgain) {
	data.byte(kPointermode) = 0;
	data.word(kTimecount) = 0;
	while (true) {
		if (examineAgain) {
			data.byte(kInmaparea) = 0;
			data.byte(kExamagain) = 0;
			data.byte(kOpenedob) = 255;
			data.byte(kOpenedtype) = 255;
			data.byte(kInvopen) = 0;
			al = data.byte(kCommandtype);
			data.byte(kObjecttype) = al;
			data.byte(kItemframe) = 0;
			data.byte(kPointerframe) = 0;
			createpanel();
			showpanel();
			showman();
			showexit();
			obicons();
			obpicture();
			describeob();
			undertextline();
			data.byte(kCommandtype) = 255;
			readmouse();
			showpointer();
			worktoscreen();
			delpointer();
			examineAgain = false;
		}

		readmouse();
		showpointer();
		vsync();
		dumppointer();
		dumptextline();
		delpointer();
		data.byte(kGetback) = 0;
		switch (data.byte(kInvopen)) {
		case 0: {
			RectWithCallback examlist[] = {
				{ 273,320,157,198,&DreamGenContext::getbackfromob },
				{ 260,300,0,44,&DreamGenContext::useobject },
				{ 210,254,0,44,&DreamGenContext::selectopenob },
				{ 144,176,64,96,&DreamGenContext::setpickup },
				{ 0,50,50,200,&DreamGenContext::examinventory },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkcoords(examlist);
			break;
		}
		case 1: {
			// NB: This table contains the non-constant openchangesize!
			RectWithCallback invlist1[] = {
				{ 273,320,157,198,&DreamGenContext::getbackfromob },
				{ 255,294,0,24,&DreamGenContext::dropobject },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamGenContext::incryanpage },
				{ kInventx, cs.word(offset_openchangesize),kInventy+100,kInventy+100+kItempicsize,&DreamGenContext::useopened },
				{ kInventx,kInventx+(5*kItempicsize), kInventy,kInventy+(2*kItempicsize),&DreamGenContext::intoinv },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkcoords(invlist1);
			break;
		}
		default: {
			RectWithCallback withlist1[] = {
				{ 273,320,157,198,&DreamGenContext::getbackfromob },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamGenContext::incryanpage },
				{ kInventx,kInventx+(5*kItempicsize), kInventy,kInventy+(2*kItempicsize),&DreamGenContext::selectob },
				{ 0,320,0,200,&DreamGenContext::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkcoords(withlist1);
			break;
		}
		}
		if (data.byte(kQuitrequested) != 0)
			break;
		if (data.byte(kExamagain) != 0)
			examineAgain = true;
		else if (data.byte(kGetback) != 0)
			break;
	}

	data.byte(kPickup) = 0;
	if (data.word(kWatchingtime) != 0 || data.byte(kNewlocation) == 255) {
		// iswatching
		makemainscreen();
	}

	data.byte(kInvopen) = 0;
	data.byte(kOpenedob) = 255;
}

void DreamGenContext::inventory() {
	if (data.byte(kMandead) == 1 || data.word(kWatchingtime) != 0) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 239) {
		data.byte(kCommandtype) = 239;
		al = 32;
		commandonly();
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;
	if (!(data.word(kMousebutton) & 1)) // only on left mouse button
		return;


	data.word(kTimecount) = 0;
	data.byte(kPointermode) = 0;
	data.byte(kInmaparea) = 0;
	animpointer();
	createpanel();
	showpanel();
	examicon();
	showman();
	showexit();
	undertextline();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openinv();
	readmouse();
	showpointer();
	worktoscreen();
	delpointer();
	data.byte(kOpenedob) = 255;
	examineob(false);
}

void DreamGenContext::transfertext() {
	segRef(data.word(kExtras)).word(kExtextdat + data.byte(kExpos) * 2) = data.word(kExtextpos);
	uint16 freeTextOffset = data.byte(kItemtotran) * 2;
	uint16 srcOffset = segRef(data.word(kFreedesc)).word(kFreetextdat + freeTextOffset);
	const char *src = (const char *)segRef(data.word(kFreedesc)).ptr(kFreetext + srcOffset, 0);
	char *dst = (char *)segRef(data.word(kExtras)).ptr(kExtext + data.word(kExtextpos), 0);

	size_t len = strlen(src);
	memcpy(dst, src, len + 1);
	data.word(kExtextpos) += len + 1;
}


} /*namespace dreamgen */

