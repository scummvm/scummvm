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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#include "common/str.h"
#include "mortevielle/dialogs.h"
#include "mortevielle/keyboard.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/speech.h"

namespace Mortevielle {

/**
 * Alert function - Show
 * @remarks	Originally called 'do_alert'
 */
int Alert::show(const Common::String &msg, int n) {
	int coldep, esp, i, caseNumb, quoi, ix;
	Common::String st, chaine;
	int limit[3][3];
	Common::String s[3];
	Common::Point curPos;
	int nbcol, lignNumb;
	bool newaff, test, test1, test2, test3, dum;
	Common::String cas;

	// Make a copy of the current screen surface for later restore
	g_vm->_backgroundSurface.copyFrom(g_vm->_screenSurface);

	memset(&limit[0][0], 0, sizeof(int) * 3 * 3);
	int do_alert_result;
	g_vm->_mouse.hideMouse();
	while (g_vm->keyPressed())
		g_vm->getChar();

	g_vm->setMouseClick(false);
	decodeAlertDetails(msg, caseNumb, lignNumb, nbcol, chaine, cas);
	g_vm->sauvecr(50, (NUM_LINES + 1) << 4);

	i = 0;
	if (chaine == "") {
		drawAlertBox(10, 5, nbcol);
	} else {
		drawAlertBox(8, 7, nbcol);
		i = 0;
		g_vm->_screenSurface._textPos.y = 70;
		do {
			curPos.x = 320;
			st = "";
			while ((chaine[i + 1] != '\174') && (chaine[i + 1] != '\135')) {
				++i;
				st = st + chaine[i];
				if (g_vm->_res == 2)
					curPos.x -= 3;
				else
					curPos.x -= 5;
			}
			g_vm->_screenSurface.putxy(curPos.x, g_vm->_screenSurface._textPos.y);
			g_vm->_screenSurface._textPos.y += 6;
			g_vm->_screenSurface.drawString(st, 4);
			++i;
		} while (chaine[i] != ']');
	}
	if (caseNumb == 1)
		esp = nbcol - 40;
	else
		esp = (uint)(nbcol - caseNumb * 40) >> 1;
	coldep = 320 - ((uint)nbcol >> 1) + ((uint)esp >> 1);
	setButtonText(cas, coldep, caseNumb, &s[0], esp);
	limit[1][1] = ((uint)(coldep) >> 1) * g_vm->_res;
	limit[1][2] = limit[1][1] + 40;
	if (caseNumb == 1) {
		limit[2][1] = limit[2][2];
	} else {
		limit[2][1] = ((uint)(320 + ((uint)esp >> 1)) >> 1) * g_vm->_res;
		limit[2][2] = (limit[2][1]) + 40;
	}
	g_vm->_mouse.showMouse();
	quoi = 0;
	dum = false;
	do {
		char dummyKey = '\377';
		g_vm->_mouse.moveMouse(dum, dummyKey);
		CHECK_QUIT0;

		curPos = g_vm->_mouse._pos;
		test = (curPos.y > 95) && (curPos.y < 105);
		newaff = false;
		if (test) {
			test1 = (curPos.x > limit[1][1]) && (curPos.x < limit[1][2]);
			test2 = test1;
			if (caseNumb > 1)
				test2 = test1 || ((curPos.x > limit[2][1]) && (curPos.x < limit[2][2]));
			if (test2) {
				newaff = true;
				if (test1)
					ix = 1;
				else
					ix = 2;
				if (ix != quoi) {
					g_vm->_mouse.hideMouse();
					if (quoi != 0) {
						setPosition(quoi, coldep, esp);

						Common::String tmp(" ");
						tmp += s[quoi];
						tmp += " ";
						g_vm->_screenSurface.drawString(tmp, 0);
					}
					setPosition(ix, coldep, esp);

					Common::String tmp2 = " ";
					tmp2 += s[ix];
					tmp2 += " ";
					g_vm->_screenSurface.drawString(tmp2, 1);

					quoi = ix;
					g_vm->_mouse.showMouse();
				}
			}
		}
		if ((quoi != 0) && ! newaff) {
			g_vm->_mouse.hideMouse();
			setPosition(quoi, coldep, esp);

			Common::String tmp3(" ");
			tmp3 += s[quoi];
			tmp3 += " ";
			g_vm->_screenSurface.drawString(tmp3, 0);

			quoi = 0;
			g_vm->_mouse.showMouse();
		}
		test3 = (curPos.y > 95) && (curPos.y < 105) && (((curPos.x > limit[1][1]) && (curPos.x < limit[1][2]))
		                                    || ((curPos.x > limit[2][1]) && (curPos.x < limit[2][2])));
	} while (!g_vm->getMouseClick());
	g_vm->setMouseClick(false);
	g_vm->_mouse.hideMouse();
	if (!test3)  {
		quoi = n;
		setPosition(n, coldep, esp);
		Common::String tmp4(" ");
		tmp4 += s[n];
		tmp4 += " ";
		g_vm->_screenSurface.drawString(tmp4, 1);
	}
	g_vm->charecr(50, (NUM_LINES + 1) << 4);
	g_vm->_mouse.showMouse();

	/* Restore the background area */
	g_vm->_screenSurface.copyFrom(g_vm->_backgroundSurface, 0, 0);

	do_alert_result = quoi;
	return do_alert_result;
}

/**
 * Alert function - Decode Alert Details
 * @remarks	Originally called 'decod'
 */
void Alert::decodeAlertDetails(Common::String inputStr, int &choiceNumb, int &lineNumb, int &col, Common::String &choiceStr, Common::String &choiceListStr) {
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
	++i;
	choiceListStr = g_vm->copy(inputStr, i, 30);
	if (g_vm->_res == 2)
		col *= 6;
	else
		col *= 10;
}

void Alert::setPosition(int ji, int coldep, int esp) {
	g_vm->_screenSurface.putxy(coldep + (40 + esp) * (ji - 1), 98);
}

/**
 * Alert function - Draw Alert Box
 * @remarks	Originally called 'fait_boite'
 */
void Alert::drawAlertBox(int lidep, int nli, int tx) {
	if (tx > 640)
		tx = 640;
	int x = 320 - ((uint)tx >> 1);
	int y = (lidep - 1) << 3;
	int xx = x + tx;
	int yy = y + (nli << 3);
	g_vm->_screenSurface.fillRect(15, Common::Rect(x, y, xx, yy));
	g_vm->_screenSurface.fillRect(0, Common::Rect(x, y + 2, xx, y + 4));
	g_vm->_screenSurface.fillRect(0, Common::Rect(x, yy - 4, xx, yy - 2));
}

/**
 * Alert function - Set Button Text
 * @remarks	Originally called 'fait_choix'
 */
void Alert::setButtonText(Common::String c, int coldep, int nbcase, Common::String *str, int esp) {
	int i, l, x;
	char ch;

	i = 1;
	x = coldep;
	for (l = 1; l <= nbcase; ++l) {
		str[l] = "";
		do {
			++i;
			ch = c[i];
			str[l] += ch;
		} while (c[i + 1] != ']');
		i += 2;

		while (str[l].size() < 3)
			str[l] += ' ';

		g_vm->_screenSurface.putxy(x, 98);

		Common::String tmp(" ");
		tmp += str[l];
		tmp += " ";

		g_vm->_screenSurface.drawString(tmp, 0);
		x += esp + 40;
	}
}

/*------------------------------------------------------------------------*/

/**
 * Questions asked before entering the hidden passage
 */
bool KnowledgeCheck::show() {
	const int textIndexArr[10] = {511, 516, 524, 531, 545, 552, 559, 563, 570, 576};
	const int correctAnswerArr[10] = {4, 7, 1, 6, 4, 4, 2, 5, 3, 1 };

	int optionPosY;
	int maxLength;
	int rep;
	int firstOption, lastOption;
	char key;
	Rect coor[kMaxRect];
	Common::String choiceArray[15];

	int currChoice, prevChoice;
	int correctCount = 0;

	for (int indx = 0; indx < 10; ++indx) {
		g_vm->_mouse.hideMouse();
		g_vm->hirs();
		g_vm->_mouse.showMouse();
		int dialogHeight;
		if (g_vm->_res == 1)
			dialogHeight = 29;
		else
			dialogHeight = 23;
		g_vm->_screenSurface.fillRect(15, Common::Rect(0, 14, 630, dialogHeight));
		Common::String tmpStr = g_vm->getString(textIndexArr[indx]);
		displayStr(tmpStr, 20, 15, 100, 2, 0);

		if (indx != 9) {
			firstOption = textIndexArr[indx] + 1;
			lastOption = textIndexArr[indx + 1] - 1;
		} else {
			firstOption = 503;
			lastOption = 510;
		}
		optionPosY = 35;
		maxLength = 0;

		prevChoice = 1;
		for (int j = firstOption; j <= lastOption; ++j, ++prevChoice) {
			tmpStr = g_vm->getString(j);
			if ((int) tmpStr.size() > maxLength)
				maxLength = tmpStr.size();
			displayStr(tmpStr, 100, optionPosY, 100, 1, 0);
			choiceArray[prevChoice] = tmpStr;
			optionPosY += 8;
		}

		for (int j = 1; j <= lastOption - firstOption + 1; ++j) {
			coor[j]._x1 = 45 * g_vm->_res;
			coor[j]._x2 = (maxLength * 3 + 55) * g_vm->_res;
			coor[j]._y1 = 27 + j * 8;
			coor[j]._y2 = 34 + j * 8;
			coor[j]._enabled = true;

			while ((int)choiceArray[j].size() < maxLength) {
				choiceArray[j] += ' ';
			}
		}
		coor[lastOption - firstOption + 2]._enabled = false;
		if (g_vm->_res == 1)
			rep = 10;
		else
			rep = 6;
		g_vm->_screenSurface.drawBox(80, 33, 40 + (maxLength * rep), (lastOption - firstOption) * 8 + 16, 15);
		rep = 0;

		prevChoice = 0;
		warning("Expected answer: %d", correctAnswerArr[indx]);
		do {
			g_vm->setMouseClick(false);
			bool flag;
			g_vm->_mouse.moveMouse(flag, key);
			CHECK_QUIT0;

			currChoice = 1;
			while (coor[currChoice]._enabled && !g_vm->_mouse.isMouseIn(coor[currChoice]))
				++currChoice;
			if (coor[currChoice]._enabled) {
				if ((prevChoice != 0) && (prevChoice != currChoice)) {
					tmpStr = choiceArray[prevChoice] + '$';
					displayStr(tmpStr, 100, 27 + (prevChoice * 8), 100, 1, 0);
				}
				if (prevChoice != currChoice) {
					tmpStr = choiceArray[currChoice] + '$';
					displayStr(tmpStr, 100, 27 + (currChoice * 8), 100, 1, 1);
					prevChoice = currChoice;
				}
			} else if (prevChoice != 0) {
				tmpStr = choiceArray[prevChoice] + '$';
				displayStr(tmpStr, 100, 27 + (prevChoice * 8), 100, 1, 0);
				prevChoice = 0;
			}
		} while (!((prevChoice != 0) && g_vm->getMouseClick()));

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
void f3f8::draw() {
	Common::String f3 = g_vm->getEngineString(S_F3);
	Common::String f8 = g_vm->getEngineString(S_F8);

	// Write the F3 and F8 text strings
	g_vm->_screenSurface.putxy(3, 44);
	g_vm->_screenSurface.drawString(f3, 5);
	g_vm->_screenSurface._textPos.y = 51;
	g_vm->_screenSurface.drawString(f8, 5);

	// Get the width of the written text strings
	int f3Width = g_vm->_screenSurface.getStringWidth(f3);
	int f8Width = g_vm->_screenSurface.getStringWidth(f8);

	// Write out the bounding box
	g_vm->_screenSurface.drawBox(0, 42, MAX(f3Width, f8Width) + 6, 16, 7);
}

/**
 * Alert function - Loop until F8 is pressed, update 
 * Graphical Device if modified
 * @remarks	Originally called 'diver'
 */
void f3f8::checkForF8(int SpeechNum, bool drawAni50Fl) {
	teskbd();
	do {
		g_vm->_speechManager.startSpeech(SpeechNum, 0, 0);
		g_vm->_key = waitForF3F8();
		CHECK_QUIT;

		if (g_vm->_newGraphicalDevice != g_vm->_currGraphicalDevice) {
			g_vm->_currGraphicalDevice = g_vm->_newGraphicalDevice;
			g_vm->hirs();
			aff50(drawAni50Fl);
		}
	} while (g_vm->_key != 66); // keycode for F8
}

/**
 * Alert function - Loop until F3 or F8 is pressed
 * @remarks	Originally called 'atf3f8'
 */
int f3f8::waitForF3F8() {
	int key;

	do {
		key = testou();
		if (g_vm->shouldQuit())
			return key;
	} while ((key != 61) && (key != 66));

	return key;
}

void f3f8::aff50(bool drawAni50Fl) {
	g_vm->_caff = 50;
	g_vm->_maff = 0;
	taffich();
	g_vm->draw(kAdrDes, 63, 12);
	if (drawAni50Fl)
		ani50();
	else
		g_vm->repon(2, kDialogStringIndex + 142);
	
	// Draw the f3/f8 dialog
	draw();
}

void f3f8::ani50() {
	g_vm->_crep = g_vm->animof(1, 1);
	g_vm->pictout(kAdrAni, g_vm->_crep, 63, 12);
	g_vm->_crep = g_vm->animof(2, 1);
	g_vm->pictout(kAdrAni, g_vm->_crep, 63, 12);
	g_vm->_largestClearScreen = (g_vm->_res == 1);
	g_vm->repon(2, kDialogStringIndex + 143);
}

} // End of namespace Mortevielle
