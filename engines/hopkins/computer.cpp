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
		_menuText[i]._actvFl = false;
		_menuText[i]._lineSize = 0;
		memset(_menuText[i]._line, 0, 90);
	}
	Common::fill(&_inputBuf[0], &_inputBuf[200], '\0');
	_breakoutSpr = NULL;
	_textColor = 0;
	_breakoutLevel = NULL;
	_breakoutBrickNbr = 0;
	_breakoutScore = 0;
	_breakoutLives = 0;
	_breakoutSpeed = 0;
	_ballRightFl = false;
	_ballUpFl = false;
	_breakoutLevelNbr = 0;
	RAQX = 0;
	CASSEP1 = 0;
	CASSEP2 = 0;
	CASSDER = 0;
}

void ComputerManager::setParent(HopkinsEngine *vm) {
	_vm = vm;
}

/**
 * Sets up textual entry mode. Used by the code for Hopkins computer.
 */
void ComputerManager::setVideoMode() {
	setTextMode();
}

/**
 * Sets up Textual entry mode
 */
void ComputerManager::setTextMode() {
	_vm->_graphicsManager.Cls_Pal();
	_vm->_graphicsManager.DD_Lock();
	_vm->_graphicsManager.Cls_Video();
	_vm->_graphicsManager.DD_Unlock();
	//SET_MODE(SCREEN_WIDTH, SCREEN_HEIGHT);

	_vm->_graphicsManager.nbrligne = SCREEN_WIDTH;
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "STFONT.SPR");
	_vm->_globals.police = _vm->_globals.dos_free2(_vm->_globals.police);
	_vm->_globals.police = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	_vm->_globals.police_l = 8;
	_vm->_globals.police_h = 8;
	_vm->_graphicsManager.LOAD_IMAGE("WINTEXT");
	_vm->_graphicsManager.FADE_INW();
	loadMenu();
	_vm->_eventsManager.souris_flag = false;
}

/**
 * Clear the screen
 */
void ComputerManager::clearScreen() {
	_vm->_graphicsManager.LOAD_IMAGE("WINTEXT");
	_vm->_graphicsManager.FADE_INW();
}

/**
 * Sets the text mode color
 */
void ComputerManager::setTextColor(int col) {
	_textColor = col;
}

/**
 * Sets the text position.
 * @param yp		Y position
 * @param xp		X position
 * @remarks		Yes, the reverse co-ordinate pair is really like that in the original game.
 */
void ComputerManager::setTextPosition(int yp, int xp) {
	_textPosition.x = xp << 3;
	_textPosition.y = yp << 4;
}

/**
 * Show a computer in the FBI office
 * @param mode		Which computer to display
 */
