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
#include "common/config-manager.h"

namespace DreamGen {

// Keyboard buffer. data.word(kBufferin) and data.word(kBufferout) are indexes
// into this, making it a ring buffer
uint8 g_keyBuffer[16];

const Room g_roomData[] = {
	{ "DREAMWEB.R00", // Ryan's apartment
	  5,255,33,10,
	  255,255,255,0,
	  1,6,2,255,3,255,255,255,255,255,0 },

	{ "DREAMWEB.R01",
	  1,255,44,10,
	  255,255,255,0,
	  7,2,255,255,255,255,6,255,255,255,1 },

	{ "DREAMWEB.R02",
	  2,255,33,0,
	  255,255,255,0,
	  1,0,255,255,1,255,3,255,255,255,2 },

	{ "DREAMWEB.R03",
	  5,255,33,10,
	  255,255,255,0,
	  2,2,0,2,4,255,0,255,255,255,3 },

	{ "DREAMWEB.R04",
	  23,255,11,30,
	  255,255,255,0,
	  1,4,0,5,255,255,3,255,255,255,4 },

	{ "DREAMWEB.R05",
	  5,255,22,30, // if demo: 22,255,22,30,
	  255,255,255,0,
	  1,2,0,4,255,255,3,255,255,255,5 },

	{ "DREAMWEB.R06",
	  5,255,11,30,
	  255,255,255,0,
	  1,0,0,1,2,255,0,255,255,255,6 },

	{ "DREAMWEB.R07",
	  255,255,0,20,
	  255,255,255,0,
	  2,2,255,255,255,255,0,255,255,255,7 },

	{ "DREAMWEB.R08",
	  8,255,0,10,
	  255,255,255,0,
	  1,2,255,255,255,255,0,11,40,0,8 },

	{ "DREAMWEB.R09",
	  9,255,22,10,
	  255,255,255,0,
	  4,6,255,255,255,255,0,255,255,255,9 },

	{ "DREAMWEB.R10",
	  10,255,33,30,
	  255,255,255,0,
	  2,0,255,255,2,2,4,22,30,255,10 }, // 22,30,0 switches
	                                    // off path 0 in skip
	{ "DREAMWEB.R11",
	  11,255,11,20,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,11 },

	{ "DREAMWEB.R12",
	  12,255,22,20,
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,12 },

	{ "DREAMWEB.R13",
	  12,255,22,20,
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,13 },

	{ "DREAMWEB.R14",
	  14,255,44,20,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,14 },

	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },
	{ "", 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 },

	{ "DREAMWEB.R19",
	  19,255,0,0,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,19 },

	{ "DREAMWEB.R20",
	  22,255,0,20,
	  255,255,255,0,
	  1,4,2,15,255,255,255,255,255,255,20 },

	{ "DREAMWEB.R21",
	  5,255,11,10, // if demo: 22,255,11,10,
	  255,255,255,0,
	  1,4,2,15,1,255,255,255,255,255,21 },

	{ "DREAMWEB.R22",
	  22,255,22,10,
	  255,255,255,0,
	  0,4,255,255,1,255,255,255,255,255,22 },

	{ "DREAMWEB.R23",
	  23,255,22,30,
	  255,255,255,0,
	  1,4,2,15,3,255,255,255,255,255,23 },

	{ "DREAMWEB.R24",
	  5,255,44,0,
	  255,255,255,0,
	  1,6,2,15,255,255,255,255,255,255,24 },

	{ "DREAMWEB.R25",
	  22,255,11,40,
	  255,255,255,0,
	  1,0,255,255,255,255,255,255,255,255,25 },

	{ "DREAMWEB.R26",
	  9,255,22,20,
	  255,255,255,0,
	  4,2,255,255,255,255,255,255,255,255,26 },

	{ "DREAMWEB.R27",
	  22,255,11,20,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,27 },

	{ "DREAMWEB.R28",
	  5,255,11,30,
	  255,255,255,0,
	  0,0,255,255,2,255,255,255,255,255,28 },

	{ "DREAMWEB.R29",
	  22,255,11,10,
	  255,255,255,0,
	  0,2,255,255,255,255,255,255,255,255,29 },



	{ "DREAMWEB.R05", // Duplicate of hotel lobby, but emerging from the lift.
	  5,255,22,10,    // if demo: 22,255,22,10
	  255,255,255,0,
	  1,4,1,15,255,255,255,255,255,255,5 },

	{ "DREAMWEB.R04",  // Duplicate of pool hall lobby,
	  23,255,22,20,    // but emerging from the lift.
	  255,255,255,0,
	  1,4,2,15,255,255,255,255,255,255,4 },

	{ "DREAMWEB.R10",  // entering alley via skip
	  10,255,22,30,
	  255,255,255,0,
	  3,6,255,255,255,255,255,255,255,255,10 },

	{ "DREAMWEB.R12",  // on the beach, getting up.
	  12,255,22,20,
	  255,255,255,0,
	  0,2,255,255,255,255,255,255,255,255,12 },

	{ "DREAMWEB.R03",  // Duplicate of Eden's lobby
	  5,255,44,0,      // but emerging from the lift
	  255,255,255,0,
	  1,6,2,255,4,255,255,255,255,255,3 },

	{ "DREAMWEB.R24",  // Duplicate of Eden's flat
	  5,255,22,0,      // but starting on the bed
	  255,255,255,0,
	  3,6,0,255,255,255,255,33,0,3,24 }, //  33,0,3 turns off path for lift

	{ "DREAMWEB.R22",  // Duplicate
	  22,255,22,20,    // of hotel but in pool room
	  255,255,255,0,
	  1,4,255,255,255,255,255,255,255,255,22 },

	{ "DREAMWEB.R22",  // Duplicate
	  22,255,22,20,    // of hotel but in pool room
	  255,255,255,0,   // coming out of bedroom
	  0,2,255,255,255,255,255,255,255,255,22 },

	{ "DREAMWEB.R11",  // Duplicate
	  11,255,22,30,    // of carpark but getting
	  255,255,255,0,   // up off the floor
	  0,0,255,255,255,255,255,255,255,255,11 },

	{ "DREAMWEB.R28",
	  5,255,11,20,
	  255,255,255,0,
	  0,6,255,255,2,255,255,255,255,255,28 },

	{ "DREAMWEB.R21",
	  5,255,11,10, // if demo: 22,255,11,10
	  255,255,255,0,
	  1,4,2,15,1,255,255,255,255,255,21 },

	{ "DREAMWEB.R26",
	  9,255,0,40,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,26 },

	{ "DREAMWEB.R19",
	  19,255,0,0,
	  255,255,255,0,
	  2,2,255,255,255,255,255,255,255,255,19 },

	{ "DREAMWEB.R08",  // leaving tvstudio into street
	  8,255,11,40,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,8 },

	{ "DREAMWEB.R01",
	  1,255,44,10,
	  255,255,255,0,
	  3,6,255,255,255,255,255,255,255,255,1 },


	{ "DREAMWEB.R45",  // Dream room
	  35,255,22,30,
	  255,255,255,0,
	  0,6,255,255,255,255,255,255,255,255,45 },

	{ "DREAMWEB.R46",  // Dream room
	  35,255,22,40,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,46 },

	{ "DREAMWEB.R47",  // Dream room
	  35,255,0,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,47 },

	{ "DREAMWEB.R45",  // Dream room
	  35,255,22,30,
	  255,255,255,0,
	  4,0,255,255,255,255,255,255,255,255,45 },

	{ "DREAMWEB.R46",  // Dream room
	  35,255,22,50,
	  255,255,255,0,
	  0,4,255,255,255,255,255,255,255,255,46 },



	{ "DREAMWEB.R50",  //  Intro sequence one
	  35,255,22,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,50 },

	{ "DREAMWEB.R51",  //  Intro sequence two
	  35,255,11,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,51 },

	{ "DREAMWEB.R52",  //  Intro sequence three
	  35,255,22,30,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,52 },

	{ "DREAMWEB.R53",  //  Intro sequence four
	  35,255,33,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,53 },

	{ "DREAMWEB.R54",  //  Intro sequence five - wasteland
	  35,255,0,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,54 },

	{ "DREAMWEB.R55",  //  End sequence
	  14,255,44,0,
	  255,255,255,0,
	  0,0,255,255,255,255,255,255,255,255,55 }
};

static const Atmosphere g_atmosphereList[] = {
	// location,map x,y,sound,repeat
	{ 0,33,10,15,255 },
	{ 0,22,10,15,255 },
	{ 0,22,0,15,255 },
	{ 0,11,0,15,255 },
	{ 0,11,10,15,255 },
	{ 0,0,10,15,255 },

	{ 1,44,10,6,255	},
	{ 1,44,0,13,255 },

	{ 2,33,0,6,255 },
	{ 2,22,0,5,255 },
	{ 2,22,10,16,255 },
	{ 2,11,10,16,255 },

	{ 3,44,0,15,255 },
	{ 3,33,10,6,255 },
	{ 3,33,0,5,255 },

	{ 4,11,30,6,255 },
	{ 4,22,30,5,255 },
	{ 4,22,20,13,255 },

	{ 10,33,30,6,255 },
	{ 10,22,30,6,255 },

	{ 9,22,10,6,255 },
	{ 9,22,20,16,255 },
	{ 9,22,30,16,255 },
	{ 9,22,40,16,255 },
	{ 9,22,50,16,255 },

	{ 6,11,30,6,255 },
	{ 6,0,10,15,255 },
	{ 6,0,20,15,255 },
	{ 6,11,20,15,255 },
	{ 6,22,20,15,255 },

	{ 7,11,20,6,255 },
	{ 7,0,20,6,255 },
	{ 7,0,30,6,255 },

	{ 55,44,0,5,255 },
	{ 55,44,10,5,255 },

	{ 5,22,30,6,255 },
	{ 5,22,20,15,255 }, // if demo: { 5,22,20,16,255 },
	{ 5,22,10,15,255 }, // if demo: { 5,22,10,16,255 },

	{ 24,22,0,15,255 },
	{ 24,33,0,15,255 },
	{ 24,44,0,15,255 },
	{ 24,33,10,15,255 },

	{ 8,0,10,6,255 },
	{ 8,11,10,6,255 },
	{ 8,22,10,6,255 },
	{ 8,33,10,6,255 },
	{ 8,33,20,6,255 },
	{ 8,33,30,6,255 },
	{ 8,33,40,6,255 },
	{ 8,22,40,6,255 },
	{ 8,11,40,6,255 },

	{ 11,11,20,12,255 },
	{ 11,11,30,12,255 },
	{ 11,22,20,12,255 },
	{ 11,22,30,12,255 },

	{ 12,22,20,12,255 },
	{ 13,22,20,12,255 },
	{ 13,33,20,12,255 },

	{ 14,44,20,12,255 },
	{ 14,33,0,12,255 },
	{ 14,33,10,12,255 },
	{ 14,33,20,12,255 },
	{ 14,33,30,12,255 },
	{ 14,33,40,12,255 },
	{ 14,22,0,16,255 },

	{ 19,0,0,12,255 },

	{ 20,0,20,16,255 },
	{ 20,0,30,16,255 },
	{ 20,11,30,16,255 },
	{ 20,0,40,16,255 },
	{ 20,11,40,16,255 },

	{ 21,11,10,15,255 }, // if demo: { 21,11,10,16,255 },
	{ 21,11,20,15,255 }, // if demo: { 21,11,20,16,255 },
	{ 21, 0,20,15,255 }, // if demo: { 21,0,20,16,255 },
	{ 21,22,20,15,255 }, // if demo: { 21,22,20,16,255 },
	{ 21,33,20,15,255 }, // if demo: { 21,33,20,16,255 },
	{ 21,44,20,15,255 }, // if demo: { 21,44,20,16,255 },
	{ 21,44,10,15,255 }, // if demo: { 21,44,10,16,255 },

	{ 22,22,10,16,255 },
	{ 22,22,20,16,255 },

	{ 23,22,30,13,255 },
	{ 23,22,40,13,255 },
	{ 23,33,40,13,255 },
	{ 23,11,40,13,255 },
	{ 23,0,40,13,255 },
	{ 23,0,50,13,255 },

	{ 25,11,40,16,255 },
	{ 25,11,50,16,255 },
	{ 25,0,50,16,255 },

	{ 27,11,20,16,255 },
	{ 27,11,30,16,255 },

	{ 29,11,10,16,255 },

	{ 45,22,30,12,255 },
	{ 45,22,40,12,255 },
	{ 45,22,50,12,255 },

	{ 46,22,40,12,255 },
	{ 46,11,50,12,255 },
	{ 46,22,50,12,255 },
	{ 46,33,50,12,255 },

	{ 47,0,0,12,255 },

	{ 26,22,20,16,255 },
	{ 26,33,10,16,255 },
	{ 26,33,20,16,255 },
	{ 26,33,30,16,255 },
	{ 26,44,30,16,255 },
	{ 26,22,30,16,255 },
	{ 26,11,30,16,255 },
	{ 26,11,20,16,255 },
	{ 26,0,20,16,255 },
	{ 26,11,40,16,255 },
	{ 26,0,40,16,255 },
	{ 26,22,40,16,255 },
	{ 26,11,50,16,255 },

	{ 28,0,30,15,255 },
	{ 28,0,20,15,255 },
	{ 28,0,40,15,255 },
	{ 28,11,30,15,255 },
	{ 28,11,20,15,255 },
	{ 28,22,30,15,255 },
	{ 28,22,20,15,255 },

	{ 255,255,255,255,255 }

};

