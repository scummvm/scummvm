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

struct MonitorKeyEntry {
	uint8 b0;
	uint8 b1;
	char  b2[24];
};

void DreamGenContext::useMon() {
	data.byte(kLasttrigger) = 0;
	memset(cs.ptr(kCurrentfile+1, 0), ' ', 12);
	memset(cs.ptr(offset_operand1+1, 0), ' ', 12);

	MonitorKeyEntry *monitorKeyEntries = (MonitorKeyEntry *)cs.ptr(offset_keys, 0);
	monitorKeyEntries[0].b0 = 1;
	monitorKeyEntries[1].b0 = 0;
	monitorKeyEntries[2].b0 = 0;
	monitorKeyEntries[3].b0 = 0;

	createPanel();
	showPanel();
	showIcon();
	drawFloor();
	getRidOfAll();
	loadIntoTemp("DREAMWEB.G03"); // mon. graphic name
	loadPersonal();
	loadNews();
	loadCart();
	loadTempCharset("DREAMWEB.C01"); // character set 2
	printOuterMon();
	initialMonCols();
	printLogo();
	workToScreen();
	turnOnPower();
	fadeupYellows();
	fadeupMonFirst();
	data.word(kMonadx) = 76;
	data.word(kMonady) = 141;
	monMessage(1);
	hangOnCurs(120);
	monMessage(2);
	randomAccess(60);
	monMessage(3);
	hangOnCurs(100);
	printLogo();
	scrollMonitor();
	data.word(kBufferin) = 0;
	data.word(kBufferout) = 0;
	do {
		di = data.word(kMonadx);
		bx = data.word(kMonady);
		push(di);
		push(bx);
		input();
		bx = pop();
		di = pop();
		data.word(kMonadx) = di;
		data.word(kMonady) = bx;
		execCommand();
		if (quitRequested()) //TODO : Check why it crashes when put before the execcommand
			break;
	} while (al == 0);
	getRidOfTemp();
	getRidOfTempCharset();
	deallocateMem(data.word(kTextfile1));
	deallocateMem(data.word(kTextfile2));
	deallocateMem(data.word(kTextfile3));
	data.byte(kGetback) = 1;
	playChannel1(26);
	data.byte(kManisoffscreen) = 0;
	restoreAll();
	redrawMainScrn();
	workToScreenM();
}

void DreamGenContext::printLogo() {
	showFrame(tempGraphics(), 56, 32, 0, 0);
	showCurrentFile();
}

void DreamGenContext::input() {
	char *inputLine = (char *)cs.ptr(kInputline, 64);
	memset(inputLine, 0, 64);
	data.word(kCurpos) = 0;
	printChar(engine->tempCharset(), data.word(kMonadx), data.word(kMonady), '>', 0, NULL, NULL);
	multiDump(data.word(kMonadx), data.word(kMonady), 6, 8);
	data.word(kMonadx) += 6;
	data.word(kCurslocx) = data.word(kMonadx);
	data.word(kCurslocy) = data.word(kMonady);
	while (true) {
		printCurs();
		vSync();
		delCurs();
		readKey();
		if (quitRequested())
			return;
		uint8 currentKey = data.byte(kCurrentkey);
		if (currentKey == 0)
			continue;
		if (currentKey == 13)
			return;
		if (currentKey == 8) {
			if (data.word(kCurpos) > 0)
				delChar();
			continue;
		}
		if (data.word(kCurpos) == 28)
			continue;
		if ((currentKey == 32) && (data.word(kCurpos) == 0))
			continue;
		al = currentKey;
		makeCaps();
		currentKey = al;
		inputLine[data.word(kCurpos) * 2 + 0] = currentKey;
		if (currentKey > 'Z')
			continue;
		multiGet(getSegment(data.word(kMapstore)).ptr(data.word(kCurpos) * 256, 0), data.word(kMonadx), data.word(kMonady), 8, 8);
		uint8 charWidth;
		printChar(engine->tempCharset(), data.word(kMonadx), data.word(kMonady), currentKey, 0, &charWidth, NULL);
		inputLine[data.word(kCurpos) * 2 + 1] = charWidth;
		data.word(kMonadx) += charWidth;
		++data.word(kCurpos);
		data.word(kCurslocx) += charWidth;
	}
}

