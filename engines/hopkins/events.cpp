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
	souris_flag = false;
	mouse_linux = false;
	souris_sizex = souris_sizey = 0;
	ofset_souris_x = ofset_souris_y = 0;
	start_x = start_y = 0;
	CASSE = false;
	souris_n = 0;
	souris_bb = 0;
	souris_b = 0;
	pointeur_souris = NULL;
	_gameCounter = 0;
	lItCounter = 0;
	ESC_KEY = false;
	GAME_KEY = KEY_NONE;
	btsouris = 0;
	OLD_ICONE = 0;
	Bufferobjet = NULL;

	_priorCounterTime = 0;
	_priorFrameTime = 0;
}

EventsManager::~EventsManager() {
	_vm->_globals.dos_free2(Bufferobjet);
}

void EventsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

// Install Mouse
void EventsManager::INSTALL_SOURIS() {
	// No implementation in original
}

// Mouse On
void EventsManager::souris_on() {
	souris_flag = true;

	if (mouse_linux) {
		souris_sizex = 52;
		souris_sizey = 32;
	} else {
		souris_sizex = 34;
		souris_sizey = 20;
	}

	ofset_souris_x = 0;
	ofset_souris_y = 0;

	if (!CASSE)
		souris_xy(300, 200);
	else
		souris_xy(150, 100);
}

// Set Mouse position
void EventsManager::souris_xy(int xp, int yp) {
	g_system->warpMouse(xp, yp);
}

// Mouse Max
void EventsManager::souris_max() {
	// No implementation in original
}

// Get Mouse X
int EventsManager::XMOUSE() {
	souris_x = start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_x + ofset_souris_x;
}

// Get Mouse Y
int EventsManager::YMOUSE() {
	souris_x = start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_y + ofset_souris_y;
}

// Get Mouse Button
int EventsManager::BMOUSE() {
	CONTROLE_MES();
	return souris_bb;
}

// Mouse Off
void EventsManager::MOUSE_OFF() {
	souris_flag = false;
	CursorMan.showMouse(false);
}

// Mouse On
void EventsManager::MOUSE_ON() {
	souris_on();
	souris_flag = true;
	CursorMan.showMouse(true);
}

// Change Mouse Cursor
void EventsManager::CHANGE_MOUSE(int id) {
	int cursorId = id;

	if (btsouris != 23) {
		if (id == 4 && btsouris == 4 && _vm->_globals.NOMARCHE)
			cursorId = 0;
		if (cursorId == 25)
			cursorId = 5;
    
		if (OLD_ICONE != cursorId || !cursorId) {
			OLD_ICONE = cursorId;
			souris_n = cursorId;

			updateCursor();
		}
	}
}

// Check Events
void EventsManager::CONTROLE_MES() {
	pollEvents();
}

void EventsManager::checkForNextFrameCounter() {
	// Check for whether to increment the game counter
	uint32 milli = g_system->getMillis();
	while ((milli - _priorCounterTime) >= 10) {
		_priorCounterTime += 10;
		lItCounter += 3;
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
			handleKey(event);
			return;

		case Common::EVENT_LBUTTONDOWN:
			souris_b = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			souris_b = 2;
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			souris_b = 0;
			return;

		default:
 			break;
		}
	}
}

void EventsManager::handleKey(Common::Event &event) {
	ESC_KEY = event.kbd.keycode == Common::KEYCODE_ESCAPE;
	
	if (event.kbd.keycode == Common::KEYCODE_i || event.kbd.keycode == Common::KEYCODE_TAB)
		GAME_KEY = KEY_INVENTORY;
	else if (event.kbd.keycode == Common::KEYCODE_F5)
		GAME_KEY = KEY_SAVE;
	else if (event.kbd.keycode == Common::KEYCODE_F7)
		GAME_KEY = KEY_LOAD;
	else if (event.kbd.keycode == Common::KEYCODE_F1 || event.kbd.keycode == Common::KEYCODE_o)
		GAME_KEY = KEY_OPTIONS;

	// Check for debugger
	if ((event.kbd.keycode == Common::KEYCODE_d) && (event.kbd.flags & Common::KBD_CTRL)) {
		// Attach to the debugger
		_vm->_debugger.attach();
		_vm->_debugger.onFrame();
	}

}

