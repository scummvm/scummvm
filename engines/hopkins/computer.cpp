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
	_padPositionX = 0;
	_minBreakoutMoveSpeed = 0;
	_maxBreakoutMoveSpeed = 0;
	_lastBreakoutMoveSpeed = 0;
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
	_vm->_graphicsManager.clearPalette();
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.clearScreen();
	_vm->_graphicsManager.unlockScreen();
	//SET_MODE(SCREEN_WIDTH, SCREEN_HEIGHT);

	_vm->_graphicsManager._lineNbr = SCREEN_WIDTH;
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "STFONT.SPR");
	_vm->_globals.police = _vm->_globals.freeMemory(_vm->_globals.police);
	_vm->_globals.police = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	_vm->_globals.police_l = 8;
	_vm->_globals.police_h = 8;
	_vm->_graphicsManager.loadImage("WINTEXT");
	_vm->_graphicsManager.fadeInLong();
	loadMenu();
	_vm->_eventsManager._mouseFl = false;
}

/**
 * Clear the screen
 */
void ComputerManager::clearScreen() {
	_vm->_graphicsManager.loadImage("WINTEXT");
	_vm->_graphicsManager.fadeInLong();
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
	_vm->_eventsManager._escKeyFl = false;
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	setVideoMode();
	setTextColor(4);
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

	TXT4(280, 224, 8);
	bool passwordMatch = false;
	if (mode == COMPUTER_HOPKINS) {
		if (!strcmp(_inputBuf, "HOPKINS"))
			passwordMatch = true;
	} else if (mode == COMPUTER_SAMANTHAS) {
		if (!strcmp(_inputBuf, "328MHZA"))
			passwordMatch = true;
	} else if (mode == COMPUTER_PUBLIC) {
		if (!strcmp(_inputBuf, "ALLFREE"))
			passwordMatch = true;
	}

	if (passwordMatch) {
		while (!_vm->shouldQuit()) {
			_vm->_eventsManager._escKeyFl = false;
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
			char keyPressed;
			do {
				keyPressed = _vm->_eventsManager.waitKeyPress();
				if (_vm->shouldQuit())
					return;

				if ((keyPressed >= '0') && (keyPressed <= '9'))
					numericFlag = true;
			} while (!numericFlag);

			// 0 - Quit
			if (keyPressed == '0')
				break;
			// 1 - Games
			if (keyPressed == '1') {
				displayGamesSubMenu();
			} else if (mode == COMPUTER_HOPKINS) {
				clearScreen();
				setTextColor(4);
				setTextPosition(2, 4);
				outText(Common::String(_menuText[0]._line));
				setTextColor(15);
				switch (keyPressed) {
				case '2':
					readText(1);
					break;
				case '3':
					readText(2);
					break;
				case '4':
					readText(3);
					break;
				case '5':
					readText(4);
					break;
				}
			} else if (mode == COMPUTER_SAMANTHAS) {
				clearScreen();
				setTextColor(4);
				setTextPosition(2, 4);
				outText(Common::String(_menuText[1]._line));
				setTextColor(15);
				switch (keyPressed) {
				case '2':
					readText(6);
					break;
				case '3':
					readText(7);
					break;
				case '4':
					readText(8);
					break;
				case '5':
					readText(9);
					break;
				case '6':
					readText(10);
					_vm->_globals._saveData->data[svField270] = 4;
					break;
				}
			}
		}
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
		restoreFBIRoom();
	} else {
		// Password doesn't match - Access Denied
		setTextColor(4);
		setTextPosition(16, 25);
		outText(Common::String(_menuText[5]._line));
		_vm->_eventsManager.VBL();
		_vm->_eventsManager.delay(1000);

		memset(_vm->_graphicsManager._vesaBuffer, 0, 307199);
		_vm->_graphicsManager.lockScreen();
		_vm->_graphicsManager.clearScreen();
		_vm->_graphicsManager.unlockScreen();
		_vm->_graphicsManager.DD_VBL();
		restoreFBIRoom();
		_vm->_eventsManager.mouseOff();
	}
	if (mode == 1)
		_vm->_globals._exitId = 13;
	if ((uint16)(mode - 2) <= 1u)
		_vm->_globals._exitId = 14;

	_vm->_graphicsManager.RESET_SEGMENT_VESA();
}