void DreamGenContext::delChar() {
	char *inputLine = (char *)cs.ptr(kInputline, 0);
	--data.word(kCurpos);
	inputLine[data.word(kCurpos) * 2] = 0;
	uint8 width = inputLine[data.word(kCurpos) * 2 + 1];
	data.word(kMonadx) -= width;
	data.word(kCurslocx) -= width;
	uint16 offset = data.word(kCurpos);
	offset = ((offset & 0x00ff) << 8) | ((offset & 0xff00) >> 8);
	multiPut(getSegment(data.word(kMapstore)).ptr(offset, 0), data.word(kMonadx), data.word(kMonady), 8, 8);
	multiDump(data.word(kMonadx), data.word(kMonady), al, 8);
}

void DreamGenContext::printCurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiGet(textUnder(), x, y, 6, height);
	++data.word(kMaintimer);
	if ((data.word(kMaintimer) & 16) == 0)
		showFrame(engine->tempCharset(), x, y, '/' - 32, 0);
	multiDump(x - 6, y, 12, height);
}

void DreamGenContext::delCurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 width = 6;
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiPut(textUnder(), x, y, width, height);
	multiDump(x, y, width, height);
}

void DreamGenContext::hangOnCurs() {
	hangOnCurs(cx);
}

void DreamGenContext::scrollMonitor() {
	printLogo();
	printUnderMon();
	workToScreenCPP();
	playChannel1(25);
}

void DreamGenContext::showCurrentFile() {
	uint16 x = 178; // TODO: Looks like this hardcoded constant in the asm doesn't match the frame
	const char *currentFile = (const char *)cs.ptr(kCurrentfile+1, 0);
	while (*currentFile) {
		char c = *currentFile++;
		c = engine->modifyChar(c);
		printChar(engine->tempCharset(), &x, 37, c, 0, NULL, NULL);
	}
}

void DreamGenContext::accessLightOn() {
	showFrame(tempGraphics(), 74, 182, 8, 0);
	multiDump(74, 182, 12, 8);
}

void DreamGenContext::accessLightOff() {
	showFrame(tempGraphics(), 74, 182, 7, 0);
	multiDump(74, 182, 12, 8);
}

void DreamGenContext::randomAccess() {
	randomAccess(cx);
}

void DreamGenContext::randomAccess(uint16 count) {
	for (uint16 i = 0; i < count; ++i) {
		vSync();
		vSync();
		uint16 v = engine->randomNumber() & 15;
		if (v < 10)
			accessLightOff();
		else
			accessLightOn();
	}
	accessLightOff();
}

void DreamGenContext::monMessage() {
	monMessage(al);
}

void DreamGenContext::monMessage(uint8 index) {
	assert(index > 0);
	const char *string = (const char *)getSegment(data.word(kTextfile1)).ptr(kTextstart, 0);
	for (uint8 i = 0; i < index; ++i) {
		while (*string++ != '+') {
		}
	}
	monPrint(string);
}

void DreamGenContext::netError() {
	monMessage(5);
	scrollMonitor();
}

void DreamGenContext::powerLightOn() {
	showFrame(tempGraphics(), 257+4, 182, 6, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamGenContext::powerLightOff() {
	showFrame(tempGraphics(), 257+4, 182, 5, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamGenContext::turnOnPower() {
	for (size_t i = 0; i < 3; ++i) {
		powerLightOn();
		hangOn(30);
		powerLightOff();
		hangOn(30);
	}
	powerLightOn();
}

void DreamGenContext::loadPersonal() {
	if (data.byte(kLocation) == 0 || data.byte(kLocation) == 42)
		data.word(kTextfile1) = standardLoad("DREAMWEB.T01"); // monitor file 1
	else
		data.word(kTextfile1) = standardLoad("DREAMWEB.T02"); // monitor file 2
}

void DreamGenContext::loadNews() {
	// textfile2 holds information accessible by anyone
	if (data.byte(kNewsitem) == 0)
		data.word(kTextfile2) = standardLoad("DREAMWEB.T10"); // monitor file 10
	else if (data.byte(kNewsitem) == 1)
		data.word(kTextfile2) = standardLoad("DREAMWEB.T11"); // monitor file 11
	else if (data.byte(kNewsitem) == 2)
		data.word(kTextfile2) = standardLoad("DREAMWEB.T12"); // monitor file 12
	else
		data.word(kTextfile2) = standardLoad("DREAMWEB.T13"); // monitor file 13
}

void DreamGenContext::loadCart() {
	lookInInterface();

	if (al == 0)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T20"); // monitor file 20
	else if (al == 1)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T21"); // monitor file 21
	else if (al == 2)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T22"); // monitor file 22
	else if (al == 3)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T23"); // monitor file 23
	else
		data.word(kTextfile3) = standardLoad("DREAMWEB.T24"); // monitor file 24
}

} /*namespace dreamgen */

