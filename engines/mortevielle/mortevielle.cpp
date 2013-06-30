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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/saveload.h"
#include "mortevielle/outtext.h"

namespace Mortevielle {

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc), _randomSource("mortevielle"),
		_soundManager(_mixer) {
	g_vm = this;
	_debugger.setParent(this);
	_dialogManager.setParent(this);
	_screenSurface.setParent(this);
	_mouse.setParent(this);
	_text.setParent(this);
	_soundManager.setParent(this);
	_speechManager.setParent(this);

	_lastGameFrame = 0;
	_mouseClick = false;
	_inMainGameLoop = false;
	_quitGame = false;

	_roomPresenceLuc = false;
	_roomPresenceIda = false;
	_purpleRoomPresenceLeo = false;
	_roomPresenceGuy = false;
	_roomPresenceEva = false;
	_roomPresenceMax = false;
	_roomPresenceBob = false;
	_roomPresencePat = false;
	_toiletsPresenceBobMax = false;
	_bathRoomPresenceBobMax = false;
	_room9PresenceLeo = false;

	_soundOff = false;
	_largestClearScreen = false;
	_hiddenHero = false;
	_heroSearching = false;
	_keyPressedEsc = false;
	_reloadCFIEC = false;

	_blo = false;
	_col = false;
	_syn = false;
	_obpart = false;
	_destinationOk = false;
	_anyone = false;
	_uptodatePresence = false;

	_textColor = 0;
	_currGraphicalDevice = -1;
	_newGraphicalDevice = -1;
	_place = -1;

	_x26KeyCount = -1;
	_caff = -1;
	_day = 0;

	memset(_mem, 0, sizeof(_mem));
}

MortevielleEngine::~MortevielleEngine() {
}

/**
 * Specifies whether the engine supports given features
 */
bool MortevielleEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

/**
 * Return true if a game can currently be loaded
 */
bool MortevielleEngine::canLoadGameStateCurrently() {
	// Saving is only allowed in the main game event loop
	return _inMainGameLoop;
}

/**
 * Return true if a game can currently be saved
 */
bool MortevielleEngine::canSaveGameStateCurrently() {
	// Loading is only allowed in the main game event loop
	return _inMainGameLoop;
}

/**
 * Load in a savegame at the specified slot number
 */
Common::Error MortevielleEngine::loadGameState(int slot) {
	return _savegameManager.loadGame(slot);
}

/**
 * Save the current game
 */
Common::Error MortevielleEngine::saveGameState(int slot, const Common::String &desc) {
	if (slot == 0)
		return Common::kWritingFailed;

	return _savegameManager.saveGame(slot, desc);
}

/**
 * Initialise the game state
 */
Common::ErrorCode MortevielleEngine::initialise() {
	// Initialise graphics mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, true);

	// Set debug channels
	DebugMan.addDebugChannel(kMortevielleCore, "core", "Core debugging");
	DebugMan.addDebugChannel(kMortevielleGraphics, "graphics", "Graphics debugging");

	// Set up an intermediate screen surface
	_screenSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	// Set the screen mode
	_currGraphicalDevice = MODE_EGA;
	_resolutionScaler = 2;

	_txxFileFl = false;
	// Load texts from TXX files
	loadTexts();

	// Load the mort.dat resource
	Common::ErrorCode result = loadMortDat();
	if (result != Common::kNoError) {
		_screenSurface.free();
		return result;
	}

	// Load some error messages (was previously in chartex())
	_hintPctMessage = getString(580);  // You should have noticed %d hints

	// Set default EGA palette
	_paletteManager.setDefaultPalette();

	// Setup the mouse cursor
	initMouse();

	_currGraphicalDevice = MODE_EGA;
	_newGraphicalDevice = _currGraphicalDevice;
	loadPalette();
	loadCFIPH();
	loadCFIEC();
	decodeNumber(&_cfiecBuffer[161 * 16], (_cfiecBufferSize - (161 * 16)) / 64);
	_x26KeyCount = 1;
	init_nbrepm();
	initMouse();

	loadPlaces();
	_soundOff = false;
	_largestClearScreen = false;

	testKeyboard();
	showConfigScreen();
	_newGraphicalDevice = _currGraphicalDevice;
	testKeyboard();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();

	return Common::kNoError;
}

/**
 * Loads the contents of the Mort.dat data file
 */
