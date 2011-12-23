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

namespace DreamGen {

void DreamGenContext::endGame() {
	loadTempText("DREAMWEB.T83");
	monkSpeaking();
	gettingShot();
	getRidOfTempText();
	data.byte(kVolumeto) = 7;
	data.byte(kVolumedirection) = 1;
	hangOn(200);
}

void DreamGenContext::monkSpeaking() {
	// FIXME: This is the CD version only.

	data.byte(kRoomssample) = 35;
	loadRoomsSample();
	loadIntoTemp("DREAMWEB.G15");
	clearWork();
	showFrame(tempGraphics(), 160, 72, 0, 128);	// show monk
	workToScreen();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = (byte)-1;
	data.byte(kVolumeto) = 5;
	playChannel0(12, 255);
	fadeScreenUps();
	hangOn(300);

	for (int i = 40; i <= 48; i++) {
		loadSpeech('T', 83, 'T', i);

		playChannel1(50 + 12);

		do {
			engine->waitForVSync();
		} while (data.byte(kCh1playing) != 255);
	}

	data.byte(kVolumedirection) = 1;
	data.byte(kVolumeto) = 7;
	fadeScreenDowns();
	hangOn(300);
	getRidOfTemp();
}

void DreamGenContext::gettingShot() {
	data.byte(kNewlocation) = 55;
	clearPalette();
	loadIntroRoom();
	fadeScreenUps();
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = (byte)-1;
	runEndSeq();
	clearBeforeLoad();
}

void DreamGenContext::bibleQuote() {
	initGraphics(640, 480, true);

	showPCX("DREAMWEB.I00");
	fadeScreenUps();

	hangOne(80);
	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) = 0;
		return; // "biblequotearly"
	}

	hangOne(560);
	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) = 0;
		return; // "biblequotearly"
	}

	fadeScreenDowns();

	hangOne(200);
	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) = 0;
		return; // "biblequotearly"
	}

	cancelCh0();

	data.byte(kLasthardkey) = 0;
}

void DreamGenContext::hangOne(uint16 delay) {
	do {
		vSync();
		if (data.byte(kLasthardkey) == 1)
			return; // "hangonearly"
	} while	(--delay);
}

void DreamGenContext::hangOne() {
	hangOne(cx);
}

void DreamGenContext::intro() {
	loadTempText("DREAMWEB.T82");
	loadPalFromIFF();
	setMode();
	data.byte(kNewlocation) = 50;
	clearPalette();
	loadIntroRoom();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = (byte)-1;
	data.byte(kVolumeto) = 4;
	playChannel0(12, 255);
	fadeScreenUps();
	runIntroSeq();

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	data.byte(kNewlocation) = 52;
	loadIntroRoom();
	runIntroSeq();

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	data.byte(kNewlocation) = 53;
	loadIntroRoom();
	runIntroSeq();

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "introearly"
	}

	clearBeforeLoad();
	allPalette();
	data.byte(kNewlocation) = 54;
	loadIntroRoom();
	runIntroSeq();

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "introearly"
	}

	getRidOfTempText();
	clearBeforeLoad();

	data.byte(kLasthardkey) =  0;
}

void DreamGenContext::runIntroSeq() {
	data.byte(kGetback) = 0;

	do {
		vSync();

		if (data.byte(kLasthardkey) == 1)
			break;

		spriteUpdate();
		vSync();

		if (data.byte(kLasthardkey) == 1)
			break;

		delEverything();
		printSprites();
		reelsOnScreen();
		afterIntroRoom();
		useTimedText();
		vSync();

		if (data.byte(kLasthardkey) == 1)
			break;

		dumpMap();
		dumpTimedText();
		vSync();

		if (data.byte(kLasthardkey) == 1)
			break;

	} while (data.byte(kGetback) != 1);


	if (data.byte(kLasthardkey) == 1) {
		getRidOfTempText();
		clearBeforeLoad();
	}

	// These were not called in this program arc
	// in the original code.. Bug?
	//getRidOfTempText();
	//clearBeforeLoad();
}

void DreamGenContext::runEndSeq() {
	atmospheres();
	data.byte(kGetback) = 0;

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
	} while (data.byte(kGetback) != 1);
}

void DreamGenContext::loadIntroRoom() {
	data.byte(kIntrocount) = 0;
	data.byte(kLocation) = 255;
	loadRoom();
	data.word(kMapoffsetx) = 72;
	data.word(kMapoffsety) = 16;
	clearSprites();
	data.byte(kThroughdoor) = 0;
	data.byte(kCurrentkey) = '0';
	data.byte(kMainmode) = 0;
	clearWork();
	data.byte(kNewobs) = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreenCPP();
}

void DreamGenContext::set16ColPalette() {
}

void DreamGenContext::realCredits() {
	data.byte(kRoomssample) = 33;
	loadRoomsSample();
	data.byte(kVolume) = 0;

	initGraphics(640, 480, true);
	hangOn(35);

	showPCX("DREAMWEB.I01");
	playChannel0(12, 0);

	hangOne(2);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	showPCX("DREAMWEB.I02");
	playChannel0(12, 0);
	hangOne(2);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	showPCX("DREAMWEB.I03");
	playChannel0(12, 0);
	hangOne(2);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	showPCX("DREAMWEB.I04");
	playChannel0(12, 0);
	hangOne(2);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	showPCX("DREAMWEB.I05");
	playChannel0(12, 0);
	hangOne(2);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	allPalette();
	hangOne(80);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	showPCX("DREAMWEB.I06");
	fadeScreenUps();
	hangOne(60);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	playChannel0(13, 0);
	hangOne(350);

	if (data.byte(kLasthardkey) == 1) {
		data.byte(kLasthardkey) =  0;
		return; // "realcreditsearly"
	}

	fadeScreenDowns();
	hangOne(256);

	data.byte(kLasthardkey) =  0;
}

} // End of namespace DreamGen