void ComputerManager::showComputer(ComputerEnum mode) {
	bool passwordMatch; 
	char *v3; 
	char s[12]; 
	const char *s2;

	_vm->_eventsManager.ESC_KEY = 0;
	passwordMatch = false;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	setVideoMode();
	setTextColor(4);

	_vm->_eventsManager.videkey();
	setTextPosition(2, 4);
	if (mode == COMPUTER_HOPKINS)
		outText(Common::String(_menuText[0]._line));
	else if (mode == COMPUTER_SAMANTHAS)
		outText(Common::String(_menuText[1]._line));
	else if (mode == COMPUTER_PUBLIC)
		outText(Common::String(_menuText[2]._line));

	setTextColor(1);
	if (mode == COMPUTER_PUBLIC) {
		setTextPosition(10, 8);
		outText(Common::String(_menuText[3]._line));
	}
	setTextPosition(12, 28);
	outText(Common::String(_menuText[4]._line));
	setTextPosition(14, 35);

	memset(s, 0, 12);
	TXT4(280, 224, 8);
	strcpy(s, _inputBuf);
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
	} else if (mode == COMPUTER_SAMANTHAS) {
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
	} else if (mode == COMPUTER_PUBLIC) {
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
			clearScreen();
			setTextColor(4);
			setTextPosition(2, 4);
			if (mode == COMPUTER_HOPKINS)
				outText(Common::String(_menuText[0]._line));
			else if (mode == COMPUTER_SAMANTHAS)
				outText(Common::String(_menuText[1]._line));
			else if (mode == COMPUTER_PUBLIC)
				outText(Common::String(_menuText[2]._line));
			setTextColor(15);
			setTextPosition(8, 25);
			setTextColor(15);
			outText2(Common::String(_menuText[6]._line));
			setTextPosition(20, 25);
			outText2(Common::String(_menuText[7]._line));
			if (mode == COMPUTER_HOPKINS) {
				setTextPosition(10, 25);
				outText2(Common::String(_menuText[8]._line));
				setTextPosition(12, 25);
				outText2(Common::String(_menuText[9]._line));
				setTextPosition(14, 25);
				outText2(Common::String(_menuText[10]._line));
				setTextPosition(16, 25);
				outText2(Common::String(_menuText[11]._line));
			} else if (mode == COMPUTER_SAMANTHAS) {
				_vm->_eventsManager.videkey();
				setTextPosition(10, 25);
//				outText2(Common::String(_menuText[0x95A])); <=== CHECKME: Unexpected value! replaced by the following line, for consistancy
				outText2(Common::String(_menuText[12]._line));
				setTextPosition(12, 25);
				outText2(Common::String(_menuText[13]._line));
				setTextPosition(14, 25);
				outText2(Common::String(_menuText[14]._line));
				setTextPosition(16, 25);
				outText2(Common::String(_menuText[15]._line));
				setTextPosition(18, 25);
				outText2(Common::String(_menuText[16]._line));
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
				displayGamesSubMenu();
			} else if (mode == COMPUTER_HOPKINS) {
				_vm->_eventsManager.videkey();
				clearScreen();
				setTextColor(4);
				setTextPosition(2, 4);
				outText(Common::String(_menuText[0]._line));
				setTextColor(15);
				if (v12 == 50)
					readText(1);
				if (v12 == 51)
					readText(2);
				if (v12 == 52)
					readText(3);
				if (v12 == 53)
					readText(4);
			} else if (mode == COMPUTER_SAMANTHAS) {
				clearScreen();
				setTextColor(4);
				setTextPosition(2, 4);
				outText(Common::String(_menuText[1]._line));
				setTextColor(15);
				if (v12 == 50)
					readText(6);
				if (v12 == 51)
					readText(7);
				if (v12 == 52)
					readText(8);
				if (v12 == 53)
					readText(9);
				if (v12 == 54) {
					readText(10);
					_vm->_globals.SAUVEGARDE->data[svField270] = 4;
				}
			}
		}
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		restoreFBIRoom();
	} else {
		// Access Denied
		setTextColor(4);
		setTextPosition(16, 25);
		outText(Common::String(_menuText[5]._line));
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.delay(1000);

		memset(_vm->_graphicsManager.VESA_BUFFER, 0, 0x4AFFFu);
		_vm->_graphicsManager.DD_Lock();
		_vm->_graphicsManager.Cls_Video();
		_vm->_graphicsManager.DD_Unlock();
		_vm->_graphicsManager.DD_VBL();
		restoreFBIRoom();
		_vm->_eventsManager.MOUSE_OFF();
	}
	if (mode == 1)
		_vm->_globals.SORTIE = 13;
	if ((uint16)(mode - 2) <= 1u)
		_vm->_globals.SORTIE = 14;
	
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

/**
 * Load Menu data
 */
void ComputerManager::loadMenu() {
	_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "COMPUTAN.TXT");
	byte *ptr = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
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
			_menuText[lineNum]._actvFl = 1;
			strPos = 0;
			for (;;) {
				byte curChar = tmpPtr[strPos + 2];
				if (curChar == '%' || curChar == 10)
					break;
				_menuText[lineNum]._line[strPos++] = curChar;
				if (strPos > 89)
					goto LABEL_11;
			}
			_menuText[lineNum]._line[strPos] = 0;
			_menuText[lineNum]._lineSize = strPos - 1;
	LABEL_11:
			++lineNum;
		}
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
			_inputBuf[textIndex--] = 0;
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
			_inputBuf[textIndex] = newChar;

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
	_inputBuf[textIndex] = 0;
	_vm->_eventsManager.souris_flag = oldMouseFlag;
}

/**
 * Outputs a text string
 */
void ComputerManager::outText(const Common::String &msg) {
	_vm->_fontManager.TEXT_COMPUT(_textPosition.x, _textPosition.y, msg, _textColor);
}

