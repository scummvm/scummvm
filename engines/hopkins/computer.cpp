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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "hopkins/computer.h"

#include "hopkins/font.h"
#include "hopkins/files.h"
#include "hopkins/globals.h"
#include "hopkins/graphics.h"
#include "hopkins/hopkins.h"
#include "hopkins/objects.h"

#include "common/system.h"
#include "common/file.h"
#include "common/textconsole.h"

namespace Hopkins {

ComputerManager::ComputerManager(HopkinsEngine *vm) {
	_vm = vm;

	for (int i = 0; i < ARRAYSIZE(_menuText); i++) {
		_menuText[i]._lineSize = 0;
		memset(_menuText[i]._line, 0, ARRAYSIZE(_menuText[0]._line));
	}
	Common::fill(&_inputBuf[0], &_inputBuf[200], '\0');
	_breakoutSpr = NULL;
	_textColor = 0;
	_breakoutLevel = (int16 *)NULL;
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
	_lowestHiScore = 0;
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
	_vm->_graphicsMan->clearPalette();
	_vm->_graphicsMan->clearScreen();

	_vm->_graphicsMan->_lineNbr = SCREEN_WIDTH;
	_vm->_fontMan->_font = _vm->_globals->freeMemory(_vm->_fontMan->_font);

	Common::String filename = "STFONT.SPR";
	Common::File f;
	if (!f.exists(filename))
		filename = "FONTE.SPR"; // Used by the BeOS and OS/2 versions as an alternative
	_vm->_fontMan->_font = _vm->_fileIO->loadFile(filename);
	_vm->_fontMan->_fontFixedWidth = 8;
	_vm->_fontMan->_fontFixedHeight = 8;

	_vm->_graphicsMan->loadImage("WINTEXT");
	_vm->_graphicsMan->fadeInLong();
	loadMenu();
	_vm->_events->_mouseFl = false;
}

/**
 * Clear the screen
 */
void ComputerManager::clearScreen() {
	_vm->_graphicsMan->loadImage("WINTEXT");
	_vm->_graphicsMan->fadeInLong();
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
	_vm->_events->_escKeyFl = false;
	_vm->_graphicsMan->resetDirtyRects();
	setVideoMode();
	setTextColor(4);
	setTextPosition(2, 4);
	if (mode == COMPUTER_HOPKINS)
		outText(Common::String(_menuText[0]._line));
	else if (mode == COMPUTER_SAMANTHA)
		outText(Common::String(_menuText[1]._line));
	else // COMPUTER_PUBLIC
		outText(Common::String(_menuText[2]._line));

	setTextColor(1);
	if (mode == COMPUTER_PUBLIC) {
		setTextPosition(10, 8);
		outText(Common::String(_menuText[3]._line));
	}
	setTextPosition(12, 28);
	outText(Common::String(_menuText[4]._line));
	setTextPosition(14, 35);

	displayMessage(280, 224, 8);
	bool passwordMatch = false;
	if ((mode == COMPUTER_HOPKINS) && !strcmp(_inputBuf, "HOPKINS"))
		passwordMatch = true;
	else if ((mode == COMPUTER_SAMANTHA) && !strcmp(_inputBuf, "328MHZA"))
		passwordMatch = true;
	else if ((mode == COMPUTER_PUBLIC) && !strcmp(_inputBuf, "ALLFREE"))
		passwordMatch = true;

	if (passwordMatch) {
		while (!_vm->shouldQuit()) {
			_vm->_events->_escKeyFl = false;
			clearScreen();
			setTextColor(4);
			setTextPosition(2, 4);
			if (mode == COMPUTER_HOPKINS)
				outText(Common::String(_menuText[0]._line));
			else if (mode == COMPUTER_SAMANTHA)
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
			} else if (mode == COMPUTER_SAMANTHA) {
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
				keyPressed = _vm->_events->waitKeyPress();
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
				default:
					break;
				}
			} else if (mode == COMPUTER_SAMANTHA) {
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
					_vm->_globals->_saveData->_data[svField270] = 4;
					break;
				default:
					break;
				}
			}
		}
		_vm->_graphicsMan->clearScreen();
		_vm->_graphicsMan->updateScreen();
		restoreFBIRoom();
	} else {
		// Password doesn't match - Access Denied
		setTextColor(4);
		setTextPosition(16, 25);
		outText(Common::String(_menuText[5]._line));
		_vm->_events->refreshScreenAndEvents();
		_vm->_events->delay(1000);

		memset(_vm->_graphicsMan->_frontBuffer, 0, 307199);
		_vm->_graphicsMan->clearScreen();
		_vm->_graphicsMan->updateScreen();
		restoreFBIRoom();
		_vm->_events->mouseOff();
	}

	if (mode == COMPUTER_HOPKINS)
		_vm->_globals->_exitId = 13;
	else // Free access or Samantha
		_vm->_globals->_exitId = 14;

	_vm->_graphicsMan->resetDirtyRects();
}

