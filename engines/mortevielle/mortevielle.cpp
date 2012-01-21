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
#include "common/debug-channels.h"
#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mort.h"
#include "mortevielle/mouse.h"
#include "mortevielle/var_mor.h"

namespace Mortevielle {

MortevielleEngine *g_vm;

MortevielleEngine::MortevielleEngine(OSystem *system, const ADGameDescription *gameDesc):
		Engine(system), _gameDescription(gameDesc), _randomSource("mortevielle") {
	g_vm = this;
	_lastGameFrame = 0;
	_mouseClick = false;
}

MortevielleEngine::~MortevielleEngine() {
}

bool MortevielleEngine::hasFeature(EngineFeature f) const {
	return false;
}

Common::ErrorCode MortevielleEngine::initialise() {
	// Initialise graphics mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, true);

	// Set debug channels
	DebugMan.addDebugChannel(kMortevielleCore, "core", "Core debugging");
	DebugMan.addDebugChannel(kMortevielleGraphics, "graphics", "Graphics debugging");

	// Set up an intermediate screen surface
	_screenSurface.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());

	// Set the screen mode
	gd = ega;
	res = 2;

	// Load the mort.dat resource
	Common::ErrorCode result = loadMortDat();
	if (result != Common::kNoError)
		return result;

	// Set default EGA palette
	_paletteManager.setDefaultPalette();

	// Setup the mouse cursor
	initMouse();

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
		} else {
			// Unknown section
			f.skip(dataSize);
		}
	}

	f.close();
	return Common::kNoError;
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

/*-------------------------------------------------------------------------*/

Common::Error MortevielleEngine::run() {
	// Initialise the game
	Common::ErrorCode err = initialise();
	if (err != Common::kNoError)
		return err;

	// Dispatch to the game's main routine
	mortevielle_main();

	return Common::kNoError;
}

} // End of namespace Mortevielle
