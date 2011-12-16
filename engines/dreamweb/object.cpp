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

void DreamBase::showRyanPage() {
	showFrame(engine->icons1(), kInventx + 167, kInventy - 12, 12, 0);
	showFrame(engine->icons1(), kInventx + 167 + 18 * data.byte(kRyanpage), kInventy - 12, 13 + data.byte(kRyanpage), 0);
}

void DreamBase::findAllRyan(uint8 *inv) {
	memset(inv, 0xff, 60);
	for (size_t i = 0; i < kNumexobjects; ++i) {
		DynObject *extra = getExAd(i);
		if (extra->mapad[0] != 4)
			continue;
		if (extra->mapad[1] != 0xff)
			continue;
		uint8 slot = extra->mapad[2];
		assert(slot < 30);
		inv[2 * slot + 0] = i;
		inv[2 * slot + 1] = 4;
	}
}

void DreamBase::fillRyan() {
	uint8 *inv = getSegment(data.word(kBuffers)).ptr(kRyaninvlist, 60);
	findAllRyan(inv);
	inv += data.byte(kRyanpage) * 2 * 10;
	for (size_t i = 0; i < 2; ++i) {
		for (size_t j = 0; j < 5; ++j) {
			uint8 objIndex = *inv++;
			uint8 objType = *inv++;
			obToInv(objIndex, objType, kInventx + j * kItempicsize, kInventy + i * kItempicsize);
		}
	}
	showRyanPage();
}

void DreamGenContext::isItWorn() {
	flags._z = isItWorn((const DynObject *)es.ptr(bx, sizeof(DynObject)));
}

bool DreamBase::isItWorn(const DynObject *object) {
	return (object->id[0] == 'W'-'A') && (object->id[1] == 'E'-'A');
}

void DreamGenContext::wornError() {
	data.byte(kCommandtype) = 255;
	delPointer();
	printMessage(76, 21, 57, 240, false);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	data.byte(kCommandtype) = 255;
	workToScreenM();
}

void DreamGenContext::makeWorn() {
	makeWorn((DynObject *)es.ptr(bx, sizeof(DynObject)));
}

void DreamGenContext::makeWorn(DynObject *object) {
	object->id[0] = 'W'-'A';
	object->id[1] = 'E'-'A';
}

void DreamGenContext::obToInv() {
	obToInv(al, ah, di, bx);
}

void DreamBase::obToInv(uint8 index, uint8 flag, uint16 x, uint16 y) {
	showFrame(engine->icons1(), x - 2, y - 1, 10, 0);
	if (index == 0xff)
		return;

	Frame *extras = (Frame *)getSegment(data.word(kExtras)).ptr(0, 0);
	Frame *frees = (Frame *)getSegment(data.word(kFreeframes)).ptr(0, 0);
	Frame *frames = (flag == 4) ? extras : frees;
	showFrame(frames, x + 18, y + 19, 3 * index + 1, 128);
	const DynObject *object = (const DynObject *)getAnyAdDir(index, flag);
	bool worn = isItWorn(object);
	if (worn)
		showFrame(engine->icons1(), x - 3, y - 2, 7, 0);
}

void DreamBase::obPicture() {
	if (data.byte(kObjecttype) == 1)
		return;
	Frame *frames;
	if (data.byte(kObjecttype) == 4)
		frames = (Frame *)getSegment(data.word(kExtras)).ptr(0, 0);
	else
		frames = (Frame *)getSegment(data.word(kFreeframes)).ptr(0, 0);
	uint8 frame = 3 * data.byte(kCommand) + 1;
	showFrame(frames, 160, 68, frame, 0x80);
}

void DreamBase::obIcons() {
	uint8 value1, value2;
	getAnyAd(&value1, &value2);
	if (value1 != 0xff) {
		// can open it
		showFrame(engine->icons2(), 210, 1, 4, 0);
	}

	showFrame(engine->icons2(), 260, 1, 1, 0);
}

