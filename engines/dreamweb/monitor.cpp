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
	uint8 keyAssigned;
	char  username[12];
	char  password[12];
};

// New monitor key list
static MonitorKeyEntry monitorKeyEntries[4] = {
	{ 1, "PUBLIC",  "PUBLIC"      },
	{ 0, "RYAN",    "BLACKDRAGON" },
	{ 0, "LOUIS",   "HENDRIX"     },
	{ 0, "BECKETT", "SEPTIMUS"    }
};

void DreamGenContext::useMon() {
	data.byte(kLasttrigger) = 0;
	memset(data.ptr(kCurrentfile+1, 0), ' ', 12);
	memset(data.ptr(offset_operand1+1, 0), ' ', 12);

	monitorKeyEntries[0].keyAssigned = 1;
	monitorKeyEntries[1].keyAssigned = 0;
	monitorKeyEntries[2].keyAssigned = 0;
	monitorKeyEntries[3].keyAssigned = 0;

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
	workToScreenCPP();
	turnOnPower();
	fadeUpYellows();
	fadeUpMonFirst();
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
	bool stop = false;
	do {
		uint16 oldMonadx = data.word(kMonadx);
		uint16 oldMonady = data.word(kMonady);
		input();
		data.word(kMonadx) = oldMonadx;
		data.word(kMonady) = oldMonady;
		stop = execCommand();
		if (quitRequested()) //TODO : Check why it crashes when put before the execcommand
			break;
	} while (!stop);
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

bool DreamGenContext::execCommand() {
	static const char *comlist[] = {
		"EXIT",
		"HELP",
		"LIST",
		"READ",
		"LOGON",
		"KEYS"
	};

	const char *inputLine = (const char *)data.ptr(kInputline, 64);
	if (*inputLine == 0) {
		// No input
		scrollMonitor();
		return false;
	}

	int cmd;
	bool done = false;
	// Loop over all commands in the list and see if we get a match
	for (cmd = 0; cmd < ARRAYSIZE(comlist); ++cmd) {
		const char *cmdStr = comlist[cmd];
		const char *inputStr = inputLine;
		// Compare the command, char by char, to see if we get a match.
		// We only care about the prefix matching, though.
		char inputChar, cmdChar;
		do {
			inputChar = *inputStr; inputStr += 2;
			cmdChar = *cmdStr++;
			if (cmdChar == 0) {
				done = true;
				break;
			}
		} while (inputChar == cmdChar);

		if (done)
			break;
	}

	// Execute the selected command
	switch (cmd) {
	case 0:
		return true;
	case 1:
		monMessage(6);
		break;
	case 2:
		dirCom();
		break;
	case 3:
		read();
		break;
	case 4:
		signOn();
		break;
	case 5:
		showKeys();
		break;
	default:
		netError();
		break;
	}
	return false;
}



void DreamBase::monitorLogo() {
	if (data.byte(kLogonum) != data.byte(kOldlogonum)) {
		data.byte(kOldlogonum) = data.byte(kLogonum);
		//fadeDownMon(); // FIXME: Commented out in ASM
		printLogo();
		printUnderMon();
		workToScreenCPP();
		printLogo();
		//fadeUpMon(); // FIXME: Commented out in ASM
		printLogo();
		playChannel1(26);
		randomAccess(20);
	} else {
		printLogo();
	}
}

void DreamBase::printLogo() {
	showFrame(tempGraphics(), 56, 32, 0, 0);
	showCurrentFile();
}

void DreamBase::input() {
	char *inputLine = (char *)data.ptr(kInputline, 64);
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
		currentKey = makeCaps(currentKey);
		inputLine[data.word(kCurpos) * 2 + 0] = currentKey;
		if (currentKey > 'Z')
			continue;
		multiGet(mapStore() + data.word(kCurpos) * 256, data.word(kMonadx), data.word(kMonady), 8, 8);
		uint8 charWidth;
		printChar(engine->tempCharset(), data.word(kMonadx), data.word(kMonady), currentKey, 0, &charWidth, NULL);
		inputLine[data.word(kCurpos) * 2 + 1] = charWidth;
		data.word(kMonadx) += charWidth;
		++data.word(kCurpos);
		data.word(kCurslocx) += charWidth;
	}
}

