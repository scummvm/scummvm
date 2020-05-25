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

#include "hopkins/events.h"

#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"

namespace Hopkins {

EventsManager::EventsManager(HopkinsEngine *vm) {
	_vm = vm;
	_mouseFl = false;
	_mouseLinuxFl = false;
	_mouseSizeX = _mouseSizeY = 0;
	_mouseOffset.x = _mouseOffset.y = 0;
	_startPos.x = _startPos.y = 0;
	_breakoutFl = false;
	_mouseSpriteId = 0;
	_curMouseButton = 0;
	_mouseButton = 0;
	_mouseCursor = NULL;
	_gameCounter = 0;
	_rateCounter = 0;
	_escKeyFl = false;
	_gameKey = KEY_NONE;
	_mouseCursorId = 0;
	_oldIconId = 0;
	_objectBuf = NULL;

	Common::fill(&_keyState[0], &_keyState[256], false);
	_priorCounterTime = _priorFrameTime = g_system->getMillis();
}

EventsManager::~EventsManager() {
	_vm->_globals->freeMemory(_objectBuf);
	_vm->_globals->freeMemory(_mouseCursor);
}

void EventsManager::clearAll() {
	_vm->_globals->freeMemory(_objectBuf);
	_objectBuf = _vm->_globals->allocMemory(2500);
}

void EventsManager::initMouseData() {
	if (_vm->getPlatform() == Common::kPlatformLinux)
		_mouseLinuxFl = true;
	else
		_mouseLinuxFl = false;

	if (_mouseLinuxFl) {
		_mouseSizeX = 52;
		_mouseSizeY = 32;
	} else {
		_mouseSizeX = 34;
		_mouseSizeY = 20;
	}

	switch (_vm->_globals->_language) {
	case LANG_EN:
		if (!_mouseLinuxFl)
			_mouseCursor = _vm->_fileIO->loadFile("SOUAN.SPR");
		else
			_mouseCursor = _vm->_fileIO->loadFile("LSOUAN.SPR");
		break;
	case LANG_FR:
		if (!_mouseLinuxFl)
			_mouseCursor = _vm->_fileIO->loadFile("SOUFR.SPR");
		else
			_mouseCursor = _vm->_fileIO->loadFile("LSOUFR.SPR");
		break;
	case LANG_SP:
		_mouseCursor = _vm->_fileIO->loadFile("SOUES.SPR");
		break;
	default:
		break;
	}
}

// Mouse On
void EventsManager::setMouseOn() {
	_mouseFl = true;

	if (_mouseLinuxFl) {
		_mouseSizeX = 52;
		_mouseSizeY = 32;
	} else {
		_mouseSizeX = 34;
		_mouseSizeY = 20;
	}

	_mouseOffset.x = 0;
	_mouseOffset.y = 0;

	if (!_breakoutFl)
		setMouseXY(300, 200);
	else
		setMouseXY(150, 100);
}

/**
 * Set Mouse position
 */
void EventsManager::setMouseXY(Common::Point pos) {
	g_system->warpMouse(pos.x, pos.y);
}

/**
 * Set Mouse position
 */
void EventsManager::setMouseXY(int xp, int yp) {
	g_system->warpMouse(xp, yp);
}

/**
 * Get Mouse X
 */
int EventsManager::getMouseX() {
	_mousePos.x = _startPos.x + g_system->getEventManager()->getMousePos().x;
	_mousePos.y = g_system->getEventManager()->getMousePos().y;

	return _mousePos.x + _mouseOffset.x;
}

/**
 * Get Mouse Y
 */
int EventsManager::getMouseY() {
	_mousePos.x = _startPos.x + g_system->getEventManager()->getMousePos().x;
	_mousePos.y = g_system->getEventManager()->getMousePos().y;

	return _mousePos.y + _mouseOffset.y;
}

/**
 * Get Mouse Button
 */
int EventsManager::getMouseButton() {
	refreshEvents();
	return _curMouseButton;
}

/**
 * Mouse Off
 */
void EventsManager::mouseOff() {
	_mouseFl = false;
	CursorMan.showMouse(false);
}

/**
 * Mouse On
 */
void EventsManager::mouseOn() {
	setMouseOn();
	_mouseFl = true;
	CursorMan.showMouse(true);
}

/**
 * Change Mouse Cursor
 */
void EventsManager::changeMouseCursor(int id) {
	int cursorId = id;

	if (_mouseCursorId == 23)
		return;

	if (id == 4 && _mouseCursorId == 4 && _vm->_globals->_freezeCharacterFl)
		cursorId = 0;
	if (cursorId == 25)
		cursorId = 5;

	if (_oldIconId != cursorId || !cursorId) {
		_oldIconId = cursorId;
		_mouseSpriteId = cursorId;

		updateCursor();
	}
}

/**
 * Check Events
 */
void EventsManager::refreshEvents() {
	_vm->_soundMan->checkSounds();

	pollEvents();
}

void EventsManager::checkForNextFrameCounter() {
	int32 delayAmount = 10 - (g_system->getMillis() - _priorCounterTime);
	if (delayAmount > 0)
		_vm->_system->delayMillis(delayAmount);

	// Check for whether to increment the game counter
	uint32 milli = g_system->getMillis();
	while ((milli - _priorCounterTime) >= 10) {
		_priorCounterTime += 10;
		_rateCounter += 3;
	}

	// Check for next game frame
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_gameCounter;
		_priorFrameTime = milli;
		_vm->_graphicsMan->updateScreen();
	}
}