static const char _englishText[] =
"% ****** FBI COMPUTER NUMBER 4985 ****** J.HOPKINS COMPUTER ******\n"
"% ****** FBI COMPUTER NUMBER 4998 ****** S.COLLINS COMPUTER ******\n"
"% ****** FBI COMPUTER NUMBER 4997 ****** ACCES FREE COMPUTER ******\n"
"% PASSWORD IS: ALLFREE\n% ENTER CURRENT PASSWORD\n"
"% ****** ACCES DENIED ******\n"
"% 1) *** GAME ***\n"
"% 0) QUIT COMPUTER\n"
"% 2) STRANGE CADAVER\n"
"% 3) STRANGE CADAVER\n"
"% 4) SENATOR FERGUSSON\n"
"% 5) DOG KILLER\n"
"% 2) SCIENTIST KIDNAPPED.\n"
"% 3) SCIENTIST KIDNAPPED (next).\n"
"% 4) SCIENTIST KIDNAPPED (next).\n"
"% 5) SCIENTIST KIDNAPPED (next).\n"
"% 6) SCIENTIST KIDNAPPED (next).\n"
"%% fin\n";

static const char _frenchText[] =
"% ****** FBI COMPUTER NUMBER 4985 ****** J.HOPKINS COMPUTER ******\n"
"% ****** FBI COMPUTER NUMBER 4998 ****** S.COLLINS COMPUTER ******\n"
"% ****** FBI COMPUTER NUMBER 4997 ****** ACCES FREE COMPUTER ******\n"
"% PASSWORD IS: ALLFREE\n"
"% ENTER CURRENT PASSWORD\n"
"% ****** ACCES DENIED ******\n"
"% 1) *** CASSE BRIQUE ***\n"
"% 0) QUITTER L'ORDINATEUR\n"
"% 2) CADAVRE SANS TETE\n"
"% 3) CADAVRE SANS TETE\n"
"% 4) AGRESSION DU SENATEUR\n"
"% 5) LES CHIENS TUEURS\n"
"% 2) DISPARITIONS DE CHERCHEURS.\n"
"% 3) DISPARITIONS (suite).\n"
"% 4) DISPARITIONS (suite).\n"
"% 5) DISPARITIONS (suite).\n"
"% 6) DISPARITIONS (suite).\n"
"%% fin\n";

static const char _spanishText[] =
"% **** ORDENADOR DEL FBI NUMERO 4985 **** ORDENADOR J.HOPKINS *****\n"
"% **** ORDENADOR DEL FBI NUMERO 4998 **** ORDENADOR S.COLLINS *****\n"
"% *** ORDENADOR DEL FBI NUMERO 4997 *** ORDENADOR DE ACCESO LIBRE ***\n"
"% LA CONTRASE\xA5" "A ES: ALLFREE\n"
"% ESCRIBE CONTRASE\xA5" "A ACTUAL\n"
"% **** ACCESO DENEGADO ****\n"
"% 1) *** JUEGO ***\n"
"% 0) SALIR DEL ORDENADOR\n"
"% 2) CADAVER EXTRA\xA5" "O\n"
"% 3) CADAVER EXTRA\xA5" "O\n"
"% 4) SENADOR FERGUSSON\n"
"% 5) MATAPERROS\n"
"% 2) CIENTIFICO SECUESTRADO.\n"
"% 3) CIENTIFICO SECUESTRADO (siguiente).\n"
"% 4) CIENTIFICO SECUESTRADO (siguiente).\n"
"% 5) CIENTIFICO SECUESTRADO (siguiente).\n"
"% 6) CIENTIFICO SECUESTRADO (siguiente).\n"
"%% fin\n";

/**
 * Load Menu data
 */
