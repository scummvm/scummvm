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

#include "common/system.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/saveload.h"
#include "mortevielle/outtext.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

const byte tabdr[32] = {
	32, 101, 115,  97, 114, 105, 110,
	117, 116, 111, 108,  13, 100,  99,
	112, 109,  46, 118, 130,  39, 102,
	98,  44, 113, 104, 103,  33,  76,
	85, 106,  30,  31
};


const byte tab30[32] = {
	69,  67,  74, 138, 133, 120,  77, 122,
	121,  68,  65,  63,  73,  80,  83,  82,
	156,  45,  58,  79,  49,  86,  78,  84,
	71,  81,  64,  66, 135,  34, 136,  91
};



const byte tab31[32]= {
	93,  47,  48,  53,  50,  70, 124,  75,
	72, 147, 140, 150, 151,  57,  56,  51,
	107, 139,  55,  89, 131,  37,  54,  88,
	119,   0,   0,   0,   0,   0,   0,   0
};

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc), _randomSource("mortevielle"),
		_soundManager(_mixer) {
	g_vm = this;
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
	_okdes = false;
	_anyone = false;
	_brt = false;

	_textColor = 0;
	_currGraphicalDevice = -1;
	_newGraphicalDevice = -1;
	_place = -1;

	_c_zzz = -1;

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
	g_res = 2;

	_txxFileFl = false;
	// Load texts from TXX files
	loadTexts();

	// Load the mort.dat resource
	Common::ErrorCode result = loadMortDat();
	if (result != Common::kNoError)
		return result;

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
	decodeNumber(&_cfiecBuffer[161 * 16], ((822 * 128) - (161 * 16)) / 64);
	_c_zzz = 1;
	init_nbrepm();
	initMouse();

	loadPlaces();
	_soundOff = false;
	_largestClearScreen = false;

	teskbd();
	showConfigScreen();
	_newGraphicalDevice = _currGraphicalDevice;
	teskbd();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();
	g_ades = 0x7000;

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
	byte desiredLanguageId = (getLanguage() == Common::EN_ANY) ? LANG_ENGLISH : LANG_FRENCH;

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

/**
 * Check is a key was pressed
 * It also delays the engine and check if the screen has to be updated
 * @remarks	Originally called 'keypressed'
 */
bool MortevielleEngine::keyPressed() {
	// Check for any pending key presses
	handleEvents();

	// Check if it's time to draw the next frame
	if (g_system->getMillis() > (_lastGameFrame + GAME_FRAME_DELAY)) {
		_lastGameFrame = g_system->getMillis();
	
		_screenSurface.updateScreen();
	}

	// Delay briefly to keep CPU usage down
	g_system->delayMillis(5);

	// Return if there are any pending key presses
	return !_keypresses.empty(); 
}

/**
 * Wait for a keypress
 * @remarks	Originally called 'get_ch'
 */
int MortevielleEngine::getChar() {
	// If there isn't any pending keypress, wait until there is
	while (!shouldQuit() && _keypresses.empty()) {
		keyPressed();
	}

	// Return the top keypress
	return shouldQuit() ? 0 : _keypresses.pop();
}

/**
 * Handle pending events
 * @remarks		Since the ScummVM screen surface is double height to handle 640x200 using 640x400,
 * the mouse Y position is divided by 2 to keep the game thinking the Y goes from 0 - 199
 */
bool MortevielleEngine::handleEvents() {
	Common::Event event;
	if (!g_system->getEventManager()->pollEvent(event))
		return false;

	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
		_mousePos = Common::Point(event.mouse.x, event.mouse.y / 2);
		_mouse._pos.x = event.mouse.x;
		_mouse._pos.y = event.mouse.y / 2;

		if (event.type == Common::EVENT_LBUTTONDOWN)
			_mouseClick = true;
		else if (event.type == Common::EVENT_LBUTTONUP)
			_mouseClick = false;

		break;
	case Common::EVENT_KEYDOWN:
		addKeypress(event);
		break;
	default:
		break;
	}

	return true;
}

/**
 * Add the specified key to the pending keypress stack
 */
void MortevielleEngine::addKeypress(Common::Event &evt) {
	// Character to add
	char ch = evt.kbd.ascii;

	// Handle alphabetic keys
	if ((evt.kbd.keycode >= Common::KEYCODE_a) && (evt.kbd.keycode <= Common::KEYCODE_z)) {
		if (evt.kbd.hasFlags(Common::KBD_CTRL)) 
			ch = evt.kbd.keycode - Common::KEYCODE_a + 1;
		else
			ch = evt.kbd.keycode - Common::KEYCODE_a + 'A';
	} else if ((evt.kbd.keycode >= Common::KEYCODE_F1) && (evt.kbd.keycode <= Common::KEYCODE_F12)) {
		// Handle function keys
		ch = 59 + evt.kbd.keycode - Common::KEYCODE_F1;
	} else {
		// Series of special cases
		switch (evt.kbd.keycode) {
		case Common::KEYCODE_KP4:
		case Common::KEYCODE_LEFT:
			ch = '4';
			break;
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_DOWN:
			ch = '2';
			break;
		case Common::KEYCODE_KP6:
		case Common::KEYCODE_RIGHT:
			ch = '6';
			break;
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_UP:
			ch = '8';
			break;
		case Common::KEYCODE_KP7:
			ch = '7';
			break;
		case Common::KEYCODE_KP1:
			ch = '1';
			break;
		case Common::KEYCODE_KP9:
			ch = '9';
			break;
		case Common::KEYCODE_KP3:
			ch = '3';
			break;
		case Common::KEYCODE_KP5:
			ch = '5';
			break;
		case Common::KEYCODE_RETURN:
			ch = '\13';
			break;
		case Common::KEYCODE_ESCAPE:
			ch = '\33';
			break;
		default:
			break;
		}
	}

	if (ch != 0)
		_keypresses.push(ch);
}