/**
 * Load Menu data
 */
void ComputerManager::loadMenu() {
	_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "COMPUTAN.TXT");
	byte *ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	byte *tmpPtr = ptr;
	int lineNum = 0;
	int strPos;
	bool loopCond = false;

	do {
		if (tmpPtr[0] == '%') {
			if (tmpPtr[1] == '%') {
				loopCond = true;
				break;
			}
			_menuText[lineNum]._actvFl = 1;
			strPos = 0;
			while (strPos <= 89) {
				byte curChar = tmpPtr[strPos + 2];
				if (curChar == '%' || curChar == 10)
					break;
				_menuText[lineNum]._line[strPos++] = curChar;
			}
			if (strPos <= 89) {
				_menuText[lineNum]._line[strPos] = 0;
				_menuText[lineNum]._lineSize = strPos - 1;
			}
			++lineNum;
		}
		tmpPtr = tmpPtr + 1;
	} while (!loopCond);
	ptr = _vm->_globals.freeMemory(ptr);
}

void ComputerManager::TXT4(int xp, int yp, int textIdx) {
	char curChar;
	char newChar;
	char mappedChar;
	Common::String charString;

	int x1 = xp;
	int x2 = 0;

	int textIndex = 0;
	bool oldMouseFlag = _vm->_eventsManager._mouseFl;
	_vm->_eventsManager._mouseFl = false;

	_vm->_fontManager.displayTextVesa(xp, yp, "_", -4);
	do {
		curChar = _vm->_eventsManager.waitKeyPress();
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
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager._vesaScreen, x1, yp, 3 * _vm->_globals.police_l, 12, _vm->_graphicsManager._vesaBuffer, x1, yp);
			_vm->_graphicsManager.addVesaSegment(x1, yp, x2, yp + 12);
			_vm->_fontManager.displayTextVesa(x1, yp, "_", -4);
		}
		if (mappedChar != '*') {
			newChar = mappedChar;
			_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager._vesaScreen, x1, yp, _vm->_globals.police_l, 12, _vm->_graphicsManager._vesaBuffer, x1, yp);
			_vm->_graphicsManager.addVesaSegment(x1, yp, _vm->_globals.police_l + x1, yp + 12);
			_inputBuf[textIndex] = newChar;

			charString = Common::String::format("%c_", newChar);
			_vm->_fontManager.displayTextVesa(x1, yp, charString, -4);
			++textIndex;
			x1 += _vm->_globals.police_l;
		}
		_vm->_eventsManager.VBL();
	} while (textIndex != textIdx && curChar != 13);

	_vm->_graphicsManager.Copy_Mem(_vm->_graphicsManager._vesaScreen, x1, yp, _vm->_globals.police_l,
		12, _vm->_graphicsManager._vesaBuffer, x1, yp);
	_vm->_graphicsManager.addVesaSegment(x1, yp, _vm->_globals.police_l + x1, yp + 12);

	_vm->_eventsManager.VBL();
	_inputBuf[textIndex] = 0;
	_vm->_eventsManager._mouseFl = oldMouseFlag;
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
	_vm->_fontManager.displayTextVesa(_textPosition.x, _textPosition.y, msg, _textColor);
}

/**
 * Restores the scene for the FBI headquarters room
 */
void ComputerManager::restoreFBIRoom() {
	_vm->_globals.police = _vm->_globals.freeMemory(_vm->_globals.police);
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "FONTE3.SPR");
	_vm->_globals.police = _vm->_fileManager.loadFile(_vm->_globals._curFilename);

	_vm->_globals.police_l = 12;
	_vm->_globals.police_h = 21;
	_vm->_eventsManager._mouseFl = true;
}

/**
 * Display texts for the given menu entry
 */
