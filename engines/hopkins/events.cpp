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
	_vm->_eventsManager.start_x = start_y = 0;
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
	souris_x = _vm->_eventsManager.start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_x + ofset_souris_x;
}

int EventsManager::YMOUSE() {
	souris_x = _vm->_eventsManager.start_x + g_system->getEventManager()->getMousePos().x;
	souris_y = g_system->getEventManager()->getMousePos().y;

	return souris_y + ofset_souris_y;
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
			ESC_KEY = event.kbd.keycode == Common::KEYCODE_ESCAPE;
			return;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			souris_b = true;
			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			souris_b = false;
			return;

		default:
 			break;
		}
	}
}

void EventsManager::VBL() {
	// Bulk of method currently disabled

	// Move any button press status into the active button status state
	souris_bb = souris_b;
	souris_b = false;

	if (!_vm->_globals.PUBEXIT)
		_vm->_objectsManager.AFF_SPRITES();
/*
	int a1 = 0;
	signed int v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	signed int v7;
	int v10;
	signed int v11 = 0;
	signed int v12 = 0;
	int v13 = 0; 
	unsigned int v14 = 0;
	int v15 = 0;

	if (REDRAW) {
		DD_Lock();
		if (_vm->_eventsManager.CASSE) {
			CopyAsm(VESA_BUFFER);
			REDRAW = 0;
		} else {
			if (_vm->_globals.iRegul == 3)
				m_scroll(VESA_BUFFER, ofscroll, 50, SCREEN_WIDTH, 340, 0, 50);
			else
				m_scroll(VESA_BUFFER, ofscroll, 20, SCREEN_WIDTH, 440, 0, 20);
			FileManager::DMESS();
			--REDRAW;
		}
		DD_Unlock();
	}

	if (_vm->_eventsManager.souris_flag) {
		v1 = 20;
		if (!_vm->_eventsManager.mouse_linux)
			v1 = 10;
		v2 = 20;
		if (!_vm->_eventsManager.mouse_linux)
			v2 = 15;

		v15 = _vm->_eventsManager.souris_x - v1;
		a1 = _vm->_eventsManager.souris_y;
		v14 = _vm->_eventsManager.souris_sizex;
		v13 = _vm->_eventsManager.souris_sizey;
		if (_vm->_eventsManager.btsouris == 23) {
			v14 = _vm->_globals.OBJL;
			v13 = _vm->_globals.OBJH;
			goto LABEL_35;
		}

		if (_vm->_eventsManager.CASSE) {
			if (v15 < min_x)
				v15 = min_x;
			if (_vm->_eventsManager.souris_y < min_y)
				a1 = min_y;
			
			if (_vm->_eventsManager.souris_sizex + v15 >= max_x)
				v14 = _vm->_eventsManager.souris_sizex - (_vm->_eventsManager.souris_sizex + v15 - max_x);
			if (a1 + _vm->_eventsManager.souris_sizey < max_y)
				goto LABEL_34;
			
			v3 = a1 + _vm->_eventsManager.souris_sizey - max_y;
		} else {
			if (v15 < min_x)
				v15 = min_x - v1;
			//v2 = v2;
			if (_vm->_eventsManager.souris_y < min_y - v2)
				a1 = min_y - v2;
			if (_vm->_eventsManager.souris_sizex + v15 >= max_x)
				v14 = _vm->_eventsManager.souris_sizex - (_vm->_eventsManager.souris_sizex + v15 - max_x - v1);
			if (a1 + _vm->_eventsManager.souris_sizey < v2 + max_y)
				goto LABEL_34;
  
			v3 = v2 + a1 + _vm->_eventsManager.souris_sizey - max_y;
		}

		v13 = _vm->_eventsManager.souris_sizey - v3;
LABEL_34:
		v12 = v14 + v15;
		v11 = a1 + v13;
	}
LABEL_35:

	if (!_vm->_globals.PUBEXIT)
		AFF_SPRITES();
	if (_vm->_eventsManager.souris_flag != 1)
		goto LABEL_54;
	if (_vm->_eventsManager.btsouris == 23)
		goto LABEL_45;

	if (a1 >= max_y || v15 >= max_x || (signed int)v14 <= 1 || v13 <= 1) {
		if (_vm->_eventsManager.btsouris != 23)
			goto LABEL_54;

LABEL_45:
		if (a1 < max_y && v15 < max_x) {
			if ((signed int)(v14 + v15) > max_x)
				v14 -= v14 + v15 - max_x;
			if (a1 + v13 > max_y)
				v13 -= a1 + v13 - max_y;
  
			if ((signed int)v14 > 1 && v13 > 1) {
				Capture_Mem(VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
				Affiche_Perfect(VESA_BUFFER, _vm->_globals.Bufferobjet, v15 + 300, a1 + 300, 0, 0, 0, 0);
				Ajoute_Segment_Vesa(v15, a1, v14 + v15, a1 + v13);
			}
		}
		goto LABEL_54;
	}

	Capture_Mem(VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
	Sprite_Vesa(VESA_BUFFER, _vm->_eventsManager.pointeur_souris, v15 + 300, a1 + 300, _vm->_eventsManager.souris_n);
	Ajoute_Segment_Vesa(v15, a1, v12, v11);

LABEL_54:
	_vm->_globals.vitesse = 2;

	do {
		for (;;) {
			// TODO: Figure out the purpose of this loop waiting on lItCounter..
			// maybe it's for cursor animatoin?
			_vm->_eventsManager.delay(10);

			while (_vm->_eventsManager.CASSE || _vm->_globals.iRegul != 1) {
				if (_vm->_eventsManager.CASSE != 1)
					goto LABEL_63;
    
				if (_vm->_eventsManager.lItCounter > 1)
					goto LABEL_65;
			}
  
			if (_vm->_globals.vitesse != 2)
				break;
  
			if (_vm->_eventsManager.lItCounter > 9)
				goto LABEL_65;
		}
LABEL_63:
		;
	} while (_vm->_globals.iRegul == 3 && _vm->_eventsManager.lItCounter <= 15);

LABEL_65:
	_vm->_globals.vitesse = 2;
	_vm->_eventsManager.lItCounter = 0;

	if (DOUBLE_ECRAN != 1 || no_scroll == 1) {
		Affiche_Segment_Vesa();
	} else {
		if (no_scroll != 2) {
			if (_vm->_eventsManager.XMOUSE() > SCROLL + 620)
				SCROLL += SPEED_SCROLL;
  
			if (_vm->_eventsManager.XMOUSE() < SCROLL + 10)
				SCROLL -= SPEED_SCROLL;
		}

		SCROLL = CLIP(SCROLL, 0, SCREEN_WIDTH);

		if (SDL_ECHELLE)
			v4 = Magic_Number(SCROLL);
		else
			v4 = SCROLL;
	
		if (OLD_SCROLL == v4) {
			Affiche_Segment_Vesa();
		} else {
			_vm->_fontManager.TEXTE_OFF(9);
			DD_Lock();
			if (SDL_ECHELLE) {
				if (Winbpp == 2) {
					v5 = Reel_Zoom(20, SDL_ECHELLE);
					m_scroll16A(VESA_BUFFER, v4, 20, 640, 440, 0, v5);
				} else {
					v6 = Reel_Zoom(20, SDL_ECHELLE);
					m_scroll2A(VESA_BUFFER, v4, 20, 640, 440, 0, v6);
				}
			
				DD_Unlock();
				dstrect[0].left = Reel_Zoom(0, SDL_ECHELLE);
				dstrect[0].top = Reel_Zoom(20, SDL_ECHELLE);
				dstrect[0].setWidth(Reel_Zoom(SCREEN_WIDTH, SDL_ECHELLE));
				dstrect[0].setHeight(Reel_Zoom(440, SDL_ECHELLE));
			} else {
				if (Winbpp == 2)
					m_scroll16(VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				else
					m_scroll2(VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				
				DD_Unlock();
				dstrect[0] = Common::Rect(0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 40);
			}
  
			if (!_vm->_globals.BPP_NOAFF) {
				// TODO: Useful for future dirty rect processing?
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
			_vm->_eventsManager.start_x = v4;
			ofscroll = v4;
			SCROLL = v4;
		}
	
		OLD_SCROLL = v4;
		_vm->_eventsManager.start_x = v4;
		ofscroll = v4;
	}
  
	_vm->_eventsManager.souris_bb = _vm->_eventsManager.souris_b;
	_vm->_eventsManager.souris_b = 0;
	if (_vm->_eventsManager.souris_flag == 1) {
		if (_vm->_eventsManager.btsouris != 23) {
			if (a1 < max_y && v15 < max_x && v14 > 1 && v13 > 1) {
				Restore_Mem(VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
				Ajoute_Segment_Vesa(v15, a1, v12, v11);
				goto LABEL_113;
			}
  
			if (_vm->_eventsManager.btsouris != 23)
				goto LABEL_113;
		}
	    
		if (a1 < max_y && v15 < max_x && v14 > 1 && v13 > 1) {
			Restore_Mem(VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
			Ajoute_Segment_Vesa(v15, a1, v14 + v15, a1 + v13);
		}
	}
		
LABEL_113:
 */
	_vm->_soundManager.VERIF_SOUND();
	return _vm->_eventsManager.CONTROLE_MES();
}	

} // End of namespace Hopkins