static byte CURSOR_ARROW_DATA[16 * 16] = {
	0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x00, 0x0f, 0x0f, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x0f, 0x0f, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

/**
 * Initialise the mouse
 */
void MortevielleEngine::initMouse() {
	CursorMan.replaceCursor(CURSOR_ARROW_DATA, 16, 16, 0, 0, 0xff);
	CursorMan.showMouse(true);
}

/**
 * Sets the mouse position
 * @remarks		Since the ScummVM screen surface is double height to handle 640x200 using 640x400,
 * the mouse Y position is doubled to convert from 0-199 to 0-399
 */
void MortevielleEngine::setMousePos(const Common::Point &pt) {
	// Adjust the passed position from simulated 640x200 to 640x400 co-ordinates
	Common::Point newPoint(pt.x, (pt.y == 199) ? 399 : pt.y * 2);

	if (newPoint != _mousePos)
		// Warp the mouse to the new position
		g_system->warpMouse(newPoint.x, newPoint.y);

	// Save the new position
	_mousePos = newPoint;
}

/**
 * Delay by a given amount
 */
void MortevielleEngine::delay(int amount) {
	uint32 endTime = g_system->getMillis() + amount;

	while (g_system->getMillis() < endTime) {
		if (g_system->getMillis() > (_lastGameFrame + GAME_FRAME_DELAY)) {
			_lastGameFrame = g_system->getMillis();
			_screenSurface.updateScreen();
		}

		g_system->delayMillis(10);
	}
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

	return Common::kNoError;
}

/**
 * Show the game introduction
 */
void MortevielleEngine::showIntroduction() {
	f3f8::aff50(false);
	g_mlec = 0;
	f3f8::checkForF8(142, false);
	CHECK_QUIT;

	f3f8::ani50();
	f3f8::checkForF8(143, true);
	CHECK_QUIT;

	// TODO: Once music is implemented, only use the below delay if music is turned off
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

	for (g_crep = 1; g_crep <= _c_zzz; ++g_crep) 
		decodeNumber(&_cfiecBuffer[161 * 16], ((822 * 128) - (161 * 16)) / 64);

	loadBRUIT5();
	_menu.initMenu();

	charToHour();
	initGame();
	hirs();
	drawRightFrame();
	_mouse.showMouse();

	// Loop to play the game
	do {
		playGame();
		CHECK_QUIT;
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
		CHECK_QUIT;
	} while (!((_quitGame) || (_endGame) || (_loseGame)));

	if (_endGame)
		endGame();
	else if (_loseGame)
		askRestart();
}

/**
 * Waits for the user to select an action, and then handles it
 * @remarks	Originally called tecran
 */
void MortevielleEngine::handleAction() {
	const int lim = 20000;
	int temps = 0;
	char inkey = '\0';
	bool oo, funct = 0;

	clearScreenType3();
	oo = false;
	g_ctrm = 0;
	if (!_keyPressedEsc) {
		_menu.drawMenu();
		_menu._menuDisplayed = true;
		temps = 0;
		g_key = 0;
		funct = false;
		inkey = '.';

		_inMainGameLoop = true;
		do {
			_menu.mdn();
			prepareRoom();
			_mouse.moveMouse(funct, inkey);
			CHECK_QUIT;
			++temps;
		} while (!((_menu._menuSelected) || (temps > lim) || (funct) || (_anyone)));
		_inMainGameLoop = false;

		_menu.eraseMenu();
		_menu._menuDisplayed = false;
		if ((inkey == '\1') || (inkey == '\3') || (inkey == '\5') || (inkey == '\7') || (inkey == '\11')) {
			changeGraphicalDevice((uint)(ord(inkey) - 1) >> 1);
			return;
		}
		if (_menu._menuSelected && (_msg[3] == MENU_SAVE)) {
			Common::String saveName = Common::String::format("Savegame #%d", _msg[4] & 7);
			_savegameManager.saveGame(_msg[4] & 7, saveName);
		}
		if (_menu._menuSelected && (_msg[3] == MENU_LOAD))
			_savegameManager.loadGame((_msg[4] & 7) - 1);
		if (inkey == '\103') {       /* F9 */
			temps = Alert::show(_hintPctMessage, 1);
			return;
		} else if (inkey == '\77') {
			if ((_menuOpcode != OPCODE_NONE) && ((_msg[3] == MENU_ACTION) || (_msg[3] == MENU_SELF))) {
				_msg[4] = _menuOpcode;
				ecr3(getEngineString(S_IDEM));
			} else
				return;
		} else if (inkey == '\104') {
			if ((g_x != 0) && (g_y != 0))
				g_num = 9999;
			return;
		}
	}
	if (inkey == '\73') {
		_quitGame = true;
		hourToChar();
	} else {
		if ((funct) && (inkey != '\77'))
			return;
		if (temps > lim) {
			repon(2, 141);
			if (g_num == 9999)
				g_num = 0;
		} else {
			_menuOpcode = _msg[3];
			if ((_msg[3] == MENU_ACTION) || (_msg[3] == MENU_SELF))
				_menuOpcode = _msg[4];
			if (!_anyone) {
				if ((_heroSearching) || (_obpart)) {
					if (_mouse._pos.y < 12)
						return;

					if ((_msg[4] == OPCODE_SOUND) || (_msg[4] == OPCODE_LIFT)) {
						oo = true;
						if ((_msg[4] == OPCODE_LIFT) || (_obpart)) {
							endSearch();
							g_caff = _coreVar._currPlace;
							g_crep = 998;
						} else
							tsuiv();
						mennor();
					}
				}
			}
			do {
				if (! oo)
					handleOpcode();

				if ((g_ctrm == 0) && (! _loseGame) && (! _endGame)) {
					taffich();
					if (_okdes) {
						_okdes = false;
						dessin(0);
					}
					if ((!_syn) || (_col))
						repon(2, g_crep);
				}
			} while (_syn);
			if (g_ctrm != 0)
				tctrm();
		}
	}
}

/**
 * Engine function - Init Places
 * @remarks	Originally called 'init_lieu'
 */
void MortevielleEngine::loadPlaces() {
	Common::File f;

	if (!f.open("MXX.mor"))
		error("Missing file - MXX.mor");

	for (int i = 0; i < 7; ++i) {
		for (int j = 0; j < 25; ++j)
			_v_lieu[i][j] = f.readByte(); 
	}

	f.close();
}

/**
 * Set Text Color
 * @remarks	Originally called 'text_color'
 */
void MortevielleEngine::setTextColor(int col) {
	_textColor = col;
}

/**
 * Prepare screen - Type 1!
 * @remarks	Originally called 'ecrf1'
 */
void MortevielleEngine::prepareScreenType1() {
	// Large drawing
	_screenSurface.drawBox(0, 11, 512, 163, 15);
}

/**
 * Prepare room - Type 2!
 * @remarks	Originally called 'ecrf2'
 */
void MortevielleEngine::prepareScreenType2() {
	setTextColor(5);
}

/**
 * Prepare room - Type 3!
 * @remarks	Originally called 'ecrf7'
 */
void MortevielleEngine::prepareScreenType3() {
	setTextColor(4);
}

/**
 * Engine function - Update hour
 * @remarks	Originally called 'calch'
 */
void MortevielleEngine::updateHour(int &day, int &hour, int &minute) {
	int newHour = readclock();
	int th = g_jh + ((newHour - g_mh) / g_t);
	minute = ((th % 2) + _currHalfHour) * 30;
	hour = ((uint)th >> 1) + _currHour;
	if (minute == 60) {
		minute = 0;
		++hour;
	}
	day = (hour / 24) + _currDay;
	hour = hour - ((day - _currDay) * 24);
}

/**
 * Engine function - Convert character index to bit index
 * @remarks	Originally called 'conv'
 */
int MortevielleEngine::convertCharacterIndexToBitIndex(int characterIndex) {
	return 128 >> (characterIndex - 1);
}

/**
 * Engine function - Convert bit index to character index
 * @remarks	Originally called 'tip'
 */
int MortevielleEngine::convertBitIndexToCharacterIndex(int bitIndex) {
	int retVal = 0;

	if (bitIndex == 128)
		retVal = 1;
	else if (bitIndex == 64)
		retVal = 2;
	else if (bitIndex == 32)
		retVal = 3;
	else if (bitIndex == 16)
		retVal = 4;
	else if (bitIndex == 8)
		retVal = 5;
	else if (bitIndex == 4)
		retVal = 6;
	else if (bitIndex == 2)
		retVal = 7;
	else if (bitIndex == 1)
		retVal = 8;

	return retVal;
}

/**
 * Engine function - Reset presence in other rooms
 * @remarks	Originally called 't5'
 */
void MortevielleEngine::resetPresenceInRooms(int roomId) {
	if (roomId == DINING_ROOM)
		_blo = false;

	if (roomId != GREEN_ROOM) {
		_roomPresenceLuc = false;
		_roomPresenceIda = false;
	}

	if (roomId != PURPLE_ROOM)
		_purpleRoomPresenceLeo = false;

	if (roomId != DARKBLUE_ROOM) {
		_roomPresenceGuy = false;
		_roomPresenceEva = false;
	}

	if (roomId != BLUE_ROOM)
		_roomPresenceMax = false;
	if (roomId != RED_ROOM)
		_roomPresenceBob = false;
	if (roomId != GREEN_ROOM2)
		_roomPresencePat = false;
	if (roomId != TOILETS)
		_toiletsPresenceBobMax = false;
	if (roomId != BATHROOM)
		_bathRoomPresenceBobMax = false;
	if (roomId != ROOM9)
		_room9PresenceLeo = false;
}

/**
 * Engine function - Show the people present in the given room 
 * @remarks	Originally called 'affper'
 */
void MortevielleEngine::showPeoplePresent(int bitIndex) {
	int xp = 580 - (_screenSurface.getStringWidth("LEO") / 2);

	for (int i = 1; i <= 8; ++i)
		_menu.disableMenuItem(_menu._discussMenu[i]);

	clearScreenType10();
	if ((bitIndex & 128) == 128) {
		_screenSurface.putxy(xp, 24);
		_screenSurface.drawString("LEO", 4);
		_menu.enableMenuItem(_menu._discussMenu[1]);
	}
	if ((bitIndex & 64) == 64) {
		_screenSurface.putxy(xp, 32);
		_screenSurface.drawString("PAT", 4);
		_menu.enableMenuItem(_menu._discussMenu[2]);
	}
	if ((bitIndex & 32) == 32) {
		_screenSurface.putxy(xp, 40);
		_screenSurface.drawString("GUY", 4);
		_menu.enableMenuItem(_menu._discussMenu[3]);
	}
	if ((bitIndex & 16) == 16) {
		_screenSurface.putxy(xp, 48);
		_screenSurface.drawString("EVA", 4);
		_menu.enableMenuItem(_menu._discussMenu[4]);
	}
	if ((bitIndex & 8) == 8) {
		_screenSurface.putxy(xp, 56);
		_screenSurface.drawString("BOB", 4);
		_menu.enableMenuItem(_menu._discussMenu[5]);
	}
	if ((bitIndex & 4) == 4) {
		_screenSurface.putxy(xp, 64);
		_screenSurface.drawString("LUC", 4);
		_menu.enableMenuItem(_menu._discussMenu[6]);
	}
	if ((bitIndex & 2) == 2) {
		_screenSurface.putxy(xp, 72);
		_screenSurface.drawString("IDA", 4);
		_menu.enableMenuItem(_menu._discussMenu[7]);
	}
	if ((bitIndex & 1) == 1) {
		_screenSurface.putxy(xp, 80);
		_screenSurface.drawString("MAX", 4);
		_menu.enableMenuItem(_menu._discussMenu[8]);
	}
	_currBitIndex = bitIndex;
}

/**
 * Engine function - Select random characters
 * @remarks	Originally called 'choix'
 */
int MortevielleEngine::selectCharacters(int min, int max) {
	bool invertSelection = false;
	int rand = getRandomNumber(min, max);

	if (rand > 4) {
		rand = 8 - rand;
		invertSelection = true;
	}

	int i = 0;
	int retVal = 0;
	while (i < rand) {
		int charIndex = getRandomNumber(1, 8);
		int charBitIndex = convertCharacterIndexToBitIndex(charIndex);
		if ((retVal & charBitIndex) != charBitIndex) {
			++i;
			retVal |= charBitIndex;
		}
	}
	if (invertSelection)
		retVal = 255 - retVal;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Green Room
 * @remarks	Originally called 'cpl1'
 */
int MortevielleEngine::getPresenceStatsGreenRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	// The original uses an || instead of an &&, resulting 
	// in an always true condition. Based on the other tests, 
	// and on other scenes, we use an && instead.
	if ((hour > 7) && (hour < 11))
		retVal = 25;
	else if ((hour > 10) && (hour < 14))
		retVal = 35;
	else if ((hour > 13) && (hour < 16))
		retVal = 50;
	else if ((hour > 15) && (hour < 18))
		retVal = 5;
	else if ((hour > 17) && (hour < 22))
		retVal = 35;
	else if ((hour > 21) && (hour < 24))
		retVal = 50;
	else if ((hour >= 0) && (hour < 8))
		retVal = 70;

	_menu.mdn();

	return retVal;
}
/**
 * Engine function - Get Presence Statistics - Purple Room
 * @remarks	Originally called 'cpl2'
 */
int MortevielleEngine::getPresenceStatsPurpleRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 11))
		retVal = -2;
	else if (hour == 11)
		retVal = 100;
	else if ((hour > 11) && (hour < 23))
		retVal = 10;
	else if (hour == 23)
		retVal = 20;
	else if ((hour >= 0) && (hour < 8))
		retVal = 50;
	
	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Toilets
 * @remarks	Originally called 'cpl3'
 */
