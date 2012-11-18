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
#include "common/file.h"
#include "common/textconsole.h"
#include "hopkins/font.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

namespace Hopkins {

ComputerManager::ComputerManager() {
	for (int i = 0; i < 50; i++) {
		MenuText[i]._actvFl = false;
		MenuText[i]._lineSize = 0;
		memset(MenuText[i]._line, 0, 90);
	}
	Common::fill(&Sup_string[0], &Sup_string[200], '\0');
	CASSESPR = NULL;
	FMOUSE = false;
	TEXT_COL = 0;
	CASSETAB = 0;
	NBBRIQUES = 0;
	CASSESCORE = 0;
	CASSEVIE = 0;
	CASSESPEED = 0;
	BALLEHORI = 0;
	BALLEVERTI = 0;
	NB_TABLE = 0;
	RAQX = 0;
	CASSEP1 = 0;
	CASSEP2 = 0;
	CASSDER = 0;
	Menu_lignes = 0;
}

void ComputerManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

void ComputerManager::setvideomode() {
	TEXT_MODE();
}

void ComputerManager::TEXT_MODE() {
	_vm->_graphicsManager.Cls_Pal();
	_vm->_graphicsManager.DD_Lock();
	_vm->_graphicsManager.Cls_Video();
	_vm->_graphicsManager.DD_Unlock();
	//SET_MODE(SCREEN_WIDTH, SCREEN_HEIGHT);

	_vm->_graphicsManager.nbrligne = SCREEN_WIDTH;
	_vm->_fileManager.CONSTRUIT_SYSTEM("STFONT.SPR");
	_vm->_globals.police = _vm->_globals.dos_free2(_vm->_globals.police);
	_vm->_globals.police = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_globals.police_l = 8;
	_vm->_globals.police_h = 8;
	_vm->_graphicsManager.LOAD_IMAGE("WINTEXT");
	_vm->_graphicsManager.FADE_INW();
	Charge_Menu();
	_vm->_eventsManager.souris_flag = false;
}

void ComputerManager::clearscreen() {
	_vm->_graphicsManager.LOAD_IMAGE("WINTEXT");
	_vm->_graphicsManager.FADE_INW();
}

void ComputerManager::settextcolor(int col) {
	TEXT_COL = col;
}

void ComputerManager::settextposition(int xp, int yp) {
	_textPosition.x = xp << 3;
	_textPosition.y = yp << 4;
}

void ComputerManager::COMPUT_HOPKINS(int mode) {
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
	char s[12]; 
	const char *s2;

	_vm->_eventsManager.ESC_KEY = 0;
	v2 = 0;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	setvideomode();
	settextcolor(4);

	_vm->_eventsManager.videkey();
	settextposition(2, 4);
	if (mode == 1)
		outtext(Common::String(MenuText[0]._line));
	if (mode == 2)
		outtext(Common::String(MenuText[1]._line));
	if (mode == 3)
		outtext(Common::String(MenuText[2]._line));

	settextcolor(1);
	if (mode == 3) {
		settextposition(10, 8);
		outtext(Common::String(MenuText[3]._line));
	}
	settextposition(12, 28);
	outtext(Common::String(MenuText[4]._line));
	settextposition(14, 35);

	v3 = &s[0];
	memset(s, 0, 8u);
	TXT4(280, 224, 8);

	strcpy(s, Sup_string);

	if (mode == 1) {
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
	if (mode == 2) {
		v6 = &s[0];
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
	if (mode == 3) {
		v9 = &s[0];
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
		while (!_vm->shouldQuit()) {
			_vm->_eventsManager.ESC_KEY = 0;
			_vm->_eventsManager.videkey();
			clearscreen();
			settextcolor(4);
			settextposition(2, 4);
			if (mode == 1)
				outtext(Common::String(MenuText[0]._line));
			if (mode == 2)
				outtext(Common::String(MenuText[1]._line));
			if (mode == 3)
				outtext(Common::String(MenuText[2]._line));
			settextcolor(15);
			settextposition(8, 25);
			settextcolor(15);
			outtext2(Common::String(MenuText[6]._line));
			settextposition(20, 25);
			outtext2(Common::String(MenuText[7]._line));
			if (mode == 1) {
				settextposition(10, 25);
				outtext2(Common::String(MenuText[8]._line));
				settextposition(12, 25);
				outtext2(Common::String(MenuText[9]._line));
				settextposition(14, 25);
				outtext2(Common::String(MenuText[10]._line));
				settextposition(16, 25);
				outtext2(Common::String(MenuText[11]._line));
			}
			if (mode == 2) {
				_vm->_eventsManager.videkey();
				settextposition(10, 25);
//				outtext2(Common::String(MenuText[0x95A])); <=== CHECKME: Unexpected value! replaced by the following line, for consistancy
				outtext2(Common::String(MenuText[12]._line));
				settextposition(12, 25);
				outtext2(Common::String(MenuText[13]._line));
				settextposition(14, 25);
				outtext2(Common::String(MenuText[14]._line));
				settextposition(16, 25);
				outtext2(Common::String(MenuText[15]._line));
				settextposition(18, 25);
				outtext2(Common::String(MenuText[16]._line));
			}

			do {
				v12 = _vm->_eventsManager.keywin();
				if (_vm->shouldQuit())
					return;

				v2 = 0;
				if ((uint16)(v12 - 48) <= 8u)
					v2 = 1;
			} while (v2 != 1);

			if (v12 == 48)
				break;
			if (v12 == 49) {
				GAMES();
			} else if (mode == 1) {
				_vm->_eventsManager.videkey();
				clearscreen();
				settextcolor(4);
				settextposition(2, 4);
				outtext(Common::String(MenuText[0]._line));
				settextcolor(15);
				if (v12 == 50)
					LIT_TEXTE(1);
				if (v12 == 51)
					LIT_TEXTE(2);
				if (v12 == 52)
					LIT_TEXTE(3);
				if (v12 == 53)
					LIT_TEXTE(4);
			} else if (mode == 2) {
				clearscreen();
				settextcolor(4);
				settextposition(2, 4);
				outtext(Common::String(MenuText[1]._line));
				settextcolor(15);
				if (v12 == 50)
					LIT_TEXTE(6);
				if (v12 == 51)
					LIT_TEXTE(7);
				if (v12 == 52)
					LIT_TEXTE(8);
				if (v12 == 53)
					LIT_TEXTE(9);
				if (v12 == 54) {
					LIT_TEXTE(10);
					_vm->_globals.SAUVEGARDE->data[svField270] = 4;
				}
			}
		}
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		RESTORE_POLICE();
	} else {
		settextcolor(4);
		settextposition(16, 25);
		outtext(Common::String(MenuText[5]._line));
		_vm->_eventsManager.VBL();
		memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x4AFFFu);
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		RESTORE_POLICE();
		_vm->_eventsManager.MOUSE_OFF();
	}
	if (mode == 1)
		_vm->_globals.SORTIE = 13;
	if ((uint16)(mode - 2) <= 1u)
		_vm->_globals.SORTIE = 14;
	
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

void ComputerManager::Charge_Menu() {
	_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "COMPUTAN.TXT");
	byte *ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	byte *tmpPtr = ptr;
	int lineNum = 0;
	int strPos;
	bool loopCond = false;

	do {
		if (tmpPtr[0] == '%') {
			if (tmpPtr[1] == '%') {
				loopCond = true;
				goto LABEL_13;
			}
			MenuText[lineNum]._actvFl = 1;
			strPos = 0;
			while (1) {
				byte curChar = tmpPtr[strPos + 2];
				if (curChar == '%' || curChar == 10)
					break;
				MenuText[lineNum]._line[strPos++] = curChar;
				if (strPos > 89)
					goto LABEL_11;
			}
			MenuText[lineNum]._line[strPos] = 0;
			MenuText[lineNum]._lineSize = strPos - 1;
	LABEL_11:
			++lineNum;
		}
		Menu_lignes = lineNum;
	LABEL_13:
		tmpPtr = tmpPtr + 1;
	} while (!loopCond);
	_vm->_globals.dos_free2(ptr);
}

void ComputerManager::TXT4(int xp, int yp, int a3) {
	int v3; 
	bool v4, v12;
	char v5; 
	int v6; 
	char v7; 
	int v8; 
	byte v9; 
	int textIndex;
	char newChar; 
	Common::String charString;

	v3 = xp;
	textIndex = 0;
	v4 = v12 = _vm->_eventsManager.souris_flag;
	_vm->_eventsManager.souris_flag = false;

	_vm->_fontManager.TEXT_NOW(xp, yp, "_", -4);
	do {
		v5 = _vm->_eventsManager.keywin();
		if (_vm->shouldQuit())
			return;

		v6 = v5;
		v7 = 42;
		if (v5 == 45)
			v7 = 45;
		if (v5 == 48)
			v7 = 48;
		if (v5 == 49)
			v7 = 49;
		if (v5 == 50)
			v7 = 50;
		if (v5 == 51)
			v7 = 51;
		if (v5 == 52)
			v7 = 52;
		if (v5 == 53)
			v7 = 53;
		if (v5 == 54)
			v7 = 54;
		if (v5 == 55)
			v7 = 55;
		if (v5 == 56)
			v7 = 56;
		if (v5 == 57)
			v7 = 57;
		if (v5 == 65 || v5 == 97)
			v7 = 65;
		if (v5 == 66 || v5 == 98)
			v7 = 66;
		if (v5 == 67 || v5 == 99)
			v7 = 67;
		if (v5 == 68 || v5 == 100)
			v7 = 68;
		if (v5 == 69 || v5 == 101)
			v7 = 69;
		if (v5 == 70 || v5 == 102)
			v7 = 70;
		if (v5 == 71 || v5 == 103)
			v7 = 71;
		if (v5 == 72 || v5 == 104)
			v7 = 72;
		if (v5 == 73 || v5 == 105)
			v7 = 73;
		if (v5 == 74 || v5 == 106)
			v7 = 74;
		if (v5 == 75 || v5 == 107)
			v7 = 75;
		if (v5 == 76 || v5 == 108)
			v7 = 76;
		if (v5 == 77 || v5 == 109)
			v7 = 77;
		if (v5 == 78 || v5 == 110)
			v7 = 78;
		if (v5 == 79 || v5 == 111)
			v7 = 79;
		if (v5 == 80 || v5 == 112)
			v7 = 80;
		if (v5 == 81 || v5 == 113)
			v7 = 81;
		if (v5 == 82 || v5 == 114)
			v7 = 82;
		if (v5 == 83 || v5 == 115)
			v7 = 83;
		if (v5 == 84 || v5 == 116)
			v7 = 84;
		if (v5 == 85 || v5 == 117)
			v7 = 85;
		if (v5 == 86 || v5 == 118)
			v7 = 86;
		if (v5 == 87 || v5 == 119)
			v7 = 87;
		if (v5 == 88 || v5 == 120)
			v7 = 88;
		if (v5 == 89 || v5 == 121)
			v7 = 89;
		if (v5 == 90 || v5 == 122)
			v7 = 90;
		if (v5 == 8 && textIndex > 0) {
			Sup_string[textIndex--] = 0;
			v3 -= _vm->_globals.police_l;
			v8 = v3 + 2 * _vm->_globals.police_l;
			v9 = v7;
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, v3, yp, 2 * _vm->_globals.police_l, 12, _vm->_graphicsManager.VESA_BUFFER, v3, yp);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v3, yp, v8, yp + 12);
			_vm->_fontManager.TEXT_NOW(v3, yp, "_", -4);
			v7 = v9;
		}
		if (v7 != '*') {
			newChar = v7;
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, v3, yp, _vm->_globals.police_l, 12, _vm->_graphicsManager.VESA_BUFFER, v3, yp);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(v3, yp, _vm->_globals.police_l + v3, yp + 12);
			Sup_string[textIndex] = newChar;

			charString = Common::String::format("%c_", newChar);
			_vm->_fontManager.TEXT_NOW(v3, yp, charString, -4);
			++textIndex;
			v3 += _vm->_globals.police_l;
		}
		_vm->_eventsManager.VBL();
		v4 = a3;
	} while (textIndex != a3 && v6 != 13);

	_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, v3, yp, _vm->_globals.police_l, 
		12, _vm->_graphicsManager.VESA_BUFFER, v3, yp);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(v3, yp, _vm->_globals.police_l + v3, yp + 12);
	
	_vm->_eventsManager.VBL();
	Sup_string[textIndex] = 0;
	_vm->_eventsManager.souris_flag = v12;
}

