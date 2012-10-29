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
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/talk.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

TalkManager::TalkManager() {
	BUFFERPERSO = NULL;
	PALPERSO = NULL;
	PERSOSPR = NULL;
	ADR_ANIM = NULL;
	TAILLEPERSO = 0;
	STATI = 0;
	PLIGNE1 = PLIGNE2 = 0;
	PLIGNE3 = PLIGNE4 = 0;
	PCHERCHE = 0;
}

void TalkManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void TalkManager::PARLER_PERSO(const Common::String &filename) {
	int v2; 
	int v3; 
	int v4; 
	int v5;
	int v6;
	int v7;
	byte *v8; 
	byte *v9; 
	byte *v10;
	int v11;
	int v14;
	int v15;
	Common::String v16; 

	v15 = 0;
	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_fontManager.TEXTE_OFF(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 1;
	bool old_DESACTIVE_INVENT = _vm->_globals.DESACTIVE_INVENT;
	_vm->_globals.DESACTIVE_INVENT = true;
	BUFFERPERSO = _vm->_fileManager.RECHERCHE_CAT(filename, 5);
	TAILLEPERSO = _vm->_globals.CAT_TAILLE;
	if (BUFFERPERSO == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, filename);
		BUFFERPERSO = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
		TAILLEPERSO = _vm->_fileManager.FLONG(_vm->_globals.NFICHIER);
	}
	_vm->_globals.SAUVEGARDE->data[svField4] = 0;
	RENVOIE_FICHIER(40, v16, (const char *)BUFFERPERSO);
	RENVOIE_FICHIER(0, FQUEST, (const char *)BUFFERPERSO);
	RENVOIE_FICHIER(20, FREPON, (const char *)BUFFERPERSO);
	if (_vm->_globals.FR == 1) {
		FREPON = FQUEST = "RUE.TXT";
	} else if (!_vm->_globals.FR) {
		FREPON = FQUEST = "RUEAN.TXT";
	} else if (_vm->_globals.FR == 2) {
		FREPON = FQUEST = "RUEES.TXT";
	}
	v2 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 40);
	v3 = 20 * (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
	PCHERCHE = 20 * (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
	PERSOSPR = _vm->_fileManager.RECHERCHE_CAT(v16, 7);
	if (PERSOSPR) {
		_vm->_globals.CAT_FLAG = false;
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, v16);
	} else {
		_vm->_globals.CAT_FLAG = true;
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_SAN.RES");
	}
	PERSOSPR = _vm->_objectsManager.CHARGE_SPRITE(_vm->_globals.NFICHIER);
	_vm->_globals.CAT_FLAG = false;

	_vm->_fileManager.CONSTRUIT_LINUX("TEMP.SCR");
	if (_vm->_graphicsManager.nbrligne == SCREEN_WIDTH)
		_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
	if (_vm->_graphicsManager.nbrligne == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x96000u);

	if (!_vm->_graphicsManager.nbrligne)
		_vm->_graphicsManager.ofscroll = 0;
	_vm->_graphicsManager.NB_SCREEN();
	_vm->_objectsManager.PERSO_ON = true;
	CHERCHE_PAL(v3, 0);
	CHERCHE_ANIM0(v3, 0);
	ANIM_PERSO_INIT();
	PLIGNE1 = v2;
	PLIGNE2 = v2 + 1;
	PLIGNE3 = v2 + 2;	
	PLIGNE4 = v2 + 3;
	v14 = _vm->_eventsManager.btsouris;
	_vm->_eventsManager.btsouris = 4;
	_vm->_eventsManager.CHANGE_MOUSE(0);
	if (!_vm->_globals.NOPARLE) {
		v4 = v2 + 3;
		do {
			v5 = DIALOGUE();
			if (v5 != v4)
				v15 = DIALOGUE_REP(v5);
			if (v15 == -1)
				v5 = v2 + 3;
			_vm->_eventsManager.VBL();
		} while (v5 != v4);
	}
	if (_vm->_globals.NOPARLE == true) {
		v6 = 1;
		do
			v7 = DIALOGUE_REP(v6++);
		while (v7 != -1);
	}
	CLEAR_ANIM_PERSO();
	_vm->_globals.NOPARLE = false;
	_vm->_globals.NECESSAIRE = 1;
	BUFFERPERSO = _vm->_globals.LIBERE_FICHIER(BUFFERPERSO);
	PERSOSPR = _vm->_globals.LIBERE_FICHIER(PERSOSPR);
	_vm->_graphicsManager.NB_SCREEN();
	_vm->_globals.NECESSAIRE = 0;

	_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager.btsouris = v14;
	
	_vm->_eventsManager.CHANGE_MOUSE(v14);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.INIT_TABLE(145, 150, _vm->_graphicsManager.Palette);
	_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	_vm->_graphicsManager.DD_LOCK();
	if (_vm->_graphicsManager.Winbpp == 2) {
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
	}
	if (_vm->_graphicsManager.Winbpp == 1) {
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
		else
			_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
	}
	_vm->_graphicsManager.DD_UNLOCK();
	v8 = _vm->_graphicsManager.VESA_BUFFER;
	v9 = _vm->_graphicsManager.VESA_SCREEN;
	memcpy(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.VESA_SCREEN, 0x95FFCu);
	v9 = v9 + 614396;
	v8 = v8 + 614396;
	*v8 = *v9;
	v8 = v8 + 2;
	*v8 = *(v9 + 2);
	v10 = v8 + 1;
	_vm->_globals.DESACTIVE_INVENT = old_DESACTIVE_INVENT;
	_vm->_graphicsManager.DD_VBL();
	v11 = 0;
	do {
		_vm->_eventsManager.VBL();
		++v11;
	} while (v11 <= 4);
	_vm->_graphicsManager.no_scroll = 0;
}

