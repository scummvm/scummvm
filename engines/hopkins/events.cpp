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

#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "hopkins/events.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/hopkins.h"
#include "hopkins/sound.h"

namespace Hopkins {

EventsManager::EventsManager() {
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
	_priorCounterTime = 0;
	_priorFrameTime = 0;
}

EventsManager::~EventsManager() {
	_vm->_globals.freeMemory(_objectBuf);
	_vm->_globals.freeMemory(_mouseCursor);
}

void EventsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
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

	if (_mouseCursorId != 23) {
		if (id == 4 && _mouseCursorId == 4 && _vm->_globals.NOMARCHE)
			cursorId = 0;
		if (cursorId == 25)
			cursorId = 5;
    
		if (_oldIconId != cursorId || !cursorId) {
			_oldIconId = cursorId;
			_mouseSpriteId = cursorId;

			updateCursor();
		}
	}
}

/**
 * Check Events
 */
void EventsManager::refreshEvents() {
	_vm->_soundManager.checkSounds();

	pollEvents();
}

void EventsManager::checkForNextFrameCounter() {
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
		g_system->updateScreen();

		// Signal the ScummVM debugger
		_vm->_debugger.onFrame();
	}
}

void EventsManager::delay(int totalMilli) {
	uint32 delayEnd = g_system->getMillis() + totalMilli;

	while (!g_system->getEventManager()->shouldQuit() && g_system->getMillis() < delayEnd) {
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
		case Common::EVENT_RTL:
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

void EventsManager::handleKey(Common::Event &event) {
	_escKeyFl = (event.kbd.keycode == Common::KEYCODE_ESCAPE);
	
	if (event.kbd.keycode == Common::KEYCODE_i || event.kbd.keycode == Common::KEYCODE_TAB)
		_gameKey = KEY_INVENTORY;
	else if (event.kbd.keycode == Common::KEYCODE_F5)
		_gameKey = KEY_SAVE;
	else if (event.kbd.keycode == Common::KEYCODE_F7)
		_gameKey = KEY_LOAD;
	else if (event.kbd.keycode == Common::KEYCODE_F1 || event.kbd.keycode == Common::KEYCODE_o)
		_gameKey = KEY_OPTIONS;

	// Check for debugger
	if ((event.kbd.keycode == Common::KEYCODE_d) && (event.kbd.flags & Common::KBD_CTRL)) {
		// Attach to the debugger
		_vm->_debugger.attach();
		_vm->_debugger.onFrame();
	}

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

		VBL();
	}

	// Wait for keypress release
	while (_keyState[(byte)foundChar] && !_vm->shouldQuit()) {
		VBL();
		g_system->delayMillis(10);
	}

	// Return character
	return foundChar;
}

void EventsManager::VBL() {
	int v1;
	int v2; 
	int v3; 
	signed int v11 = 0;
	signed int v12 = 0;
	int v13 = 0; 
	unsigned int v14 = 0; 
	int v15 = 0; 
	int yp = 0;

	if (_vm->_graphicsManager.REDRAW) {
		_vm->_graphicsManager.DD_Lock();
		if (_breakoutFl) {
			_vm->_graphicsManager.CopyAsm(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.REDRAW = 0;
		} else {
			if (_vm->_globals.iRegul == 3)
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 50, 0x280u, 340, 0, 50);
			else
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 20, 0x280u, 440, 0, 20);
			--_vm->_graphicsManager.REDRAW;
		}
		_vm->_graphicsManager.DD_Unlock();
	}
	if (_mouseFl == true) {
		v1 = 20;
		if (!_mouseLinuxFl)
			v1 = 10;
		v2 = 20;
		if (!_mouseLinuxFl)
			v2 = 15;
		v15 = _mousePos.x - v1;
		yp = _mousePos.y;
		v14 = _mouseSizeX;
		v13 = _mouseSizeY;
		if (_mouseCursorId == 23) {
			v14 = _vm->_globals.OBJL;
			v13 = _vm->_globals.OBJH;
			goto LABEL_35;
		}
		if (_breakoutFl) {
			if (v15 < _vm->_graphicsManager.min_x)
				v15 = _vm->_graphicsManager.min_x;
			if (_mousePos.y < _vm->_graphicsManager.min_y)
				yp = _vm->_graphicsManager.min_y;
			if (_mouseSizeX + v15 >= _vm->_graphicsManager.max_x)
				v14 = _mouseSizeX - (_mouseSizeX + v15 - _vm->_graphicsManager.max_x);
			if (yp + _mouseSizeY < _vm->_graphicsManager.max_y)
				goto LABEL_34;
			v3 = yp + _mouseSizeY - _vm->_graphicsManager.max_y;
		} else {
			if (v15 < _vm->_graphicsManager.min_x)
				v15 = _vm->_graphicsManager.min_x - v1;
			v2 = (int16)v2;
			if (_mousePos.y < _vm->_graphicsManager.min_y - (int16)v2)
				yp = _vm->_graphicsManager.min_y - (int16)v2;
			if (_mouseSizeX + v15 >= _vm->_graphicsManager.max_x)
				v14 = _mouseSizeX - (_mouseSizeX + v15 - _vm->_graphicsManager.max_x - v1);
			if (yp + _mouseSizeY < v2 + _vm->_graphicsManager.max_y)
				goto LABEL_34;
			v3 = v2 + yp + _mouseSizeY - _vm->_graphicsManager.max_y;
		}
		v13 = _mouseSizeY - v3;
LABEL_34:
		v12 = v14 + v15;
		v11 = yp + v13;
	}
LABEL_35:
	if (!_vm->_globals.PUBEXIT)
		_vm->_objectsManager.AFF_SPRITES();
	if (_mouseFl != true) {
		updateCursor();
		goto LABEL_54;
	}
	if (_mouseCursorId == 23)
		goto LABEL_45;
	if (yp >= _vm->_graphicsManager.max_y || v15 >= _vm->_graphicsManager.max_x || v14 <= 1 || v13 <= 1) {
		if (_mouseCursorId != 23)
			goto LABEL_54;
LABEL_45:
		if (yp < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x) {
			if ((signed int)(v14 + v15) > _vm->_graphicsManager.max_x)
				v14 -= v14 + v15 - _vm->_graphicsManager.max_x;
			if (yp + v13 > _vm->_graphicsManager.max_y)
				v13 -= yp + v13 - _vm->_graphicsManager.max_y;
			if (v14 > 1 && v13 > 1) {
				_vm->_eventsManager.updateCursor();
/* Commented out in favour of using ScummVM cursor display
				_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, yp, v14, v13);

				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Bufferobjet, v15 + 300, yp + 300, 0, 0, 0, 0);
				_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, yp, v14 + v15, yp + v13);
*/
			}
		}
		goto LABEL_54;
	}

/* Commented out in favour of using ScummVM cursor display
	_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, yp, v14, v13);
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, pointeur_souris, v15 + 300, yp + 300, souris_n);
*/

	_vm->_eventsManager.updateCursor();
	_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, yp, v12, v11);