void ComputerManager::outtext(const Common::String &msg) {
	_vm->_fontManager.TEXT_COMPUT(_textPosition.x, _textPosition.y, msg, TEXT_COL);
}

void ComputerManager::outtext2(const Common::String &msg) {
	_vm->_fontManager.TEXT_NOW(_textPosition.x, _textPosition.y, msg, TEXT_COL);
}

void ComputerManager::RESTORE_POLICE() {
	_vm->_globals.police = _vm->_globals.dos_free2(_vm->_globals.police);
	_vm->_fileManager.CONSTRUIT_SYSTEM("FONTE3.SPR");
	_vm->_globals.police = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);

	_vm->_globals.police_l = 12;
	_vm->_globals.police_h = 21;
	_vm->_eventsManager.souris_flag = true;
}

void ComputerManager::LIT_TEXTE(int a1) {
	uint16 v1; 
	int v2; 
	uint16 v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	int v8; 
	uint16 v10; 
	byte *ptr; 
	Common::String v12; 
	Common::String numStr; 
	int num;

	_vm->_eventsManager.ESC_KEY = false;

	if (_vm->_globals.FR == 0)
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "THOPKAN.TXT");
	if (_vm->_globals.FR == 1)
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "THOPK.TXT");
	if (_vm->_globals.FR == 2)
		_vm->_fileManager.CONSTRUIT_FICHIER(_vm->_globals.HOPLINK, "THOPKES.TXT");

	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	v1 = _vm->_fileManager.FLONG(_vm->_globals.NFICHIER);
	v2 = 0;
	v3 = 0;
	if (v1 > 0u) {
		do {
			if (*(ptr + v3) == '%') {
				numStr = Common::String::format("%c%c", *(ptr + v3 + 1), *(ptr + v3 + 2));
				num	= atol(numStr.c_str());

				if (num == a1)
					v2 = 1;
			}
			if (v2 == 1)
				break;
			++v3;
		} while (v3 < v1);
	}
	v4 = v3;
	if (v3 > v1 - 1)
		error("Error with Hopkins computer file");
		
	v10 = v3 + 3;
	v5 = 1;
	v6 = 5;
	v7 = 0;
	do {
		v4 = *(ptr + v10);
		if (v4 == 13) {
//			v12[v7] = 0;
//			v7 = 0;
			v8 = v4;
			settextposition(v6, v5);
			outtext(v12);

			++v6;
			v5 = 1;
			_vm->_eventsManager.VBL();
			v4 = v8;
			v12 = "";
		} else if (v4 != '%') {
			//v12[v7++] = v4;
			v12 += v4;
			++v7;
		}
		++v10;
	} while (v4 != 37);

	_vm->_eventsManager.videkey();
	_vm->_eventsManager.wingetch();
	_vm->_globals.dos_free2(ptr);
}