void DreamGenContext::examineOb(bool examineAgain) {
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
			createPanel();
			showPanel();
			showMan();
			showExit();
			obIcons();
			obPicture();
			describeOb();
			underTextLine();
			data.byte(kCommandtype) = 255;
			readMouse();
			showPointer();
			workToScreenCPP();
			delPointer();
			examineAgain = false;
		}

		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		delPointer();
		data.byte(kGetback) = 0;
		switch (data.byte(kInvopen)) {
		case 0: {
			RectWithCallback examList[] = {
				{ 273,320,157,198,&DreamGenContext::getBackFromOb },
				{ 260,300,0,44,&DreamGenContext::useObject },
				{ 210,254,0,44,&DreamGenContext::selectOpenOb },
				{ 144,176,64,96,&DreamGenContext::setPickup },
				{ 0,50,50,200,&DreamBase::examineInventory },
				{ 0,320,0,200,&DreamBase::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(examList);
			break;
		}
		case 1: {
			// Note: This table contains the non-constant _openChangeSize!
			RectWithCallback invList1[] = {
				{ 273,320,157,198,&DreamGenContext::getBackFromOb },
				{ 255,294,0,24,&DreamGenContext::dropObject },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamGenContext::incRyanPage },
				{ kInventx,_openChangeSize,kInventy+100,kInventy+100+kItempicsize,&DreamGenContext::useOpened },
				{ kInventx,kInventx+(5*kItempicsize),kInventy,kInventy+(2*kItempicsize),&DreamGenContext::inToInv },
				{ 0,320,0,200,&DreamBase::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(invList1);
			break;
		}
		default: {
			RectWithCallback withList1[] = {
				{ 273,320,157,198,&DreamGenContext::getBackFromOb },
				{ kInventx+167,kInventx+167+(18*3),kInventy-18,kInventy-2,&DreamGenContext::incRyanPage },
				{ kInventx,kInventx+(5*kItempicsize), kInventy,kInventy+(2*kItempicsize),&DreamGenContext::selectOb },
				{ 0,320,0,200,&DreamBase::blank },
				{ 0xFFFF,0,0,0,0 }
			};
			checkCoords(withList1);
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
		// isWatching
		makeMainScreen();
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
		commandOnly();
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;
	if (!(data.word(kMousebutton) & 1)) // only on left mouse button
		return;


	data.word(kTimecount) = 0;
	data.byte(kPointermode) = 0;
	data.byte(kInmaparea) = 0;
	animPointer();
	createPanel();
	showPanel();
	examIcon();
	showMan();
	showExit();
	underTextLine();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openInv();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
	data.byte(kOpenedob) = 255;
	examineOb(false);
}

void DreamGenContext::transferText() {
	getSegment(data.word(kExtras)).word(kExtextdat + data.byte(kExpos) * 2) = data.word(kExtextpos);
	uint16 freeTextOffset = data.byte(kItemtotran) * 2;
	uint16 srcOffset = getSegment(data.word(kFreedesc)).word(kFreetextdat + freeTextOffset);
	const char *src = (const char *)getSegment(data.word(kFreedesc)).ptr(kFreetext + srcOffset, 0);
	char *dst = (char *)getSegment(data.word(kExtras)).ptr(kExtext + data.word(kExtextpos), 0);

	size_t len = strlen(src);
	memcpy(dst, src, len + 1);
	data.word(kExtextpos) += len + 1;
}

void DreamBase::getBackFromOb() {
	if (data.byte(kPickup) != 1)
		getBack1();
	else
		blank();
}

void DreamGenContext::getOpenedSize() {
	ax = getOpenedSizeCPP();
}

byte DreamGenContext::getOpenedSizeCPP() {
	byte obj = data.byte(kOpenedob);
	switch (data.byte(kOpenedtype)) {
	case 4:
		return getExAd(obj)->b7;
	case 2:
		return getFreeAd(obj)->b7;
	default:
		return getSetAd(obj)->b3;
	}
}

void DreamGenContext::openOb() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	copyName(data.byte(kOpenedtype), data.byte(kOpenedob), commandLine);

	printMessage(kInventx, kInventy+86, 62, 240, false);

	al = printDirect(commandLine, data.word(kLastxpos) + 5, kInventy+86, 220, false);

	fillOpen();
	_openChangeSize = getOpenedSizeCPP() * kItempicsize + kInventx;
}

void DreamGenContext::identifyOb() {
	if (data.word(kWatchingtime) != 0 ||
		data.word(kMousex) - data.word(kMapadx) >= 22 * 8 ||
		data.word(kMousey) - data.word(kMapady) >= 20 * 8) {
		blank();
		return;
	}

	data.byte(kInmaparea) = 1;
	ah = bl;
	push(ax);
	findPathOfPoint();
	data.byte(kPointerspath) = dl;
	ax = pop();
	push(ax);
	findFirstPath();
	data.byte(kPointerfirstpath) = al;
	ax = pop();

	byte x = al;
	byte y = ah;

	if (checkIfEx(x, y) || checkIfFree(x, y) ||
		checkIfPerson(x, y) || checkIfSet(x, y))
		return; // finishidentify

	x = (data.word(kMousex) - data.word(kMapadx)) & 0xFF;
	y = (data.word(kMousey) - data.word(kMapady)) & 0xFF;
	byte flag, flagEx, type, flagX, flagY;

	checkOne(x, y, &flag, &flagEx, &type, &flagX, &flagY);

	if (type != 0 && data.byte(kMandead) != 1)
		obName(type, 3);
	else
		blank();
}

} // End of namespace DreamGen
