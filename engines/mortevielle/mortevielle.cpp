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
#include "mortevielle/actions.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/asm.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/level15.h"
#include "mortevielle/menu.h"
#include "mortevielle/mor.h"
#include "mortevielle/mor2.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/ovd1.h"
#include "mortevielle/parole2.h"
#include "mortevielle/prog.h"
#include "mortevielle/saveload.h"
#include "mortevielle/taffich.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc), _randomSource("mortevielle"),
		_soundManager(_mixer) {
	g_vm = this;
	_lastGameFrame = 0;
	_mouseClick = false;
	_inMainGameLoop = false;
	_quitGame = false;
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
	res = 2;

	_txxFileFl = false;
	// Load texts from TXX files
	chartex();

	// Load the mort.dat resource
	Common::ErrorCode result = loadMortDat();
	if (result != Common::kNoError)
		return result;

	// Load some error messages (was previously in chartex())
	int length = 0;
	char str[1410];

	deline(578, str, length);
	al_mess = delig;
	deline(579, str, length);
	err_mess = delig;
	deline(580, str, length);
	ind_mess = delig;
	deline(581, str, length);
	al_mess2 = delig;

	// Set default EGA palette
	_paletteManager.setDefaultPalette();

	// Setup the mouse cursor
	initMouse();

	_currGraphicalDevice = MODE_EGA;
	_newGraphicalDevice = _currGraphicalDevice;
	zuul = false;
	tesok = false;
	charpal();
	charge_cfiph();
	charge_cfiec();
	zzuul(&adcfiec[161 * 16], ((822 * 128) - (161 * 16)) / 64);
	c_zzz = 1;
	init_nbrepm();
	initMouse();

	init_lieu();
	_soundOff = false;
	f2_all = false;

	teskbd();
	dialpre();
	_newGraphicalDevice = _currGraphicalDevice;
	teskbd();
	if (_newGraphicalDevice != _currGraphicalDevice)
		_currGraphicalDevice = _newGraphicalDevice;
	hirs();
	ades = 0x7000;

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

bool MortevielleEngine::keyPressed() {
	// Check for any pending key presses
	handleEvents();

	// Check if it's time to draw the next frame
	if (g_system->getMillis() > (_lastGameFrame + GAME_FRAME_DELAY)) {
		_lastGameFrame = g_system->getMillis();
	
		g_vm->_screenSurface.updateScreen();
	}

	// Delay briefly to keep CPU usage down
	g_system->delayMillis(5);

	// Return if there are any pending key presses
	return !_keypresses.empty(); 
}

int MortevielleEngine::getChar() {
	// If there isn't any pending keypress, wait until there is
	while (!shouldQuit() && _keypresses.empty()) {
		keypressed();
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
		x_s = event.mouse.x;
		y_s = event.mouse.y / 2;

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
			g_vm->_screenSurface.updateScreen();
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
	_savegameManager.takesav(loadSlot);

	// Run the main game loop
	mainGame();

	return Common::kNoError;
}

/**
 * Show the game introduction
 */
void MortevielleEngine::showIntroduction() {
	f3f8::aff50(false);
	mlec = 0;
	f3f8::checkForF8(142, false);
	CHECK_QUIT;

	f3f8::ani50();
	f3f8::checkForF8(143, true);
	CHECK_QUIT;

	// TODO: Once music is implemented, only use the below delay if music is turned off
	suite();
	delay(3000);
	music();
}

/**
 * Main game loop. Handles potentially playing the game multiple times, such as if the player
 * loses, and chooses to start playing the game again.
 */
void MortevielleEngine::mainGame() {
	if (rech_cfiec)
		charge_cfiec();

	for (crep = 1; crep <= c_zzz; ++crep) 
		zzuul(&adcfiec[161 * 16], ((822 * 128) - (161 * 16)) / 64);

	charge_bruit5();
	_menu.initMenu();

	theure();
	dprog();
	hirs();
	dessine_rouleau();
	showMouse();

	// Loop to play the game
	do {
		playGame();
		CHECK_QUIT;
	} while (!_quitGame);
}

/**
 * This method handles playing a loaded game
 * @remarks	Originally called tojouer
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
		loseGame();
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

	clsf3();
	oo = false;
	ctrm = 0;
	if (! iesc) {
		g_vm->_menu.drawMenu();
		imen = true;
		temps = 0;
		key = 0;
		funct = false;
		inkey = '.';

		_inMainGameLoop = true;
		do {
			g_vm->_menu.mdn();
			tinke();
			moveMouse(funct, inkey);
			CHECK_QUIT;
			temps = temps + 1;
		} while (!((choisi) || (temps > lim) || (funct) || (anyone)));
		_inMainGameLoop = false;

		g_vm->_menu.eraseMenu();
		imen = false;
		if ((inkey == '\1') || (inkey == '\3') || (inkey == '\5') || (inkey == '\7') || (inkey == '\11')) {
			changeGraphicalDevice((uint)(ord(inkey) - 1) >> 1);
			return;
		}
		if (choisi && (msg[3] == MENU_SAVE)) {
			Common::String saveName = Common::String::format("Savegame #%d", msg[4] & 7);
			g_vm->_savegameManager.saveGame(msg[4] & 7, saveName);
		}
		if (choisi && (msg[3] == MENU_LOAD))
			g_vm->_savegameManager.loadGame((msg[4] & 7) - 1);
		if (inkey == '\103') {       /* F9 */
			temps = Alert::show(stpou, 1);
			return;
		} else if (inkey == '\77') {
			if ((mnumo != OPCODE_NONE) && ((msg[3] == MENU_ACTION) || (msg[3] == MENU_SUB_ACTION))) {
				msg[4] = mnumo;
				ecr3(g_vm->getEngineString(S_IDEM));
			} else
				return;
		} else if (inkey == '\104') {
			if ((x != 0) && (y != 0))
				num = 9999;
			return;
		}
	}
	if (inkey == '\73') {
		_quitGame = true;
		tmaj3();
	} else {
		if ((funct) && (inkey != '\77'))
			return;
		if (temps > lim) {
			repon(2, 141);
			if (num == 9999)
				num = 0;
		} else {
			mnumo = msg[3];
			if ((msg[3] == MENU_ACTION) || (msg[3] == MENU_SUB_ACTION))
				mnumo = msg[4];
			if (! anyone) {
				if ((fouil) || (obpart)) {
					if (y_s < 12)
						return;

					if ((msg[4] == OPCODE_SOUND) || (msg[4] == OPCODE_LIFT)) {
						oo = true;
						if ((msg[4] == OPCODE_LIFT) || (obpart)) {
							finfouil();
							caff = s.mlieu;
							crep = 998;
						} else
							tsuiv();
						mennor();
					}
				}
			}
			do {
				if (! oo)
					tsitu();

				if ((ctrm == 0) && (! _loseGame) && (! _endGame)) {
					taffich();
					if (okdes) {
						okdes = false;
						dessin(0);
					}
					if ((! syn) || (col))
						repon(2, crep);
				}
			} while (syn);
			if (ctrm != 0)
				tctrm();
		}
	}
}

} // End of namespace Mortevielle