LABEL_54:
	_vm->_globals.vitesse = 2;
	do {
		while (!_vm->shouldQuit()) {
			checkForNextFrameCounter();

			while (_breakoutFl || _vm->_globals.iRegul != 1) {
				checkForNextFrameCounter();

				if (_breakoutFl != true)
					goto LABEL_63;
				if (_rateCounter > 1)
					goto LABEL_65;
			}
			if (_vm->_globals.vitesse != 2)
				break;
			if (_rateCounter > 9)
				goto LABEL_65;
		}
LABEL_63:
		;
	} while (!_vm->shouldQuit() && _vm->_globals.iRegul == 3 && _rateCounter <= 15);
LABEL_65:
	_vm->_globals.vitesse = 2;
	_rateCounter = 0;
	if (_vm->_graphicsManager.DOUBLE_ECRAN != true || _vm->_graphicsManager.no_scroll == 1) {
		_vm->_graphicsManager.Affiche_Segment_Vesa();
	} else {
		int v4; 
		if (_vm->_graphicsManager.no_scroll != 2) {
			if (getMouseX() > _vm->_graphicsManager.SCROLL + 620)
				_vm->_graphicsManager.SCROLL += _vm->_graphicsManager.SPEED_SCROLL;
			if (getMouseX() < _vm->_graphicsManager.SCROLL + 10)
				_vm->_graphicsManager.SCROLL -= _vm->_graphicsManager.SPEED_SCROLL;
		}
		if (_vm->_graphicsManager.SCROLL < 0)
			_vm->_graphicsManager.SCROLL = 0;
		if (_vm->_graphicsManager.SCROLL > SCREEN_WIDTH)
			_vm->_graphicsManager.SCROLL = SCREEN_WIDTH;
		if (_vm->_graphicsManager.SDL_ECHELLE)
			v4 = _vm->_graphicsManager.Magic_Number(_vm->_graphicsManager.SCROLL);
		else
			v4 = _vm->_graphicsManager.SCROLL;
		if (_vm->_graphicsManager.OLD_SCROLL == v4) {
			_vm->_graphicsManager.Affiche_Segment_Vesa();
		} else {
			_vm->_fontManager.hideText(9);
			_vm->_graphicsManager.DD_Lock();
			if (_vm->_graphicsManager.SDL_ECHELLE) {
				if (_vm->_graphicsManager.Winbpp == 2) {
					int v5 = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, SCREEN_WIDTH, 440, 0, v5);
				} else {
					int v6 = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, SCREEN_WIDTH, 440, 0, v6);
				}
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0].left = _vm->_graphicsManager.Reel_Zoom(0, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].top = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].setWidth(_vm->_graphicsManager.Reel_Zoom(0x280u, _vm->_graphicsManager.SDL_ECHELLE));
				_vm->_graphicsManager.dstrect[0].setHeight(_vm->_graphicsManager.Reel_Zoom(0x1B8u, _vm->_graphicsManager.SDL_ECHELLE));
			} else {
				if (_vm->_graphicsManager.Winbpp == 2)
					_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, v4, 20, SCREEN_WIDTH, 440, 0, 20);
				else
					_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_BUFFER, v4, 20, SCREEN_WIDTH, 440, 0, 20);
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0] = Common::Rect(0, 20, SCREEN_WIDTH, 460);
			}

			// CHECKME: Useless?
			// if (!_vm->_globals.BPP_NOAFF) {
			//	// SDL_UpdateRects(LinuxScr, 1, dstrect);
			// }

			if (_vm->_globals.NBBLOC) {
				int v10 = _vm->_globals.NBBLOC + 1;
				for (int i = 1; i != v10; i++) {
					if (_vm->_globals.BLOC[i].field0 == 1)
						_vm->_globals.BLOC[i].field0 = 0;
				}
			}
			_vm->_globals.NBBLOC = 0;
			_startPos.x = v4;
			_vm->_graphicsManager.ofscroll = v4;
			_vm->_graphicsManager.SCROLL = v4;
		}
		_vm->_graphicsManager.OLD_SCROLL = v4;
		_startPos.x = v4;
		_vm->_graphicsManager.ofscroll = v4;
	}
	_curMouseButton = _mouseButton;
	_mouseButton = 0;