void DreamGenContext::dreamweb() {
	STACK_CHECK;

	switch(engine->getLanguage()) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		// Implicit data.byte(kForeignrelease) = 0
		break;
	default:
		data.byte(kForeignrelease) = 1;
		break;
	}

	seeCommandTail();
	// soundStartup used to be done here...
	// setKeyboardInt used to be done here...
	allocateBuffers();

	// setMouse
	data.word(kOldpointerx) = 0xffff;

	fadeDOS();
	getTime();
	clearBuffers();
	clearPalette();
	set16ColPalette();
	readSetData();
	data.byte(kWongame) = 0;

	engine->loadSounds(0, "DREAMWEB.V99"); // basic sample

	bool firstLoop = true;

	int savegameId = Common::ConfigManager::instance().getInt("save_slot");

	while (true) {

		uint count = scanForNames();

		bool startNewGame = true;

		if (firstLoop && savegameId >= 0) {

			// loading a savegame requested from launcher/command line

			cls();
			setMode();
			loadPalFromIFF();
			clearPalette();

			doLoad(savegameId);
			workToScreenCPP();
			fadeScreenUp();
			startNewGame = false;

		} else if (count == 0 && firstLoop) {

			// no savegames found, and we're not restarting.

			setMode();
			loadPalFromIFF();

		} else {
			// "doDecisions"

			// Savegames found, so ask if we should load one.
			// (If we're restarting after game over, we also always show these
			// options.)

			cls();
			setMode();
			decide();
			if (quitRequested())
				goto done;

			if (data.byte(kGetback) == 4)
				startNewGame = false; // savegame has been loaded

		}

		firstLoop = false;

		if (startNewGame) {
			// "playGame"

			// "titles"
			// TODO: In the demo version, titles() did nothing
			clearPalette();
			bibleQuote();
			if (!quitRequested()) // "titlesearly"
				intro();

			if (quitRequested())
				goto done;

			// "credits"
			clearPalette();
			realCredits();

			if (quitRequested())
				goto done;

			clearChanges();
			setMode();
			loadPalFromIFF();
			data.byte(kLocation) = 255;
			data.byte(kRoomafterdream) = 1;
			data.byte(kNewlocation) = 35;
			data.byte(kVolume) = 7;
			loadRoom();
			clearSprites();
			initMan();
			entryTexts();
			entryAnims();
			data.byte(kDestpos) = 3;
			initialInv();
			data.byte(kLastflag) = 32;
			startup1();
			data.byte(kVolumeto) = 0;
			data.byte(kVolumedirection) = (byte)-1;
			data.byte(kCommandtype) = 255;

		}

		// main loop
		while (true) {

			if (quitRequested())
				goto done;

			screenUpdate();

			if (quitRequested())
				goto done;

			if (data.byte(kWongame) != 0) {
				// "endofgame"
				clearBeforeLoad();
				fadeScreenDowns();
				hangOn(200);
				endGame();
				quickQuit2();
				goto done;
			}

			if (data.byte(kMandead) == 1 || data.byte(kMandead) == 2)
				break;

			if (data.word(kWatchingtime) > 0) {
				if (data.byte(kFinaldest) == data.byte(kManspath))
					data.word(kWatchingtime)--;
			}

			if (data.word(kWatchingtime) == 0) {
				// "notWatching"

				if (data.byte(kMandead) == 4)
					break;

				if (data.byte(kNewlocation) != 255) {
					// "loadNew"
					clearBeforeLoad();
					loadRoom();
					clearSprites();
					initMan();
					entryTexts();
					entryAnims();
					data.byte(kNewlocation) = 255;
					startup();
					data.byte(kCommandtype) = 255;
					workToScreenM();
				}
			}
		}

		// "gameOver"
		clearBeforeLoad();
		showGun();
		fadeScreenDown();
		hangOn(100);

	}
done: // The engine will need some cleaner finalization, let's put it here for now
	// FIXME: This triggers "Deallocating non existent segment" errors when
	// quitting from a menu.
	getRidOfAll();
	engine->freeIcons1();
	engine->freeIcons2();
}

bool DreamBase::quitRequested() {
	return data.byte(kQuitrequested);
}

void DreamGenContext::screenUpdate() {
	newPlace();
	mainScreen();
	if (quitRequested())
		return;
	animPointer();

	showPointer();
	if ((data.word(kWatchingtime) == 0) && (data.byte(kNewlocation) != 0xff))
		return;
	vSync();
	uint16 mouseState = 0;
	mouseState |= readMouseState();
	dumpPointer();

	dumpTextLine();
	delPointer();
	autoLook();
	spriteUpdate();
	watchCount();
	zoom();

	showPointer();
	if (data.byte(kWongame))
		return;
	vSync();
	mouseState |= readMouseState();
	dumpPointer();

	dumpZoom();
	delPointer();
	delEverything();
	printSprites();
	reelsOnScreen();
	afterNewRoom();

	showPointer();
	vSync();
	mouseState |= readMouseState();
	dumpPointer();

	dumpMap();
	dumpTimedText();
	delPointer();

	showPointer();
	vSync();
	data.word(kOldbutton) = data.word(kMousebutton);
	mouseState |= readMouseState();
	data.word(kMousebutton) = mouseState;
	dumpPointer();

	dumpWatch();
	delPointer();
}

void DreamBase::startup() {
	data.byte(kCurrentkey) = 0;
	data.byte(kMainmode) = 0;
	createPanel();
	data.byte(kNewobs) = 1;
	drawFloor();
	showIcon();
	getUnderZoom();
	spriteUpdate();
	printSprites();
	underTextLine();
	reelsOnScreen();
	atmospheres();
}

void DreamGenContext::startup1() {
	clearPalette();
	data.byte(kThroughdoor) = 0;

	startup();

	workToScreenCPP();
	fadeScreenUp();
}

void DreamBase::switchRyanOn() {
	data.byte(kRyanon) = 255;
}

void DreamBase::switchRyanOff() {
	data.byte(kRyanon) = 1;
}

