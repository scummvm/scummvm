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

void ComputerManager::settextposition(int yp, int xp) {
	_textPosition.x = xp << 3;
	_textPosition.y = yp << 4;
}

void ComputerManager::COMPUT_HOPKINS(ComputerEnum mode) {
	bool passwordMatch; 
	char *v3; 
	char s[12]; 
	const char *s2;

	_vm->_eventsManager.ESC_KEY = 0;
	passwordMatch = false;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	setvideomode();
	settextcolor(4);

	_vm->_eventsManager.videkey();
	settextposition(2, 4);
	if (mode == COMPUTER_HOPKINS)
		outtext(Common::String(MenuText[0]._line));
	if (mode == COMPUTER_SAMANTHAS)
		outtext(Common::String(MenuText[1]._line));
	if (mode == COMPUTER_PUBLIC)
		outtext(Common::String(MenuText[2]._line));

	settextcolor(1);
	if (mode == COMPUTER_PUBLIC) {
		settextposition(10, 8);
		outtext(Common::String(MenuText[3]._line));
	}
	settextposition(12, 28);
	outtext(Common::String(MenuText[4]._line));
	settextposition(14, 35);

	memset(s, 0, 12);
	TXT4(280, 224, 8);
	strcpy(s, Sup_string);
	v3 = &s[0];

	if (mode == COMPUTER_HOPKINS) {
		s2 = "HOPKINS";
		int v4 = 8;
		char v5 = 1;
		do {
			if (!v4)
				break;
			v5 = *v3++ == *s2++;
			--v4;
		} while (v5);
		if (v5)
			passwordMatch = true;
	}
	if (mode == COMPUTER_SAMANTHAS) {
		char *v6 = &s[0];
		s2 = "328MHZA";
		int v7 = 8;
		bool v8 = true;
		do {
			if (!v7)
				break;
			v8 = (*v6++ == *s2++);
			--v7;
		} while (v8);
		if (v8)
			passwordMatch = true;
	}
	if (mode == COMPUTER_PUBLIC) {
		char *v9 = &s[0];
		s2 = "ALLFREE";
		int v10 = 8;
		bool v11 = true;
		do {
			if (!v10)
				break;
			v11 = (*v9++ == *s2++);
			--v10;
		} while (v11);
		if (v11)
			passwordMatch = true;
	}
	if (passwordMatch) {
		while (!_vm->shouldQuit()) {
			_vm->_eventsManager.ESC_KEY = false;
			_vm->_eventsManager.videkey();
			clearscreen();
			settextcolor(4);
			settextposition(2, 4);
			if (mode == COMPUTER_HOPKINS)
				outtext(Common::String(MenuText[0]._line));
			if (mode == COMPUTER_SAMANTHAS)
				outtext(Common::String(MenuText[1]._line));
			if (mode == COMPUTER_PUBLIC)
				outtext(Common::String(MenuText[2]._line));
			settextcolor(15);
			settextposition(8, 25);
			settextcolor(15);
			outtext2(Common::String(MenuText[6]._line));
			settextposition(20, 25);
			outtext2(Common::String(MenuText[7]._line));
			if (mode == COMPUTER_HOPKINS) {
				settextposition(10, 25);
				outtext2(Common::String(MenuText[8]._line));
				settextposition(12, 25);
				outtext2(Common::String(MenuText[9]._line));
				settextposition(14, 25);
				outtext2(Common::String(MenuText[10]._line));
				settextposition(16, 25);
				outtext2(Common::String(MenuText[11]._line));
			}
			if (mode == COMPUTER_SAMANTHAS) {
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

			bool numericFlag = false;
			char v12;
			do {
				v12 = _vm->_eventsManager.keywin();
				if (_vm->shouldQuit())
					return;

				if ((uint16)(v12 - 48) <= 8u)
					numericFlag = true;
			} while (!numericFlag);

			// 0 - Quit
			if (v12 == '0')
				break;
			// 1 - Games
			if (v12 == '1') {
				GAMES();
			} else if (mode == COMPUTER_HOPKINS) {
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
			} else if (mode == COMPUTER_SAMANTHAS) {
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
		// Access Denied
		settextcolor(4);
		settextposition(16, 25);
		outtext(Common::String(MenuText[5]._line));
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.delay(1000);

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

void ComputerManager::TXT4(int xp, int yp, int textIdx) {
	char curChar;
	char newChar;
	char mappedChar;
	Common::String charString;

	int x1 = xp;
	int x2 = 0;

	int textIndex = 0;
	bool oldMouseFlag = _vm->_eventsManager.souris_flag;
	_vm->_eventsManager.souris_flag = false;

	_vm->_fontManager.TEXT_NOW(xp, yp, "_", -4);
	do {
		curChar = _vm->_eventsManager.keywin();
		if (_vm->shouldQuit())
			return;

		mappedChar = '*';

		if (curChar == '-')
			mappedChar = 45;
		else if (curChar == '0')
			mappedChar = 48;
		else if (curChar == '1')
			mappedChar = 49;
		else if (curChar == '2')
			mappedChar = 50;
		else if (curChar == '3')
			mappedChar = 51;
		else if (curChar == '4')
			mappedChar = 52;
		else if (curChar == '5')
			mappedChar = 53;
		else if (curChar == '6')
			mappedChar = 54;
		else if (curChar == '7')
			mappedChar = 55;
		else if (curChar == '8')
			mappedChar = 56;
		else if (curChar == '9')
			mappedChar = 57;
		else if (curChar == 'A' || curChar == 'a')
			mappedChar = 65;
		else if (curChar == 'B' || curChar == 'b')
			mappedChar = 66;
		else if (curChar == 'C' || curChar == 'c')
			mappedChar = 67;
		else if (curChar == 'D' || curChar == 'd')
			mappedChar = 68;
		else if (curChar == 'E' || curChar == 'e')
			mappedChar = 69;
		else if (curChar == 'F' || curChar == 'f')
			mappedChar = 70;
		else if (curChar == 'G' || curChar == 'g')
			mappedChar = 71;
		else if (curChar == 'H' || curChar == 'h')
			mappedChar = 72;
		else if (curChar == 'I' || curChar == 'i')
			mappedChar = 73;
		else if (curChar == 'J' || curChar == 'j')
			mappedChar = 74;
		else if (curChar == 'K' || curChar == 'k')
			mappedChar = 75;
		else if (curChar == 'L' || curChar == 'l')
			mappedChar = 76;
		else if (curChar == 'M' || curChar == 'm')
			mappedChar = 77;
		else if (curChar == 'N' || curChar == 'n')
			mappedChar = 78;
		else if (curChar == 'O' || curChar == 'o')
			mappedChar = 79;
		else if (curChar == 'P' || curChar == 'p')
			mappedChar = 80;
		else if (curChar == 'Q' || curChar == 'q')
			mappedChar = 81;
		else if (curChar == 'R' || curChar == 'r')
			mappedChar = 82;
		else if (curChar == 'S' || curChar == 's')
			mappedChar = 83;
		else if (curChar == 'T' || curChar == 't')
			mappedChar = 84;
		else if (curChar == 'U' || curChar == 'u')
			mappedChar = 85;
		else if (curChar == 'V' || curChar == 'v')
			mappedChar = 86;
		else if (curChar == 'W' || curChar == 'w')
			mappedChar = 87;
		else if (curChar == 'X' || curChar == 'x')
			mappedChar = 88;
		else if (curChar == 'Y' || curChar == 'y')
			mappedChar = 89;
		else if (curChar == 'Z' || curChar == 'z')
			mappedChar = 90;

		// BackSpace
		if (curChar == 8 && textIndex > 0) {
			Sup_string[textIndex--] = 0;
			x1 -= _vm->_globals.police_l;
			x2 = x1 + 2 * _vm->_globals.police_l;
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, x1, yp, 3 * _vm->_globals.police_l, 12, _vm->_graphicsManager.VESA_BUFFER, x1, yp);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(x1, yp, x2, yp + 12);
			_vm->_fontManager.TEXT_NOW(x1, yp, "_", -4);
		}
		if (mappedChar != '*') {
			newChar = mappedChar;
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, x1, yp, _vm->_globals.police_l, 12, _vm->_graphicsManager.VESA_BUFFER, x1, yp);
			_vm->_graphicsManager.Ajoute_Segment_Vesa(x1, yp, _vm->_globals.police_l + x1, yp + 12);
			Sup_string[textIndex] = newChar;

			charString = Common::String::format("%c_", newChar);
			_vm->_fontManager.TEXT_NOW(x1, yp, charString, -4);
			++textIndex;
			x1 += _vm->_globals.police_l;
		}
		_vm->_eventsManager.VBL();
	} while (textIndex != textIdx && curChar != 13);

	_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager.VESA_SCREEN, x1, yp, _vm->_globals.police_l, 
		12, _vm->_graphicsManager.VESA_BUFFER, x1, yp);
	_vm->_graphicsManager.Ajoute_Segment_Vesa(x1, yp, _vm->_globals.police_l + x1, yp + 12);
	
	_vm->_eventsManager.VBL();
	Sup_string[textIndex] = 0;
	_vm->_eventsManager.souris_flag = oldMouseFlag;
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
	_vm->_eventsManager.keywin();
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
	_vm->_eventsManager.CASSE = true;
	_vm->_eventsManager.CASSE_SOURIS_ON();
	CASSETAB = (int16 *)g_PTRNUL;
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
	_vm->_fileManager.CONSTRUIT_SYSTEM("CASSE.SPR");
	CASSESPR = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	CHARGE_SCORE();
	MODE_VGA256();
	NEWTAB();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	PLAY_BRIQUE();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	CASSESPR = _vm->_globals.LIBERE_FICHIER(CASSESPR);
	_vm->_globals.dos_free2((byte *)CASSETAB);
	CASSETAB = (int16 *)g_PTRNUL;
	_vm->_objectsManager.Sprite[0].spriteData = v1;

	_vm->_soundManager.DEL_SAMPLE(1);
	_vm->_soundManager.DEL_SAMPLE(2);
	_vm->_soundManager.DEL_SAMPLE(3);
	_vm->_globals.vitesse = v2;
	_vm->_eventsManager.CASSE = false;
	_vm->_eventsManager.CASSE_SOURIS_OFF();
	setvideomode();
	settextcolor(15);
	clearscreen();
	_vm->_graphicsManager.max_x = 680;
	_vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_y = 460;
}

void ComputerManager::CHARGE_SCORE() {
	char nextChar; 
	byte *ptr; 

	_vm->_fileManager.CONSTRUIT_LINUX("HISCORE.DAT");
	ptr = _vm->_globals.dos_malloc2(100);
	_vm->_saveLoadManager.bload(_vm->_globals.NFICHIER, ptr);

	for (int scoreIndex = 0; scoreIndex < 6; ++scoreIndex) {
		for (int i = 0; i < 5; ++i) {
			nextChar = *(ptr + i + (16 * scoreIndex));
			if (!nextChar)
				nextChar = ' ';
			Score[scoreIndex].name += nextChar;
		} 

		for (int i = 0; i < 9; ++i) {
			nextChar = *(ptr + i + scoreIndex * 16 + 6);
			if (!nextChar)
				nextChar = '0';
			Score[scoreIndex].score += nextChar;
		} 
	} 

	_vm->_globals.dos_free2(ptr);
	CASSE_HISCORE = atol(Score[5].score.c_str());
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
	if (CASSETAB != (int16 *)g_PTRNUL) {
		_vm->_globals.dos_free2((byte *)CASSETAB);
		CASSETAB = (int16 *)g_PTRNUL;
	}

	++NB_TABLE;
	while (!_vm->shouldQuit()) {
		file = Common::String::format("TAB%d.TAB", NB_TABLE);

		_vm->_fileManager.CONSTRUIT_SYSTEM(file);
		if (f.open(_vm->_globals.NFICHIER))
			break;

		NB_TABLE = 1;
	}
	f.close();

	CASSETAB = (int16 *)_vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
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
	int xp; 
	int yp; 
	int v2; 
	uint16 v3; 
	int16 *v4; 

	NBBRIQUES = 0;
	CASSESPEED = 1;
	v4 = CASSETAB;
	v3 = 0;
	do {
		xp = v4[v3];
		yp = v4[v3 + 1];
		v2 = v4[v3 + 4];
		if (xp != -1) {
			if (v2 <= 6)
				++NBBRIQUES;
			
			if (v2 == 3)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 17);
			else if (v2 == 6)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 18);
			else if (v2 == 5)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 19);
			else if (v2 == 4)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 20);
			else if (v2 == 1)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 21);
			else if (v2 == 2)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 22);
			else if (v2 == 31)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, xp, yp, 23);
		}
		v3 += 6;
	} while (xp != -1);

	IMPRIMESCORE();
}