/**
 * Outputs a text string
 */
void ComputerManager::outText2(const Common::String &msg) {
	_vm->_fontManager.TEXT_NOW(_textPosition.x, _textPosition.y, msg, _textColor);
}

/**
 * Restores the scene for the FBI headquarters room
 */
void ComputerManager::restoreFBIRoom() {
	_vm->_globals.police = _vm->_globals.dos_free2(_vm->_globals.police);
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "FONTE3.SPR");
	_vm->_globals.police = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);

	_vm->_globals.police_l = 12;
	_vm->_globals.police_h = 21;
	_vm->_eventsManager.souris_flag = true;
}

/**
 * Display texts for the given menu entry
 */
void ComputerManager::readText(int idx) {
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
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPKAN.TXT");
	else if (_vm->_globals.FR == 1)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPK.TXT");
	else if (_vm->_globals.FR == 2)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPKES.TXT");

	ptr = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	v1 = _vm->_fileManager.fileSize(_vm->_globals.NFICHIER);
	v2 = 0;
	v3 = 0;
	if (v1 > 0u) {
		do {
			if (*(ptr + v3) == '%') {
				numStr = Common::String::format("%c%c", *(ptr + v3 + 1), *(ptr + v3 + 2));
				num	= atol(numStr.c_str());

				if (num == idx)
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
			setTextPosition(v6, v5);
			outText(v12);

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

/**
 * Display breakout when Games sub-menu is selected
 */
void ComputerManager::displayGamesSubMenu() {
	const byte *v1 = _vm->_objectsManager.Sprite[0].spriteData;
	uint oldSpeed = _vm->_globals.vitesse;

	_vm->_globals.vitesse = 1;
	_vm->_eventsManager.CHANGE_MOUSE(0);
	_breakoutSpr = g_PTRNUL;
	_vm->_eventsManager.CASSE = true;
	_vm->_eventsManager.CASSE_SOURIS_ON();
	_breakoutLevel = (int16 *)g_PTRNUL;
	_breakoutBrickNbr = 0;
	_breakoutScore = 0;
	_breakoutLives = 5;
	_breakoutSpeed = 1;
	_ballRightFl = false;
	_ballUpFl = false;
	_breakoutLevelNbr = 0;
	_vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_x = 320;
	_vm->_graphicsManager.max_y = 200;
	_vm->_soundManager.CHARGE_SAMPLE(1, "SOUND37.WAV");
	_vm->_soundManager.CHARGE_SAMPLE(2, "SOUND38.WAV");
	_vm->_soundManager.CHARGE_SAMPLE(3, "SOUND39.WAV");
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "CASSE.SPR");
	_breakoutSpr = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	loadHiscore();
	setModeVGA256();
	newLevel();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	playBreakout();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	_breakoutSpr = _vm->_globals.LIBERE_FICHIER(_breakoutSpr);
	_vm->_globals.dos_free2((byte *)_breakoutLevel);
	_breakoutLevel = (int16 *)g_PTRNUL;
	_vm->_objectsManager.Sprite[0].spriteData = v1;

	_vm->_soundManager.DEL_SAMPLE(1);
	_vm->_soundManager.DEL_SAMPLE(2);
	_vm->_soundManager.DEL_SAMPLE(3);
	_vm->_globals.vitesse = oldSpeed;
	_vm->_eventsManager.CASSE = false;
	_vm->_eventsManager.CASSE_SOURIS_OFF();
	setVideoMode();
	setTextColor(15);
	clearScreen();
	_vm->_graphicsManager.max_x = 680;
	_vm->_graphicsManager.min_y = 0;
	_vm->_graphicsManager.max_y = 460;
}

/**
 * Load Highscore from file
 */
void ComputerManager::loadHiscore() {
	char nextChar; 
	byte *ptr; 

	_vm->_fileManager.constructLinuxFilename("HISCORE.DAT");
	ptr = _vm->_globals.dos_malloc2(100);
	_vm->_saveLoadManager.bload(_vm->_globals.NFICHIER, ptr);

	for (int scoreIndex = 0; scoreIndex < 6; ++scoreIndex) {
		for (int i = 0; i < 5; ++i) {
			nextChar = *(ptr + i + (16 * scoreIndex));
			if (!nextChar)
				nextChar = ' ';
			_score[scoreIndex]._name += nextChar;
		} 

		for (int i = 0; i < 9; ++i) {
			nextChar = *(ptr + i + scoreIndex * 16 + 6);
			if (!nextChar)
				nextChar = '0';
			_score[scoreIndex]._score += nextChar;
		} 
	} 

	_vm->_globals.dos_free2(ptr);
	_breakoutHiscore = atol(_score[5]._score.c_str());
}

/** 
 * VGA 256 col
 */
void ComputerManager::setModeVGA256() {
	_vm->_graphicsManager.DD_Lock();
	_vm->_graphicsManager.Cls_Video();
	_vm->_graphicsManager.DD_Unlock();
	_vm->_graphicsManager.Cls_Pal();
	_vm->_graphicsManager.SCANLINE(0x140u);
}

/**
 * Load new level
 */
void ComputerManager::newLevel() {
	Common::String file;
	Common::File f;

	_vm->_objectsManager.SPRITE_OFF(0);
	_vm->_objectsManager.SPRITE_OFF(1);
	++_breakoutLives;
	if (_breakoutLives > 11)
		_breakoutLives = 11;
	_vm->_graphicsManager.LOAD_IMAGEVGA("CASSEF.PCX");
	displayLives();
	if (_breakoutLevel != (int16 *)g_PTRNUL) {
		_vm->_globals.dos_free2((byte *)_breakoutLevel);
		_breakoutLevel = (int16 *)g_PTRNUL;
	}

	++_breakoutLevelNbr;
	while (!_vm->shouldQuit()) {
		file = Common::String::format("TAB%d.TAB", _breakoutLevelNbr);

		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, file);
		if (f.open(_vm->_globals.NFICHIER))
			break;

		_breakoutLevelNbr = 1;
	}
	f.close();

	_breakoutLevel = (int16 *)_vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	displayBricks();
	_vm->_objectsManager.SPRITE(_breakoutSpr, 150, 192, 0, 13, 0, 0, 0, 0);
	_vm->_objectsManager.SPRITE(_breakoutSpr, 164, 187, 1, 14, 0, 0, 0, 0);
	_ballPosition = Common::Point(164, 187);
	RAQX = 150;
	_vm->_objectsManager.SPRITE_ON(0);
	_vm->_objectsManager.SPRITE_ON(1);
	_vm->_eventsManager.MOUSE_ON1();
	_vm->_soundManager.PLAY_SAMPLE(3, 5);
}

/**
 * Display bricks in breakout game
 */
void ComputerManager::displayBricks() {
	int xp; 
	int yp; 
	int v2; 
	uint16 v3; 
	int16 *v4; 

	_breakoutBrickNbr = 0;
	_breakoutSpeed = 1;
	v4 = _breakoutLevel;
	v3 = 0;
	do {
		xp = v4[v3];
		yp = v4[v3 + 1];
		v2 = v4[v3 + 4];
		if (xp != -1) {
			if (v2 <= 6)
				++_breakoutBrickNbr;
			
			if (v2 == 3)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 17);
			else if (v2 == 6)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 18);
			else if (v2 == 5)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 19);
			else if (v2 == 4)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 20);
			else if (v2 == 1)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 21);
			else if (v2 == 2)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 22);
			else if (v2 == 31)
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, yp, 23);
		}
		v3 += 6;
	} while (xp != -1);

	displayScore();
}