int MortevielleEngine::getPresenceStatsToilets() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 10)) || ((hour > 19) && (hour < 24)))
		retVal = 34;
	else if (((hour > 9) && (hour < 20)) || ((hour >= 0) && (hour < 9)))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Blue Room
 * @remarks	Originally called 'cpl5'
 */
int MortevielleEngine::getPresenceStatsBlueRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 6) && (hour < 10))
		retVal = 0;
	else if (hour == 10)
		retVal = 100;
	else if ((hour > 10) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 7))
		retVal = 50;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Red Room
 * @remarks	Originally called 'cpl6'
 */
int MortevielleEngine::getPresenceStatsRedRoom() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 13)) || ((hour > 17) && (hour < 20)))
		retVal = -2;
	else if (((hour > 12) && (hour < 17)) || ((hour > 19) && (hour < 24)))
		retVal = 35;
	else if (hour == 17)
		retVal = 100;
	else if ((hour >= 0) && (hour < 8))
		retVal = 60;

	return retVal;
}

/**
 * Shows the "you are alone" message in the status area
 * on the right hand side of the screen
 * @remarks	Originally called 'person'
 */
void MortevielleEngine::displayAloneText() {
	for (int cf = 1; cf <= 8; ++cf)
		_menu.disableMenuItem(_menu._discussMenu[cf]);

	Common::String sYou = getEngineString(S_YOU);
	Common::String sAre = getEngineString(S_ARE);
	Common::String sAlone = getEngineString(S_ALONE);

	clearScreenType10();
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sYou) / 2), 30);
	_screenSurface.drawString(sYou, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAre) / 2), 50);
	_screenSurface.drawString(sAre, 4);
	_screenSurface.putxy(580 - (_screenSurface.getStringWidth(sAlone) / 2), 70);
	_screenSurface.drawString(sAlone, 4);

	_currBitIndex = 0;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl10'
 */
int MortevielleEngine::getPresenceStatsDiningRoom(int &hour) {
	int day, minute;

	int retVal = 0;
	updateHour(day, hour, minute);
	if (((hour > 7) && (hour < 11)) || ((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 100;
	else if ((hour == 11) || ((hour > 20) && (hour < 24)))
		retVal = 45;
	else if (((hour > 13) && (hour < 17)) || (hour == 18))
		retVal = 35;
	else if (hour == 17)
		retVal = 60;
	else if ((hour >= 0) && (hour < 8))
		retVal = 5;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Bureau
 * @remarks	Originally called 'cpl11'
 */
int MortevielleEngine::getPresenceStatsBureau(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 12)) || ((hour > 20) && (hour < 24)))
		retVal = 25;
	else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21)))
		retVal = 5;
	else if ((hour > 13) && (hour < 17))
		retVal = 55;
	else if ((hour > 16) && (hour < 19))
		retVal = 45;
	else if ((hour >= 0) && (hour < 9))
		retVal = 0;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Kitchen
 * @remarks	Originally called 'cpl12'
 */
int MortevielleEngine::getPresenceStatsKitchen() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (((hour > 8) && (hour < 15)) || ((hour > 16) && (hour < 22)))
		retVal = 55;
	else if (((hour > 14) && (hour < 17)) || ((hour > 21) && (hour < 24)))
		retVal = 25;
	else if ((hour >= 0) && (hour < 5))
		retVal = 0;
	else if ((hour > 4) && (hour < 9))
		retVal = 15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Attic
 * @remarks	Originally called 'cpl13'
 */
int MortevielleEngine::getPresenceStatsAttic() {
	return 0;
}

/**
 * Engine function - Get Presence Statistics - Room Landing
 * @remarks	Originally called 'cpl15'
 */
int MortevielleEngine::getPresenceStatsLanding() {
	int day, hour, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if ((hour > 7) && (hour < 12))
		retVal = 25;
	else if ((hour > 11) && (hour < 14))
		retVal = 0;
	else if ((hour > 13) && (hour < 18))
		retVal = 10;
	else if ((hour > 17) && (hour < 20))
		retVal = 55;
	else if ((hour > 19) && (hour < 22))
		retVal = 5;
	else if ((hour > 21) && (hour < 24))
		retVal = 15;
	else if ((hour >= 0) && (hour < 8))
		retVal = -15;

	return retVal;
}

/**
 * Engine function - Get Presence Statistics - Room Chapel
 * @remarks	Originally called 'cpl20'
 */
int MortevielleEngine::getPresenceStatsChapel(int &hour) {
	int day, minute;
	int retVal = 0;

	updateHour(day, hour, minute);
	if (hour == 10)
		retVal = 65;
	else if ((hour > 10) && (hour < 21))
		retVal = 5;
	else if ((hour > 20) && (hour < 24))
		retVal = -15;
	else if ((hour >= 0) && (hour < 5))
		retVal = -300;
	else if ((hour > 4) && (hour < 10))
		retVal = -5;

	return retVal;
}

/**
 * Engine function - Check who is in the Green Room
 * @remarks	Originally called 'quelq1'
 */
