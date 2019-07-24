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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#include "mortevielle/mortevielle.h"

#include "mortevielle/dialogs.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"

#include "common/str.h"

namespace Mortevielle {

DialogManager::DialogManager(MortevielleEngine *vm) {
	_vm = vm;
}

/**
 * Alert function - Show
 * @remarks	Originally called 'do_alert'
 */
int DialogManager::show(const Common::String &msg) {
	// Make a copy of the current screen surface for later restore
	_vm->_backgroundSurface.copyFrom(*_vm->_screenSurface);

	_vm->_mouse->hideMouse();
	while (_vm->keyPressed())
		_vm->getChar();

	_vm->setMouseClick(false);

	int colNumb = 0;
	int lignNumb = 0;
	int caseNumb = 0;
	Common::String alertStr = "";
	Common::String caseStr;

	decodeAlertDetails(msg, caseNumb, lignNumb, colNumb, alertStr, caseStr);

	Common::Point curPos;
	if (alertStr == "") {
		drawAlertBox(10, 5, colNumb);
	} else {
		drawAlertBox(8, 7, colNumb);
		int i = -1;
		_vm->_screenSurface->_textPos.y = 70;
		do {
			curPos.x = 320;
			Common::String displayStr = "";
			while ((alertStr[i + 1] != '|') && (alertStr[i + 1] != ']')) {
				++i;
				displayStr += alertStr[i];
				curPos.x -= 3;
			}
			_vm->_screenSurface->putxy(curPos.x, _vm->_screenSurface->_textPos.y);
			_vm->_screenSurface->_textPos.y += 6;
			_vm->_screenSurface->drawString(displayStr, 4);
			++i;
		} while (alertStr[i] != ']');
	}
	int esp;
	if (caseNumb == 1)
		esp = colNumb - 40;
	else
		esp = (uint)(colNumb - caseNumb * 40) / 2;

	int coldep = 320 - ((uint)colNumb / 2) + ((uint)esp / 2);
	Common::String buttonStr[3];
	setButtonText(caseStr, coldep, caseNumb, &buttonStr[0], esp);

	int limit[3][3];
	memset(&limit[0][0], 0, sizeof(int) * 3 * 3);

	limit[1][1] = ((uint)(coldep) / 2) * kResolutionScaler;
	limit[1][2] = limit[1][1] + 40;
	if (caseNumb == 1) {
		limit[2][1] = limit[2][2];
	} else {
		limit[2][1] = ((uint)(320 + ((uint)esp >> 1)) / 2) * kResolutionScaler;
		limit[2][2] = (limit[2][1]) + 40;
	}
	_vm->_mouse->showMouse();
	int id = 0;
	bool dummyFl = false;
	bool test3;
	do {
		char dummyKey = '\377';
		_vm->_mouse->moveMouse(dummyFl, dummyKey);
		if (_vm->shouldQuit())
			return 0;

		curPos = _vm->_mouse->_pos;
		bool newaff = false;
		if ((curPos.y > 95) && (curPos.y < 105)) {
			bool test1 = (curPos.x > limit[1][1]) && (curPos.x < limit[1][2]);
			bool test2 = test1;
			if (caseNumb > 1)
				test2 |= ((curPos.x > limit[2][1]) && (curPos.x < limit[2][2]));
			if (test2) {
				newaff = true;

				int ix;
				if (test1)
					ix = 1;
				else
					ix = 2;
				if (ix != id) {
					_vm->_mouse->hideMouse();
					if (id != 0) {
						setPosition(id, coldep, esp);

						Common::String tmpStr(" ");
						tmpStr += buttonStr[id];
						tmpStr += " ";
						_vm->_screenSurface->drawString(tmpStr, 0);
					}
					setPosition(ix, coldep, esp);

					Common::String tmp2 = " ";
					tmp2 += buttonStr[ix];
					tmp2 += " ";
					_vm->_screenSurface->drawString(tmp2, 1);

					id = ix;
					_vm->_mouse->showMouse();
				}
			}
		}
		if ((id != 0) && !newaff) {
			_vm->_mouse->hideMouse();
			setPosition(id, coldep, esp);

			Common::String tmp3(" ");
			tmp3 += buttonStr[id];
			tmp3 += " ";
			_vm->_screenSurface->drawString(tmp3, 0);

			id = 0;
			_vm->_mouse->showMouse();
		}
		test3 = (curPos.y > 95) && (curPos.y < 105) && (((curPos.x > limit[1][1]) && (curPos.x < limit[1][2]))
		                                    || ((curPos.x > limit[2][1]) && (curPos.x < limit[2][2])));
	} while (!_vm->getMouseClick());
	_vm->setMouseClick(false);
	_vm->_mouse->hideMouse();
	if (!test3)  {
		id = 1;
		setPosition(1, coldep, esp);
		Common::String tmp4(" ");
		tmp4 += buttonStr[1];
		tmp4 += " ";
		_vm->_screenSurface->drawString(tmp4, 1);
	}
	_vm->_mouse->showMouse();

	/* Restore the background area */
	_vm->_screenSurface->copyFrom(_vm->_backgroundSurface, 0, 0);

	return id;
}

/**
 * Alert function - Decode Alert Details
 * @remarks	Originally called 'decod'
 */
void DialogManager::decodeAlertDetails(Common::String inputStr, int &choiceNumb, int &lineNumb, int &col, Common::String &choiceStr, Common::String &choiceListStr) {
	// The second character of the string contains the number of choices
	choiceNumb = atoi(inputStr.c_str() + 1);

	choiceStr = "";
	col = 0;
	lineNumb = 0;

	// Originally set to 5, decreased to 4 because strings are 0 based, and not 1 based as in Pascal
	int i = 4;
	int k = 0;
	bool empty = true;

	for (; inputStr[i] != ']'; ++i) {
		choiceStr += inputStr[i];
		if ((inputStr[i] == '|') || (inputStr[i + 1] == ']')) {
			if (k > col)
				col = k;
			k = 0;
			++lineNumb;
		} else if (inputStr[i] != ' ')
			empty = false;
		++k;
	}

	if (empty)  {
		choiceStr = "";
		col = 20;
	} else {
		choiceStr += ']';
		col += 6;
	}

	choiceListStr = Common::String(inputStr.c_str() + i);
	col *= 6;
}

void DialogManager::setPosition(int ji, int coldep, int esp) {
	_vm->_screenSurface->putxy(coldep + (40 + esp) * (ji - 1), 98);
}

/**
 * Alert function - Draw Alert Box
 * @remarks	Originally called 'fait_boite'
 */
void DialogManager::drawAlertBox(int firstLine, int lineNum, int width) {
	if (width > 640)
		width = 640;
	int x = 320 - ((uint)width / 2);
	int y = (firstLine - 1) * 8;
	int xx = x + width;
	int yy = y + (lineNum * 8);
	_vm->_screenSurface->fillRect(15, Common::Rect(x, y, xx, yy));
	_vm->_screenSurface->fillRect(0, Common::Rect(x, y + 2, xx, y + 4));
	_vm->_screenSurface->fillRect(0, Common::Rect(x, yy - 4, xx, yy - 2));
}

/**
 * Alert function - Set Button Text
 * @remarks	Originally called 'fait_choix'
 */
void DialogManager::setButtonText(Common::String c, int coldep, int nbcase, Common::String *str, int esp) {
	int i = 1;
	int x = coldep;
	for (int l = 1; l <= nbcase; ++l) {
		str[l] = "";
		do {
			++i;
			char ch = c[i];
			str[l] += ch;
		} while (c[i + 1] != ']');
		i += 2;

		while (str[l].size() < 3)
			str[l] += ' ';

		_vm->_screenSurface->putxy(x, 98);

		Common::String tmp(" ");
		tmp += str[l];
		tmp += " ";

		_vm->_screenSurface->drawString(tmp, 0);
		x += esp + 40;
	}
}

/*------------------------------------------------------------------------*/

/**
 * Questions asked before entering the hidden passage
 */
bool DialogManager::showKnowledgeCheck() {
	const int textIndexArr[10] = {511, 516, 524, 531, 545, 552, 559, 563, 570, 576};
	const int correctAnswerArr[10] = {4, 7, 1, 6, 4, 4, 2, 5, 3, 1 };

	Hotspot coor[kMaxHotspots+1];

	for (int i = 0; i <= kMaxHotspots; ++i) {
		coor[i]._rect = Common::Rect();
		coor[i]._enabled = false;
	}

	Common::String choiceArray[15];

	int currChoice;
	int correctCount = 0;

	for (int indx = 0; indx < 10; ++indx) {
		_vm->_mouse->hideMouse();
		_vm->clearScreen();
		_vm->_mouse->showMouse();
		int dialogHeight = 23;
		_vm->_screenSurface->fillRect(15, Common::Rect(0, 14, 630, dialogHeight));
		Common::String tmpStr = _vm->getString(textIndexArr[indx]);
		_vm->_text->displayStr(tmpStr, 20, 15, 100, 2, 0);

		int firstOption;
		int lastOption;

		if (indx != 9) {
			firstOption = textIndexArr[indx] + 1;
			lastOption = textIndexArr[indx + 1] - 1;
		} else {
			firstOption = 503;
			lastOption = 510;
		}
		int optionPosY = 35;
		int maxLength = 0;

		int prevChoice = 1;
		for (int j = firstOption; j <= lastOption; ++j, ++prevChoice) {
			tmpStr = _vm->getString(j);
			if ((int)tmpStr.size() > maxLength)
				maxLength = tmpStr.size();
			_vm->_text->displayStr(tmpStr, 100, optionPosY, 100, 1, 0);
			choiceArray[prevChoice] = tmpStr;
			optionPosY += 8;
		}

		for (int j = 1; j <= lastOption - firstOption + 1; ++j) {
			coor[j]._rect = Common::Rect(45 * kResolutionScaler, 27 + j * 8, (maxLength * 3 + 55) * kResolutionScaler, 34 + j * 8);
			coor[j]._enabled = true;

			while ((int)choiceArray[j].size() < maxLength) {
				choiceArray[j] += ' ';
			}
		}
		coor[lastOption - firstOption + 2]._enabled = false;
		int rep = 6;
		_vm->_screenSurface->drawBox(80, 33, 40 + (maxLength * rep), (lastOption - firstOption) * 8 + 16, 15);
		rep = 0;

		prevChoice = 0;
		warning("Expected answer: %d", correctAnswerArr[indx]);
		do {
			_vm->setMouseClick(false);
			bool flag;
			char key;
			_vm->_mouse->moveMouse(flag, key);
			if (_vm->shouldQuit())
				return false;

			currChoice = 1;
			while (coor[currChoice]._enabled && !_vm->_mouse->isMouseIn(coor[currChoice]._rect))
				++currChoice;
			if (coor[currChoice]._enabled) {
				if ((prevChoice != 0) && (prevChoice != currChoice)) {
					tmpStr = choiceArray[prevChoice] + '$';
					_vm->_text->displayStr(tmpStr, 100, 27 + (prevChoice * 8), 100, 1, 0);
				}
				if (prevChoice != currChoice) {
					tmpStr = choiceArray[currChoice] + '$';
					_vm->_text->displayStr(tmpStr, 100, 27 + (currChoice * 8), 100, 1, 1);
					prevChoice = currChoice;
				}
			} else if (prevChoice != 0) {
				tmpStr = choiceArray[prevChoice] + '$';
				_vm->_text->displayStr(tmpStr, 100, 27 + (prevChoice * 8), 100, 1, 0);
				prevChoice = 0;
			}
		} while (!((prevChoice != 0) && _vm->getMouseClick()));

		if (prevChoice == correctAnswerArr[indx])
			// Answer is correct
			++correctCount;
		else {
			// Skip questions that may give hints on previous wrong answer
			if (indx == 4)
				++indx;
			else if ((indx == 6) || (indx == 7))
				indx = 9;
		}
	}

	return (correctCount == 10);
}

/*------------------------------------------------------------------------*/

/**
 * Draw the F3/F8 dialog
 */
void DialogManager::drawF3F8() {
	Common::String f3 = _vm->getEngineString(S_F3);
	Common::String f8 = _vm->getEngineString(S_F8);

	// Write the F3 and F8 text strings
	_vm->_screenSurface->putxy(3, 44);
	_vm->_screenSurface->drawString(f3, 5);
	_vm->_screenSurface->_textPos.y = 51;
	_vm->_screenSurface->drawString(f8, 5);

	// Get the width of the written text strings
	int f3Width = _vm->_screenSurface->getStringWidth(f3);
	int f8Width = _vm->_screenSurface->getStringWidth(f8);

	// Write out the bounding box
	_vm->_screenSurface->drawBox(0, 42, MAX(f3Width, f8Width) + 4, 16, 7);
}

/**
 * Alert function - Loop until F8 is pressed, update
 * Graphical Device if modified
 * @remarks	Originally called 'diver'
 */
void DialogManager::checkForF8(int SpeechNum, bool drawFrame2Fl) {
	_vm->testKeyboard();
	do {
		_vm->_soundManager->startSpeech(SpeechNum, 0, 0);
		_vm->_key = waitForF3F8();
		if (_vm->shouldQuit())
			return;
	} while (_vm->_key != 66); // keycode for F8
	// just stop the speech when pressing F8
#ifdef USE_TTS
	if (_vm->_soundManager->_ttsMan != nullptr)
		_vm->_soundManager->_ttsMan->stop();
#endif
}

/**
 * Alert function - Loop until F3 or F8 is pressed
 * @remarks	Originally called 'atf3f8'
 */
int DialogManager::waitForF3F8() {
	int key;

	do {
		key = _vm->gettKeyPressed();
		if (_vm->shouldQuit())
			return key;
	} while ((key != 61) && (key != 66));

	return key;
}

/**
 * Intro function - display intro screen
 * @remarks	Originally called 'aff50'
 */
void DialogManager::displayIntroScreen(bool drawFrame2Fl) {
	_vm->_caff = 50;
	_vm->_maff = 0;
	_vm->_text->taffich();
	_vm->draw(63, 12);
	if (drawFrame2Fl)
		displayIntroFrame2();
	else
		_vm->handleDescriptionText(2, kDialogStringIndex + 142);

	// Draw the f3/f8 dialog
	drawF3F8();
}

/**
 * Intro function - display 2nd frame of intro
 * @remarks	Originally called 'ani50'
 */
void DialogManager::displayIntroFrame2() {
	_vm->_crep = _vm->getAnimOffset(1, 1);
	_vm->displayPicture(&_vm->_curAnim[_vm->_crep], 63, 12);
	_vm->_crep = _vm->getAnimOffset(2, 1);
	_vm->displayPicture(&_vm->_curAnim[_vm->_crep], 63, 12);
	_vm->_largestClearScreen = false;
	_vm->handleDescriptionText(2, kDialogStringIndex + 143);
}

} // End of namespace Mortevielle