/**
 * Display Lives in breakout game
 */
void ComputerManager::displayLives() {
	int v3; 
	int v4; 

	int v0 = _breakoutLives - 1;
	int v1 = 10;

	for (int i = 0; i <= 11; i++) {
		_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, v1, 10, 15);
		v1 += 7;
	}

	if (v0) {
		v3 = 10;
		v4 = 0;
		if (v0 > 0) {
			do {
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, v3, 10, 14);
				v3 += 7;
				++v4;
			} while (v4 < v0);
		}
	}
}

/**
 * Main function for breakout game
 */
void ComputerManager::playBreakout() {
	int v1 = 0; 
	int v; 

	while (!_vm->shouldQuit()) {
		while (!_vm->shouldQuit()) {
			// Set up the racket and ball
			_vm->_eventsManager.MOUSE_OFF();
			_ballPosition = Common::Point(RAQX + 14, 187);
			_vm->_objectsManager.SETYSPR(1, 187);
			_vm->_objectsManager.SETXSPR(1, _ballPosition.x);
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

			_breakoutSpeed = 1;
			_ballPosition = Common::Point(RAQX + 14, 187);
			_ballRightFl = (RAQX > 135);
			_ballUpFl = false;

			// Play loop
			do {
				_vm->_soundManager.checkSounds();

				RAQX = _vm->_eventsManager.XMOUSE();
				if (_vm->_eventsManager.souris_x <= 4)
					RAQX = 5;
				if (RAQX > 282)
					RAQX = 282;
				_vm->_objectsManager.SETXSPR(0, RAQX);
				v1 = moveBall();
				_vm->_eventsManager.VBL();
			} while (!_vm->shouldQuit() && !v1);
			if (v1 != 1)
				break;
			_vm->_graphicsManager.FADE_OUT_CASSE();
			--_breakoutLives;

			if (_breakoutLives) {
				displayLives();
				if (_breakoutLives)
					continue;
			}
			_vm->_eventsManager.MOUSE_ON1();
			_vm->_objectsManager.SPRITE_OFF(0);
			_vm->_objectsManager.SPRITE_OFF(1);
			if (_breakoutScore > _breakoutHiscore)
				getScoreName();
			v = displayHiscores();
			if (v != 1)
				break;

			_breakoutBrickNbr = 0;
			_breakoutScore = 0;
			_breakoutLives = 4;
			_breakoutSpeed = 1;
			_ballRightFl = false;
			_ballUpFl = false;
			_breakoutLevelNbr = 0;
			loadHiscore();
			newLevel();
		}
		if (v1 != 2)
			return;
		_vm->_graphicsManager.FADE_OUT_CASSE();
		newLevel();
	}
}