void MortevielleEngine::setPresenceGreenRoom(int roomId) {
	int rand = getRandomNumber(1, 2);
	if (roomId == GREEN_ROOM) {
		if (rand == 1)
			_roomPresenceLuc = true;
		else
			_roomPresenceIda = true;
	} else if (roomId == DARKBLUE_ROOM) {
		if (rand == 1)
			_roomPresenceGuy = true;
		else
			_roomPresenceEva = true;
	}

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Purple Room
 * @remarks	Originally called 'quelq2'
 */
void MortevielleEngine::setPresencePurpleRoom() {
	if (_place == PURPLE_ROOM)
		_purpleRoomPresenceLeo = true;
	else
		_room9PresenceLeo = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Blue Room
 * @remarks	Originally called 'quelq5'
 */
void MortevielleEngine::setPresenceBlueRoom() {
	_roomPresenceMax = true;
	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Red Room
 * @remarks	Originally called 'quelq6'
 */
void MortevielleEngine::setPresenceRedRoom(int roomId) {
	if (roomId == RED_ROOM)
		_roomPresenceBob = true;
	else if (roomId == GREEN_ROOM2)
		_roomPresencePat = true;

	_currBitIndex = 10;
}

/**
 * Engine function - Check who is in the Dining Room
 * @remarks	Originally called 'quelq10'
 */
int MortevielleEngine::setPresenceDiningRoom(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if ((hour > 7) && (hour < 10)) {
			min = 5;
			max = 7;
		} else if ((hour > 9) && (hour < 12)) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 15)) || ((hour > 18) && (hour < 21))) {
			min = 6;
			max = 8;
		} else if (((hour > 14) && (hour < 19)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 5;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Bureau
 * @remarks	Originally called 'quelq11'
 */
int MortevielleEngine::setPresenceBureau(int hour) {
	int retVal = 0;

	if ((hour >= 0) && (hour < 8))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if (((hour > 7) && (hour < 10)) || ((hour > 20) && (hour < 24))) {
			min = 1;
			max = 3;
		} else if (((hour > 9) && (hour < 12)) || ((hour > 13) && (hour < 19))) {
			min = 1;
			max = 4;
		} else if (((hour > 11) && (hour < 14)) || ((hour > 18) && (hour < 21))) {
			min = 1;
			max = 2;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Kitchen
 * @remarks	Originally called 'quelq12'
 */
int MortevielleEngine::setPresenceKitchen() {
	int retVal = chlm();
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the Landing
 * @remarks	Originally called 'quelq15'
 */
int MortevielleEngine::setPresenceLanding() {
	bool test = false;
	int rand = 0;
	do {
		rand = getRandomNumber(1, 8);
		test = (((rand == 1) && (_purpleRoomPresenceLeo || _room9PresenceLeo)) ||
		        ((rand == 2) && _roomPresencePat) ||
		        ((rand == 3) && _roomPresenceGuy) ||
		        ((rand == 4) && _roomPresenceEva) ||
		        ((rand == 5) && _roomPresenceBob) ||
		        ((rand == 6) && _roomPresenceLuc) ||
		        ((rand == 7) && _roomPresenceIda) ||
		        ((rand == 8) && _roomPresenceMax));
	} while (test);

	int retVal = convertCharacterIndexToBitIndex(rand);
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Check who is in the chapel
 * @remarks	Originally called 'quelq20'
 */
int MortevielleEngine::setPresenceChapel(int hour) {
	int retVal = 0;

	if (((hour >= 0) && (hour < 10)) || ((hour > 18) && (hour < 24)))
		retVal = chlm();
	else {
		int min = 0, max = 0;
		if ((hour > 9) && (hour < 12)) {
			min = 3;
			max = 7;
		} else if ((hour > 11) && (hour < 18)) {
			min = 1;
			max = 2;
		} else if (hour == 18) {
			min = 2;
			max = 4;
		}
		retVal = selectCharacters(min, max);
	}
	showPeoplePresent(retVal);

	return retVal;
}

/**
 * Engine function - Get the answer after you known a door
 * @remarks	Originally called 'frap'
 */
void MortevielleEngine::getKnockAnswer() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour >= 0) && (hour < 8))
		g_crep = 190;
	else {
		if (getRandomNumber(1, 100) > 70)
			g_crep = 190;
		else
			g_crep = 147;
	}
}

/**
 * Engine function - Get Room Presence Bit Index
 * @remarks	Originally called 'nouvp'
 */
int MortevielleEngine::getPresenceBitIndex(int roomId) {
	int bitIndex = 0;
	if (roomId == GREEN_ROOM) {
		if (_roomPresenceLuc)
			bitIndex = 4;  // LUC
		if (_roomPresenceIda)
			bitIndex = 2;  // IDA
	} else if ( ((roomId == PURPLE_ROOM) && (_purpleRoomPresenceLeo))
			 || ((roomId == ROOM9) && (_room9PresenceLeo)))
		bitIndex = 128;    // LEO
	else if (roomId == DARKBLUE_ROOM) {
		if (_roomPresenceGuy)
			bitIndex = 32; // GUY
		if (_roomPresenceEva)
			bitIndex = 16; // EVA
	} else if ((roomId == BLUE_ROOM) && (_roomPresenceMax))
		bitIndex = 1;      // MAX
	else if ((roomId == RED_ROOM) && (_roomPresenceBob))
		bitIndex = 8;      // BOB
	else if ((roomId == GREEN_ROOM2) && (_roomPresencePat))
		bitIndex = 64;     // PAT
	else if ( ((roomId == TOILETS) && (_toiletsPresenceBobMax))
		   || ((roomId == BATHROOM) && (_bathRoomPresenceBobMax)) )
		bitIndex = 9;      // BOB + MAX

	if (bitIndex != 9)
		showPeoplePresent(bitIndex);

	return bitIndex;
}

/**
 * Engine function - initGame
 * @remarks	Originally called 'dprog'
 */
void MortevielleEngine::initGame() {
	_place = MANOR_FRONT;
	g_jh = 0;
	if (!_coreVar._alreadyEnteredManor)
		_blo = true;
	g_t = kTime1;
	g_mh = readclock();
}

/**
 * Engine function - Set Random Presence - Green Room
 * @remarks	Originally called 'pl1'
 */
void MortevielleEngine::setRandomPresenceGreenRoom(int cf) {
	if ( ((_place == GREEN_ROOM) && (!_roomPresenceLuc) && (!_roomPresenceIda))
	  || ((_place == DARKBLUE_ROOM) && (!_roomPresenceGuy) && (!_roomPresenceEva)) ) {
		int p = getPresenceStatsGreenRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceGreenRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Purple Room
 * @remarks	Originally called 'pl2'
 */
void MortevielleEngine::setRandomPresencePurpleRoom(int cf) {
	if (!_purpleRoomPresenceLeo) {
		int p = getPresenceStatsPurpleRoom();
		int rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Blue Room
 * @remarks	Originally called 'pl5'
 */
void MortevielleEngine::setRandomPresenceBlueRoom(int cf) {
	if (!_roomPresenceMax) {
		int p = getPresenceStatsBlueRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceBlueRoom();
	}
}

/**
 * Engine function - Set Random Presence - Red Room
 * @remarks	Originally called 'pl6'
 */
void MortevielleEngine::setRandomPresenceRedRoom(int cf) {
	if ( ((_place == RED_ROOM) && (!_roomPresenceBob))
	  || ((_place == GREEN_ROOM2) && (!_roomPresencePat)) ) {
		int p = getPresenceStatsRedRoom();
		int rand;

		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresenceRedRoom(_place);
	}
}

/**
 * Engine function - Set Random Presence - Room 9
 * @remarks	Originally called 'pl9'
 */
void MortevielleEngine::setRandomPresenceRoom9(int cf) {
	if (!_room9PresenceLeo) {
		cf = -10;
		int p, rand;
		phaz(rand, p, cf);

		if (rand > p)
			displayAloneText();
		else
			setPresencePurpleRoom();
	}
}

/**
 * Engine function - Set Random Presence - Dining Room
 * @remarks	Originally called 'pl10'
 */
void MortevielleEngine::setRandomPresenceDiningRoom(int cf) {
	int h, rand;
	int p = getPresenceStatsDiningRoom(h);
	phaz(rand, p, cf);

	if (rand > p)
		displayAloneText();
	else
		setPresenceDiningRoom(h);
}

/**
 * Engine function - Set Random Presence - Bureau
 * @remarks	Originally called 'pl11'
 */
void MortevielleEngine::setRandomPresenceBureau(int cf) {
	int h, rand;

	int p = getPresenceStatsBureau(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceBureau(h);
}

/**
 * Engine function - Set Random Presence - Kitchen
 * @remarks	Originally called 'pl12'
 */
void MortevielleEngine::setRandomPresenceKitchen(int cf) {
	int p, rand;

	p = getPresenceStatsKitchen();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Attic / Cellar
 * @remarks	Originally called 'pl13'
 */
void MortevielleEngine::setRandomPresenceAttic(int cf) {
	int p, rand;

	p = getPresenceStatsAttic();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceKitchen();
}

/**
 * Engine function - Set Random Presence - Landing
 * @remarks	Originally called 'pl15'
 */
void MortevielleEngine::setRandomPresenceLanding(int cf) {
	int p, rand;

	p = getPresenceStatsLanding();
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceLanding();
}

/**
 * Engine function - Set Random Presence - Chapel
 * @remarks	Originally called 'pl20'
 */
void MortevielleEngine::setRandomPresenceChapel(int cf) {
	int h, rand;

	int p = getPresenceStatsChapel(h);
	phaz(rand, p, cf);
	if (rand > p)
		displayAloneText();
	else
		setPresenceChapel(h);
}

/**
 * Start music or speech
 * @remarks	Originally called 'musique'
 */
void MortevielleEngine::startMusicOrSpeech(int so) {
	if (so == 0) {
		/* musik(0) */
		;
	} else if ((g_prebru == 0) && (!_coreVar._alreadyEnteredManor)) {
		// Type 1: Speech
		_speechManager.startSpeech(10, 1, 1);
		++g_prebru;
	} else {
		if (((_coreVar._currPlace == MOUNTAIN) || (_coreVar._currPlace == MANOR_FRONT) || (_coreVar._currPlace == MANOR_BACK)) && (getRandomNumber(1, 3) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(9, getRandomNumber(2, 4), 1);
		else if ((_coreVar._currPlace == CHAPEL) && (getRandomNumber(1, 2) == 1))
			// Type 1: Speech
			_speechManager.startSpeech(8, 1, 1);
		else if ((_coreVar._currPlace == WELL) && (getRandomNumber(1, 2) == 2))
			// Type 1: Speech
			_speechManager.startSpeech(12, 1, 1);
		else if (_coreVar._currPlace == INSIDE_WELL)
			// Type 1: Speech
			_speechManager.startSpeech(13, 1, 1);
		else
			// Type 2 : music
			_speechManager.startSpeech(getRandomNumber(1, 17), 1, 2);
	}
}

/**
 * Engine function - You lose!
 * @remarks	Originally called 'tperd'
 */
void MortevielleEngine::loseGame() {
	initouv();
	g_ment = 0;
	g_iouv = 0;
	g_mchai = 0;
	_menu.unsetSearchMenu();
	if (!_blo)
		t11(MANOR_FRONT);

	_loseGame = true;
	clearScreenType1();
	_screenSurface.drawBox(60, 35, 400, 50, 15);
	repon(9, g_crep);
	clearScreenType2();
	clearScreenType3();
	_col = false;
	_syn = false;
	_okdes = false;
}

/**
 * Engine function - Check inventory for a given object
 * @remarks	Originally called 'cherjer'
 */
bool MortevielleEngine::checkInventory(int objectId) {
	bool retVal = false;
	for (int i = 1; i <= 6; ++i)
		retVal = (retVal || (ord(_coreVar._sjer[i]) == objectId));

	if (_coreVar._selectedObjectId == objectId)
		retVal = true;

	return retVal;
}

/**
 * Engine function - Display Dining Room
 * @remarks	Originally called 'st1sama'
 */
void MortevielleEngine::displayDiningRoom() {
	_coreVar._currPlace = DINING_ROOM;
	affrep();
}

/**
 * Engine function - Start non interactive Dialog
 * @remarks	Originally called 'sparl'
 */
void MortevielleEngine::startDialog(int16 rep) {
	const int haut[9] = { 0, 0, 1, -3, 6, -2, 2, 7, -1 };
	int key;

	assert(rep >= 0);

	_mouse.hideMouse();
	Common::String dialogStr = getString(rep + kDialogStringIndex);
	displayStr(dialogStr, 230, 4, 65, 24, 5);
	f3f8::draw();
	
	key = 0;
	do {
		_speechManager.startSpeech(rep, haut[g_caff - 69], 0);
		key = f3f8::waitForF3F8();
		CHECK_QUIT;
	} while (key != 66);
	hirs();
	_mouse.showMouse();
}

/**
 * Engine function - End of Search: reset globals
 * @remarks	Originally called 'finfouill'
 */
void MortevielleEngine::endSearch() {
	_heroSearching = false;
	_obpart = false;
	g_cs = 0;
	g_is = 0;
	_menu.unsetSearchMenu();
}

/**
 * Engine function - Go to Dining room
 * @remarks	Originally called 't1sama'
 */
void MortevielleEngine::gotoDiningRoom() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if ((hour < 5) && (_coreVar._currPlace > ROOM18)) {
		if (!checkInventory(137)) {        //You don't have the keys, and it's late
			g_crep = 1511;
			loseGame();
		} else
			displayDiningRoom();
	} else if (!_coreVar._alreadyEnteredManor) {     //Is it your first time?
		_currBitIndex = 255; // Everybody is present
		showPeoplePresent(_currBitIndex);
		g_caff = 77;
		afdes();
		_screenSurface.drawBox(223, 47, 155, 91, 15);
		repon(2, 33);
		tkey1(false);
		mennor();
		_mouse.hideMouse();
		hirs();
		premtet();
		startDialog(140);
		drawRightFrame();
		drawClock();
		_mouse.showMouse();
		_coreVar._currPlace = OWN_ROOM;
		affrep();
		resetPresenceInRooms(DINING_ROOM);
		if (!_blo)
			minute = t11(OWN_ROOM);
		_currBitIndex = 0;
		g_mpers = 0;
		_coreVar._alreadyEnteredManor = true;
	} else
		displayDiningRoom();
}

/**
 * Engine function - Check Manor distance (in the mountains)
 * @remarks	Originally called 't1neig'
 */
void MortevielleEngine::checkManorDistance() {
	++_manorDistance;
	if (_manorDistance > 2) {
		g_crep = 1506;
		loseGame();
	} else {
		_okdes = true;
		_coreVar._currPlace = MOUNTAIN;
		affrep();
	}
}

/**
 * Engine function - Go to Manor front
 * @remarks	Originally called 't1deva'
 */
void MortevielleEngine::gotoManorFront() {
	_manorDistance = 0;
	_coreVar._currPlace = MANOR_FRONT;
	affrep();
}

/**
 * Engine function - Go to Manor back
 * @remarks	Originally called 't1derr'
 */
void MortevielleEngine::gotoManorBack() {
	_coreVar._currPlace = MANOR_BACK;
	affrep();
}

/**
 * Engine function - Dead : Flooded in Well 
 * @remarks	Originally called 't1deau'
 */
void MortevielleEngine::floodedInWell() {
	g_crep = 1503;
	loseGame();
}

/**
 * Engine function - Change Graphical Device
 * @remarks	Originally called 'change_gd'
 */
void MortevielleEngine::changeGraphicalDevice(int newDevice) {
	_mouse.hideMouse();
	_currGraphicalDevice = newDevice;
	hirs();
	_mouse.initMouse();
	_mouse.showMouse();
	drawRightFrame();
	prepareRoom();
	drawClock();
	if (_currBitIndex != 0)
		showPeoplePresent(_currBitIndex);
	else
		displayAloneText();
	clearScreenType2();
	clearScreenType3();
	g_maff = 68;
	afdes();
	repon(2, g_crep);
	_menu.displayMenu();
}

/**
 * Called when a savegame has been loaded.
 * @remarks	Originally called 'antegame'
 */
void MortevielleEngine::gameLoaded() {
	_mouse.hideMouse();
	_menu._menuDisplayed = false;
	_loseGame = true;
	_anyone = false;
	_okdes = true;
	_col = false;
	_hiddenHero = false;
	_brt = false;
	g_maff = 68;
	_menuOpcode = OPCODE_NONE;
	g_prebru = 0;
	g_x = 0;
	g_y = 0;
	g_num = 0;
	g_hdb = 0;
	g_hfb = 0;
	g_cs = 0;
	g_is = 0;
	g_ment = 0;
	_syn = true;
	_heroSearching = true;
	g_mchai = 0;
	_manorDistance = 0;
	initouv();
	g_iouv = 0;
	g_dobj = 0;
	affrep();
	_hintPctMessage = getString(580);

	_okdes = false;
	_endGame = true;
	_loseGame = false;
	_heroSearching = false;

	displayAloneText();
	prepareRoom();
	drawClock();
	afdes();
	repon(2, g_crep);
	clearScreenType3();
	_endGame = false;
	_menu.setDestinationMenuText(_coreVar._currPlace);
	modinv();
	if (_coreVar._selectedObjectId != 0)
		modobj(_coreVar._selectedObjectId + 400);
	_mouse.showMouse();
}

/**
 * Engine function - Handle OpCodes
 * @remarks	Originally called 'tsitu'
 */
void MortevielleEngine::handleOpcode() {
	if (!_col)
		clearScreenType2();
	_syn = false;
	_keyPressedEsc = false;
	if (!_anyone) {
		if (_brt) {
			if ((_msg[3] == MENU_MOVE) || (_msg[4] == OPCODE_LEAVE) || (_msg[4] == OPCODE_SLEEP) || (_msg[4] == OPCODE_EAT)) {
				g_ctrm = 4;
				mennor();
				return;
			}
		}
		if (_msg[3] == MENU_MOVE)
			fctMove();
		if (_msg[3] == MENU_DISCUSS)
			fctDiscuss();
		if (_msg[3] == MENU_INVENTORY)
			fctInventoryTake();
		if (_msg[4] == OPCODE_ATTACH)
			fctAttach();
		if (_msg[4] == OPCODE_WAIT)
			fctWait();
		if (_msg[4] == OPCODE_FORCE)
			fctForce();
		if (_msg[4] == OPCODE_SLEEP)
			fctSleep();
		if (_msg[4] == OPCODE_LISTEN)
			fctListen();
		if (_msg[4] == OPCODE_ENTER)
			fctEnter();
		if (_msg[4] == OPCODE_CLOSE)
			fctClose();
		if (_msg[4] == OPCODE_SEARCH)
			fctSearch();
		if (_msg[4] == OPCODE_KNOCK)
			fctKnock();
		if (_msg[4] == OPCODE_SCRATCH)
			fctScratch();
		if (_msg[4] == OPCODE_READ)
			fctRead();
		if (_msg[4] == OPCODE_EAT)
			fctEat();
		if (_msg[4] == OPCODE_PLACE)
			fctPlace();
		if (_msg[4] == OPCODE_OPEN)
			fctOpen();
		if (_msg[4] == OPCODE_TAKE)
			fctTake();
		if (_msg[4] == OPCODE_LOOK)
			fctLook();
		if (_msg[4] == OPCODE_SMELL)
			fctSmell();
		if (_msg[4] == OPCODE_SOUND)
			fctSound();
		if (_msg[4] == OPCODE_LEAVE)
			fctLeave();
		if (_msg[4] == OPCODE_LIFT)
			fctLift();
		if (_msg[4] == OPCODE_TURN)
			fctTurn();
		if (_msg[4] == OPCODE_SSEARCH)
			fctSelfSearch();
		if (_msg[4] == OPCODE_SREAD)
			fctSelfRead();
		if (_msg[4] == OPCODE_SPUT)
			fctSelfPut();
		if (_msg[4] == OPCODE_SLOOK)
			fctSelftLook();
		_hiddenHero = false;

		if (_msg[4] == OPCODE_SHIDE)
			fctSelfHide();
	} else {
		if (_anyone) {
			quelquun();
			_anyone = false;
			mennor();
			return;
		}
	}
	int hour, day, minute;
	updateHour(day, hour, minute);
	if ((((hour == 12) || (hour == 13) || (hour == 19)) && (_coreVar._currPlace != 10)) ||
	        ((hour > 0) && (hour < 6) && (_coreVar._currPlace != 0)))
		++_coreVar._faithScore;
	if (((_coreVar._currPlace < CRYPT) || (_coreVar._currPlace > MOUNTAIN)) && (_coreVar._currPlace != INSIDE_WELL)
	        && (_coreVar._currPlace != OWN_ROOM) && (_coreVar._selectedObjectId != 152) && (!_loseGame)) {
		if ((_coreVar._faithScore > 99) && (hour > 8) && (hour < 16)) {
			g_crep = 1501;
			loseGame();
		}
		if ((_coreVar._faithScore > 99) && (hour > 0) && (hour < 9)) {
			g_crep = 1508;
			loseGame();
		}
		if ((day > 1) && (hour > 8) && (!_loseGame)) {
			g_crep = 1502;
			loseGame();
		}
	}
	mennor();
}

/**
 * Engine function - Transform time into a char
 * @remarks	Originally called 'tmaj3'
 */
void MortevielleEngine::hourToChar() {
	int day, hour, minute;

	updateHour(day, hour, minute);
	if (minute == 30)
		minute = 1;
	hour += day * 24;
	minute += hour * 2;
	_coreVar._fullHour = chr(minute);
}

/**
 * Engine function - extract time from a char
 * @remarks	Originally called 'theure'
 */
void MortevielleEngine::charToHour() {
	int fullHour = ord(_coreVar._fullHour);
	int tmpHour = fullHour % 48;
	_currDay = fullHour / 48;
	_currHalfHour = tmpHour % 2;
	_currHour = tmpHour / 2;
	_hour = _currHour;
	if (_currHalfHour == 1)
		_minute = 30;
	else
		_minute = 0;
}

/**
 * Engine function - Clear Screen - Type 1
 * @remarks	Originally called 'clsf1'
 */
void MortevielleEngine::clearScreenType1() {
	_mouse.hideMouse();
	_screenSurface.fillRect(0, Common::Rect(0, 11, 514, 175));
	_mouse.showMouse();
}

/**
 * Engine function - Clear Screen - Type 2
 * @remarks	Originally called 'clsf2'
 */
void MortevielleEngine::clearScreenType2() {
	_mouse.hideMouse();
	if (_largestClearScreen) {
		_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 199));
		_screenSurface.drawBox(0, 175, 634, 24, 15);
		_largestClearScreen = false;
	} else {
		_screenSurface.fillRect(0, Common::Rect(1, 176, 633, 190));
		_screenSurface.drawBox(0, 175, 634, 15, 15);
	}
	_mouse.showMouse();
}

/**
 * Engine function - Clear Screen - Type 3
 * @remarks	Originally called 'clsf3'
 */
void MortevielleEngine::clearScreenType3() {
	_mouse.hideMouse();
	_screenSurface.fillRect(0, Common::Rect(1, 192, 633, 199));
	_screenSurface.drawBox(0, 191, 634, 8, 15);
	_mouse.showMouse();
}

/**
 * Engine function - Clear Screen - Type 10
 * @remarks	Originally called 'clsf10'
 */
void MortevielleEngine::clearScreenType10() {
	int co, cod;
	Common::String st;

	_mouse.hideMouse();
	if (g_res == 1) {
		co = 634;
		cod = 534;
	} else {
		co = 600;
		cod = 544;
	}
	_screenSurface.fillRect(15, Common::Rect(cod, 93, co, 98));
	if (_coreVar._faithScore < 33)
		st = getEngineString(S_COOL);
	else if (_coreVar._faithScore < 66)
		st = getEngineString(S_LOURDE);
	else if (_coreVar._faithScore > 65)
		st = getEngineString(S_MALSAINE);
	
	co = 580 - (_screenSurface.getStringWidth(st) / 2);
	_screenSurface.putxy(co, 92);
	_screenSurface.drawString(st, 4);

	_screenSurface.fillRect(15, Common::Rect(560, 24, 610, 86));
	/* rempli(69,12,32,5,255);*/
	_mouse.showMouse();
}

/**
 * Engine function - Get a random number between two values
 * @remarks	Originally called 'get_random_number' and 'hazard'
 */
int MortevielleEngine::getRandomNumber(int minval, int maxval) {
	return _randomSource.getRandomNumber(maxval - minval) + minval;
}

/**
 * Engine function - Show alert "use move menu"
 * @remarks	Originally called 'aldepl'
 */
void MortevielleEngine::showMoveMenuAlert() {
	Alert::show(getEngineString(S_USE_DEP_MENU), 1);
}

/**
 * The original engine used this method to display a starting text screen letting the player
 * select the graphics mode to use
 * @remarks	Originally called 'dialpre'
 */
void MortevielleEngine::showConfigScreen() {
	g_crep = 998;
}

/**
 * Decodes a number of 64 byte blocks
 * @param pStart	Start of data
 * @param count		Number of 64 byte blocks
 * @remarks	Originally called 'zzuul'
 */
void MortevielleEngine::decodeNumber(byte *pStart, int count) {
	while (count-- > 0) {
		for (int idx = 0; idx < 64; ++pStart, ++idx) {
			uint16 v = ((*pStart - 0x80) << 1) + 0x80;

			if (v & 0x8000)
				*pStart = 0;
			else if (v & 0xff00)
				*pStart = 0xff;
			else 
				*pStart = (byte)v;
		}
	}
}

void MortevielleEngine::cinq_huit(char &c, int &idx, byte &pt, bool &the_end) {
	uint16 oct, ocd;

	/* 5-8 */
	oct = _inpBuffer[idx];
	oct = ((uint16)(oct << (16 - pt))) >> (16 - pt);
	if (pt < 6) {
		++idx;
		oct = oct << (5 - pt);
		pt += 11;
		oct = oct | ((uint)_inpBuffer[idx] >> pt);
	} else {
		pt -= 5;
		oct = (uint)oct >> pt;
	}

	switch (oct) {
	case 11:
		c = '$';
		the_end = true;
		break;
	case 30:
	case 31:
		ocd = _inpBuffer[idx];
		ocd = (uint16)(ocd << (16 - pt)) >> (16 - pt);
		if (pt < 6) {
			++idx;
			ocd = ocd << (5 - pt);
			pt += 11;
			ocd = ocd | ((uint)_inpBuffer[idx] >> pt);
		} else {
			pt -= 5;
			ocd = (uint)ocd >> pt;
		}

		if (oct == 30)
			c = chr(tab30[ocd]);
		else
			c = chr(tab31[ocd]);

		if (c == '\0') {
			the_end = true;
			c = '#';
		}
		break;
	default:
		c = chr(tabdr[oct]);
		break;
	}
}

/**
 * Decode and extract the line with the given Id
 * @remarks	Originally called 'deline'
 */
Common::String MortevielleEngine::getString(int num) {
	Common::String wrkStr = "";

	if (num < 0) {
		warning("deline: num < 0! Skipping");
	} else if (!_txxFileFl) {
		wrkStr = getGameString(num);
	} else {
		int hint = _ntpBuffer[num]._hintId;
		byte point = _ntpBuffer[num]._point;
		int length = 0;
		bool endFl = false;
		char let;
		do {
			cinq_huit(let, hint, point, endFl);
			if (length < 254)
				wrkStr += let;
			++length;
		} while (!endFl);
	}

	while (wrkStr.lastChar() == '$')
		// Remove trailing '$'s
		wrkStr.deleteLastChar();

	return wrkStr;
}

void MortevielleEngine::copcha() {
	int i = kAcha;
	do {
		_tabdon[i] = _tabdon[i + 390];
		++i;
	} while (i != kAcha + 390);
}

/**
 * Engine function - When restarting the game, reset the main variables used by the engine
 * @remarks	Originally called 'inzon'
 */
void MortevielleEngine::resetVariables() {
	copcha();

	_coreVar._alreadyEnteredManor = false;
	_coreVar._selectedObjectId = 0;
	_coreVar._cellarObjectId = 0;
	_coreVar._atticBallHoleObjectId = 0;
	_coreVar._atticRodHoleObjectId = 0;
	_coreVar._wellObjectId = 0;
	_coreVar._secretPassageObjectId = 0;
	_coreVar._purpleRoomObjectId = 136;
	_coreVar._cryptObjectId = 141;
	_coreVar._faithScore = getRandomNumber(4, 10);
	_coreVar._currPlace = MANOR_FRONT;

	for (int i = 2; i <= 6; ++i)
		_coreVar._sjer[i] = chr(0);

	_coreVar._sjer[1] = chr(113);
	_coreVar._fullHour = chr(20);

	for (int i = 1; i <= 10; ++i)
		_coreVar._pourc[i] = ' ';

	for (int i = 1; i <= 6; ++i)
		_coreVar._teauto[i] = '*';

	for (int i = 7; i <= 9; ++i)
		_coreVar._teauto[i] = ' ';

	for (int i = 10; i <= 28; ++i)
		_coreVar._teauto[i] = '*';

	for (int i = 29; i <= 42; ++i)
		_coreVar._teauto[i] = ' ';

	_coreVar._teauto[33] = '*';

	for (int i = 1; i <= 8; ++i)
		_nbrep[i] = 0;

	init_nbrepm();
}

/**
 * Engine function - Set the palette
 * @remarks	Originally called 'writepal'
 */
void MortevielleEngine::setPal(int n) {
	switch (_currGraphicalDevice) {
	case MODE_TANDY:
	case MODE_EGA:
	case MODE_AMSTRAD1512:
		for (int i = 1; i <= 16; ++i) {
			g_mem[(0x7000 * 16) + (2 * i)] = _stdPal[n][i].x;
			g_mem[(0x7000 * 16) + (2 * i) + 1] = _stdPal[n][i].y;
		}
		break;
	case MODE_CGA: {
		nhom pal[16];
		for (int i = 0; i < 16; ++i) {
			pal[i] = _cgaPal[n]._a[i];
		}

		if (n < 89)
			palette(_cgaPal[n]._p);
		
		for (int i = 0; i <= 15; ++i)
			displayCGAPattern(i, _patternArr[pal[i]._id], pal);
		}
		break;
	default:
		break;
	}
}

/**
 * Engine function - Display a CGA pattern, using a specified palette
 * @remarks	Originally called 'outbloc'
 */
void MortevielleEngine::displayCGAPattern(int n, Pattern p, nhom *pal) {
	int addr = n * 404 + 0xd700;

	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + addr], p._tax);
	WRITE_LE_UINT16(&g_mem[0x6000 * 16 + addr + 2], p._tay);
	addr += 4;
	for (int i = 0; i < p._tax; ++i) {
		for (int j = 0; j < p._tay; ++j)
			g_mem[(0x6000 * 16) + addr + j * p._tax + i] = pal[n]._hom[p._des[i + 1][j + 1]];
	}
}

/**
 * Engine function - Load Palette from File
 * @remarks	Originally called 'charpal'
 */
void MortevielleEngine::loadPalette() {
	Common::File f;
	byte b;

	if (!f.open("fxx.mor"))
		error("Missing file - fxx.mor");
	for (int i = 0; i < 108; ++i)
		_fxxBuffer[i] = f.readSint16LE();
	f.close();

	if (!f.open("plxx.mor"))
		error("Missing file - plxx.mor");
	for (int i = 0; i <= 90; ++i) {
		for (int j = 1; j <= 16; ++j) {
			_stdPal[i][j].x = f.readByte();
			_stdPal[i][j].y = f.readByte();
		}
	}
	f.close();
	
	if (!f.open("cxx.mor"))
		error("Missing file - cxx.mor");

	for (int j = 0; j <= 90; ++j) {
		_cgaPal[j]._p = f.readByte();
		for (int i = 0; i <= 15; ++i) {
			nhom &with = _cgaPal[j]._a[i];

			b = f.readByte();
			with._id = (uint)b >> 4;
			with._hom[0] = ((uint)b >> 2) & 3;
			with._hom[1] = b & 3;
		}
	}

	_cgaPal[10]._a[9] = _cgaPal[10]._a[5];
	for (int j = 0; j <= 14; ++j) {
		_patternArr[j]._tax = f.readByte();
		_patternArr[j]._tay = f.readByte();
		for (int i = 1; i <= 20; ++i) {
			for (int k = 1; k <= 20; ++k)
				_patternArr[j]._des[i][k] = f.readByte();
		}
	}
	f.close();
}

/**
 * Engine function - Load Texts from File
 * @remarks	Originally called 'chartex'
 */
void MortevielleEngine::loadTexts() {
	Common::File inpFile;
	Common::File ntpFile;

	_txxFileFl = false;
	if (getLanguage() == Common::EN_ANY) {
		warning("English version expected - Switching to DAT file");
		return;
	}

	if (!inpFile.open("TXX.INP")) {
		if (!inpFile.open("TXX.MOR")) {
			warning("Missing file - TXX.INP or .MOR - Switching to DAT file");
			return;
		}
	} 
	if (!ntpFile.open("TXX.NTP")) {
		warning("Missing file - TXX.INP or .MOR - Switching to DAT file");
		return;
	}
	
	if ((inpFile.size() > (kMaxTi * 2)) || (ntpFile.size() > (kMaxTd * 3))) {
		warning("TXX file - Unexpected format - Switching to DAT file");
		return;
	} 

	for (int i = 0; i < inpFile.size() / 2; ++i)
		_inpBuffer[i] = inpFile.readUint16LE();

	inpFile.close();
	_txxFileFl = true;

	for (int i = 0; i < (ntpFile.size() / 3); ++i) {
		_ntpBuffer[i]._hintId = ntpFile.readSint16LE();
		_ntpBuffer[i]._point = ntpFile.readByte();
	}

	ntpFile.close();

}

void MortevielleEngine::loadBRUIT5() {
	Common::File f;

	if (!f.open("bruit5"))
		error("Missing file - bruit5");

	f.read(&g_mem[kAdrNoise5 * 16 + 0], 149 * 128);
	f.close();
}

void MortevielleEngine::loadCFIEC() {
	Common::File f;

	if (!f.open("cfiec.mor"))
		error("Missing file - cfiec.mor");

	f.read(&_cfiecBuffer[0], 822 * 128);
	f.close();

	_reloadCFIEC = false;
}


void MortevielleEngine::loadCFIPH() {
	Common::File f;

	if (!f.open("cfiph.mor"))
		error("Missing file - cfiph.mor");

	for (int i = 0; i < (f.size() / 2); ++i)
		_speechManager._cfiphBuffer[i] = f.readSint16LE();

	f.close();
}

/**
 * Engine function - Play Music
 * @remarks	Originally called 'music'
 */
void MortevielleEngine::music() {
	if (_soundOff)
		return;

	_reloadCFIEC = true;
	
	Common::File fic;
	if (!fic.open("mort.img"))
		error("Missing file - mort.img");

	fic.read(&g_mem[0x3800 * 16 + 0], 500);
	fic.read(&g_mem[0x47a0 * 16 + 0], 123);
	fic.close();

	_soundManager.decodeMusic(&g_mem[0x3800 * 16], &g_mem[0x5000 * 16], 623);
	_addfix = (float)((kTempoMusic - g_addv[1])) / 256;
	_speechManager.cctable(_speechManager._tbi);

	bool fin = false;
	int k = 0;
	do {
		fin = keyPressed();
		_soundManager.musyc(_speechManager._tbi, 9958, kTempoMusic);
		++k;
		fin = fin | keyPressed() | (k >= 5);
	} while (!fin);
	while (keyPressed())
		getChar();
}

/**
 * Engine function - Show title screen
 * @remarks	Originally called 'suite'
 */
void MortevielleEngine::showTitleScreen() {
	hirs();
	repon(7, 2035);
	g_caff = 51;
	taffich();
	teskbd();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();
	draw(g_ades, 0, 0);

	Common::String cpr = "COPYRIGHT 1989 : LANKHOR";
	_screenSurface.putxy(104 + 72 * g_res, 185);
	_screenSurface.drawString(cpr, 0);
}

/**
 * Draw picture
 * @remarks	Originally called 'dessine'
 */
void MortevielleEngine::draw(int ad, int x, int y) {
	_mouse.hideMouse();
	setPal(_numpal);
	pictout(ad, 0, x, y);
	_mouse.showMouse();
}

/**
 * Draw right frame
 * @remarks	Originally called 'dessine_rouleau'
 */
void MortevielleEngine::drawRightFrame() {
	setPal(89);
	if (_currGraphicalDevice == MODE_HERCULES) {
		g_mem[0x7000 * 16 + 14] = 15;
	}
	_mouse.hideMouse();
	pictout(0x73a2, 0, 0, 0);
	_mouse.showMouse();
}

/**
 * Read the current system time
 */
int MortevielleEngine::readclock() {
	TimeDate dateTime;
	g_system->getTimeAndDate(dateTime);

	int m = dateTime.tm_min * 60;
	int h = dateTime.tm_hour * 3600;
	return h + m + dateTime.tm_sec;
}

/**
 * Engine function - Prepare room and hint string
 * @remarks	Originally called 'tinke'
 */
void MortevielleEngine::prepareRoom() {
	Common::String d1 = getEngineString(S_SHOULD_HAVE_NOTICED);
	Common::String d2 = getEngineString(S_NUMBER_OF_HINTS);
	const char d3 = '[';
	const char d4 = ']';
	const char d5 = '1';
	Common::String d6 = getEngineString(S_OK);
	int cf, day, hour, minute;
	Common::String stpo;

	_anyone = false;
	updateHour(day, hour, minute);
	if (day != _day) {
		_day = day;
		int i = 0;
		do {
			++i;
			if (_nbrepm[i] != 0)
				--_nbrepm[i];
			_nbrep[i] = 0;
		} while (i != 8);
	}
	if ((hour > _hour) || ((hour == 0) && (_hour == 23))) {
		_hour = hour;
		_minute = 0;
		drawClock();
		cf = 0;
		for (int i = 1; i <= 10; ++i) {
			if (_coreVar._pourc[i] == '*')
				++cf;
		}

		if (cf == 10)
			stpo = "10";
		else
			stpo = chr(cf + 48);

		_hintPctMessage = Common::String(d3);
		_hintPctMessage += d5;
		_hintPctMessage += d4;
		_hintPctMessage += d3;
		_hintPctMessage += d1;
		_hintPctMessage += stpo;
		_hintPctMessage += '0';
		_hintPctMessage += d2;
		_hintPctMessage += d4;
		_hintPctMessage += d3;
		_hintPctMessage += d6;
		_hintPctMessage += d4;
	}
	if (minute > _minute) {
		_minute = 30;
		drawClock();
	}
	if (_mouse._pos.y < 12)
		return;

	if (!_blo) {
		if ((hour == 12) || ((hour > 18) && (hour < 21)) || ((hour >= 0) && (hour < 7)))
			g_t = kTime2;
		else
			g_t = kTime1;
		cf = _coreVar._faithScore;
		if ((cf > 33) && (cf < 66))
			g_t -= (g_t / 3);

		if (cf > 65)
			g_t -= ((g_t / 3) * 2);

		int nh = readclock();
		if ((nh - g_mh) > g_t) {
			bool activeMenu = _menu._menuActive;
			_menu.eraseMenu();
			g_jh += ((nh - g_mh) / g_t);
			g_mh = nh;
			switch (_place) {
			case GREEN_ROOM:
			case DARKBLUE_ROOM:
				setRandomPresenceGreenRoom(cf);
				break;
			case PURPLE_ROOM:
				setRandomPresencePurpleRoom(cf);
				break;
			case BLUE_ROOM:
				setRandomPresenceBlueRoom(cf);
				break;
			case RED_ROOM:
			case GREEN_ROOM2:
				setRandomPresenceRedRoom(cf);
				break;
			case ROOM9:
				setRandomPresenceRoom9(cf);
				break;
			case DINING_ROOM:
				setRandomPresenceDiningRoom(cf);
				break;
			case BUREAU:
				setRandomPresenceBureau(cf);
				break;
			case KITCHEN:
				setRandomPresenceKitchen(cf);
				break;
			case ATTIC:
			case CELLAR:
				setRandomPresenceAttic(cf);
				break;
			case LANDING:
			case ROOM26:
				setRandomPresenceLanding(cf);
				break;
			case CHAPEL:
				setRandomPresenceChapel(cf);
				break;
			}
			if ((g_mpers != 0) && (_currBitIndex != 10))
				g_mpers = _currBitIndex;

			if ((g_mpers == 0) && (_currBitIndex > 0)) {
				if ((_coreVar._currPlace == ATTIC) || (_coreVar._currPlace == CELLAR)) {
					cavegre();
				} else if (_currBitIndex == 10) {
					_currBitIndex = 0;
					if (!_brt) {
						_brt = true;
						g_hdb = readclock();
						if (getRandomNumber(1, 5) < 5) {
							clearScreenType3();
							prepareScreenType2();
							ecr3(getEngineString(S_HEAR_NOISE));
							int rand = (getRandomNumber(0, 4)) - 2;
							_speechManager.startSpeech(1, rand, 1);
							clearScreenType3();
						}
					}
				}
			}

			if (activeMenu)
				_menu.drawMenu();
		}
	}
	g_hfb = readclock();
	if ((_brt) && ((g_hfb - g_hdb) > 17)) {
		getPresenceBitIndex(_place);
		_brt = false;
		g_hdb = 0;
		if ((_coreVar._currPlace > OWN_ROOM) && (_coreVar._currPlace < DINING_ROOM))
			_anyone = true;
	}
}

/**
 * Engine function - Draw Clock
 * @remarks	Originally called 'pendule'
 */
void MortevielleEngine::drawClock() {
	const int cv[2][12] = {
		{  5,  8, 10,  8,  5,  0, -5, -8, -10, -8, -5,  0 },
		{ -5, -3,  0,  3,  5,  6,  5,  3,   0, -3, -5, -6 }
	};
	const int x = 580;
	const int y = 123;
	const int rg = 9;
	int h, co;

	_mouse.hideMouse();
	
	paint_rect(570, 118, 20, 10);
	paint_rect(578, 114, 6, 18);
	if ((_currGraphicalDevice == MODE_CGA) || (_currGraphicalDevice == MODE_HERCULES))
		co = 0;
	else
		co = 1;

	if (_minute == 0)
		_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y - rg), co);
	else 
		_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)x >> 1) * g_res, (y + rg), co);

	h = _hour;
	if (h > 12)
		h -= 12;
	if (h == 0)
		h = 12;

	_screenSurface.drawLine(((uint)x >> 1) * g_res, y, ((uint)(x + cv[0][h - 1]) >> 1) * g_res, y + cv[1][h - 1], co);
	_mouse.showMouse();
	_screenSurface.putxy(568, 154);

	if (_hour > 11)
		_screenSurface.drawString("PM ", 1);
	else
		_screenSurface.drawString("AM ", 1);

	_screenSurface.putxy(550, 160);
	if ((_day >= 0) && (_day <= 8)) {
		Common::String tmp = getEngineString(S_DAY);
		tmp.insertChar((char)(_day + 49), 0);
		_screenSurface.drawString(tmp, 1);
	}
}

void MortevielleEngine::palette(int v1) {
	warning("TODO: palette");
}

/**
 * Returns a substring of the given string
 * @param s		Source string
 * @param idx	Starting index (1 based)
 * @param size	Number of characters to return
 */

Common::String MortevielleEngine::copy(const Common::String &s, int idx, size_t size) {
	// Copy the substring into a temporary buffer
	char *tmp = new char[size + 1];
	strncpy(tmp, s.c_str() + idx - 1, size);
	tmp[size] = '\0';

	Common::String result(tmp);
	delete[] tmp;
	return result;
}

} // End of namespace Mortevielle