void ComputerManager::loadMenu() {
	debug(9, "ComputerManager::loadMenu()");
	char *ptr;
	if (_vm->_fileIO->fileExists("COMPUTAN.TXT")) {
		ptr = (char *)_vm->_fileIO->loadFile("COMPUTAN.TXT");
	} else {
		switch (_vm->_globals->_language) {
		case LANG_FR:
			ptr = (char *)_vm->_globals->allocMemory(sizeof(_frenchText));
			Common::strlcpy(ptr, _frenchText, sizeof(_frenchText));
			break;
		case LANG_SP:
			ptr = (char *)_vm->_globals->allocMemory(sizeof(_spanishText));
			Common::strlcpy(ptr, _spanishText, sizeof(_spanishText));
			break;
		default:
			ptr = (char *)_vm->_globals->allocMemory(sizeof(_englishText));
			Common::strlcpy(ptr, _englishText, sizeof(_englishText));
			break;
		}
	}

	char *tmpPtr = ptr;
	int lineNum = 0;

	const char lineSep = tmpPtr[0];

	while (tmpPtr[0] != '\0' && lineNum < ARRAYSIZE(_menuText)) {
		if (tmpPtr[0] == '%' && tmpPtr[1] == '%') {
			// End of file marker found - Break out of parse loop
			break;
		}

		if (tmpPtr[0] == lineSep) {
			int strPos = 0;
			while (strPos < ARRAYSIZE(_menuText[0]._line)) {
				char curChar = tmpPtr[strPos + 2];
				if (curChar == '\0' || curChar == lineSep || curChar == 0x0a) // Line Feed
					break;
				_menuText[lineNum]._line[strPos++] = curChar;
			}

			if (strPos < ARRAYSIZE(_menuText[0]._line)) {
				_menuText[lineNum]._line[strPos] = 0;
				_menuText[lineNum]._lineSize = strPos - 1;
			}

			if (strPos != 0) {
				debug(9, "_menuText[%d]._line (size: %d): \"%s\"", lineNum, _menuText[lineNum]._lineSize, _menuText[lineNum]._line);
				++lineNum;
			}
		}
		++tmpPtr;
	}

	_vm->_globals->freeMemory((byte *)ptr);
}

void ComputerManager::displayMessage(int xp, int yp, int textIdx) {
	char curChar;

	int x1 = xp;
	int x2 = 0;

	int textIndex = 0;
	bool oldMouseFlag = _vm->_events->_mouseFl;
	_vm->_events->_mouseFl = false;

	_vm->_fontMan->displayTextVesa(xp, yp, "_", 252);
	do {
		curChar = _vm->_events->waitKeyPress();
		if (_vm->shouldQuit())
			return;

		char mappedChar = '*';

		if ((curChar == '-') || ((curChar >= '0') && (curChar <= '9')) || ((curChar >= 'A') && (curChar <= 'Z')))
			mappedChar = curChar;
		else if ((curChar >= 'a') && (curChar <= 'z'))
			mappedChar = curChar - 32;

		// BackSpace
		if (curChar == 8 && textIndex > 0) {
			_inputBuf[textIndex--] = 0;
			x1 -= _vm->_fontMan->_fontFixedWidth;
			x2 = x1 + 2 * _vm->_fontMan->_fontFixedWidth;
			_vm->_graphicsMan->copyRect(_vm->_graphicsMan->_backBuffer, x1, yp, 3 * _vm->_fontMan->_fontFixedWidth, 12, _vm->_graphicsMan->_frontBuffer, x1, yp);
			_vm->_graphicsMan->addDirtyRect(x1, yp, x2, yp + 12);
			_vm->_fontMan->displayTextVesa(x1, yp, "_", 252);
		}
		if (mappedChar != '*') {
			char newChar = mappedChar;
			_vm->_graphicsMan->copyRect(_vm->_graphicsMan->_backBuffer, x1, yp, _vm->_fontMan->_fontFixedWidth, 12, _vm->_graphicsMan->_frontBuffer, x1, yp);
			_vm->_graphicsMan->addDirtyRect(x1, yp, _vm->_fontMan->_fontFixedWidth + x1, yp + 12);
			_inputBuf[textIndex] = newChar;

			Common::String charString = Common::String::format("%c_", newChar);
			_vm->_fontMan->displayTextVesa(x1, yp, charString, 252);
			++textIndex;
			x1 += _vm->_fontMan->_fontFixedWidth;
		}
		_vm->_events->refreshScreenAndEvents();
	} while (textIndex != textIdx && curChar != 13);

	_vm->_graphicsMan->copyRect(_vm->_graphicsMan->_backBuffer, x1, yp, _vm->_fontMan->_fontFixedWidth, 12, _vm->_graphicsMan->_frontBuffer, x1, yp);
	_vm->_graphicsMan->addDirtyRect(x1, yp, _vm->_fontMan->_fontFixedWidth + x1, yp + 12);

	_vm->_events->refreshScreenAndEvents();
	_inputBuf[textIndex] = 0;
	_vm->_events->_mouseFl = oldMouseFlag;
}

/**
 * Outputs a text string
 */
void ComputerManager::outText(const Common::String &msg) {
	_vm->_fontMan->renderTextDisplay(_textPosition.x, _textPosition.y, msg, _textColor);
}

/**
 * Outputs a text string
 */
void ComputerManager::outText2(const Common::String &msg) {
	_vm->_fontMan->displayTextVesa(_textPosition.x, _textPosition.y, msg, _textColor);
}

/**
 * Restores the scene for the FBI headquarters room
 */
void ComputerManager::restoreFBIRoom() {
	_vm->_graphicsMan->fadeOutShort();

	_vm->_globals->freeMemory(_vm->_fontMan->_font);
	_vm->_fontMan->_font = _vm->_fileIO->loadFile("FONTE3.SPR");
	_vm->_fontMan->_fontFixedWidth = 12;
	_vm->_fontMan->_fontFixedHeight = 21;

	_vm->_events->_mouseFl = true;
}