/**
 * Show the high scores for the Breakout game
 * @return		The selected button index: 1 = Game, 2 = Quit
 */
int ComputerManager::displayHiscores() {
	int yp; 
	int buttonIndex; 
	int xp; 
	byte *ptr; 

	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	loadHiscore();
	_vm->_graphicsManager.LOAD_IMAGEVGA("HISCORE.PCX");
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "ALPHA.SPR");
	ptr = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	// Loop for displaying the scores
	for (int scoreIndex = 0; scoreIndex <= 5; scoreIndex++) {
		yp = 19 * scoreIndex;
		yp += 46;

		// Display the characters of the name
		for (int i = 0; i <= 5; i++)
			displayHiscoreLine(ptr, 9 * i + 69, yp, _score[scoreIndex]._name[i]);

		// Display the digits of the score
		for (int i = 0; i <= 8; i++)
			displayHiscoreLine(ptr, 9 * i + 199, yp, _score[scoreIndex]._score[i]);
	}

	_vm->_graphicsManager.FADE_IN_CASSE();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	buttonIndex = 0;
	do {
		_vm->_eventsManager.CONTROLE_MES();
		xp = _vm->_eventsManager.XMOUSE();
		yp = _vm->_eventsManager.YMOUSE();

		if (_vm->_eventsManager.BMOUSE() == 1 && ABS(xp - 79) <= 33 && ABS(yp - 396) <= 13)
			buttonIndex = 1;
		else if (_vm->_eventsManager.BMOUSE() == 1 && ABS(xp - 583) <= 32 && ABS(yp - 396) <= 13)
			buttonIndex = 2;

		_vm->_eventsManager.VBL();
	} while (!buttonIndex && !_vm->shouldQuit());

	_vm->_eventsManager.MOUSE_OFF();
	_vm->_graphicsManager.FADE_OUT_CASSE();
	_vm->_globals.LIBERE_FICHIER(ptr);
	return buttonIndex;
}

/**
 * Display a screen to enter player name in the case of a new hiscore
 */
