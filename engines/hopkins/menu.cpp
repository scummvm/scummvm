/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is _globals.FRee software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the _globals.FRee Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the _globals.FRee Software
 * Foundation, Inc., 51 _globals.FRanklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
	signed int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	int v11;

	v6 = 0;
	while (!g_system->getEventManager()->shouldQuit()) {
		_vm->_globals.FORET = 0;
		_vm->_eventsManager.CASSE = 0;
		_vm->_globals.DESACTIVE_INVENT = 1;
		_vm->_globals.FLAG_VISIBLE = 0;
		_vm->_globals.SORTIE = 0;

		for (int idx = 0; idx < 31; ++idx)
			_vm->_globals.INVENTAIRE[idx] = 0;
    
		memset(_vm->_globals.SAUVEGARDE, 0, 2000);
		_vm->_objectsManager.AJOUTE_OBJET(14);
		v7 = 0;
		v8 = 0;
		v9 = 0;
		v10 = 0;
		v11 = 0;


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
						v11 = 0;
						v10 = 0;
						v9 = 0;
						v8 = 0;
						v7 = 0;
						break;
					case PLAY_GAME:
						v11 = 1;
						v10 = 0;
						v9 = 0;
						v8 = 0;
						v7 = 0;
						break;
					case LOAD_GAME:
						v11 = 0;
						v10 = 1;
						v9 = 0;
						v8 = 0;
						v7 = 0;
						break;
					case OPTIONS:
						v11 = 0;
						v10 = 0;
						v9 = 1;
						v8 = 0;
						v7 = 0;
						break;
					case INTRODUCTION:
						v11 = 0;
						v10 = 0;
						v9 = 0;
						v8 = 1;
						v7 = 0;
						break;
					case QUIT:
						v11 = 0;
						v10 = 0;
						v9 = 0;
						v8 = 0;
						v7 = 1;
					}
          
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 259, v11);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 291, v10 + 2);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 322, v9 + 4);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 354, v8 + 6);
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 386, v7 + 8);
					_vm->_eventsManager.VBL();
          
					if (_vm->_eventsManager.BMOUSE() == 1 && menuIndex != MENU_NONE)
						selectionMade = 1;
				} while (!selectionMade);
        
				if (menuIndex == PLAY_GAME) {
					_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 259, 10);
					_vm->_eventsManager.VBL();
					_vm->_eventsManager.delay(200);
					v6 = 1;
				}
				if (menuIndex != LOAD_GAME)
					break;

				_vm->_graphicsManager.AFFICHE_SPEED(spriteData, 230, 291, 11);
				_vm->_eventsManager.VBL();
				_vm->_eventsManager.delay(200);
        
				_vm->_globals.SORTIE = -1;
				_vm->_dialogsManager.CHARGE_PARTIE();
        
				if (_vm->_globals.SORTIE != -1) {
					v6 = _vm->_globals.SORTIE;
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
			v6 = -1;
		}
		break;
	}
  
	_vm->_globals.LIBERE_FICHIER(spriteData);
	_vm->_globals.DESACTIVE_INVENT = 0;
	_vm->_globals.FLAG_VISIBLE = 0;
	_vm->_graphicsManager.FADE_OUTW();
	return v6;
}