void DreamGenContext::makeCaps() {
	al = makeCaps(al);
}

byte DreamBase::makeCaps(byte c) {
	// TODO: Replace calls to this by toupper() ?
	if (c >= 'a')
		c -= 'a' - 'A'; // = 32
	return c;
}

void DreamBase::delChar() {
	char *inputLine = (char *)data.ptr(kInputline, 0);
	--data.word(kCurpos);
	inputLine[data.word(kCurpos) * 2] = 0;
	uint8 width = inputLine[data.word(kCurpos) * 2 + 1];
	data.word(kMonadx) -= width;
	data.word(kCurslocx) -= width;
	uint16 offset = data.word(kCurpos);
	offset = ((offset & 0x00ff) << 8) | ((offset & 0xff00) >> 8);
	multiPut(mapStore() + offset, data.word(kMonadx), data.word(kMonady), 8, 8);
	multiDump(data.word(kMonadx), data.word(kMonady), 8, 8);
}

void DreamBase::printCurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiGet(_textUnder, x, y, 6, height);
	++data.word(kMaintimer);
	if ((data.word(kMaintimer) & 16) == 0)
		showFrame(engine->tempCharset(), x, y, '/' - 32, 0);
	multiDump(x - 6, y, 12, height);
}

void DreamBase::delCurs() {
	uint16 x = data.word(kCurslocx);
	uint16 y = data.word(kCurslocy);
	uint16 width = 6;
	uint16 height;
	if (data.byte(kForeignrelease)) {
		y -= 3;
		height = 11;
	} else
		height = 8;
	multiPut(_textUnder, x, y, width, height);
	multiDump(x, y, width, height);
}

void DreamBase::scrollMonitor() {
	printLogo();
	printUnderMon();
	workToScreenCPP();
	playChannel1(25);
}

void DreamBase::showCurrentFile() {
	uint16 x = 178; // TODO: Looks like this hardcoded constant in the asm doesn't match the frame
	const char *currentFile = (const char *)data.ptr(kCurrentfile+1, 0);
	while (*currentFile) {
		char c = *currentFile++;
		c = engine->modifyChar(c);
		printChar(engine->tempCharset(), &x, 37, c, 0, NULL, NULL);
	}
}

void DreamBase::accessLightOn() {
	showFrame(tempGraphics(), 74, 182, 8, 0);
	multiDump(74, 182, 12, 8);
}

void DreamBase::accessLightOff() {
	showFrame(tempGraphics(), 74, 182, 7, 0);
	multiDump(74, 182, 12, 8);
}

void DreamGenContext::randomAccess() {
	randomAccess(cx);
}