/**
 * Display texts for the given menu entry
 */
void ComputerManager::readText(int idx) {
	_vm->_events->_escKeyFl = false;

	Common::String filename;
	switch (_vm->_globals->_language) {
	case LANG_EN:
		filename = "THOPKAN.TXT";
		break;
	case LANG_FR:
		filename = "THOPK.TXT";
		break;
	case LANG_SP:
		filename = "THOPKES.TXT";
		break;
	default:
		break;
	}

	byte *ptr = _vm->_fileIO->loadFile(filename);
	uint16 fileSize = _vm->_fileIO->fileSize(filename);
	int pos;
	for (pos = 0; pos < fileSize; pos++) {
		if (ptr[pos] == '%') {
			Common::String numStr = Common::String::format("%c%c", ptr[pos + 1], ptr[pos + 2]);
			if (idx == atol(numStr.c_str()))
				break;
		}
	}
	if (pos > fileSize - 3)
		error("Error with Hopkins computer file");

	pos += 3;
	int lineNum = 5;
	Common::String curStr = "";
	byte curChar;
	do {
		curChar = ptr[pos];
		if (curChar == 13) {
			setTextPosition(lineNum, 1);
			outText(curStr);

			++lineNum;
			_vm->_events->refreshScreenAndEvents();
			curStr = "";
		} else if (curChar != '%') {
			curStr += curChar;
		}
		++pos;
		assert(pos <= fileSize);
	} while (curChar != '%');

	_vm->_events->waitKeyPress();
	ptr = _vm->_globals->freeMemory(ptr);
}

/**
 * Display breakout when Games sub-menu is selected
 */
void ComputerManager::displayGamesSubMenu() {
	const byte *oldSpriteData = _vm->_objectsMan->_sprite[0]._spriteData;
	uint oldSpeed = _vm->_globals->_speed;

	_vm->_globals->_speed = 1;
	_vm->_events->changeMouseCursor(0);
	_breakoutSpr = NULL;
	_vm->_events->_breakoutFl = true;
	_breakoutLevel = (int16 *)NULL;
	_breakoutBrickNbr = 0;
	_breakoutScore = 0;
	_breakoutLives = 5;
	_breakoutSpeed = 1;
	_ballRightFl = false;
	_ballUpFl = false;
	_breakoutLevelNbr = 0;
	_vm->_graphicsMan->_minY = 0;
	_vm->_graphicsMan->_maxX = 320;
	_vm->_graphicsMan->_maxY = 200;
	_vm->_soundMan->loadSample(1, "SOUND37.WAV");
	_vm->_soundMan->loadSample(2, "SOUND38.WAV");
	_vm->_soundMan->loadSample(3, "SOUND39.WAV");
	_breakoutSpr = _vm->_fileIO->loadFile("CASSE.SPR");
	loadHiscore();
	setModeVGA256();

	newLevel();
	_vm->_graphicsMan->updateScreen();

	playBreakout();
	_vm->_graphicsMan->resetDirtyRects();
	_breakoutSpr = _vm->_globals->freeMemory(_breakoutSpr);
	_breakoutLevel = (int16 *)_vm->_globals->freeMemory((byte *)_breakoutLevel);
	_vm->_objectsMan->_sprite[0]._spriteData = oldSpriteData;

	_vm->_soundMan->removeSample(1);
	_vm->_soundMan->removeSample(2);
	_vm->_soundMan->removeSample(3);
	_vm->_globals->_speed = oldSpeed;
	_vm->_events->_breakoutFl = false;
	setVideoMode();
	setTextColor(15);
	clearScreen();
	_vm->_graphicsMan->_maxX = 680;
	_vm->_graphicsMan->_minY = 0;
	_vm->_graphicsMan->_maxY = 460;
}

/**
 * Load Highscore from file
 */
void ComputerManager::loadHiscore() {
	byte *ptr = _vm->_globals->allocMemory(100);
	memset(ptr, 0, 100);

	if (_vm->_saveLoad->saveExists(_vm->getTargetName() + "-highscore.dat"))
		_vm->_saveLoad->load(_vm->getTargetName() + "-highscore.dat", ptr);

	for (int scoreIndex = 0; scoreIndex < 6; ++scoreIndex) {
		_score[scoreIndex]._name = "      ";
		_score[scoreIndex]._score = "         ";

		for (int i = 0; i < 6; ++i) {
			char nextChar = ptr[(16 * scoreIndex) + i];
			if (!nextChar)
				nextChar = ' ';
			_score[scoreIndex]._name.setChar(nextChar, i);
		}

		for (int i = 0; i < 9; ++i) {
			char nextChar = ptr[(scoreIndex * 16) + 6 + i];
			if (!nextChar)
				nextChar = '0';
			_score[scoreIndex]._score.setChar(nextChar, i);
		}
	}

	_lowestHiScore = atol(_score[5]._score.c_str());
	_vm->_globals->freeMemory(ptr);
}

