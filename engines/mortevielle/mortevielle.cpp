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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"

#include "mortevielle/dialogs.h"
#include "mortevielle/menu.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/saveload.h"
#include "mortevielle/outtext.h"

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/translation.h"
#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"

namespace Mortevielle {

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const MortevielleGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc), _randomSource("mortevielle") {
	// Set debug channels
	DebugMan.addDebugChannel(kMortevielleCore, "core", "Core debugging");
	DebugMan.addDebugChannel(kMortevielleGraphics, "graphics", "Graphics debugging");

	g_vm = this;
	setDebugger(new Debugger(this));
	_dialogManager = new DialogManager(this);
	_screenSurface = new ScreenSurface(this);
	_mouse = new MouseHandler(this);
	_text = new TextHandler(this);
	_soundManager = new SoundManager(this, _mixer);
	_savegameManager = new SavegameManager(this);
	_menu = new Menu(this);

	_lastGameFrame = 0;
	_mouseClick = false;
	_inMainGameLoop = false;
	_quitGame = false;
	_pauseStartTime = -1;

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
	_juliaRoomPresenceLeo = false;

	_soundOff = false;
	_largestClearScreen = false;
	_hiddenHero = false;
	_heroSearching = false;
	_keyPressedEsc = false;
	_reloadCFIEC = false;

	_outsideOnlyFl = true;
	_col = false;
	_syn = false;
	_obpart = false;
	_destinationOk = false;
	_anyone = false;
	_uptodatePresence = false;

	_textColor = 0;
	_place = -1;

	_x26KeyCount = -1;
	_caff = -1;
	_day = 0;

	_curPict = nullptr;
	_curAnim = nullptr;
	_rightFramePict = nullptr;

	resetCoreVar();

	_maff = 0;
	_crep = 0;

	_minute = 0;
	_curSearchObjId = 0;
	_controlMenu = 0;
	_startTime = 0;
	_endTime = 0;
	_roomDoorId = OWN_ROOM;
	_openObjCount = 0;
	_takeObjCount = 0;
	_num = 0;
	_searchCount = 0;
	_introSpeechPlayed = false;
	_inGameHourDuration = 0;
	_x = 0;
	_y = 0;
	_currentHourCount = 0;
	_currentTime = 0;
	_cfiecBuffer = nullptr;
	_cfiecBufferSize = 0;
	for (int i = 0; i < 601; i++) {
		_dialogHintArray[i]._hintId = 0;
		_dialogHintArray[i]._point = 0;
	}
	_currMenu = OPCODE_NONE;
	_currAction = OPCODE_NONE;
	_menuOpcode = OPCODE_NONE;
	_addFix = 0;
	_currBitIndex = 0;
	_currDay = 0;
	_currHour = 10;
	_currHalfHour = 0;
	_hour = 10;
	_key = 0;
	_manorDistance = 0;
	_numpal = 0;
	_savedBitIndex = 0;
	_endGame = false;
	_loseGame = false;
	_txxFileFl = false;
	_is = 0;
}

MortevielleEngine::~MortevielleEngine() {
	delete _menu;
	delete _savegameManager;
	delete _soundManager;
	delete _text;
	delete _mouse;
	delete _screenSurface;
	delete _dialogManager;

	free(_curPict);
	free(_curAnim);
	free(_rightFramePict);
}

/**
 * Specifies whether the engine supports given features
 */
bool MortevielleEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
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
	return _savegameManager->loadGame(slot);
}

/**
 * Save the current game
 */
Common::Error MortevielleEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	if (slot == 0)
		return Common::kWritingFailed;

	return _savegameManager->saveGame(slot, desc);
}

/**
 * Support method that generates a savegame name
 * @param slot		Slot number
 */
Common::String MortevielleEngine::generateSaveFilename(const Common::String &target, int slot) {
	if (slot == 0)
		// Initial game state loaded when the game starts
		return "sav0.mor";

	return Common::String::format("%s.%03d", target.c_str(), slot);
}

/**
 * Pause the game.
 */
void MortevielleEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (pause) {
		if (_pauseStartTime == -1)
			_pauseStartTime = readclock();
	} else {
		if (_pauseStartTime != -1) {
			int pauseEndTime = readclock();
			_currentTime += (pauseEndTime - _pauseStartTime);
			if (_uptodatePresence)
				_startTime += (pauseEndTime - _pauseStartTime);
		}
		_pauseStartTime = -1;
	}
}

/**
 * Initialize the game state
 */