void DreamBase::randomAccess(uint16 count) {
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

void DreamBase::monMessage(uint8 index) {
	assert(index > 0);
	const char *string = (const char *)getSegment(data.word(kTextfile1)).ptr(kTextstart, 0);
	for (uint8 i = 0; i < index; ++i) {
		while (*string++ != '+') {
		}
	}
	monPrint(string);
}

void DreamBase::netError() {
	monMessage(5);
	scrollMonitor();
}

void DreamBase::powerLightOn() {
	showFrame(tempGraphics(), 257+4, 182, 6, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamBase::powerLightOff() {
	showFrame(tempGraphics(), 257+4, 182, 5, 0);
	multiDump(257+4, 182, 12, 8);
}

void DreamBase::lockLightOn() {
	showFrame(tempGraphics(), 56, 182, 10, 0);
	multiDump(58, 182, 12, 8);
}

void DreamBase::lockLightOff() {
	showFrame(tempGraphics(), 56, 182, 9, 0);
	multiDump(58, 182, 12, 8);
}

void DreamBase::turnOnPower() {
	for (size_t i = 0; i < 3; ++i) {
		powerLightOn();
		hangOn(30);
		powerLightOff();
		hangOn(30);
	}
	powerLightOn();
}

void DreamBase::printOuterMon() {
	showFrame(tempGraphics(), 40, 32, 1, 0);
	showFrame(tempGraphics(), 264, 32, 2, 0);
	showFrame(tempGraphics(), 40, 12, 3, 0);
	showFrame(tempGraphics(), 40, 164, 4, 0);
}

void DreamBase::loadPersonal() {
	if (data.byte(kLocation) == 0 || data.byte(kLocation) == 42)
		data.word(kTextfile1) = standardLoad("DREAMWEB.T01"); // monitor file 1
	else
		data.word(kTextfile1) = standardLoad("DREAMWEB.T02"); // monitor file 2
}

void DreamBase::loadNews() {
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

void DreamBase::loadCart() {
	byte cartridgeId = 0;
	uint16 objectIndex = findSetObject("INTF");
	uint16 cartridgeIndex = checkInside(objectIndex, 1);
	if (cartridgeIndex != kNumexobjects)
		cartridgeId = getExAd(cartridgeIndex)->id[3] + 1;

	if (cartridgeId == 0)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T20"); // monitor file 20
	else if (cartridgeId == 1)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T21"); // monitor file 21
	else if (cartridgeId == 2)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T22"); // monitor file 22
	else if (cartridgeId == 3)
		data.word(kTextfile3) = standardLoad("DREAMWEB.T23"); // monitor file 23
	else
		data.word(kTextfile3) = standardLoad("DREAMWEB.T24"); // monitor file 24
}

void DreamGenContext::showKeys() {
	randomAccess(10);
	scrollMonitor();
	monMessage(18);

	for (int i = 0; i < 4; i++) {
		if (monitorKeyEntries[i].keyAssigned)
			monPrint(monitorKeyEntries[i].username);
	}

	scrollMonitor();
}

void DreamGenContext::getKeyAndLogo() {
	byte newLogo = es.byte(bx + 1) - 48;
	byte keyNum = es.byte(bx + 1 + 2) - 48;
	bx += 1 + 2 + 1;

	if (monitorKeyEntries[keyNum].keyAssigned == 1) {
		// Key OK
		data.byte(kLogonum) = newLogo;
		al = 0;
	} else {
		monMessage(12);	// "Access denied, key required -"
		monPrint(monitorKeyEntries[keyNum].username);
		scrollMonitor();
		al = 1;
	}
}

void DreamGenContext::signOn() {
	parser();

	int8 foundIndex = -1;
	Common::String inputLine = (const char *)data.ptr(offset_operand1 + 1, 0);
	inputLine.trim();

	for (byte i = 0; i < 4; i++) {
		if (inputLine.equalsIgnoreCase(monitorKeyEntries[i].username)) {
			// Check if the key has already been assigned
			if (monitorKeyEntries[i].keyAssigned) {
				monMessage(17);
				return;
			} else {
				foundIndex = i;
				break;
			}
		}
	}

	if (foundIndex == -1) {
		monMessage(13);
		return;
	}

	monMessage(15);

	uint16 prevX = data.word(kMonadx);
	uint16 prevY = data.word(kMonady);
	input();	// password input
	data.word(kMonadx) = prevX;
	data.word(kMonady) = prevY;

	inputLine = (const char *)data.ptr(kInputline, 0);
	inputLine.toUppercase();

	// The entered line has zeroes in-between each character
	uint32 len = strlen(monitorKeyEntries[foundIndex].password);
	bool found = true;

	for (uint32 i = 0; i < len; i++) {
		if (monitorKeyEntries[foundIndex].password[i] != inputLine[i * 2]) {
			found = false;
			break;
		}
	}

	if (!found) {
		scrollMonitor();
		monMessage(16);
	} else {
		monMessage(14);
		monPrint(monitorKeyEntries[foundIndex].username);
		scrollMonitor();
		monitorKeyEntries[foundIndex].keyAssigned = 1;
	}
}

} // End of namespace DreamGen