void EventsManager::delay(int totalMilli) {
	uint32 delayEnd = g_system->getMillis() + totalMilli;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
		g_system->delayMillis(10);
	}
}

void EventsManager::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		// Handle keypress
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_KEYDOWN:
			_keyState[(byte)toupper(event.kbd.ascii)] = true;
			handleKey(event);
			return;
		case Common::EVENT_KEYUP:
			_keyState[(byte)toupper(event.kbd.ascii)] = false;
			return;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButton = 1;
			return;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButton = 2;
			return;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			_mouseButton = 0;
			return;
		default:
			break;
		}
	}

	for (char chr = 'A'; chr <= 'Z'; chr++)
		_keyState[(byte)chr] = false;

	for (char chr = '0'; chr <= '9'; chr++)
		_keyState[(byte)chr] = false;
}

void EventsManager::handleKey(const Common::Event &event) {
	_escKeyFl = (event.kbd.keycode == Common::KEYCODE_ESCAPE);

	if (event.kbd.keycode == Common::KEYCODE_i || event.kbd.keycode == Common::KEYCODE_TAB)
		_gameKey = KEY_INVENTORY;
	else if (event.kbd.keycode == Common::KEYCODE_F5)
		_gameKey = KEY_SAVE;
	else if (event.kbd.keycode == Common::KEYCODE_F7)
		_gameKey = KEY_LOAD;
	else if (event.kbd.keycode == Common::KEYCODE_F1 || event.kbd.keycode == Common::KEYCODE_o)
		_gameKey = KEY_OPTIONS;
}

/**
 * Waits for a keypress, ignoring mouse events
 * @return		Keypress, or -1 if game quit was requested
 */
int EventsManager::waitKeyPress() {
	char foundChar = '\0';

	while (!foundChar) {
		if (_vm->shouldQuit())
			return -1;

		for (char ch = 'A'; ch <= 'Z'; ++ch) {
			if (_keyState[(byte)ch]) {
				foundChar = ch;
				break;
			}
		}

		for (char ch = '0'; ch <= '9'; ++ch) {
			if (_keyState[(byte)ch]) {
				foundChar = ch;
				break;
			}
		}

		if (_keyState[(byte)'.'])
			foundChar = '.';
		else if (_keyState[8])
			// BACKSPACE
			foundChar = 8;
		else if (_keyState[13])
			// ENTER
			foundChar = 13;
		else if (_keyState[(byte)' '])
			foundChar = ' ';

		refreshScreenAndEvents();
	}

	// Wait for keypress release
	while (_keyState[(byte)foundChar] && !_vm->shouldQuit()) {
		refreshScreenAndEvents();
		g_system->delayMillis(10);
	}

	// Return character
	return foundChar;
}