void ComputerManager::readText(int idx) {
	uint16 v1;
	bool foundFl;
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

	_vm->_eventsManager._escKeyFl = false;

	if (_vm->_globals._language == LANG_EN)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPKAN.TXT");
	else if (_vm->_globals._language == LANG_FR)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPK.TXT");
	else if (_vm->_globals._language == LANG_SP)
		_vm->_fileManager.constructFilename(_vm->_globals.HOPLINK, "THOPKES.TXT");

	ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	v1 = _vm->_fileManager.fileSize(_vm->_globals._curFilename);
	foundFl = false;
	int v3;
	for (v3 = 0; v3 < v1; v3++) {
		if (ptr[v3] == '%') {
			numStr = Common::String::format("%c%c", ptr[v3 + 1], ptr[v3 + 2]);
			num	= atol(numStr.c_str());

			if (num == idx)
				foundFl = true;
		}
		if (foundFl)
			break;
	}
	v4 = v3;
	if (v3 > v1 - 1)
		error("Error with Hopkins computer file");

	v10 = v3 + 3;
	v5 = 1;
	v6 = 5;
	v7 = 0;
	do {
		v4 = ptr[v10];
		if (v4 == 13) {
			v8 = v4;
			setTextPosition(v6, v5);
			outText(v12);

			++v6;
			v5 = 1;
			_vm->_eventsManager.VBL();
			v4 = v8;
			v12 = "";
		} else if (v4 != '%') {
			v12 += v4;
			++v7;
		}
		++v10;
	} while (v4 != '%');

	_vm->_eventsManager.waitKeyPress();
	ptr = _vm->_globals.freeMemory(ptr);
}

/**
 * Display breakout when Games sub-menu is selected
 */
void ComputerManager::displayGamesSubMenu() {
	const byte *v1 = _vm->_objectsManager._sprite[0]._spriteData;
	uint oldSpeed = _vm->_globals._speed;

	_vm->_globals._speed = 1;
	_vm->_eventsManager.changeMouseCursor(0);
	_breakoutSpr = g_PTRNUL;
	_vm->_eventsManager._breakoutFl = true;
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
	_vm->_soundManager.loadSample(1, "SOUND37.WAV");
	_vm->_soundManager.loadSample(2, "SOUND38.WAV");
	_vm->_soundManager.loadSample(3, "SOUND39.WAV");
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "CASSE.SPR");
	_breakoutSpr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	loadHiscore();
	setModeVGA256();
	newLevel();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	playBreakout();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	_breakoutSpr = _vm->_globals.freeMemory(_breakoutSpr);
	_breakoutLevel = (int16 *)_vm->_globals.freeMemory((byte *)_breakoutLevel);
	_vm->_objectsManager._sprite[0]._spriteData = v1;

	_vm->_soundManager.DEL_SAMPLE(1);
	_vm->_soundManager.DEL_SAMPLE(2);
	_vm->_soundManager.DEL_SAMPLE(3);
	_vm->_globals._speed = oldSpeed;
	_vm->_eventsManager._breakoutFl = false;
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
	ptr = _vm->_globals.allocMemory(100);
	_vm->_saveLoadManager.load(_vm->_globals._curFilename, ptr);

	for (int scoreIndex = 0; scoreIndex < 6; ++scoreIndex) {
		for (int i = 0; i < 5; ++i) {
			nextChar = ptr[i + (16 * scoreIndex)];
			if (!nextChar)
				nextChar = ' ';
			_score[scoreIndex]._name += nextChar;
		}

		for (int i = 0; i < 9; ++i) {
			nextChar = ptr[i + scoreIndex * 16 + 6];
			if (!nextChar)
				nextChar = '0';
			_score[scoreIndex]._score += nextChar;
		}
	}

	_vm->_globals.freeMemory(ptr);
	_breakoutHiscore = atol(_score[5]._score.c_str());
}

/**
 * VGA 256 col
 */
void ComputerManager::setModeVGA256() {
	_vm->_graphicsManager.lockScreen();
	_vm->_graphicsManager.clearScreen();
	_vm->_graphicsManager.unlockScreen();
	_vm->_graphicsManager.clearPalette();
	_vm->_graphicsManager.SCANLINE(320);
}