void ComputerManager::AFF_VIE() {
	int v3; 
	int v4; 

	int v0 = CASSEVIE - 1;
	int v1 = 10;

	for (int v2 = 0; v2 <= 11; v2++) {
		_vm->_graphicsManager.AFFICHE_SPEEDVGA(CASSESPR, v1, 10, 15);
		v1 += 7;
	}

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
			// Set up the racket and ball
			_vm->_eventsManager.MOUSE_OFF();
			BALLE = Common::Point(RAQX + 14, 187);
			_vm->_objectsManager.SETYSPR(1, 187);
			_vm->_objectsManager.SETXSPR(1, BALLE.x);
			_vm->_graphicsManager.RESET_SEGMENT_VESA();
			_vm->_eventsManager.VBL();
			_vm->_graphicsManager.FADE_IN_CASSE();

			// Wait for mouse press to start playing
			do {
				RAQX = _vm->_eventsManager.XMOUSE();
				if (_vm->_eventsManager.souris_x <= 4)
					RAQX = 5;
				if (RAQX > 282)
					RAQX = 282;
				_vm->_objectsManager.SETXSPR(0, RAQX);
				_vm->_objectsManager.SETXSPR(1, RAQX + 14);
				_vm->_objectsManager.SETYSPR(1, 187);
				_vm->_eventsManager.VBL();
			} while (!_vm->shouldQuit() && _vm->_eventsManager.BMOUSE() != 1);

			CASSESPEED = 1;
			BALLE = Common::Point(RAQX + 14, 187);
			BALLEHORI = RAQX > 135;
			BALLEVERTI = 0;

			// Play loop
			do {
				RAQX = _vm->_eventsManager.XMOUSE();
				if (_vm->_eventsManager.souris_x <= 4)
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
	int v3; 
	int v4; 
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

	for (int v6 = 0; v6 <= 5; v6++) {
		v0 = 19 * v6;
		v0 += 46;
		for (int v1 = 0; v1 <= 5; v1++)
			PRINT_HSCORE(ptr, 9 * v1 + 69, v0, Score[v6].name[v1]);

		for (int v2 = 0; v2 <= 8; v2++)
			PRINT_HSCORE(ptr, 9 * v2 + 199, v0, Score[v6].score[v2]);

	}

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
	char curChar; 
	byte *ptr; 

	_vm->_graphicsManager.LOAD_IMAGEVGA("NAME.PCX");
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_fileManager.CONSTRUIT_SYSTEM("ALPHA.SPR");
	ptr = _vm->_fileManager.CHARGE_FICHIER(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.FADE_IN_CASSE();
	for (int strPos = 0; strPos <= 4; strPos++) {
		PRINT_HSCORE(ptr, 9 * strPos + 140, 78, 1);

		curChar = toupper(_vm->_eventsManager.keywin());
		if ((curChar <= '/') || (curChar > 'Z'))
			curChar = ' ';
		if ((uint16)(curChar - ':') <= 6u)
			curChar = ' ';

		Score[5].name.setChar(curChar, strPos);
		PRINT_HSCORE(ptr, 9 * strPos + 140, 78, curChar);

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
	}
	Score[5].score = "         ";

	char score[16]; 
	sprintf(score, "%d", CASSESCORE);
	int scoreLen = 0;
	do
		++scoreLen;
	while (score[scoreLen]);
	int scorePos = 8;
	for (int i = scoreLen; ; Score[5].score.setChar(score[i], scorePos--)) {
		--i;
		if (i <= -1)
			break;
	}
	_vm->_graphicsManager.FADE_OUT_CASSE();
	_vm->_globals.dos_free2(ptr);
	SAUVE_SCORE();
}

void ComputerManager::IMPRIMESCORE() {
	int16 v0; 
	int16 v1; 
	int16 i; 
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
	int16 v2; 
	int16 v3; 

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
	int v1; 
	int v2; 
	int v4; 
	int v6; 
	char v8; 
	int v9; 
	char v11; 
	int v14; 
	byte *ptr; 
	int v16[6]; 
	int v17[6]; 

	for (int v0 = 0; v0 <= 5; v0++) {
		v1 = atol(Score[v0].score.c_str());
		v17[v0] = v1;
		if (!v1)
			v17[v0] = 5;
	}

	for (int v13 = 0; v13 <= 5; v13++) {
		v2 = 0;
		bool v3 = false;
		do {
			v4 = v17[v2];
			if (v4 && v17[0] <= v4 && v17[1] <= v4 && v17[2] <= v4 && v17[3] <= v4 
					&& v17[4] <= v4 && v17[5] <= v4)
				v3 = true;
			if (v3) {
				v16[v13] = v2;
				v17[v2] = 0;
			} else {
				++v2;
			}
		} while (!v3);
	}

	ptr = _vm->_globals.dos_malloc2(0x64u);
	memset(ptr, 0, 0x63u);
	for (int v5 = 0; v5 <= 5; v5++) {
		v6 = 16 * v5;
		v14 = v16[v5];
		for (int v7 = 0; v7 <= 4; v7++) {
			v8 = Score[v14].name[v7];
			if (!v8)
				v8 = 32;
			*(ptr + (16 * v5) + v7) = v8;
		};

		*(ptr + v6 + 5) = 0;
		v9 = v6 + 6;

		for (int v10 = 0; v10 <= 8; v10++) {
			v11 = Score[v14].score[v10];
			if (!v11)
				v11 = 48;
			*(ptr + v9 + v10) = v11;
		};
		*(ptr + v9 + 9) = 0;
	}

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
	int16 v1; 
	int16 v4 = 0;
	//(signed int)(6.0 * (long double)_vm->getRandomNumber( rand() / 2147483648.0) + 1;
	// TODO: Figure out random number
	int v0 = _vm->getRandomNumber(6); 
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
			int v2 = BALLE.x + 6;
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
	int v1; 
	int v2; 
	int v3; 
	int v4; 
	int v10; 
	int v11; 

	int v7 = 0;
	//v6 = (signed int)(6.0 * (long double)rand() / 2147483648.0) + 1;
	// TODO: Check if correct
	int v6 = _vm->getRandomNumber(6) + 1;
	int v0 = BALLE.x;
	int v13 = BALLE.y;
	int v5 = BALLE.x + 6;
	int v12 = BALLE.y + 6;
	int16 *v9 = CASSETAB;
	uint16 v8 = 0;
	do {
		v1 = v9[v8];
		v11 = v9[v8 + 1];
		v2 = v9[v8 + 2];
		v10 = v9[v8 + 3];
		v3 = v9[v8 + 4];
		if (v9[v8 + 5] != 1 || v1 == -1)
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