/**
 * VGA 256 col
 */
void ComputerManager::setModeVGA256() {
	_vm->_graphicsMan->clearScreen();
	_vm->_graphicsMan->clearPalette();
	_vm->_graphicsMan->setScreenWidth(320);
}

/**
 * Load new level
 */
void ComputerManager::newLevel() {
	_vm->_objectsMan->removeSprite(0);
	_vm->_objectsMan->removeSprite(1);
	++_breakoutLives;
	if (_breakoutLives > 11)
		_breakoutLives = 11;
	_vm->_graphicsMan->loadVgaImage("CASSEF.PCX");
	displayLives();
	_breakoutLevel = (int16 *)_vm->_globals->freeMemory((byte *)_breakoutLevel);

	++_breakoutLevelNbr;
	Common::String file;
	Common::File f;
	while (!_vm->shouldQuit()) {
		file = Common::String::format("TAB%d.TAB", _breakoutLevelNbr);
		if (f.open(file))
			break;

		_breakoutLevelNbr = 1;
	}
	f.close();

	_breakoutLevel = (int16 *)_vm->_fileIO->loadFile(file);
	displayBricks();

	_vm->_objectsMan->addStaticSprite(_breakoutSpr, Common::Point(150, 192), 0, 13, 0, false, 0, 0);
	_vm->_objectsMan->addStaticSprite(_breakoutSpr, Common::Point(164, 187), 1, 14, 0, false, 0, 0);

	_ballPosition = Common::Point(164, 187);
	_padPositionX = 150;
	_vm->_objectsMan->animateSprite(0);
	_vm->_objectsMan->animateSprite(1);

	_vm->_events->mouseOn();
	_vm->_soundMan->playSample(3, 5);
}

/**
 * Display bricks in breakout game
 */
void ComputerManager::displayBricks() {
	_breakoutBrickNbr = 0;
	_breakoutSpeed = 1;
	int16 *level = _breakoutLevel;

	for (int levelIdx = 0; ; levelIdx += 6) {
		int cellLeft = (int16)FROM_LE_16(level[levelIdx]);
		if (cellLeft == -1)
			break;
		int cellTop = FROM_LE_16(level[levelIdx + 1]);
		int cellType = FROM_LE_16(level[levelIdx + 4]);

		if (cellType <= 6)
			++_breakoutBrickNbr;

		switch (cellType) {
		case 1:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 21);
			break;
		case 2:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 22);
			break;
		case 3:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 17);
			break;
		case 4:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 20);
			break;
		case 5:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 19);
			break;
		case 6:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 18);
			break;
		case 31:
			_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellTop, 23);
			break;
		default:
			break;
		}
	}

	displayScore();

	// Refresh the entire screen
	_vm->_graphicsMan->addRefreshRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	_vm->_graphicsMan->updateScreen();
}

/**
 * Display Lives in breakout game
 */
void ComputerManager::displayLives() {
	for (int i = 0, xp = 10; i <= 11; i++, xp += 7)
		_vm->_graphicsMan->fastDisplay2(_breakoutSpr, xp, 10, 15);

	for (int i = 0, xp = 10; i < _breakoutLives - 1; i++, xp += 7)
		_vm->_graphicsMan->fastDisplay2(_breakoutSpr, xp, 10, 14);

	_vm->_graphicsMan->updateScreen();
}

/**
 * Main function for breakout game
 */