/**
 * Load new level
 */
void ComputerManager::newLevel() {
	Common::String file;
	Common::File f;

	_vm->_objectsManager.removeSprite(0);
	_vm->_objectsManager.removeSprite(1);
	++_breakoutLives;
	if (_breakoutLives > 11)
		_breakoutLives = 11;
	_vm->_graphicsManager.loadVgaImage("CASSEF.PCX");
	displayLives();
	_breakoutLevel = (int16 *)_vm->_globals.freeMemory((byte *)_breakoutLevel);

	++_breakoutLevelNbr;
	while (!_vm->shouldQuit()) {
		file = Common::String::format("TAB%d.TAB", _breakoutLevelNbr);

		_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, file);
		if (f.open(_vm->_globals._curFilename))
			break;

		_breakoutLevelNbr = 1;
	}
	f.close();

	_breakoutLevel = (int16 *)_vm->_fileManager.loadFile(_vm->_globals._curFilename);
	displayBricks();
	_vm->_objectsManager.SPRITE(_breakoutSpr, Common::Point(150, 192), 0, 13, 0, 0, 0, 0);
	_vm->_objectsManager.SPRITE(_breakoutSpr, Common::Point(164, 187), 1, 14, 0, 0, 0, 0);
	_ballPosition = Common::Point(164, 187);
	_padPositionX = 150;
	_vm->_objectsManager.SPRITE_ON(0);
	_vm->_objectsManager.SPRITE_ON(1);
	_vm->_eventsManager.mouseOn();
	_vm->_soundManager.playSample(3, 5);
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
	int xp = 10;
	for (int i = 0; i <= 11; i++) {
		_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, 10, 15);
		xp += 7;
	}

	xp = 10;
	for (int i = 0; i < _breakoutLives - 1; i++) {
		_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, 10, 14);
		xp += 7;
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
			_vm->_eventsManager.mouseOff();
			_ballPosition = Common::Point(_padPositionX + 14, 187);
			_vm->_objectsManager.setSpriteY(1, 187);
			_vm->_objectsManager.setSpriteX(1, _ballPosition.x);
			_vm->_graphicsManager.RESET_SEGMENT_VESA();
			_vm->_eventsManager.VBL();
			_vm->_graphicsManager.fadeInBreakout();

			// Wait for mouse press to start playing
			do {
				_padPositionX = _vm->_eventsManager.getMouseX();
				if (_vm->_eventsManager._mousePos.x <= 4)
					_padPositionX = 5;
				if (_padPositionX > 282)
					_padPositionX = 282;
				_vm->_objectsManager.setSpriteX(0, _padPositionX);
				_vm->_objectsManager.setSpriteX(1, _padPositionX + 14);
				_vm->_objectsManager.setSpriteY(1, 187);
				_vm->_eventsManager.VBL();
			} while (!_vm->shouldQuit() && _vm->_eventsManager.getMouseButton() != 1);

			_breakoutSpeed = 1;
			_ballPosition = Common::Point(_padPositionX + 14, 187);
			_ballRightFl = (_padPositionX > 135);
			_ballUpFl = false;

			// Play loop
			do {
				_vm->_soundManager.checkSounds();

				_padPositionX = _vm->_eventsManager.getMouseX();
				if (_vm->_eventsManager._mousePos.x <= 4)
					_padPositionX = 5;
				if (_padPositionX > 282)
					_padPositionX = 282;
				_vm->_objectsManager.setSpriteX(0, _padPositionX);
				v1 = moveBall();
				_vm->_eventsManager.VBL();
			} while (!_vm->shouldQuit() && !v1);
			if (v1 != 1)
				break;
			_vm->_graphicsManager.fateOutBreakout();
			--_breakoutLives;

			if (_breakoutLives) {
				displayLives();
				if (_breakoutLives)
					continue;
			}
			_vm->_eventsManager.mouseOn();
			_vm->_objectsManager.removeSprite(0);
			_vm->_objectsManager.removeSprite(1);
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
		_vm->_graphicsManager.fateOutBreakout();
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
	_vm->_graphicsManager.loadVgaImage("HISCORE.PCX");
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "ALPHA.SPR");
	ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
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

	_vm->_graphicsManager.fadeInBreakout();
	_vm->_graphicsManager.RESET_SEGMENT_VESA();
	buttonIndex = 0;
	do {
		_vm->_eventsManager.refreshEvents();
		xp = _vm->_eventsManager.getMouseX();
		yp = _vm->_eventsManager.getMouseY();

		if (_vm->_eventsManager.getMouseButton() == 1 && ABS(xp - 79) <= 33 && ABS(yp - 396) <= 13)
			buttonIndex = 1;
		else if (_vm->_eventsManager.getMouseButton() == 1 && ABS(xp - 583) <= 32 && ABS(yp - 396) <= 13)
			buttonIndex = 2;

		_vm->_eventsManager.VBL();
	} while (!buttonIndex && !_vm->shouldQuit());

	_vm->_eventsManager.mouseOff();
	_vm->_graphicsManager.fateOutBreakout();
	_vm->_globals.freeMemory(ptr);
	return buttonIndex;
}