void ComputerManager::getScoreName() {
	char curChar; 
	byte *ptr; 

	_vm->_graphicsManager.LOAD_IMAGEVGA("NAME.PCX");
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "ALPHA.SPR");
	ptr = _vm->_fileManager.loadFile(_vm->_globals.NFICHIER);
	_vm->_graphicsManager.FADE_IN_CASSE();
	for (int strPos = 0; strPos <= 4; strPos++) {
		displayHiscoreLine(ptr, 9 * strPos + 140, 78, 1);

		curChar = toupper(_vm->_eventsManager.keywin());
		if ((curChar <= '/') || (curChar > 'Z'))
			curChar = ' ';
		if ((uint16)(curChar - ':') <= 6u)
			curChar = ' ';

		_score[5]._name.setChar(curChar, strPos);
		displayHiscoreLine(ptr, 9 * strPos + 140, 78, curChar);

		for (int idx = 0; idx < 12; ++idx)
			_vm->_eventsManager.VBL();
	}
	_score[5]._score = "         ";

	char score[16]; 
	sprintf(score, "%d", _breakoutScore);
	int scoreLen = 0;
	do
		++scoreLen;
	while (score[scoreLen]);
	int scorePos = 8;
	for (int i = scoreLen; ; _score[5]._score.setChar(score[i], scorePos--)) {
		--i;
		if (i <= -1)
			break;
	}
	_vm->_graphicsManager.FADE_OUT_CASSE();
	_vm->_globals.dos_free2(ptr);
	saveScore();
}

/**
 * Display current score
 */
void ComputerManager::displayScore() {
	int16 v0; 
	int16 v1; 
	int16 i; 
	char s[40]; 

	sprintf(s, "%d", _breakoutScore);
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
	int16 v2 = 203;
	int16 v3 = 3;

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
	_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, v2 - 3, 11, v3);
}

/**
 * Save Hiscore in file
 */
void ComputerManager::saveScore() {
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
		v1 = atol(_score[v0]._score.c_str());
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
			v8 = _score[v14]._name[v7];
			if (!v8)
				v8 = 32;
			*(ptr + (16 * v5) + v7) = v8;
		};

		*(ptr + v6 + 5) = 0;
		v9 = v6 + 6;

		for (int v10 = 0; v10 <= 8; v10++) {
			v11 = _score[v14]._score[v10];
			if (!v11)
				v11 = 48;
			*(ptr + v9 + v10) = v11;
		};
		*(ptr + v9 + 9) = 0;
	}

	_vm->_fileManager.constructLinuxFilename("HISCORE.DAT");
	_vm->_saveLoadManager.SAUVE_FICHIER(_vm->_globals.NFICHIER, ptr, 0x64u);
	_vm->_globals.dos_free2(ptr);
}

/**
 * Display parts of the hiscore line
 */
void ComputerManager::displayHiscoreLine(byte *objectData, int x, int y, int a4) {
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
	_vm->_graphicsManager.AFFICHE_SPEEDVGA(objectData, x, y, v5);
}

/**
 * Handle ball moves
 */
int ComputerManager::moveBall() {
	int16 v1; 
	int16 v4 = 0;
	//(signed int)(6.0 * (long double)_vm->getRandomNumber( rand() / 2147483648.0) + 1;
	// TODO: Figure out random number
	int v0 = _vm->getRandomNumber(6); 
	if (_breakoutSpeed == 1) {
		CASSEP1 = 1;
		CASSEP2 = 1;
	}
	if (_breakoutSpeed == 2) {
		CASSEP1 = 1;
		CASSEP2 = 2;
	}
	if (_breakoutSpeed == 3) {
		CASSEP1 = 2;
		CASSEP2 = 2;
	}
	if (_breakoutSpeed == 4) {
		CASSEP1 = 3;
		CASSEP2 = 2;
	}
	v1 = CASSEP1;
	if (CASSDER == CASSEP1)
		v1 = CASSEP2;

	if (_ballUpFl)
		_ballPosition.y += v1;
	else
		_ballPosition.y -= v1;

	if (_ballRightFl)
		_ballPosition.x += v1;
	else 
		_ballPosition.x -= v1;

	CASSDER = v1;
	if (_ballPosition.x <= 6) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		_ballPosition.x = v0 + 6;
		_ballRightFl = !_ballRightFl;
	}
	if (_ballPosition.x > 307) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		_ballPosition.x = 307 - v0;
		_ballRightFl = !_ballRightFl;
	}
	if (_ballPosition.y <= 6) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		_ballPosition.y = v0 + 7;
		_ballUpFl = !_ballUpFl;
	}
	if ((uint16)(_ballPosition.y - 186) <= 8u) {
		_vm->_soundManager.PLAY_SAMPLE(2, 6);
		if (_ballPosition.x > RAQX - 2) {
			int v2 = _ballPosition.x + 6;
			if (v2 < RAQX + 36) {
				_ballUpFl = false;
				if (v2 <= RAQX + 15) {
					_ballRightFl = false;
					if (_ballPosition.x >= RAQX && v2 <= RAQX + 5)
						_ballPosition.x -= 4;
					if (_ballPosition.x >= RAQX + 5 && _ballPosition.x + 6 <= RAQX + 10)
						_ballPosition.x -= 2;
				}
				if (_ballPosition.x >= RAQX + 19 && _ballPosition.x + 6 <= RAQX + 36) {
					_ballRightFl = true;
					if (_ballPosition.x >= RAQX + 29)
						_ballPosition.x += 4;
					if (_ballPosition.x >= RAQX + 24 && _ballPosition.x + 6 <= RAQX + 29)
						_ballPosition.x += 2;
				}
			}
		}
	}
	if (_ballPosition.y > 194)
		v4 = 1;
	checkBallCollisions();
	_vm->_objectsManager.SETXSPR(1, _ballPosition.x);
	_vm->_objectsManager.SETYSPR(1, _ballPosition.y);
	if (!_breakoutBrickNbr)
		v4 = 2;
	return v4;
}