void TalkManager::PARLER_PERSO2(const Common::String &filename) {
	int v1;
	int v2; 
	int v3;
	int v4;
	int v5;
	byte v7;
	int v8;
	int v9;

	v9 = 0;
	_vm->_objectsManager.DESACTIVE_CURSOR = 1;
	STATI = 1;
	v7 = _vm->_dialogsManager.DESACTIVE_INVENT;
	_vm->_dialogsManager.DESACTIVE_INVENT = 1;
	BUFFERPERSO = _vm->_fileManager.RECHERCHE_CAT(filename, 5);
	TAILLEPERSO = _vm->_globals.CAT_TAILLE;
	if (BUFFERPERSO == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, filename);
		BUFFERPERSO = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
		TAILLEPERSO = _vm->_fileManager.FLONG(_vm->_globals.NFICHIER);
	}

	_vm->_globals.SAUVEGARDE->data[svField4] = 0;
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

	v1 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 40);
	PCHERCHE = 20 * (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
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
			if (v3 != v2)
				v9 = DIALOGUE_REP(v3);
			
			if (v9 == -1)
				v3 = v1 + 3;
		} while (v3 != v2);
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
	_vm->_dialogsManager.DESACTIVE_INVENT = v7;
	STATI = 0;
}

void TalkManager::RENVOIE_FICHIER(int srcStart, Common::String &dest, const char *srcData) {
	dest = Common::String(srcData + srcStart);
}

int TalkManager::DIALOGUE() {
	byte *v0; 
	int v1;
	int v2;
	int v3;
	int v4;
	int v5;
	int v6;
	int v7;
	int v8;
	int v9;
	int v10;
	byte *v11; 
	int v12;
	int v13;
	int v14;
	int v15;
	int v16;
	int v18; 
	int v19;
	int v20;
	int v21;

	if (STATI) {
		v0 = BUFFERPERSO;
		v1 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 48);
		if (v1)
			_vm->_objectsManager.BOBANIM_ON(v1);
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 1)
			_vm->_objectsManager.BOBANIM_ON((int16)READ_LE_UINT16((uint16 *)v0 + 49));
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 2)
			_vm->_objectsManager.BOBANIM_ON((int16)READ_LE_UINT16((uint16 *)v0 + 50));
		if ( (int16)READ_LE_UINT16((uint16 *)v0 + 48) != 3)
			_vm->_objectsManager.BOBANIM_ON((int16)READ_LE_UINT16((uint16 *)v0 + 51));
		if ((int16)READ_LE_UINT16((uint16 *)v0 + 48) != 4)
			_vm->_objectsManager.BOBANIM_ON((int16)READ_LE_UINT16((uint16 *)v0 + 52));
	} else {
		VISU_WAIT();
	}
	
	v19 = VERIF_BOITE(PLIGNE1, FQUEST, 65);
	v2 = VERIF_BOITE(PLIGNE2, FQUEST, 65);
	v3 = VERIF_BOITE(PLIGNE3, FQUEST, 65);
	v20 = 420 - 20 * VERIF_BOITE(PLIGNE4, FQUEST, 65);
	v21 = v20 - 20 * v3;
	v18 = v20 - 20 * v3 - 1;
	v4 = v20 - 20 * v3 - 20 * v2;
	_vm->_fontManager.DOS_TEXT(5, PLIGNE1, FQUEST, 5, v4 - 20 * v19, 0, 0, 0, 65, 255);
	_vm->_fontManager.DOS_TEXT(6, PLIGNE2, FQUEST, 5, v4, 0, 0, 0, 65, 255);
	_vm->_fontManager.DOS_TEXT(7, PLIGNE3, FQUEST, 5, v21, 0, 0, 0, 65, 255);
	_vm->_fontManager.DOS_TEXT(8, PLIGNE4, FQUEST, 5, v20, 0, 0, 0, 65, 255);
	_vm->_fontManager.TEXTE_ON(5);
	_vm->_fontManager.TEXTE_ON(6);
	_vm->_fontManager.TEXTE_ON(7);
	_vm->_fontManager.TEXTE_ON(8);

	v5 = -1;
	v6 = 0;
  	do {
		v7 = _vm->_eventsManager.YMOUSE();
		if ((v4 - 20 * v19) < v7 && (v4 - 1) > v7) {
			v8 = v7;
			_vm->_fontManager.OPTI_COUL_TXT(6, 7, 8, 5);
			v5 = PLIGNE1;
			v7 = v8;
		}
		if (v7 > v4 && v18 > v7) {
			v9 = v7;
			_vm->_fontManager.OPTI_COUL_TXT(5, 7, 8, 6);
			v5 = PLIGNE2;
			v7 = v9;
		}
		if (v21 < v7 && (v20 - 1) > v7) {
			v10 = v7;
			_vm->_fontManager.OPTI_COUL_TXT(5, 6, 8, 7);
			v5 = PLIGNE3;
			v7 = v10;
		}
		if (v20 < v7 && v7 < 419) {
			_vm->_fontManager.OPTI_COUL_TXT(5, 6, 7, 8);
			v5 = PLIGNE4;
		}
    
		_vm->_eventsManager.VBL();
		if (_vm->_eventsManager.BMOUSE())
			v6 = 1;
		if (v5 == -1)
			v6 = 0;
	} while (!_vm->shouldQuit() && v6 != 1);
  
	_vm->_soundManager.VOICE_MIX(v5, 1);
	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_fontManager.TEXTE_OFF(6);
	_vm->_fontManager.TEXTE_OFF(7);
	_vm->_fontManager.TEXTE_OFF(8);
  
	if (STATI) {
		v11 = BUFFERPERSO;
		v12 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 48);
		if (v12)
			_vm->_objectsManager.BOBANIM_OFF(v12);
		v13 = (int16)READ_LE_UINT16((uint16 *)v11 + 49);
		if (v13 != 1)
			_vm->_objectsManager.BOBANIM_OFF(v13);
		v14 = (int16)READ_LE_UINT16((uint16 *)v11 + 50);
		if (v14 != 2)
			_vm->_objectsManager.BOBANIM_OFF(v14);
		v15 = (int16)READ_LE_UINT16((uint16 *)v11 + 51);
		if (v15 != 3)
			_vm->_objectsManager.BOBANIM_OFF(v15);
		v16 = (int16)READ_LE_UINT16((uint16 *)v11 + 52);
		if (v16 != 4)
			_vm->_objectsManager.BOBANIM_OFF(v16);
	} else {
		FIN_VISU_WAIT();
	}
  
	_vm->_eventsManager.VBL();
  return v5;
}