/**
 * Display a screen to enter player name in the case of a new hiscore
 */
void ComputerManager::getScoreName() {
	char curChar;
	byte *ptr;

	_vm->_graphicsManager.loadVgaImage("NAME.PCX");
	_vm->_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_vm->_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_vm->_fileManager.constructFilename(_vm->_globals.HOPSYSTEM, "ALPHA.SPR");
	ptr = _vm->_fileManager.loadFile(_vm->_globals._curFilename);
	_vm->_graphicsManager.fadeInBreakout();
	for (int strPos = 0; strPos <= 4; strPos++) {
		displayHiscoreLine(ptr, 9 * strPos + 140, 78, 1);

		curChar = toupper(_vm->_eventsManager.waitKeyPress());
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
	_vm->_graphicsManager.fateOutBreakout();
	_vm->_globals.freeMemory(ptr);
	saveScore();
}

/**
 * Display current score
 */
void ComputerManager::displayScore() {
	char s[40];

	sprintf(s, "%d", _breakoutScore);
	int v0 = 0;
	do
		++v0;
	while (s[v0]);
	int v1 = 0;
	for (int i = v0; i > -1; i--) {
		IMPSCORE(v1++, (byte)s[i]);
	}
}

void ComputerManager::IMPSCORE(int charPos, int charDisp) {
	int16 xp = 200;
	int16 idx = 3;

	if (charPos == 1)
		xp = 190;
	else if (charPos == 2)
		xp = 180;
	else if (charPos == 3)
		xp = 167;
	else if (charPos == 4)
		xp = 157;
	else if (charPos == 5)
		xp = 147;
	else if (charPos == 9)
		xp = 134;

	if (charDisp >= 48 && charDisp <= 57)
	    idx = charDisp - 45;

	_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, xp, 11, idx);
}

/**
 * Save Hiscore in file
 */
void ComputerManager::saveScore() {
	int v1;
	int v2;
	int v4;
	int v6;
	int v9;
	int v14;
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

	byte *ptr = _vm->_globals.allocMemory(100);
	memset(ptr, 0, 99);
	for (int v5 = 0; v5 <= 5; v5++) {
		v6 = 16 * v5;
		v14 = v16[v5];
		for (int namePos = 0; namePos <= 4; namePos++) {
			char curChar = _score[v14]._name[namePos];
			if (!curChar)
				curChar = ' ';
			ptr[v6 + namePos] = curChar;
		};

		ptr[v6 + 5] = 0;
		v9 = v6 + 6;

		for (int scorePos = 0; scorePos <= 8; scorePos++) {
			char curChar = _score[v14]._score[scorePos];
			if (!curChar)
				curChar = '0';
			ptr[v9 + scorePos] = curChar;
		};
		ptr[v9 + 9] = 0;
	}

	_vm->_fileManager.constructLinuxFilename("HISCORE.DAT");
	_vm->_saveLoadManager.saveFile(_vm->_globals._curFilename, ptr, 100);
	_vm->_globals.freeMemory(ptr);
}