void ComputerManager::GAMES() {
	const byte *v1; 
	int v2; 

	v1 = _vm->_objectsManager.Sprite[0].spriteData;
	v2 = _vm->_globals.vitesse;
	_vm->_globals.vitesse = 1;
	_vm->_eventsManager.CHANGE_MOUSE(0);
	CASSESPR = g_PTRNUL;
	_vm->_eventsManager.CASSE = 1;
	_vm->_eventsManager.CASSE_SOURIS_ON();
	CASSETAB = g_PTRNUL;
	NBBRIQUES = 0;
	CASSESCORE = 0;
	CASSEVIE = 5;
	CASSESPEED = 1;
	BALLEHORI = 0;
	BALLEVERTI = 0;
	NB_TABLE = 0;
	_vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_x = 320;
	_vm->_graphicsManager.max_y = 200;
	_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND37.WAV");
	_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND38.WAV");
	_vm->_soundManager.CHARGE_SAMPLE(3, "SOUND39.WAV");
	_vm->_fileManager.CONSTRUIT_SYSTEM("_vm->_eventsManager.CASSE.SPR");
	CASSESPR = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	CHARGE_SCORE();
	MODE_VGA256();
	NEWTAB();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	PLAY_BRIQUE();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	CASSESPR = _vm->_globals.LIBERE_FICHIER(CASSESPR);
	CASSETAB = _vm->_globals.dos_free2(CASSETAB);
	_vm->_objectsManager.Sprite[0].spriteData = v1;

	_vm->_soundManager.DEL_SAMPLE(1);
	_vm->_soundManager.DEL_SAMPLE(2);
	_vm->_soundManager.DEL_SAMPLE(3);
	_vm->_globals.vitesse = v2;
	_vm->_eventsManager.CASSE = 0;
	_vm->_eventsManager.CASSE_SOURIS_OFF();
	setvideomode();
	settextcolor(15);
	clearscreen();
	_vm->_graphicsManager.max_x = 680;
	_vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_y = 460;
}