int TalkManager::DIALOGUE_REP(int idx) {
	int v1;
	int v2;
	byte *v3;
	int i; 
	int v6;
	int v7;
	byte *v8;
	int v9; 
	int v10;
	int v11;
	int v12;
	int v13;
	int v14;
	void *v15;
	int v16; 
	int v17; 
	int v18; 
	int v19; 
	int v20; 
	int v21; 
	int v22; 
	int v23; 
	int v24; 
	int v25; 
	
	v1 = 0;
	v2 = 0;
	v3 = BUFFERPERSO + 110;
	for (i = idx; (int16)READ_LE_UINT16(v3) != idx; v3 = BUFFERPERSO + 20 * v1 + 110) {
		++v1;
		if ((int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) < v1)
			v2 = 1;
		if (v2 == 1)
			return -1;
//		HIWORD(i) = HIWORD(BUFFERPERSO);
	}
	if (v2 == 1)
		return -1;

	v22 = (int16)READ_LE_UINT16((uint16 *)v3 + 1);
	v25 = (int16)READ_LE_UINT16((uint16 *)v3 + 2);
	v24 = (int16)READ_LE_UINT16((uint16 *)v3 + 3);
	i = (int16)READ_LE_UINT16((uint16 *)v3 + 4);
	v23 = (int16)READ_LE_UINT16((uint16 *)v3 + 4);
	PLIGNE1 = (int16)READ_LE_UINT16((uint16 *)v3 + 5);
	PLIGNE2 = (int16)READ_LE_UINT16((uint16 *)v3 + 6);
	PLIGNE3 = (int16)READ_LE_UINT16((uint16 *)v3 + 7);
	v6 = (int16)READ_LE_UINT16((uint16 *)v3 + 8);
	v7 = (int16)READ_LE_UINT16((uint16 *)v3 + 9);
  
	if (v7)
		_vm->_globals.SAUVEGARDE->data[svField4] = v7;

	if (!v6)
		v6 = 10;
	if (STATI) {
		v8 = BUFFERPERSO;
		v9 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 43);
		if (v9)
			_vm->_objectsManager.BOBANIM_ON(v9);
		v10 = (int16)READ_LE_UINT16((uint16 *)v8 + 44);
		if (v10)
			_vm->_objectsManager.BOBANIM_ON(v10);
		v11 = (int16)READ_LE_UINT16((uint16 *)v8 + 45);
		if (v11)
			_vm->_objectsManager.BOBANIM_ON(v11);
		v12 = (int16)READ_LE_UINT16((uint16 *)v8 + 46);
		if (v12)
			_vm->_objectsManager.BOBANIM_ON(v12);
		v13 = (int16)READ_LE_UINT16((uint16 *)v8 + 47);
		if (v13)
			_vm->_objectsManager.BOBANIM_ON(v13);
	} else {
		VISU_PARLE();
	}

	if (!_vm->_soundManager.TEXTOFF) {
		_vm->_fontManager.DOS_TEXT(9, v22, FREPON, v25, v24, 20, 25, 5, v23, 252);
		_vm->_fontManager.TEXTE_ON(9);
	}
	if (!_vm->_soundManager.VOICE_MIX(v22, 1)) {
		v14 = 0;
		_vm->_eventsManager.souris_bb = false;
		_vm->_eventsManager.souris_b = false;

		do {
			_vm->_eventsManager.VBL();
			++v14;
		} while (v14 != v6);
	}

	if (!_vm->_soundManager.TEXTOFF)
		_vm->_fontManager.TEXTE_OFF(9);
	if (STATI) {
		v15 = BUFFERPERSO;
		v16 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 43);
		if (v16)
			_vm->_objectsManager.BOBANIM_OFF(v16);
		v17 = (int16)READ_LE_UINT16((uint16 *)v15 + 44);
		if (v17)
			_vm->_objectsManager.BOBANIM_OFF(v17);
		v18 = (int16)READ_LE_UINT16((uint16 *)v15 + 45);
		if (v18)
			_vm->_objectsManager.BOBANIM_OFF(v18);
		v19 = (int16)READ_LE_UINT16((uint16 *)v15 + 46);
		if (v19)
			_vm->_objectsManager.BOBANIM_OFF(v19);
		v20 = (int16)READ_LE_UINT16((uint16 *)v15 + 47);
		if (v20)
			_vm->_objectsManager.BOBANIM_OFF(v20);
	} else {
		FIN_VISU_PARLE();
	}
	v21 = 0;
	if (!PLIGNE1)
		v21 = -1;

	return v21;
}

void TalkManager::CHERCHE_PAL(int a1, int a2) {
	int v2;
	size_t v4; 
	unsigned int v5;
	byte *palette; 
	int v8; 

	v2 = 0;
	v8 = 0;
	v4 = a1;
	for (;;) {
		if ( *(BUFFERPERSO + v4) == 'P' 
				&& *(BUFFERPERSO + v4 + 1) == 'A'
				&& *(BUFFERPERSO + v4 + 2) == 'L') {
			v8 = 1;
			v2 = v4;
		}
		++v4;
		if (v8 == 1)
			break;
		if (TAILLEPERSO == v4)
			return;
	}
  
	v5 = v2 + 5;
	palette = BUFFERPERSO + v5;
	PALPERSO = BUFFERPERSO + v5;
	if (a2 == 0) {
		*(palette + 762) = 0;
		*(palette + 763) = 0;
		*(palette + 764) = 0;
		*(palette + 765) = 0xe0;
		*(palette + 766) = 0xe0;
		*(palette + 767) = 0xff;
		*(palette + 759) = 0xff;
		*(palette + 760) = 0xff;
		*(palette + 761) = 86;
		*palette = 0;
		*(palette + 1) = 0;
		*(palette + 2) = 0;
	}
	if (a2 == 1) {
		*(palette + 765) = 0xe0;
		*(palette + 766) = 0xe0;
		*(palette + 767) = 0xff;
		*(palette + 759) = 0xff;
		*(palette + 760) = 0xff;
		*(palette + 761) = 0xff;
		*palette = 0;
		*(palette + 1) = 0;
		*(palette + 2) = 0;
		*(palette + 762) = 0;
		*(palette + 763) = 0;
		*(palette + 764) = 0;
	}
  
	_vm->_graphicsManager.setpal_vga256(palette);
	_vm->_graphicsManager.INIT_TABLE(145, 150, palette);
}

