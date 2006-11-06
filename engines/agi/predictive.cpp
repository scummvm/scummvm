/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/text.h"

#include "common/func.h"

namespace Agi {

#define kModePre 0
#define kModeNum 1
#define kModeAbc 2

bool TextMan::predictiveDialog(void) {
	int key, active = 0;
	bool rc = false;
	int x;
	int y;
	int bx[17], by[17];
	String prefix = "";
	char temp[MAXWORDLEN + 1];

	const char *buttonStr[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
	const char *buttons[] = {
		"(1)'-.&",  "(2)abc", "(3)def",
		"(4)ghi",  "(5)jkl", "(6)mno",
		"(7)pqrs", "(8)tuv", "(9)wxyz",
		"next",    "add",
		"<",
		"Cancel",  "OK", 
		"Pre", "(0) ", NULL
	};
	const int colors[] = {
		15, 0, 15, 0, 15, 0,
		15, 0, 15, 0, 15, 0,
		15, 0, 15, 0, 15, 0,
		15, 12, 15, 12,
		15, 0,
		15, 0, 15, 0,
		14, 0, 15, 0, 0, 0
	};
	const char *modes[] = { "Pre", "123", "Abc" };

	if (!_dict.size())
		loadDict();

	draw_window(50, 40, 269, 159);
	draw_rectangle(62, 54, 249, 66, MSG_BOX_TEXT);
	flush_block(62, 54, 249, 66);

	print_character(3, 11, game.cursor_char, MSG_BOX_COLOUR, MSG_BOX_TEXT);

	bx[15] = 73; // Zero/space
	by[15] = 120;
	bx[9] = 120; // next
	by[9] = 120;
	bx[10] = 160; // add
	by[10] = 120;
	bx[14] = 200; // Mode
	by[14] = 120;
	bx[11] = 252; // Backspace
	by[11] = 57;
	bx[12] = 180; // Cancel
	by[12] = 140;
	bx[13] = 240; // OK
	by[13] = 140;

	x = 73;
	y = 75;
	for (int i = 0; i < 9; i++) {
		bx[i] = x;
		by[i] = y;
		x += 60;
		if (i % 3 == 2) {
			y += 15;
			x = 73;
		}
	}

	/* clear key queue */
	while (keypress()) {
		get_key();
	}

	_wordPosition = 0;
	_currentCode = "";
	_currentWord = "";
	_matchedWord = "";
	_wordNumber = 0;
	_nextIsActive = _addIsActive = false;

	int mode = kModePre;

	bool needRefresh = true;

	while (42) {
		if (needRefresh) {
			for (int i = 0; buttons[i]; i++) {
				int color1 = colors[i * 2];
				int color2 = colors[i * 2 + 1];

				if (i == 9 && !_nextIsActive) { // Next
					color2 = 7;
				}
				if (i == 10 && !_addIsActive) { // Add
					color2 = 7;
				}
				if (i == 14) {
					draw_button(bx[i], by[i], modes[mode], i == active, 0, color1, color2);
				} else {
					draw_button(bx[i], by[i], buttons[i], i == active, 0, color1, color2);
				}
			}

			if (_currentWord != "") {
				temp[MAXWORDLEN] = 0;

				strncpy(temp, prefix.c_str(), MAXWORDLEN);
				strncat(temp, _currentWord.c_str(), MAXWORDLEN);

				for (int i = prefix.size() + _currentCode.size(); i < MAXWORDLEN; i++)
					temp[i] = ' ';

				print_text(temp, 0, 8, 7, MAXWORDLEN, 15, 0);
				flush_block(62, 54, 249, 66);
			}
		}

		poll_timer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		switch (key) {
		case KEY_ENTER:
			rc = true;
			goto press;
		case KEY_ESCAPE:
			rc = false;
			goto getout;
		case BUTTON_LEFT:
			for (int i = 0; buttons[i]; i++) {
				if (test_button(bx[i], by[i], buttons[i])) {
					needRefresh = true;
					active = i;

					if (active == 15 && mode != kModeNum) { // Space
						strncpy(temp, _currentWord.c_str(), _currentCode.size());

						temp[_currentCode.size()] = 0;

						prefix += temp;
						prefix += " ";
						_wordPosition = 0;
						_currentCode = "";
					} if (active < 9 || active == 11 || active == 15) { // number or backspace
						if (active == 11) { // backspace
							if (_currentCode.size()) {
								_currentCode.deleteLastChar();
								_wordPosition--;
							} else {
								if (prefix.size())
									prefix.deleteLastChar();
							}
						} else if (active == 15) { // zero
							_currentCode += buttonStr[9];
							_wordPosition++;
						} else {
							_currentCode += buttonStr[active];
							_wordPosition++;
						}

						if (mode == kModeNum) {
							_currentWord = _currentCode;
						} else if (mode == kModePre) {
							if (!matchWord() && _currentCode.size()) {
								_currentCode.deleteLastChar();
								_wordPosition--;
								matchWord();
							}
						}
					} else if (active == 9) { // next
						if (_nextIsActive) {
							int wordsNumber = (_matchedWord.size() + 1) / _currentCode.size();
							int start;

							_wordNumber = (_wordNumber + 1) % wordsNumber;

							start = _wordNumber * (_currentCode.size() + 1);

							strncpy(temp, _matchedWord.c_str() + start, _currentCode.size());
							temp[_matchedWord.size() + 1] = 0;

							_currentWord = temp;
						}
					} else if (active == 10) { // add
						debug(0, "add");
					} else if (active == 13) { // Ok
						rc = true;
						goto press;
					} else if (active == 14) { // Mode
						mode++;
						if (mode > kModeAbc)
							mode = kModePre;
					} else {
						goto press;
					}
				}
			}
			break;
		case 0x09:	/* Tab */
			debugC(3, kDebugLevelText, "Focus change");
			active++;
			active %= ARRAYSIZE(buttons) - 1;
			needRefresh = true;
			break;
		}
		do_update();
	}

 press:
	strncpy(_predictiveResult, prefix.c_str(), 40);
	strncat(_predictiveResult, _currentWord.c_str(), 40);
	_predictiveResult[prefix.size() + _currentCode.size() + 1] = 0;

 getout:
	close_window();

	return rc;
}

static char *ltrim(char *t) {
        while (isspace(*t))
                t++;
        return t;
}

static char *rtrim(char *t) {
        int l = strlen(t) - 1;
        while (l >= 0 && isspace(t[l]))
                t[l--] = 0;
        return t;
}

#define MAXLINELEN 80

void TextMan::loadDict(void) {
	Common::File in;
	char buf[MAXLINELEN];

	in.open("pred.txt");

	while (!in.eos()) {
		if (!in.readLine(buf, MAXLINELEN))
			break;

		// Skip leading & trailing whitespaces
		char *t = rtrim(ltrim(buf));
		char *k = t;
		int len = 0;
		char key[30];

		// Skip empty lines
		if (*t == 0)
			continue;

		while (!isspace(*t)) {
			len++;
			t++;
		}

		while (isspace(*t))
			t++;

		strncpy(key, k, len);
		key[len] = 0;

		_dict[String(key)] = String(t);
		_dictKeys.push_back(String(key));
	}

	Common::sort(_dictKeys.begin(), _dictKeys.end());

	debug(0, "Loaded %d keys", _dict.size());
}

bool TextMan::matchWord(void) {
	_addIsActive = false;

	if (!_currentCode.size()) {
		return false;
	}

	if (_dict.contains(_currentCode)) {
		_currentWord = _matchedWord = _dict[_currentCode];

		_nextIsActive = ((_matchedWord.size() + 1) / _currentCode.size() > 1);
		return true;
	}

	// Else search first partial match
	for (uint i = 0; i < _dictKeys.size(); i++) {
		bool matched = true;

		if (_dictKeys[i].size() < _wordPosition)
			continue;

		for (uint j = 0; j < _dictKeys[i].size() && j < _wordPosition; j++) {
			if (_currentCode[j] != _dictKeys[i][j]) {
				matched = false;
				break;
			}
		}
		if (matched && _dictKeys[i].size() >= _wordPosition) {
			_currentWord = _matchedWord = _dict[_dictKeys[i]];

			_nextIsActive = ((_matchedWord.size() + 1) / _currentCode.size() > 1);
			return true;
		}
	}

	_currentWord = _matchedWord = "";
	_nextIsActive = false;
	_addIsActive = true;

	return false;
}



} // End of namespace Agi
