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

bool DreamBase::isItWorn(const DynObject *object) {
	return (object->id[0] == 'W'-'A') && (object->id[1] == 'E'-'A');
}

void DreamBase::wornError() {
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

void DreamBase::makeWorn(DynObject *object) {
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
	if (data.byte(kObjecttype) == kSetObjectType1)
		return;
	Frame *frames;
	if (data.byte(kObjecttype) == kExObjectType)
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
			data.byte(kObjecttype) = data.byte(kCommandtype);
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
			workToScreen();
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
			RectWithCallback<DreamGenContext> examList[] = {
				{ 273,320,157,198,&DreamBase::getBackFromOb },
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
			RectWithCallback<DreamGenContext> invList1[] = {
				{ 273,320,157,198,&DreamBase::getBackFromOb },
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
			RectWithCallback<DreamGenContext> withList1[] = {
				{ 273,320,157,198,&DreamBase::getBackFromOb },
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
		commandOnly(32);
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
	workToScreen();
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
	//ax = getOpenedSlotCount();

	// We need to call the ASM-style versions of get*Ad, as these also set
	// bx and es

	al = data.byte(kOpenedob);

	switch (data.byte(kOpenedtype)) {
	case 4:
		getExAd();
		ax = es.word(bx+7);
		break;
	case 2:
		getFreeAd();
		ax = es.word(bx+7);
		break;
	default:
		getSetAd();
		ax = es.word(bx+3);
		break;
	}
}

byte DreamGenContext::getOpenedSlotCount() {
	byte obj = data.byte(kOpenedob);
	switch (data.byte(kOpenedtype)) {
	case 4:
		return getExAd(obj)->slotCount;
	case 2:
		return getFreeAd(obj)->slotCount;
	default:
		return getSetAd(obj)->slotCount;
	}
}

byte DreamGenContext::getOpenedSlotSize() {
	byte obj = data.byte(kOpenedob);
	switch (data.byte(kOpenedtype)) {
	case 4:
		return getExAd(obj)->slotSize;
	case 2:
		return getFreeAd(obj)->slotSize;
	default:
		return getSetAd(obj)->slotSize;
	}
}

void DreamGenContext::openOb() {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";

	copyName(data.byte(kOpenedtype), data.byte(kOpenedob), commandLine);

	printMessage(kInventx, kInventy+86, 62, 240, false);

	al = printDirect(commandLine, data.word(kLastxpos) + 5, kInventy+86, 220, false);

	fillOpen();
	_openChangeSize = getOpenedSlotCount() * kItempicsize + kInventx;
}

void DreamGenContext::identifyOb() {
	if (data.word(kWatchingtime) != 0) {
		blank();
		return;
	}

	ax = data.word(kMousex) - data.word(kMapadx);
	bx = data.word(kMousey) - data.word(kMapady);

	if (ax >= 22 * 8 || bx >= 20 * 8) {
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
	data.byte(kPointerfirstpath) = findFirstPath(al, ah);
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

uint16 DreamGenContext::findInvPosCPP() {
	uint16 x = data.word(kMousex) - kInventx;
	uint16 y = data.word(kMousey) - kInventy;
	uint16 pos = (x / kItempicsize) + (y / kItempicsize) * 5;
	uint16 invPos = data.byte(kRyanpage) * 10 + pos;
	data.byte(kLastinvpos) = invPos & 0xFF;
	return invPos * 2 + kRyaninvlist;
}

void DreamGenContext::findInvPos() {
	bx = findInvPosCPP();
	es = data.word(kBuffers);
}

void DreamGenContext::selectOb() {
	uint16 objectId = getSegment(data.word(kBuffers)).word(findInvPosCPP());
	if ((objectId & 0xFF) == 255) {
		blank();
		return;
	}

	data.byte(kWithobject) = objectId & 0x00FF;
	data.byte(kWithtype)   = objectId >> 8;

	if (objectId != data.word(kOldsubject) || data.byte(kCommandtype) != 221) {
		if (objectId == data.word(kOldsubject))
			data.byte(kCommandtype) = 221;
		data.word(kOldsubject) = objectId;
		commandWithOb(0, data.byte(kWithtype), data.byte(kWithobject));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	delPointer();
	data.byte(kInvopen) = 0;
	useRoutine();
}

void DreamGenContext::setPickup() {
	if (data.byte(kObjecttype) != kSetObjectType1 && data.byte(kObjecttype) != kSetObjectType3) {
		// Object types 1 and 3 are excluded, so the resulting object is a DynObject
		uint8 dummy;
		DynObject *object = (DynObject *)getAnyAd(&dummy, &dummy);
		if (object->mapad[0] == 4) {
			blank();
			return;
		}
	} else {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 209) {
		data.byte(kCommandtype) = 209;
		commandWithOb(33, data.byte(kObjecttype), data.byte(kCommand));
	}

	if (data.word(kMousebutton) != 1 || data.word(kMousebutton) == data.word(kOldbutton))
		return;

	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	data.byte(kPickup) = 1;
	data.byte(kInvopen) = 2;

	if (data.byte(kObjecttype) != kExObjectType) {
		data.byte(kItemframe) = data.byte(kCommand);
		data.byte(kOpenedob) = 255;
		transferToEx();
		data.byte(kItemframe) = al;
		data.byte(kObjecttype) = kExObjectType;
		DynObject *object = getExAd(data.byte(kItemframe));
		object->mapad[0] = 20;
		object->mapad[1] = 255;
	} else {
		data.byte(kItemframe) = data.byte(kCommand);
		data.byte(kOpenedob) = 255;
	}

	openInv();
	workToScreenM();
}

void DreamBase::deleteExFrame(uint8 frameNum) {
	Frame *frame = (Frame *)getSegment(data.word(kExtras)).ptr(kExframedata + sizeof(Frame)*frameNum, sizeof(Frame));

	uint16 frameSize = frame->width * frame->height;
	// Note: the original asm didn't subtract frameSize from remainder
	uint16 remainder = kExframeslen - frame->ptr() - frameSize;
	uint16 startOff = kExframes + frame->ptr();
	uint16 endOff = startOff + frameSize;

	// Shift frame data after this one down
	memmove(getSegment(data.word(kExtras)).ptr(startOff, remainder), getSegment(data.word(kExtras)).ptr(endOff, remainder), remainder);

	// Combined frame data is now frameSize smaller
	data.word(kExframepos) -= frameSize;

	// Adjust all frame pointers pointing into the shifted data
	for (unsigned int i = 0; i < 3*kNumexobjects; ++i) {
		frame = (Frame *)getSegment(data.word(kExtras)).ptr(kExframedata + sizeof(Frame)*i, sizeof(Frame));
		if (frame->ptr() >= startOff)
			frame->setPtr(frame->ptr() - frameSize);
	}
}

void DreamBase::deleteExText(uint8 textNum) {
	uint16 offset = getSegment(data.word(kExtras)).word(kExtextdat + 2*textNum);

	uint16 startOff = kExtext + offset;
	uint16 textSize = strlen((char *)getSegment(data.word(kExtras)).ptr(startOff, 0)) + 1;
	uint16 endOff = startOff + textSize;
	uint16 remainder = kExtextlen - offset - textSize;

	// Shift text data after this one down
	memmove(getSegment(data.word(kExtras)).ptr(startOff, remainder), getSegment(data.word(kExtras)).ptr(endOff, remainder), remainder);

	// Combined text data is now frameSize smaller
	data.word(kExtextpos) -= textSize;

	// Adjust all text pointers pointing into the shifted data
	for (unsigned int i = 0; i < kNumexobjects; ++i) {
		uint16 t = getSegment(data.word(kExtras)).word(kExtextdat + 2*i);
		if (t >= offset + textSize)
			getSegment(data.word(kExtras)).word(kExtextdat + 2*i) = t - textSize;
	}
}

void DreamBase::deleteExObject(uint8 index) {
	DynObject *obj = getExAd(index);

	memset(obj, 0xFF, sizeof(DynObject));

	deleteExFrame(3*index);
	deleteExFrame(3*index + 1);

	deleteExText(index);

	for (uint8 i = 0; i < kNumexobjects; ++i) {
		DynObject *t = getExAd(i);
		// Is this object contained in the one we've just deleted?
		if (t->mapad[0] == 4 && t->mapad[1] == index)
			deleteExObject(i);
	}
}

void DreamBase::removeObFromInv() {
	if (data.byte(kCommand) == 100)
		return; // object doesn't exist

	assert(data.byte(kObjecttype) == kExObjectType);

	deleteExObject(data.byte(kCommand));
}

void DreamGenContext::inToInv() {
	if (!data.byte(kPickup)) {
		outOfInv();
		return;
	}

	uint16 subject = getSegment(data.word(kBuffers)).word(findInvPosCPP());

	if ((subject & 0xFF) != 255) {
		swapWithInv();
		return;
	}

	subject = (data.byte(kObjecttype) << 8) | data.byte(kItemframe);

	if (subject != data.word(kOldsubject) || data.byte(kCommandtype) != 220) {
		if (subject == data.word(kOldsubject))
			data.byte(kCommandtype) = 220;
		data.word(kOldsubject) = subject;
		commandWithOb(35, data.byte(kObjecttype), data.byte(kItemframe));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return; // notletgo2
	
	delPointer();
	DynObject *object = getExAd(data.byte(kItemframe));
	object->mapad[0] = 4;
	object->mapad[1] = 255;
	object->mapad[2] = data.byte(kLastinvpos);
	data.byte(kPickup) = 0;
	fillRyan();
	readMouse();
	showPointer();
	outOfInv();
	workToScreen();
	delPointer();
}

void DreamGenContext::outOfInv() {
	uint16 subject = getSegment(data.word(kBuffers)).word(findInvPosCPP());

	if ((subject & 0xFF) == 255) {
		blank();
		return;
	}

	if (data.word(kMousebutton) == 2) {
		reExFromInv();
		return;
	}

	if (subject != data.word(kOldsubject) || data.byte(kCommandtype) != 221) {
		if (subject == data.word(kOldsubject))
			data.byte(kCommandtype) = 221;
		data.word(kOldsubject) = subject;
		byte type = subject >> 8;
		byte frame = subject & 0xFF;
		commandWithOb(36, type, frame);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	delPointer();
	data.byte(kPickup) = 1;
	subject = getSegment(data.word(kBuffers)).word(findInvPosCPP());
	data.byte(kObjecttype) = subject >> 8;
	data.byte(kItemframe) = subject & 0xFF;
	DynObject *object = getExAd(data.byte(kItemframe));
	object->mapad[0] = 20;
	object->mapad[1] = 255;
	fillRyan();
	readMouse();
	showPointer();
	inToInv();
	workToScreen();
	delPointer();
}

void DreamBase::purgeALocation(uint8 index) {
	// index == al
	for (uint8 i = 0; i < kNumexobjects; ++i) {
		DynObject *t = getExAd(i);
		if (t->currentLocation == index && t->mapad[0] == 0) {
			deleteExObject(i);
		}
	}
}

const uint8 *DreamBase::getObTextStart() {
	uint16 textSeg, textDatOff, textOff;
	if (data.byte(kObjecttype) == kFreeObjectType) {
		textSeg = data.word(kFreedesc);
		textDatOff = kFreetextdat;
		textOff = kFreetext;
	} else if (data.byte(kObjecttype) == kSetObjectType1) {
		textSeg = data.word(kSetdesc);
		textDatOff = kSettextdat;
		textOff = kSettext;
	} else {
		textSeg = data.word(kExtras);
		textDatOff = kExtextdat;
		textOff = kExtext;
	}
	const uint8 *textBase = getSegment(textSeg).ptr(textOff, 0);
	const uint8 *text = textBase + getSegment(textSeg).word(textDatOff + 2*data.byte(kCommand));

	if (data.byte(kObjecttype) != kSetObjectType1)
		return text;

	const uint8 *obname = text;
	while (true) {
		const uint8 *start = text;
		findNextColon(&text);

		// Not an empty description string?
		if (*text != 0 && *text != ':')
			return start;

		// If the description string (of a SetObjectType1 object) is empty,
		// look for an object with the same name.
		// Example: Eden's garage door outside has two parts. The right part
		// has no description of its own but uses that of the left part.

		bool found = false;
		do {
			text++;
			uint8 c = *obname;

			// scan for matching first character
			while (*text != c) {
				text++;

				// arbitrary give-up counter
				if (text - (textBase - textOff) >= 8000) {
					warning("Object description for %d/%d not found", data.byte(kObjecttype), data.byte(kCommand));
					return obname;
				}
			}

			// found matching first character, so match the rest
			const uint8 *s1 = obname;
			const uint8 *s2 = text;
			do {
				s1++;
				s2++;
			} while (*s1 != ':' && *s1 != 0 && *s1 == *s2);

			if (*s1 == ':' || *s1 == 0)
				found = true; // (prefix) matched the entire object name
		} while (!found);

		// We found an object with the same name. The next loop iteration
		// will check if this one again has an empty description.
	}
}

void DreamBase::dropObject() {
	if (data.byte(kCommandtype) != 223) {
		data.byte(kCommandtype) = 223;
		if (!data.byte(kPickup)) {
			blank();
			return;
		}
		commandWithOb(37, data.byte(kObjecttype), data.byte(kItemframe));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	if (isItWorn(getEitherAdCPP())) {
		wornError();
		return;
	}

	if (data.byte(kReallocation) != 47) {
		byte flag, flagEx, type, flagX, flagY;
		checkOne(data.byte(kRyanx) + 12, data.byte(kRyany) + 12, &flag, &flagEx, &type, &flagX, &flagY);

		if (flag >= 2) {
			dropError();
			return;
		}
	} else {
		dropError();
		return;
	}

	if (data.byte(kMapxsize) == 64 && data.byte(kMapysize) == 64) {
		// Inside lift
		dropError();
		return;
	}

	if (compare(data.byte(kItemframe), 4, "GUNA") || compare(data.byte(kItemframe), 4, "SHLD")) {
		cantDrop();
		return;
	}

	data.byte(kObjecttype) = 4;
	DynObject *object = getExAd(data.byte(kItemframe));
	object->mapad[0] = 0;
	object->mapad[1] = ((data.byte(kRyanx) + 4) >> 4) + data.byte(kMapx);
	object->mapad[2] = (data.byte(kRyanx) + 4) & 0xF;
	object->mapad[3] = ((data.byte(kRyany) + 8) >> 4) + data.byte(kMapy);
	object->mapad[4] = (data.byte(kRyany) + 8) & 0xF;
	data.byte(kPickup) = 0;
	object->currentLocation = data.byte(kReallocation);
}

bool DreamGenContext::checkObjectSizeCPP() {
	byte containerSize = getOpenedSlotSize();
	DynObject *object = getEitherAdCPP();
	// If there is no size defined for the object in the editor, set its size
	// to 6. This could be a bad idea, according to the original source.
	byte objectSize = (object->objectSize != 255) ? object->objectSize : 6;

	if (containerSize >= 100) {
		// Special type of container: only objects of the same special type fit.
		if (containerSize == objectSize)
			return true;

		errorMessage3();
		return false;
	}

	if (objectSize >= 100) {
		// Special type of object, but a regular container.
		// Subtract 100 from the size to get its regular size.
		objectSize -= 100;
	}

	if (containerSize >= objectSize)
		return true;

	errorMessage2();
	return false;
}

void DreamGenContext::selectOpenOb() {
	uint8 slotSize, slotCount;
	getAnyAd(&slotSize, &slotCount);
	if (slotCount == 255) {
		// Can't open the object
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 224) {
		data.byte(kCommandtype) = 224;
		commandWithOb(38, data.byte(kObjecttype), data.byte(kCommand));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	data.byte(kOpenedob) = data.byte(kCommand);
	data.byte(kOpenedtype) = data.byte(kObjecttype);
	createPanel();
	showPanel();
	showMan();
	examIcon();
	showExit();
	openInv();
	openOb();
	underTextLine();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::reExFromInv() {
	uint16 objectId = getSegment(data.word(kBuffers)).word(findInvPosCPP());
	data.byte(kCommandtype) = objectId >> 8;
	data.byte(kCommand)     = objectId & 0x00FF;
	data.byte(kExamagain) = 1;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::swapWithInv() {
	uint16 subject = (data.byte(kObjecttype) << 8) | data.byte(kItemframe);
	if (subject == data.word(kOldsubject)) {
		if (data.byte(kCommandtype) != 243) {
			data.byte(kCommandtype) = 243;
			data.word(kOldsubject) = subject;
			commandWithOb(34, data.byte(kObjecttype), data.byte(kItemframe));
		}
	} else {
		data.word(kOldsubject) = subject;
		commandWithOb(34, data.byte(kObjecttype), data.byte(kItemframe));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	byte prevType = data.byte(kObjecttype);
	byte prevFrame = data.byte(kItemframe);
	uint16 objectId = getSegment(data.word(kBuffers)).word(findInvPosCPP());
	data.byte(kItemframe)  = objectId & 0x00FF;
	data.byte(kObjecttype) = objectId >> 8;
	DynObject *object = getEitherAdCPP();
	object->mapad[0] = 20;
	object->mapad[1] = 255;
	byte prevType2 = data.byte(kObjecttype);
	byte prevFrame2 = data.byte(kItemframe);
	data.byte(kObjecttype) = prevType;
	data.byte(kItemframe) = prevFrame;
	//findInvPosCPP();	// found in the original source, but it seems to be useless
	delPointer();
	object = getEitherAdCPP();
	object->mapad[0] = 4;
	object->mapad[1] = 255;
	object->mapad[2] = data.byte(kLastinvpos);
	data.byte(kObjecttype) = prevType2;
	data.byte(kItemframe) = prevFrame2;
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::useOpened() {
	if (data.byte(kOpenedob) == 255)
		return;	// cannot use opened object

	if (!data.byte(kPickup)) {
		outOfOpen();
		return;
	}

	uint16 objectId = getSegment(data.word(kBuffers)).word(findOpenPos());

	if ((objectId & 0x00FF) != 255) {
		swapWithOpen();
		return;
	}

	if (data.byte(kPickup) != 1) {
		blank();
		return;
	}

	objectId = (data.byte(kObjecttype) << 8) | data.byte(kItemframe);
	if (objectId == data.word(kOldsubject)) {
		if (data.byte(kCommandtype) != 227) {
			data.byte(kCommandtype) = 227;
			data.word(kOldsubject) = objectId;
			commandWithOb(35, data.byte(kObjecttype), data.byte(kItemframe));
		}
	} else {
		data.word(kOldsubject) = objectId;
		commandWithOb(35, data.byte(kObjecttype), data.byte(kItemframe));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	if (isItWorn(getEitherAdCPP())) {
		wornError();
		return;
	}

	delPointer();

	if (data.byte(kItemframe) == data.byte(kOpenedob) &&
		data.byte(kObjecttype) == data.byte(kOpenedtype)) {
		errorMessage1();
		return;
	}

	if (!checkObjectSizeCPP())
		return;

	data.byte(kPickup) = 0;
	DynObject *object = getEitherAdCPP();
	object->mapad[0] = data.byte(kOpenedtype);
	object->mapad[1] = data.byte(kOpenedob);
	object->mapad[2] = data.byte(kLastinvpos);
	object->mapad[3] = data.byte(kReallocation);
	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::outOfOpen() {
	if (data.byte(kOpenedob) == 255)
		return;	// cannot use opened object

	uint16 objectId = getSegment(data.word(kBuffers)).word(findOpenPos());

	if ((objectId & 0x00FF) == 255) {
		blank();
		return;
	}

	if (objectId == data.word(kOldsubject)) {
		if (data.byte(kCommandtype) != 228) {
			data.byte(kCommandtype) = 228;
			data.word(kOldsubject) = objectId;
			commandWithOb(36, objectId >> 8, objectId & 0x00FF);
		}
	} else {
		data.word(kOldsubject) = objectId;
		commandWithOb(36, objectId >> 8, objectId & 0x00FF);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;	// notletgo4

	if (data.word(kMousebutton) != 1) {
		if (data.word(kMousebutton) != 2)
			return;	// notletgo4

		reExFromOpen();
		return;
	}

	delPointer();
	data.byte(kPickup) = 1;
	objectId = getSegment(data.word(kBuffers)).word(findOpenPos());
	data.byte(kObjecttype) = objectId >> 8;
	data.byte(kItemframe)  = objectId & 0xFF;

	if (data.byte(kObjecttype) != 4) {
		transferToEx();
		data.byte(kItemframe) = al;
		data.byte(kObjecttype) = 4;
	}

	DynObject *object = getEitherAdCPP();
	object->mapad[0] = 20;
	object->mapad[1] = 255;

	fillOpen();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamGenContext::swapWithOpen() {
	uint16 subject = (data.byte(kObjecttype) << 8) | data.byte(kItemframe);
	if (subject == data.word(kOldsubject)) {
		if (data.byte(kCommandtype) != 242) {
			data.byte(kCommandtype) = 242;
			data.word(kOldsubject) = subject;
			commandWithOb(34, data.byte(kObjecttype), data.byte(kItemframe));
		}
	} else {
		data.word(kOldsubject) = subject;
		commandWithOb(34, data.byte(kObjecttype), data.byte(kItemframe));
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	if (isItWorn(getEitherAdCPP())) {
		wornError();
		return;
	}

	delPointer();

	if (data.byte(kItemframe) == data.byte(kOpenedob) &&
		data.byte(kObjecttype) == data.byte(kOpenedtype)) {
		errorMessage1();
		return;
	}

	if (!checkObjectSizeCPP())
		return;

	byte prevType = data.byte(kObjecttype);
	byte prevFrame = data.byte(kItemframe);
	uint16 objectId = getSegment(data.word(kBuffers)).word(findOpenPos());
	data.byte(kObjecttype) = objectId >> 8;
	data.byte(kItemframe)  = objectId & 0xFF;

	if (data.byte(kObjecttype) != 4) {
		transferToEx();
		data.byte(kItemframe) = al;
		data.byte(kObjecttype) = 4;
	}

	DynObject *object = getEitherAdCPP();
	object->mapad[0] = 20;
	object->mapad[1] = 255;

	byte prevType2 = data.byte(kObjecttype);
	byte prevFrame2 = data.byte(kItemframe);
	data.byte(kObjecttype) = prevType;
	data.byte(kItemframe) = prevFrame;
	//findOpenPos();	// was in the original source, looks to be unused
	object = getEitherAdCPP();
	object->mapad[0] = data.byte(kOpenedtype);
	object->mapad[1] = data.byte(kOpenedob);
	object->mapad[2] = data.byte(kLastinvpos);
	object->mapad[3] = data.byte(kReallocation);
	data.byte(kObjecttype) = prevType2;
	data.byte(kItemframe) = prevFrame2;
	fillOpen();
	fillRyan();
	underTextLine();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

uint16 DreamBase::findOpenPos() {
	uint16 pos = (data.word(kMousex) - kInventx) / kItempicsize;
	data.byte(kLastinvpos) = pos & 0xFF;

	return pos * 2 + kOpeninvlist;	// return the object position in the inventory data
}

} // End of namespace DreamGen
