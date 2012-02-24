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
#include "mortevielle/level15.h"
#include "mortevielle/mortevielle.h"
#include "mortevielle/mor.h"
#include "mortevielle/mouse.h"
#include "mortevielle/outtext.h"
#include "mortevielle/parole2.h"
#include "mortevielle/taffich.h"

namespace Mortevielle {

static const int nligne = 7;

/**
 * Alert function - Show
 * @remarks	Originally called 'do_alert'
 */
int Alert::show(const Common::String &msg, int n) {
	int coldep, esp, i, caseNumb, quoi, ix;
	Common::String st, chaine;
	int limit[3][3];
	char dumi;
	Common::String s[3];
	int cx, cy, nbcol, lignNumb;
	bool newaff, test, test1, test2, test3, dum;
	Common::String cas;

	// Make a copy of the current screen surface for later restore
	g_vm->_backgroundSurface.copyFrom(g_vm->_screenSurface);

	/*debug('** do_alert **');*/
	memset(&limit[0][0], 0, sizeof(int) * 3 * 3);
	int do_alert_result;
	hideMouse();
	while (keypressed())
		dumi = get_ch();	// input >> kbd >> dumi;

	g_vm->setMouseClick(false);
	decodeAlertDetails(msg, caseNumb, lignNumb, nbcol, chaine, cas);
	sauvecr(50, (nligne + 1) << 4);

	i = 0;
	if (chaine == "") {
		drawAlertBox(10, 5, nbcol);
	} else {
		drawAlertBox(8, 7, nbcol);
		i = 0;
		g_vm->_screenSurface._textPos.y = 70;
		do {
			cx = 320;
			st = "";
			while ((chaine[i + 1] != '\174') && (chaine[i + 1] != '\135')) {
				++i;
				st = st + chaine[i];
				if (res == 2)
					cx -= 3;
				else
					cx -= 5;
			}
			g_vm->_screenSurface.putxy(cx, g_vm->_screenSurface._textPos.y);
			g_vm->_screenSurface._textPos.y += 6;
			g_vm->_screenSurface.writeg(st, 4);
			++i;
		} while (!(chaine[i] == ']'));
	}
	if (caseNumb == 1)
		esp = nbcol - 40;
	else
		esp = (uint)(nbcol - caseNumb * 40) >> 1;
	coldep = 320 - ((uint)nbcol >> 1) + ((uint)esp >> 1);
	setButtonText(cas, coldep, caseNumb, &s[0], esp);
	limit[1][1] = ((uint)(coldep) >> 1) * res;
	limit[1][2] = limit[1][1] + 40;
	if (caseNumb == 1) {
		limit[2][1] = limit[2][2];
	} else {
		limit[2][1] = ((uint)(320 + ((uint)esp >> 1)) >> 1) * res;
		limit[2][2] = (limit[2][1]) + 40;
	}
	showMouse();
	quoi = 0;
	dum = false;
	do {
		dumi = '\377';
		moveMouse(dum, dumi);
		CHECK_QUIT0;

		cx = x_s;
		cy = y_s;
		test = (cy > 95) && (cy < 105);
		newaff = false;
		if (test) {
			test1 = (cx > limit[1][1]) && (cx < limit[1][2]);
			test2 = test1;
			if (caseNumb > 1)
				test2 = test1 || ((cx > limit[2][1]) && (cx < limit[2][2]));
			if (test2) {
				newaff = true;
				if (test1)
					ix = 1;
				else
					ix = 2;
				if (ix != quoi) {
					hideMouse();
					if (quoi != 0) {
						setPosition(quoi, coldep, esp);

						Common::String tmp(" ");
						tmp += s[quoi];
						tmp += " ";
						g_vm->_screenSurface.writeg(tmp, 0);
					}
					setPosition(ix, coldep, esp);

					Common::String tmp2 = " ";
					tmp2 += s[ix];
					tmp2 += " ";
					g_vm->_screenSurface.writeg(tmp2, 1);

					quoi = ix;
					showMouse();
				}
			}
		}
		if ((quoi != 0) && ! newaff) {
			hideMouse();
			setPosition(quoi, coldep, esp);

			Common::String tmp3(" ");
			tmp3 += s[quoi];
			tmp3 += " ";
			g_vm->_screenSurface.writeg(tmp3, 0);

			quoi = 0;
			showMouse();
		}
		test3 = (cy > 95) && (cy < 105) && (((cx > limit[1][1]) && (cx < limit[1][2]))
		                                    || ((cx > limit[2][1]) && (cx < limit[2][2])));
	} while (!g_vm->getMouseClick());
	g_vm->setMouseClick(false);
	hideMouse();
	if (!test3)  {
		quoi = n;
		setPosition(n, coldep, esp);
		Common::String tmp4(" ");
		tmp4 += s[n];
		tmp4 += " ";
		g_vm->_screenSurface.writeg(tmp4, 1);
	}
	charecr(50, (nligne + 1) << 4);
	showMouse();

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
	choiceListStr = copy(inputStr, i, 30);
	if (res == 2)
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
		} while (!(c[i + 1] == ']'));
		i += 2;

