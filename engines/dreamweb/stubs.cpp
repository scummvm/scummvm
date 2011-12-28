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

// Keyboard buffer. _bufferIn and _bufferOut are indexes
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

void DreamBase::dreamweb() {
	switch(engine->getLanguage()) {
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
		_foreignRelease = false;
		break;
	default:
		_foreignRelease = true;
		break;
	}

	seeCommandTail();
	// soundStartup used to be done here...
	// setKeyboardInt used to be done here...
	allocateBuffers();

	// setMouse
	_oldPointerX = 0xffff;

	fadeDOS();
	getTime();
	clearBuffers();
	clearPalette();
	set16ColPalette();
	readSetData();
	_wonGame = false;

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
			if (_quitRequested)
				goto done;

			if (_getBack == 4)
				startNewGame = false; // savegame has been loaded

		}

		firstLoop = false;

		if (startNewGame) {
			// "playGame"

			// "titles"
			// TODO: In the demo version, titles() did nothing
			clearPalette();
			bibleQuote();
			if (!_quitRequested) // "titlesearly"
				intro();

			if (_quitRequested)
				goto done;

			// "credits"
			clearPalette();
			realCredits();

			if (_quitRequested)
				goto done;

			clearChanges();
			setMode();
			loadPalFromIFF();
			data.byte(kLocation) = 255;
			data.byte(kRoomafterdream) = 1;
			_newLocation = 35;
			_volume = 7;
			loadRoom();
			clearSprites();
			initMan();
			entryTexts();
			entryAnims();
			_destPos = 3;
			initialInv();
			_lastFlag = 32;
			startup1();
			_volumeTo = 0;
			_volumeDirection = -1;
			_commandType = 255;

		}

		// main loop
		while (true) {
			if (_quitRequested)
				goto done;

			screenUpdate();

			if (_quitRequested)
				goto done;

			if (_wonGame) {
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
				if (_finalDest == _mansPath)
					data.word(kWatchingtime)--;
			}

			if (data.word(kWatchingtime) == 0) {
				// "notWatching"

				if (data.byte(kMandead) == 4)
					break;

				if (_newLocation != 255) {
					// "loadNew"
					clearBeforeLoad();
					loadRoom();
					clearSprites();
					initMan();
					entryTexts();
					entryAnims();
					_newLocation = 255;
					startup();
					_commandType = 255;
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

	_icons1.clear();
	_icons2.clear();
	_charset1.clear();
	_tempGraphics.clear();
	_tempGraphics2.clear();
	_tempGraphics3.clear();
	_tempCharset.clear();
	_mainSprites.clear();

	_exFrames.clear();
	_exText.clear();

	_setFrames.clear();
	_freeFrames.clear();
	_reel1.clear();
	_reel2.clear();
	_reel3.clear();
	_setDesc.clear();
	_blockDesc.clear();
	_roomDesc.clear();
	_freeDesc.clear();
	_personText.clear();

	_textFile1.clear();
	_textFile2.clear();
	_textFile3.clear();
	_travelText.clear();
	_puzzleText.clear();
	_commandText.clear();
}

void DreamBase::loadTextFile(TextFile &file, const char *fileName)
{
	FileHeader header;

	Common::File f;
	f.open(fileName);
	f.read((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);
	assert(sizeInBytes >= 2*66);

	delete[] file._text;
	file._text = new char[sizeInBytes - 2*66];

	f.read(file._offsetsLE, 2*66);
	f.read(file._text, sizeInBytes - 2*66);
}

void DreamBase::screenUpdate() {
	newPlace();
	mainScreen();
	if (_quitRequested)
		return;
	animPointer();

	showPointer();
	if ((data.word(kWatchingtime) == 0) && (_newLocation != 0xff))
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
	if (_wonGame)
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
	_oldButton = _mouseButton;
	mouseState |= readMouseState();
	_mouseButton = mouseState;
	dumpPointer();

	dumpWatch();
	delPointer();
}

void DreamBase::startup() {
	_currentKey = 0;
	_mainMode = 0;
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

void DreamBase::startup1() {
	clearPalette();
	data.byte(kThroughdoor) = 0;

	startup();

	workToScreen();
	fadeScreenUp();
}

void DreamBase::switchRyanOn() {
	data.byte(kRyanon) = 255;
}

void DreamBase::switchRyanOff() {
	data.byte(kRyanon) = 1;
}

void DreamBase::loadGraphicsFile(GraphicsFile &file, const char *fileName) {
	FileHeader header;

	Common::File f;
	f.open(fileName);
	f.read((uint8 *)&header, sizeof(FileHeader));
	uint16 sizeInBytes = header.len(0);

	assert(sizeInBytes >= 2080);
	delete[] file._data;
	file._data = new uint8[sizeInBytes - 2080];

	f.read((uint8 *)file._frames, 2080);
	f.read(file._data, sizeInBytes - 2080);
}

void DreamBase::loadGraphicsSegment(GraphicsFile &file, unsigned int len) {
	assert(len >= 2080);
	delete[] file._data;
	file._data = new uint8[len - 2080];
	engine->readFromFile((uint8 *)file._frames, 2080);
	engine->readFromFile(file._data, len - 2080);
}

void DreamBase::loadTextSegment(TextFile &file, unsigned int len) {
	unsigned int headerSize = 2 * file._size;
	assert(len >= headerSize);
	delete[] file._text;
	file._text = new char[len - headerSize];
	engine->readFromFile((uint8 *)file._offsetsLE, headerSize);
	engine->readFromFile((uint8 *)file._text, len - headerSize);
}

void DreamBase::loadIntoTemp(const char *fileName) {
	loadGraphicsFile(_tempGraphics, fileName);
}

void DreamBase::loadIntoTemp2(const char *fileName) {
	loadGraphicsFile(_tempGraphics2, fileName);
}

void DreamBase::loadIntoTemp3(const char *fileName) {
	loadGraphicsFile(_tempGraphics3, fileName);
}

void DreamBase::loadTempCharset(const char *fileName) {
	loadGraphicsFile(_tempCharset, fileName);
}

void DreamBase::hangOnCurs(uint16 frameCount) {
	for (uint16 i = 0; i < frameCount; ++i) {
		printCurs();
		vSync();
		delCurs();
	}
}

void DreamBase::seeCommandTail() {
	_brightness = 1;
}

void DreamBase::quickQuit() {
	engine->quit();
}

void DreamBase::quickQuit2() {
	engine->quit();
}

void DreamBase::readMouse() {
	_oldButton = _mouseButton;
	uint16 state = readMouseState();
	_mouseButton = state;
}

uint16 DreamBase::readMouseState() {
	_oldX = _mouseX;
	_oldY = _mouseY;
	uint16 x, y, state;
	engine->mouseCall(&x, &y, &state);
	_mouseX = x;
	_mouseY = y;
	return state;
}

void DreamBase::dumpTextLine() {
	if (_newTextLine != 1)
		return;
	_newTextLine = 0;
	uint16 x = _textAddressX;
	uint16 y = _textAddressY;
	if (_foreignRelease)
		y -= 3;
	multiDump(x, y, 228, 13);
}

void DreamBase::getUnderTimed() {
	if (_foreignRelease)
		multiGet(_underTimedText, _timedX, _timedY - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiGet(_underTimedText, _timedX, _timedY, 240, kUnderTimedTextSizeY);
}

void DreamBase::putUnderTimed() {
	if (_foreignRelease)
		multiPut(_underTimedText, _timedX, _timedY - 3, 240, kUnderTimedTextSizeY_f);
	else
		multiPut(_underTimedText, _timedX, _timedY, 240, kUnderTimedTextSizeY);
}

void DreamBase::triggerMessage(uint16 index) {
	multiGet(_mapStore, 174, 153, 200, 63);
	const uint8 *string = (const uint8 *)_puzzleText.getString(index);
	uint16 y = 156;
	printDirect(&string, 174, &y, 141, true);
	hangOn(140);
	workToScreen();
	hangOn(340);
	multiPut(_mapStore, 174, 153, 200, 63);
	workToScreen();
	data.byte(kLasttrigger) = 0;
}

void DreamBase::processTrigger() {
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
	if (_timeCount == 0)
		return;
	--_timeCount;
	if (_timeCount == 0) {
		putUnderTimed();
		_needToDumpTimed = 1;
		return;
	}

	if (_timeCount == _countToTimed)
		getUnderTimed();
	else if (_timeCount > _countToTimed)
		return;

	const uint8 *string = (const uint8 *)_timedString;
	printDirect(string, _timedX, _timedY, 237, true);
	_needToDumpTimed = 1;
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
		if (_speechLoaded && !_subtitles)
			return;
	}
#endif

	if (_timeCount != 0)
		return;
	_timedY = y;
	_timedX = x;
	_countToTimed = countToTimed;
	_timeCount = timeCount + countToTimed;
	_timedString = _textFile1.getString(textIndex);
	debug(1, "setupTimedTemp: (%d, %d) => '%s'", textIndex, voiceIndex, _timedString);
}

void DreamBase::dumpTimedText() {
	if (_needToDumpTimed != 1)
		return;
	uint8 y = _timedY;
	if (_foreignRelease)
		y -= 3;

	multiDump(_timedX, y, 240, kUndertimedysize);
	_needToDumpTimed = 0;
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
}

void DreamBase::DOSReturn() {
	if (_commandType != 250) {
		_commandType = 250;
		commandOnly(46);
	}

	if (_mouseButton & 1) {
		_mouseButton = 0;
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
		if (sprite._objData)
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
	if (_lastHardKey == 57) {
		// Clear the keyboard buffer. Otherwise the space that caused
		// the pause will be read immediately unpause the game.
		do {
			readKey();
		} while (_currentKey != 0);

		lockLightOn();
		while (!engine->shouldQuit()) {
			engine->waitForVSync();
			readKey();
			if (_currentKey == ' ')
				break;
		}
		// Forget the last "hard" key, otherwise the space that caused
		// the unpausing will immediately re-pause the game.
		_lastHardKey = 0;
		lockLightOff();
	}
}

void DreamBase::clearAndLoad(uint8 *buf, uint8 c,
                                   unsigned int size, unsigned int maxSize) {
	assert(size <= maxSize);
	memset(buf, c, maxSize);
	engine->readFromFile(buf, size);
}

void DreamBase::startLoading(const Room &room) {
	data.byte(kCombatcount) = 0;
	_roomsSample = room.roomsSample;
	_mapX = room.mapX;
	_mapY = room.mapY;
	data.byte(kLiftflag) = room.liftFlag;
	_mansPath = room.b21;
	_destination = room.b21;
	_finalDest = room.b21;
	_facing = room.facing;
	_turnToFace = room.facing;
	data.byte(kCounttoopen) = room.countToOpen;
	data.byte(kLiftpath) = room.liftPath;
	data.byte(kDoorpath) = room.doorPath;
	data.byte(kLastweapon) = (uint8)-1;
	_realLocation = room.realLocation;

	loadRoomData(room, false);

	findRoomInLoc();
	deleteTaken();
	setAllChanges();
	autoAppear();
//	const Room &newRoom = g_roomData[_newLocation];
	data.byte(kLastweapon) = (uint8)-1;
	data.byte(kMandead) = 0;
	_lookCounter = 160;
	_newLocation = 255;
	_linePointer = 254;
	if (room.b27 != 255) {
		_mansPath = room.b27;
		autoSetWalk();
	}
	findXYFromPath();
}

void DreamBase::dealWithSpecial(uint8 firstParam, uint8 secondParam) {
	uint8 type = firstParam - 220;
	if (type == 0) {
		placeSetObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 1) {
		removeSetObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 2) {
		placeFreeObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 3) {
		removeFreeObject(secondParam);
		_haveDoneObs = 1;
	} else if (type == 4) {
		switchRyanOff();
	} else if (type == 5) {
		_turnToFace = secondParam;
		_facing = secondParam;
		switchRyanOn();
	} else if (type == 6) {
		_newLocation = secondParam;
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
	if ((_commandType != 3) && (_commandType < 10)) {
		frame = 9;
	} else {
		frame = 29;
	}
	showFrame(_icons1, kZoomx + 24, kZoomy + 19, frame, 0);
}

void DreamBase::delTextLine() {
	if (_foreignRelease)
		multiPut(_textUnder, _textAddressX, _textAddressY - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiPut(_textUnder, _textAddressX, _textAddressY, kUnderTextSizeX, kUnderTextSizeY);
}

void DreamBase::commandOnly(uint8 command) {
	delTextLine();
	const uint8 *string = (const uint8 *)_commandText.getString(command);
	printDirect(string, _textAddressX, _textAddressY, _textLen, (bool)(_textLen & 1));
	_newTextLine = 1;
}

bool DreamBase::checkIfPerson(uint8 x, uint8 y) {
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

bool DreamBase::checkIfFree(uint8 x, uint8 y) {
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

bool DreamBase::checkIfEx(uint8 x, uint8 y) {
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
		uint16 i = 64 * (index & 127);
		return (const uint8 *)_personText.getString(i);
	} else if (type == 4) {
		return (const uint8 *)_exText.getString(index);
	} else if (type == 2) {
		return (const uint8 *)_freeDesc.getString(index);
	} else if (type == 1) {
		return (const uint8 *)_setDesc.getString(index);
	} else {
		return (const uint8 *)_blockDesc.getString(index);
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

void DreamBase::commandWithOb(uint8 command, uint8 type, uint8 index) {
	uint8 commandLine[64] = "OBJECT NAME ONE                         ";
	delTextLine();
	uint8 textLen = _textLen;

	const uint8 *string = (const uint8 *)_commandText.getString(command);
	printDirect(string, _textAddressX, _textAddressY, textLen, (bool)(textLen & 1));

	copyName(type, index, commandLine);
	uint16 x = _lastXPos;
	if (command != 0)
		x += 5;
	printDirect(commandLine, x, _textAddressY, textLen, (bool)(textLen & 1));
	_newTextLine = 1;
}

void DreamBase::examineObText() {
	commandWithOb(1, _commandType, _command);
}

void DreamBase::blockNameText() {
	commandWithOb(0, _commandType, _command);
}

void DreamBase::personNameText() {
	commandWithOb(2, _commandType, _command & 127);
}

void DreamBase::walkToText() {
	commandWithOb(3, _commandType, _command);
}

void DreamBase::findOrMake(uint8 index, uint8 value, uint8 type) {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change) {
		if (index == change->index && _realLocation == change->location && type == change->type) {
			change->value = value;
			return;
		}
	}

	change->index = index;
	change->location = _realLocation;
	change->value = value;
	change->type = type;
}

void DreamBase::setAllChanges() {
	Change *change = _listOfChanges;
	for (; change->index != 0xff; ++change)
		if (change->location == _realLocation)
			doChange(change->index, change->value, change->type);
}

DynObject *DreamBase::getFreeAd(uint8 index) {
	return &_freeDat[index];
}

DynObject *DreamBase::getExAd(uint8 index) {
	return &_exData[index];
}

DynObject *DreamBase::getEitherAdCPP() {
	if (_objectType == kExObjectType)
		return getExAd(_itemFrame);
	else
		return getFreeAd(_itemFrame);
}

void *DreamBase::getAnyAd(uint8 *slotSize, uint8 *slotCount) {
	if (_objectType == kExObjectType) {
		DynObject *exObject = getExAd(_command);
		*slotSize = exObject->slotSize;
		*slotCount = exObject->slotCount;
		return exObject;
	} else if (_objectType == kFreeObjectType) {
		DynObject *freeObject = getFreeAd(_command);
		*slotSize = freeObject->slotSize;
		*slotCount = freeObject->slotCount;
		return freeObject;
	} else {	// 1 or 3. 0 should never happen
		SetObject *setObject = getSetAd(_command);
		// Note: the original returned slotCount/priority (bytes 4 and 5)
		// instead of slotSize/slotCount (bytes 3 and 4).
		// Changed this for consistency with the Ex/Free cases, and also
		// with getOpenedSize()
		*slotSize = setObject->slotSize;
		*slotCount = setObject->slotCount;
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
	return &_setDat[index];
}

void DreamBase::doChange(uint8 index, uint8 value, uint8 type) {
	if (type == 0) { //object
		getSetAd(index)->mapad[0] = value;
	} else if (type == 1) { //freeObject
		DynObject *freeObject = getFreeAd(index);
		if (freeObject->mapad[0] == 0xff)
			freeObject->mapad[0] = value;
	} else { //path
		_pathData[type - 100].nodes[index].on = value;
	}
}

void DreamBase::deleteTaken() {
	for (size_t i = 0; i < kNumexobjects; ++i) {
		uint8 location = _exData[i].initialLocation;
		if (location == _realLocation) {
			uint8 index = _exData[i].index;
			_freeDat[index].mapad[0] = 0xfe;
		}
	}
}

uint8 DreamBase::getExPos() {
	DynObject *objects = _exData;
	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (objects[i].mapad[0] == 0xff)
			return i;
	}
	error("Out of Ex object positions");
}

void DreamBase::placeSetObject(uint8 index) {
	findOrMake(index, 0, 0);
	getSetAd(index)->mapad[0] = 0;
}

void DreamBase::removeSetObject(uint8 index) {
	findOrMake(index, 0xff, 0);
	getSetAd(index)->mapad[0] = 0xff;
}

bool DreamBase::finishedWalking() {
	return (_linePointer == 254) && (_facing == _turnToFace);
}

void DreamBase::getFlagUnderP(uint8 *flag, uint8 *flagEx) {
	uint8 type, flagX, flagY;
	checkOne(_mouseX - _mapAdX, _mouseY - _mapAdY, flag, flagEx, &type, &flagX, &flagY);
	_lastFlag = *flag;
}

void DreamBase::walkAndExamine() {
	if (!finishedWalking())
		return;
	_commandType = _walkExamType;
	_command = _walkExamNum;
	_walkAndExam = 0;
	if (_commandType != 5)
		examineOb();
}

void DreamBase::obName(uint8 command, uint8 commandType) {
	if (_reAssesChanges == 0) {
		if ((commandType == _commandType) && (command == _command)) {
			if (_walkAndExam == 1) {
				walkAndExamine();
				return;
			} else if (_mouseButton == 0)
				return;
			else if ((_commandType == 3) && (_lastFlag < 2))
				return;
			else if ((_mansPath != _pointersPath) || (_commandType == 3)) {
				setWalk();
				_reAssesChanges = 1;
				return;
			} else if (!finishedWalking())
				return;
			else if (_commandType == 5) {
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
		_reAssesChanges = 0;

	_command = command;
	_commandType = commandType;
	if ((_linePointer != 254) || (data.word(kWatchingtime) != 0) || (_facing != _turnToFace)) {
		blockNameText();
		return;
	} else if (_commandType != 3) {
		if (_mansPath != _pointersPath) {
			walkToText();
			return;
		} else if (_commandType == 3) {
			blockNameText();
			return;
		} else if (_commandType == 5) {
			personNameText();
			return;
		} else {
			examineObText();
			return;
		}
	}
	if (_mansPath == _pointersPath) {
		uint8 flag, flagEx, type, flagX, flagY;
		checkOne(_ryanX + 12, _ryanY + 12, &flag, &flagEx, &type, &flagX, &flagY);
		if (flag < 2) {
			blockNameText();
			return;
		}
	}

	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (_lastFlag < 2) {
		blockNameText();
		return;
	} else if (_lastFlag >= 128) {
		blockNameText();
		return;
	} else {
		walkToText();
		return;
	}
}

void DreamBase::delPointer() {
	if (_oldPointerX == 0xffff)
		return;
	_delHereX = _oldPointerX;
	_delHereY = _oldPointerY;
	_delXS = _pointerXS;
	_delYS = _pointerYS;
	multiPut(_pointerBack, _delHereX, _delHereY, _pointerXS, _pointerYS);
}

void DreamBase::showBlink() {
	if (_manIsOffScreen == 1)
		return;
	++_blinkCount;
	if (data.byte(kShadeson) != 0)
		return;
	if (_realLocation >= 50) // eyesshut
		return;
	if (_blinkCount != 3)
		return;
	_blinkCount = 0;
	uint8 blinkFrame = _blinkFrame;
	++blinkFrame; // Implicit %256
	_blinkFrame = blinkFrame;
	if (blinkFrame > 6)
		blinkFrame = 6;
	static const uint8 blinkTab[] = { 16,18,18,17,16,16,16 };
	uint8 width, height;
	showFrame(_icons1, 44, 32, blinkTab[blinkFrame], 0, &width, &height);
}

void DreamBase::dumpBlink() {
	if (data.byte(kShadeson) != 0)
		return;
	if (_blinkCount != 0)
		return;
	if (_blinkFrame >= 6)
		return;
	multiDump(44, 32, 16, 12);
}

void DreamBase::dumpPointer() {
	dumpBlink();
	multiDump(_delHereX, _delHereY, _delXS, _delYS);
	if ((_oldPointerX != _delHereX) || (_oldPointerY != _delHereY))
		multiDump(_oldPointerX, _oldPointerY, _pointerXS, _pointerYS);
}

template <class T>
void DreamBase::checkCoords(const RectWithCallback<T> *rectWithCallbacks) {
	if (_newLocation != 0xff)
		return;

	const RectWithCallback<T> *r;
	for (r = rectWithCallbacks; r->_xMin != 0xffff; ++r) {
		if (r->contains(_mouseX, _mouseY)) {
			(((T *)this)->*(r->_callback))();
			return;
		}
	}
}

void DreamBase::showPointer() {
	showBlink();
	uint16 x = _mouseX;
	_oldPointerX = _mouseX;
	uint16 y = _mouseY;
	_oldPointerY = _mouseY;
	if (_pickUp == 1) {
		const GraphicsFile *frames;
		if (_objectType != kExObjectType)
			frames = &_freeFrames;
		else
			frames = &_exFrames;
		const Frame *frame = &frames->_frames[(3 * _itemFrame + 1)];

		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		_pointerXS = width;
		_pointerYS = height;
		uint16 xMin = (x >= width / 2) ? x - width / 2 : 0;
		uint16 yMin = (y >= height / 2) ? y - height / 2 : 0;
		_oldPointerX = xMin;
		_oldPointerY = yMin;
		multiGet(_pointerBack, xMin, yMin, width, height);
		showFrame(*frames, x, y, 3 * _itemFrame + 1, 128);
		showFrame(_icons1, x, y, 3, 128);
	} else {
		const Frame *frame = &_icons1._frames[_pointerFrame + 20];
		uint8 width = frame->width;
		uint8 height = frame->height;
		if (width < 12)
			width = 12;
		if (height < 12)
			height = 12;
		_pointerXS = width;
		_pointerYS = height;
		multiGet(_pointerBack, x, y, width, height);
		showFrame(_icons1, x, y, _pointerFrame + 20, 0);
	}
}

void DreamBase::animPointer() {

	if (_pointerMode == 2) {
		_pointerFrame = 0;
		if ((_realLocation == 14) && (_commandType == 211))
			_pointerFrame = 5;
		return;
	} else if (_pointerMode == 3) {
		if (_pointerSpeed != 0) {
			--_pointerSpeed;
		} else {
			_pointerSpeed = 5;
			++_pointerCount;
			if (_pointerCount == 16)
				_pointerCount = 0;
		}
		static const uint8 flashMouseTab[] = { 1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2 };
		_pointerFrame = flashMouseTab[_pointerCount];
		return;
	}
	if (data.word(kWatchingtime) != 0) {
		_pointerFrame = 11;
		return;
	}
	_pointerFrame = 0;
	if (_inMapArea == 0)
		return;
	if (_pointerFirstPath == 0)
		return;
	uint8 flag, flagEx;
	getFlagUnderP(&flag, &flagEx);
	if (flag < 2)
		return;
	if (flag >= 128)
		return;
	if (flag & 4) {
		_pointerFrame = 3;
		return;
	}
	if (flag & 16) {
		_pointerFrame = 4;
		return;
	}
	if (flag & 2) {
		_pointerFrame = 5;
		return;
	}
	if (flag & 8) {
		_pointerFrame = 6;
		return;
	}
	_pointerFrame = 8;
}

void DreamBase::printMessage(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered) {
	const uint8 *string = (const uint8 *)_commandText.getString(index);
	printDirect(string, x, y, maxWidth, centered);
}

void DreamBase::printMessage2(uint16 x, uint16 y, uint8 index, uint8 maxWidth, bool centered, uint8 count) {
	const uint8 *string = (const uint8 *)_commandText.getString(index);
	while (count--) {
		findNextColon(&string);
	}
	printDirect(string, x, y, maxWidth, centered);
}

bool DreamBase::objectMatches(void *object, const char *id) {
	const char *objId = (const char *)object + 12; // whether it is a DynObject or a SetObject
	for (size_t i = 0; i < 4; ++i) {
		if (id[i] != objId[i] + 'A')
			return false;
	}
	return true;
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

uint16 DreamBase::findExObject(const char *id) {
	for (uint16 index = 0; index < kNumexobjects; index++) {
		if (objectMatches(getExAd(index), id))
			return index;
	}

	return kNumexobjects;
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
	const char *string = _setDesc.getString(pos->index);
	return string[0] != 0;
}

bool DreamBase::isCD() {
	// The original sources has two codepaths depending if the game is 'if cd' or not
	// This is a hack to guess which version to use with the assumption that if we have a cd version
	// we managed to load the speech. At least it is isolated in this function and can be changed.
	// Maybe detect the version during game id?
	return (_speechLoaded);
}

void DreamBase::showIcon() {
	if (_realLocation < 50) {
		showPanel();
		showMan();
		roomName();
		panelIcons1();
		zoomIcon();
	} else {
		error("Unimplemented tempsprites code called");
		// the tempsprites segment is never initialized, but used here.
/*
		Frame *tempSprites = (Frame *)getSegment(data.word(kTempsprites)).ptr(0, 0);
		showFrame(tempSprites, 72, 2, 45, 0);
		showFrame(tempSprites, 72+47, 2, 46, 0);
		showFrame(tempSprites, 69-10, 21, 49, 0);
		showFrame(tempSprites, 160+88, 2, 45, 4 & 0xfe);
		showFrame(tempSprites, 160+43, 2, 46, 4 & 0xfe);
		showFrame(tempSprites, 160+101, 21, 49, 4 & 0xfe);

		// middle panel
		showFrame(tempSprites, 72 + 47 + 20, 0, 48, 0);
		showFrame(tempSprites, 72 + 19, 21, 47, 0);
		showFrame(tempSprites, 160 + 23, 0, 48, 4);
		showFrame(tempSprites, 160 + 71, 21, 47, 4);
*/
	}
}

bool DreamBase::checkIfSet(uint8 x, uint8 y) {
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

void DreamBase::hangOn(uint16 frameCount) {
	while (frameCount) {
		vSync();
		--frameCount;
		if (_quitRequested)
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
		if (_quitRequested)
			break;
	}
}

void DreamBase::hangOnP(uint16 count) {
	_mainTimer = 0;
	uint8 pointerFrame = _pointerFrame;
	uint8 pickup = _pickUp;
	_pointerMode = 3;
	_pickUp = 0;
	_commandType = 255;
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
		if (_quitRequested)
			break;
		if (_mouseButton != 0 && _mouseButton != _oldButton)
			break;
	}

	delPointer();
	_pointerFrame = pointerFrame;
	_pickUp = pickup;
	_pointerMode = 0;
}

uint8 DreamBase::findNextColon(const uint8 **string) {
	uint8 c;
	do {
		c = **string;
		++(*string);
	} while ((c != 0) && (c != ':'));
	return c;
}

void DreamBase::enterSymbol() {
	_manIsOffScreen = 1;
	getRidOfReels();
	loadIntoTemp("DREAMWEB.G12"); // symbol graphics
	_symbolTopX = 24;
	_symbolTopDir = 0;
	_symbolBotX = 24;
	_symbolBotDir = 0;
	redrawMainScrn();
	showSymbol();
	underTextLine();
	workToScreenM();
	_getBack = 0;
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
	} while ((_getBack == 0) && !_quitRequested);
	if ((_symbolBotNum == 3) && (_symbolTopNum == 5)) {
		removeSetObject(43);
		placeSetObject(46);
		turnAnyPathOn(0, _roomNum + 12);
		_manIsOffScreen = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
		playChannel1(13);
	} else {
		removeSetObject(46);
		placeSetObject(43);
		turnAnyPathOff(0, _roomNum + 12);
		_manIsOffScreen = 0;
		redrawMainScrn();
		getRidOfTemp();
		restoreReels();
		workToScreenM();
	}
}

void DreamBase::zoomOnOff() {
	if (data.word(kWatchingtime) != 0 || _pointerMode == 2) {
		blank();
		return;
	}

	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(39);
	}

	if (!(_mouseButton & 1) || (_mouseButton == _oldButton))
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
	uint8 *dst = _mapData;
	for (uint16 y = 0; y < kMaplength; ++y) {
		memcpy(dst, src, kMapwidth);
		dst += kMapwidth;
		src += 132;
	}
}

void DreamBase::mainScreen() {
	_inMapArea = 0;
	if (data.byte(kWatchon) == 1) {
		RectWithCallback<DreamBase> mainList[] = {
			{ 44,70,32,46,&DreamBase::look },
			{ 0,50,0,180,&DreamBase::inventory },
			{ 226,244,10,26,&DreamBase::zoomOnOff },
			{ 226,244,26,40,&DreamBase::saveLoad },
			{ 240,260,100,124,&DreamBase::madmanRun },
			{ 0,320,0,200,&DreamBase::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList);
	} else {
		RectWithCallback<DreamBase> mainList2[] = {
			{ 44,70,32,46,&DreamBase::look },
			{ 0,50,0,180,&DreamBase::inventory },
			{ 226+48,244+48,10,26,&DreamBase::zoomOnOff },
			{ 226+48,244+48,26,40,&DreamBase::saveLoad },
			{ 240,260,100,124,&DreamBase::madmanRun },
			{ 0,320,0,200,&DreamBase::identifyOb },
			{ 0xFFFF,0,0,0,0 }
		};
		checkCoords(mainList2);
	}

	if (_walkAndExam != 0)
		walkAndExamine();
}

void DreamBase::showWatch() {
	if (data.byte(kWatchon)) {
		showFrame(_icons1, 250, 1, 6, 0);
		showTime();
	}
}

void DreamBase::dumpWatch() {
	if (_watchDump != 1)
		return;
	multiDump(256, 21, 40, 12);
	_watchDump = 0;
}

void DreamBase::showTime() {
	if (data.byte(kWatchon) == 0)
		return;

	int seconds = data.byte(kSecondcount);
	int minutes = data.byte(kMinutecount);
	int hours = data.byte(kHourcount);

	showFrame(_charset1, 282+5, 21, 91*3+10 + seconds / 10, 0);
	showFrame(_charset1, 282+9, 21, 91*3+10 + seconds % 10, 0);

	showFrame(_charset1, 270+5, 21, 91*3 + minutes / 10, 0);
	showFrame(_charset1, 270+11, 21, 91*3 + minutes % 10, 0);

	showFrame(_charset1, 256+5, 21, 91*3 + hours / 10, 0);
	showFrame(_charset1, 256+11, 21, 91*3 + hours % 10, 0);

	showFrame(_charset1, 267+5, 21, 91*3+20, 0);
}

void DreamBase::watchCount() {
	if (data.byte(kWatchon) == 0)
		return;
	++_timerCount;
	if (_timerCount == 9) {
		showFrame(_charset1, 268+4, 21, 91*3+21, 0);
		_watchDump = 1;
	} else if (_timerCount == 18) {
		_timerCount = 0;
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
		_watchDump = 1;
	}
}

void DreamBase::roomName() {
	printMessage(88, 18, 53, 240, false);
	uint16 textIndex = _roomNum;
	if (textIndex >= 32)
		textIndex -= 32;
	_lineSpacing = 7;
	uint8 maxWidth = (data.byte(kWatchon) == 1) ? 120 : 160;
	const uint8 *string = (const uint8 *)_roomDesc.getString(textIndex);
	printDirect(string, 88, 25, maxWidth, false);
	_lineSpacing = 10;
	useCharset1();
}

void DreamBase::zoomIcon() {
	if (data.byte(kZoomon) == 0)
		return;
	showFrame(_icons1, kZoomx, kZoomy-1, 8, 0);
}

void DreamBase::loadRoom() {
	_roomLoaded = 1;
	_timeCount = 0;
	_mainTimer = 0;
	_mapOffsetX = 104;
	_mapOffsetY = 38;
	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
	data.byte(kLocation) = _newLocation;
	const Room &room = g_roomData[_newLocation];
	startLoading(room);
	loadRoomsSample();
	switchRyanOn();
	drawFlags();

	uint8 mapXstart, mapYstart;
	uint8 mapXsize, mapYsize;
	getDimension(&mapXstart, &mapYstart, &mapXsize, &mapYsize);
}

void DreamBase::readSetData() {
	loadGraphicsFile(_charset1, "DREAMWEB.C00");
	loadGraphicsFile(_icons1, "DREAMWEB.G00");
	loadGraphicsFile(_icons2, "DREAMWEB.G01");
	loadGraphicsFile(_mainSprites, "DREAMWEB.S00");
	loadTextFile(_puzzleText, "DREAMWEB.T80");
	loadTextFile(_commandText, "DREAMWEB.T84");
	useCharset1();

	// FIXME: Why is this commented out?
	//engine->openFile("DREAMWEB.VOL");
	//uint8 *volumeTab = getSegment(data.word(kSoundbuffer)).ptr(16384, 0);
	//engine->readFromFile(volumeTab, 2048-256);
	//engine->closeFile();
}

void DreamBase::findRoomInLoc() {
	uint8 x = _mapX / 11;
	uint8 y = _mapY / 10;
	uint8 roomNum = y * 6 + x;
	_roomNum = roomNum;
}

void DreamBase::autoLook() {
	if ((_mouseX != _oldX) || (_mouseY != _oldY)) {
		_lookCounter = 1000;
		return;
	}

	--_lookCounter;
	if (_lookCounter)
		return;
	if (data.word(kWatchingtime))
		return;
	doLook();
}

void DreamBase::look() {
	if (data.word(kWatchingtime) || (_pointerMode == 2)) {
		blank();
		return;
	}
	if (_commandType != 241) {
		_commandType = 241;
		commandOnly(25);
	}
	if ((_mouseButton == 1) && (_mouseButton != _oldButton))
		doLook();
}

void DreamBase::doLook() {
	createPanel();
	showIcon();
	underTextLine();
	workToScreenM();
	_commandType = 255;
	dumpTextLine();
	uint8 index = _roomNum & 31;
	const uint8 *string = (const uint8 *)_roomDesc.getString(index);
	findNextColon(&string);
	uint16 x;
	if (_realLocation < 50)
		x = 66;
	else
		x = 40;
	if (printSlow(string, x, 80, 241, true) != 1)
		hangOnP(400);

	_pointerMode = 0;
	_commandType = 0;
	redrawMainScrn();
	workToScreenM();
}

void DreamBase::useCharset1() {
	_currentCharset = &_charset1;
}

void DreamBase::useTempCharset() {
	_currentCharset = &_tempCharset;
}

void DreamBase::getRidOfTemp() {
	_tempGraphics.clear();
}

void DreamBase::getRidOfTempText() {
	_textFile1.clear();
}

void DreamBase::getRidOfTemp2() {
	_tempGraphics2.clear();
}

void DreamBase::getRidOfTemp3() {
	_tempGraphics3.clear();
}

void DreamBase::getRidOfTempCharset() {
	_tempCharset.clear();
}

void DreamBase::getRidOfAll() {
	delete[] _backdropBlocks;
	_backdropBlocks = 0;

	_setFrames.clear();
	_reel1.clear();
	_reel2.clear();
	_reel3.clear();
	delete[] _reelList;
	_reelList = 0;
	_personText.clear();
	_setDesc.clear();
	_blockDesc.clear();
	_roomDesc.clear();
	_freeFrames.clear();
	_freeDesc.clear();
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

	assert(len[0] >= 192);
	_backdropBlocks = new uint8[len[0] - 192];
	engine->readFromFile((uint8 *)_backdropFlags, 192);
	engine->readFromFile(_backdropBlocks, len[0] - 192);

	clearAndLoad(workspace(), 0, len[1], 132*66); // 132*66 = maplen
	sortOutMap();

	loadGraphicsSegment(_setFrames, len[2]);
	if (!skipDat)
		clearAndLoad((uint8 *)_setDat, 255, len[3], kSetdatlen);
	else
		engine->skipBytes(len[3]);
	// NB: The skipDat version of this function as called by restoreall
	// had a 'call bloc' instead of 'call loadseg' for reel1,
	// but 'bloc' was not defined.
	loadGraphicsSegment(_reel1, len[4]);
	loadGraphicsSegment(_reel2, len[5]);
	loadGraphicsSegment(_reel3, len[6]);

	// segment 7 consists of 36*38 pathNodes followed by 'reelList'
	engine->readFromFile((uint8 *)_pathData, 36*sizeof(RoomPaths));
	unsigned int reelLen = len[7] - 36*sizeof(RoomPaths);
	unsigned int reelCount = (reelLen + sizeof(Reel) - 1) / sizeof(Reel);
	delete[] _reelList;
	_reelList = new Reel[reelCount];
	engine->readFromFile((uint8 *)_reelList, reelLen);

	// segment 8 consists of 12 personFrames followed by a TextFile
	engine->readFromFile((uint8 *)_personFramesLE, 24);
	loadTextSegment(_personText, len[8] - 24);

	loadTextSegment(_setDesc, len[9]);
	loadTextSegment(_blockDesc, len[10]);
	loadTextSegment(_roomDesc, len[11]);
	loadGraphicsSegment(_freeFrames, len[12]);
	if (!skipDat)
		clearAndLoad((uint8 *)_freeDat, 255, len[13], kFreedatlen);
	else
		engine->skipBytes(len[13]);
	loadTextSegment(_freeDesc, len[14]);

	engine->closeFile();
}

void DreamBase::restoreAll() {
	const Room &room = g_roomData[data.byte(kLocation)];
	loadRoomData(room, true);
	setAllChanges();
}

void DreamBase::restoreReels() {
	if (_roomLoaded == 0)
		return;

	const Room &room = g_roomData[_realLocation];

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
	loadGraphicsSegment(_reel1, len[4]);
	loadGraphicsSegment(_reel2, len[5]);
	loadGraphicsSegment(_reel3, len[6]);

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
	_commandType = 255;
	if (_folderPage) {
		useTempCharset();
		createPanel2();
		showFrame(_tempGraphics, 0, 0, 0, 0);
		showFrame(_tempGraphics, 143, 0, 1, 0);
		showFrame(_tempGraphics, 0, 92, 2, 0);
		showFrame(_tempGraphics, 143, 92, 3, 0);
		folderExit();
		if (_folderPage != 1)
			showLeftPage();
		if (_folderPage != 12)
			showRightPage();
		useCharset1();
		underTextLine();
	} else {
		createPanel2();
		showFrame(_tempGraphics3, 143-28, 0, 0, 0);
		showFrame(_tempGraphics3, 143-28, 92, 1, 0);
		folderExit();
		underTextLine();
	}
}

void DreamBase::showLeftPage() {
	showFrame(_tempGraphics2, 0, 12, 3, 0);
	uint16 y = 12+5;
	for (size_t i = 0; i < 9; ++i) {
		showFrame(_tempGraphics2, 0, y, 4, 0);
		y += 16;
	}
	showFrame(_tempGraphics2, 0, y, 5, 0);
	_lineSpacing = 8;
	_charShift = 91;
	_kerning = 1;
	uint8 pageIndex = _folderPage - 2;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 2, &y, 140, false);
			y += _lineSpacing;
		} while (lastChar != '\0');
	}
	_kerning = 0;
	_charShift = 0;
	_lineSpacing = 10;
	uint8 *bufferToSwap = workspace() + (48*320)+2;
	for (size_t i = 0; i < 120; ++i) {
		for (size_t j = 0; j < 65; ++j) {
			SWAP(bufferToSwap[j], bufferToSwap[130 - j]);
		}
		bufferToSwap += 320;
	}
}

void DreamBase::showRightPage() {
	showFrame(_tempGraphics2, 143, 12, 0, 0);
	uint16 y = 12+37;
	for (size_t i = 0; i < 7; ++i) {
		showFrame(_tempGraphics2, 143, y, 1, 0);
		y += 16;
	}

	showFrame(_tempGraphics2, 143, y, 2, 0);
	_lineSpacing = 8;
	_kerning = 1;
	uint8 pageIndex = _folderPage - 1;
	const uint8 *string = getTextInFile1(pageIndex * 2);
	y = 48;
	for (size_t i = 0; i < 2; ++i) {
		uint8 lastChar;
		do {
			lastChar = printDirect(&string, 152, &y, 140, false);
			y += _lineSpacing;
		} while (lastChar != '\0');
	}
	_kerning = 0;
	_lineSpacing = 10;
}

void DreamBase::showExit() {
	showFrame(_icons1, 274, 154, 11, 0);
}

void DreamBase::showMan() {
	showFrame(_icons1, 0, 0, 0, 0);
	showFrame(_icons1, 0, 114, 1, 0);
	if (data.byte(kShadeson))
		showFrame(_icons1, 28, 25, 2, 0);
}

void DreamBase::panelIcons1() {
	uint16 x;
	if (data.byte(kWatchon) != 1)
		x = 48;
	else
		x = 0;
	showFrame(_icons2, 204 + x, 4, 2, 0);
	if (data.byte(kZoomon) != 1)
		showFrame(_icons1, 228 + x, 8, 5, 0);
	showWatch();
}

void DreamBase::examIcon() {
	showFrame(_icons2, 254, 5, 3, 0);
}

const uint8 *DreamBase::getTextInFile1(uint16 index) {
	return (const uint8 *)_textFile1.getString(index);
}

void DreamBase::checkFolderCoords() {
	RectWithCallback<DreamBase> folderList[] = {
		{ 280,320,160,200, &DreamBase::quitKey },
		{ 143,300,6,194, &DreamBase::nextFolder },
		{ 0,143,6,194, &DreamBase::lastFolder },
		{ 0,320,0,200, &DreamBase::blank },
		{ 0xFFFF,0,0,0, 0 }
	};
	checkCoords(folderList);
}

void DreamBase::nextFolder() {
	if (_folderPage == 12) {
		blank();
		return;
	}
	if (_commandType != 201) {
		_commandType = 201;
		commandOnly(16);
	}
	if ((_mouseButton == 1) && (_mouseButton != _oldButton)) {
		++_folderPage;
		folderHints();
		delPointer();
		showFolder();
		_mouseButton = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamBase::lastFolder() {
	if (_folderPage == 0) {
		blank();
		return;
	}
	if (_commandType != 202) {
		_commandType = 202;
		commandOnly(17);
	}

	if ((_mouseButton == 1) && (_mouseButton != _oldButton)) {
		--_folderPage;
		delPointer();
		showFolder();
		_mouseButton = 0;
		checkFolderCoords();
		workToScreenM();
	}
}

void DreamBase::folderHints() {
	if (_folderPage == 5) {
		if ((data.byte(kAidedead) != 1) && (getLocation(13) != 1)) {
			setLocation(13);
			showFolder();
			const uint8 *string = getTextInFile1(30);
			printDirect(string, 0, 86, 141, true);
			workToScreenM();
			hangOnP(200);
		}
	} else if (_folderPage == 9) {
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
	showFrame(_tempGraphics2, 296, 178, 6, 0);
}

void DreamBase::loadTravelText() {
	loadTextFile(_travelText, "DREAMWEB.T81"); // location descs
}

void DreamBase::loadTempText(const char *fileName) {
	loadTextFile(_textFile1, fileName);
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
	_exFrames.clear();
	_exFrames._data = new uint8[kExframeslen];
	_exText.clear();
	_exText._text = new char[kExtextlen];
}

void DreamBase::workToScreenM() {
	animPointer();
	readMouse();
	showPointer();
	vSync();
	workToScreen();
	delPointer();
}

void DreamBase::loadMenu() {
	loadIntoTemp("DREAMWEB.S02"); // sprite name 3
	loadIntoTemp2("DREAMWEB.G07"); // mon. graphics 2
}

void DreamBase::showMenu() {
	++_menuCount;
	if (_menuCount == 37*2)
		_menuCount = 0;
	showFrame(_tempGraphics, kMenux, kMenuy, _menuCount / 2, 0);
}

void DreamBase::dumpMenu() {
	multiDump(kMenux, kMenuy, 48, 48);
}

void DreamBase::useMenu() {
	getRidOfReels();
	loadMenu();
	createPanel();
	showPanel();
	showIcon();
	data.byte(kNewobs) = 0;
	drawFloor();
	printSprites();
	showFrame(_tempGraphics2, kMenux-48, kMenuy-4, 4, 0);
	getUnderMenu();
	showFrame(_tempGraphics2, kMenux+54, kMenuy+72, 5, 0);
	workToScreenM();
	_getBack = 0;
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
	} while ((_getBack != 1) && !_quitRequested);
	_manIsOffScreen = 0;
	redrawMainScrn();
	getRidOfTemp();
	getRidOfTemp2();
	restoreReels();
	workToScreenM();
}

void DreamBase::atmospheres() {

	const Atmosphere *a = &g_atmosphereList[0];

	for (; a->_location != 255; ++a) {
		if (a->_location != _realLocation)
			continue;
		if (a->_mapX != _mapX || a->_mapY != _mapY)
			continue;
		if (a->_sound != _channel0Playing) {

			if (data.byte(kLocation) == 45 && data.word(kReeltowatch) == 45)
				continue; // "web"

			playChannel0(a->_sound, a->_repeat);

			// NB: The asm here reads
			//	cmp reallocation,2
			//  cmp mapy,0
			//  jz fullvol
			//  jnz notlouisvol
			//  I'm interpreting this as if the cmp reallocation is below the jz

			if (_mapY == 0) {
				_volume = 0; // "fullvol"
				return;
			}

			if (_realLocation == 2 && _mapX == 22 && _mapY == 10)
				_volume = 5; // "louisvol"

			if (isCD() && _realLocation == 14) {
				if (_mapX == 33) {
					_volume = 0; // "ismad2"
					return;
				}

				if (_mapX == 22) {
					_volume = 5;
					return;
				}

			}
		}

		if (_realLocation == 2) {
			if (_mapX == 22) {
				_volume = 5; // "louisvol"
				return;
			}

			if (_mapX == 11) {
				_volume = 0; // "fullvol"
				return;
			}
		}
		return;
	}

	cancelCh0();
}

uint8 DreamBase::nextSymbol(uint8 symbol) {
	uint8 result = symbol + 1;
	if (result == 6)
		return 0;
	if (result == 12)
		return 6;
	return result;
}

void DreamBase::showSymbol() {
	showFrame(_tempGraphics, kSymbolx, kSymboly, 12, 0);

	showFrame(_tempGraphics, _symbolTopX + kSymbolx-44, kSymboly+20, _symbolTopNum, 32);
	uint8 nextTopSymbol = nextSymbol(_symbolTopNum);
	showFrame(_tempGraphics, _symbolTopX + kSymbolx+5, kSymboly+20, nextTopSymbol, 32);
	uint8 nextNextTopSymbol = nextSymbol(nextTopSymbol);
	showFrame(_tempGraphics, _symbolTopX + kSymbolx+54, kSymboly+20, nextNextTopSymbol, 32);

	showFrame(_tempGraphics, _symbolBotX + kSymbolx-44, kSymboly+49, 6 + _symbolBotNum, 32);
	uint8 nextBotSymbol = nextSymbol(_symbolBotNum);
	showFrame(_tempGraphics, _symbolBotX + kSymbolx+5, kSymboly+49, 6 + nextBotSymbol, 32);
	uint8 nextNextBotSymbol = nextSymbol(nextBotSymbol);
	showFrame(_tempGraphics, _symbolBotX + kSymbolx+54, kSymboly+49, 6 + nextNextBotSymbol, 32);
}

void DreamBase::readKey() {
	uint16 bufOut = _bufferOut;

	if (bufOut == _bufferIn) {
		// empty buffer
		_currentKey = 0;
		return;
	}

	bufOut = (bufOut + 1) & 15; // The buffer has size 16
	_currentKey = g_keyBuffer[bufOut];
	_bufferOut = bufOut;
}

void DreamBase::setTopLeft() {
	if (_symbolTopDir != 0) {
		blank();
		return;
	}

	if (_commandType != 210) {
		_commandType = 210;
		commandOnly(19);
	}

	if (_mouseButton != 0)
		_symbolTopDir = 0xFF;
}

void DreamBase::setTopRight() {
	if (_symbolTopDir != 0) {
		blank();
		return;
	}

	if (_commandType != 211) {
		_commandType = 211;
		commandOnly(20);
	}

	if (_mouseButton != 0)
		_symbolTopDir = 1;
}

void DreamBase::setBotLeft() {
	if (_symbolBotDir != 0) {
		blank();
		return;
	}

	if (_commandType != 212) {
		_commandType = 212;
		commandOnly(21);
	}

	if (_mouseButton != 0)
		_symbolBotDir = 0xFF;
}

void DreamBase::setBotRight() {
	if (_symbolBotDir != 0) {
		blank();
		return;
	}

	if (_commandType != 213) {
		_commandType = 213;
		commandOnly(22);
	}

	if (_mouseButton != 0)
		_symbolBotDir = 1;
}

void DreamBase::newGame() {
	if (_commandType != 251) {
		_commandType = 251;
		commandOnly(47);
	}

	if (_mouseButton == 1)
		_getBack = 3;
}

void DreamBase::pickupOb(uint8 command, uint8 pos) {
	_lastInvPos = pos;
	_objectType = kFreeObjectType;
	_itemFrame = command;
	_command = command;
	//uint8 dummy;
	//getAnyAd(&dummy, &dummy);	// was in the original source, seems useless here
	transferToEx(command);
}

void DreamBase::initialInv() {
	if (_realLocation != 24)
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

void DreamBase::walkIntoRoom() {
	if (data.byte(kLocation) == 14 && _mapX == 22) {
		_destination = 1;
		_finalDest = 1;
		autoSetWalk();
	}
}

void DreamBase::afterIntroRoom() {
	if (_nowInNewRoom == 0)
		return; // notnewintro

	clearWork();
	findRoomInLoc();
	data.byte(kNewobs) = 1;
	drawFloor();
	reelsOnScreen();
	spriteUpdate();
	printSprites();
	workToScreen();
	_nowInNewRoom = 0;
}

void DreamBase::redrawMainScrn() {
	_timeCount = 0;
	createPanel();
	data.byte(kNewobs) = 0;
	drawFloor();
	printSprites();
	reelsOnScreen();
	showIcon();
	getUnderZoom();
	underTextLine();
	readMouse();
	_commandType = 255;
}

void DreamBase::blank() {
	if (_commandType != 199) {
		_commandType = 199;
		commandOnly(0);
	}
}

void DreamBase::allPointer() {
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
	_commandType = 255;
	animPointer();
	workToScreenM();
	_commandType = 200;
	_manIsOffScreen = 0;
}

void DreamBase::openInv() {
	_invOpen = 1;
	printMessage(80, 58 - 10, 61, 240, (240 & 1));
	fillRyan();
	_commandType = 255;
}

void DreamBase::obsThatDoThings() {
	if (!compare(_command, _objectType, "MEMB"))
		return; // notlouiscard

	if (getLocation(4) != 1) {
		setLocation(4);
		lookAtCard();
	}
}

void DreamBase::describeOb() {
	const uint8 *obText = getObTextStart();
	uint16 y = 92;
	if (_foreignRelease && _objectType == kSetObjectType1)
		y = 82;
	_charShift = 91 + 91;
	printDirect(&obText, 33, &y, 241, 241 & 1);
	_charShift = 0;
	y = 104;
	if (_foreignRelease && _objectType == kSetObjectType1)
		y = 94;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	obsThatDoThings();

	// Additional text
	if (compare(_command, _objectType, "CUPE")) {
		// Empty cup
		const uint8 *string = (const uint8 *)_puzzleText.getString(40);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	} else if (compare(_command, _objectType, "CUPF")) {
		// Full cup
		const uint8 *string = (const uint8 *)_puzzleText.getString(39);
		printDirect(string, 36, y + 10, 241, 241 & 1);
	}
}

void DreamBase::delEverything() {
	if (_mapYSize + _mapOffsetY < 182) {
		mapToPanel();
	} else {
		// Big room
		_mapYSize -= 8;
		mapToPanel();
		_mapYSize += 8;
	}
}

void DreamBase::errorMessage1() {
	delPointer();
	printMessage(76, 21, 58, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamBase::errorMessage2() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 59, 240, (240 & 1));
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	readMouse();
	useOpened();
	showPointer();
	workToScreen();
	delPointer();
}

void DreamBase::errorMessage3() {
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
	workToScreen();
	delPointer();
}

void DreamBase::reExFromOpen() {

}

void DreamBase::putBackObStuff() {
	createPanel();
	showPanel();
	showMan();
	obIcons();
	showExit();
	obPicture();
	describeOb();
	underTextLine();
	_commandType = 255;
	readMouse();
	showPointer();
	workToScreen();
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
	if (_commandType != 249) {
		_commandType = 249;
		commandOnly(32);
	}

	if (!(_mouseButton & 1))
		return;

	createPanel();
	showPanel();
	showMan();
	showExit();
	examIcon();
	_pickUp = 0;
	_invOpen = 2;
	openInv();
	workToScreenM();
}

void DreamBase::middlePanel() {
}

void DreamBase::showDiary() {
	showFrame(_tempGraphics, kDiaryx, kDiaryy + 37, 1, 0);
	showFrame(_tempGraphics, kDiaryx + 176, kDiaryy + 108, 2, 0);
}

void DreamBase::underTextLine() {
	if (_foreignRelease)
		multiGet(_textUnder, _textAddressX, _textAddressY - 3, kUnderTextSizeX_f, kUnderTextSizeY_f);
	else
		multiGet(_textUnder, _textAddressX, _textAddressY, kUnderTextSizeX, kUnderTextSizeY);
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
	if (_realLocation == 26 && reelPointer == 104)
		data.byte(kShakecounter) = 0xFF;
}

void DreamBase::watchReel() {
	if (data.word(kReeltowatch) != 0xFFFF) {
		if (_mansPath != _finalDest)
			return; // Wait until stopped walking
		if (_turnToFace != _facing)
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
			_destination = data.byte(kDestafterhold);
			_finalDest = data.byte(kDestafterhold);
			autoSetWalk();
			return;
		}
	}

	uint16 reelPointer = data.word(kReeltohold);
	plotReel(reelPointer);
}

void DreamBase::afterNewRoom() {
	if (_nowInNewRoom == 0)
		return; // notnew

	_timeCount = 0;
	createPanel();
	_commandType = 0;
	findRoomInLoc();
	if (data.byte(kRyanon) != 1) {
		_mansPath = findPathOfPoint(_ryanX + 12, _ryanY + 12);
		findXYFromPath();
		_resetManXY = 1;
	}
	data.byte(kNewobs) = 1;
	drawFloor();
	_lookCounter = 160;
	_nowInNewRoom = 0;
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

void DreamBase::madmanRun() {
	if (data.byte(kLocation)    != 14 ||
		_mapX        != 22 ||
		_pointerMode !=  2 ||
		data.byte(kMadmanflag)  !=  0) {
		identifyOb();
		return;
	}

	if (_commandType != 211) {
		_commandType = 211;
		commandOnly(52);
	}

	if (_mouseButton == 1 &&
		_mouseButton != _oldButton)
		data.byte(kLastweapon) = 8;
}


void DreamBase::decide() {
	setMode();
	loadPalFromIFF();
	clearPalette();
	_pointerMode = 0;
	data.word(kWatchingtime) = 0;
	_pointerFrame = 0;
	_textAddressX = 70;
	_textAddressY = 182 - 8;
	_textLen = 181;
	_manIsOffScreen = 1;
	loadSaveBox();
	showDecisions();
	workToScreen();
	fadeScreenUp();
	_getBack = 0;

	RectWithCallback<DreamBase> decideList[] = {
		{ kOpsx+69,kOpsx+124,kOpsy+30,kOpsy+76,&DreamBase::newGame },
		{ kOpsx+20,kOpsx+87,kOpsy+10,kOpsy+59,&DreamBase::DOSReturn },
		{ kOpsx+123,kOpsx+190,kOpsy+10,kOpsy+59,&DreamBase::loadOld },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	do {
		if (_quitRequested)
			return;

		readMouse();
		showPointer();
		vSync();
		dumpPointer();
		dumpTextLine();
		delPointer();
		checkCoords(decideList);
	} while (!_getBack);

	if (_getBack != 4)
		getRidOfTemp();	// room not loaded

	_textAddressX = 13;
	_textAddressY = 182;
	_textLen = 240;
}

void DreamBase::showGun() {
	_addToRed = 0;
	_addToGreen = 0;
	_addToBlue = 0;
	palToStartPal();
	palToEndPal();
	greyscaleSum();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(130);
	endPalToStart();
	clearEndPal();
	_fadeDirection = 1;
	_fadeCount = 63;
	_colourPos = 0;
	_numToFade = 128;
	hangOn(200);
	_roomsSample = 34;
	loadRoomsSample();
	_volume = 0;
	loadIntoTemp("DREAMWEB.G13");
	createPanel2();
	showFrame(_tempGraphics, 100, 4, 0, 0);
	showFrame(_tempGraphics, 158, 106, 1, 0);
	workToScreen();
	getRidOfTemp();
	fadeScreenUp();
	hangOn(160);
	playChannel0(12, 0);
	loadTempText("DREAMWEB.T83");
	rollEndCreditsGameLost();
	getRidOfTempText();
}

void DreamBase::diaryKeyP() {
	if (_commandType != 214) {
		_commandType = 214;
		commandOnly(23);
	}

	if (!_mouseButton ||
		_oldButton == _mouseButton ||
		_pressCount)
		return; // notkeyp

	playChannel1(16);
	_pressCount = 12;
	_pressed = 'P';
	_diaryPage--;

	if (_diaryPage == 0xFF)
		_diaryPage = 11;
}

void DreamBase::diaryKeyN() {
	if (_commandType != 213) {
		_commandType = 213;
		commandOnly(23);
	}

	if (!_mouseButton ||
		_oldButton == _mouseButton ||
		_pressCount)
		return; // notkeyn

	playChannel1(16);
	_pressCount = 12;
	_pressed = 'N';
	_diaryPage++;

	if (_diaryPage == 12)
		_diaryPage = 0;
}

void DreamBase::dropError() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 56, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	_commandType = 255;
	workToScreenM();
}

void DreamBase::cantDrop() {
	_commandType = 255;
	delPointer();
	printMessage(76, 21, 24, 240, 240 & 1);
	workToScreenM();
	hangOnP(50);
	showPanel();
	showMan();
	examIcon();
	_commandType = 255;
	workToScreenM();
}

void DreamBase::getBack1() {
	if (_pickUp != 0) {
		blank();
		return;
	}


	if (_commandType != 202) {
		_commandType = 202;
		commandOnly(26);
	}

	if (_mouseButton == _oldButton)
		return;

	if (_mouseButton & 1) {
		// Get back
		_getBack = 1;
		_pickUp = 0;
	}
}

void DreamBase::autoAppear() {
	if (data.byte(kLocation) == 32) {
		// In alley
		resetLocation(5);
		setLocation(10);
		_destPos = 10;
		return;
	}

	if (_realLocation == 24) {
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
		if (_realLocation == 25) {
			// Sart roof
			data.byte(kNewsitem) = 3;
			resetLocation(6);
			setLocation(11);
			_destPos = 11;
		} else {
			if (_realLocation == 2 && data.byte(kRockstardead) != 0)
				placeSetObject(23);
		}
	}
}

void DreamBase::quitKey() {
	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(4);
	}

	if (_mouseButton != _oldButton && (_mouseButton & 1))
		_getBack = 1;
}

void DreamBase::setupTimedUse(uint16 textIndex, uint16 countToTimed, uint16 timeCount, byte x, byte y) {
	if (_timeCount != 0)
		return; // can't setup

	_timedY = y;
	_timedX = x;
	_countToTimed = countToTimed;
	_timeCount = timeCount + countToTimed;
	_timedString = _puzzleText.getString(textIndex);
	debug(1, "setupTimedUse: %d => '%s'", textIndex, _timedString);
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

void DreamBase::entryAnims() {
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
		_symbolTopNum = 2;
		_symbolBotNum = 1;
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
		if (_realLocation == 46 && data.byte(kSartaindead) == 1) {	// Crystal
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
			turnAnyPathOn(2, _roomNum - 1);
		}
	}
}

void DreamBase::updateSymbolTop() {
	if (!_symbolTopDir)
		return; // topfinished

	if (_symbolTopDir == (byte)-1) {
		// Backward
		_symbolTopX--;
		if (_symbolTopX != (byte)-1) {
			// Not wrapping
			if (_symbolTopX != 24)
				return; // topfinished
			_symbolTopDir = 0;
		} else {
			_symbolTopX = 48;
			_symbolTopNum++;
			if (_symbolTopNum != 6)
				return; // topfinished
			_symbolTopNum = 0;
		}
	} else {
		// Forward
		_symbolTopX++;
		if (_symbolTopX != 49) {
			// Not wrapping
			if (_symbolTopX != 24)
				return; // topfinished
			_symbolTopDir = 0;
		} else {
			_symbolTopX = 0;
			_symbolTopNum--;
			if (_symbolTopNum != (byte)-1)
				return; // topfinished
			_symbolTopNum = 5;
		}
	}
}

void DreamBase::updateSymbolBot() {
	if (!_symbolBotDir)
		return; // botfinished

	if (_symbolBotDir == (byte)-1) {
		// Backward
		_symbolBotX--;
		if (_symbolBotX != (byte)-1) {
			// Not wrapping
			if (_symbolBotX != 24)
				return; // botfinished
			_symbolBotDir = 0;
		} else {
			_symbolBotX = 48;
			_symbolBotNum++;
			if (_symbolBotNum != 6)
				return; // botfinished
			_symbolBotNum = 0;
		}
	} else {
		// Forward
		_symbolBotX++;
		if (_symbolBotX != 49) {
			// Not wrapping
			if (_symbolBotX != 24)
				return; // botfinished
			_symbolBotDir = 0;
		} else {
			_symbolBotX = 0;
			_symbolBotNum--;
			if (_symbolBotNum != (byte)-1)
				return; // botfinished
			_symbolBotNum = 5;
		}
	}
}

void DreamBase::showDiaryPage() {
	showFrame(_tempGraphics, kDiaryx, kDiaryy, 0, 0);
	_kerning = 1;
	useTempCharset();
	_charShift = 91+91;
	const uint8 *string = getTextInFile1(_diaryPage);
	uint16 y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	y = kDiaryy + 16;
	printDirect(&string, kDiaryx + 129, &y, 240, 240 & 1);
	y = kDiaryy + 23;
	printDirect(&string, kDiaryx + 48, &y, 240, 240 & 1);
	_kerning = 0;
	_charShift = 0;
	useCharset1();
}

void DreamBase::dumpDiaryKeys() {
	if (_pressCount == 1) {
		if (data.byte(kSartaindead) != 1 && _diaryPage == 5 && getLocation(6) != 1) {
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

void DreamBase::lookAtCard() {
	_manIsOffScreen = 1;
	getRidOfReels();
	loadKeypad();
	createPanel2();
	showFrame(_tempGraphics, 160, 80, 42, 128);
	const uint8 *obText = getObTextStart();
	findNextColon(&obText);
	findNextColon(&obText);
	findNextColon(&obText);
	uint16 y = 124;
	printDirect(&obText, 36, &y, 241, 241 & 1);
	workToScreenM();
	hangOnW(280);
	createPanel2();
	showFrame(_tempGraphics, 160, 80, 42, 128);
	printDirect(obText, 36, 130, 241, 241 & 1);
	workToScreenM();
	hangOnW(200);
	_manIsOffScreen = 0;
	getRidOfTemp();
	restoreReels();
	putBackObStuff();
}

void DreamBase::clearBuffers() {
	memcpy(_initialVars, data.ptr(kStartvars, kLengthofvars), kLengthofvars);

	clearChanges();
}

void DreamBase::clearChanges() {
	memset(_listOfChanges, 0xFF, 4*kNumchanges);

	setupInitialReelRoutines();

	memcpy(data.ptr(kStartvars, kLengthofvars), _initialVars, kLengthofvars);

	data.word(kExframepos) = 0;
	data.word(kExtextpos) = 0;

	memset(_exFrames._frames, 0xFF, 2080);
	memset(_exFrames._data, 0xFF, kExframeslen);
	memset(_exData, 0xFF, sizeof(DynObject) * kNumexobjects);
	memset(_exText._offsetsLE, 0xFF, 2*(kNumexobjects+2));
	memset(_exText._text, 0xFF, kExtextlen);

	const uint8 initialRoomsCanGo[] = { 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	memcpy(_roomsCanGo, initialRoomsCanGo, 16);
}

void DreamBase::showDiaryKeys() {
	if (!_pressCount)
		return; // nokeyatall

	_pressCount--;

	if (!_pressCount)
		return; // nokeyatall

	if (_pressed == 'N') {
		byte frame = (_pressCount == 1) ? 3 : 4;
		showFrame(_tempGraphics, kDiaryx + 94, kDiaryy + 97, frame, 0);
	} else {
		byte frame = (_pressCount == 1) ? 5 : 6;
		showFrame(_tempGraphics, kDiaryx + 151, kDiaryy + 71, frame, 0);
	}

	if (_pressCount == 1)
		showDiaryPage();
}

void DreamBase::edensFlatReminders() {
	if (_realLocation != 24 || _mapX != 44)
		return; // not in Eden's lift

	if (data.byte(kProgresspoints))
		return; // not the first time in Eden's apartment

	uint16 exObjextIndex = findExObject("CSHR");
	if (!isRyanHolding("DKEY") || exObjextIndex == kNumexobjects) {
		setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	}

	DynObject *object = getExAd(exObjextIndex);

	if (object->mapad[0] != 4) {
		setupTimedUse(50, 48, 8, 54, 70);	// forgot something
		return;
	} else if (object->mapad[1] != 255) {
		if (!compare(object->mapad[1], object->mapad[0], "PURS")) {
			setupTimedUse(50, 48, 8, 54, 70);	// forgot something
			return;
		}
	}

	data.byte(kProgresspoints)++;	// got card
}

void DreamBase::incRyanPage() {
	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(31);
	}

	if (_mouseButton == _oldButton || !(_mouseButton & 1))
		return;

	data.byte(kRyanpage) = (_mouseX - (kInventx + 167)) / 18;

	delPointer();
	fillRyan();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();

}

void DreamBase::emergencyPurge() {
	while (true) {
		if (data.word(kExframepos) + 4000 < kExframeslen) {
			// Not near frame end
			if (data.word(kExtextpos) + 400 < kExtextlen)
				return;	// notneartextend
		}

		purgeAnItem();
	}
}

void DreamBase::purgeAnItem() {
	const DynObject *extraObjects = _exData;

	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (extraObjects[i].mapad[0] && extraObjects[i].id[0] == 255 &&
			extraObjects[i].initialLocation != _realLocation) {
			deleteExObject(i);
			return;
		}
	}

	for (size_t i = 0; i < kNumexobjects; ++i) {
		if (extraObjects[i].mapad[0] && extraObjects[i].id[0] == 255) {
			deleteExObject(i);
			return;
		}
	}
}

} // End of namespace DreamGen