void ComputerManager::playBreakout() {
	int lastBreakoutEvent = 0;
	while (!_vm->shouldQuit()) {
		while (!_vm->shouldQuit()) {
			// Set up the racket and ball
			_vm->_events->mouseOff();
			_ballPosition = Common::Point(_padPositionX + 14, 187);
			_vm->_objectsMan->setSpriteY(1, 187);
			_vm->_objectsMan->setSpriteX(1, _ballPosition.x);

			_vm->_graphicsMan->resetDirtyRects();
			_vm->_events->refreshScreenAndEvents();
			_vm->_graphicsMan->fadeInBreakout();

			// Wait for mouse press to start playing
			do {
				_padPositionX = _vm->_events->getMouseX();
				if (_vm->_events->_mousePos.x <= 4)
					_padPositionX = 5;
				if (_padPositionX > 282)
					_padPositionX = 282;
				_vm->_objectsMan->setSpriteX(0, _padPositionX);
				_vm->_objectsMan->setSpriteX(1, _padPositionX + 14);
				_vm->_objectsMan->setSpriteY(1, 187);
				_vm->_events->refreshScreenAndEvents();
			} while (!_vm->shouldQuit() && _vm->_events->getMouseButton() != 1);

			_breakoutSpeed = 1;
			_ballPosition = Common::Point(_padPositionX + 14, 187);
			_ballRightFl = (_padPositionX > 135);
			_ballUpFl = false;

			// Play loop
			do {
				_vm->_soundMan->checkSounds();

				_padPositionX = _vm->_events->getMouseX();
				if (_vm->_events->_mousePos.x <= 4)
					_padPositionX = 5;
				if (_padPositionX > 282)
					_padPositionX = 282;
				_vm->_objectsMan->setSpriteX(0, _padPositionX);
				lastBreakoutEvent = moveBall();
				_vm->_events->refreshScreenAndEvents();
			} while (!_vm->shouldQuit() && !lastBreakoutEvent);
			if (lastBreakoutEvent != 1)
				break;

			--_breakoutLives;

			if (_breakoutLives) {
				displayLives();
				if (_breakoutLives)
					continue;
			}

			_vm->_graphicsMan->fadeOutBreakout();
			_vm->_events->mouseOn();
			_vm->_objectsMan->removeSprite(0);
			_vm->_objectsMan->removeSprite(1);
			if (_breakoutScore > _lowestHiScore)
				getScoreName();
			if (displayHiscores() != 1)
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
		if (lastBreakoutEvent != 2)
			return;
		_vm->_graphicsMan->fadeOutBreakout();
		newLevel();
	}
}

/**
 * Show the high scores for the Breakout game
 * @return		The selected button index: 1 = Game, 2 = Quit
 */
int ComputerManager::displayHiscores() {
	_vm->_graphicsMan->resetDirtyRects();
	loadHiscore();
	_vm->_graphicsMan->loadVgaImage("HISCORE.PCX");
	byte *ptr = _vm->_fileIO->loadFile("ALPHA.SPR");
	_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);

	int yp;
	// Loop for displaying the scores
	for (int scoreIndex = 0; scoreIndex <= 5; scoreIndex++) {
		yp = 19 * scoreIndex;
		yp += 46;

		// Display the characters of the name
		for (int i = 0; i < 6; i++)
			displayHiscoreLine(ptr, 9 * i + 69, yp, _score[scoreIndex]._name[i]);

		// Display the digits of the score
		for (int i = 0; i < 9; i++)
			displayHiscoreLine(ptr, 9 * i + 199, yp, _score[scoreIndex]._score[i]);
	}

	_vm->_graphicsMan->fadeInBreakout();
	_vm->_graphicsMan->resetDirtyRects();
	int buttonIndex = 0;
	do {
		_vm->_events->refreshEvents();
		int xp = _vm->_events->getMouseX();
		yp = _vm->_events->getMouseY();

		if (_vm->_events->getMouseButton() == 1 && ABS(xp - 79) <= 33 && ABS(yp - 396) <= 13)
			buttonIndex = 1;
		else if (_vm->_events->getMouseButton() == 1 && ABS(xp - 583) <= 32 && ABS(yp - 396) <= 13)
			buttonIndex = 2;

		_vm->_events->refreshScreenAndEvents();
	} while (!buttonIndex && !_vm->shouldQuit());

	_vm->_events->mouseOff();
	_vm->_graphicsMan->fadeOutBreakout();
	_vm->_globals->freeMemory(ptr);
	return buttonIndex;
}

/**
 * Display a screen to enter player name in the case of a new hiscore
 */
void ComputerManager::getScoreName() {
	_vm->_graphicsMan->loadVgaImage("NAME.PCX");
	_vm->_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_vm->_graphicsMan->setColorPercentage(254, 0, 0, 0);
	byte *ptr = _vm->_fileIO->loadFile("ALPHA.SPR");
	_vm->_graphicsMan->fadeInBreakout();

	// Figure out the line to put the new high score on
	int scoreLine = 0;
	while (scoreLine < 5 && _breakoutScore < atol(_score[scoreLine]._score.c_str()))
		++scoreLine;

	// If it's not the lasat line, move the lines down
	for (int line = 5; line > scoreLine; --line) {
		_score[line]._name = _score[line - 1]._name;
		_score[line]._score = _score[line - 1]._score;
	}

	// Get the name for the new high score
	for (int strPos = 0; strPos <= 4; strPos++) {
		displayHiscoreLine(ptr, 9 * strPos + 140, 78, 1);

		char curChar = toupper(_vm->_events->waitKeyPress());
		if ((curChar < '0') || (curChar > 'Z'))
			curChar = ' ';
		if ((curChar > '9') && (curChar < 'A'))
			curChar = ' ';

		_score[scoreLine]._name.setChar(curChar, strPos);
		displayHiscoreLine(ptr, 9 * strPos + 140, 78, curChar);

		for (int idx = 0; idx < 12; ++idx)
			_vm->_events->refreshScreenAndEvents();
	}

	// Set up the new score
	_score[scoreLine]._score = "         ";

	char score[16];
	sprintf(score, "%d", _breakoutScore);
	int scoreLen = 0;
	do {
		++scoreLen;
	} while (score[scoreLen]);

	for (int i = scoreLen - 1, scorePos = 8; i >= 0; i--) {
		_score[scoreLine]._score.setChar(score[i], scorePos--);
	}
	_vm->_graphicsMan->fadeOutBreakout();
	_vm->_globals->freeMemory(ptr);
	saveScore();
}