void TalkManager::VISU_WAIT() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::FIN_VISU_WAIT() {
	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_OFF(idx);
	}

	for (int idx = 26; idx <= 30; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

void TalkManager::FIN_VISU_PARLE() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_OFF(idx);
	}
  
	_vm->_eventsManager.VBL();
	_vm->_eventsManager.VBL();
	
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			_vm->_objectsManager.BOB_ZERO(idx);
	}
}

int TalkManager::VERIF_BOITE(int idx, const Common::String &file, int a3) {
	int v6;
	int v7;
	char v8; 
	int v9;
	int v10;
	char v11; 
	char v13; 
	char v14; 
	int v15;
	byte *ptr; 
	int v17;
	int v18;
	byte *v19; 
	uint32 indexData[4047];
	Common::String filename;
	Common::String dest;
	Common::File f;
	int filesize;

	v18 = 0;
	_vm->_globals.police_l = 11;
	_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, file);
	
	// Build up the filename
	filename = dest = _vm->_globals.NFICHIER;
	while (filename.lastChar() != '.')
		filename.deleteLastChar();
	filename += "IND";

	if (!f.open(filename))
		error("Could not open file - %s", filename.c_str());
	filesize = f.size();
	assert(filesize < 16188);	

	for (int i = 0; i < (filesize / 4); ++i)
		indexData[i] = f.readUint32LE();
	f.close();

	if (!f.open(dest))
		error("Error opening file - %s", dest.c_str());

	f.seek(indexData[idx]);
	ptr = _vm->_globals.dos_malloc2(2058);
	if (ptr == g_PTRNUL)
		error("temporary TEXT");
	f.read(ptr, 2048);
	f.close();

	v19 = ptr;
	v6 = 0;
	do {
		v13 = *v19;
		if ((byte)(*v19 + 46) > 0x1Bu) {
			if ((byte)(v13 + 80) > 0x1Bu) {
				if ((byte)(v13 - 65) <= 0x19u || (byte)(v13 - 97) <= 0x19u)
					v13 = 32;
			} else {
				v13 -= 79;
			}
		} else {
			v13 = *v19 + 111;
		}
		*v19 = v13;
		v19 = v19 + 1;
		++v6;
	} while ((unsigned int)v6 < 0x800);
  
	v7 = 0;
	do {
		v8 = *(ptr + v7);
		if ( v8 == 10 || v8 == 13 )
			*(ptr + v7) = 0;
		++v7;
	} while ((unsigned int)v7 < 0x800);

	v9 = 0;
	v15 = (11 * a3) - 4;
	do {
		v10 = 0;
		for (;;) {
			v17 = v10;
			do {
				v11 = *(ptr + v9 + v10);
				v14 = v11;
				++v10;
			} while (v11 != 32 && v11 != 37);
			if (v10 >= v15 / _vm->_globals.police_l)
				break;
			if (v11 == 37) {
				if (v10 < v15 / _vm->_globals.police_l)
					goto LABEL_31;
				break;
			}
		}
		if (v11 != 37)
			goto LABEL_33;
		v14 = 32;
LABEL_31:
		if (v14 == 37)
			v17 = v10;
LABEL_33:
		++v18;
		v9 += v17;
	} while (v14 != 37);
	free(ptr);
	return v18;
}

void TalkManager::VISU_PARLE() {
	for (int idx = 21; idx <= 25; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].field4 == 1)
			BOB_VISU_PARLE(idx);
	}
}

void TalkManager::BOB_VISU_PARLE(int idx) {
	int v4;
	byte *v5;

	_vm->_objectsManager.PRIORITY = 1;
	if (!_vm->_globals.Bob[idx].field0) {
		_vm->_objectsManager.BOB_ZERO(idx);
		v5 = _vm->_globals.Bqe_Anim[idx].data;
		v4 = (int16)READ_LE_UINT16(v5 + 2);
		if (!v4)
			v4 = 1;
		if ((int16)READ_LE_UINT16(v5 + 24)) {
			_vm->_globals.Bob[idx].isSprite = true;
			_vm->_globals.Bob[idx].field36 = 0;
			_vm->_globals.Bob[idx].field38 = 0;
			_vm->_globals.Bob[idx].animData = _vm->_globals.Bqe_Anim[idx].data;
			_vm->_globals.Bob[idx].field0 = 10;
			v5 = PERSOSPR;
			_vm->_globals.Bob[idx].spriteData = PERSOSPR;
			_vm->_globals.Bob[idx].field1E = v4;
			_vm->_globals.Bob[idx].field20 = -1;
			_vm->_globals.Bob[idx].field22 = 0;
			_vm->_globals.Bob[idx].offsetY = 0;
		}
	}
}