/**
 * Display parts of the hiscore line
 */
void ComputerManager::displayHiscoreLine(byte *objectData, int x, int y, int curChar) {
	int idx = 36;

	if (curChar == 100)
		idx = 0;
	else if (curChar >= '0' && curChar <= '9')
		idx = curChar - '0';
	else if (curChar >= 'A' && curChar <= 'Z')
		idx = curChar - 'A' + 10;
	else if (curChar == 1)
		idx = 37;
	_vm->_graphicsManager.AFFICHE_SPEEDVGA(objectData, x, y, idx);
}

/**
 * Handle ball moves
 */
int ComputerManager::moveBall() {
	int16 retVal = 0;
	//(signed int)(6.0 * (long double)_vm->getRandomNumber( rand() / 2147483648.0) + 1;
	// TODO: Figure out random number
	int randVal = _vm->getRandomNumber(6);
	switch (_breakoutSpeed) {
	case 1:
		_minBreakoutMoveSpeed = 1;
		_maxBreakoutMoveSpeed = 1;
		break;
	case 2:
		_minBreakoutMoveSpeed = 1;
		_maxBreakoutMoveSpeed = 2;
		break;
	case 3:
		_minBreakoutMoveSpeed = 2;
		_maxBreakoutMoveSpeed = 2;
		break;
	case 4:
		_minBreakoutMoveSpeed = 3;
		_maxBreakoutMoveSpeed = 2;
		break;
	}

	int moveSpeed = _minBreakoutMoveSpeed;
	if (_lastBreakoutMoveSpeed == _minBreakoutMoveSpeed)
		moveSpeed = _maxBreakoutMoveSpeed;

	if (_ballUpFl)
		_ballPosition.y += moveSpeed;
	else
		_ballPosition.y -= moveSpeed;

	if (_ballRightFl)
		_ballPosition.x += moveSpeed;
	else
		_ballPosition.x -= moveSpeed;

	_lastBreakoutMoveSpeed = moveSpeed;
	if (_ballPosition.x <= 6) {
		_vm->_soundManager.playSample(2, 6);
		_ballPosition.x = randVal + 6;
		_ballRightFl = !_ballRightFl;
	}
	if (_ballPosition.x > 307) {
		_vm->_soundManager.playSample(2, 6);
		_ballPosition.x = 307 - randVal;
		_ballRightFl = !_ballRightFl;
	}
	if (_ballPosition.y <= 6) {
		_vm->_soundManager.playSample(2, 6);
		_ballPosition.y = randVal + 7;
		_ballUpFl = !_ballUpFl;
	}
	if (_ballPosition.y >= 186 && _ballPosition.y <= 194) {
		_vm->_soundManager.playSample(2, 6);
		if (_ballPosition.x > _padPositionX - 2) {
			int v2 = _ballPosition.x + 6;
			if (v2 < _padPositionX + 36) {
				_ballUpFl = false;
				if (v2 <= _padPositionX + 15) {
					_ballRightFl = false;
					if (_ballPosition.x >= _padPositionX && v2 <= _padPositionX + 5)
						_ballPosition.x -= 4;
					if (_ballPosition.x >= _padPositionX + 5 && _ballPosition.x + 6 <= _padPositionX + 10)
						_ballPosition.x -= 2;
				}
				if (_ballPosition.x >= _padPositionX + 19 && _ballPosition.x + 6 <= _padPositionX + 36) {
					_ballRightFl = true;
					if (_ballPosition.x >= _padPositionX + 29)
						_ballPosition.x += 4;
					if (_ballPosition.x >= _padPositionX + 24 && _ballPosition.x + 6 <= _padPositionX + 29)
						_ballPosition.x += 2;
				}
			}
		}
	}
	if (_ballPosition.y > 194)
		retVal = 1;
	checkBallCollisions();
	_vm->_objectsManager.setSpriteX(1, _ballPosition.x);
	_vm->_objectsManager.setSpriteY(1, _ballPosition.y);
	if (!_breakoutBrickNbr)
		retVal = 2;
	return retVal;
}

/**
 * Check ball collision with bricks
 */