void EventsManager::refreshScreenAndEvents() {
	int bottom = 0;
	int right = 0;
	int height = 0;
	int width = 0;
	int xp = 0;
	int yp = 0;

	if (_mouseFl) {
		int mouseWidth = 20;
		if (!_mouseLinuxFl)
			mouseWidth = 10;
		int mouseHeight = 20;
		if (!_mouseLinuxFl)
			mouseHeight = 15;
		xp = _mousePos.x - mouseWidth;
		yp = _mousePos.y;
		width = _mouseSizeX;
		height = _mouseSizeY;
		if (_mouseCursorId == 23) {
			width = _vm->_objectsMan->getObjectWidth();
			height = _vm->_objectsMan->getObjectHeight();
		} else {
			if (_breakoutFl) {
				if (xp < _vm->_graphicsMan->_minX)
					xp = _vm->_graphicsMan->_minX;
				if (_mousePos.y < _vm->_graphicsMan->_minY)
					yp = _vm->_graphicsMan->_minY;
				if (_mouseSizeX + xp >= _vm->_graphicsMan->_maxX)
					width = _mouseSizeX - (_mouseSizeX + xp - _vm->_graphicsMan->_maxX);
				if (yp + _mouseSizeY >= _vm->_graphicsMan->_maxY)
					height = _vm->_graphicsMan->_maxY - yp;
			} else {
				if (xp < _vm->_graphicsMan->_minX)
					xp = _vm->_graphicsMan->_minX - mouseWidth;
				mouseHeight = (int16)mouseHeight;
				if (_mousePos.y < _vm->_graphicsMan->_minY - mouseHeight)
					yp = _vm->_graphicsMan->_minY - mouseHeight;
				if (_mouseSizeX + xp >= _vm->_graphicsMan->_maxX)
					width = _mouseSizeX - (_mouseSizeX + xp - _vm->_graphicsMan->_maxX - mouseWidth);
				if (yp + _mouseSizeY >= mouseHeight + _vm->_graphicsMan->_maxY)
					height = _vm->_graphicsMan->_maxY - mouseHeight - yp;
			}
			right = xp + width;
			bottom = yp + height;
		}
	}

	if (!_vm->_globals->_linuxEndDemoFl)
		_vm->_objectsMan->displaySprite();
	if (!_mouseFl) {
		updateCursor();
	} else if (_mouseCursorId == 23) {
		if (yp < _vm->_graphicsMan->_maxY && xp < _vm->_graphicsMan->_maxX) {
			if (width + xp > _vm->_graphicsMan->_maxX)
				width = _vm->_graphicsMan->_maxX - xp;
			if (yp + height > _vm->_graphicsMan->_maxY)
				height = _vm->_graphicsMan->_maxY - yp;
			if (width > 1 && height > 1) {
				updateCursor();
			}
		}
	} else if (yp < _vm->_graphicsMan->_maxY && xp < _vm->_graphicsMan->_maxX && width > 1 && height > 1) {
		updateCursor();
		_vm->_graphicsMan->addDirtyRect(xp, yp, right, bottom);
	}

	_vm->_globals->_speed = 2;
	bool externalLoopFl = false;
	do {
		while (!_vm->shouldQuit()) {
			checkForNextFrameCounter();
			bool innerLoopFl = false;

			while (!_vm->shouldQuit() && (_breakoutFl || _vm->_globals->_eventMode != EVENTMODE_IGNORE)) {
				checkForNextFrameCounter();

				if (!_breakoutFl) {
					innerLoopFl = true;
					break;
				}
				if (_rateCounter > 1) {
					externalLoopFl = true;
					break;
				}
			}
			if (innerLoopFl || _vm->_globals->_speed != 2)
				break;
			if (externalLoopFl ||_rateCounter > 9) {
				externalLoopFl = true;
				break;
			}
		}
		if (externalLoopFl)
			break;
	} while (!_vm->shouldQuit() && _vm->_globals->_eventMode == EVENTMODE_CREDITS && _rateCounter <= 15);
	_vm->_globals->_speed = 2;
	_rateCounter = 0;
	if (!_vm->_graphicsMan->_largeScreenFl || _vm->_graphicsMan->_scrollStatus == 1) {
		_vm->_graphicsMan->displayDirtyRects();
	} else {
		if (_vm->_graphicsMan->_scrollStatus != 2) {
			if (getMouseX() > _vm->_graphicsMan->_scrollPosX + 620)
				_vm->_graphicsMan->_scrollPosX += _vm->_graphicsMan->_scrollSpeed;
			if (getMouseX() < _vm->_graphicsMan->_scrollPosX + 10)
				_vm->_graphicsMan->_scrollPosX -= _vm->_graphicsMan->_scrollSpeed;
		}
		_vm->_graphicsMan->_scrollPosX = CLIP(_vm->_graphicsMan->_scrollPosX, 0, SCREEN_WIDTH);
		if (_vm->_graphicsMan->_oldScrollPosX == _vm->_graphicsMan->_scrollPosX) {
			_vm->_graphicsMan->displayDirtyRects();
		} else {
			_vm->_fontMan->hideText(9);
			_vm->_graphicsMan->display8BitRect(_vm->_graphicsMan->_frontBuffer, _vm->_graphicsMan->_scrollPosX, 20, SCREEN_WIDTH, 440, 0, 20);
			_vm->_graphicsMan->resetRefreshRects();
			_vm->_graphicsMan->addRefreshRect(0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 20);

			_vm->_graphicsMan->resetDirtyRects();

			_startPos.x = _vm->_graphicsMan->_scrollPosX;
			_vm->_graphicsMan->_scrollOffset = _vm->_graphicsMan->_scrollPosX;
		}
		_vm->_graphicsMan->_oldScrollPosX = _vm->_graphicsMan->_scrollPosX;
		_startPos.x = _vm->_graphicsMan->_scrollPosX;
		_vm->_graphicsMan->_scrollOffset = _vm->_graphicsMan->_scrollPosX;
	}
	_curMouseButton = _mouseButton;
	_mouseButton = 0;
	_vm->_soundMan->checkSoundEnd();
	refreshEvents();
}