/**
 * Display current score
 */
void ComputerManager::displayScore() {
	Common::String scoreStr = Common::String::format("%d", _breakoutScore);
	int strSize = scoreStr.size();
	for (int i = strSize - 1, idx = 0; i >= 0; i--) {
		displayScoreChar(idx++, scoreStr[i]);
	}
}

/**
 * Display a character of the score
 */
void ComputerManager::displayScoreChar(int charPos, int charDisp) {
	int xp;
	switch (charPos) {
	case 1:
		xp = 190;
		break;
	case 2:
		xp = 180;
		break;
	case 3:
		xp = 167;
		break;
	case 4:
		xp = 157;
		break;
	case 5:
		xp = 147;
		break;
	case 9:
		xp = 134;
		break;
	default:
		xp = 200;
		break;
	}

	int idx = 3;
	if (charDisp >= '0' && charDisp <= '9')
		idx = charDisp - 45;

	_vm->_graphicsMan->fastDisplay2(_breakoutSpr, xp, 11, idx);
}

/**
 * Save Hiscore in file
 */
void ComputerManager::saveScore() {
	int scores[6];
	// Load high scores in an array
	for (int i = 0; i <= 5; i++) {
		scores[i] = atol(_score[i]._score.c_str());
		if (!scores[i])
			scores[i] = 5;
	}

	int scorePlace[6];
	// order high scores
	for (int scorePlaceIdx = 0; scorePlaceIdx <= 5; scorePlaceIdx++) {
		for(int i = 0;;i++) {
			int curScore = scores[i];
			if (curScore && scores[0] <= curScore && scores[1] <= curScore && scores[2] <= curScore && scores[3] <= curScore
				&& scores[4] <= curScore && scores[5] <= curScore) {
				scorePlace[scorePlaceIdx] = i;
				scores[i] = 0;
				break;
			}
		}
	}

	byte *ptr = _vm->_globals->allocMemory(100);
	memset(ptr, 0, 100);
	for (int scorePlaceIdx = 0; scorePlaceIdx <= 5; scorePlaceIdx++) {
		int curBufPtr = 16 * scorePlaceIdx;
		for (int namePos = 0; namePos < 6; namePos++) {
			char curChar = _score[scorePlace[scorePlaceIdx]]._name[namePos];
			if (!curChar)
				curChar = ' ';
			ptr[curBufPtr + namePos] = curChar;
		};

		ptr[curBufPtr + 5] = 0;

		for (int scorePos = 0; scorePos <= 8; scorePos++) {
			char curChar = _score[scorePlace[scorePlaceIdx]]._score[scorePos];
			if (!curChar)
				curChar = '0';
			ptr[curBufPtr + 6 + scorePos] = curChar;
		};
		ptr[curBufPtr + 15] = 0;
	}

	_vm->_saveLoad->saveFile(_vm->getTargetName() + "-highscore.dat", ptr, 100);
	_vm->_globals->freeMemory(ptr);
}

/**
 * Display parts of the hiscore line
 */
void ComputerManager::displayHiscoreLine(const byte *objectData, int x, int y, int curChar) {
	int idx = 36;

	if (curChar == 100)
		idx = 0;
	else if (curChar >= '0' && curChar <= '9')
		idx = curChar - '0';
	else if (curChar >= 'A' && curChar <= 'Z')
		idx = curChar - 'A' + 10;
	else if (curChar == 1)
		idx = 37;
	_vm->_graphicsMan->fastDisplay2(objectData, x, y, idx);
}

/**
 * Handle ball moves
 */