void ComputerManager::CHARGE_SCORE() {
	int v0; 
	int v1; 
	char v2; 
	int v3; 
	int v4; 
	char v5; 
	byte *ptr; 
	int v8; 

	_vm->_fileManager.CONSTRUIT_LINUX("HISCORE.DAT");
	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	v8 = 0;
	do {
		v0 = 16 * v8;
		v1 = 0;
		do {
			v2 = *(ptr + v1 + (int)(16 * v8));
			if (!v2)
				v2 = 32;
			*(&Score[v0] + v1++) = v2;
		} while (v1 <= 4);
		v3 = v0 + 6;
		v4 = 0;
		do {
			v5 = *(ptr + v4 + v3);
			if (!v5)
				v5 = 48;
			*(&Score[16 * v8 + 6] + v4++) = v5;
		} while (v4 <= 8);
		*(ptr + v3 + 9) = 0;
		++v8;
	} while (v8 <= 5);

	_vm->_globals.dos_free2(ptr);
	CASSE_HISCORE = atol((const char *)&Score[86]);
}

void ComputerManager::MODE_VGA256() {
	_vm->_graphicsManager.DD_Lock();
	_vm->_graphicsManager.Cls_Video();
	_vm->_graphicsManager.DD_Unlock();
	_vm->_graphicsManager.Cls_Pal();
	_vm->_graphicsManager.SCANLINE(0x140u);
	_vm->_fileManager.DMESS();
}

