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

namespace DreamWeb {

void DreamWebEngine::endGame() {
	loadTempText("T83");
	monkSpeaking();
	if (_quitRequested)
		return;
	gettingShot();
	getRidOfTempText();
	_volumeTo = 7;
	_volumeDirection = 1;
	hangOn(200);
}

void DreamWebEngine::monkSpeaking() {
	_roomsSample = 35;
	loadRoomsSample();
	GraphicsFile graphics;
	loadGraphicsFile(graphics, "G15");
	clearWork();
	showFrame(graphics, 160, 72, 0, 128);	// show monk
	workToScreen();
	_volume = 7;
	_volumeDirection = -1;
	_volumeTo = hasSpeech() ? 5 : 0;
	playChannel0(12, 255);
	fadeScreenUps();
	hangOn(300);

	// TODO: Subtitles+speech mode
	if (hasSpeech()) {
		for (int i = 40; i < 48; i++) {
			loadSpeech('T', 83, 'T', i);

			playChannel1(50 + 12);

			do {
				waitForVSync();
				if (_quitRequested)
					return;
			} while (_channel1Playing != 255);
		}
	} else {
		for (int i = 40; i <= 44; i++) {
			uint8 printResult = 0;
			const uint8 *string = getTextInFile1(i);

			do {
				uint16 y = 140;
				printResult = printDirect(&string, 36, &y, 239, 239 & 1);
				workToScreen();
				clearWork();
				showFrame(graphics, 160, 72, 0, 128);	// show monk
				hangOnP(240);
				if (_quitRequested)
					return;
			} while (printResult != 0);
		}
	}

	_volumeDirection = 1;
	_volumeTo = 7;
	fadeScreenDowns();
	hangOn(300);
	graphics.clear();
}

void DreamWebEngine::gettingShot() {
	_newLocation = 55;
	clearPalette();
	loadIntroRoom();
	fadeScreenUps();
	_volumeTo = 0;
	_volumeDirection = -1;
	runEndSeq();
	clearBeforeLoad();
}

void DreamWebEngine::bibleQuote() {
	initGraphics(640, 480, true);

	showPCX("I00");
	fadeScreenUps();

	hangOne(80);
	if (_lastHardKey == 1) {
		_lastHardKey = 0;
		return; // "biblequotearly"
	}

	hangOne(560);
	if (_lastHardKey == 1) {
		_lastHardKey = 0;
		return; // "biblequotearly"
	}

	fadeScreenDowns();

	hangOne(200);
	if (_lastHardKey == 1) {
		_lastHardKey = 0;
		return; // "biblequotearly"
	}

	cancelCh0();

	_lastHardKey = 0;
}

void DreamWebEngine::hangOne(uint16 delay) {
	do {
		vSync();
		if (_lastHardKey == 1)
			return; // "hangonearly"
	} while	(--delay);
}

void DreamWebEngine::intro() {
	loadTempText("T82");
	loadPalFromIFF();
	setMode();
	_newLocation = 50;
	clearPalette();
	loadIntroRoom();
	_volume = 7;
	_volumeDirection = -1;
	_volumeTo = hasSpeech() ? 4 : 0;
	playChannel0(12, 255);
	fadeScreenUps();
	runIntroSeq();

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	_newLocation = 52;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	_newLocation = 53;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	allPalette();
	_newLocation = 54;
	loadIntroRoom();
	runIntroSeq();

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "introearly"
	}

	getRidOfTempText();
	clearBeforeLoad();

	_lastHardKey =  0;
}

void DreamWebEngine::runIntroSeq() {
	_getBack = 0;

	do {
		vSync();

		if (_lastHardKey == 1)
			break;

		spriteUpdate();
		vSync();

		if (_lastHardKey == 1)
			break;

		delEverything();
		printSprites();
		reelsOnScreen();
		afterIntroRoom();
		useTimedText();
		vSync();

		if (_lastHardKey == 1)
			break;

		dumpMap();
		dumpTimedText();
		vSync();

		if (_lastHardKey == 1)
			break;

	} while (_getBack != 1);


	if (_lastHardKey == 1) {
		getRidOfTempText();
		clearBeforeLoad();
	}

	// These were not called in this program arc
	// in the original code.. Bug?
	//getRidOfTempText();
	//clearBeforeLoad();
}

void DreamWebEngine::runEndSeq() {
	atmospheres();
	_getBack = 0;

	do {
		vSync();
		spriteUpdate();
		vSync();
		delEverything();
		printSprites();
		reelsOnScreen();
		afterIntroRoom();
		useTimedText();
		vSync();
		dumpMap();
		dumpTimedText();
		vSync();
	} while (_getBack != 1 && !_quitRequested);
}

void DreamWebEngine::loadIntroRoom() {
	_introCount = 0;
	_vars._location = 255;
	loadRoom();
	_mapOffsetX = 72;
	_mapOffsetY = 16;
	clearSprites();
	_vars._throughDoor = 0;
	_currentKey = '0';
	_mainMode = 0;
	clearWork();
	_vars._newObs = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreen();
}

void DreamWebEngine::set16ColPalette() {
}

void DreamWebEngine::realCredits() {
	_roomsSample = 33;
	loadRoomsSample();
	_volume = 0;

	initGraphics(640, 480, true);
	hangOn(35);

	showPCX("I01");
	playChannel0(12, 0);

	hangOne(2);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	showPCX("I02");
	playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	showPCX("I03");
	playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	showPCX("I04");
	playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	showPCX("I05");
	playChannel0(12, 0);
	hangOne(2);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	showPCX("I06");
	fadeScreenUps();
	hangOne(60);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	playChannel0(13, 0);
	hangOne(350);

	if (_lastHardKey == 1) {
		_lastHardKey =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	_lastHardKey =  0;
}

} // End of namespace DreamWeb