uint16 DreamBase::standardLoad(const char *fileName, uint16 *outSizeInBytes) {
	FileHeader header;

	Common::File file;
	file.open(fileName);
	file.read((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);
	if (outSizeInBytes)
		*outSizeInBytes = sizeInBytes;
	uint16 result = allocateMem((sizeInBytes + 15) / 16);
	file.read(getSegment(result).ptr(0, 0), sizeInBytes);
	return result;
}

void *DreamBase::standardLoadCPP(const char *fileName, uint16 *outSizeInBytes) {
	uint16 sizeInBytes;
	uint16 seg = standardLoad(fileName, &sizeInBytes);
	void *buffer = malloc(sizeInBytes);
	memcpy(buffer, getSegment(seg).ptr(0, 0), sizeInBytes);
	deallocateMem(seg);
	if (outSizeInBytes)
		*outSizeInBytes = sizeInBytes;
	return buffer;
}

void DreamBase::loadIntoTemp(const char *fileName) {
	data.word(kTempgraphics) = standardLoad(fileName);
}

void DreamBase::loadIntoTemp2(const char *fileName) {
	data.word(kTempgraphics2) = standardLoad(fileName);
}

void DreamBase::loadIntoTemp3(const char *fileName) {
	data.word(kTempgraphics3) = standardLoad(fileName);
}

void DreamBase::loadTempCharset(const char *fileName) {
	engine->setTempCharset(standardLoadCPP(fileName));
}

void DreamBase::hangOnCurs(uint16 frameCount) {
	for (uint16 i = 0; i < frameCount; ++i) {
		printCurs();
		vSync();
		delCurs();
	}
}

void DreamGenContext::seeCommandTail() {
	data.byte(kBrightness) = 1;
}

void DreamGenContext::quickQuit() {
	engine->quit();
}

void DreamGenContext::quickQuit2() {
	engine->quit();
}

void DreamBase::readMouse() {
	data.word(kOldbutton) = data.word(kMousebutton);
	uint16 state = readMouseState();
	data.word(kMousebutton) = state;
}

uint16 DreamBase::readMouseState() {
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	return state;
}

void DreamBase::dumpTextLine() {
	if (data.byte(kNewtextline) != 1)
		return;
	data.byte(kNewtextline) = 0;
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multiDump(x, y, 228, 13);
}

void DreamBase::getUnderTimed() {
	if (data.byte(kForeignrelease))
		multiGet(_underTimedText, data.byte(kTimedx), data.byte(kTimedy) - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiGet(_underTimedText, data.byte(kTimedx), data.byte(kTimedy), 240, kUnderTimedTextSizeY);
}

void DreamBase::putUnderTimed() {
	if (data.byte(kForeignrelease))
		multiPut(_underTimedText, data.byte(kTimedx), data.byte(kTimedy) - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiPut(_underTimedText, data.byte(kTimedx), data.byte(kTimedy), 240, kUnderTimedTextSizeY);
}

void DreamGenContext::triggerMessage(uint16 index) {
	multiGet(mapStore(), 174, 153, 200, 63);
	uint16 offset = kTextstart + getSegment(data.word(kPuzzletext)).word(index * 2);
	const uint8 *string = getSegment(data.word(kPuzzletext)).ptr(offset, 0);
	uint16 y = 156;
	printDirect(&string, 174, &y, 141, true);
	hangOn(140);
	workToScreenCPP();
	hangOn(340);
	multiPut(mapStore(), 174, 153, 200, 63);
	workToScreenCPP();
	data.byte(kLasttrigger) = 0;
}

void DreamGenContext::processTrigger() {
	if (data.byte(kLasttrigger) == '1') {
		setLocation(8);
		triggerMessage(45);
	} else if (data.byte(kLasttrigger) == '2') {
		setLocation(9);
		triggerMessage(55);
	} else if (data.byte(kLasttrigger) == '3') {
		setLocation(2);
		triggerMessage(59);
	}
}

void DreamBase::useTimedText() {
	if (data.word(kTimecount) == 0)
		return;
	--data.word(kTimecount);
	if (data.word(kTimecount) == 0) {
		putUnderTimed();
		data.byte(kNeedtodumptimed) = 1;
		return;
	}

	if (data.word(kTimecount) == data.word(kCounttotimed))
		getUnderTimed();
	else if (data.word(kTimecount) > data.word(kCounttotimed))
		return;

	const uint8 *string = getSegment(data.word(kTimedseg)).ptr(data.word(kTimedoffset), 0);
	uint16 y = data.byte(kTimedy);
	printDirect(&string, data.byte(kTimedx), &y, 237, true);
	data.byte(kNeedtodumptimed) = 1;
}

void DreamBase::setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {
#if 1 // if cd
	if (voiceIndex != 0) {
		if (loadSpeech('T', voiceIndex, 'T', textIndex)) {
			playChannel1(50+12);
		}

		// FIXME: This fallthrough does not properly support subtitles+speech
		// mode. The parameters to setuptimedtemp() are sometimes different
		// for speech and for subtitles. See e.g., madmantext()
		if ((data.byte(kSpeechloaded) == 1) && (data.byte(kSubtitles) != 1))
			return;
	}
#endif

	if (data.word(kTimecount) != 0)
		return;
	data.byte(kTimedy) = y;
	data.byte(kTimedx) = x;
	data.word(kCounttotimed) = countToTimed;
	data.word(kTimecount) = timeCount + countToTimed;
	data.word(kTimedseg) = data.word(kTextfile1);
	data.word(kTimedoffset) = kTextstart + getSegment(data.word(kTextfile1)).word(textIndex * 2);
	const uint8 *string = getSegment(data.word(kTextfile1)).ptr(data.word(kTimedoffset), 0);
	debug(1, "setupTimedTemp: (%d, %d) => '%s'", textIndex, voiceIndex, string);
}

void DreamBase::dumpTimedText() {
	if (data.byte(kNeedtodumptimed) != 1)
		return;
	uint8 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;

	multiDump(data.byte(kTimedx), y, 240, kUndertimedysize);
	data.byte(kNeedtodumptimed) = 0;
}

void DreamBase::getTime() {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	data.byte(kSecondcount) = t.tm_sec;
	data.byte(kMinutecount) = t.tm_min;
	data.byte(kHourcount) = t.tm_hour;
}

uint16 DreamBase::allocateMem(uint16 paragraphs) {
	uint size = (paragraphs + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	SegmentRef seg = allocateSegment(size);
	uint16 result = (uint16)seg;
	debug(1, "\tsegment address -> %04x", result);
	return result;
}

void DreamBase::deallocateMem(uint16 segment) {
	debug(1, "deallocating segment %04x", segment);
	deallocateSegment(segment);

	uint tsize = 16 * 32;
	uint16 bseg = data.word(kBuffers);
	if (!bseg)
		return;
	MutableSegmentRef buffers(this);
	buffers = bseg;
	uint8 *ptr = buffers.ptr(kSpritetable, tsize);
	for (uint i = 0; i < tsize; i += 32) {
		uint16 seg = READ_LE_UINT16(ptr + i + 6);
		//debug(1, "sprite segment = %04x", seg);
		if (seg == segment)
			memset(ptr + i, 0xff, 32);
	}
}

void DreamBase::DOSReturn() {
	if (data.byte(kCommandtype) != 250) {
		data.byte(kCommandtype) = 250;
		commandOnly(46);
	}

	if (data.word(kMousebutton) & 1) {
		data.word(kMousebutton) = 0;
		engine->quit();
	}
}

void DreamBase::eraseOldObs() {
	if (data.byte(kNewobs) == 0)
		return;

	// Note: the original didn't delete sprites here, but marked the
	// entries as unused, to be filled again by makeSprite. This can
	// change the order of entries, but since they are drawn based on the
	// priority field, this shouldn't matter.
	Common::List<Sprite>::iterator i;
	for (i = _spriteTable.begin(); i != _spriteTable.end(); ) {
		Sprite &sprite = *i;
		if (sprite._objData != 0xffff)
			i = _spriteTable.erase(i);
		else
			++i;
	}
}

void DreamBase::lockMon() {
	// Pressing space pauses text output in the monitor. We use the "hard"
	// key because calling readkey() drains characters from the input
	// buffer, we we want the user to be able to type ahead while the text
	// is being printed.
	if (data.byte(kLasthardkey) == 57) {
		// Clear the keyboard buffer. Otherwise the space that caused
		// the pause will be read immediately unpause the game.
		do {
			readKey();
		} while (data.byte(kCurrentkey) != 0);

		lockLightOn();
		while (!engine->shouldQuit()) {
			engine->waitForVSync();
			readKey();
			if (data.byte(kCurrentkey) == ' ')
				break;
		}
		// Forget the last "hard" key, otherwise the space that caused
		// the unpausing will immediately re-pause the game.
		data.byte(kLasthardkey) = 0;
		lockLightOff();
	}
}

uint16 DreamBase::allocateAndLoad(unsigned int size) {
	// allocatemem adds 32 bytes, so it doesn't matter that size/16 rounds down
	uint16 result = allocateMem(size / 16);
	engine->readFromFile(getSegment(result).ptr(0, size), size);
	return result;
}

void DreamBase::clearAndLoad(uint8 *buf, uint8 c,
                                   unsigned int size, unsigned int maxSize) {
	assert(size <= maxSize);
	memset(buf, c, maxSize);
	engine->readFromFile(buf, size);
}

void DreamBase::clearAndLoad(uint16 seg, uint8 c,
                                   unsigned int size, unsigned int maxSize) {
	assert(size <= maxSize);
	uint8 *buf = getSegment(seg).ptr(0, maxSize);
	clearAndLoad(buf, c, size, maxSize);
}

void DreamBase::startLoading(const Room &room) {
	data.byte(kCombatcount) = 0;
	data.byte(kRoomssample) = room.roomsSample;
	data.byte(kMapx) = room.mapX;
	data.byte(kMapy) = room.mapY;
	data.byte(kLiftflag) = room.liftFlag;
	data.byte(kManspath) = room.b21;
	data.byte(kDestination) = room.b21;
	data.byte(kFinaldest) = room.b21;
	data.byte(kFacing) = room.facing;
	data.byte(kTurntoface) = room.facing;
	data.byte(kCounttoopen) = room.countToOpen;
	data.byte(kLiftpath) = room.liftPath;
	data.byte(kDoorpath) = room.doorPath;
	data.byte(kLastweapon) = (uint8)-1;
	data.byte(kReallocation) = room.realLocation;

	loadRoomData(room, false);

	findRoomInLoc();
	deleteTaken();
	setAllChanges();
	autoAppear();
//	const Room &newRoom = g_roomData[data.byte(kNewlocation)];
	data.byte(kLastweapon) = (uint8)-1;
	data.byte(kMandead) = 0;
	data.word(kLookcounter) = 160;
	data.byte(kNewlocation) = 255;
	data.byte(kLinepointer) = 254;
	if (room.b27 != 255) {
		data.byte(kManspath) = room.b27;
		autoSetWalk();
	}
	findXYFromPath();
}

void DreamBase::dealWithSpecial(uint8 firstParam, uint8 secondParam) {
	uint8 type = firstParam - 220;
	if (type == 0) {
		placeSetObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 1) {
		removeSetObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 2) {
		placeFreeObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 3) {
		removeFreeObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 4) {
		switchRyanOff();
	} else if (type == 5) {
		data.byte(kTurntoface) = secondParam;
		data.byte(kFacing) = secondParam;
		switchRyanOn();
	} else if (type == 6) {
		data.byte(kNewlocation) = secondParam;
	} else {
		moveMap(secondParam);
	}
}

void DreamBase::plotReel(uint16 &reelPointer) {
	Reel *reel = getReelStart(reelPointer);
	while (reel->x >= 220 && reel->x != 255) {
		dealWithSpecial(reel->x, reel->y);
		++reelPointer;
		reel += 8;
	}

	for (size_t i = 0; i < 8; ++i) {
		if (reel->frame() != 0xffff)
			showReelFrame(reel);
		++reel;
	}
	soundOnReels(reelPointer);
}

void DreamBase::crosshair() {
	uint8 frame;
	if ((data.byte(kCommandtype) != 3) && (data.byte(kCommandtype) < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	showFrame(engine->icons1(), kZoomx + 24, kZoomy + 19, frame, 0);
}

void DreamBase::delTextLine() {
	if (data.byte(kForeignrelease))
		multiPut(_textUnder, data.byte(kTextaddressx), data.word(kTextaddressy) - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiPut(_textUnder, data.byte(kTextaddressx), data.word(kTextaddressy), kUnderTextSizeX, kUnderTextSizeY);
}

void DreamGenContext::commandOnly() {
	commandOnly(al);
}

void DreamBase::commandOnly(uint8 command) {
	delTextLine();
	uint16 index = command * 2;
	uint16 offset = kTextstart + getSegment(data.word(kCommandtext)).word(index);
	uint16 y = data.word(kTextaddressy);
	const uint8 *string = getSegment(data.word(kCommandtext)).ptr(offset, 0);
	printDirect(&string, data.word(kTextaddressx), &y, data.byte(kTextlen), (bool)(data.byte(kTextlen) & 1));
	data.byte(kNewtextline) = 1;
}

void DreamGenContext::checkIfPerson() {
	flags._z = !checkIfPerson(al, ah);
}

bool DreamGenContext::checkIfPerson(uint8 x, uint8 y) {
	Common::List<People>::iterator i;
	for (i = _peopleList.begin(); i != _peopleList.end(); ++i) {
		People &people = *i;
		Reel *reel = getReelStart(people._reelPointer);
		if (reel->frame() == 0xffff)
			++reel;
		const Frame *frame = getReelFrameAX(reel->frame());
		uint8 xmin = reel->x + frame->x;
		uint8 ymin = reel->y + frame->y;
		uint8 xmax = xmin + frame->width;
		uint8 ymax = ymin + frame->height;
		if (x < xmin)
			continue;
		if (y < ymin)
			continue;
		if (x >= xmax)
			continue;
		if (y >= ymax)
			continue;
		_personData = people._routinePointer;
		obName(people.b4, 5);
		return true;
	}
	return false;
}

void DreamGenContext::checkIfFree() {
	flags._z = !checkIfFree(al, ah);
}

bool DreamGenContext::checkIfFree(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _freeList.reverse_begin(); i != _freeList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		obName(pos.index, 2);
		return true;
	}
	return false;
}

void DreamGenContext::checkIfEx() {
	flags._z = !checkIfEx(al, ah);
}

bool DreamGenContext::checkIfEx(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _exList.reverse_begin(); i != _exList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		obName(pos.index, 4);
		return true;
	}
	return false;
}

const uint8 *DreamBase::findObName(uint8 type, uint8 index) {
	if (type == 5) {
		uint16 i = 64 * 2 * (index & 127);
		uint16 offset = getSegment(data.word(kPeople)).word(kPersontxtdat + i) + kPersontext;
		return getSegment(data.word(kPeople)).ptr(offset, 0);
	} else if (type == 4) {
		uint16 offset = getSegment(data.word(kExtras)).word(kExtextdat + index * 2) + kExtext;
		return getSegment(data.word(kExtras)).ptr(offset, 0);
	} else if (type == 2) {
		uint16 offset = getSegment(data.word(kFreedesc)).word(kFreetextdat + index * 2) + kFreetext;
		return getSegment(data.word(kFreedesc)).ptr(offset, 0);
	} else if (type == 1) {
		uint16 offset = getSegment(data.word(kSetdesc)).word(kSettextdat + index * 2) + kSettext;
		return getSegment(data.word(kSetdesc)).ptr(offset, 0);
	} else {
		uint16 offset = getSegment(data.word(kBlockdesc)).word(kBlocktextdat + index * 2) + kBlocktext;
		return getSegment(data.word(kBlockdesc)).ptr(offset, 0);
	}
}

void DreamBase::copyName(uint8 type, uint8 index, uint8 *dst) {
	const uint8 *src = findObName(type, index);
	size_t i;
	for (i = 0; i < 28; ++i) {
		char c = src[i];
		if (c == ':')
			break;
		if (c == 0)
			break;
		dst[i] = c;
	}
	dst[i] = 0;
}

void DreamGenContext::commandWithOb() {
	commandWithOb(al, bh, bl);
}

void DreamBase::commandWithOb(uint8 command, uint8 type, uint8 index) {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";
	delTextLine();
	uint16 commandText = kTextstart + getSegment(data.word(kCommandtext)).word(command * 2);
	uint8 textLen = data.byte(kTextlen);
	{
		const uint8 *string = getSegment(data.word(kCommandtext)).ptr(commandText, 0);
		printDirect(string, data.word(kTextaddressx), data.word(kTextaddressy), textLen, (bool)(textLen & 1));
	}
	copyName(type, index, commandLine);
	uint16 x = data.word(kLastxpos);
	if (command != 0)
		x += 5;
	printDirect(commandLine, x, data.word(kTextaddressy), textLen, (bool)(textLen & 1));
	data.byte(kNewtextline) = 1;
}

void DreamBase::examineObText() {
	commandWithOb(1, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamBase::blockNameText() {
	commandWithOb(0, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamBase::personNameText() {
	commandWithOb(2, data.byte(kCommandtype), data.byte(kCommand) & 127);
}

void DreamBase::walkToText() {
	commandWithOb(3, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamBase::findOrMake(uint8 index, uint8 value, uint8 type) {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change) {
		if (index == change->index && data.byte(kReallocation) == change->location && type == change->type) {
			change->value = value;
			return;
		}
	}

	change->index = index;
	change->location = data.byte(kReallocation);
	change->value = value;
	change->type = type;
}

void DreamBase::setAllChanges() {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change)
		if (change->location == data.byte(kReallocation))
			doChange(change->index, change->value, change->type);
}

DynObject *DreamBase::getFreeAd(uint8 index) {
	return (DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0) + index;
}

DynObject *DreamBase::getExAd(uint8 index) {
	return (DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, 0) + index;
}

DynObject *DreamBase::getEitherAdCPP() {
	if (data.byte(kObjecttype) == kExObjectType)
		return getExAd(data.byte(kItemframe));
	else
		return getFreeAd(data.byte(kItemframe));
}

void *DreamBase::getAnyAd(uint8 *value1, uint8 *value2) {
	if (data.byte(kObjecttype) == kExObjectType) {
		DynObject *exObject = getExAd(data.byte(kCommand));
		*value1 = exObject->slotSize;
		*value2 = exObject->slotCount;
		return exObject;
	} else if (data.byte(kObjecttype) == kFreeObjectType) {
		DynObject *freeObject = getFreeAd(data.byte(kCommand));
		*value1 = freeObject->slotSize;
		*value2 = freeObject->slotCount;
		return freeObject;
	} else {	// 1 or 3. 0 should never happen
		SetObject *setObject = getSetAd(data.byte(kCommand));
		// Note: the original returned slotCount/priority (bytes 4 and 5)
		// instead of slotSize/slotCount (bytes 3 and 4).
		// Changed this for consistency with the Ex/Free cases, and also
		// with getOpenedSize()
		*value1 = setObject->slotSize;
		*value2 = setObject->slotCount;
		return setObject;
	}
}

void *DreamBase::getAnyAdDir(uint8 index, uint8 flag) {
	if (flag == 4)
		return getExAd(index);
	else if (flag == 2)
		return getFreeAd(index);
	else
		return getSetAd(index);
}

SetObject *DreamBase::getSetAd(uint8 index) {
	return (SetObject *)getSegment(data.word(kSetdat)).ptr(0, 0) + index;
}

void DreamBase::doChange(uint8 index, uint8 value, uint8 type) {
	if (type == 0) { //object
		getSetAd(index)->mapad[0] = value;
	} else if (type == 1) { //freeObject
		DynObject *freeObject = getFreeAd(index);
		if (freeObject->mapad[0] == 0xff)
			freeObject->mapad[0] = value;
	} else { //path
//		getSegment(data.word(kReels)).byte(kPathdata + (type - 100) * 144 + index * 8 + 6) = value;
		PathNode *paths = (PathNode *)getSegment(data.word(kReels)).ptr(kPathdata + 144 * (type - 100), 0);
		paths[index].on = value;
	}
}

void DreamBase::deleteTaken() {
	const DynObject *extraObjects = (const DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, 0);
	DynObject *freeObjects = (DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0);
	for (size_t i = 0; i < kNumexobjects; ++i) {
		uint8 location = extraObjects[i].initialLocation;
		if (location == data.byte(kReallocation)) {
			uint8 index = extraObjects[i].index;
			freeObjects[index].mapad[0] = 0xfe;
		}
	}
}

void DreamGenContext::getExPos() {
	es = data.word(kExtras);
	const DynObject *objects = (const DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, sizeof(DynObject));
	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (objects[i].mapad[0] == 0xff) {
			data.byte(kExpos) = i;
			di = kExdata + i * sizeof(DynObject);
			return;
		}
	}
	data.byte(kExpos) = kNumexobjects;
	di = kExdata + kNumexobjects * sizeof(DynObject);
}

void DreamBase::placeSetObject(uint8 index) {
	findOrMake(index, 0, 0);
	getSetAd(index)->mapad[0] = 0;
}

void DreamBase::removeSetObject(uint8 index) {
	findOrMake(index, 0xff, 0);
	getSetAd(index)->mapad[0] = 0xff;
}

void DreamGenContext::finishedWalking() {
	flags._z = finishedWalkingCPP();
}

bool DreamGenContext::finishedWalkingCPP() {
	return (data.byte(kLinepointer) == 254) && (data.byte(kFacing) == data.byte(kTurntoface));
}

void DreamBase::getFlagUnderP(uint8 *flag, uint8 *flagEx) {
	uint8 type, flagX, flagY;
	checkOne(data.word(kMousex) - data.word(kMapadx), data.word(kMousey) - data.word(kMapady), flag, flagEx, &type, &flagX, &flagY);
	data.byte(kLastflag) = *flag;
}

void DreamGenContext::walkAndExamine() {
	if (!finishedWalkingCPP())
		return;
	data.byte(kCommandtype) = data.byte(kWalkexamtype);
	data.byte(kCommand) = data.byte(kWalkexamnum);
	data.byte(kWalkandexam) = 0;
	if (data.byte(kCommandtype) != 5)
		examineOb();
}

void DreamGenContext::obName(uint8 command, uint8 commandType) {
	if (data.byte(kReasseschanges) == 0) {
		if ((commandType == data.byte(kCommandtype)) && (command == data.byte(kCommand))) {
			if (data.byte(kWalkandexam) == 1) {
				walkAndExamine();
				return;
			} else if (data.word(kMousebutton) == 0)
				return;
			else if ((data.byte(kCommandtype) == 3) && (data.byte(kLastflag) < 2))
				return;
			else if ((data.byte(kManspath) != data.byte(kPointerspath)) || (data.byte(kCommandtype) == 3)) {
				setWalk();
				data.byte(kReasseschanges) = 1;
				return;
			} else if (! finishedWalkingCPP())
				return;
			else if (data.byte(kCommandtype) == 5) {
				if (data.word(kWatchingtime) == 0)
					talk();
				return;
			} else {
				if (data.word(kWatchingtime) == 0)
					examineOb();
				return;
			}
		}
	} else
		data.byte(kReasseschanges) = 0;

	data.byte(kCommand) = command;
	data.byte(kCommandtype) = commandType;
	if ((data.byte(kLinepointer) != 254) || (data.word(kWatchingtime) != 0) || (data.byte(kFacing) != data.byte(kTurntoface))) {
		blockNameText();
		return;
	} else if (data.byte(kCommandtype) != 3) {
		if (data.byte(kManspath) != data.byte(kPointerspath)) {
			walkToText();
			return;
		} else if (data.byte(kCommandtype) == 3) {
			blockNameText();
			return;
		} else if (data.byte(kCommandtype) == 5) {
			personNameText();
			return;
		} else {
			examineObText();
			return;
		}
	}
	if (data.byte(kManspath) == data.byte(kPointerspath)) {
		uint8 flag, flagEx, type, flagX, flagY;
		checkOne(data.byte(kRyanx) + 12, data.byte(kRyany) + 12, &flag, &flagEx, &type, &flagX, &flagY);
		if (flag < 2) {
			blockNameText();
			return;
		}
	}

	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (data.byte(kLastflag) < 2) {
		blockNameText();
		return;
	} else if (data.byte(kLastflag) >= 128) {
		blockNameText();
		return;
	} else {
		walkToText();
		return;
	}
}

void DreamBase::delPointer() {
	if (data.word(kOldpointerx) == 0xffff)
		return;
	data.word(kDelherex) = data.word(kOldpointerx);
	data.word(kDelherey) = data.word(kOldpointery);
	data.byte(kDelxs) = data.byte(kPointerxs);
	data.byte(kDelys) = data.byte(kPointerys);
	multiPut(_pointerBack, data.word(kDelherex), data.word(kDelherey), data.byte(kPointerxs), data.byte(kPointerys));
}

void DreamBase::showBlink() {
	if (data.byte(kManisoffscreen) == 1)
		return;
	++data.byte(kBlinkcount);
	if (data.byte(kShadeson) != 0)
		return;
	if (data.byte(kReallocation) >= 50) // eyesshut
		return;
	if (data.byte(kBlinkcount) != 3)
		return;
	data.byte(kBlinkcount) = 0;
	uint8 blinkFrame = data.byte(kBlinkframe);
	++blinkFrame; // Implicit %256
	data.byte(kBlinkframe) = blinkFrame;
	if (blinkFrame > 6)
		blinkFrame = 6;
	static const uint8 blinkTab[] = { 16,18,18,17,16,16,16 };
	uint8 width, height;
	showFrame(engine->icons1(), 44, 32, blinkTab[blinkFrame], 0, &width, &height);
}

void DreamBase::dumpBlink() {
	if (data.byte(kShadeson) != 0)
		return;
	if (data.byte(kBlinkcount) != 0)
		return;
	if (data.byte(kBlinkframe) >= 6)
		return;
	multiDump(44, 32, 16, 12);
}

void DreamBase::dumpPointer() {
	dumpBlink();
	multiDump(data.word(kDelherex), data.word(kDelherey), data.byte(kDelxs), data.byte(kDelys));
	if ((data.word(kOldpointerx) != data.word(kDelherex)) || (data.word(kOldpointery) != data.word(kDelherey)))
		multiDump(data.word(kOldpointerx), data.word(kOldpointery), data.byte(kPointerxs), data.byte(kPointerys));
}

template <class T>
void DreamBase::checkCoords(const RectWithCallback<T> *rectWithCallbacks) {
	if (data.byte(kNewlocation) != 0xff)
		return;

	const RectWithCallback<T> *r;
	for (r = rectWithCallbacks; r->_xMin != 0xffff; ++r) {
		if (r->contains(data.word(kMousex), data.word(kMousey))) {
			(((T *)this)->*(r->_callback))();
			return;
		}
	}
}

void DreamBase::showPointer() {
	showBlink();
	uint16 x = data.word(kMousex);
	data.word(kOldpointerx) = data.word(kMousex);
	uint16 y = data.word(kMousey);
	data.word(kOldpointery) = data.word(kMousey);
	if (data.byte(kPickup) == 1) {
		const Frame *frames;
		if (data.byte(kObjecttype) != kExObjectType)
			frames = (const Frame *)getSegment(data.word(kFreeframes)).ptr(0, 0);
		else
			frames = (const Frame *)getSegment(data.word(kExtras)).ptr(0, 0);
		const Frame *frame = frames + (3 * data.byte(kItemframe) + 1);
		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		data.byte(kPointerxs) = width;
		data.byte(kPointerys) = height;
		uint16 xMin = (x >= width / 2) ? x - width / 2 : 0;
		uint16 yMin = (y >= height / 2) ? y - height / 2 : 0;
		data.word(kOldpointerx) = xMin;
		data.word(kOldpointery) = yMin;
		multiGet(_pointerBack, xMin, yMin, width, height);
		showFrame(frames, x, y, 3 * data.byte(kItemframe) + 1, 128);
		showFrame(engine->icons1(), x, y, 3, 128);
	} else {
		const Frame *frame = engine->icons1() + (data.byte(kPointerframe) + 20);
		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		data.byte(kPointerxs) = width;
		data.byte(kPointerys) = height;
		multiGet(_pointerBack, x, y, width, height);
		showFrame(engine->icons1(), x, y, data.byte(kPointerframe) + 20, 0);
	}
}

void DreamBase::animPointer() {

	if (data.byte(kPointermode) == 2) {
		data.byte(kPointerframe) = 0;
		if ((data.byte(kReallocation) == 14) && (data.byte(kCommandtype) == 211))
			data.byte(kPointerframe) = 5;
		return;
	} else if (data.byte(kPointermode) == 3) {
		if (data.byte(kPointerspeed) != 0) {
			--data.byte(kPointerspeed);
		} else {
			data.byte(kPointerspeed) = 5;
			++data.byte(kPointercount);
			if (data.byte(kPointercount) == 16)
				data.byte(kPointercount) = 0;
		}
		static const uint8 flashMouseTab[] = { 1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2 };
		data.byte(kPointerframe) = flashMouseTab[data.byte(kPointercount)];
		return;
	}
	if (data.word(kWatchingtime) != 0) {
		data.byte(kPointerframe) = 11;
		return;
	}
	data.byte(kPointerframe) = 0;
	if (data.byte(kInmaparea) == 0)
		return;
	if (data.byte(kPointerfirstpath) == 0)
		return;
	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (flag < 2)
		return;
	if (flag >= 128)
		return;
	if (flag & 4) {
		data.byte(kPointerframe) = 3;
		return;
	}
	if (flag & 16) {
		data.byte(kPointerframe) = 4;
		return;
	}
	if (flag & 2) {
		data.byte(kPointerframe) = 5;
		return;
	}
	if (flag & 8) {
		data.byte(kPointerframe) = 6;
		return;
	}
	data.byte(kPointerframe) = 8;
}

void DreamBase::printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered) {
	uint16 offset = kTextstart + getSegment(data.word(kCommandtext)).word(index * 2);
	const uint8 *string = getSegment(data.word(kCommandtext)).ptr(offset, 0);
	printDirect(string, x, y, maxWidth, centered);
}

void DreamBase::printMessage2(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered, uint8 count) {
	uint16 offset = kTextstart + getSegment(data.word(kCommandtext)).word(index * 2);
	const uint8 *string = getSegment(data.word(kCommandtext)).ptr(offset, 0);
	while (count--) {
		findNextColon(&string);
	}
	printDirect(string, x, y, maxWidth, centered);
}

static bool objectMatches(void *object, const char *id) {
	const char *objId = (const char *)(((const uint8 *)object) + 12); // whether it is a DynObject or a SetObject
	for (size_t i = 0; i < 4; ++i) {
		if (id[i] != objId[i] + 'A')
			return false;
	}
	return true;
}

void DreamGenContext::compare() {
	char id[4] = { cl, ch, dl, dh };
	flags._z = compare(al, ah, id);
}

bool DreamBase::compare(uint8 index, uint8 flag, const char id[4]) {
	return objectMatches(getAnyAdDir(index, flag), id);
}

uint16 DreamBase::findSetObject(const char *id) {
	for (uint16 index = 0; index < 128; index++) {
		if (objectMatches(getSetAd(index), id))
			return index;
	}

	return 128;
}

void DreamGenContext::findExObject() {
	char id[5];
	id[0] = al;
	id[1] = ah;
	id[2] = cl;
	id[3] = ch;
	id[4] = '\0';
	al = findExObject(id);
	es = data.word(kExtras);
	bx = kExdata + al * 16;
}

uint16 DreamBase::findExObject(const char *id) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		if (objectMatches(getExAd(index), id))
			return index;
	}

	return kNumexobjects;
}

void DreamGenContext::isRyanHolding() {
	char id[5];
	id[0] = al;
	id[1] = ah;
	id[2] = cl;
	id[3] = ch;
	id[4] = '\0';
	flags._z = !isRyanHolding(id);
}

bool DreamBase::isRyanHolding(const char *id) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		DynObject *object = getExAd(index);
		if (object->mapad[0] == 4 && objectMatches(object, id))
			return true;
	}

	return false;
}

bool DreamBase::isItDescribed(const ObjPos *pos) {
	uint16 offset = getSegment(data.word(kSetdesc)).word(kSettextdat + pos->index * 2);
	uint8 result = getSegment(data.word(kSetdesc)).byte(kSettext + offset);
	return result != 0;
}

bool DreamBase::isCD() {
	// The original sources has two codepaths depending if the game is 'if cd' or not
	// This is a hack to guess which version to use with the assumption that if we have a cd version
	// we managed to load the speech. At least it is isolated in this function and can be changed.
	// Maybe detect the version during game id?
	return (data.byte(kSpeechloaded) == 1);
}

void DreamBase::showIcon() {
	if (data.byte(kReallocation) < 50) {
		showPanel();
		showMan();
		roomName();
		panelIcons1();
		zoomIcon();
	} else {
		Frame *tempSprites = (Frame *)getSegment(data.word(kTempsprites)).ptr(0, 0);
		showFrame(tempSprites, 72, 2, 45, 0);
		showFrame(tempSprites, 72+47, 2, 46, 0);
		showFrame(tempSprites, 69-10, 21, 49, 0);
		showFrame(tempSprites, 160+88, 2, 45, 4 & 0xfe);
		showFrame(tempSprites, 160+43, 2, 46, 4 & 0xfe);
		showFrame(tempSprites, 160+101, 21, 49, 4 & 0xfe);
		middlePanel();
	}
}

bool DreamGenContext::checkIfSet(uint8 x, uint8 y) {
	Common::List<ObjPos>::const_iterator i;
	for (i = _setList.reverse_begin(); i != _setList.end(); --i) {
		const ObjPos &pos = *i;
		assert(pos.index != 0xff);
		if (!pos.contains(x,y))
			continue;
		if (!pixelCheckSet(&pos, x, y))
			continue;
		if (!isItDescribed(&pos))
			continue;
		obName(pos.index, 1);
		return true;
	}
	return false;
}

void DreamGenContext::hangOn() {
	hangOn(cx);
}

void DreamBase::hangOn(uint16 frameCount) {
	while (frameCount) {
		vSync();
		--frameCount;
		if (quitRequested())
			break;
	}
}

void DreamBase::hangOnW(uint16 frameCount) {
	while (frameCount) {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		vSync();
		dumpPointer();
		--frameCount;
		if (quitRequested())
			break;
	}
}

void DreamBase::hangOnP(uint16 count) {
	data.word(kMaintimer) = 0;
	uint8 pointerFrame = data.byte(kPointerframe);
	uint8 pickup = data.byte(kPickup);
	data.byte(kPointermode) = 3;
	data.byte(kPickup) = 0;
	data.byte(kCommandtype) = 255;
	readMouse();
	animPointer();
	showPointer();
	vSync();
	dumpPointer();

	count *= 3;
	for (uint16 i = 0; i < count; ++i) {
		delPointer();
		readMouse();
		animPointer();
		showPointer();
		vSync();
		dumpPointer();
		if (quitRequested())
			break;
		if (data.word(kMousebutton) != 0 && data.word(kMousebutton) != data.word(kOldbutton))
			break;
	}

	delPointer();
	data.byte(kPointerframe) = pointerFrame;
	data.byte(kPickup) = pickup;
	data.byte(kPointermode) = 0;
}

void DreamGenContext::findNextColon() {
	const uint8 *initialString = es.ptr(si, 0);
	const uint8 *string = initialString;
	al = findNextColon(&string);
	si += (string - initialString);
}

uint8 DreamBase::findNextColon(const uint8 **string) {
	uint8 c;
	do {
		c = **string;
		++(*string);
	} while ((c != 0) && (c != ':'));
	return c;
}

const uint8 *DreamGenContext::getObTextStartCPP() {
	push(es);
	push(si);
	getObTextStart();
	const uint8 *result = es.ptr(si, 0);
	si = pop();
	es = pop();
	return result;
}

void DreamGenContext::enterSymbol() {
	data.byte(kManisoffscreen) = 1;
	getRidOfReels();
	loadIntoTemp("DREAMWEB.G12"); // symbol graphics
	data.byte(kSymboltopx) = 24;
	data.byte(kSymboltopdir) = 0;
	data.byte(kSymbolbotx) = 24;
	data.byte(kSymbolbotdir) = 0;
	redrawMainScrn();
	showSymbol();
	underTextLine();
	workToScreenM();
	data.byte(kGetback) = 0;
	do {
		delPointer();
		updateSymbolTop();
		updateSymbolBot();
		showSymbol();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		dumpSymbol();
		RectWithCallback<DreamBase> symbolList[] = {
			{ kSymbolx+40,kSymbolx+64,kSymboly+2,kSymboly+16,&DreamBase::quitSymbol },
			{ kSymbolx,kSymbolx+52,kSymboly+20,kSymboly+50,&DreamBase::setTopLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+20,kSymboly+50,&DreamBase::setTopRight },
			{ kSymbolx,kSymbolx+52,kSymboly+50,kSymboly+80,&DreamBase::setBotLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+50,kSymboly+80,&DreamBase::setBotRight },
			{ 0,320,0,200,&DreamBase::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(symbolList);
	} while ((data.byte(kGetback) == 0) && !quitRequested());
	if ((data.byte(kSymbolbotnum) == 3) && (data.byte(kSymboltopnum) == 5)) {
		removeSetObject(43);
		placeSetObject(46);
		turnAnyPathOn(0, data.byte(kRoomnum) + 12);
		data.byte(kManisoffscreen) = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
		playChannel1(13);
	} else {
		removeSetObject(46);
		placeSetObject(43);
		turnAnyPathOff(0, data.byte(kRoomnum) + 12);
		data.byte(kManisoffscreen) = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
	}
}

void DreamGenContext::zoomOnOff() {
	if (data.word(kWatchingtime) != 0 || data.byte(kPointermode) == 2) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(39);
	}

	if (!(data.word(kMousebutton) & 1) || (data.word(kMousebutton) == data.word(kOldbutton)))
		return;

	data.byte(kZoomon) ^= 1;
	createPanel();
	data.byte(kNewobs) = 0;
	drawFloor();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	commandOnly(39);
	readMouse();
	workToScreenM();
}

void DreamBase::sortOutMap() {
	const uint8 *src = workspace();
	uint8 *dst = (uint8 *)getSegment(data.word(kMapdata)).ptr(0, 0);
	for (uint16 y = 0; y < kMaplength; ++y) {
		memcpy(dst, src, kMapwidth);
		dst += kMapwidth;
		src += 132;
	}
}

void DreamGenContext::mainScreen() {
	data.byte(kInmaparea) = 0;
	if (data.byte(kWatchon) == 1) {
		RectWithCallback<DreamGenContext> mainList[] = {
			{ 44,70,32,46,&DreamGenContext::look },
			{ 0,50,0,180,&DreamGenContext::inventory },
			{ 226,244,10,26,&DreamGenContext::zoomOnOff },
			{ 226,244,26,40,&DreamGenContext::saveLoad },
			{ 240,260,100,124,&DreamGenContext::madmanRun },
			{ 0,320,0,200,&DreamGenContext::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList);
	} else {
		RectWithCallback<DreamGenContext> mainList2[] = {
			{ 44,70,32,46,&DreamGenContext::look },
			{ 0,50,0,180,&DreamGenContext::inventory },
			{ 226+48,244+48,10,26,&DreamGenContext::zoomOnOff },
			{ 226+48,244+48,26,40,&DreamGenContext::saveLoad },
			{ 240,260,100,124,&DreamGenContext::madmanRun },
			{ 0,320,0,200,&DreamGenContext::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList2);
	}

	if (data.byte(kWalkandexam) != 0)
		walkAndExamine();
}

void DreamBase::showWatch() {
	if (data.byte(kWatchon)) {
		showFrame(engine->icons1(), 250, 1, 6, 0);
		showTime();
	}
}

void DreamGenContext::dumpWatch() {
	if (data.byte(kWatchdump) != 1)
		return;
	multiDump(256, 21, 40, 12);
	data.byte(kWatchdump) = 0;
}

void DreamBase::showTime() {
	if (data.byte(kWatchon) == 0)
		return;
	Frame *charset = (Frame *)getSegment(data.word(kCharset1)).ptr(0, 0);

	int seconds = data.byte(kSecondcount);
	int minutes = data.byte(kMinutecount);
	int hours = data.byte(kHourcount);

	showFrame(charset, 282+5, 21, 91*3+10 + seconds / 10, 0);
	showFrame(charset, 282+9, 21, 91*3+10 + seconds % 10, 0);

	showFrame(charset, 270+5, 21, 91*3 + minutes / 10, 0);
	showFrame(charset, 270+11, 21, 91*3 + minutes % 10, 0);

	showFrame(charset, 256+5, 21, 91*3 + hours / 10, 0);
	showFrame(charset, 256+11, 21, 91*3 + hours % 10, 0);

	showFrame(charset, 267+5, 21, 91*3+20, 0);
}

void DreamGenContext::watchCount() {
	if (data.byte(kWatchon) == 0)
		return;
	++data.byte(kTimercount);
	if (data.byte(kTimercount) == 9) {
		showFrame((Frame *)getSegment(data.word(kCharset1)).ptr(0, 0), 268+4, 21, 91*3+21, 0);
		data.byte(kWatchdump) = 1;
	} else if (data.byte(kTimercount) == 18) {
		data.byte(kTimercount) = 0;
		++data.byte(kSecondcount);
		if (data.byte(kSecondcount) == 60) {
			data.byte(kSecondcount) = 0;
			++data.byte(kMinutecount);
			if (data.byte(kMinutecount) == 60) {
				data.byte(kMinutecount) = 0;
				++data.byte(kHourcount);
				if (data.byte(kHourcount) == 24)
					data.byte(kHourcount) = 0;
			}
		}
		showTime();
		data.byte(kWatchdump) = 1;
	}
}

void DreamBase::roomName() {
	printMessage(88, 18, 53, 240, false);
	uint16 textIndex = data.byte(kRoomnum);
	if (textIndex >= 32)
		textIndex -= 32;
	data.word(kLinespacing) = 7;
	uint8 maxWidth = (data.byte(kWatchon) == 1) ? 120 : 160;
	uint16 descOffset = getSegment(data.word(kRoomdesc)).word(kIntextdat + textIndex * 2);
	const uint8 *string = getSegment(data.word(kRoomdesc)).ptr(kIntext + descOffset, 0);
	printDirect(string, 88, 25, maxWidth, false);
	data.word(kLinespacing) = 10;
	useCharset1();
}

void DreamBase::zoomIcon() {
	if (data.byte(kZoomon) == 0)
		return;
	showFrame(engine->icons1(), kZoomx, kZoomy-1, 8, 0);
}

void DreamBase::loadRoom() {
	data.byte(kRoomloaded) = 1;
	data.word(kTimecount) = 0;
	data.word(kMaintimer) = 0;
	data.word(kMapoffsetx) = 104;
	data.word(kMapoffsety) = 38;
	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
	data.byte(kLocation) = data.byte(kNewlocation);
	const Room &room = g_roomData[data.byte(kNewlocation)];
	startLoading(room);
	loadRoomsSample();
	switchRyanOn();
	drawFlags();

	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
}

void DreamGenContext::readSetData() {
	data.word(kCharset1) = standardLoad("DREAMWEB.C00");

	void *icons1Buffer = standardLoadCPP("DREAMWEB.G00");
	engine->setIcons1(icons1Buffer);
	void *icons2Buffer = standardLoadCPP("DREAMWEB.G01");
	engine->setIcons2(icons2Buffer);

	data.word(kMainsprites) = standardLoad("DREAMWEB.S00");
	data.word(kPuzzletext) = standardLoad("DREAMWEB.T80");
	data.word(kCommandtext) = standardLoad("DREAMWEB.T84");
	useCharset1();

	// FIXME: Why is this commented out?
	//engine->openFile("DREAMWEB.VOL");
	//uint8 *volumeTab = getSegment(data.word(kSoundbuffer)).ptr(16384, 0);
	//engine->readFromFile(volumeTab, 2048-256);
	//engine->closeFile();
}

Frame * DreamBase::tempGraphics() {
	return (Frame *)getSegment(data.word(kTempgraphics)).ptr(0, 0);
}

Frame * DreamBase::tempGraphics2() {
	return (Frame *)getSegment(data.word(kTempgraphics2)).ptr(0, 0);
}

Frame * DreamBase::tempGraphics3() {
	return (Frame *)getSegment(data.word(kTempgraphics3)).ptr(0, 0);
}

void DreamBase::findRoomInLoc() {
	uint8 x = data.byte(kMapx) / 11;
	uint8 y = data.byte(kMapy) / 10;
	uint8 roomNum = y * 6 + x;
	data.byte(kRoomnum) = roomNum;
}

void DreamGenContext::autoLook() {
	if ((data.word(kMousex) != data.word(kOldx)) || (data.word(kMousey) != data.word(kOldy))) {
		data.word(kLookcounter) = 1000;
		return;
	}

	--data.word(kLookcounter);
	if (data.word(kLookcounter))
		return;
	if (data.word(kWatchingtime))
		return;
	doLook();
}

void DreamGenContext::look() {
	if (data.word(kWatchingtime) || (data.byte(kPointermode) == 2)) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 241) {
		data.byte(kCommandtype) = 241;
		commandOnly(25);
	}
	if ((data.word(kMousebutton) == 1) && (data.word(kMousebutton) != data.word(kOldbutton)))
		doLook();
}

void DreamGenContext::doLook() {
	createPanel();
	showIcon();
	underTextLine();
	workToScreenM();
	data.byte(kCommandtype) = 255;
	dumpTextLine();
	uint8 index = data.byte(kRoomnum) & 31;
	uint16 offset = getSegment(data.word(kRoomdesc)).word(kIntextdat + index * 2);
	const uint8 *string = getSegment(data.word(kRoomdesc)).ptr(kIntext, 0) + offset;
	findNextColon(&string);
	uint16 x;
	if (data.byte(kReallocation) < 50)
		x = 66;
	else
		x = 40;
	if (printSlow(string, x, 80, 241, true) != 1)
		hangOnP(400);

	data.byte(kPointermode) = 0;
	data.byte(kCommandtype) = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamBase::useCharset1() {
	engine->setCurrentCharset((Frame *)getSegment(data.word(kCharset1)).ptr(0, 0));
}

void DreamBase::useTempCharset() {
	engine->setCurrentCharset(engine->tempCharset());
}

void DreamBase::getRidOfTemp() {
	deallocateMem(data.word(kTempgraphics));
}

void DreamBase::getRidOfTempText() {
	deallocateMem(data.word(kTextfile1));
}

void DreamBase::getRidOfTemp2() {
	deallocateMem(data.word(kTempgraphics2));
}

void DreamBase::getRidOfTemp3() {
	deallocateMem(data.word(kTempgraphics3));
}

void DreamBase::getRidOfTempCharset() {
	engine->freeTempCharset();
}

void DreamBase::getRidOfTempsP() {
	deallocateMem(data.word(kTempsprites));
}

void DreamBase::getRidOfAll() {
	deallocateMem(data.word(kBackdrop));
	deallocateMem(data.word(kSetframes));
	deallocateMem(data.word(kReel1));
	deallocateMem(data.word(kReel2));
	deallocateMem(data.word(kReel3));
	deallocateMem(data.word(kReels));
	deallocateMem(data.word(kPeople));
	deallocateMem(data.word(kSetdesc));
	deallocateMem(data.word(kBlockdesc));
	deallocateMem(data.word(kRoomdesc));
	deallocateMem(data.word(kFreeframes));
	deallocateMem(data.word(kFreedesc));
}

// if skipDat, skip clearing and loading Setdat and Freedat
void DreamBase::loadRoomData(const Room &room, bool skipDat) {
	engine->openFile(room.name);

	FileHeader header;
	engine->readFromFile((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from room file header
	int len[15];
	for (int i = 0; i < 15; ++i)
		len[i] = header.len(i);

	data.word(kBackdrop) = allocateAndLoad(len[0]);
	clearAndLoad(workspace(), 0, len[1], 132*66); // 132*66 = maplen
	sortOutMap();
	data.word(kSetframes) = allocateAndLoad(len[2]);
	if (!skipDat)
		clearAndLoad(data.word(kSetdat), 255, len[3], kSetdatlen);
	else
		engine->skipBytes(len[3]);
	// NB: The skipDat version of this function as called by restoreall
	// had a 'call bloc' instead of 'call loadseg' for reel1,
	// but 'bloc' was not defined.
	data.word(kReel1) = allocateAndLoad(len[4]);
	data.word(kReel2) = allocateAndLoad(len[5]);
	data.word(kReel3) = allocateAndLoad(len[6]);
	data.word(kReels) = allocateAndLoad(len[7]);
	data.word(kPeople) = allocateAndLoad(len[8]);
	data.word(kSetdesc) = allocateAndLoad(len[9]);
	data.word(kBlockdesc) = allocateAndLoad(len[10]);
	data.word(kRoomdesc) = allocateAndLoad(len[11]);
	data.word(kFreeframes) = allocateAndLoad(len[12]);
	if (!skipDat)
		clearAndLoad(data.word(kFreedat), 255, len[13], kFreedatlen);
	else
		engine->skipBytes(len[13]);
	data.word(kFreedesc) = allocateAndLoad(len[14]);

	engine->closeFile();
}

void DreamBase::restoreAll() {
	const Room &room = g_roomData[data.byte(kLocation)];
	loadRoomData(room, true);
	setAllChanges();
}

void DreamBase::restoreReels() {
	if (data.byte(kRoomloaded) == 0)
		return;

	const Room &room = g_roomData[data.byte(kReallocation)];

	engine->openFile(room.name);

	FileHeader header;
	engine->readFromFile((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from room file header
	int len[15];
	for (int i = 0; i < 15; ++i)
		len[i] = header.len(i);

	engine->skipBytes(len[0]);
	engine->skipBytes(len[1]);
	engine->skipBytes(len[2]);
	engine->skipBytes(len[3]);
	data.word(kReel1) = allocateAndLoad(len[4]);
	data.word(kReel2) = allocateAndLoad(len[5]);
	data.word(kReel3) = allocateAndLoad(len[6]);

	engine->closeFile();
}

void DreamBase::loadFolder() {
	loadIntoTemp("DREAMWEB.G09"); // folder graphics 1
	loadIntoTemp2("DREAMWEB.G10"); // folder graphics 2
	loadIntoTemp3("DREAMWEB.G11"); // folder graphics 3
	loadTempCharset("DREAMWEB.C02"); // character set 3
	loadTempText("DREAMWEB.T50"); // folder text
}

void DreamBase::showFolder() {
	data.byte(kCommandtype) = 255;
	if (data.byte(kFolderpage)) {
		useTempCharset();
		createPanel2();
		showFrame(tempGraphics(), 0, 0, 0, 0);
		showFrame(tempGraphics(), 143, 0, 1, 0);
		showFrame(tempGraphics(), 0, 92, 2, 0);
		showFrame(tempGraphics(), 143, 92, 3, 0);
		folderExit();
		if (data.byte(kFolderpage) != 1)
			showLeftPage();
		if (data.byte(kFolderpage) != 12)
			showRightPage();
		useCharset1();
		underTextLine();
	} else {
		createPanel2();
		showFrame(tempGraphics3(), 143-28, 0, 0, 0);
		showFrame(tempGraphics3(), 143-28, 92, 1, 0);
		folderExit();
		underTextLine();
	}
}

void DreamBase::showLeftPage() {
	showFrame(tempGraphics2(), 0, 12, 3, 0);
	uint16 y = 12+5;
	for (size_t i = 0; i < 9; ++i) {
		showFrame(tempGraphics2(), 0, y, 4, 0);
		y += 16;
	}
	showFrame(tempGraphics2(), 0, y, 5, 0);
	data.word(kLinespacing) = 8;
	data.word(kCharshift) = 91;
	data.byte(kKerning) = 1;
	uint8 pageIndex = data.byte(kFolderpage) - 2;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 2, &y, 140, false);
			y += data.word(kLinespacing);
		} while (lastChar != '\0');
	}
	data.byte(kKerning) = 0;
	data.word(kCharshift) = 0;
	data.word(kLinespacing) = 10;
	uint8 *bufferToSwap = workspace() + (48*320)+2;
	for (size_t i = 0; i < 120; ++i) {
		for (size_t j = 0; j < 65; ++j) {
			SWAP(bufferToSwap[j], bufferToSwap[130 - j]);
		}
		bufferToSwap += 320;
	}
}

void DreamBase::showRightPage() {
	showFrame(tempGraphics2(), 143, 12, 0, 0);
	uint16 y = 12+37;
	for (size_t i = 0; i < 7; ++i) {
		showFrame(tempGraphics2(), 143, y, 1, 0);
		y += 16;
	}

	showFrame(tempGraphics2(), 143, y, 2, 0);
	data.word(kLinespacing) = 8;
	data.byte(kKerning) = 1;
	uint8 pageIndex = data.byte(kFolderpage) - 1;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 152, &y, 140, false);
			y += data.word(kLinespacing);
		} while (lastChar != '\0');
	}
	data.byte(kKerning) = 0;
	data.word(kLinespacing) = 10;
}

void DreamBase::showExit() {
	showFrame(engine->icons1(), 274, 154, 11, 0);
}

void DreamBase::showMan() {
	showFrame(engine->icons1(), 0, 0, 0, 0);
	showFrame(engine->icons1(), 0, 114, 1, 0);
	if (data.byte(kShadeson))
		showFrame(engine->icons1(), 28, 25, 2, 0);
}

void DreamBase::panelIcons1() {
	uint16 x;
	if (data.byte(kWatchon) != 1)
		x = 48;
	else
		x = 0;
	showFrame(engine->icons2(), 204 + x, 4, 2, 0);
	if (data.byte(kZoomon) != 1)
		showFrame(engine->icons1(), 228 + x, 8, 5, 0);
	showWatch();
}

void DreamBase::examIcon() {
	showFrame(engine->icons2(), 254, 5, 3, 0);
}

const uint8 *DreamBase::getTextInFile1(uint16 index) {
	SegmentRef text = getSegment(data.word(kTextfile1));
	uint16 offset = text.word(index * 2) + kTextstart;
	const uint8 *string = text.ptr(offset, 0);
	return string;
}

void DreamGenContext::checkFolderCoords() {
	RectWithCallback<DreamGenContext> folderList[] = {
		{ 280,320,160,200, &DreamBase::quitKey },
		{ 143,300,6,194, &DreamGenContext::nextFolder },
		{ 0,143,6,194, &DreamGenContext::lastFolder },
		{ 0,320,0,200, &DreamBase::blank },
		{ 0xFFFF,0,0,0, 0 }
	};
	checkCoords(folderList);
}

void DreamGenContext::nextFolder() {
	if (data.byte(kFolderpage) == 12) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 201) {
		data.byte(kCommandtype) = 201;
		commandOnly(16);
	}
	if ((data.word(kMousebutton) == 1) && (data.word(kMousebutton) != data.word(kOldbutton))) {
		++data.byte(kFolderpage);
		folderHints();
		delPointer();
		showFolder();
		data.word(kMousebutton) = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamGenContext::lastFolder() {
	if (data.byte(kFolderpage) == 0) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 202) {
		data.byte(kCommandtype) = 202;
		commandOnly(17);
	}

	if ((data.word(kMousebutton) == 1) && (data.word(kMousebutton) != data.word(kOldbutton))) {
		--data.byte(kFolderpage);
		delPointer();
		showFolder();
		data.word(kMousebutton) = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamBase::folderHints() {
	if (data.byte(kFolderpage) == 5) {
		if ((data.byte(kAidedead) != 1) && (getLocation(13) != 1)) {
			setLocation(13);
			showFolder();
			const uint8 *string = getTextInFile1(30);
			printDirect(string, 0, 86, 141, true);
			workToScreenM();
			hangOnP(200);
		}
	} else if (data.byte(kFolderpage) == 9) {
		if (getLocation(7) != 1) {
			setLocation(7);
			showFolder();
			const uint8 *string = getTextInFile1(31);
			printDirect(string, 0, 86, 141, true);
			workToScreenM();
			hangOnP(200);
		}
	}
}

void DreamBase::folderExit() {
	showFrame(tempGraphics2(), 296, 178, 6, 0);
}

void DreamBase::loadTravelText() {
	data.word(kTraveltext) = standardLoad("DREAMWEB.T81"); // location descs
}

void DreamBase::loadTempText(const char *fileName) {
	data.word(kTextfile1) = standardLoad(fileName);
}

void DreamBase::drawFloor() {
	eraseOldObs();
	drawFlags();
	calcMapAd();
	doBlocks();
	showAllObs();
	showAllFree();
	showAllEx();
	panelToMap();
	initRain();
	data.byte(kNewobs) = 0;
}

void DreamBase::allocateBuffers() {
	data.word(kExtras) = allocateMem(kLengthofextra/16);
	data.word(kMapdata) = allocateMem(kLengthofmap/16);
	data.word(kBuffers) = allocateMem(kLengthofbuffer/16);
	data.word(kFreedat) = allocateMem(kFreedatlen/16);
	data.word(kSetdat) = allocateMem(kSetdatlen/16);
	data.word(kMapstore) = allocateMem(kLenofmapstore/16);
	data.word(kSounddata) = allocateMem(2048/16);
	data.word(kSounddata2) = allocateMem(2048/16);
}

void DreamBase::workToScreenM() {
	animPointer();
	readMouse();
	showPointer();
	vSync();
	workToScreenCPP();
	delPointer();
}

void DreamBase::loadMenu() {
	loadIntoTemp("DREAMWEB.S02"); // sprite name 3
	loadIntoTemp2("DREAMWEB.G07"); // mon. graphics 2
}

void DreamBase::showMenu() {
	++data.byte(kMenucount);
	if (data.byte(kMenucount) == 37*2)
		data.byte(kMenucount) = 0;
	showFrame(tempGraphics(), kMenux, kMenuy, data.byte(kMenucount) / 2, 0);
}

void DreamBase::dumpMenu() {
	multiDump(kMenux, kMenuy, 48, 48);
}

void DreamGenContext::useMenu() {
	getRidOfReels();
	loadMenu();
	createPanel();
	showPanel();
	showIcon();
	data.byte(kNewobs) = 0;
	drawFloor();
	printSprites();
	showFrame(tempGraphics2(), kMenux-48, kMenuy-4, 4, 0);
	getUnderMenu();
	showFrame(tempGraphics2(), kMenux+54, kMenuy+72, 5, 0);
	workToScreenM();
	data.byte(kGetback) = 0;
	do {
		delPointer();
		putUnderMenu();
		showMenu();
		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpMenu();
		dumpTextLine();
		RectWithCallback<DreamBase> menuList[] = {
			{ kMenux+54,kMenux+68,kMenuy+72,kMenuy+88,&DreamBase::quitKey },
			{ 0,320,0,200,&DreamBase::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(menuList);
	} while ((data.byte(kGetback) != 1) && !quitRequested());
	data.byte(kManisoffscreen) = 0;
	redrawMainScrn();
	getRidOfTemp();
	getRidOfTemp2();
	restoreReels();
	workToScreenM();
}

void DreamBase::atmospheres() {

	const Atmosphere *a = &g_atmosphereList[0];

	for (; a->_location != 255; ++a) {
		if (a->_location != data.byte(kReallocation))
			continue;
		if (a->_mapX != data.byte(kMapx) || a->_mapY != data.byte(kMapy))
			continue;
		if (a->_sound != data.byte(kCh0playing)) {

			if (data.byte(kLocation) == 45 && data.word(kReeltowatch) == 45)
				continue; // "web"

			playChannel0(a->_sound, a->_repeat);

			// NB: The asm here reads
			//	cmp reallocation,2
			//  cmp mapy,0
			//  jz fullvol
			//  jnz notlouisvol
			//  I'm interpreting this as if the cmp reallocation is below the jz

			if (data.byte(kMapy) == 0) {
				data.byte(kVolume) = 0; // "fullvol"
				return;
			}

			if (data.byte(kReallocation) == 2 && data.byte(kMapx) == 22 && data.byte(kMapy) == 10)
				data.byte(kVolume) = 5; // "louisvol"

			if (isCD() && data.byte(kReallocation) == 14) {
				if (data.byte(kMapx) == 33) {
					data.byte(kVolume) = 0; // "ismad2"
					return;
				}

				if (data.byte(kMapx) == 22) {
					data.byte(kVolume) = 5;
					return;
				}

			}
		}

		if (data.byte(kReallocation) == 2) {
			if (data.byte(kMapx) == 22) {
				data.byte(kVolume) = 5; // "louisvol"
				return;
			}

			if (data.byte(kMapx) == 11) {
				data.byte(kVolume) = 0; // "fullvol"
				return;
			}
		}
		return;
	}

	cancelCh0();
}

uint8 DreamGenContext::nextSymbol(uint8 symbol) {
	uint8 result = symbol + 1;
	if (result == 6)
		return 0;
	if (result == 12)
		return 6;
	return result;
}

void DreamGenContext::showSymbol() {
	showFrame(tempGraphics(), kSymbolx, kSymboly, 12, 0);

	showFrame(tempGraphics(), data.byte(kSymboltopx) + kSymbolx-44, kSymboly+20, data.byte(kSymboltopnum), 32);
	uint8 nextTopSymbol = nextSymbol(data.byte(kSymboltopnum));
	showFrame(tempGraphics(), data.byte(kSymboltopx) + kSymbolx+5, kSymboly+20, nextTopSymbol, 32);
	uint8 nextNextTopSymbol = nextSymbol(nextTopSymbol);
	showFrame(tempGraphics(), data.byte(kSymboltopx) + kSymbolx+54, kSymboly+20, nextNextTopSymbol, 32);

	showFrame(tempGraphics(), data.byte(kSymbolbotx) + kSymbolx-44, kSymboly+49, 6 + data.byte(kSymbolbotnum), 32);
	uint8 nextBotSymbol = nextSymbol(data.byte(kSymbolbotnum));
	showFrame(tempGraphics(), data.byte(kSymbolbotx) + kSymbolx+5, kSymboly+49, 6 + nextBotSymbol, 32);
	uint8 nextNextBotSymbol = nextSymbol(nextBotSymbol);
	showFrame(tempGraphics(), data.byte(kSymbolbotx) + kSymbolx+54, kSymboly+49, 6 + nextNextBotSymbol, 32);
}

void DreamBase::readKey() {
	uint16 bufOut = data.word(kBufferout);

	if (bufOut == data.word(kBufferin)) {
		// empty buffer
		data.byte(kCurrentkey) = 0;
		return;
	}

	bufOut = (bufOut + 1) & 15; // The buffer has size 16
	data.byte(kCurrentkey) = g_keyBuffer[bufOut];
	data.word(kBufferout) = bufOut;
}

void DreamBase::setTopLeft() {
	if (data.byte(kSymboltopdir) != 0) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 210) {
		data.byte(kCommandtype) = 210;
		commandOnly(19);
	}

	if (data.word(kMousebutton) != 0)
		data.byte(kSymboltopdir) = 0xFF;
}

void DreamBase::setTopRight() {
	if (data.byte(kSymboltopdir) != 0) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 211) {
		data.byte(kCommandtype) = 211;
		commandOnly(20);
	}

	if (data.word(kMousebutton) != 0)
		data.byte(kSymboltopdir) = 1;
}

void DreamBase::setBotLeft() {
	if (data.byte(kSymbolbotdir) != 0) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 212) {
		data.byte(kCommandtype) = 212;
		commandOnly(21);
	}

	if (data.word(kMousebutton) != 0)
		data.byte(kSymbolbotdir) = 0xFF;
}

void DreamBase::setBotRight() {
	if (data.byte(kSymbolbotdir) != 0) {
		blank();
		return;
	}

	if (data.byte(kCommandtype) != 213) {
		data.byte(kCommandtype) = 213;
		commandOnly(22);
	}

	if (data.word(kMousebutton) != 0)
		data.byte(kSymbolbotdir) = 1;
}

void DreamBase::newGame() {
	if (data.byte(kCommandtype) != 251) {
		data.byte(kCommandtype) = 251;
		commandOnly(47);
	}

	if (data.word(kMousebutton) == 1)
		data.byte(kGetback) = 3;
}

void DreamGenContext::pickupOb(uint8 command, uint8 pos) {
	data.byte(kLastinvpos) = pos;
	data.byte(kObjecttype) = kFreeObjectType;
	data.byte(kItemframe) = command;
	data.byte(kCommand) = command;
	getAnyAd();
	transferToEx();
}

void DreamGenContext::initialInv() {
	if (data.byte(kReallocation) != 24)
		return;

	pickupOb(11, 5);
	pickupOb(12, 6);
	pickupOb(13, 7);
	pickupOb(14, 8);
	pickupOb(18, 0);
	pickupOb(19, 1);
	pickupOb(20, 9);
	pickupOb(16, 2);
	data.byte(kWatchmode) = 1;
	data.word(kReeltohold) = 0;
	data.word(kEndofholdreel) = 6;
	data.byte(kWatchspeed) = 1;
	data.byte(kSpeedcount) = 1;
	switchRyanOff();
}

void DreamGenContext::walkIntoRoom() {
	if (data.byte(kLocation) == 14 && data.byte(kMapx) == 22) {
		data.byte(kDestination) = 1;
		data.byte(kFinaldest) = 1;
		autoSetWalk();
	}
}

void DreamGenContext::afterIntroRoom() {
	if (data.byte(kNowinnewroom) == 0)
		return; // notnewintro

	clearWork();
	findRoomInLoc();
	data.byte(kNewobs) = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreenCPP();
	data.byte(kNowinnewroom) = 0;
}

void DreamBase::redrawMainScrn() {
	data.word(kTimecount) = 0;
	createPanel();
	data.byte(kNewobs) = 0;
	drawFloor();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	readMouse();
	data.byte(kCommandtype) = 255;
}

void DreamBase::blank() {
	if (data.byte(kCommandtype) != 199) {
		data.byte(kCommandtype) = 199;
		commandOnly(0);
	}
}

void DreamGenContext::allPointer() {
	readMouse();
	showPointer();
	dumpPointer();
}

void DreamBase::makeMainScreen() {
	createPanel();
	data.byte(kNewobs) = 1;
	drawFloor();
	spriteUpdate();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	data.byte(kCommandtype) = 255;
	animPointer();
	workToScreenM();
	data.byte(kCommandtype) = 200;
	data.byte(kManisoffscreen) = 0;
}

void DreamBase::openInv() {
	data.byte(kInvopen) = 1;
	printMessage(80, 58 - 10, 61, 240, (240 & 1));
	fillRyan();
	data.byte(kCommandtype) = 255;
}

void DreamGenContext::obsThatDoThings() {
	if (!compare(data.byte(kCommand), data.byte(kObjecttype), "MEMB"))
		return; // notlouiscard

	if (getLocation(4) != 1) {
		setLocation(4);
		lookAtCard();
	}
}

void DreamGenContext::describeOb() {
	const uint8 *obText = getObTextStartCPP();
	uint16 y = 92;
	if (data.byte(kForeignrelease) && data.byte(kObjecttype) == kSetObjectType1)
		y = 82;
	data.word(kCharshift) = 91 + 91;
	printDirect(&obText, 33, &y, 241, 241 & 1);
	data.word(kCharshift) = 0;
	y = 104;
	if (data.byte(kForeignrelease) && data.byte(kObjecttype) == kSetObjectType1)
		y = 94;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	obsThatDoThings();

	// Additional text
	if (compare(data.byte(kCommand), data.byte(kObjecttype), "CUPE")) {
		// Empty cup
		uint16 offset = kTextstart + getSegment(data.word(kPuzzletext)).word(40 * 2);
		const uint8 *string = getSegment(data.word(kPuzzletext)).ptr(offset, 0);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	} else if (compare(data.byte(kCommand), data.byte(kObjecttype), "CUPF")) {
		// Full cup
		uint16 offset = kTextstart + getSegment(data.word(kPuzzletext)).word(39 * 2);
		const uint8 *string = getSegment(data.word(kPuzzletext)).ptr(offset, 0);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	}
}

void DreamGenContext::delEverything() {
	if (data.byte(kMapysize) + data.word(kMapoffsety) < 182) {
		mapToPanel();
	} else {
		// Big room
		data.byte(kMapysize) -= 8;
		mapToPanel();
		data.byte(kMapysize) += 8;
	}
}

void DreamGenContext::errorMessage1() {
	delPointer();
	printMessage(76, 21, 58, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamGenContext::errorMessage2() {
	data.byte(kCommandtype) = 255;
	delPointer();
	printMessage(76, 21, 59, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamGenContext::errorMessage3() {
	delPointer();
	printMessage(76, 21, 60, 240, (240 & 1));
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreenCPP();
	delPointer();
}

void DreamGenContext::reExFromOpen() {

}

void DreamGenContext::putBackObStuff() {
	createPanel();
	showPanel();
	showMan();
	obIcons();
	showExit();
	obPicture();
	describeOb();
	underTextLine();
	data.byte(kCommandtype) = 255;
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();
}

bool DreamBase::isSetObOnMap(uint8 index) {
	return (getSetAd(index)->mapad[0] == 0);
}

void DreamBase::dumpZoom() {
	if (data.byte(kZoomon) == 1)
		multiDump(kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamBase::examineInventory() {
	if (data.byte(kCommandtype) != 249) {
		data.byte(kCommandtype) = 249;
		commandOnly(32);
	}

	if (!(data.word(kMousebutton) & 1))
		return;

	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	data.byte(kPickup) = 0;
	data.byte(kInvopen) = 2;
	openInv();
	workToScreenM();
}

void DreamBase::middlePanel() {
	Frame *tempSprites = (Frame *)getSegment(data.word(kTempsprites)).ptr(0, 0);
	showFrame(tempSprites, 72 + 47 + 20, 0, 48, 0);
	showFrame(tempSprites, 72 + 19, 21, 47, 0);
	showFrame(tempSprites, 160 + 23, 0, 48, 4);
	showFrame(tempSprites, 160 + 71, 21, 47, 4);
}

void DreamBase::showDiary() {
	showFrame(tempGraphics(), kDiaryx, kDiaryy + 37, 1, 0);
	showFrame(tempGraphics(), kDiaryx + 176, kDiaryy + 108, 2, 0);
}

void DreamBase::underTextLine() {
	if (data.byte(kForeignrelease))
		multiGet(_textUnder, data.byte(kTextaddressx), data.word(kTextaddressy) - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiGet(_textUnder, data.byte(kTextaddressx), data.word(kTextaddressy), kUnderTextSizeX, kUnderTextSizeY);
}

void DreamBase::getUnderZoom() {
	multiGet(_zoomSpace, kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamBase::putUnderZoom() {
	multiPut(_zoomSpace, kZoomx + 5, kZoomy + 4, 46, 40);
}

void DreamBase::showWatchReel() {
	uint16 reelPointer = data.word(kReeltowatch);
	plotReel(reelPointer);
	data.word(kReeltowatch) = reelPointer;

	// check for shake
	if (data.byte(kReallocation) == 26 && reelPointer == 104)
		data.byte(kShakecounter) = 0xFF;
}

void DreamBase::watchReel() {
	if (data.word(kReeltowatch) != 0xFFFF) {
		if (data.byte(kManspath) != data.byte(kFinaldest))
			return; // Wait until stopped walking
		if (data.byte(kTurntoface) != data.byte(kFacing))
			return;

		if (--data.byte(kSpeedcount) != 0xFF) {
			showWatchReel();
			return;
		}
		data.byte(kSpeedcount) = data.byte(kWatchspeed);
		if (data.word(kReeltowatch) != data.word(kEndwatchreel)) {
			++data.word(kReeltowatch);
			showWatchReel();
			return;
		}
		if (data.word(kWatchingtime)) {
			showWatchReel();
			return;
		}
		data.word(kReeltowatch) = 0xFFFF;
		data.byte(kWatchmode) = 0xFF;
		if (data.word(kReeltohold) == 0xFFFF)
			return; // No more reel
		data.byte(kWatchmode) = 1;
	} else if (data.byte(kWatchmode) != 1) {
		if (data.byte(kWatchmode) != 2)
			return; // "notreleasehold"
		if (--data.byte(kSpeedcount) == 0xFF) {
			data.byte(kSpeedcount) = data.byte(kWatchspeed);
			++data.word(kReeltohold);
		}
		if (data.word(kReeltohold) == data.word(kEndofholdreel)) {
			data.word(kReeltohold) = 0xFFFF;
			data.byte(kWatchmode) = 0xFF;
			data.byte(kDestination) = data.byte(kDestafterhold);
			data.byte(kFinaldest) = data.byte(kDestafterhold);
			autoSetWalk();
			return;
		}
	}

	uint16 reelPointer = data.word(kReeltohold);
	plotReel(reelPointer);
}

void DreamGenContext::afterNewRoom() {
	if (data.byte(kNowinnewroom) == 0)
		return; // notnew

	data.word(kTimecount) = 0;
	createPanel();
	data.byte(kCommandtype) = 0;
	findRoomInLoc();
	if (data.byte(kRyanon) != 1) {
		al = data.byte(kRyanx) + 12;
		ah = data.byte(kRyany) + 12;
		findPathOfPoint();
		data.byte(kManspath) = dl;
		findXYFromPath();
		data.byte(kResetmanxy) = 1;
	}
	data.byte(kNewobs) = 1;
	drawFloor();
	data.word(kLookcounter) = 160;
	data.byte(kNowinnewroom) = 0;
	showIcon();
	spriteUpdate();
	printSprites();
	underTextLine();
	reelsOnScreen();
	mainScreen();
	getUnderZoom();
	zoom();
	workToScreenM();
	walkIntoRoom();
	edensFlatReminders();
	atmospheres();
}

void DreamGenContext::madmanRun() {
	if (data.byte(kLocation)    != 14 ||
		data.byte(kMapx)        != 22 ||
		data.byte(kPointermode) !=  2 ||
		data.byte(kMadmanflag)  !=  0) {
		identifyOb();
		return;
	}

	if (data.byte(kCommandtype) != 211) {
		data.byte(kCommandtype) = 211;
		commandOnly(52);
	}

	if (data.word(kMousebutton) == 1 &&
		data.word(kMousebutton) != data.word(kOldbutton))
		data.byte(kLastweapon) = 8;
}


void DreamGenContext::decide() {
	setMode();
	loadPalFromIFF();
	clearPalette();
	data.byte(kPointermode) = 0;
	data.word(kWatchingtime) = 0;
	data.byte(kPointerframe) = 0;
	data.word(kTextaddressx) = 70;
	data.word(kTextaddressy) = 182 - 8;
	data.byte(kTextlen) = 181;
	data.byte(kManisoffscreen) = 1;
	loadSaveBox();
	showDecisions();
	workToScreenCPP();
	fadeScreenUp();
	data.byte(kGetback) = 0;

	RectWithCallback<DreamGenContext> decideList[] = {
		{ kOpsx+69,kOpsx+124,kOpsy+30,kOpsy+76,&DreamBase::newGame },
		{ kOpsx+20,kOpsx+87,kOpsy+10,kOpsy+59,&DreamBase::DOSReturn },
		{ kOpsx+123,kOpsx+190,kOpsy+10,kOpsy+59,&DreamGenContext::loadOld },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		if (data.byte(kQuitrequested) != 0)
			return;

		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		delPointer();
		checkCoords(decideList);
	} while (!data.byte(kGetback));

	if (data.byte(kGetback) != 4)
		getRidOfTemp();	// room not loaded

	data.word(kTextaddressx) = 13;
	data.word(kTextaddressy) = 182;
	data.byte(kTextlen) = 240;
}

void DreamGenContext::showGun() {
	data.byte(kAddtored) = 0;
	data.byte(kAddtogreen) = 0;
	data.byte(kAddtoblue) = 0;
	palToStartPal();
	palToEndPal();
	greyscaleSum();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(130);
	endPalToStart();
	clearEndPal();
	data.byte(kFadedirection) = 1;
	data.byte(kFadecount) = 63;
	data.byte(kColourpos) = 0;
	data.byte(kNumtofade) = 128;
	hangOn(200);
	data.byte(kRoomssample) = 34;
	loadRoomsSample();
	data.byte(kVolume) = 0;
	loadIntoTemp("DREAMWEB.G13");
	createPanel2();
	showFrame(tempGraphics(), 100, 4, 0, 0);
	showFrame(tempGraphics(), 158, 106, 1, 0);
	workToScreenCPP();
	getRidOfTemp();
	fadeScreenUp();
	hangOn(160);
	playChannel0(12, 0);
	loadTempText("DREAMWEB.T83");
	rollEndCredits2();
	getRidOfTempText();
}

void DreamBase::diaryKeyP() {
	if (data.byte(kCommandtype) != 214) {
		data.byte(kCommandtype) = 214;
		commandOnly(23);
	}

	if (!data.word(kMousebutton) ||
		data.word(kOldbutton) == data.word(kMousebutton) ||
		data.byte(kPresscount))
		return; // notkeyp

	playChannel1(16);
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'P';
	data.byte(kDiarypage)--;

	if (data.byte(kDiarypage) == 0xFF)
		data.byte(kDiarypage) = 11;
}

void DreamBase::diaryKeyN() {
	if (data.byte(kCommandtype) != 213) {
		data.byte(kCommandtype) = 213;
		commandOnly(23);
	}

	if (!data.word(kMousebutton) ||
		data.word(kOldbutton) == data.word(kMousebutton) ||
		data.byte(kPresscount))
		return; // notkeyn

	playChannel1(16);
	data.byte(kPresscount) = 12;
	data.byte(kPressed) = 'N';
	data.byte(kDiarypage)++;

	if (data.byte(kDiarypage) == 12)
		data.byte(kDiarypage) = 0;
}

void DreamGenContext::dropError() {
	data.byte(kCommandtype) = 255;
	delPointer();
	printMessage(76, 21, 56, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	data.byte(kCommandtype) = 255;
	workToScreenM();
}

void DreamGenContext::cantDrop() {
	data.byte(kCommandtype) = 255;
	delPointer();
	printMessage(76, 21, 24, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	data.byte(kCommandtype) = 255;
	workToScreenM();
}

void DreamBase::getBack1() {
	if (data.byte(kPickup) != 0) {
		blank();
		return;
	}


	if (data.byte(kCommandtype) != 202) {
		data.byte(kCommandtype) = 202;
		commandOnly(26);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;

	if (data.word(kMousebutton) & 1) {
		// Get back
		data.byte(kGetback) = 1;
		data.byte(kPickup) = 0;
	}
}

void DreamGenContext::useButtonA() {
	if (!isSetObOnMap(95)) {
		showFirstUse();
		turnAnyPathOn(0, data.byte(kRoomnum) - 1);
		removeSetObject(9);
		placeSetObject(95);
		data.word(kWatchingtime) = 15 * 2;
		data.word(kReeltowatch) = 71;
		data.word(kEndwatchreel) = 85;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		data.byte(kGetback) = 1;
		data.byte(kProgresspoints)++;
	} else {
		// Done this bit
		showSecondUse();
		putBackObStuff();
	}
}

void DreamBase::autoAppear() {
	if (data.byte(kLocation) == 32) {
		// In alley
		resetLocation(5);
		setLocation(10);
		data.byte(kDestpos) = 10;
		return;
	}

	if (data.byte(kReallocation) == 24) {
		// In Eden's apartment
		if (data.byte(kGeneraldead) == 1) {
			data.byte(kGeneraldead)++;
			placeSetObject(44);
			placeSetObject(18);
			placeSetObject(93);
			removeSetObject(92);
			removeSetObject(55);
			removeSetObject(75);
			removeSetObject(84);
			removeSetObject(85);
		} else if (data.byte(kSartaindead) == 1) {
			// Eden's part 2
			removeSetObject(44);
			removeSetObject(93);
			placeSetObject(55);
			data.byte(kSartaindead)++;
		}
	} else {
		// Not in Eden's
		if (data.byte(kReallocation) == 25) {
			// Sart roof
			data.byte(kNewsitem) = 3;
			resetLocation(6);
			setLocation(11);
			data.byte(kDestpos) = 11;
		} else {
			if (data.byte(kReallocation) == 2 && data.byte(kRockstardead) != 0)
				placeSetObject(23);
		}
	}
}

void DreamBase::quitKey() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(4);
	}

	if (data.word(kMousebutton) != data.word(kOldbutton) && (data.word(kMousebutton) & 1))
		data.byte(kGetback) = 1;
}

void DreamGenContext::setupTimedUse() {
	DreamBase::setupTimedUse(al, cx, dx, bl, bh);
}

void DreamBase::setupTimedUse(uint16 textIndex, uint16 countToTimed, uint16 timeCount, byte x, byte y) {
	if (data.word(kTimecount) != 0)
		return; // can't setup

	data.byte(kTimedy) = y;
	data.byte(kTimedx) = x;
	data.word(kCounttotimed) = countToTimed;
	data.word(kTimecount) = timeCount + countToTimed;
	data.word(kTimedseg) = data.word(kPuzzletext);
	data.word(kTimedoffset) = kTextstart + getSegment(data.word(kPuzzletext)).word(textIndex * 2);
	const uint8 *string = getSegment(data.word(kPuzzletext)).ptr(data.word(kTimedoffset), 0);
	debug(1, "setupTimedUse: %d => '%s'", textIndex, string);
}

void DreamBase::entryTexts() {
	switch (data.byte(kLocation)) {
	case 21:
		setupTimedUse(28, 60, 11, 68, 64);
		break;
	case 30:
		setupTimedUse(27, 60, 11, 68, 64);
		break;
	case 23:
		setupTimedUse(29, 60, 11, 68, 64);
		break;
	case 31:
		setupTimedUse(30, 60, 11, 68, 64);
		break;
	case 20:	// Sarter's 2
		setupTimedUse(31, 60, 11, 68, 64);
		break;
	case 24:	// Eden's lobby
		setupTimedUse(32, 60, 3, 68, 64);
		break;
	case 34:	// Eden 2
		setupTimedUse(33, 60, 3, 68, 64);
		break;
	default:
		break;
	}
}

void DreamGenContext::entryAnims() {
	data.word(kReeltowatch) = 0xFFFF;
	data.byte(kWatchmode) = (byte)-1;

	switch (data.byte(kLocation)) {
	case 33:	// beach
		switchRyanOff();
		data.word(kWatchingtime) = 76 * 2;
		data.word(kReeltowatch) = 0;
		data.word(kEndwatchreel) = 76;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		break;
	case 44:	// Sparky's
		resetLocation(8);
		data.word(kWatchingtime) = 50*2;
		data.word(kReeltowatch) = 247;
		data.word(kEndwatchreel) = 297;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		switchRyanOff();
		break;
	case 22:	// lift
		data.word(kWatchingtime) = 31 * 2;
		data.word(kReeltowatch) = 0;
		data.word(kEndwatchreel) = 30;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		switchRyanOff();
		break;
	case 26:	// under church
		data.byte(kSymboltopnum) = 2;
		data.byte(kSymbolbotnum) = 1;
		break;
	case 45:	// entered Dreamweb
		data.byte(kKeeperflag) = 0;
		data.word(kWatchingtime) = 296;
		data.word(kReeltowatch) = 45;
		data.word(kEndwatchreel) = 198;
		data.byte(kWatchspeed) = 1;
		data.byte(kSpeedcount) = 1;
		switchRyanOff();
		break;
	default:
		if (data.byte(kReallocation) == 46 && data.byte(kSartaindead) == 1) {	// Crystal
			removeFreeObject(0);
		} else if (data.byte(kLocation) == 9 && !checkIfPathIsOn(2) && data.byte(kAidedead) != 0) {
			// Top of church
			if (checkIfPathIsOn(3))
				turnPathOn(2);

			// Make doors open
			removeSetObject(4);
			placeSetObject(5);
		} else if (data.byte(kLocation) == 47) {	// Dream centre
			placeSetObject(4);
			placeSetObject(5);
		} else if (data.byte(kLocation) == 38) {	// Car park
			data.word(kWatchingtime) = 57 * 2;
			data.word(kReeltowatch) = 4;
			data.word(kEndwatchreel) = 57;
			data.byte(kWatchspeed) = 1;
			data.byte(kSpeedcount) = 1;
			switchRyanOff();
		} else if (data.byte(kLocation) == 32) {	// Alley
			data.word(kWatchingtime) = 66 * 2;
			data.word(kReeltowatch) = 0;
			data.word(kEndwatchreel) = 66;
			data.byte(kWatchspeed) = 1;
			data.byte(kSpeedcount) = 1;
			switchRyanOff();
		} else if (data.byte(kLocation) == 24) {	// Eden's again
			turnAnyPathOn(2, data.byte(kRoomnum) - 1);
		}
	}
}

void DreamGenContext::updateSymbolTop() {
	if (!data.byte(kSymboltopdir))
		return; // topfinished

	if (data.byte(kSymboltopdir) == (byte)-1) {
		// Backward
		data.byte(kSymboltopx)--;
		if (data.byte(kSymboltopx) != (byte)-1) {
			// Not wrapping
			if (data.byte(kSymboltopx) != 24)
				return; // topfinished
			data.byte(kSymboltopdir) = 0;
		} else {
			data.byte(kSymboltopx) = 48;
			data.byte(kSymboltopnum)++;
			if (data.byte(kSymboltopnum) != 6)
				return; // topfinished
			data.byte(kSymboltopnum) = 0;
		}
	} else {
		// Forward
		data.byte(kSymboltopx)++;
		if (data.byte(kSymboltopx) != 49) {
			// Not wrapping
			if (data.byte(kSymboltopx) != 24)
				return; // topfinished
			data.byte(kSymboltopdir) = 0;
		} else {
			data.byte(kSymboltopx) = 0;
			data.byte(kSymboltopnum)--;
			if (data.byte(kSymboltopnum) != (byte)-1)
				return; // topfinished
			data.byte(kSymboltopnum) = 5;
		}
	}
}

void DreamGenContext::updateSymbolBot() {
	if (!data.byte(kSymbolbotdir))
		return; // botfinished

	if (data.byte(kSymbolbotdir) == (byte)-1) {
		// Backward
		data.byte(kSymbolbotx)--;
		if (data.byte(kSymbolbotx) != (byte)-1) {
			// Not wrapping
			if (data.byte(kSymbolbotx) != 24)
				return; // botfinished
			data.byte(kSymbolbotdir) = 0;
		} else {
			data.byte(kSymbolbotx) = 48;
			data.byte(kSymbolbotnum)++;
			if (data.byte(kSymbolbotnum) != 6)
				return; // botfinished
			data.byte(kSymbolbotnum) = 0;
		}
	} else {
		// Forward
		data.byte(kSymbolbotx)++;
		if (data.byte(kSymbolbotx) != 49) {
			// Not wrapping
			if (data.byte(kSymbolbotx) != 24)
				return; // botfinished
			data.byte(kSymbolbotdir) = 0;
		} else {
			data.byte(kSymbolbotx) = 0;
			data.byte(kSymbolbotnum)--;
			if (data.byte(kSymbolbotnum) != (byte)-1)
				return; // botfinished
			data.byte(kSymbolbotnum) = 5;
		}
	}
}

void DreamGenContext::showDiaryPage() {
	showFrame(tempGraphics(), kDiaryx, kDiaryy, 0, 0);
	data.byte(kKerning) = 1;
	useTempCharset();
	data.word(kCharshift) = 91+91;
	const uint8 *string = getTextInFile1(data.byte(kDiarypage));
	uint16 y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 129, &y, 240, 240 & 1);
	y = kDiaryy + 23;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	data.byte(kKerning) = 0;
	data.word(kCharshift) = 0;
	useCharset1();
}

void DreamGenContext::dumpDiaryKeys() {
	if (data.byte(kPresscount) == 1) {
		if (data.byte(kSartaindead) != 1 && data.byte(kDiarypage) == 5 && getLocation(6) != 1) {
			// Add Sartain Industries note
			setLocation(6);
			delPointer();
			const uint8 *string = getTextInFile1(12);
			printDirect(string, 70, 106, 241, 241 & 1);
			workToScreenM();
			hangOnP(200);
			createPanel();
			showIcon();
			showDiary();
			showDiaryPage();
			workToScreenM();
			showPointer();
			return;
		} else {
			multiDump(kDiaryx + 48, kDiaryy + 15, 200, 16);
		}
	}

	multiDump(kDiaryx + 94, kDiaryy + 97, 16, 16);
	multiDump(kDiaryx + 151, kDiaryy + 71, 16, 16);
}

void DreamGenContext::lookAtCard() {
	data.byte(kManisoffscreen) = 1;
	getRidOfReels();
	loadKeypad();
	createPanel2();
	showFrame(tempGraphics(), 160, 80, 42, 128);
	const uint8 *obText = getObTextStartCPP();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	uint16 y = 124;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	workToScreenM();
	hangOnW(280);
	createPanel2();
	showFrame(tempGraphics(), 160, 80, 42, 128);
	printDirect(obText, 36, 130, 241, 241 & 1);
	workToScreenM();
	hangOnW(200);
	data.byte(kManisoffscreen) = 0;
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamBase::clearBuffers() {
	memset(getSegment(data.word(kBuffers)).ptr(0, kLengthofbuffer), 0, kLengthofbuffer);

	memset(getSegment(data.word(kExtras)).ptr(0, kLengthofextra), 0xFF, kLengthofextra);

	memcpy(_initialVars, data.ptr(kStartvars, kLengthofvars), kLengthofvars);

	clearChanges();
}

void DreamBase::clearChanges() {
	memset(_listOfChanges, 0xFF, 4*kNumchanges);

	setupInitialReelRoutines();

	memcpy(data.ptr(kStartvars, kLengthofvars), _initialVars, kLengthofvars);

	data.byte(kExpos) = 0;
	data.word(kExframepos) = 0;
	data.word(kExtextpos) = 0;

	memset(getSegment(data.word(kExtras)).ptr(0, kLengthofextra), 0xFF, kLengthofextra);

	const uint8 initialRoomsCanGo[] = { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	memcpy(data.ptr(kRoomscango, 16), initialRoomsCanGo, 16);
}

void DreamGenContext::showDiaryKeys() {
	if (!data.byte(kPresscount))
		return; // nokeyatall

	data.byte(kPresscount)--;

	if (!data.byte(kPresscount))
		return; // nokeyatall

	if (data.byte(kPressed) == 'N') {
		byte frame = (data.byte(kPresscount) == 1) ? 3 : 4;
		showFrame(tempGraphics(), kDiaryx + 94, kDiaryy + 97, frame, 0);
	} else {
		byte frame = (data.byte(kPresscount) == 1) ? 5 : 6;
		showFrame(tempGraphics(), kDiaryx + 151, kDiaryy + 71, frame, 0);
	}

	if (data.byte(kPresscount) == 1)
		showDiaryPage();
}

void DreamGenContext::edensFlatReminders() {
	if (data.byte(kReallocation) != 24 || data.byte(kMapx) != 44)
		return; // not in Eden's lift

	if (data.byte(kProgresspoints))
		return; // not the first time in Eden's apartment

	uint16 exObjextIndex = findExObject("CSHR");
	if (!isRyanHolding("DKEY") || exObjextIndex == kNumexobjects) {
		DreamBase::setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	}

	DynObject *object = getExAd(exObjextIndex);

	if (object->mapad[0] != 4) {
		DreamBase::setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	} else if (object->mapad[1] != 255) {
		if (!compare(object->mapad[1], object->mapad[0], "PURS")) {
			DreamBase::setupTimedUse(50, 48, 8, 54, 70);	// forgot something
			return;
		}
	}

	data.byte(kProgresspoints)++;	// got card
}

void DreamGenContext::incRyanPage() {
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(31);
	}

	if (data.word(kMousebutton) == data.word(kOldbutton) || !(data.word(kMousebutton) & 1))
		return;

	data.byte(kRyanpage) = (data.word(kMousex) - (kInventx + 167)) / 18;

	delPointer();
	fillRyan();
	readMouse();
	showPointer();
	workToScreenCPP();
	delPointer();

}
} // End of namespace DreamGen