		while (str[l].size() < 3)
			str[l] += ' ';

		g_vm->_screenSurface.putxy(x, 98);

		Common::String tmp(" ");
		tmp += str[l];
		tmp += " ";

		g_vm->_screenSurface.writeg(tmp, 0);
		x += esp + 40;
	}
}

/*------------------------------------------------------------------------*/

/**
 * Questions asked before entering the hidden passage
 */
bool Ques::show() {
	const int textIndexArr[11] = {0, 511, 516, 524, 531, 545, 552, 559, 563, 570, 576};
	const int correctAnswerArr[11] = {0, 4, 7, 1, 6, 4, 4, 2, 5, 3, 1 };

	int optionPosY;
	int curLength, maxLength;
	int rep;
	int firstOption, lastOption;
	char key;
	rectangle coor[max_rect];
	Common::String choiceArray[15];
	char st[1410];

	int currChoice, prevChoice;
	int indx = 0;
	int correctCount = 0;
	bool protectionCheck = false;

	do {
		hideMouse();
		hirs();
		showMouse();
		++indx;
		deline(textIndexArr[indx], st, curLength);
		int dialogHeight;
		if (res == 1)
			dialogHeight = 29;
		else
			dialogHeight = 23;
		g_vm->_screenSurface.fillRect(15, Common::Rect(0, 14, 630, dialogHeight));
		afftex(st, 20, 15, 100, 2, 0);
		if (indx != 10) {
			firstOption = textIndexArr[indx] + 1;
			lastOption = textIndexArr[indx + 1] - 1;
		} else {
			firstOption = 503;
			lastOption = 510;
		}
		optionPosY = 35;
		maxLength = 0;

		for (int j = firstOption, prevChoice = 1; j <= lastOption; ++j, ++prevChoice) {
			deline(j, st, curLength);
			if (curLength > maxLength)
				maxLength = curLength;
			afftex(st, 100, optionPosY, 100, 1, 0);
			choiceArray[prevChoice] = delig;
			optionPosY += 8;
		}
		for (int j = 1; j <= lastOption - firstOption + 1; ++j) {
			rectangle &with = coor[j];

			with.x1 = 45 * res;
			with.x2 = (maxLength * 3 + 55) * res;
			with.y1 = 27 + j * 8;
			with.y2 = 34 + j * 8;
			with.enabled = true;

			while ((int)choiceArray[j].size() < maxLength) {
				choiceArray[j] += ' ';
			}
		}
		coor[lastOption - firstOption + 2].enabled = false;
		if (res == 1)
			rep = 10;
		else
			rep = 6;
		g_vm->_screenSurface.drawBox(80, 33, 40 + (maxLength * rep), (lastOption - firstOption) * 8 + 16, 15);
		rep = 0;

		prevChoice = 0;
		do {
			g_vm->setMouseClick(false);
			tesok = false;
			bool flag;
			moveMouse(flag, key);
			CHECK_QUIT0;

			currChoice = 1;
			while (coor[currChoice].enabled && !dans_rect(coor[currChoice]))
				++currChoice;
			if (coor[currChoice].enabled) {
				if ((prevChoice != 0) && (prevChoice != currChoice)) {
					st[0] = ' ';
//					for (j = 0; j <= maxLength; ++j)
//						st[j + 1] = choiceArray[prevChoice][j];
					strncpy(st + 1, choiceArray[prevChoice].c_str(), maxLength);
					st[1 + maxLength] = '$';
					afftex(st, 100, 27 + (prevChoice * 8), 100, 1, 0);
				}
				if (prevChoice != currChoice) {
					st[0] = ' ';
//					for (j = 0; j <= maxLength; ++j)
//						st[j + 1] = choiceArray[currChoice][j];
					strncpy(st + 1, choiceArray[currChoice].c_str(), maxLength);
					st[1 + maxLength] = '$';
					afftex(st, 100, 27 + (currChoice * 8), 100, 1, 1);
					prevChoice = currChoice;
				}
			} else if (prevChoice != 0) {
					st[0] = ' ';
//				for (j = 0; j <= maxLength; ++j)
//					st[j + 1] = choiceArray[prevChoice][j];
				strncpy(st + 1, choiceArray[prevChoice].c_str(), maxLength);
				st[1 + maxLength] = '$';
				afftex(st, 100, 27 + (prevChoice * 8), 100, 1, 0);
				prevChoice = 0;
			}
		} while (!((prevChoice != 0) && g_vm->getMouseClick()));

		warning("Expected answer: %d, answer: %d", prevChoice, correctAnswerArr[indx]);
		if (prevChoice == correctAnswerArr[indx])
			// Answer is correct
			++correctCount;
		else {
			// Skip questions that may give hints on previous wrong answer
			if (indx == 5)
				++indx;

			if ((indx == 7) || (indx == 8))
				indx = 10;
		}
		if (indx == 10) {
			warning("Skipping protection check: testprot()");
			protectionCheck = true;
			tesok = true;
		}
	} while (!(indx == 10));

	return (correctCount == 10) && protectionCheck;
}