void ComputerManager::NEWTAB() {
	Common::String file;
	Common::File f;

	_vm->_objectsManager.SPRITE_OFF(0);
	_vm->_objectsManager.SPRITE_OFF(1);
	++CASSEVIE;
	if (CASSEVIE > 11)
		CASSEVIE = 11;
	_vm->_graphicsManager.LOAD_IMAGEVGA("CASSEF.PCX");
	AFF_VIE();
	if (g_PTRNUL != CASSETAB)
		CASSETAB = _vm->_globals.dos_free2(CASSETAB);

	++NB_TABLE;
	while (!_vm->shouldQuit()) {
		file = Common::String::format("TAB%d.TAB", NB_TABLE);

		_vm->_fileManager.CONSTRUIT_SYSTEM(file);
		if (f.open(_vm->_globals.NFICHIER))
			break;

		NB_TABLE = 1;
	}
	f.close();

	CASSETAB = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	AFF_BRIQUES();
	_vm->_objectsManager.SPRITE(CASSESPR, 150, 192, 0, 13, 0, 0, 0, 0);
	_vm->_objectsManager.SPRITE(CASSESPR, 164, 187, 1, 14, 0, 0, 0, 0);
	BALLE = Common::Point(164, 187);
	RAQX = 150;
	_vm->_objectsManager.SPRITE_ON(0);
	_vm->_objectsManager.SPRITE_ON(1);
	_vm->_eventsManager.MOUSE_ON1();
	FMOUSE = true;
	_vm->_soundManager.PLAY_SAMPLE(3, 5);
}

void ComputerManager::AFF_BRIQUES() {
	int v0; 
	int v1; 
	int v2; 
	uint16 v3; 
	byte *v4; 

	NBBRIQUES = 0;
	CASSESPEED = 1;
	v4 = CASSETAB;
	v3 = 0;
	do {
		v0 = *((uint16 *)v4 + v3);
		v1 = *((uint16 *)v4 + v3 + 1);
		v2 = *((uint16 *)v4 + v3 + 4);
		if (v0 != -1) {
			if (v2 <= 6)
				++NBBRIQUES;
			if (v2 == 3)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 17);
			if (v2 == 6)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 18);
			if (v2 == 5)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 19);
			if (v2 == 4)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 20);
			if (v2 == 1)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 21);
			if (v2 == 2)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 22);
			if (v2 == 31)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v0, v1, 23);
		}
		v3 += 6;
	} while (v0 != -1);
	IMPRIMESCORE();
}

void ComputerManager::AFF_VIE() {
	int v0; 
	int v1; 
	int v2; 
	int v3; 
	int v4; 

	v0 = CASSEVIE - 1;
	v1 = 10;
	v2 = 0;
	do {
		_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v1, 10, 15);
		v1 += 7;
		++v2;
	} while (v2 <= 11);
	if (v0) {
		v3 = 10;
		v4 = 0;
		if (v0 > 0) {
			do {
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v3, 10, 14);
				v3 += 7;
				++v4;
			} while (v4 < v0);
		}
	}
}

void ComputerManager::PLAY_BRIQUE() {
	int v1 = 0; 
	int v; 

	while (!_vm->shouldQuit()) {
		while (!_vm->shouldQuit()) {
			_vm->_eventsManager.MOUSE_OFF();
			BALLE = Common::Point(RAQX + 14, 187);
			_vm->_objectsManager.SETYSPR(1, 187);
			_vm->_objectsManager.SETXSPR(1, BALLE.x);
			_vm->_graphicsManager.RESET_SEGMENT_VESA();
			_vm->_eventsManager.VBL();
			_vm->_graphicsManager.FADE_IN_CASSE();
			do {
				RAQX = _vm->_eventsManager.souris_x;
				if (_vm->_eventsManager.souris_x <= 4)
					RAQX = 5;
				if (RAQX > 282)
					RAQX = 282;
				_vm->_objectsManager.SETXSPR(0, RAQX);
				_vm->_objectsManager.SETXSPR(1, RAQX + 14);
				_vm->_objectsManager.SETYSPR(1, 187);
				_vm->_eventsManager.VBL();
			} while (_vm->_eventsManager.BMOUSE() != 1);

			CASSESPEED = 1;
			BALLE = Common::Point(RAQX + 14, 187);
			BALLEHORI = RAQX > 135;
			BALLEVERTI = 0;
		
			do {
				RAQX = _vm->_eventsManager.souris_x;
				if ((int)_vm->_eventsManager.souris_x <= 4)
					RAQX = 5;
				if (RAQX > 282)
					RAQX = 282;
				_vm->_objectsManager.SETXSPR(0, RAQX);
				v1 = DEP_BALLE();
				_vm->_eventsManager.VBL();
			} while (!_vm->shouldQuit() && !v1);
			if (v1 != 1)
				break;
			_vm->_graphicsManager.FADE_OUT_CASSE();
			--CASSEVIE;
			if (CASSEVIE) {
				AFF_VIE();
				if (CASSEVIE)
					continue;
			}
			_vm->_eventsManager.MOUSE_ON1();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.SPRITE_OFF(1);
			if (CASSESCORE > CASSE_HISCORE)
				NAME_SCORE();
			v = HIGHT_SCORE();
			if (v != 1)
				break;

			NBBRIQUES = 0;
			CASSESCORE = 0;
			CASSEVIE = 4;
			CASSESPEED = 1;
			BALLEHORI = 0;
			BALLEVERTI = 0;
			NB_TABLE = 0;
			CHARGE_SCORE();
			NEWTAB();
		}
		if (v1 != 2)
			return;
		_vm->_graphicsManager.FADE_OUT_CASSE();
		NEWTAB();
	}
}

