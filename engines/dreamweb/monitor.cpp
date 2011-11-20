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

void DreamGenContext::usemon() {
	data.byte(kLasttrigger) = 0;
	memset(cs.ptr(kCurrentfile+1, 0), ' ', 12);
	memset(cs.ptr(offset_operand1+1, 0), ' ', 12);

	MonitorKeyEntry *monitorKeyEntries = (MonitorKeyEntry *)cs.ptr(offset_keys, 0);
	monitorKeyEntries[0].b0 = 1;
	monitorKeyEntries[1].b0 = 0;
	monitorKeyEntries[2].b0 = 0;
	monitorKeyEntries[3].b0 = 0;

	createpanel();
	showpanel();
	showicon();
	drawfloor();
	getridofall();
	loadintotemp("DREAMWEB.G03");
	loadpersonal();
	loadnews();
	loadcart();
	loadtempcharset("DREAMWEB.C01");
	printoutermon();
	initialmoncols();
	printlogo();
	worktoscreen();
	turnonpower();
	fadeupyellows();
	fadeupmonfirst();
	data.word(kMonadx) = 76;
	data.word(kMonady) = 141;
	monmessage(1);
	hangoncurs(120);
	monmessage(2);
	randomaccess(60);
	monmessage(3);
	hangoncurs(100);
	printlogo();
	scrollmonitor();
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
		execcommand();
		if (data.byte(kQuitrequested)) //TODO : Check why it crashes when put before the execcommand
			break;
	} while (al == 0);
	getridoftemp();
	getridoftempcharset();
	deallocatemem(data.word(kTextfile1));
	deallocatemem(data.word(kTextfile2));
	deallocatemem(data.word(kTextfile3));
	data.byte(kGetback) = 1;
	al = 26;
	playchannel1();
	data.byte(kManisoffscreen) = 0;
	restoreall();
	redrawmainscrn();
	worktoscreenm();
}

void DreamGenContext::printlogo() {
	showframe(tempGraphics(), 56, 32, 0, 0);
	showcurrentfile();
}

void DreamGenContext::input() {
	char *inputLine = (char *)cs.ptr(kInputline, 64);
	memset(inputLine, 0, 64);
	data.word(kCurpos) = 0;
	printchar(tempCharset(), data.word(kMonadx), data.word(kMonady), '>', 0, NULL, NULL);
	multidump(data.word(kMonadx), data.word(kMonady), 6, 8);
	data.word(kMonadx) += 6;
	data.word(kCurslocx) = data.word(kMonadx);
	data.word(kCurslocy) = data.word(kMonady);
	while (true) {
		printcurs();
		vsync();
		delcurs();
		readkey();
		if (data.byte(kQuitrequested))
			return;
		uint8 currentKey = data.byte(kCurrentkey);
		if (currentKey == 0)
			continue;
		if (currentKey == 13)
			return;
		if (currentKey == 8) {
			if (data.word(kCurpos) > 0)
				delchar();
			continue;
		}
		if (data.word(kCurpos) == 28)
			continue;
		if ((currentKey == 32) && (data.word(kCurpos) == 0))
			continue;
		al = currentKey;
		makecaps();
		currentKey = al;
		inputLine[data.word(kCurpos) * 2 + 0] = currentKey;
		if (currentKey > 'Z')
			continue;
		multiget(segRef(data.word(kMapstore)).ptr(data.word(kCurpos) * 256, 0), data.word(kMonadx), data.word(kMonady), 8, 8);
		uint8 charWidth;
		printchar(tempCharset(), data.word(kMonadx), data.word(kMonady), currentKey, 0, &charWidth, NULL);
		inputLine[data.word(kCurpos) * 2 + 1] = charWidth;
		data.word(kMonadx) += charWidth;
		++data.word(kCurpos);
		data.word(kCurslocx) += charWidth;
	}
}

void DreamGenContext::printcurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiget(textUnder(), x, y, 6, height);
	++data.word(kMaintimer);
	if ((data.word(kMaintimer) & 16) == 0)
		showframe(tempCharset(), x, y, '/' - 32, 0);
	multidump(x - 6, y, 12, height);
}

void DreamGenContext::delcurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 width = 6;
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiput(textUnder(), x, y, width, height);
	multidump(x, y, width, height);
}

void DreamGenContext::hangoncurs() {
	hangoncurs(cx);
}

void DreamGenContext::scrollmonitor() {
	printlogo();
	printundermon();
	workToScreenCPP();
	al = 25;
	playchannel1();
}

void DreamGenContext::showcurrentfile() {
	uint16 x = 178; // TODO: Looks like this hardcoded constant in the asm doesn't match the frame
	const char *currentFile = (const char *)cs.ptr(kCurrentfile+1, 0);
	while (*currentFile) {
		char c = *currentFile++;
		c = engine->modifyChar(c);
		printchar(tempCharset(), &x, 37, c, 0, NULL, NULL);
	}
}

void DreamGenContext::accesslighton() {
	showframe(tempGraphics(), 74, 182, 8, 0);
	multidump(74, 182, 12, 8);
}

void DreamGenContext::accesslightoff() {
	showframe(tempGraphics(), 74, 182, 7, 0);
	multidump(74, 182, 12, 8);
}

void DreamGenContext::randomaccess() {
	randomaccess(cx);
}

void DreamGenContext::randomaccess(uint16 count) {
	for (uint16 i = 0; i < count; ++i) {
		vsync();
		vsync();
		uint16 v = engine->randomNumber() & 15;
		if (v < 10)
			accesslightoff();
		else
			accesslighton();
	}
	accesslightoff();
}

void DreamGenContext::monmessage() {
	monmessage(al);
}

void DreamGenContext::monmessage(uint8 index) {
	assert(index > 0);
	const char *string = (const char *)segRef(data.word(kTextfile1)).ptr(kTextstart, 0);
	for (uint8 i = 0; i < index; ++i) {
		while (*string++ != '+') {
		}
	}
	monprint(string);
}

void DreamGenContext::neterror() {
	monmessage(5);
	scrollmonitor();
}

} /*namespace dreamgen */

