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
}

void EventsManager::MOUSE_ON() {
	souris_on();
	souris_flag = true;
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

void EventsManager::VBL() {
	int a1 = 0;
	signed __int16 v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	signed __int16 v7;
	int v10;
	signed int v11 = 0;
	signed int v12 = 0;
	int v13 = 0; 
	unsigned int v14 = 0;
	int v15 = 0;

	if (_vm->_graphicsManager.REDRAW) {
		_vm->_graphicsManager.DD_Lock();
		if (CASSE) {
			_vm->_graphicsManager.CopyAsm(_vm->_graphicsManager.VESA_BUFFER);
			_vm->_graphicsManager.REDRAW = 0;
		} else {
			if (_vm->_globals.iRegul == 3)
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 50, SCREEN_WIDTH, 340, 0, 50);
			else
				_vm->_graphicsManager.m_scroll(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.ofscroll, 20, SCREEN_WIDTH, 440, 0, 20);
			FileManager::DMESS();
			--_vm->_graphicsManager.REDRAW;
		}
		_vm->_graphicsManager.DD_Unlock();
	}

	if (souris_flag) {
		v1 = 20;
		if (!mouse_linux)
			v1 = 10;
		v2 = 20;
		if (!mouse_linux)
			v2 = 15;

		v15 = souris_x - v1;
		a1 = souris_y;
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
				a1 = _vm->_graphicsManager.min_y;
			
			if (souris_sizex + v15 >= _vm->_graphicsManager.max_x)
				v14 = souris_sizex - (souris_sizex + v15 - _vm->_graphicsManager.max_x);
			if (a1 + souris_sizey < _vm->_graphicsManager.max_y)
				goto LABEL_34;
			
			v3 = a1 + souris_sizey - _vm->_graphicsManager.max_y;
		} else {
			if (v15 < _vm->_graphicsManager.min_x)
				v15 = _vm->_graphicsManager.min_x - v1;
			v2 = (signed __int16)v2;
			if (souris_y < _vm->_graphicsManager.min_y - (signed __int16)v2)
				a1 = _vm->_graphicsManager.min_y - (signed __int16)v2;
			if (souris_sizex + v15 >= _vm->_graphicsManager.max_x)
				v14 = souris_sizex - (souris_sizex + v15 - _vm->_graphicsManager.max_x - v1);
			if (a1 + souris_sizey < v2 + _vm->_graphicsManager.max_y)
				goto LABEL_34;
  
			v3 = v2 + a1 + souris_sizey - _vm->_graphicsManager.max_y;
		}

		v13 = souris_sizey - v3;
LABEL_34:
		v12 = v14 + v15;
		v11 = a1 + v13;
	}
LABEL_35:

	if (!_vm->_globals.PUBEXIT)
		_vm->_graphicsManager.AFF_SPRITES();
	if (souris_flag != 1)
		goto LABEL_54;
	if (btsouris == 23)
		goto LABEL_45;

	if (a1 >= _vm->_graphicsManager.max_y || v15 >= _vm->_graphicsManager.max_x || (signed int)v14 <= 1 || v13 <= 1) {
		if (btsouris != 23)
			goto LABEL_54;

LABEL_45:
		if (a1 < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x) {
			if ((signed int)(v14 + v15) > _vm->_graphicsManager.max_x)
				v14 -= v14 + v15 - _vm->_graphicsManager.max_x;
			if (a1 + v13 > _vm->_graphicsManager.max_y)
				v13 -= a1 + v13 - _vm->_graphicsManager.max_y;
  
			if ((signed int)v14 > 1 && v13 > 1) {
				_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
				_vm->_graphicsManager.Affiche_Perfect(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.Bufferobjet, v15 + 300, a1 + 300, 0, 0, 0, 0);
				_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, a1, v14 + v15, a1 + v13);
			}
		}
		goto LABEL_54;
	}

	_vm->_graphicsManager.Capture_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
	_vm->_graphicsManager.Sprite_Vesa(_vm->_graphicsManager.VESA_BUFFER, pointeur_souris, v15 + 300, a1 + 300, souris_n);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, a1, v12, v11);

LABEL_54:
	_vm->_globals.vitesse = 2;

	do {
		for (;;) {
			while (CASSE || _vm->_globals.iRegul != 1) {
				if (CASSE != 1)
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
	} while (_vm->_globals.iRegul == 3 && lItCounter <= 15);

LABEL_65:
	_vm->_globals.vitesse = 2;
	lItCounter = 0;

	if (_vm->_graphicsManager.DOUBLE_ECRAN != 1 || _vm->_graphicsManager.no_scroll == 1) {
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

		if (_vm->_graphicsManager.SCROLL > SCREEN_WIDTH)
			_vm->_graphicsManager.SCROLL = SCREEN_WIDTH;

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
					v5 = _vm->_graphicsManager.Reel_Zoom(20, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, v5);
				} else {
					v6 = _vm->_graphicsManager.Reel_Zoom(20, _vm->_graphicsManager.SDL_ECHELLE);
					_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, v6);
				}
			
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0].left = _vm->_graphicsManager.Reel_Zoom(0, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].top = _vm->_graphicsManager.Reel_Zoom(20, _vm->_graphicsManager.SDL_ECHELLE);
				_vm->_graphicsManager.dstrect[0].setWidth(_vm->_graphicsManager.Reel_Zoom(SCREEN_WIDTH, _vm->_graphicsManager.SDL_ECHELLE));
				_vm->_graphicsManager.dstrect[0].setHeight(_vm->_graphicsManager.Reel_Zoom(440, _vm->_graphicsManager.SDL_ECHELLE));
			} else {
				if (_vm->_graphicsManager.Winbpp == 2)
					_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				else
					_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_BUFFER, v4, 20, 640, 440, 0, 20);
				
				_vm->_graphicsManager.DD_Unlock();
				_vm->_graphicsManager.dstrect[0] = Common::Rect(0, 20, SCREEN_WIDTH, SCREEN_HEIGHT - 40);
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
	if (souris_flag == 1) {
		if (btsouris != 23) {
			if (a1 < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x && v14 > 1 && v13 > 1) {
				_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
				_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, a1, v12, v11);
				goto LABEL_113;
			}
  
			if (btsouris != 23)
				goto LABEL_113;
		}
	    
		if (a1 < _vm->_graphicsManager.max_y && v15 < _vm->_graphicsManager.max_x && v14 > 1 && v13 > 1) {
			_vm->_graphicsManager.Restore_Mem(_vm->_graphicsManager.VESA_BUFFER, _vm->_globals.cache_souris, v15, a1, v14, v13);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v15, a1, v14 + v15, a1 + v13);
		}
	}
		
LABEL_113:
 
	_vm->_soundManager.VERIF_SOUND();
	return CONTROLE_MES();
}	

} // End of namespace Hopkins