int ComputerManager::HIGHT_SCORE() {
	int v0; 
	int v1; 
	int v2; 
	int v3; 
	int v4; 
	int v6; 
	byte *ptr; 

	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	CHARGE_SCORE();
	_vm->_graphicsManager.LOAD_IMAGEVGA("HISCORE.PCX");
	_vm->_fileManager.CONSTRUIT_SYSTEM("ALPHA.SPR");
	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	v6 = 0;
	do {
		v0 = 19 * v6;
		v0 = v0 + 46;
		v1 = 0;
		do {
			PRINT_HSCORE(ptr, 9 * v1 + 69, v0, *(&Score[16 * v6] + v1));
			++v1;
		} while (v1 <= 5);
		v2 = 0;
		do {
			PRINT_HSCORE(ptr, 9 * v2 + 199, v0, *(&Score[16 * v6 + 6] + v2));
			++v2;
		} while (v2 <= 8);
		++v6;
	} while (v6 <= 5);
	_vm->_graphicsManager.FADE_IN_CASSE();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	v3 = 0;
	do {
		v4 = _vm->_eventsManager.souris_x + 13;
		v0 = _vm->_eventsManager.souris_y;
		if (_vm->_eventsManager.BMOUSE() == 1 && (uint16)(v4 - 15) <= 0x21u && (uint16)(v0 - 176) <= 0xDu)
			v3 = 1;
		if (_vm->_eventsManager.BMOUSE() == 1
		        && (uint16)(v4 - 274) <= 0x20u
		        && (uint16)(v0 - 176) <= 0xDu)
			v3 = 2;
		_vm->_eventsManager.VBL();
	} while (!v3);
	_vm->_eventsManager.MOUSE_OFF();
	_vm->_graphicsManager.FADE_OUT_CASSE();
	_vm->_globals.LIBERE_FICHIER(ptr);
	return v3;
}

void ComputerManager::NAME_SCORE() {
	int v0; 
	int v1; 
	int v2; 
	int v3; 
	int i; 
	byte *ptr; 
	char s[16]; 

	_vm->_graphicsManager.LOAD_IMAGEVGA("NAME.PCX");
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_fileManager.CONSTRUIT_SYSTEM("ALPHA.SPR");
	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.FADE_IN_CASSE();
	v0 = 0;
	do {
		PRINT_HSCORE(ptr, 9 * v0 + 140, 78, 1);

		v1 = toupper(_vm->_eventsManager.keywin());
		if (v1 <= 47)
			v1 = 32;
		if (v1 > 90)
			v1 = 32;
		if ((uint16)(v1 - 58) <= 6u)
			v1 = 32;
		Score[v0 + 80] = v1;
		PRINT_HSCORE(ptr, 9 * v0 + 140, 78, v1);

		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.VBL();
		++v0;
	} while (v0 <= 4);
	memset(&Score[86], 32, 9u);
	sprintf(s, "%d", CASSESCORE);
	v2 = 0;
	do
		++v2;
	while (s[v2]);
	v3 = 8;
	for (i = v2; ; Score[v3-- + 86] = s[i]) {
		--i;
		if (i <= -1)
			break;
	}
	_vm->_graphicsManager.FADE_OUT_CASSE();
	_vm->_globals.dos_free2(ptr);
	SAUVE_SCORE();
}

void ComputerManager::IMPRIMESCORE() {
	__int16 v0; 
	__int16 v1; 
	__int16 i; 
	char s[40]; 

	sprintf(s, "%d", CASSESCORE);
	v0 = 0;
	do
		++v0;
	while (s[v0]);
	v1 = 0;
	for (i = v0; ; IMPSCORE(v1++, (byte)s[i])) {
		--i;
		if (i <= -1)
			break;
	}
}

void ComputerManager::IMPSCORE(int a1, int a2) {
	signed __int16 v2; 
	__int16 v3; 

	v2 = 203;
	v3 = 3;
	if (a1 == 1)
		v2 = 193;
	if (a1 == 2)
		v2 = 183;
	if (a1 == 3)
		v2 = 170;
	if (a1 == 4)
		v2 = 160;
	if (a1 == 5)
		v2 = 150;
	if (a1 == 9)
		v2 = 137;
	if (a2 == 49)
		v3 = 4;
	if (a2 == 50)
		v3 = 5;
	if (a2 == 51)
		v3 = 6;
	if (a2 == 52)
		v3 = 7;
	if (a2 == 53)
		v3 = 8;
	if (a2 == 54)
		v3 = 9;
	if (a2 == 55)
		v3 = 10;
	if (a2 == 56)
		v3 = 11;
	if (a2 == 57)
		v3 = 12;
	_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v2 - 3, 11, v3);
}

