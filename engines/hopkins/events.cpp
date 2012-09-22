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
#include "common/events.h"
#include "common/textconsole.h"
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
	lItCounter = 0;
	ESC_KEY = false;
	_priorFrameTime = 0;
	btsouris = 0;
}

void EventsManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void EventsManager::INSTALL_SOURIS() {
	// No implementation in original
}

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

void EventsManager::souris_xy(int xp, int yp) {
	g_system->warpMouse(xp, yp);
}

void EventsManager::souris_max() {
	// No implementation in original
}

int EventsManager::XMOUSE() {
	souris_x = start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_x + ofset_souris_x;
}

int EventsManager::YMOUSE() {
	souris_x = start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_x + ofset_souris_x;
}

bool EventsManager::BMOUSE() {
	CONTROLE_MES();
	return souris_bb;
}

void EventsManager::MOUSE_OFF() {
	souris_flag = false;
	g_system->showMouse(false);
}

void EventsManager::MOUSE_ON() {
	souris_on();
	souris_flag = true;
	g_system->showMouse(true);
}

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

			// Backup the current sprite clipping bounds and reset them
			Common::Rect clipBounds(_vm->_graphicsManager.min_x, _vm->_graphicsManager.min_y,
				_vm->_graphicsManager.max_x, _vm->_graphicsManager.max_y);
			_vm->_graphicsManager.min_x = _vm->_graphicsManager.min_y = 0;
			_vm->_graphicsManager.max_x = _vm->_globals.OBJL;
			_vm->_graphicsManager.max_y = _vm->_globals.OBJH;
			int pitch = _vm->_graphicsManager.nbrligne2;
			_vm->_graphicsManager.nbrligne2 = _vm->_globals.OBJL;

			// Draw the cursor onto a temporary surface
			byte *cursorSurface = new byte[_vm->_globals.OBJH * _vm->_globals.OBJL];
			Common::fill(cursorSurface, cursorSurface + _vm->_globals.OBJH * _vm->_globals.OBJL, 0);
			_vm->_graphicsManager.Sprite_Vesa(cursorSurface, pointeur_souris, 300, 300, cursorId);

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

			// Set the ScummVM cursor from the surface
			Graphics::PixelFormat pixelFormat = g_system->getScreenFormat();
			g_system->setMouseCursor(cursorPixels, _vm->_globals.OBJL, _vm->_globals.OBJH,
				0, 0, 0, true, &pixelFormat);

			// Delete the cursor surface 
			delete[] cursorPixels;
			delete[] cursorSurface;
		}
	}
}

void EventsManager::CONTROLE_MES() {
	pollEvents();
}

void EventsManager::checkForNextFrameCounter() {
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		_priorFrameTime = milli;
		++lItCounter;

		g_system->updateScreen();
	}
}

void EventsManager::delay(int delay) {
	uint32 delayEnd = g_system->getMillis() + delay;

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
			ESC_KEY = event.kbd.keycode == Common::KEYCODE_ESCAPE;
			return;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:

			return;

		default:
 			break;
		}
	}
}

} // End of namespace Hopkins