void ComputerManager::checkBallCollisions() {
	int cellLeft;
	int cellRight;
	int cellType;
	bool collisionFl;
	int cellBottom;
	int cellUp;

	bool v7 = false;
	// TODO: Check if correct
	int randVal = _vm->getRandomNumber(6) + 1;
	int ballLeft = _ballPosition.x;
	int ballTop = _ballPosition.y;
	int ballRight = _ballPosition.x + 6;
	int ballBottom = _ballPosition.y + 6;
	int16 *level = _breakoutLevel;
	uint16 v8 = 0;
	do {
		cellLeft = level[v8];
		cellUp = level[v8 + 1];
		cellRight = level[v8 + 2];
		cellBottom = level[v8 + 3];
		cellType = level[v8 + 4];
		if (level[v8 + 5] == 1 && cellLeft != -1) {
			collisionFl = false;
			if (ballTop <= cellBottom && ballBottom >= cellBottom) {
				if (ballLeft >= cellLeft && ballRight <= cellRight) {
					collisionFl = true;
					_ballUpFl = true;
				}
				if (ballRight >= cellLeft) {
					if (ballLeft <= cellLeft) {
						collisionFl = true;
						_ballUpFl = true;
						_ballRightFl = false;
						if (cellType == 31)
							_ballPosition.x -= randVal;
					}
				}
				if (ballLeft <= cellRight) {
					if (ballRight >= cellRight) {
						collisionFl = true;
						_ballUpFl = true;
						_ballRightFl = true;
						if (cellType == 31)
							_ballPosition.x += randVal;
					}
				}
			}
			if (ballBottom >= cellUp && ballTop <= cellUp) {
				if (ballLeft >= cellLeft && ballRight <= cellRight) {
					collisionFl = true;
					_ballUpFl = false;
				}
				if (ballRight >= cellLeft) {
					if (ballLeft <= cellLeft) {
						collisionFl = true;
						_ballUpFl = false;
						_ballRightFl = false;
						if (cellType == 31)
							_ballPosition.x -= 2;
					}
				}
				if (ballLeft <= cellRight) {
					if (ballRight >= cellRight) {
						collisionFl = true;
						_ballUpFl = false;
						_ballRightFl = true;
						if (cellType == 31)
							_ballPosition.x += randVal;
					}
				}
			}
			if (ballTop >= cellUp) {
				if (ballBottom <= cellBottom) {
					if (ballRight >= cellLeft) {
						if (ballLeft <= cellLeft) {
							collisionFl = true;
							_ballRightFl = false;
							if (cellType == 31)
								_ballPosition.x -= randVal;
						}
					}
					if (ballLeft <= cellRight) {
						if (ballRight >= cellRight) {
							collisionFl = true;
							_ballRightFl = true;
							if (cellType == 31)
								_ballPosition.x += randVal;
						}
					}
				}
			}
			if (collisionFl) {
				if (cellType == 31) {
					_vm->_soundManager.playSample(2, 6);
				} else {
					_vm->_soundManager.playSample(1, 5);
					_vm->_graphicsManager.AFFICHE_SPEEDVGA(_breakoutSpr, cellLeft, cellUp, 16);
					if (cellType == 1)
						_breakoutScore += 10;
					if (cellType == 2)
						_breakoutScore += 5;
					if (cellType == 3) {
						_breakoutScore += 50;
						if (_breakoutSpeed <= 1)
							_breakoutSpeed = 2;
						if (_breakoutBrickNbr <= 19)
							_breakoutSpeed = 3;
					}
					if (cellType == 4)
						_breakoutScore += 20;
					if (cellType == 5) {
						_breakoutScore += 30;
						if (_breakoutSpeed <= 1)
							_breakoutSpeed = 2;
					}
					if (cellType == 6)
						_breakoutScore += 40;
					displayScore();
					--_breakoutBrickNbr;
					*((uint16 *)level + v8 + 5) = 0;
					v7 = true;
				}
			}
		}

		if (v7)
			cellLeft = -1;
		v8 += 6;
	} while (cellLeft != -1);
}

} // End of namespace Hopkins