Common::ErrorCode MortevielleEngine::loadMortDat() {
	Common::File f;

	// Open the mort.dat file
	if (!f.open(MORT_DAT)) {
		GUIErrorMessage("Could not locate Mort.dat file");
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char fileId[4];
	f.read(fileId, 4);
	if (strncmp(fileId, "MORT", 4) != 0) {
		GUIErrorMessage("The located mort.dat data file is invalid");
		return Common::kReadingFailed;
	}

	// Check the version
	if (f.readByte() < MORT_DAT_REQUIRED_VERSION) {
		GUIErrorMessage("The located mort.dat data file is too a version");
		return Common::kReadingFailed;
	}
	f.readByte();		// Minor version

	// Loop to load resources from the data file
	while (f.pos() < f.size()) {
		// Get the Id and size of the next resource
		char dataType[4];
		int dataSize;
		f.read(dataType, 4);
		dataSize = f.readUint16LE();

		if (!strncmp(dataType, "FONT", 4)) {
			// Font resource
			_screenSurface.readFontData(f, dataSize);
		} else if (!strncmp(dataType, "SSTR", 4)) {
			readStaticStrings(f, dataSize, kStaticStrings);
		} else if ((!strncmp(dataType, "GSTR", 4)) && (!_txxFileFl)) {
			readStaticStrings(f, dataSize, kGameStrings);
		} else {
			// Unknown section
			f.skip(dataSize);
		}
	}

	// Close the file
	f.close();

	assert(_engineStrings.size() > 0);
	return Common::kNoError;
}

/**
 * Read in a static strings block, and if the language matches, load up the static strings
 */
void MortevielleEngine::readStaticStrings(Common::File &f, int dataSize, DataType dataType) {
	// Figure out what language Id is needed
	byte desiredLanguageId;
	switch(getLanguage()) {
	case Common::EN_ANY:
		desiredLanguageId = LANG_ENGLISH;
		break;
	case Common::FR_FRA:
		desiredLanguageId = LANG_FRENCH;
		break;
	case Common::DE_DEU:
		desiredLanguageId = LANG_GERMAN;
		break;
	default:
		warning("Language not supported, switching to English");
		desiredLanguageId = LANG_ENGLISH;
		break;
	}

	// Read in the language
	byte languageId = f.readByte();
	--dataSize;

	// If the language isn't correct, then skip the entire block
	if (languageId != desiredLanguageId) {
		f.skip(dataSize);
		return;
	}

	// Load in each of the strings
	while (dataSize > 0) {
		Common::String s;
		char ch;
		while ((ch = (char)f.readByte()) != '\0')
			s += ch;

		if (dataType == kStaticStrings)
			_engineStrings.push_back(s);
		else if (dataType == kGameStrings)
			_gameStrings.push_back(s);

		dataSize -= s.size() + 1;
	}
	assert(dataSize == 0);
}

/*-------------------------------------------------------------------------*/

Common::Error MortevielleEngine::run() {
	// Initialise the game
	Common::ErrorCode err = initialise();
	if (err != Common::kNoError)
		return err;

	// Check for a savegame
	int loadSlot = 0;
	if (ConfMan.hasKey("save_slot")) {
		int gameToLoad = ConfMan.getInt("save_slot");
		if ((gameToLoad >= 1) && (gameToLoad <= 999))
			loadSlot = gameToLoad;
	}

	if (loadSlot == 0)
		// Show the game introduction
		showIntroduction();

	// Either load the initial game state savegame, or the specified savegame number
	adzon();
	_savegameManager.loadSavegame(loadSlot);

	// Run the main game loop
	mainGame();
	
	// Cleanup (allocated in initialise())
	_screenSurface.free();
	free(_speechManager._cfiphBuffer);
	free(_cfiecBuffer);

	return Common::kNoError;
}

/**
 * Show the game introduction
 */
void MortevielleEngine::showIntroduction() {
	_dialogManager.aff50(false);
	_speechManager._mlec = 0;
	_dialogManager.checkForF8(142, false);
	if (shouldQuit())
		return;

	_dialogManager.ani50();
	_dialogManager.checkForF8(143, true);
	if (shouldQuit())
		return;

	// TODO: Once music (Amiga/Atari ports) is implemented, only use the below delay if music is turned off
	showTitleScreen();
	delay(3000);
	music();
}

/**
 * Main game loop. Handles potentially playing the game multiple times, such as if the player
 * loses, and chooses to start playing the game again.
 */
void MortevielleEngine::mainGame() {
	if (_reloadCFIEC)
		loadCFIEC();

	for (_crep = 1; _crep <= _x26KeyCount; ++_crep)
		decodeNumber(&_cfiecBuffer[161 * 16], (_cfiecBufferSize - (161 * 16)) / 64);

	loadBRUIT5();
	_menu.initMenu(this);

	charToHour();
	initGame();
	hirs();
	drawRightFrame();
	_mouse.showMouse();

	// Loop to play the game
	do {
		playGame();
		if (shouldQuit())
			return;
	} while (!_quitGame);
}

/**
 * This method handles playing a loaded game
 * @remarks	Originally called tjouer
 */
void MortevielleEngine::playGame() {
	gameLoaded();

	// Loop handling actions until the game has to be quit, or show the lose or end sequence
	do {
		handleAction();
		if (shouldQuit())
			return;
	} while (!((_quitGame) || (_endGame) || (_loseGame)));

	if (_endGame)
		endGame();
	else if (_loseGame)
		askRestart();
}

} // End of namespace Mortevielle
