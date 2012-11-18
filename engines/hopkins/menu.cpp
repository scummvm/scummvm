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

#include "common/scummsys.h"
#include "common/events.h"
#include "common/file.h"
#include "common/util.h"
#include "hopkins/menu.h"
#include "hopkins/dialogs.h"
#include "hopkins/files.h"
#include "hopkins/hopkins.h"
#include "hopkins/globals.h"
#include "hopkins/events.h"
#include "hopkins/graphics.h"
#include "hopkins/sound.h"

namespace Hopkins {

void MenuManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

enum MenuSelection { MENU_NONE = 0, PLAY_GAME = 1, LOAD_GAME = 2, OPTIONS = 3, INTRODUCTION = 4, QUIT = 5 };

int MenuManager::MENU() {
	byte *spriteData = NULL; 
	MenuSelection menuIndex;
	Common::Point mousePos;
	signed int result;
	int frame5Index;
	int frame4Index;
	int frame3Index;
	int frame2Index;
	int frame1Index;

	result = 0;
	while (!g_system->getEventManager()->shouldQuit()) {
		_vm->_globals.FORET = false;
		_vm->_eventsManager.CASSE = false;
		_vm->_globals.DESACTIVE_INVENT = true;
		_vm->_globals.FLAG_VISIBLE = false;
		_vm->_globals.SORTIE = 0;

		for (int idx = 0; idx < 31; ++idx)
			_vm->_globals.INVENTAIRE[idx] = 0;
    
		memset(_vm->_globals.SAUVEGARDE, 0, 2000);
		_vm->_objectsManager.AJOUTE_OBJET(14);
		frame5Index = 0;
		frame4Index = 0;
		frame3Index = 0;
		frame2Index = 0;
		frame1Index = 0;


		if (_vm->_globals.FR == 0)
			_vm->_graphicsManager.LOAD_IMAGE("MENUAN");
		else if (_vm->_globals.FR == 1)
			_vm->_graphicsManager.LOAD_IMAGE("MENUFR");
		else if (_vm->_globals.FR == 2)
			_vm->_graphicsManager.LOAD_IMAGE("MENUES");
    
		_vm->_graphicsManager.FADE_INW();
		if (_vm->_globals.FR == 0)
			_vm->_fileManager.CONSTRUIT_SYSTEM("MENUAN.SPR");
		if (_vm->_globals.FR == 1)
			_vm->_fileManager.CONSTRUIT_SYSTEM("MENUFR.SPR");
		if (_vm->_globals.FR == 2)
			_vm->_fileManager.CONSTRUIT_SYSTEM("MENUES.SPR");
    
		spriteData = _vm->_objectsManager.CHARGE_SPRITE(_vm->_globals.NFICHIER);
		_vm->_eventsManager.MOUSE_ON();
		_vm->_eventsManager.CHANGE_MOUSE(0);
		_vm->_eventsManager.btsouris = 0;
		_vm->_eventsManager.souris_n = 0;
    
		for (;;) {
			for (;;) {
				_vm->_soundManager.WSOUND(28);

				// Loop to make menu selection
				bool selectionMade = false;
				do {
					if (g_system->getEventManager()->shouldQuit())
						return -1;

					menuIndex = MENU_NONE;
					mousePos = Common::Point(_vm->_eventsManager.XMOUSE(), _vm->_eventsManager.YMOUSE());
          
					if ((uint16)(mousePos.x - 232) <= 176) {
						if ((uint16)(mousePos.y - 261) <= 23)
							menuIndex = PLAY_GAME;
						if ((uint16)(mousePos.y - 293) <= 23)
							menuIndex = LOAD_GAME;
						if ((uint16)(mousePos.y - 325) <= 22)
							menuIndex = OPTIONS;
						if ((uint16)(mousePos.y - 356) <= 23)
							menuIndex = INTRODUCTION;
            
						if ((uint16)(mousePos.y - 388) <= 23)
							menuIndex = QUIT;
					}
          
					switch (menuIndex) {
					case MENU_NONE:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case PLAY_GAME:
						frame1Index = 1;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case LOAD_GAME:
						frame1Index = 0;
						frame2Index = 1;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case OPTIONS:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 1;
						frame4Index = 0;
						frame5Index = 0;
						break;
					case INTRODUCTION:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 1;
						frame5Index = 0;
						break;
					case QUIT:
						frame1Index = 0;
						frame2Index = 0;
						frame3Index = 0;
						frame4Index = 0;
						frame5Index = 1;
					}
          
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 259, frame1Index);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 291, frame2Index + 2);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 322, frame3Index + 4);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 354, frame4Index + 6);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 386, frame5Index + 8);
					_vm->_eventsManager.VBL();
          
					if (_vm->_eventsManager.BMOUSE() == 1 && menuIndex != MENU_NONE)
						selectionMade = 1;
				} while (!selectionMade);
        
				if (menuIndex == PLAY_GAME) {
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 259, 10);
					_vm->_eventsManager.VBL();
					_vm->_eventsManager.delay(200);
					result = 1;
				}
				if (menuIndex != LOAD_GAME)
					break;

				_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 291, 11);
				_vm->_eventsManager.VBL();
				_vm->_eventsManager.delay(200);
        
				_vm->_globals.SORTIE = -1;
				_vm->_dialogsManager.CHARGE_PARTIE();
        
				if (_vm->_globals.SORTIE != -1) {
					result = _vm->_globals.SORTIE;
					break;
				}
				_vm->_globals.SORTIE = 0;
			}
      
			if (menuIndex != OPTIONS)
				break;
      
			// Options menu item selected
			_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 322, 12);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);

			// Show the options dialog
			_vm->_dialogsManager.showOptionsDialog();
		}
		if (menuIndex == INTRODUCTION) {
			_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 354, 13);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);
			_vm->INTRORUN();
			continue;
		}

		if ( menuIndex == QUIT) {
			_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 386, 14);
			_vm->_eventsManager.VBL();
			_vm->_eventsManager.delay(200);
			result = -1;
		}
		break;
	}
  
	_vm->_globals.LIBERE_FICHIER(spriteData);
	_vm->_globals.DESACTIVE_INVENT = false;
	_vm->_globals.FLAG_VISIBLE = false;
	_vm->_graphicsManager.FADE_OUTW();
	return result;
}



} // End of namespace Hopkins