void ComputerManager::SAUVE_SCORE() {
	int v0; 
	int v1; 
	int v2; 
	int v3; 
	int v4; 
	int v5; 
	int v6; 
	int v7; 
	char v8; 
	int v9; 
	int v10; 
	char v11; 
	int v13; 
	int v14; 
	byte *ptr; 
	int v16[6]; 
	int v17[6]; 

	v0 = 0;
	do {
		v1 = atol((const char *)(16 * v0 + Score + 6));
		v17[v0] = v1;
		if (!v1)
			v17[v0] = 5;
		++v0;
	} while (v0 <= 5);
	v13 = 0;
	do {
		v2 = 0;
		v3 = 0;
		do {
			v4 = v17[v2];
			if (v4 && v17[0] <= v4 && v17[1] <= v4 && v17[2] <= v4 && v17[3] <= v4 
					&& v17[4] <= v4 && v17[5] <= v4)
				v3 = 1;
			if (v3 == 1) {
				v16[v13] = v2;
				v17[v2] = 0;
			} else {
				++v2;
			}
		} while (v3 != 1);
		++v13;
	} while (v13 <= 5);

	ptr = _vm->_globals.dos_malloc2(0x64u);
	memset(ptr, 0, 0x63u);
	v5 = 0;
	do {
		v6 = 16 * v5;
		v14 = v16[v5];
		v7 = 0;
		do {
			v8 = *(&Score[16 * v14] + v7);
			if (!v8)
				v8 = 32;
			*(ptr + (16 * v5) + v7++) = v8;
		} while (v7 <= 4);

		*(ptr + v6 + 5) = 0;
		v9 = v6 + 6;
		v10 = 0;
		do {
			v11 = *(&Score[16 * v14 + 6] + v10);
			if (!v11)
				v11 = 48;
			*(ptr + v9 + v10++) = v11;
		} while (v10 <= 8);
		*(ptr + v9 + 9) = 0;
		++v5;
	} while (v5 <= 5);

	_vm->_fileManager.CONSTRUIT_LINUX("HISCORE.DAT");
	_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, ptr, 0x64u);
	_vm->_globals.dos_free2(ptr);
}

void ComputerManager::PRINT_HSCORE(byte *objectData, int a2, int a3, int a4) {
	char v4; 
	int v5; 

	v4 = a4;
	v5 = 36;
	if (!a4)
		v4 = 32;
	if (v4 == 100)
		v5 = 0;
	if ((byte)(v4 - 48) <= 9u)
		v5 = (byte)v4 - 48;
	if ((byte)(v4 - 65) <= 0x19u)
		v5 = (byte)v4 - 55;
	if (v4 == 32)
		v5 = 36;
	if (v4 == 1)
		v5 = 37;
	_vm->_graphicsManager.AFFICHE_SPEEDVGA(objectData, a2, a3, v5);
}

int ComputerManager::DEP_BALLE() {
	int v0; 
	__int16 v1; 
	int v2; 
	signed __int16 v4; 

	v4 = 0;
	//(signed int)(6.0 * (long double)_vm->getRandomNumber( rand() / 2147483648.0) + 1;
	// TODO: Figure out random number
	v0 = _vm->getRandomNumber(6); 
	if (CASSESPEED == 1) {
		CASSEP1 = 1;
		CASSEP2 = 1;
	}
	if (CASSESPEED == 2) {
		CASSEP1 = 1;
		CASSEP2 = 2;
	}
	if (CASSESPEED == 3) {
		CASSEP1 = 2;
		CASSEP2 = 2;
	}
	if (CASSESPEED == 4) {
		CASSEP1 = 3;
		CASSEP2 = 2;
	}
	v1 = CASSEP1;
	if (CASSDER == CASSEP1)
		v1 = CASSEP2;
	if (BALLEVERTI == 1)
		BALLE.y += v1;
	if (!BALLEVERTI)
		BALLE.y -= v1;
	if (BALLEHORI == 1)
		BALLE.x += v1;
	if (!BALLEHORI)
		BALLE.x -= v1;
	CASSDER = v1;
	if (BALLE.x <= 6) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		BALLE.x = v0 + 6;
		BALLEHORI = BALLEHORI != 1;
	}
	if (BALLE.x > 307) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		BALLE.x = 307 - v0;
		BALLEHORI = BALLEHORI != 1;
	}
	if (BALLE.y <= 6) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		BALLE.y = v0 + 7;
		BALLEVERTI = BALLEVERTI != 1;
	}
	if ((uint16)(BALLE.y - 186) <= 8u) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		if (BALLE.x > RAQX - 2) {
			v2 = BALLE.x + 6;
			if (v2 < RAQX + 36) {
				BALLEVERTI = 0;
				if (v2 <= RAQX + 15) {
					BALLEHORI = 0;
					if (BALLE.x >= RAQX && v2 <= RAQX + 5)
						BALLE.x -= 4;
					if (BALLE.x >= RAQX + 5 && BALLE.x + 6 <= RAQX + 10)
						BALLE.x -= 2;
				}
				if (BALLE.x >= RAQX + 19 && BALLE.x + 6 <= RAQX + 36) {
					BALLEHORI = 1;
					if (BALLE.x >= RAQX + 29)
						BALLE.x += 4;
					if (BALLE.x >= RAQX + 24 && BALLE.x + 6 <= RAQX + 29)
						BALLE.x += 2;
				}
			}
		}
	}
	if (BALLE.y > 194)
		v4 = 1;
	VERIFBRIQUES();
	_vm->_objectsManager.SETXSPR(1, BALLE.x);
	_vm->_objectsManager.SETYSPR(1, BALLE.y);
	if (!NBBRIQUES)
		v4 = 2;
	return v4;
}