void MenuManager::COMPUT_HOPKINS(int idx) {
	// This is a text mode display?
	warning("TODO: COMPUT_HOPKINS");
	/*
	int v2; 
	char *v3; 
	int v4; 
	char v5; 
	char *v6; 
	int v7; 
	char v8; 
	char *v9; 
	int v10; 
	char v11; 
	int v12; 
	Common::String s; 
	Common::String s2;

	_vm->_eventsManager.ESC_KEY = 0;
	v2 = 0;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	setvideomode();
	settextcolor(4);
	_vm->_graphicsManager.videkey();
	settextposition(2, 4);
	if (idx == 1)
		outtext((const char *)&MenuText[10]);
	if (idx == 2)
		outtext((const char *)&MenuText[0xDE]);
	if (idx == 3)
		outtext((const char *)&MenuText[0x1B2]);
	settextcolor(1);
	if (idx == 3) {
		settextposition(10, 8);
		outtext((const char *)&MenuText[0x286]);
	}
	settextposition(12, 28);
	outtext((const char *)&MenuText[0x35A]);
	settextposition(14, 35);
	v3 = &s;
	memset(&s, 0, 8u);
	TXT4(280, 224, 8);
	s = Sup_string;
	if (idx == 1) {
		s2 = "HOPKINS";
		v4 = 8;
		v5 = 1;
		do {
			if (!v4)
				break;
			v5 = *v3++ == *s2++;
			--v4;
		} while (v5);
		if (v5)
			v2 = 1;
	}
	if (idx == 2) {
		v6 = &s;
		s2 = "328MHZA";
		v7 = 8;
		v8 = 1;
		do {
			if (!v7)
				break;
			v8 = *v6++ == *s2++;
			--v7;
		} while (v8);
		if (v8)
			v2 = 1;
	}
	if (idx == 3) {
		v9 = &s;
		s2 = "ALLFREE";
		v10 = 8;
		v11 = 1;
		do {
			if (!v10)
				break;
			v11 = *v9++ == *s2++;
			--v10;
		} while (v11);
		if (v11)
			v2 = 1;
	}
	if (v2) {
		while (1) {
			ESC_KEY = 0;
			_vm->_graphicsManager.videkey();
			clearscreen();
			settextcolor(4);
			settextposition(2, 4);
			if (idx == 1)
				outtext((const char *)&MenuText[10]);
			if (idx == 2)
				outtext((const char *)&MenuText[0xDE]);
			if (idx == 3)
				outtext((const char *)&MenuText[0x1B2]);
			settextcolor(15);
			settextposition(8, 25);
			settextcolor(15);
			outtext2((const char *)&MenuText[0x502]);
			settextposition(20, 25);
			outtext2((const char *)&MenuText[0x5D6]);
			if (idx == 1) {
				settextposition(10, 25);
				outtext2((const char *)&MenuText[0x6AA]);
				settextposition(12, 25);
				outtext2((const char *)&MenuText[0x77E]);
				settextposition(14, 25);
				outtext2((const char *)&MenuText[0x852]);
				settextposition(16, 25);
				outtext2((const char *)&MenuText[0x926]);
			}
			if (idx == 2) {
				_vm->_graphicsManager.videkey();
				settextposition(10, 25);
				outtext2((const char *)&MenuText[0x95A]);
				settextposition(12, 25);
				outtext2((const char *)&MenuText[0xACE]);
				settextposition(14, 25);
				outtext2((const char *)&MenuText[0xBA2]);
				settextposition(16, 25);
				outtext2((const char *)&MenuText[0xC76]);
				settextposition(18, 25);
				outtext2((const char *)&MenuText[0xD4A]);
			}
			do {
				v12 = (byte)keywin(v2, s2);
				v2 = 0;
				if ((uint16)(v12 - 48) <= 8u)
					v2 = 1;
			} while (v2 != 1);
			if (v12 == 48)
				break;
			if (v12 == 49) {
				GAMES(s2);
			} else if (idx == 1) {
				_vm->_graphicsManager.videkey();
				clearscreen();
				settextcolor(4);
				settextposition(2, 4);
				outtext((const char *)&MenuText[10]);
				settextcolor(15);
				if (v12 == 50)
					LIT_TEXTE(1);
				if (v12 == 51)
					LIT_TEXTE(2);
				if (v12 == 52)
					LIT_TEXTE(3);
				if (v12 == 53)
					LIT_TEXTE(4);
			} else if (idx == 2) {
				clearscreen();
				settextcolor(4);
				settextposition(2, 4);
				outtext((const char *)&MenuText[0xDE]);
				settextcolor(15);
				if (v12 == 50)
					LIT_TEXTE(6);
				if (v12 == 51)
					LIT_TEXTE(7);
				if (v12 == 52)
					LIT_TEXTE(8);
				if (v12 == 53)
					LIT_TEXTE(9);
				if (v12ii == 54) {
					LIT_TEXTE(10);
					_vm->_globals.SAUVEGARDE->data[svField270] = 4;
				}
			}
		}
		_vm->_graphicsManager.DD_Lock();
		Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		RESTORE_POLICE();
	} else {
		settextcolor(4);
		settextposition(16, 25);
		outtext((const char *)&MenuText[0x42E]);
		_vm->_eventsManager.VBL();
		memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x4AFFFu);
		_vm->_graphicsManager.DD_Lock();
		Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		RESTORE_POLICE();
		_vm->_eventsManager.MOUSE_OFF();
	}
	if (idx == 1)
		_vm->_globals.SORTIE = 13;
	if ((uint16)(idx - 2) <= 1u)
		_vm->_globals.SORTIE = 14;
	return _vm->_graphicsManager.RESET_SEGMENT_VESA();
	*/
}


} // End of namespace Hopkins
