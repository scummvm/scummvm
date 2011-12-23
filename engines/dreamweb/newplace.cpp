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

void DreamGenContext::newPlace() {
	if (data.byte(kNeedtotravel) == 1) {
		data.byte(kNeedtotravel) = 0;
		selectLocation();
	} else if (data.byte(kAutolocation) != 0xFF) {
		data.byte(kNewlocation) = data.byte(kAutolocation);
		data.byte(kAutolocation) = 0xFF;
	}
}

void DreamGenContext::selectLocation() {
	data.byte(kInmaparea) = 0;
	clearBeforeLoad();
	data.byte(kGetback) = 0;
	data.byte(kPointerframe) = 22;
	readCityPic();
	showCity();
	getRidOfTemp();
	readDestIcon();
	loadTravelText();
	showPanel();
	showMan();
	showArrows();
	showExit();
	locationPic();
	underTextLine();
	data.byte(kCommandtype) = 255;
	readMouse();
	data.byte(kPointerframe) = 0;
	showPointer();
	workToScreenCPP();
	playChannel0(9, 255);
	data.byte(kNewlocation) = 255;

	while (data.byte(kNewlocation) == 255) {
		if (quitRequested())
			break;

		delPointer();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();

		if (data.byte(kGetback) == 1)
			break;

		RectWithCallback<DreamGenContext> destList[] = {
			{ 238,258,4,44,&DreamGenContext::nextDest },
			{ 104,124,4,44,&DreamGenContext::lastDest },
			{ 280,308,4,44,&DreamGenContext::lookAtPlace },
			{ 104,216,138,192,&DreamGenContext::destSelect },
			{ 273,320,157,198,&DreamBase::getBack1 },
			{ 0,320,0,200,&DreamBase::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(destList);
	}

	if (quitRequested() || data.byte(kGetback) == 1 || data.byte(kNewlocation) == data.byte(kLocation)) {
		data.byte(kNewlocation) = data.byte(kReallocation);
		data.byte(kGetback) = 0;
	}

	getRidOfTemp();
	getRidOfTemp2();
	getRidOfTemp3();
	deallocateMem(data.word(kTraveltext));
}

void DreamGenContext::showCity() {
	clearWork();
	showFrame(tempGraphics(), 57, 32, 0, 0);
	showFrame(tempGraphics(), 120+57, 32, 1, 0);
}

void DreamGenContext::lookAtPlace() {
	if (data.byte(kCommandtype) != 224) {
		data.byte(kCommandtype) = 224;
		commandOnly(27);
	}

	if (!(data.word(kMousebutton) & 1) ||
		data.word(kMousebutton) == data.word(kOldbutton) ||
		data.byte(kDestpos) >= 15)
		return; // noinfo

	delPointer();
	delTextLine();
	getUnderCentre();
	showFrame(tempGraphics3(), 60, 72, 0, 0);
	showFrame(tempGraphics3(), 60, 72 + 55, 4, 0);
	if (data.byte(kForeignrelease))
		showFrame(tempGraphics3(), 60, 72+55+21, 4, 0);

	uint16 offset = kTextstart + getSegment(data.word(kTraveltext)).word(data.byte(kDestpos) * 2);
	const uint8 *string = getSegment(data.word(kTraveltext)).ptr(offset, 0);
	findNextColon(&string);
	uint16 y = (data.byte(kForeignrelease)) ? 84 + 4 : 84;
	printDirect(&string, 63, &y, 191, 191 & 1);
	workToScreenM();
	hangOnP(500);
	data.byte(kPointermode) = 0;
	data.byte(kPointerframe) = 0;
	putUnderCentre();
	workToScreenM();
}

void DreamBase::getUnderCentre() {
	multiGet(mapStore(), 58, 72, 254, 110);
}

void DreamBase::putUnderCentre() {
	multiPut(mapStore(), 58, 72, 254, 110);
}

// TODO: put Locationpic here

// TODO: put Getdestinfo here

void DreamBase::showArrows() {
	showFrame(tempGraphics(), 116 - 12, 16, 0, 0);
	showFrame(tempGraphics(), 226 + 12, 16, 1, 0);
	showFrame(tempGraphics(), 280, 14, 2, 0);
}

void DreamGenContext::nextDest() {
	if (data.byte(kCommandtype) != 218) {
		data.byte(kCommandtype) = 218;
		commandOnly(28);
	}

	if (!(data.word(kMousebutton) & 1) || data.word(kOldbutton) == 1)
		return;	// nodu

	do {
		data.byte(kDestpos)++;
		if (data.byte(kDestpos) == 15)
			data.byte(kDestpos) = 0;	// last destination

		getDestInfo();
	} while (al == 0);

	data.byte(kNewtextline) = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamGenContext::lastDest() {
	if (data.byte(kCommandtype) != 219) {
		data.byte(kCommandtype) = 219;
		commandOnly(29);
	}

	if (!(data.word(kMousebutton) & 1) || data.word(kOldbutton) == 1)
		return;	// nodd

	do {
		data.byte(kDestpos)--;
		if (data.byte(kDestpos) == 0xFF)
			data.byte(kDestpos) = 15;	// first destination

		getDestInfo();
	} while (al == 0);

	data.byte(kNewtextline) = 1;
	delTextLine();
	delPointer();
	showPanel();
	showMan();
	showArrows();
	locationPic();
	underTextLine();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamGenContext::destSelect() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(30);
	}

	if (!(data.word(kMousebutton) & 1) || data.word(kOldbutton) == 1)
		return;	// notrav

	getDestInfo();
	data.byte(kNewlocation) = data.byte(kDestpos);
}

uint8 DreamBase::getLocation(uint8 index) {
	return data.byte(kRoomscango + index);
}

void DreamBase::setLocation(uint8 index) {
	data.byte(kRoomscango + index) = 1;
}

void DreamBase::resetLocation(uint8 index) {
	if (index == 5) {
		// delete hotel
		purgeALocation(5);
		purgeALocation(21);
		purgeALocation(22);
		purgeALocation(27);
	} else if (index == 8) {
		// delete TV studio
		purgeALocation(8);
		purgeALocation(28);
	} else if (index == 6) {
		// delete sarters
		purgeALocation(6);
		purgeALocation(20);
		purgeALocation(25);
	} else if (index == 13) {
		// delete boathouse
		purgeALocation(13);
		purgeALocation(29);
	}

	data.byte(kRoomscango + index) = 0;
}

void DreamGenContext::readDestIcon() {
	loadIntoTemp("DREAMWEB.G05");
	loadIntoTemp2("DREAMWEB.G06");
	loadIntoTemp3("DREAMWEB.G08");
}

void DreamGenContext::readCityPic() {
	loadIntoTemp("DREAMWEB.G04");
}



} // End of namespace DreamGen