#if 0
	// Commented by Strangerke. Looks completely useless.

	if (souris_flag == true) {
		if (btsouris != 23) {
			if (yp < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x && v14 > 1 && v13 > 1) {
/* Commented out in favour of using ScummVM cursor display
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, yp, v14, v13);
				_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, yp, v12, v11);
*/
				goto LABEL_113;
			}
			if (btsouris != 23)
				goto LABEL_113;
		}
		if (yp < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x && v14 > 1 && v13 > 1) {
/* Commented out in favour of using ScummVM cursor display
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, yp, v14, v13);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, yp, v14 + v15, yp + v13);
*/
		}
	}
LABEL_113:
#endif
	_vm->_soundManager.VERIF_SOUND();
	refreshEvents();
}	

void EventsManager::updateCursor() {
	// Backup the current sprite clipping bounds and reset them
	Common::Rect clipBounds(_vm->_graphicsManager.min_x, _vm->_graphicsManager.min_y,
		_vm->_graphicsManager.max_x, _vm->_graphicsManager.max_y);
	_vm->_graphicsManager.min_x = _vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_x = _vm->_globals.OBJL;
	_vm->_graphicsManager.max_y = _vm->_globals.OBJH;
	int pitch = _vm->_graphicsManager.nbrligne2;
	_vm->_graphicsManager.nbrligne2 = _vm->_globals.OBJL;

	// Create the temporary cursor surface
	byte *cursorSurface = new byte[_vm->_globals.OBJH * _vm->_globals.OBJL];
	Common::fill(cursorSurface, cursorSurface + _vm->_globals.OBJH * _vm->_globals.OBJL, 0);

	if (_mouseCursorId != 23) {
		// Draw standard cursor
		_vm->_graphicsManager.Sprite_Vesa(cursorSurface, _mouseCursor, 300, 300, _mouseSpriteId);
	} else {
		// Draw the active inventory object
		_vm->_graphicsManager.Affiche_Perfect(cursorSurface, _objectBuf, 300, 300, 0, 0, 0, 0);
	}

	// Reset the clipping bounds
	_vm->_graphicsManager.min_x = clipBounds.left;
	_vm->_graphicsManager.min_y = clipBounds.top;
	_vm->_graphicsManager.max_x = clipBounds.right;
	_vm->_graphicsManager.max_y = clipBounds.bottom;
	_vm->_graphicsManager.nbrligne2 = pitch;
	
	// Convert the cursor to the pixel format. At the moment, it's hardcoded
	// to expect the game to be in 16-bit mode
	uint16 *cursorPixels = new uint16[_vm->_globals.OBJH * _vm->_globals.OBJL];
	const byte *srcP = cursorSurface;
	uint16 *destP = cursorPixels;

	for (int yp = 0; yp < _vm->_globals.OBJH; ++yp) {
		const byte *lineSrcP = srcP;
		uint16 *lineDestP = destP;

		for (int xp = 0; xp < _vm->_globals.OBJL; ++xp)
			*lineDestP++ = *(uint16 *)&_vm->_graphicsManager.PAL_PIXELS[*lineSrcP++ * 2];

		srcP += _vm->_globals.OBJL;
		destP += _vm->_globals.OBJL;
	}

	// Calculate the X offset within the pointer image to the actual cursor data
	int xOffset = !_mouseLinuxFl ? 10 : 20;

	// Set the ScummVM cursor from the surface
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	CursorMan.replaceCursor(cursorPixels, _vm->_globals.OBJL, _vm->_globals.OBJH,
		xOffset, 0, *((uint16 *)cursorPixels), true, &pixelFormat);

	// Delete the cursor surface 
	delete[] cursorPixels;
	delete[] cursorSurface;
}

} // End of namespace Hopkins