void TalkManager::CHERCHE_ANIM0(int a1, int a2) {
	size_t v2;
	int v3; 
	size_t v4;
	unsigned int v5;
	unsigned int v6;
	int v7;
	byte *v8; 
	byte *v9; 

	v2 = 0;
	v3 = 0;
	v4 = a1;
	while (1) {
		if (*(BUFFERPERSO + v4) == 'A'
		        && *(BUFFERPERSO + v4 + 1) == 'N'
		        && *(BUFFERPERSO + v4 + 2) == 'I'
		        && *(BUFFERPERSO + v4 + 3) == 'M'
		        && *(BUFFERPERSO + v4 + 4) == 1) {
			v3 = 1;
			v2 = v4;
		}
		++v4;
		if (v3 == 1)
			break;
		if (TAILLEPERSO == v4)
			return;
	}
	v5 = v2 + 25;
	v9 = BUFFERPERSO + v5;
	v8 = BUFFERPERSO + v5;
	ADR_ANIM = BUFFERPERSO + v5;
	if (!a2) {
		v6 = 0;
		do {
			v7 = (int16)READ_LE_UINT16(&v8[2 * v6 + 4]);
			if (v7 && _vm->_globals.vitesse != 501)
				_vm->_graphicsManager.AFFICHE_SPEED(PERSOSPR, _vm->_eventsManager.start_x + (int16)READ_LE_UINT16(&v8[2 * v6]),
				    (int16)READ_LE_UINT16(&v8[2 * v6 + 2]), v9[2 * v6 + 8]);
			v6 += 5;
		} while (_vm->_globals.vitesse != 501 && v7);
	}
}

void TalkManager::ANIM_PERSO_INIT() {
	byte *v0;
	byte *v1; 
	int v2;
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10;
	int v11;
	int v12;

	v0 = BUFFERPERSO;
	v1 = BUFFERPERSO + 110;
	v2 = (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 43);
	if (v2)
		RECHERCHE_ANIM_PERSO(21, (BUFFERPERSO + 110), v2, TAILLEPERSO);
	v4 = (int16)READ_LE_UINT16((uint16 *)v0 + 44);
	if (v4)
		RECHERCHE_ANIM_PERSO(22, v1, v4, TAILLEPERSO);
	v5 = (int16)READ_LE_UINT16((uint16 *)v0 + 45);
	if (v5)
		RECHERCHE_ANIM_PERSO(23, v1, v5, TAILLEPERSO);
	v6 = (int16)READ_LE_UINT16((uint16 *)v0 + 46);
	if (v6)
		RECHERCHE_ANIM_PERSO(24, v1, v6, TAILLEPERSO);
	v7 = (int16)READ_LE_UINT16((uint16 *)v0 + 47);
	if (v7)
		RECHERCHE_ANIM_PERSO(25, v1, v7, TAILLEPERSO);
	v8 = (int16)READ_LE_UINT16((uint16 *)v0 + 48);
	if (v8)
		RECHERCHE_ANIM_PERSO(26, v1, v8, TAILLEPERSO);
	v9 = (int16)READ_LE_UINT16((uint16 *)v0 + 49);
	if (v9)
		RECHERCHE_ANIM_PERSO(27, v1, v9, TAILLEPERSO);
	v10 = (int16)READ_LE_UINT16((uint16 *)v0 + 50);
	if (v10)
		RECHERCHE_ANIM_PERSO(28, v1, v10, TAILLEPERSO);
	v11 = (int16)READ_LE_UINT16((uint16 *)v0 + 51);
	if (v11)
		RECHERCHE_ANIM_PERSO(29, v1, v11, TAILLEPERSO);
	v12 = (int16)READ_LE_UINT16((uint16 *)v0 + 52);
	if (v12)
		RECHERCHE_ANIM_PERSO(30, v1, v12, TAILLEPERSO);
}

void TalkManager::CLEAR_ANIM_PERSO() {
	for (int idx = 21; idx <= 34; ++idx) {
		if (_vm->_globals.Bqe_Anim[idx].data != g_PTRNUL)
			_vm->_globals.Bqe_Anim[idx].data = _vm->_globals.dos_free2(_vm->_globals.Bqe_Anim[idx].data);

		_vm->_globals.Bqe_Anim[idx].data = g_PTRNUL;
		_vm->_globals.Bqe_Anim[idx].field4 = 0;
	}
}

bool TalkManager::RECHERCHE_ANIM_PERSO(int a1, const byte *bufPerso, int a3, int a4) {
	bool result; 
	const byte *v5; 
	int v6; 
	int v7;
	byte *v8; 
	byte *v9; 
	int v10;
	int v11;
	int v12;
	char v13; 
	int v14;
	int v15;
	int v16;
	char v17; 
	int v18; 
	int v19;
	int v20; 
	int v22; 
	int v23;
	const byte *v24; 

	v22 = 0;
	result = false;

	v20 = a1;
	do {
		if (*(v22 + bufPerso) == 'A' && *(bufPerso + v22 + 1) == 'N' && *(bufPerso + v22 + 2) == 'I' && *(bufPerso + v22 + 3) == 'M'
		        && *(bufPerso + v22 + 4) == a3) {
			v5 = v22 + bufPerso + 5;
			v18 = v22 + 5;
			v6 = 0;
			v7 = 0;
			do {
				if (*v5 == 'A' && *(v5 + 1) == 'N' && *(v5 + 2) == 'I' && *(v5 + 3) == 'M')
					v7 = 1;
				if (*v5 == 'F' && *(v5 + 1) == 'I' && *(v5 + 2) == 'N')
					v7 = 1;
				if (v18 > a4) {
					_vm->_globals.Bqe_Anim[a1].field4 = 0;
					result = g_PTRNUL;
					_vm->_globals.Bqe_Anim[v20].data = g_PTRNUL;
				}
				++v18;
				++v6;
				++v5;
			} while (v7 != 1);
			_vm->_globals.Bqe_Anim[v20].data = _vm->_globals.dos_malloc2(v6 + 50);
			_vm->_globals.Bqe_Anim[a1].field4 = 1;
			memcpy(_vm->_globals.Bqe_Anim[v20].data, (const byte *)(v22 + bufPerso + 5), 20);
			v8 = _vm->_globals.Bqe_Anim[v20].data;

			v9 = v8 + 20;
			v24 = v22 + bufPerso + 25;
			v10 = (int16)READ_LE_UINT16(v22 + bufPerso + 25);
			v11 = (int16)READ_LE_UINT16(v22 + bufPerso + 27);
			v23 = (int16)READ_LE_UINT16(v22 + bufPerso + 29);
			v12 = (int16)READ_LE_UINT16(v22 + bufPerso + 31);
			v13 = *(v22 + bufPerso + 33);
			*(v8 + 29) = *(v22 + bufPerso + 34);
			WRITE_LE_UINT16(v8 + 20, v10);
			WRITE_LE_UINT16(v8 + 22, v11);
			WRITE_LE_UINT16(v8 + 24, v23);
			WRITE_LE_UINT16(v8 + 26, v12);
			*(v8 + 28) = v13;
			v19 = 1;
			do {
				v9 += 10;
				v24 += 10;
				if (!v23)
					break;
				v14 = (int16)READ_LE_UINT16(v24);
				v15 = (int16)READ_LE_UINT16(v24 + 2);
				v23 = (int16)READ_LE_UINT16(v24 + 4);
				v16 = (int16)READ_LE_UINT16(v24 + 6);
				v17 = *(v24 + 8);
				*(v9 + 9) = *(v24 + 9);
				WRITE_LE_UINT16(v9, v14);
				WRITE_LE_UINT16(v9 + 2, v15);
				WRITE_LE_UINT16(v9 + 4, v23);
				WRITE_LE_UINT16(v9 + 6, v16);
				*(v9 + 8) = v17;
				++v19;
			} while (v19 <= 4999);
			result = 1;
		}
		if (*(bufPerso + v22) == 'F' && *(bufPerso + v22 + 1) == 'I' && *(bufPerso + v22 + 2) == 'N')
			result = true;
		++v22;
	} while (v22 <= a4 && result != 1);

	return result;
}