void EventsManager::VBL() {
	int v1;
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7;
	int v10; 
	signed int v11 = 0;
	signed int v12 = 0;
	int v13 = 0; 
	unsigned int v14 = 0; 
	int v15 = 0; 
	int yp = 0;

	if (_vm->_graphicsManager.REDRAW) {
		_vm->_graphicsManager.DD_Lock();
		if (CASSE) {
			_vm->_graphicsManager.CopyAsm(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.REDRAW = 0;
		} else {
			if (_vm->_globals.iRegul == 3)
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 50, 0x280u, 340, 0, 50);
			else
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 20, 0x280u, 440, 0, 20);
			_vm->_fileManager.DMESS();
			--_vm->_graphicsManager.REDRAW;
		}
		_vm->_graphicsManager.DD_Unlock();
	}
	if (souris_flag == true) {
		v1 = 20;
		if (!mouse_linux)
			v1 = 10;
		v2 = 20;
		if (!mouse_linux)
			v2 = 15;
		v15 = souris_x - v1;
		yp = souris_y;
		v14 = souris_sizex;
		v13 = souris_sizey;
		if (btsouris == 23) {
			v14 = _vm->_globals.OBJL;
			v13 = _vm->_globals.OBJH;
			goto LABEL_35;
		}
		if (CASSE) {
			if (v15 < _vm->_graphicsManager.min_x)
				v15 = _vm->_graphicsManager.min_x;
			if (souris_y < _vm->_graphicsManager.min_y)
				yp = _vm->_graphicsManager.min_y;
			if (souris_sizex + v15 >= _vm->_graphicsManager.max_x)
				v14 = souris_sizex - (souris_sizex + v15 - _vm->_graphicsManager.max_x);
			if (yp + souris_sizey < _vm->_graphicsManager.max_y)
				goto LABEL_34;
			v3 = yp + souris_sizey - _vm->_graphicsManager.max_y;
		} else {
			if (v15 < _vm->_graphicsManager.min_x)
				v15 = _vm->_graphicsManager.min_x - v1;
			v2 = (int16)v2;
			if (souris_y < _vm->_graphicsManager.min_y - (int16)v2)
				yp = _vm->_graphicsManager.min_y - (int16)v2;
			if (souris_sizex + v15 >= _vm->_graphicsManager.max_x)
				v14 = souris_sizex - (souris_sizex + v15 - _vm->_graphicsManager.max_x - v1);
			if (yp + souris_sizey < v2 + _vm->_graphicsManager.max_y)
				goto LABEL_34;
			v3 = v2 + yp + souris_sizey - _vm->_graphicsManager.max_y;
		}
		v13 = souris_sizey - v3;
LABEL_34:
		v12 = v14 + v15;
		v11 = yp + v13;
	}
LABEL_35:
	if (!_vm->_globals.PUBEXIT)
		_vm->_objectsManager.AFF_SPRITES();
	if (souris_flag != true)
		goto LABEL_54;
	if (btsouris == 23)
		goto LABEL_45;
	if (yp >= _vm->_graphicsManager.max_y || v15 >= _vm->_graphicsManager.max_x || v14 <= 1 || v13 <= 1) {
		if (btsouris != 23)
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
	_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, yp, v12, v11);
LABEL_54:
	_vm->_globals.vitesse = 2;
	do {
		while (!_vm->shouldQuit()) {
			checkForNextFrameCounter();

			while (CASSE || _vm->_globals.iRegul != 1) {
				if (CASSE != true)
					goto LABEL_63;
				if (lItCounter > 1)
					goto LABEL_65;
			}
			if (_vm->_globals.vitesse != 2)
				break;
			if (lItCounter > 9)
				goto LABEL_65;
		}
LABEL_63:
		;
	} while (!_vm->shouldQuit() && _vm->_globals.iRegul == 3 && lItCounter <= 15);