/*------------------------------------------------------------------------*/

/**
 * Draw the F3/F8 dialog
 */
void f3f8::draw() {
	Common::String f3 = g_vm->getString(S_F3);
	Common::String f8 = g_vm->getString(S_F8);

	// Write the F3 and F8 text strings
	g_vm->_screenSurface.putxy(3, 44);
	g_vm->_screenSurface.writeg(f3, 5);
	g_vm->_screenSurface._textPos.y = 51;
	g_vm->_screenSurface.writeg(f8, 5);

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
		parole(SpeechNum, 0, 0);
		waitForF3F8(key);
		CHECK_QUIT;

		if (_newGraphicalDevice != _currGraphicalDevice) {
			_currGraphicalDevice = _newGraphicalDevice;
			hirs();
			aff50(drawAni50Fl);
		}
	} while (!(key == 66)); // keycode for F8
}

/**
 * Alert function - Loop until F3 or F8 is pressed
 * @remarks	Originally called 'atf3f8'
 */
void f3f8::waitForF3F8(int &key) {
	do {
		key = testou();
		CHECK_QUIT;
	} while (!((key == 61) || (key == 66)));
}

void f3f8::aff50(bool drawAni50Fl) {
	caff = 50;
	_maff = 0;
	taffich();
	dessine(ades, 63, 12);
	if (drawAni50Fl)
		ani50();
	else
		repon(2, c_paroles + 142);
	
	// Draw the f3/f8 dialog
	draw();
}

void f3f8::ani50() {
	crep = animof(1, 1);
	pictout(adani, crep, 63, 12);
	crep = animof(2, 1);
	pictout(adani, crep, 63, 12);
	f2_all = (res == 1);
	repon(2, c_paroles + 143);
}


} // End of namespace Mortevielle