void TalkManager::REPONSE(int a1, int a2) {
	byte v2; 
	byte v3; 
	byte *v5; 
	byte *v6; 
	uint16 v7; 
	byte *v8; 
	int v9; 
	int v10; 
	uint16 v11; 
	int v12; 
	int v13; 
	int v14; 
	int v15; 
	int v16; 
	int v17; 
	byte *ptr; 

	v2 = a1;
	v3 = a2;
LABEL_2:
	v15 = 0;
	if (_vm->_globals.COUCOU != g_PTRNUL) {
		v5 = _vm->_globals.COUCOU;
		while (1) {
			if (*v5 == 'C') {
				if (*(v5 + 1) == 'O') {
					if (*(v5 + 2) == 'D') {
						if (*(v5 + 3) == v2) {
							if (*(v5 + 4) == v3)
								v15 = 1;
						}
					}
				}
			}
			if (*v5 == 'F' && *(v5 + 1) == 'I' && *(v5 + 2) == 'N')
				break;
			if (!v15)
				v5 = v5 + 1;
			if (v15 == 1) {
				v6 = v5 + 5;
				ptr = _vm->_globals.dos_malloc2(620);
				if (g_PTRNUL == ptr)
					error("TRADUC");
				memset(ptr, 0, 620);
				v7 = 0;
				v12 = 0;
				v14 = 0;
				do {
					v16 = 0;
					if (*(v7 + v6) == 'F' && *(v6 + v7 + 1) == 'C') {
						++v12;
						assert(v12 < (620 / 20));

						v8 = (ptr + 20 * v12);
						v11 = 0;
						do {
							assert(v11 < 20);
							*(v11++ + v8) = *(v7++ + v6);
							if (*(v7 + v6) == 'F' && *(v6 + v7 + 1) == 'F') {
								v16 = 1;
								v9 = v11;
								*(v9 + v8) = 'F';
								*(v8 + v9 + 1) = 'F';
								++v7;
							}
						} while (v16 != 1);
					}
					if (v16 != 1) {
						if (*(v7 + v6) == 'C' && *(v6 + v7 + 1) == 'O' && *(v6 + v7 + 2) == 'D')
							v14 = 1;
						if (v16 != 1 && *(v7 + v6) == 'F' && *(v6 + v7 + 1) == 'I' && *(v6 + v7 + 2) == 'N')
							v14 = 1;
					}
					v6 += v7 + 1;
					v7 = 0;
				} while (v14 != 1);
				v17 = 0;
				v13 = 1;
				do {
					v10 =  _vm->_scriptManager.Traduction(ptr + 20 * v13);
					if (_vm->shouldQuit())
						return;

					if (v10 == 2)
						v13 =  _vm->_scriptManager.Control_Goto(ptr + 20 * v13);
					if (v10 == 3)
						v13 =  _vm->_scriptManager.Control_If(ptr, v13);
					if (v13 == -1)
						error("Invalid IFF function");
					if (v10 == 1 || v10 == 4)
						++v13;
					if (!v10 || v10 == 5)
						v17 = 1;
					if (v10 == 6) {
						_vm->_globals.dos_free2(ptr);
						v2 = _vm->_objectsManager.NVZONE;
						v3 = _vm->_objectsManager.NVVERBE;
						goto LABEL_2;
					}
				} while (v17 != 1);
				_vm->_globals.dos_free2(ptr);
				_vm->_globals.SAUVEGARDE->data[svField2] = 0;
				return;
			}
		}
	}
}