void EventsManager::updateCursor() {
	// Backup the current sprite clipping bounds and reset them
	Common::Rect clipBounds(_vm->_graphicsMan->_minX, _vm->_graphicsMan->_minY,
		_vm->_graphicsMan->_maxX, _vm->_graphicsMan->_maxY);
	_vm->_graphicsMan->_minX = _vm->_graphicsMan->_minY = 0;
	_vm->_graphicsMan->_maxX = _vm->_objectsMan->getObjectWidth();
	_vm->_graphicsMan->_maxY = _vm->_objectsMan->getObjectHeight();
	int pitch = _vm->_graphicsMan->_lineNbr2;
	_vm->_graphicsMan->_lineNbr2 = _vm->_objectsMan->getObjectWidth();

	// Create the temporary cursor surface
	byte *cursorSurface = new byte[_vm->_objectsMan->getObjectHeight() * _vm->_objectsMan->getObjectWidth()];
	Common::fill(cursorSurface, cursorSurface + _vm->_objectsMan->getObjectHeight() * _vm->_objectsMan->getObjectWidth(), 0);

	if (_mouseCursorId != 23) {
		// Draw standard cursor
		_vm->_graphicsMan->drawVesaSprite(cursorSurface, _mouseCursor, 300, 300, _mouseSpriteId);
	} else {
		// Draw the active inventory object
		_vm->_graphicsMan->drawCompressedSprite(cursorSurface, _objectBuf, 300, 300, 0, 0, 0, false);
	}

	// Reset the clipping bounds
	_vm->_graphicsMan->_minX = clipBounds.left;
	_vm->_graphicsMan->_minY = clipBounds.top;
	_vm->_graphicsMan->_maxX = clipBounds.right;
	_vm->_graphicsMan->_maxY = clipBounds.bottom;
	_vm->_graphicsMan->_lineNbr2 = pitch;

	// Create a cursor palette
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();

	byte *cursorPalette = new byte[3 * PALETTE_SIZE];
	uint16 *paletteColors = (uint16 *)_vm->_graphicsMan->_palettePixels;

	for (int i = 0; i < PALETTE_SIZE; i++) {
		uint8 r, g, b;
		pixelFormat.colorToRGB(READ_LE_UINT16(&paletteColors[i]), r, g, b);
		cursorPalette[3 * i] = r;
		cursorPalette[3 * i + 1] = g;
		cursorPalette[3 * i + 2] = b;
	}

	// Calculate the X offset within the pointer image to the actual cursor data
	int xOffset = !_mouseLinuxFl ? 10 : 20;

	// Set the ScummVM cursor from the surface
	CursorMan.replaceCursorPalette(cursorPalette, 0, PALETTE_SIZE - 1);
	CursorMan.replaceCursor(cursorSurface, _vm->_objectsMan->getObjectWidth(), _vm->_objectsMan->getObjectHeight(),
		xOffset, 0, 0, true);

	// Delete the cursor surface and palette
	delete[] cursorPalette;
	delete[] cursorSurface;
}

} // End of namespace Hopkins
