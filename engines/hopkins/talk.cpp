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
#include "hopkins/talk.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

void TalkManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void TalkManager::PARLER_PERSO2(const Common::String &filename) {
	int v1;
	int v2; 
	uint v3;
	int v4;
	int v5;
	byte v7;
	int v8;
	int v9;

	v9 = 0;
	_vm->_objectsManager.DESACTIVE_CURSOR = 1;
	STATI = 1;
	v7 = _vm->_objectsManager.DESACTIVE_INVENT;
	_vm->_objectsManager.DESACTIVE_INVENT = 1;
	BUFFERPERSO = FileManager::RECHERCHE_CAT(filename, 5);
	TAILLEPERSO = _vm->_globals.CAT_TAILLE;
	if (BUFFERPERSO == PTRNUL) {
		FileManager::CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, filename);
		BUFFERPERSO = FileManager::CHARGE_FICHIER(_vm->_globals.NFICHIER);
		TAILLEPERSO = FileManager::FLONG(_vm->_globals.NFICHIER);
	}

	_vm->_globals.SAUVEGARDE->field4 = 0;
	RENVOIE_FICHIER(0, FQUEST, (const char *)BUFFERPERSO);
	RENVOIE_FICHIER(20, FREPON, (const char *)BUFFERPERSO);

	switch (_vm->_globals.FR) {
	case 0:
		FQUEST = "RUEAN.TXT";
		FREPON = "RUEAN.TXT";
		break;
	case 1:
		FQUEST = "RUE.TXT";
		FREPON = "RUE.TXT";
		break;
	case 2:
		FQUEST = "RUEES.TXT";
		FREPON = "RUEES.TXT";
		break;
	}

	v1 = READ_LE_UINT16((uint16 *)BUFFERPERSO + 40);
	PCHERCHE = 20 * READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
	CHERCHE_PAL(PCHERCHE, 0);
	PLIGNE1 = v1;
	PLIGNE2 = v1 + 1;
	PLIGNE3 = v1 + 2;
	PLIGNE4 = v1 + 3;
	v8 = _vm->_eventsManager.btsouris;
	_vm->_eventsManager.btsouris = 4;
	_vm->_eventsManager.CHANGE_MOUSE(0);
  
	if (!_vm->_globals.NOPARLE) {
		v2 = v1 + 3;
		do {
			v3 = DIALOGUE();
			if ((signed __int16)v3 != v2)
				v9 = DIALOGUE_REP(v3);
			
			if (v9 == -1)
				v3 = v1 + 3;
		} while ((signed __int16)v3 != v2);
	}

	if (_vm->_globals.NOPARLE) {
		v4 = 1;
	    do
			v5 = DIALOGUE_REP(v4++);
		while (v5 != -1);
	}

	BUFFERPERSO = _vm->_globals.LIBERE_FICHIER(BUFFERPERSO);
	_vm->_eventsManager.btsouris = v8;

	_vm->_eventsManager.CHANGE_MOUSE(v8);
	_vm->_graphicsManager.INIT_TABLE(145, 150, _vm->_graphicsManager.Palette);
	_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	_vm->_objectsManager.DESACTIVE_CURSOR = 0;
	_vm->_objectsManager.DESACTIVE_INVENT = v7;
	STATI = 0;
}

void TalkManager::RENVOIE_FICHIER(int srcStart, Common::String &dest, const char *srcData) {
	dest = Common::String(srcData + srcStart);
}

int TalkManager::DIALOGUE_REP(int idx) {
	warning("TODO: DIALOGUE_REP");
	return 0;
}

int TalkManager::DIALOGUE() {
	warning("TODO: DIALOGUE");
	return 0;
}

void TalkManager::CHERCHE_PAL(int a1, int a2) {

}

} // End of namespace Hopkins
