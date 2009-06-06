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
 * $URL$
 * $Id$
 *
 */

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"

#include "common/func.h"
#include "common/config-manager.h"

#ifdef __DS__
#include "wordcompletion.h"
#endif

namespace Agi {

#define kModePre 0
#define kModeNum 1
#define kModeAbc 2

#define MAXLINELEN 80

uint8 countWordsInString(char *str) {
  // Count the number of (space separated) words in the given string.
	char *ptr;

	if (!str)
		return 0;

	ptr = strchr(str, ' ');
	if (!ptr) {
		debug("Invalid dictionary line");
		return 0;
	}

	uint8 num = 1;
	ptr++;
	while ((ptr = strchr(ptr, ' '))) {
		ptr++;
		num++;
	}
	return num;
}

void bringWordtoTop(char *str, int wordnum) {
	// This function reorders the words on the given pred.dic line
	// by moving the word at position 'wordnum' to the front (that is, right behind
	// right behind the numerical code word at the start of the line).
	Common::StringList words;
	char buf[MAXLINELEN];

	if (!str)
		return;
	strncpy(buf, str, MAXLINELEN);
	char *word = strtok(buf, " ");
	if (!word) {
		debug("Invalid dictionary line");
		return;
	}

	words.push_back(word);
	while ((word = strtok(NULL, " ")) != NULL)
		words.push_back(word);
	words.insert_at(1, words.remove_at(wordnum + 1));

	Common::String tmp;
	for (uint8 i = 0; i < words.size(); i++)
			tmp += words[i] + " ";
	tmp.deleteLastChar();
	memcpy(str, tmp.c_str(), strlen(str));
}

bool AgiEngine::predictiveDialog(void) {
	int key = 0, active = -1, lastactive = 0;
	bool rc = false;
	uint8 x;
	int y;
	int bx[17], by[17];
	String prefix;
	char temp[MAXWORDLEN + 1], repeatcount[MAXWORDLEN];
	AgiBlock tmpwindow;
	bool navigationwithkeys = false;
	bool processkey;

	const char *buttonStr[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" };
	const char *buttons[] = {
		"(1)'-.&",  "(2)abc", "(3)def",
		"(4)ghi",  "(5)jkl", "(6)mno",
		"(7)pqrs", "(8)tuv", "(9)wxyz",
		"(#)next",    "add",
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
	const char *modes[] = { "(*)Pre", "(*)123", "(*)Abc" };

	// FIXME: Move this to a more appropriate place.
	if (!_predictiveDictText) {
		loadDict();
		if (!_predictiveDictText)
			return false;
	}
	_predictiveDictActLine = NULL;
	uint8 numMatchingWords = 0;

	_predictiveDialogRunning = true;
	_system->setFeatureState(OSystem::kFeatureDisableKeyFiltering, true);

	memset(repeatcount, 0, MAXWORDLEN);

	// show the predictive dialog.
	// if another window is already in display, save its state into tmpwindow
	memset(&tmpwindow, 0, sizeof(tmpwindow));
	tmpwindow.active = false;
	if (_game.window.active)
		memcpy(&tmpwindow, &(_game.window), sizeof(AgiBlock));
	drawWindow(50, 40, 269, 159);
	_gfx->drawRectangle(62, 54, 249, 66, MSG_BOX_TEXT);
	_gfx->flushBlock(62, 54, 249, 66);

	bx[15] = 73; // Zero/space
	by[15] = 120;
	bx[9] = 110; // next
	by[9] = 120;
	bx[10] = 172; // add
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

	// clear key queue
	while (_gfx->keypress())
		_gfx->getKey();

	prefix.clear();
	_currentCode.clear();
	_currentWord.clear();
	_wordNumber = 0;

	int mode = kModePre;

	bool needRefresh = true;

	for (;;) {
		if (needRefresh) {
			for (int i = 0; buttons[i]; i++) {
				int color1 = colors[i * 2];
				int color2 = colors[i * 2 + 1];

				if (i == 9 && !((mode != kModeAbc && _predictiveDictActLine && numMatchingWords > 1)
							|| (mode == kModeAbc && _currentWord.size() && _currentWord.lastChar() != ' '))) { // Next
					color2 = 7;
				}

				// needs fixing, or remove it!
				bool _addIsActive = false; // FIXME: word adding is not implemented
				if (i == 10 && !_addIsActive) { // Add
					color2 = 7;
				}
				if (i == 14) {
					_gfx->drawDefaultStyleButton(bx[i], by[i], modes[mode], i == active, 0, color1, color2);
				} else {
					_gfx->drawDefaultStyleButton(bx[i], by[i], buttons[i], i == active, 0, color1, color2);
				}
			}

			temp[MAXWORDLEN] = 0;

			strncpy(temp, prefix.c_str(), MAXWORDLEN);
			strncat(temp, _currentWord.c_str(), MAXWORDLEN);

			for (int i = prefix.size() + _currentCode.size(); i < MAXWORDLEN; i++)
				temp[i] = ' ';

			printText(temp, 0, 8, 7, MAXWORDLEN, 15, 0);
			_gfx->flushBlock(62, 54, 249, 66);

			if (active >= 0 && !navigationwithkeys) {
				// provide visual feedback only when not navigating with the arrows
				// so that the user can see the active button.
				active = -1;
				needRefresh = true;
			} else
				needRefresh = false;

			_gfx->doUpdate();
		}

		_gfx->pollTimer();	// msdos driver -> does nothing
		key = doPollKeyboard();
		processkey = false;
		switch (key) {
		case KEY_ENTER:
			if (navigationwithkeys) {
				// when the user has utilized arrow key navigation,
				// interpret enter as 'click' on the active button
				active = lastactive;
			} else {
				// else it is a shortcut for 'Ok'
				active = 13;
			}
			processkey = true;
			break;
		case KEY_ESCAPE:
			rc = false;
			goto getout;
		case BUTTON_LEFT:
			navigationwithkeys = false;
			for (int i = 0; buttons[i]; i++) {
				if (_gfx->testButton(bx[i], by[i], buttons[i])) {
					active = i;
					processkey = true;
					break;
				}
			}
			break;
		case KEY_BACKSPACE:
			active = 11;
			processkey = true;
			break;
		case '#':
			active = 9;
			processkey = true;
			break;
		case '*':
			active = 14;
			processkey = true;
			break;
		case 0x09:	// Tab
			navigationwithkeys = true;
			debugC(3, kDebugLevelText, "Focus change");
			lastactive = active = lastactive + 1;
			active %= ARRAYSIZE(buttons) - 1;
			needRefresh = true;
			break;
		case KEY_LEFT:
			navigationwithkeys = true;
			if (lastactive == 0 || lastactive == 3 || lastactive == 6)
				active = lastactive + 2;
			else if (lastactive == 9)
				active = 15;
			else if (lastactive == 11)
				active = 11;
			else if (lastactive == 12)
				active = 13;
			else if (lastactive == 14)
				active = 10;
			else
				active = lastactive - 1;
			lastactive = active;
			needRefresh = true;
			break;
		case KEY_RIGHT:
			navigationwithkeys = true;
			if (lastactive == 2 || lastactive == 5 || lastactive == 8)
				active = lastactive - 2;
			else if (lastactive == 10)
				active = 14;
			else if (lastactive == 11)
				active = 11;
			else if (lastactive == 13)
				active = 12;
			else if (lastactive == 15)
				active = 9;
			else
				active = lastactive + 1;
			lastactive = active;
			needRefresh = true;
			break;
		case KEY_UP:
			navigationwithkeys = true;
			if (lastactive <= 2)
				active = 11;
			else if (lastactive == 9 || lastactive == 10)
				active = lastactive - 2;
			else if (lastactive == 11)
				active = 13;
			else if (lastactive == 14)
				active = 8;
			else if (lastactive == 15)
				active = 6;
			else
				active = lastactive - 3;
			lastactive = active;
			needRefresh = true;
			break;
		case KEY_DOWN:
			navigationwithkeys = true;
			if (lastactive == 6)
				active = 15;
			else if (lastactive == 7 || lastactive == 8)
				active = lastactive + 2;
			else if (lastactive == 11)
				active = 0;
			else if (lastactive == 12 || lastactive == 13)
				active = 11;
			else if (lastactive == 14 || lastactive == 15)
				active = lastactive - 2;
			else
				active = lastactive + 3;
			lastactive = active;
			needRefresh = true;
			break;
		default:
			// handle numeric buttons
			if (key >= '1' && key <= '9') {
				active = key - '1';
				processkey = true;
			} else if (key == '0') {
				active = 15;
				processkey = true;
			}
			break;
		}

		if (processkey) {
			if (active >= 0) {
				needRefresh = true;
				lastactive = active;
				if (active == 15 && mode != kModeNum) { // Space
					// bring MRU word at the top of the list when changing words
					if (mode == kModePre && _predictiveDictActLine && numMatchingWords > 1 && _wordNumber != 0)
						bringWordtoTop(_predictiveDictActLine, _wordNumber);
					strncpy(temp, _currentWord.c_str(), _currentCode.size());
					temp[_currentCode.size()] = 0;
					prefix += temp;
					prefix += " ";
					_currentCode.clear();
					_currentWord.clear();
					numMatchingWords = 0;
					memset(repeatcount, 0, MAXWORDLEN);
				} else if (active < 9 || active == 11 || active == 15) { // number or backspace
					if (active == 11) { // backspace
						if (_currentCode.size()) {
							repeatcount[_currentCode.size() - 1] = 0;
							_currentCode.deleteLastChar();
						} else {
							if (prefix.size())
								prefix.deleteLastChar();
						}
					} else if (prefix.size() + _currentCode.size() < MAXWORDLEN - 1) { // don't overflow the dialog line
						if (active == 15) { // zero
							_currentCode += buttonStr[9];
						} else {
							_currentCode += buttonStr[active];
						}
					}

					switch (mode) {
					case kModeNum:
						_currentWord = _currentCode;
						break;
					case kModePre:
						if (!matchWord() && _currentCode.size()) {
							_currentCode.deleteLastChar();
							matchWord();
						}
						numMatchingWords = countWordsInString(_predictiveDictActLine);
						break;
					case kModeAbc:
						for (x = 0; x < _currentCode.size(); x++)
							if (_currentCode[x] >= '1')
								temp[x] = buttons[_currentCode[x] - '1'][3 + repeatcount[x]];
						temp[_currentCode.size()] = 0;
						_currentWord = temp;
					}
				} else if (active == 9) { // next
					if (mode == kModePre) {
						if (_predictiveDictActLine && numMatchingWords > 1) {
							_wordNumber = (_wordNumber + 1) % numMatchingWords;
							char tmp[MAXLINELEN];
							strncpy(tmp, _predictiveDictActLine, MAXLINELEN);
							char *tok = strtok(tmp, " ");
							for (uint8 i = 0; i <= _wordNumber; i++)
								tok = strtok(NULL, " ");
							_currentWord = String(tok, _currentCode.size());
						}
					} else if (mode == kModeAbc){
						x = _currentCode.size();
						if (x) {
							if (_currentCode.lastChar() == '1' || _currentCode.lastChar() == '7' || _currentCode.lastChar() == '9')
								repeatcount[x - 1] = (repeatcount[x - 1] + 1) % 4;
							else
								repeatcount[x - 1] = (repeatcount[x - 1] + 1) % 3;
							if (_currentCode.lastChar() >= '1')
								_currentWord.setChar(buttons[_currentCode[x - 1] - '1'][3 + repeatcount[x - 1]], x-1);
						}
					}
				} else if (active == 10) { // add
					debug(0, "add");
				} else if (active == 13) { // Ok
					// bring MRU word at the top of the list when ok'ed out of the dialog
					if (mode == kModePre && _predictiveDictActLine && numMatchingWords > 1 && _wordNumber != 0)
						bringWordtoTop(_predictiveDictActLine, _wordNumber);
					rc = true;
					goto press;
				} else if (active == 14) { // Mode
					mode++;
					if (mode > kModeAbc)
						mode = kModePre;

					// truncate current input at mode change
					strncpy(temp, _currentWord.c_str(), _currentCode.size());
					temp[_currentCode.size()] = 0;
					prefix += temp;
					_currentCode.clear();
					_currentWord.clear();
					memset(repeatcount, 0, MAXWORDLEN);
				} else {
					goto press;
				}
			}
		}
	}

 press:
	strncpy(_predictiveResult, prefix.c_str(), 40);
	strncat(_predictiveResult, _currentWord.c_str(), 40);
	_predictiveResult[prefix.size() + _currentCode.size() + 1] = 0;

 getout:
	// if another window was shown, bring it up again
	if (!tmpwindow.active)
		closeWindow();
	else {
		_gfx->restoreBlock(_game.window.x1, _game.window.y1,
				_game.window.x2, _game.window.y2, _game.window.buffer);

		free(_game.window.buffer);
		memcpy(&(_game.window), &tmpwindow, sizeof(AgiBlock));
		_gfx->doUpdate();
	}

	_system->setFeatureState(OSystem::kFeatureDisableKeyFiltering, false);
	_predictiveDialogRunning = false;

	return rc;
}

void AgiEngine::loadDict(void) {
	Common::File inFile;
	int lines = 0;

	ConfMan.registerDefault("predictive_dictionary", "pred.dic");

	uint32 time1 = _system->getMillis();
	Common::String inFileName(ConfMan.get("predictive_dictionary"));
	if (!inFile.open(inFileName))
		return;

	char *ptr;
	int size = inFile.size();

	_predictiveDictText = (char *)malloc(size + 1);
	if (!_predictiveDictText) {
		warning("Not enough memory to load the predictive dictionary");
		return;
	}
	inFile.read(_predictiveDictText, size);
	_predictiveDictText[size] = 0;
	uint32 time2 = _system->getMillis();
	debug("Time to read %s: %d bytes, %d ms", inFileName.c_str(), size, time2-time1);
	inFile.close();

	ptr = _predictiveDictText;
	lines = 1;
	while ((ptr = strchr(ptr, '\n'))) {
		lines++;
		ptr++;
	}

	_predictiveDictLine = (char **)calloc(1, sizeof(char *) * lines);
	if (_predictiveDictLine == NULL) {
		warning("Cannot allocate memory for line index buffer.");
		return;
	}
	_predictiveDictLine[0] = _predictiveDictText;
	ptr = _predictiveDictText;
	int i = 1;
	while ((ptr = strchr(ptr, '\n'))) {
		*ptr = 0;
		ptr++;
#ifdef __DS__
		// Pass the line on to the DS word list
		DS::addAutoCompleteLine(_predictiveDictLine[i - 1]);
#endif
		_predictiveDictLine[i++] = ptr;
	}
	if (_predictiveDictLine[lines - 1][0] == 0)
		lines--;

	_predictiveDictLineCount = lines;
	debug("Loaded %d lines", _predictiveDictLineCount);

	// FIXME: We use binary search on _predictiveDictLine, yet we make no attempt
	// to ever sort this array (except for the DS port). That seems risky, doesn't it?

#ifdef __DS__
	// Sort the DS word completion list, to allow for a binary chop later (in the ds backend)
	DS::sortAutoCompleteWordList();
#endif

	uint32 time3 = _system->getMillis();
	printf("Time to parse pred.dic: %d, total: %d\n", time3-time2, time3-time1);
}

bool AgiEngine::matchWord(void) {
	if (_currentCode.empty()) {
		return false;
	}
	// Lookup word in the dictionary
	int line = 0, cmpRes = 0, len = 0;
	char target[MAXWORDLEN];

	strncpy(target, _currentCode.c_str(), MAXWORDLEN);
	strcat(target, " ");

	// do the search at most two times:
	// first try to match the exact code, by matching also the space after the code
	// if there is not an exact match, do it once more for the best matching prefix (drop the space)
	len = _currentCode.size() + 1;
	for (int i = 0; i < 2; ++i) {
		// Perform a binary search.
		int hi = _predictiveDictLineCount - 1;
		int lo = 0;
		while (lo <= hi) {
			line = (lo + hi) / 2;
			cmpRes = strncmp(_predictiveDictLine[line], target, len);
			if (cmpRes > 0)
				hi = line - 1;
			else if (cmpRes < 0)
				lo = line + 1;
			else
				break;
		}

		if (cmpRes == 0)  // Exact match found? -> stop now
			break;
		len--;  // Remove the trailing space
	}

	_currentWord.clear();
	_wordNumber = 0;
	if (!strncmp(_predictiveDictLine[line], target, len)) {
		_predictiveDictActLine = _predictiveDictLine[line];
		char tmp[MAXLINELEN];
		strncpy(tmp, _predictiveDictActLine, MAXLINELEN);
		char *tok = strtok(tmp, " ");
		tok = strtok(NULL, " ");
		_currentWord = String(tok, _currentCode.size());
		return true;
	} else {
		_predictiveDictActLine = NULL;
		return false;
	}
}

} // End of namespace Agi