void ComputerManager::VERIFBRIQUES() {
	__int16 v0; 
	__int16 v1; 
	__int16 v2; 
	__int16 v3; 
	signed __int16 v4; 
	__int16 v5; 
	__int16 v6; 
	signed __int16 v7; 
	uint16 v8; 
	byte *v9; 
	__int16 v10; 
	__int16 v11; 
	__int16 v12; 
	__int16 v13; 

	v7 = 0;
	//v6 = (signed int)(6.0 * (long double)rand() / 2147483648.0) + 1;
	// TODO: Check if correct
	v6 = _vm->getRandomNumber(6) + 1;
	v0 = BALLE.x;
	v13 = BALLE.y;
	v5 = BALLE.x + 6;
	v12 = BALLE.y + 6;
	v9 = CASSETAB;
	v8 = 0;
	do {
		v1 = *((uint16 *)v9 + v8);
		v11 = *((uint16 *)v9 + v8 + 1);
		v2 = *((uint16 *)v9 + v8 + 2);
		v10 = *((uint16 *)v9 + v8 + 3);
		v3 = *((uint16 *)v9 + v8 + 4);
		if (*((uint16 *)v9 + v8 + 5) != 1 || v1 == -1)
			goto LABEL_60;
		v4 = 0;
		if (v13 <= v10 && v12 >= v10) {
			if (v0 >= v1 && v5 <= v2) {
				v4 = 1;
				BALLEVERTI = 1;
			}
			if (v5 >= v1) {
				if (v0 <= v1) {
					++v4;
					BALLEVERTI = 1;
					BALLEHORI = 0;
					if (v3 == 31)
						BALLE.x -= v6;
				}
			}
			if (v0 <= v2) {
				if (v5 >= v2) {
					++v4;
					BALLEVERTI = 1;
					BALLEHORI = 1;
					if (v3 == 31)
						BALLE.x += v6;
				}
			}
		}
		if (v12 >= v11) {
			if (v13 > v11)
				goto LABEL_31;
			if (v0 >= v1 && v5 <= v2) {
				++v4;
				BALLEVERTI = 0;
			}
			if (v5 >= v1) {
				if (v0 <= v1) {
					++v4;
					BALLEVERTI = 0;
					BALLEHORI = 0;
					if (v3 == 31)
						BALLE.x -= 2;
				}
			}
			if (v0 <= v2) {
				if (v5 >= v2) {
					++v4;
					BALLEVERTI = 0;
					BALLEHORI = 1;
					if (v3 == 31)
						BALLE.x += v6;
				}
			}
		}
		if (v13 >= v11) {
LABEL_31:
			if (v12 <= v10) {
				if (v5 >= v1) {
					if (v0 <= v1) {
						++v4;
						BALLEHORI = 0;
						if (v3 == 31)
							BALLE.x -= v6;
					}
				}
				if (v0 <= v2) {
					if (v5 >= v2) {
						++v4;
						BALLEHORI = 1;
						if (v3 == 31)
							BALLE.x += v6;
					}
				}
			}
		}
		if (v4) {
			if (v3 == 31) {
				_vm->_soundManager.PLAY_SAMPLE(2, 6);
			} else {
				_vm->_soundManager.PLAY_SAMPLE(1, 5);
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v1, v11, 16);
				if (v3 == 1)
					CASSESCORE += 10;
				if (v3 == 2)
					CASSESCORE += 5;
				if (v3 == 3) {
					CASSESCORE += 50;
					if (CASSESPEED <= 1)
						CASSESPEED = 2;
					if (NBBRIQUES <= 19)
						CASSESPEED = 3;
				}
				if (v3 == 4)
					CASSESCORE += 20;
				if (v3 == 5) {
					CASSESCORE += 30;
					if (CASSESPEED <= 1)
						CASSESPEED = 2;
				}
				if (v3 == 6)
					CASSESCORE += 40;
				IMPRIMESCORE();
				--NBBRIQUES;
				*((uint16 *)v9 + v8 + 5) = 0;
				v7 = 1;
			}
		}
LABEL_60:
		if (v7 == 1)
			v1 = -1;
		v8 += 6;
	} while (v1 != -1);
}

} // End of namespace Hopkins