void TalkManager::REPONSE2(int a1, int a2) {
	signed int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	int v9; 
	int v10; 
	int v11; 

	v3 = 0;
	if (a2 == 5 && _vm->_globals.SAUVEGARDE->data[svField3] == 4) {
		if ((uint16)(a1 - 22) <= 1u) {
			_vm->_objectsManager.SETFLIPSPR(0, 0);
			_vm->_objectsManager.SETANISPR(0, 62);
			_vm->_objectsManager.SPACTION(_vm->_globals.FORETSPR, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, 0);
			if (a1 == 22) {
				v4 = _vm->_objectsManager.BOBX(3);
				_vm->_objectsManager.BLOQUE_ANIMX(6, v4);
			}
			if (a1 == 23) {
				v5 = _vm->_objectsManager.BOBX(4);
				_vm->_objectsManager.BLOQUE_ANIMX(6, v5);
			}
			if (a1 == 22) {
				v6 = _vm->_objectsManager.BOBX(3);
				_vm->_objectsManager.BLOQUE_ANIMX(8, v6);
			}
			if (a1 == 23) {
				v7 = _vm->_objectsManager.BOBX(4);
				_vm->_objectsManager.BLOQUE_ANIMX(8, v7);
			}
			_vm->_objectsManager.BOBANIM_OFF(3);
			_vm->_objectsManager.BOBANIM_OFF(4);
			_vm->_objectsManager.BOBANIM_ON(6);
			_vm->_soundManager.PLAY_SAMPLE2(1);
			_vm->_objectsManager.SPACTION1(_vm->_globals.FORETSPR, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
			do
				_vm->_eventsManager.VBL();
			while (_vm->_objectsManager.BOBPOSI(6) <= 12 && _vm->_objectsManager.BOBPOSI(6) != 12);
			_vm->_objectsManager.BOBANIM_OFF(6);
			_vm->_objectsManager.BOBANIM_ON(8);
			if (_vm->_globals.ECRAN == 35)
				v3 = 201;
			if (_vm->_globals.ECRAN == 36)
				v3 = 203;
			if (_vm->_globals.ECRAN == 37)
				v3 = 205;
			if (_vm->_globals.ECRAN == 38)
				v3 = 207;
			if (_vm->_globals.ECRAN == 39)
				v3 = 209;
			if (_vm->_globals.ECRAN == 40)
				v3 = 211;
			if (_vm->_globals.ECRAN == 41)
				v3 = 213;
			_vm->_globals.SAUVEGARDE->data[v3] = 2;
			_vm->_objectsManager.ZONE_OFF(22);
			_vm->_objectsManager.ZONE_OFF(23);
		}
		if ((uint16)(a1 - 20) <= 1u) {
			_vm->_objectsManager.SETFLIPSPR(0, 1);
			_vm->_objectsManager.SETANISPR(0, 62);
			_vm->_objectsManager.SPACTION(_vm->_globals.FORETSPR, "2,3,4,5,6,7,8,9,10,11,12,-1,", 0, 0, 4, 1);
			if (a1 == 20) {
				v8 = _vm->_objectsManager.BOBX(1);
				_vm->_objectsManager.BLOQUE_ANIMX(5, v8);
			}
			if (a1 == 21) {
				v9 = _vm->_objectsManager.BOBX(2);
				_vm->_objectsManager.BLOQUE_ANIMX(5, v9);
			}
			if (a1 == 20) {
				v10 = _vm->_objectsManager.BOBX(1);
				_vm->_objectsManager.BLOQUE_ANIMX(7, v10);
			}
			if (a1 == 21) {
				v11 = _vm->_objectsManager.BOBX(2);
				_vm->_objectsManager.BLOQUE_ANIMX(7, v11);
			}
			_vm->_objectsManager.BOBANIM_OFF(1);
			_vm->_objectsManager.BOBANIM_OFF(2);
			_vm->_objectsManager.BOBANIM_ON(5);
			_vm->_soundManager.PLAY_SAMPLE2(1);
			_vm->_objectsManager.SPACTION1(_vm->_globals.FORETSPR, "13,14,15,14,13,12,13,14,15,16,-1,", 0, 0, 4);
			do
				_vm->_eventsManager.VBL();
			while (_vm->_objectsManager.BOBPOSI(5) <= 12 && _vm->_objectsManager.BOBPOSI(5) != 12);
			_vm->_objectsManager.BOBANIM_OFF(5);
			_vm->_objectsManager.BOBANIM_ON(7);
			if (_vm->_globals.ECRAN == 35)
				v3 = 200;
			if (_vm->_globals.ECRAN == 36)
				v3 = 202;
			if (_vm->_globals.ECRAN == 37)
				v3 = 204;
			if (_vm->_globals.ECRAN == 38)
				v3 = 206;
			if (_vm->_globals.ECRAN == 39)
				v3 = 208;
			if (_vm->_globals.ECRAN == 40)
				v3 = 210;
			if (_vm->_globals.ECRAN == 41)
				v3 = 212;
			_vm->_globals.SAUVEGARDE->data[v3] = 2;
			_vm->_objectsManager.ZONE_OFF(21);
			_vm->_objectsManager.ZONE_OFF(20);
		}
	}
}

void TalkManager::OBJET_VIVANT(const Common::String &a2) {
	int v2; 
	const char *v4; 
	int v5; 
	bool v6;
	int v10; 
	byte *v11; 
	int v12; 
	int v13; 
	byte *v14; 
	byte *v15; 
	byte *v16; 
	int v17; 
	Common::String s;
	Common::String v20; 
	Common::String v22; 
	Common::String v23; 

	_vm->_fontManager.TEXTE_OFF(5);
	_vm->_fontManager.TEXTE_OFF(9);
	_vm->_eventsManager.VBL();
	_vm->_graphicsManager.no_scroll = 1;
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_globals.RESET_CACHE();
	v2 = 0;
	do
		_vm->_globals.BOBZONE[v2++] = 0;
	while (v2 <= 44);
	_vm->_objectsManager.NUMZONE = -1;
	_vm->_eventsManager.btsouris = 4;
	_vm->_eventsManager.CHANGE_MOUSE(0);
	BUFFERPERSO = _vm->_fileManager.RECHERCHE_CAT(a2, 5);
	TAILLEPERSO = _vm->_globals.CAT_TAILLE;
	if (BUFFERPERSO == g_PTRNUL) {
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, a2);
		BUFFERPERSO = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
		TAILLEPERSO = _vm->_fileManager.FLONG(_vm->_globals.NFICHIER);
	}
	RENVOIE_FICHIER(40, v23, (const char *)BUFFERPERSO);
	RENVOIE_FICHIER(0, v22, (const char *)BUFFERPERSO);
	RENVOIE_FICHIER(20, v20, (const char *)BUFFERPERSO);
	v4 = "NULL";
	v5 = 5;

	v6 = v20 != v4;
	if (!v6) {
		v20 = Common::String::format("IM%d", _vm->_globals.ECRAN);
	}
	PERSOSPR = _vm->_fileManager.RECHERCHE_CAT(v23, 7);
	if (PERSOSPR) {
		_vm->_globals.CAT_FLAG = false;
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, v23);
	} else {
		_vm->_globals.CAT_FLAG = true;
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPANIM, "RES_SAN.RES");
	}
	PERSOSPR = _vm->_objectsManager.CHARGE_SPRITE(_vm->_globals.NFICHIER);
	_vm->_globals.CAT_FLAG = false;

	_vm->_fileManager.CONSTRUIT_LINUX("TEMP.SCR");
	if (_vm->_graphicsManager.nbrligne == SCREEN_WIDTH)
		_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x4B000u);
	if (_vm->_graphicsManager.nbrligne == (SCREEN_WIDTH * 2))
		_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, _vm->_graphicsManager.VESA_SCREEN, 0x96000u);

	if (!_vm->_graphicsManager.nbrligne)
		_vm->_graphicsManager.ofscroll = 0;
	_vm->_graphicsManager.NB_SCREEN();
	v10 = 20 * (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
	PCHERCHE = 20 * (int16)READ_LE_UINT16((uint16 *)BUFFERPERSO + 42) + 110;
	_vm->_graphicsManager.NB_SCREEN();
	_vm->_objectsManager.PERSO_ON = true;
	CHERCHE_PAL(v10, 1);
	CHERCHE_ANIM0(v10, 0);
	v11 = _vm->_globals.COUCOU;
	_vm->_globals.COUCOU = g_PTRNUL;
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager.INILINK(v22);
	_vm->_objectsManager.PERSO_ON = true;
	_vm->_globals.GOACTION = 0;
	_vm->_objectsManager.NUMZONE = -1;
	ANIM_PERSO_INIT();
	VISU_PARLE();
	VISU_WAIT();
	_vm->_graphicsManager.INI_ECRAN2(v22);
	_vm->_globals.NOMARCHE = true;
	_vm->_objectsManager.FORCEZONE = 1;
	_vm->_objectsManager.NUMZONE = -1;
	do {
		v12 = _vm->_eventsManager.BMOUSE();
		if (v12 == 1) {
			_vm->_objectsManager.BTGAUCHE();
			v12 = 1;
		}
		if (v12 == 2)
			_vm->_objectsManager.BTDROITE();
		_vm->_objectsManager.VERIFZONE();
		if (_vm->_globals.GOACTION == 1)
			_vm->_objectsManager.PARADISE();
		_vm->_eventsManager.VBL();
	} while (!_vm->_globals.SORTIE);
	FIN_VISU_PARLE();
	FIN_VISU_WAIT();
	CLEAR_ANIM_PERSO();
	CLEAR_ANIM_PERSO();
	_vm->_globals.NOPARLE = false;
	_vm->_globals.NECESSAIRE = 1;
	BUFFERPERSO = _vm->_globals.LIBERE_FICHIER(BUFFERPERSO);
	PERSOSPR = _vm->_globals.LIBERE_FICHIER(PERSOSPR);
	_vm->_graphicsManager.NB_SCREEN();
	_vm->_globals.NECESSAIRE = 0;
	_vm->_linesManager.CLEAR_ZONE();
	_vm->_linesManager.RESET_OBSTACLE();
	_vm->_globals.RESET_CACHE();
	v13 = 0;
	do
		_vm->_globals.BOBZONE[v13++] = 0;
	while (v13 <= 44);
	_vm->_globals.LIBERE_FICHIER(_vm->_globals.COUCOU);
	_vm->_globals.COUCOU = v11;
	_vm->_objectsManager.DESACTIVE = true;
	_vm->_objectsManager.INILINK(v20);
	_vm->_graphicsManager.INI_ECRAN2(v20);
	_vm->_objectsManager.DESACTIVE = false;
	_vm->_globals.NOMARCHE = false;
	if (_vm->_globals.SORTIE == 101)
		_vm->_globals.SORTIE = 0;

	_vm->_saveLoadManager.bload("TEMP.SCR", _vm->_graphicsManager.VESA_SCREEN);
	g_system->getSavefileManager()->removeSavefile("TEMP.SCR");

	_vm->_objectsManager.PERSO_ON = false;
	_vm->_eventsManager.btsouris = 4;
	_vm->_eventsManager.CHANGE_MOUSE(4);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.INIT_TABLE(145, 150, _vm->_graphicsManager.Palette);
	_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	_vm->_graphicsManager.DD_Lock();
	if (_vm->_graphicsManager.Winbpp == 2) {
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll16A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
		else
			_vm->_graphicsManager.m_scroll16(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
	}
	if (_vm->_graphicsManager.Winbpp == 1) {
		if (_vm->_graphicsManager.SDL_ECHELLE)
			_vm->_graphicsManager.m_scroll2A(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
		else
			_vm->_graphicsManager.m_scroll2(_vm->_graphicsManager.VESA_SCREEN, _vm->_eventsManager.start_x, 0, 640, 480, 0, 0);
	}
	_vm->_graphicsManager.DD_Unlock();
	_vm->_graphicsManager.setpal_vga256(_vm->_graphicsManager.Palette);
	v14 = _vm->_graphicsManager.VESA_BUFFER;
	v15 = _vm->_graphicsManager.VESA_SCREEN;
	memcpy(_vm->_graphicsManager.VESA_BUFFER, _vm->_graphicsManager.VESA_SCREEN, 0x95FFCu);
	v15 = v15 + 614396;
	v14 = v14 + 614396;
	WRITE_LE_UINT16(v14, (int16)READ_LE_UINT16(v15));
	v14 = v14 + 2;
	*v14 = *(v15 + 2);
	v16 = v14 + 1;
	_vm->_globals.DESACTIVE_INVENT = false;
	_vm->_graphicsManager.DD_VBL();
	v17 = 0;
	do {
		_vm->_eventsManager.VBL();
		++v17;
	} while (v17 <= 4);
	_vm->_graphicsManager.no_scroll = 0;
}

} // End of namespace Hopkins