LABEL_65:
	_vm->_globals.vitesse = 2;
	lItCounter = 0;
	if (_vm->_graphicsManager.DOUBLE_ECRAN != true || _vm->_graphicsManager.no_scroll == 1) {
		_vm->_graphicsManager.Affiche_Segment_Vesa();
	} else {
		if (_vm->_graphicsManager.no_scroll != 2) {
			if (XMOUSE() > _vm->_graphicsManager.SCROLL + 620)
				_vm->_graphicsManager.SCROLL += _vm->_graphicsManager.SPEED_SCROLL;
			if (XMOUSE() < _vm->_graphicsManager.SCROLL + 10)
				_vm->_graphicsManager.SCROLL -= _vm->_graphicsManager.SPEED_SCROLL;
		}
		if (_vm->_graphicsManager.SCROLL < 0)
			_vm->_graphicsManager.SCROLL = 0;
		if (_vm->_graphicsManager.SCROLL > 640)
			_vm->_graphicsManager.SCROLL = 640;
		if (_vm->_graphicsManager.SDL_ECHELLE)
			v4 = _vm->_graphicsManager.Magic_Number(_vm->_graphicsManager.SCROLL);
		else
			v4 = _vm->_graphicsManager.SCROLL;
		if (_vm->_graphicsManager.OLD_SCROLL == v4) {
			_vm->_graphicsManager.Affiche_Segment_Vesa();
		} else {
			_vm->_fontManager.TEXTE_OFF(9);
			_vm->_graphicsManager.DD_Lock();
			if (_vm->_graphicsManager.SDL_ECHELLE) {
				if (_vm->_graphicsManager.Winbpp == 2) {
					v5 = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, v5);
				} else {
					v6 = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, v6);
				}
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0].left = _vm->_graphicsManager.Reel_Zoom(0, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].top = _vm->_graphicsManager.Reel_Zoom(0x14u, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].setWidth(_vm->_graphicsManager.Reel_Zoom(0x280u, _vm->_graphicsManager.SDL_ECHELLE));
				_vm->_graphicsManager.dstrect[0].setHeight(_vm->_graphicsManager.Reel_Zoom(0x1B8u, _vm->_graphicsManager.SDL_ECHELLE));
			} else {
				if (_vm->_graphicsManager.Winbpp == 2)
					_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				else
					_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0] = Common::Rect(0, 20, 640, 460);
			}
			if (!_vm->_globals.BPP_NOAFF) {
//				SDL_UpdateRects(LinuxScr, 1, dstrect);
			}
			if (_vm->_globals.NBBLOC) {
				v7 = 1;
				v10 = _vm->_globals.NBBLOC + 1;
				do {
					if (_vm->_globals.BLOC[v7].field0 == 1)
						_vm->_globals.BLOC[v7].field0 = 0;
					++v7;
				} while (v10 != v7);
			}
			_vm->_globals.NBBLOC = 0;
			start_x = v4;
			_vm->_graphicsManager.ofscroll = v4;
			_vm->_graphicsManager.SCROLL = v4;
		}
		_vm->_graphicsManager.OLD_SCROLL = v4;
		start_x = v4;
		_vm->_graphicsManager.ofscroll = v4;
	}
	souris_bb = souris_b;
	souris_b = 0;
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
	_vm->_soundManager.VERIF_SOUND();
	CONTROLE_MES();
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

	if (btsouris != 23) {
		// Draw standard cursor
		_vm->_graphicsManager.Sprite_Vesa(cursorSurface, pointeur_souris, 300, 300, souris_n);
	} else {
		// Draw the active inventory object
		_vm->_graphicsManager.Affiche_Perfect(cursorSurface, Bufferobjet, 300, 300, 0, 0, 0, 0);
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
	int xOffset = !mouse_linux ? 10 : 20;

	// Set the ScummVM cursor from the surface
	Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
	CursorMan.replaceCursor(cursorPixels, _vm->_globals.OBJL, _vm->_globals.OBJH,
		xOffset, 0, 0, true, &pixelFormat);

	// Delete the cursor surface 
	delete[] cursorPixels;
	delete[] cursorSurface;
}

} // End of namespace Hopkins