int ComputerManager::moveBall() {
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
	default:
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
		_vm->_soundMan->playSample(2, 6);
		_ballPosition.x = randVal + 6;
		_ballRightFl = !_ballRightFl;
	} else if (_ballPosition.x > 307) {
		_vm->_soundMan->playSample(2, 6);
		_ballPosition.x = 307 - randVal;
		_ballRightFl = !_ballRightFl;
	}

	if (_ballPosition.y <= 6) {
		_vm->_soundMan->playSample(2, 6);
		_ballPosition.y = randVal + 7;
		_ballUpFl = !_ballUpFl;
	} else if (_ballPosition.y >= 186 && _ballPosition.y <= 194) {
		_vm->_soundMan->playSample(2, 6);
		int ballPosXRight = _ballPosition.x + 6;
		if ((_ballPosition.x > _padPositionX - 2) && (ballPosXRight < _padPositionX + 36)) {
			_ballUpFl = false;
			if (ballPosXRight <= _padPositionX + 15) {
				_ballRightFl = false;
				if (_ballPosition.x >= _padPositionX && ballPosXRight <= _padPositionX + 5)
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

	int retVal = 0;
	if (_ballPosition.y > 194)
		retVal = 1;
	checkBallCollisions();
	_vm->_objectsMan->setSpriteX(1, _ballPosition.x);
	_vm->_objectsMan->setSpriteY(1, _ballPosition.y);
	if (!_breakoutBrickNbr)
		retVal = 2;
	return retVal;
}

/**
 * Check ball collision with bricks
 */
void ComputerManager::checkBallCollisions() {
	int cellLeft;

	bool brickDestroyedFl = false;
	// TODO: Check if correct
	int randVal = _vm->getRandomNumber(6) + 1;
	int ballLeft = _ballPosition.x;
	int ballTop = _ballPosition.y;
	int ballRight = _ballPosition.x + 6;
	int ballBottom = _ballPosition.y + 6;
	int16 *level = _breakoutLevel;
	uint16 levelIdx = 0;
	do {
		cellLeft = level[levelIdx];
		int cellUp = level[levelIdx + 1];
		int cellRight = level[levelIdx + 2];
		int cellBottom = level[levelIdx + 3];
		int cellType = level[levelIdx + 4];
		if (level[levelIdx + 5] == 1 && cellLeft != -1) {
			bool collisionFl = false;
			if (ballTop <= cellBottom && ballBottom >= cellBottom) {
				if (ballLeft >= cellLeft && ballRight <= cellRight) {
					collisionFl = true;
					_ballUpFl = true;
				}
				if ((ballRight >= cellLeft) && (ballLeft <= cellLeft)) {
					collisionFl = true;
					_ballUpFl = true;
					_ballRightFl = false;
					if (cellType == 31)
						_ballPosition.x -= randVal;
				}
				if ((ballLeft <= cellRight) && (ballRight >= cellRight)) {
					collisionFl = true;
					_ballUpFl = true;
					_ballRightFl = true;
					if (cellType == 31)
						_ballPosition.x += randVal;
				}
			}
			if (ballBottom >= cellUp && ballTop <= cellUp) {
				if (ballLeft >= cellLeft && ballRight <= cellRight) {
					collisionFl = true;
					_ballUpFl = false;
				}
				if ((ballRight >= cellLeft) && (ballLeft <= cellLeft)) {
					collisionFl = true;
					_ballUpFl = false;
					_ballRightFl = false;
					if (cellType == 31)
						_ballPosition.x -= 2;
				}
				if ((ballLeft <= cellRight) && (ballRight >= cellRight)) {
					collisionFl = true;
					_ballUpFl = false;
					_ballRightFl = true;
					if (cellType == 31)
						_ballPosition.x += randVal;
				}
			}
			if ((ballTop >= cellUp) && (ballBottom <= cellBottom)) {
				if ((ballRight >= cellLeft) && (ballLeft <= cellLeft)) {
					collisionFl = true;
					_ballRightFl = false;
					if (cellType == 31)
						_ballPosition.x -= randVal;
				}
				if ((ballLeft <= cellRight) && (ballRight >= cellRight)) {
					collisionFl = true;
					_ballRightFl = true;
					if (cellType == 31)
						_ballPosition.x += randVal;
				}
			}
			if (collisionFl) {
				if (cellType == 31) {
					_vm->_soundMan->playSample(2, 6);
				} else {
					_vm->_soundMan->playSample(1, 5);
					_vm->_graphicsMan->fastDisplay2(_breakoutSpr, cellLeft, cellUp, 16);
					switch (cellType) {
					case 1:
						_breakoutScore += 10;
						break;
					case 2:
						_breakoutScore += 5;
						break;
					case 3:
						_breakoutScore += 50;
						if (_breakoutSpeed <= 1)
							_breakoutSpeed = 2;
						if (_breakoutBrickNbr <= 19)
							_breakoutSpeed = 3;
						break;
					case 4:
						_breakoutScore += 20;
						break;
					case 5:
						_breakoutScore += 30;
						if (_breakoutSpeed <= 1)
							_breakoutSpeed = 2;
						break;
					case 6:
						_breakoutScore += 40;
						break;
					default:
						break;
					}
					displayScore();
					--_breakoutBrickNbr;
					level[levelIdx + 5] = 0;
					brickDestroyedFl = true;
				}
			}
		}

		if (brickDestroyedFl)
			cellLeft = -1;
		levelIdx += 6;
	} while (cellLeft != -1);
}

} // End of namespace Hopkins