Common::ErrorCode MortevielleEngine::initialize() {
	// Initialize graphics mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Set up an intermediate screen surface
	_screenSurface->create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	_txxFileFl = false;
	// Load texts from TXX files
	loadTexts();

	// Load the mort.dat resource
	Common::ErrorCode result = loadMortDat();
	if (result != Common::kNoError) {
		_screenSurface->free();
		return result;
	}

	// Load some error messages (was previously in chartex())
	_hintPctMessage = getString(580);  // You should have noticed %d hints

	// Set default EGA palette
	_paletteManager.setDefaultPalette();

	// Setup the mouse cursor
	initMouse();

	loadPalette();
	loadCFIPH();
	loadCFIEC();
	decodeNumber(&_cfiecBuffer[161 * 16], (_cfiecBufferSize - (161 * 16)) / 64);
	_x26KeyCount = 1;
	initMaxAnswer();
	initMouse();

	loadPlaces();
	_soundOff = false;
	_largestClearScreen = false;

	testKeyboard();
	showConfigScreen();
	testKeyboard();
	clearScreen();

	_soundManager->loadNoise();
	_soundManager->loadAmbiantSounds();

	return Common::kNoError;
}

/**
 * Loads the contents of the mort.dat data file
 */
Common::ErrorCode MortevielleEngine::loadMortDat() {
	Common::File f;

	// Open the mort.dat file
	if (!f.open(MORT_DAT)) {
		GUIErrorMessageFormat(_("Unable to locate the '%s' engine data file."), MORT_DAT);
		return Common::kReadingFailed;
	}

	// Validate the data file header
	char fileId[4];
	f.read(fileId, 4);
	if (strncmp(fileId, "MORT", 4) != 0) {
		GUIErrorMessageFormat(_("The '%s' engine data file is corrupt."), MORT_DAT);
		return Common::kReadingFailed;
	}

	// Check the version
	int majVer = f.readByte();
	int minVer = f.readByte();

	if (majVer < MORT_DAT_REQUIRED_VERSION) {
		GUIErrorMessageFormat(
			_("Incorrect version of the '%s' engine data file found. Expected %d.%d but got %d.%d."),
			MORT_DAT, MORT_DAT_REQUIRED_VERSION, 0, majVer, minVer);
		return Common::kReadingFailed;
	}

	// Loop to load resources from the data file
	while (f.pos() < f.size()) {
		// Get the Id and size of the next resource
		char dataType[4];
		int dataSize;
		f.read(dataType, 4);
		dataSize = f.readUint16LE();

		if (!strncmp(dataType, "FONT", 4)) {
			// Font resource
			_screenSurface->readFontData(f, dataSize);
		} else if (!strncmp(dataType, "SSTR", 4)) {
			readStaticStrings(f, dataSize, kStaticStrings);
		} else if ((!strncmp(dataType, "GSTR", 4)) && (!_txxFileFl)) {
			readStaticStrings(f, dataSize, kGameStrings);
		} else if (!strncmp(dataType, "VERB", 4)) {
			_menu->readVerbNums(f, dataSize);
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
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
		break;
	case Common::FR_FRA:
		desiredLanguageId = MORTDAT_LANG_FRENCH;
		break;
	case Common::DE_DEU:
		desiredLanguageId = MORTDAT_LANG_GERMAN;
		break;
	default:
		warning("Language not supported, switching to English");
		desiredLanguageId = MORTDAT_LANG_ENGLISH;
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
	// Initialize the game
	Common::ErrorCode err = initialize();
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
	else {
		_caff = 51;
		_text->taffich();
	}

	// Either load the initial game state savegame, or the specified savegame number
	adzon();
	resetVariables();
	if (loadSlot != 0)
		_savegameManager->loadSavegame(getSaveStateName(loadSlot));

	// Run the main game loop
	mainGame();

	// Cleanup (allocated in initialize())
	_screenSurface->free();
	free(_soundManager->_cfiphBuffer);
	free(_cfiecBuffer);

	return Common::kNoError;
}

/**
 * Show the game introduction
 */
void MortevielleEngine::showIntroduction() {
	_dialogManager->displayIntroScreen(false);
	_dialogManager->checkForF8(142, false);
	if (shouldQuit())
		return;

	_dialogManager->displayIntroFrame2();
	_dialogManager->checkForF8(143, true);
	if (shouldQuit())
		return;

	showTitleScreen();
	music();
	_mixer->stopAll();
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

	_menu->initMenu();

	charToHour();
	initGame();
	clearScreen();
	drawRightFrame();
	_mouse->showMouse();

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