/**
 * Check ball collision with bricks
 */
void ComputerManager::checkBallCollisions() {
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
	int v0 = _ballPosition.x;
	int v13 = _ballPosition.y;
	int v5 = _ballPosition.x + 6;
	int v12 = _ballPosition.y + 6;
	int16 *v9 = _breakoutLevel;
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
				_ballUpFl = true;
			}
			if (v5 >= v1) {
				if (v0 <= v1) {
					++v4;
					_ballUpFl = true;
					_ballRightFl = false;
					if (v3 == 31)
						_ballPosition.x -= v6;
				}
			}
			if (v0 <= v2) {
				if (v5 >= v2) {
					++v4;
					_ballUpFl = true;
					_ballRightFl = true;
					if (v3 == 31)
						_ballPosition.x += v6;
				}
			}
		}
		if (v12 >= v11) {
			if (v13 > v11)
				goto LABEL_31;
			if (v0 >= v1 && v5 <= v2) {
				++v4;
				_ballUpFl = false;
			}
			if (v5 >= v1) {
				if (v0 <= v1) {
					++v4;
					_ballUpFl = false;
					_ballRightFl = false;
					if (v3 == 31)
						_ballPosition.x -= 2;
				}
			}
			if (v0 <= v2) {
				if (v5 >= v2) {
					++v4;
					_ballUpFl = false;
					_ballRightFl = true;
					if (v3 == 31)
						_ballPosition.x += v6;
				}
			}
		}
		if (v13 >= v11) {
LABEL_31:
			if (v12 <= v10) {
				if (v5 >= v1) {
					if (v0 <= v1) {
						++v4;
						_ballRightFl = false;
						if (v3 == 31)
							_ballPosition.x -= v6;
					}
				}
				if (v0 <= v2) {
					if (v5 >= v2) {
						++v4;
						_ballRightFl = true;
						if (v3 == 31)
							_ballPosition.x += v6;
					}
				}
			}
		}
		if (v4) {
			if (v3 == 31) {
				_vm->_soundManager.PLAY_SAMPLE(2, 6);
			} else {
				_vm->_soundManager.PLAY_SAMPLE(1, 5);
				_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, v1, v11, 16);
				if (v3 == 1)
					_breakoutScore += 10;
				if (v3 == 2)
					_breakoutScore += 5;
				if (v3 == 3) {
					_breakoutScore += 50;
					if (_breakoutSpeed <= 1)
						_breakoutSpeed = 2;
					if (_breakoutBrickNbr <= 19)
						_breakoutSpeed = 3;
				}
				if (v3 == 4)
					_breakoutScore += 20;
				if (v3 == 5) {
					_breakoutScore += 30;
					if (_breakoutSpeed <= 1)
						_breakoutSpeed = 2;
				}
				if (v3 == 6)
					_breakoutScore += 40;
				displayScore();
				--_breakoutBrickNbr;
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
