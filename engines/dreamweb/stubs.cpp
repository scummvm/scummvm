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
	soundStartup();
	setKeyboardInt();
	allocateBuffers();
	setMouse();
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

		unsigned int count = scanForNames();

		bool startNewGame = true;

		if (firstLoop && savegameId >= 0) {

			// loading a savegame requested from launcher/command line

			cls();
			setMode();
			loadPalFromIFF();
			clearPalette();

			doLoad(savegameId);
			workToScreen();
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
			data.byte(kVolumedirection) = (uint8)-1;
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
	getRidOfAll();
	engine->freeIcons1();
	engine->freeIcons2();
}

bool DreamGenContext::quitRequested() {
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

void DreamGenContext::startup() {
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

	workToScreen();
	fadeScreenUp();
}

void DreamGenContext::switchRyanOn() {
	data.byte(kRyanon) = 255;
}

void DreamGenContext::switchRyanOff() {
	data.byte(kRyanon) = 1;
}

Common::String DreamGenContext::getFilename(Context &context) {
	const char *name = (const char *)context.cs.ptr(context.dx, 0);
	return Common::String(name);
}

uint8 *DreamGenContext::textUnder() {
	return getSegment(data.word(kBuffers)).ptr(kTextunder, 0);
}

uint16 DreamGenContext::standardLoad(const char *fileName, uint16 *outSizeInBytes) {
	FileHeader header;

	engine->openFile(fileName);
	engine->readFromFile((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);
	if (outSizeInBytes)
		*outSizeInBytes = sizeInBytes;
	uint16 result = allocateMem((sizeInBytes + 15) / 16);
	engine->readFromFile(getSegment(result).ptr(0, 0), sizeInBytes);
	engine->closeFile();
	return result;
}

void *DreamGenContext::standardLoadCPP(const char *fileName, uint16 *outSizeInBytes) {
	uint16 sizeInBytes;
	uint16 seg = standardLoad(fileName, &sizeInBytes);
	void *buffer = malloc(sizeInBytes);
	memcpy(buffer, getSegment(seg).ptr(0, 0), sizeInBytes);
	deallocateMem(seg);
	if (outSizeInBytes)
		*outSizeInBytes = sizeInBytes;
	return buffer;
}

void DreamGenContext::standardLoad() {
	ax = standardLoad((const char *)cs.ptr(dx, 0), NULL);
}

void DreamGenContext::loadIntoTemp() {
	loadIntoTemp((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadIntoTemp(const char *fileName) {
	data.word(kTempgraphics) = standardLoad(fileName);
}

void DreamGenContext::loadIntoTemp2(const char *fileName) {
	data.word(kTempgraphics2) = standardLoad(fileName);
}

void DreamGenContext::loadIntoTemp3(const char *fileName) {
	data.word(kTempgraphics3) = standardLoad(fileName);
}

void DreamGenContext::loadTempCharset() {
	loadTempCharset((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadTempCharset(const char *fileName) {
	engine->setTempCharset(standardLoadCPP(fileName));
}

void DreamGenContext::hangOnCurs(uint16 frameCount) {
	for (uint16 i = 0; i < frameCount; ++i) {
		printCurs();
		vSync();
		delCurs();
	}
}

void DreamGenContext::seeCommandTail() {
	data.word(kSoundbaseadd) = 0x220;
	data.byte(kSoundint) = 5;
	data.byte(kSounddmachannel) = 1;
	data.byte(kBrightness) = 1;
	data.word(kHowmuchalloc) = 0x9360;
}

void DreamGenContext::randomNumber() {
	al = engine->randomNumber();
}

void DreamGenContext::quickQuit() {
	engine->quit();
}

void DreamGenContext::quickQuit2() {
	engine->quit();
}

void DreamGenContext::keyboardRead() {
	::error("keyboardread"); //this keyboard int handler, must never be called
}

void DreamGenContext::resetKeyboard() {
}

void DreamGenContext::setKeyboardInt() {
}

void DreamGenContext::closeFile() {
	engine->closeFile();
	data.byte(kHandle) = 0;
}

void DreamGenContext::openForSave(unsigned int slot) {
	//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	debug(1, "openForSave(%s)", filename.c_str());
	engine->openSaveFileForWriting(filename);
}

bool DreamGenContext::openForLoad(unsigned int slot) {
	//Common::String filename = ConfMan.getActiveDomainName() + Common::String::format(".d%02d", savegameId);
	Common::String filename = Common::String::format("DREAMWEB.D%02d", slot);
	debug(1, "openForLoad(%s)", filename.c_str());
	return engine->openSaveFileForReading(filename);
}

void DreamGenContext::openFileNoCheck() {
	const char *name = (const char *)ds.ptr(dx, 13);
	debug(1, "checkSaveFile(%s)", name);
	bool ok = engine->openSaveFileForReading(name);
	flags._c = !ok;
}

void DreamGenContext::openFileFromC() {
	openFileNoCheck();
}

void DreamGenContext::createFile() {
	::error("createfile");
}

void DreamGenContext::dontLoadSeg() {
	ax = es.word(di);
	_add(di, 2);
	dx = ax;
	cx = 0;
	unsigned pos = engine->skipBytes(dx);
	dx = pos >> 16;
	ax = pos & 0xffff;
	flags._c = false;
}

void DreamGenContext::mouseCall() {
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	cx = x;
	dx = y;
	bx = state;
}

uint8 *DreamGenContext::mapStore() {
	return getSegment(data.word(kMapstore)).ptr(0, 0);
}

void DreamGenContext::readMouse() {
	data.word(kOldbutton) = data.word(kMousebutton);
	uint16 state = readMouseState();
	data.word(kMousebutton) = state;
}

uint16 DreamGenContext::readMouseState() {
	data.word(kOldx) = data.word(kMousex);
	data.word(kOldy) = data.word(kMousey);
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	data.word(kMousex) = x;
	data.word(kMousey) = y;
	return state;
}

void DreamGenContext::setMouse() {
	data.word(kOldpointerx) = 0xffff;
}

void DreamGenContext::dumpTextLine() {
	if (data.byte(kNewtextline) != 1)
		return;
	data.byte(kNewtextline) = 0;
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multiDump(x, y, 228, 13);
}

void DreamGenContext::getUnderTimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiGet(ds.ptr(si, 0), data.byte(kTimedx), y, 240, kUndertimedysize);
}

void DreamGenContext::putUnderTimed() {
	uint16 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease))
		y -= 3;
	ds = data.word(kBuffers);
	si = kUndertimedtext;
	multiPut(ds.ptr(si, 0), data.byte(kTimedx), y, 240, kUndertimedysize);
}

void DreamGenContext::useTimedText() {
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

	es = data.word(kTimedseg);
	si = data.word(kTimedoffset);
	const uint8 *string = es.ptr(si, 0);
	uint16 y = data.byte(kTimedy);
	printDirect(&string, data.byte(kTimedx), &y, 237, true);
	data.byte(kNeedtodumptimed) = 1;
}

void DreamGenContext::setupTimedTemp(uint8 textIndex, uint8 voiceIndex, uint8 x, uint8 y, uint16 countToTimed, uint16 timeCount) {
#if 1 // if cd
	if (voiceIndex != 0) {
		push(ax);
		push(bx);
		push(cx);
		push(dx);
		dl = 'T';
		dh = voiceIndex;
		cl = 'T';
		ah = 0;
		al = textIndex;
		loadSpeech();
		if (data.byte(kSpeechloaded) == 1) {
			playChannel1(50+12);
		}
		dx = pop();
		cx = pop();
		bx = pop();
		ax = pop();

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

void DreamGenContext::dumpTimedText() {
	if (data.byte(kNeedtodumptimed) != 1)
		return;
	uint8 y = data.byte(kTimedy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;

	multiDump(data.byte(kTimedx), y, 240, kUndertimedysize);
	data.byte(kNeedtodumptimed) = 0;
}

void DreamGenContext::getTime() {
	TimeDate t;
	g_system->getTimeAndDate(t);
	debug(1, "\tgettime: %02d:%02d:%02d", t.tm_hour, t.tm_min, t.tm_sec);
	ch = t.tm_hour;
	cl = t.tm_min;
	dh = t.tm_sec;
	data.byte(kSecondcount) = dh;
	data.byte(kMinutecount) = cl;
	data.byte(kHourcount) = ch;
}

void DreamGenContext::allocateMem() {
	ax = allocateMem(bx);
}

uint16 DreamGenContext::allocateMem(uint16 paragraphs) {
	uint size = (paragraphs + 2) * 16;
	debug(1, "allocate mem, %u bytes", size);
	flags._c = false;
	SegmentRef seg = allocateSegment(size);
	uint16 result = (uint16)seg;
	debug(1, "\tsegment address -> %04x", result);
	return result;
}

void DreamGenContext::deallocateMem() {
	deallocateMem((uint16)es);
}

void DreamGenContext::deallocateMem(uint16 segment) {
	debug(1, "deallocating segment %04x", segment);
	deallocateSegment(segment);

	//fixing invalid entries in the sprite table
	es = data;
	uint tsize = 16 * 32;
	uint16 bseg = data.word(kBuffers);
	if (!bseg)
		return;
	MutableSegmentRef buffers(this);
	buffers = bseg;
	uint8 *ptr = buffers.ptr(kSpritetable, tsize);
	for(uint i = 0; i < tsize; i += 32) {
		uint16 seg = READ_LE_UINT16(ptr + i + 6);
		//debug(1, "sprite segment = %04x", seg);
		if (seg == segment)
			memset(ptr + i, 0xff, 32);
	}
}

void DreamGenContext::soundStartup() {}
void DreamGenContext::soundEnd() {}
void DreamGenContext::interruptTest() {}
void DreamGenContext::disableSoundInt() {}
void DreamGenContext::enableSoundInt() {}
void DreamGenContext::checkSoundInt() {
	data.byte(kTestresult) = 1;
}

void DreamGenContext::loadSpeech() {
	cancelCh1();
	data.byte(kSpeechloaded) = 0;
	createName();
	const char *name = (const char *)data.ptr(di, 13);
	//warning("name = %s", name);
	if (engine->loadSpeech(name))
		data.byte(kSpeechloaded) = 1;
}

void DreamGenContext::saveFileRead() {
	ax = engine->readFromSaveFile(ds.ptr(dx, cx), cx);
}

void DreamGenContext::loadSeg() {
	ax = es.word(di);
	di += 2;

	uint16 dst_offset = dx;
	uint16 size = ax;

	debug(1, "loadseg(%04x:%u, %u)", (uint16)ds, dst_offset, size);
	ax = engine->readFromFile(ds.ptr(dst_offset, size), size);
	flags._c = false;
}

void DreamGenContext::DOSReturn() {
	if (data.byte(kCommandtype) != 250) {
		data.byte(kCommandtype) = 250;
		commandOnly(46);
	}

	if (data.word(kMousebutton) & 1) {
		data.word(kMousebutton) = 0;
		engine->quit();
	}
}

void DreamGenContext::set16ColPalette() {
}

void DreamGenContext::showGroup() {
	engine->setPalette();
}

void DreamGenContext::fadeDOS() {
	engine->fadeDos();
}

void DreamGenContext::eraseOldObs() {
	if (data.byte(kNewobs) == 0)
		return;

	Sprite *sprites = spriteTable();
	for (size_t i = 0; i < 16; ++i) {
		Sprite &sprite = sprites[i];
		if (sprite.objData() != 0xffff) {
			memset(&sprite, 0xff, sizeof(Sprite));
		}
	}
}

void DreamGenContext::modifyChar() {
	al = engine->modifyChar(al);
}

void DreamGenContext::lockMon() {
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

void DreamGenContext::cancelCh0() {
	data.byte(kCh0repeat) = 0;
	data.word(kCh0blockstocopy) = 0;
	data.byte(kCh0playing) = 255;
	engine->stopSound(0);
}

void DreamGenContext::cancelCh1() {
	data.word(kCh1blockstocopy) = 0;
	data.byte(kCh1playing) = 255;
	engine->stopSound(1);
}

void DreamGenContext::makeBackOb(SetObject *objData) {
	if (data.byte(kNewobs) == 0)
		return;
	uint8 priority = objData->priority;
	uint8 type = objData->type;
	Sprite *sprite = makeSprite(data.word(kObjectx), data.word(kObjecty), addr_backobject, data.word(kSetframes), 0);

	uint16 objDataOffset = (uint8 *)objData - getSegment(data.word(kSetdat)).ptr(0, 0);
	assert(objDataOffset % sizeof(SetObject) == 0);
	assert(objDataOffset < 128 * sizeof(SetObject));
	sprite->setObjData(objDataOffset);
	if (priority == 255)
		priority = 0;
	sprite->priority = priority;
	sprite->type = type;
	sprite->b16 = 0;
	sprite->delay = 0;
	sprite->animFrame = 0;
}

uint16 DreamGenContext::allocateAndLoad(unsigned int size) {
	// allocatemem adds 32 bytes, so it doesn't matter that size/16 rounds down
	uint16 result = allocateMem(size / 16);
	engine->readFromFile(getSegment(result).ptr(0, size), size);
	return result;
}

void DreamGenContext::clearAndLoad(uint16 seg, uint8 c,
                                   unsigned int size, unsigned int maxSize) {
	assert(size <= maxSize);
	uint8 *buf = getSegment(seg).ptr(0, maxSize);
	memset(buf, c, maxSize);
	engine->readFromFile(buf, size);
}

void DreamGenContext::startLoading(const Room &room) {
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
	ah = data.byte(kReallocation);
	data.byte(kReallocation) = room.realLocation;

	loadRoomData(room, false);

	findRoomInLoc();
	deleteTaken();
	setAllChanges();
	autoAppear();
//	const Room &newRoom = g_roomData[data.byte(kNewlocation)];
	bx = 0x7fff; // TODO: bx used to be set to the offset of newRoom
	             // It seems to be unused (like newRoom itself), but set it
	             // to an invalid value to catch any missed use of it.
	             // (The push/pop of bx below is likely also unnecessary)
	data.byte(kLastweapon) = (uint8)-1;
	data.byte(kMandead) = 0;
	data.word(kLookcounter) = 160;
	data.byte(kNewlocation) = 255;
	data.byte(kLinepointer) = 254;
	if (room.b27 != 255) {
		data.byte(kManspath) = room.b27;
		push(bx);
		autoSetWalk();
		bx = pop();
	}
	findXYFromPath();
}

void DreamGenContext::fillSpace() {
	memset(ds.ptr(dx, cx), al, cx);
}

void DreamGenContext::dealWithSpecial(uint8 firstParam, uint8 secondParam) {
	uint8 type = firstParam - 220;
	if (type == 0) {
		placeSetObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 1) {
		removeSetObject(secondParam);
		data.byte(kHavedoneobs) = 1;
	} else if (type == 2) {
		al = secondParam;
		placeFreeObject();
		data.byte(kHavedoneobs) = 1;
	} else if (type == 3) {
		al = secondParam;
		removeFreeObject();
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

void DreamGenContext::plotReel() {
	Reel *reel = getReelStart();
	while (reel->x >= 220 && reel->x != 255) {
		dealWithSpecial(reel->x, reel->y);
		++data.word(kReelpointer);
		reel += 8;
	}

	for (size_t i = 0; i < 8; ++i) {
		if (reel->frame() != 0xffff)
			showReelFrame(reel);
		++reel;
	}
	push(es);
	push(bx);
	soundOnReels();
	bx = pop();
	es = pop();
}

void DreamGenContext::crosshair() {
	uint8 frame;
	if ((data.byte(kCommandtype) != 3) && (data.byte(kCommandtype) < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	showFrame(engine->icons1(), kZoomx + 24, kZoomy + 19, frame, 0);
}

void DreamGenContext::delTextLine() {
	uint16 x = data.word(kTextaddressx);
	uint16 y = data.word(kTextaddressy);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	multiPut(textUnder(), x, y, kUndertextsizex, kUndertextsizey);
}

void DreamGenContext::commandOnly() {
	commandOnly(al);	
}

void DreamGenContext::commandOnly(uint8 command) {
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
	People *people = (People *)getSegment(data.word(kBuffers)).ptr(kPeoplelist, 0);

	for (size_t i = 0; i < 12; ++i, ++people) {
		if (people->b4 == 255)
			continue;
		data.word(kReelpointer) = people->reelPointer();
		Reel *reel = getReelStart();
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
		data.word(kPersondata) = people->routinePointer();
		obName(people->b4, 5);
		return true;
	}
	return false;
}

void DreamGenContext::checkIfFree() {
	flags._z = !checkIfFree(al, ah);
}

bool DreamGenContext::checkIfFree(uint8 x, uint8 y) {
	const ObjPos *freeList = (const ObjPos *)getSegment(data.word(kBuffers)).ptr(kFreelist, 80 * sizeof(ObjPos));
	for (size_t i = 0; i < 80; ++i) {
		const ObjPos *objPos = freeList + 79 - i;
		if (objPos->index == 0xff || !objPos->contains(x,y))
			continue;
		obName(objPos->index, 2);
		return true;
	}
	return false;
}

void DreamGenContext::checkIfEx() {
	flags._z = !checkIfEx(al, ah);
}

bool DreamGenContext::checkIfEx(uint8 x, uint8 y) {
	const ObjPos *exList = (const ObjPos *)getSegment(data.word(kBuffers)).ptr(kExlist, 100 * sizeof(ObjPos));
	for (size_t i = 0; i < 100; ++i) {
		const ObjPos *objPos = exList + 99 - i;
		if (objPos->index == 0xff || !objPos->contains(x,y))
			continue;
		obName(objPos->index, 4);
		return true;
	}
	return false;
}

const uint8 *DreamGenContext::findObName(uint8 type, uint8 index) {
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

void DreamGenContext::copyName() {
	copyName(ah, al, cs.ptr(di, 0));
}

void DreamGenContext::copyName(uint8 type, uint8 index, uint8 *dst) {
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

void DreamGenContext::commandWithOb(uint8 command, uint8 type, uint8 index) {
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

void DreamGenContext::examineObText() {
	commandWithOb(1, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::showPanel() {
	showFrame(engine->icons1(), 72, 0, 19, 0);
	showFrame(engine->icons1(), 192, 0, 19, 0);
}

void DreamGenContext::blockNameText() {
	commandWithOb(0, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::personNameText() {
	commandWithOb(2, data.byte(kCommandtype), data.byte(kCommand) & 127);
}

void DreamGenContext::walkToText() {
	commandWithOb(3, data.byte(kCommandtype), data.byte(kCommand));
}

void DreamGenContext::findOrMake() {
	uint8 b0 = al;
	uint8 b2 = cl;
	uint8 b3 = ch;
	findOrMake(b0, b2, b3);
}

void DreamGenContext::findOrMake(uint8 index, uint8 value, uint8 type) {
	Change *change = (Change *)getSegment(data.word(kBuffers)).ptr(kListofchanges, sizeof(Change));
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

void DreamGenContext::setAllChanges() {
	Change *change = (Change *)getSegment(data.word(kBuffers)).ptr(kListofchanges, sizeof(Change));
	for (; change->index != 0xff; ++change)
		if (change->location == data.byte(kReallocation))
			doChange(change->index, change->value, change->type);
}

DynObject *DreamGenContext::getFreeAd(uint8 index) {
	return (DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0) + index;
}

DynObject *DreamGenContext::getExAd(uint8 index) {
	return (DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, 0) + index;
}

DynObject *DreamGenContext::getEitherAdCPP() {
	if (data.byte(kObjecttype) == 4)
		return getExAd(data.byte(kItemframe));
	else
		return getFreeAd(data.byte(kItemframe));
}

void *DreamGenContext::getAnyAd(uint8 *value1, uint8 *value2) {
	if (data.byte(kObjecttype) == 4) {
		DynObject *exObject = getExAd(data.byte(kCommand));
		*value1 = exObject->b7;
		*value2 = exObject->b8;
		return exObject;
	} else if (data.byte(kObjecttype) == 2) {
		DynObject *freeObject = getFreeAd(data.byte(kCommand));
		*value1 = freeObject->b7;
		*value2 = freeObject->b8;
		return freeObject;
	} else {
		SetObject *setObject = getSetAd(data.byte(kCommand));
		*value1 = setObject->b4;
		*value2 = setObject->priority;
		return setObject;
	}
}

void *DreamGenContext::getAnyAdDir(uint8 index, uint8 flag) {
	if (flag == 4)
		return getExAd(index);
	else if (flag == 2)
		return getFreeAd(index);
	else
		return getSetAd(index);
}

SetObject *DreamGenContext::getSetAd(uint8 index) {
	return (SetObject *)getSegment(data.word(kSetdat)).ptr(0, 0) + index;
}

void DreamGenContext::doChange(uint8 index, uint8 value, uint8 type) {
	if (type == 0) { //object
		getSetAd(index)->mapad[0] = value;
	} else if (type == 1) { //freeObject
		DynObject *freeObject = getFreeAd(index);
		if (freeObject->mapad[0] == 0xff)
			freeObject->mapad[0] = value;
	} else { //path
		bx = kPathdata + (type - 100) * 144 + index * 8;
		es = data.word(kReels);
		es.byte(bx+6) = value;
	}
}

void DreamGenContext::deleteTaken() {
	const DynObject *extraObjects = (const DynObject *)getSegment(data.word(kExtras)).ptr(kExdata, 0);
	DynObject *freeObjects = (DynObject *)getSegment(data.word(kFreedat)).ptr(0, 0);
	for(size_t i = 0; i < kNumexobjects; ++i) {
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

void DreamGenContext::placeSetObject() {
	placeSetObject(al);
}

void DreamGenContext::placeSetObject(uint8 index) {
	findOrMake(index, 0, 0);
	getSetAd(index)->mapad[0] = 0;
}

void DreamGenContext::removeSetObject() {
	removeSetObject(al);
}

void DreamGenContext::removeSetObject(uint8 index) {
	findOrMake(index, 0xff, 0);
	getSetAd(index)->mapad[0] = 0xff;
}

void DreamGenContext::finishedWalking() {
	flags._z = finishedWalkingCPP();
}

bool DreamGenContext::finishedWalkingCPP() {
	return (data.byte(kLinepointer) == 254) && (data.byte(kFacing) == data.byte(kTurntoface));
}

void DreamGenContext::getFlagUnderP() {
	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	cl = flag;
	ch = flagEx;
}

void DreamGenContext::getFlagUnderP(uint8 *flag, uint8 *flagEx) {
	uint8 type, flagX, flagY;
	checkOne(data.word(kMousex) - data.word(kMapadx), data.word(kMousey) - data.word(kMapady), flag, flagEx, &type, &flagX, &flagY);
	cl = data.byte(kLastflag) = *flag;
	ch = data.byte(kLastflagex) = *flagEx;
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

void DreamGenContext::obName() {
	obName(al, ah);
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

	getFlagUnderP();
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

void DreamGenContext::delPointer() {
	if (data.word(kOldpointerx) == 0xffff)
		return;
	data.word(kDelherex) = data.word(kOldpointerx);
	data.word(kDelherey) = data.word(kOldpointery);
	data.byte(kDelxs) = data.byte(kPointerxs);
	data.byte(kDelys) = data.byte(kPointerys);
	multiPut(getSegment(data.word(kBuffers)).ptr(kPointerback, 0), data.word(kDelherex), data.word(kDelherey), data.byte(kPointerxs), data.byte(kPointerys));
}

void DreamGenContext::showBlink() {
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

void DreamGenContext::dumpBlink() {
	if (data.byte(kShadeson) != 0)
		return;
	if (data.byte(kBlinkcount) != 0)
		return;
	if (data.byte(kBlinkframe) >= 6)
		return;
	multiDump(44, 32, 16, 12);
}

void DreamGenContext::dumpPointer() {
	dumpBlink();
	multiDump(data.word(kDelherex), data.word(kDelherey), data.byte(kDelxs), data.byte(kDelys));
	if ((data.word(kOldpointerx) != data.word(kDelherex)) || (data.word(kOldpointery) != data.word(kDelherey)))
		multiDump(data.word(kOldpointerx), data.word(kOldpointery), data.byte(kPointerxs), data.byte(kPointerys));
}

void DreamGenContext::checkCoords() {

	// FIXME: Move all these lists to the callers

	switch ((uint16)bx) {
	case offset_talklist: {
		RectWithCallback talkList[] = {
			{ 273,320,157,198,&DreamGenContext::getBack1 },
			{ 240,290,2,44,&DreamGenContext::moreTalk },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(talkList);
		break;
	}
	case offset_quitlist: {
		RectWithCallback quitList[] = {
			{ 273,320,157,198,&DreamGenContext::getBack1 },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(quitList);
		break;
	}
	case offset_destlist: {
		RectWithCallback destList[] = {
			{ 238,258,4,44,&DreamGenContext::nextDest },
			{ 104,124,4,44,&DreamGenContext::lastDest },
			{ 280,308,4,44,&DreamGenContext::lookAtPlace },
			{ 104,216,138,192,&DreamGenContext::destSelect },
			{ 273,320,157,198,&DreamGenContext::getBack1 },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(destList);
		break;
	}
	case offset_diarylist: {
		RectWithCallback diaryList[] = {
			{ kDiaryx+94,kDiaryx+110,kDiaryy+97,kDiaryy+113,&DreamGenContext::diaryKeyN },
			{ kDiaryx+151,kDiaryx+167,kDiaryy+71,kDiaryy+87,&DreamGenContext::diaryKeyP },
			{ kDiaryx+176,kDiaryx+192,kDiaryy+108,kDiaryy+124,&DreamGenContext::quitKey },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(diaryList);
		break;
	}
	case offset_opslist: {
		RectWithCallback opsList[] = {
			{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamGenContext::getBackFromOps },
			{ kOpsx+10,kOpsx+77,kOpsy+10,kOpsy+59,&DreamGenContext::DOSReturn },
			{ kOpsx+128,kOpsx+190,kOpsy+16,kOpsy+100,&DreamGenContext::discOps },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(opsList);
		break;
	}
	case offset_discopslist: {
		RectWithCallback discOpsList[] = {
			{ kOpsx+59,kOpsx+114,kOpsy+30,kOpsy+76,&DreamGenContext::loadGame },
			{ kOpsx+10,kOpsx+79,kOpsy+10,kOpsy+59,&DreamGenContext::saveGame },
			{ kOpsx+176,kOpsx+192,kOpsy+60,kOpsy+76,&DreamGenContext::getBackToOps },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(discOpsList);
		break;
	}
	case offset_decidelist: {
		RectWithCallback decideList[] = {
			{ kOpsx+69,kOpsx+124,kOpsy+30,kOpsy+76,&DreamGenContext::newGame },
			{ kOpsx+20,kOpsx+87,kOpsy+10,kOpsy+59,&DreamGenContext::DOSReturn },
			{ kOpsx+123,kOpsx+190,kOpsy+10,kOpsy+59,&DreamGenContext::loadOld },
			{ 0,320,0,200,&DreamGenContext::blank },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(decideList);
		break;
	}
	default:
		::error("Unimplemented checkcoords() call");
	}
}

void DreamGenContext::checkCoords(const RectWithCallback *rectWithCallbacks) {
	if (data.byte(kNewlocation) != 0xff)
		return;

	const RectWithCallback *r;
	for (r = rectWithCallbacks; r->_xMin != 0xffff; ++r) {
		if (r->contains(data.word(kMousex), data.word(kMousey))) {
			(this->*(r->_callback))();
			return;
		}
	}
}

void DreamGenContext::showPointer() {
	showBlink();
	uint16 x = data.word(kMousex);
	data.word(kOldpointerx) = data.word(kMousex);
	uint16 y = data.word(kMousey);
	data.word(kOldpointery) = data.word(kMousey);
	if (data.byte(kPickup) == 1) {
		const Frame *frames;
		if (data.byte(kObjecttype) != 4)
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
		multiGet(getSegment(data.word(kBuffers)).ptr(kPointerback, 0), xMin, yMin, width, height);
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
		multiGet(getSegment(data.word(kBuffers)).ptr(kPointerback, 0), x, y, width, height);
		showFrame(engine->icons1(), x, y, data.byte(kPointerframe) + 20, 0);
	}
}

void DreamGenContext::animPointer() {

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

void DreamGenContext::printMessage() {
	printMessage(di, bx, al, dl, (bool)(dl & 1));
}

void DreamGenContext::printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered) {
	uint16 offset = kTextstart + getSegment(data.word(kCommandtext)).word(index * 2);
	const uint8 *string = getSegment(data.word(kCommandtext)).ptr(offset, 0);
	printDirect(&string, x, &y, maxWidth, centered);
}

void DreamGenContext::compare() {
	char id[4] = { cl, ch, dl, dh };
	flags._z = compare(al, ah, id);
}

bool DreamGenContext::compare(uint8 index, uint8 flag, const char id[4]) {
	void *ptr = getAnyAdDir(index, flag);
	const char *objId = (const char *)(((const uint8 *)ptr) + 12); // whether it is a DynObject or a SetObject
	for (size_t i = 0; i < 4; ++i) {
		if(id[i] != objId[i] + 'A')
			return false;
	}
	return true;
}

bool DreamGenContext::isItDescribed(const ObjPos *pos) {
	uint16 offset = getSegment(data.word(kSetdesc)).word(kSettextdat + pos->index * 2);
	uint8 result = getSegment(data.word(kSetdesc)).byte(kSettext + offset);
	return result != 0;
}

bool DreamGenContext::isCD() {
	// The original sources has two codepaths depending if the game is 'if cd' or not
	// This is a hack to guess which version to use with the assumption that if we have a cd version
	// we managed to load the speech. At least it is isolated in this function and can be changed.
	// Maybe detect the version during game id?
	return (data.byte(kSpeechloaded) == 1);
}

void DreamGenContext::showIcon() {
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

void DreamGenContext::checkIfSet() {
	flags._z = !checkIfSet(al, ah);
}

bool DreamGenContext::checkIfSet(uint8 x, uint8 y) {
	const ObjPos *setList = (const ObjPos *)getSegment(data.word(kBuffers)).ptr(kSetlist, sizeof(ObjPos) * 128);
	for (size_t i = 0; i < 128; ++i) {
		const ObjPos *pos = setList + 127 - i;
		if (pos->index == 0xff || !pos->contains(x,y))
			continue;
		if (! pixelCheckSet(pos, x, y))
			continue;
		if (! isItDescribed(pos))
			continue;
		obName(pos->index, 1);
		return true;
	}
	return false;
}

void DreamGenContext::showRyanPage() {
	showFrame(engine->icons1(), kInventx + 167, kInventy - 12, 12, 0);
	showFrame(engine->icons1(), kInventx + 167 + 18 * data.byte(kRyanpage), kInventy - 12, 13 + data.byte(kRyanpage), 0);
}

void DreamGenContext::findAllRyan() {
	findAllRyan(es.ptr(di, 60));
}

void DreamGenContext::findAllRyan(uint8 *inv) {
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

void DreamGenContext::hangOn() {
	hangOn(cx);
}

void DreamGenContext::hangOn(uint16 frameCount) {
	while (frameCount) {
		vSync();
		--frameCount;
		if (quitRequested())
			break;
	}
}

void DreamGenContext::hangOnW() {
	hangOnW(cx);
}

void DreamGenContext::hangOnW(uint16 frameCount) {
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

void DreamGenContext::hangOnP() {
	hangOnP(cx);
}

void DreamGenContext::hangOnP(uint16 count) {
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
	uint8 *initialString = es.ptr(si, 0);
	uint8 *string = initialString;
	al = findNextColon(&string);
	si += (string - initialString);
}

uint8 DreamGenContext::findNextColon(uint8 **string) {
	uint8 c;
	do {
		c = **string;
		++(*string);
	} while ((c != 0) && (c != ':'));
	return c;
}

uint8 *DreamGenContext::getObTextStartCPP() {
	push(es);
	push(si);
	getObTextStart();
	uint8 *result = es.ptr(si, 0);
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
		RectWithCallback symbolList[] = {
			{ kSymbolx+40,kSymbolx+64,kSymboly+2,kSymboly+16,&DreamGenContext::quitSymbol },
			{ kSymbolx,kSymbolx+52,kSymboly+20,kSymboly+50,&DreamGenContext::setTopLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+20,kSymboly+50,&DreamGenContext::setTopRight },
			{ kSymbolx,kSymbolx+52,kSymboly+50,kSymboly+80,&DreamGenContext::setBotLeft },
			{ kSymbolx+52,kSymbolx+104,kSymboly+50,kSymboly+80,&DreamGenContext::setBotRight },
			{ 0,320,0,200,&DreamGenContext::blank },
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
	if (data.word(kWatchingtime) != 0) {
		blank();
		return;
	}
	if (data.byte(kPointermode) == 2) {
		blank();
		return;
	}
	if (data.byte(kCommandtype) != 222) {
		data.byte(kCommandtype) = 222;
		commandOnly(39);
	}
	if (data.word(kMousebutton) == data.word(kOldbutton))
		return;
	if ((data.word(kMousebutton) & 1) == 0)
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

void DreamGenContext::sortOutMap() {
	const uint8 *src = workspace();
	uint8 *dst = (uint8 *)getSegment(data.word(kMapdata)).ptr(0, 0);
	for (uint16 y = 0; y < kMaplength; ++y) {
		memcpy(dst, src, kMapwidth);
		dst += kMapwidth;
		src += 132;
	}
}

void DreamGenContext::showCity() {
	clearWork();
	showFrame(tempGraphics(), 57, 32, 0, 0);
	showFrame(tempGraphics(), 120+57, 32, 1, 0);
}

void DreamGenContext::mainScreen() {
	data.byte(kInmaparea) = 0;
	if (data.byte(kWatchon) == 1) {
		RectWithCallback mainList[] = {
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
		RectWithCallback mainList2[] = {
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

void DreamGenContext::showWatch() {
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

void DreamGenContext::showTime() {
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

void DreamGenContext::roomName() {
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

void DreamGenContext::zoomIcon() {
	if (data.byte(kZoomon) == 0)
		return;
	showFrame(engine->icons1(), kZoomx, kZoomy-1, 8, 0);
}

void DreamGenContext::loadRoom() {
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
	getDimension();
}

void DreamGenContext::loadRoomsSample() {
	uint8 sample = data.byte(kRoomssample);

	if (sample == 255 || data.byte(kCurrentsample) == sample)
		return; // loaded already

	assert(sample < 100);
	Common::String sampleName = Common::String::format("DREAMWEB.V%02d", sample);

	uint8 ch0 = data.byte(kCh0playing);
	if (ch0 >= 12 && ch0 != 255)
		cancelCh0();
	uint8 ch1 = data.byte(kCh1playing);
	if (ch1 >= 12)
		cancelCh1();
	engine->loadSounds(1, sampleName.c_str());
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
	if (data.byte(kSoundint) == 0xff)
		return;

	//engine->openFile("DREAMWEB.VOL");
	//uint8 *volumeTab = getSegment(data.word(kSoundbuffer)).ptr(16384, 0);
	//engine->readFromFile(volumeTab, 2048-256);
	//engine->closeFile();
}

Frame * DreamGenContext::tempGraphics() {
	return (Frame *)getSegment(data.word(kTempgraphics)).ptr(0, 0);
}

Frame * DreamGenContext::tempGraphics2() {
	return (Frame *)getSegment(data.word(kTempgraphics2)).ptr(0, 0);
}

Frame * DreamGenContext::tempGraphics3() {
	return (Frame *)getSegment(data.word(kTempgraphics3)).ptr(0, 0);
}

void DreamGenContext::playChannel0(uint8 index, uint8 repeat) {
	if (data.byte(kSoundint) == 255)
		return;

	data.byte(kCh0playing) = index;
	Sound *soundBank;
	if (index >= 12) {
		soundBank = (Sound *)getSegment(data.word(kSounddata2)).ptr(0, 0);
		index -= 12;
	} else
		soundBank = (Sound *)getSegment(data.word(kSounddata)).ptr(0, 0);

	data.byte(kCh0repeat) = repeat;
	data.word(kCh0emmpage) = soundBank[index].emmPage;
	data.word(kCh0offset) = soundBank[index].offset();
	data.word(kCh0blockstocopy) = soundBank[index].blockCount();
	if (repeat) {
		data.word(kCh0oldemmpage) = data.word(kCh0emmpage);
		data.word(kCh0oldoffset) = data.word(kCh0offset);
		data.word(kCh0oldblockstocopy) = data.word(kCh0blockstocopy);
	}
}

void DreamGenContext::playChannel0() {
	playChannel0(al, ah);
}

void DreamGenContext::playChannel1(uint8 index) {
	if (data.byte(kSoundint) == 255)
		return;
	if (data.byte(kCh1playing) == 7)
		return;

	data.byte(kCh1playing) = index;
	Sound *soundBank;
	if (index >= 12) {
		soundBank = (Sound *)getSegment(data.word(kSounddata2)).ptr(0, 0);
		index -= 12;
	} else
		soundBank = (Sound *)getSegment(data.word(kSounddata)).ptr(0, 0);

	data.word(kCh1emmpage) = soundBank[index].emmPage;
	data.word(kCh1offset) = soundBank[index].offset();
	data.word(kCh1blockstocopy) = soundBank[index].blockCount();
}

void DreamGenContext::playChannel1() {
	playChannel1(al);
}

void DreamGenContext::findRoomInLoc() {
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
	uint8 *string = getSegment(data.word(kRoomdesc)).ptr(kIntext, 0) + offset;
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

void DreamGenContext::useCharset1() {
	engine->setCurrentCharset((Frame *)getSegment(data.word(kCharset1)).ptr(0, 0));
}

void DreamGenContext::useTempCharset() {
	engine->setCurrentCharset(engine->tempCharset());
}

void DreamGenContext::getRidOfTemp() {
	deallocateMem(data.word(kTempgraphics));
}

void DreamGenContext::getRidOfTempText() {
	deallocateMem(data.word(kTextfile1));
}

void DreamGenContext::getRidOfTemp2() {
	deallocateMem(data.word(kTempgraphics2));
}

void DreamGenContext::getRidOfTemp3() {
	deallocateMem(data.word(kTempgraphics3));
}

void DreamGenContext::getRidOfTempCharset() {
	engine->freeTempCharset();
}

void DreamGenContext::getRidOfTempsP() {
	deallocateMem(data.word(kTempsprites));
}

void DreamGenContext::getRidOfAll() {
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
void DreamGenContext::loadRoomData(const Room &room, bool skipDat) {
	engine->openFile(room.name);
	cs.word(kHandle) = 1; //only one handle
	flags._c = false;

	FileHeader header;
	engine->readFromFile((uint8 *)&header, sizeof(FileHeader));

	// read segment lengths from room file header
	int len[15];
	for (int i = 0; i < 15; ++i)
		len[i] = header.len(i);

	data.word(kBackdrop) = allocateAndLoad(len[0]);
	clearAndLoad(data.word(kWorkspace), 0, len[1], 132*66); // 132*66 = maplen
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

	closeFile();
}

void DreamGenContext::restoreAll() {
	const Room &room = g_roomData[data.byte(kLocation)];
	loadRoomData(room, true);
	setAllChanges();
}

void DreamGenContext::restoreReels() {
	if (data.byte(kRoomloaded) == 0)
		return;

	const Room &room = g_roomData[data.byte(kReallocation)];

	engine->openFile(room.name);
	cs.word(kHandle) = 1; //only one handle
	flags._c = false;

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

	closeFile();
}

void DreamGenContext::loadFolder() {
	loadIntoTemp("DREAMWEB.G09"); // folder graphics 1
	loadIntoTemp2("DREAMWEB.G10"); // folder graphics 2
	loadIntoTemp3("DREAMWEB.G11"); // folder graphics 3
	loadTempCharset("DREAMWEB.C02"); // character set 3
	loadTempText("DREAMWEB.T50"); // folder text
}

void DreamGenContext::showFolder() {
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

void DreamGenContext::showLeftPage() {
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

void DreamGenContext::showRightPage() {
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

void DreamGenContext::showExit() {
	showFrame(engine->icons1(), 274, 154, 11, 0);
}

void DreamGenContext::showMan() {
	showFrame(engine->icons1(), 0, 0, 0, 0);
	showFrame(engine->icons1(), 0, 114, 1, 0);
	if (data.byte(kShadeson))
		showFrame(engine->icons1(), 28, 25, 2, 0);
}

void DreamGenContext::panelIcons1() {
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

void DreamGenContext::examIcon() {
	showFrame(engine->icons2(), 254, 5, 3, 0);
}

uint8 DreamGenContext::getLocation(uint8 index) {
	return data.byte(kRoomscango + index);
}

void DreamGenContext::getLocation() {
	al = getLocation(al);
}

void DreamGenContext::setLocation(uint8 index) {
	data.byte(kRoomscango + index) = 1;
}

void DreamGenContext::setLocation() {
	setLocation(al);
}

const uint8 *DreamGenContext::getTextInFile1(uint16 index) {
	uint16 offset = getSegment(data.word(kTextfile1)).word(index * 2) + kTextstart;
	const uint8 *string = getSegment(data.word(kTextfile1)).ptr(offset, 0);
	return string;
}

void DreamGenContext::checkFolderCoords() {
	RectWithCallback folderList[] = {
		{ 280,320,160,200, &DreamGenContext::quitKey },
		{ 143,300,6,194, &DreamGenContext::nextFolder },
		{ 0,143,6,194, &DreamGenContext::lastFolder },
		{ 0,320,0,200, &DreamGenContext::blank },
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

void DreamGenContext::folderHints() {
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

void DreamGenContext::folderExit() {
	showFrame(tempGraphics2(), 296, 178, 6, 0);
}

void DreamGenContext::loadTravelText() {
	data.word(kTraveltext) = standardLoad("DREAMWEB.T81"); // location descs
}

void DreamGenContext::loadTempText() {
	loadTempText((const char *)cs.ptr(dx, 0));
}

void DreamGenContext::loadTempText(const char *fileName) {
	data.word(kTextfile1) = standardLoad(fileName);
}

void DreamGenContext::drawFloor() {
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

void DreamGenContext::allocateBuffers() {
	data.word(kExtras) = allocateMem(kLengthofextra/16);
	data.word(kMapdata) = allocateMem(kLengthofmap/16);
	data.word(kBuffers) = allocateMem(kLengthofbuffer/16);
	data.word(kFreedat) = allocateMem(kFreedatlen/16);
	data.word(kSetdat) = allocateMem(kSetdatlen/16);
	data.word(kMapstore) = allocateMem(kLenofmapstore/16);
	allocateWork();
	data.word(kSounddata) = allocateMem(2048/16);
	data.word(kSounddata2) = allocateMem(2048/16);
}

void DreamGenContext::workToScreenM() {
	animPointer();
	readMouse();
	showPointer();
	vSync();
	workToScreen();
	delPointer();
}

void DreamGenContext::loadMenu() {
	loadIntoTemp("DREAMWEB.S02"); // sprite name 3
	loadIntoTemp2("DREAMWEB.G07"); // mon. graphics 2
}

void DreamGenContext::showMenu() {
	++data.byte(kMenucount);
	if (data.byte(kMenucount) == 37*2)
		data.byte(kMenucount) = 0;
	showFrame(tempGraphics(), kMenux, kMenuy, data.byte(kMenucount) / 2, 0);
}

void DreamGenContext::dumpMenu() {
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
		RectWithCallback menuList[] = {
			{ kMenux+54,kMenux+68,kMenuy+72,kMenuy+88,&DreamGenContext::quitKey },
			{ 0,320,0,200,&DreamGenContext::blank },
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

void DreamGenContext::atmospheres() {

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

void DreamGenContext::readCityPic() {
	loadIntoTemp("DREAMWEB.G04");
}

void DreamGenContext::readDestIcon() {
	loadIntoTemp("DREAMWEB.G05");
	loadIntoTemp2("DREAMWEB.G06");
	loadIntoTemp3("DREAMWEB.G08");
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

void DreamGenContext::readKey() {
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

void DreamGenContext::intro() {
	loadTempText("DREAMWEB.T82");
	loadPalFromIFF();
	setMode();
	data.byte(kNewlocation) = 50;
	clearPalette();
	loadIntroRoom();
	data.byte(kVolume) = 7;
	data.byte(kVolumedirection) = -1;
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

void DreamGenContext::setTopLeft() {
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

void DreamGenContext::setTopRight() {
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

void DreamGenContext::setBotLeft() {
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

void DreamGenContext::setBotRight() {
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

void DreamGenContext::newGame() {
	if (data.byte(kCommandtype) != 251) {
		data.byte(kCommandtype) = 251;
		commandOnly(47);
	}

	if (data.word(kMousebutton) == 1)
		data.byte(kGetback) = 3;
}

void DreamGenContext::getBackFromOps() {
	if (data.byte(kMandead) == 2)
		blank();
	else
		getBack1();
}

void DreamGenContext::getBackToOps() {
	if (data.byte(kCommandtype) != 201) {
		data.byte(kCommandtype) = 201;
		commandOnly(42);
	}

	if (data.word(kMousebutton) != data.word(kOldbutton)) {
		if (data.word(kMousebutton) & 1) {
			oldToNames();
			data.byte(kGetback) = 2;
		}
	}
}

void DreamGenContext::pickupOb(uint8 command, uint8 pos) {
	data.byte(kLastinvpos) = pos;
	data.byte(kObjecttype) = 2;
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
	workToScreen();
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
	workToScreen();
	data.byte(kNowinnewroom) = 0;
}

void DreamGenContext::gettingShot() {
	data.byte(kNewlocation) = 55;
	clearPalette();
	loadIntroRoom();
	fadeScreenUps();
	data.byte(kVolumeto) = 0;
	data.byte(kVolumedirection) = 0xFF;
	runEndSeq();
	clearBeforeLoad();
}

void DreamGenContext::redrawMainScrn() {
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

void DreamGenContext::selectSlot2() {
	if (data.word(kMousebutton) ==  0)
		selectSlot();
	else
		data.byte(kLoadingorsave) = 2;
}

void DreamGenContext::blank() {
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

} /*namespace dreamgen */

