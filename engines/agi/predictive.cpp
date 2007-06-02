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

namespace Agi {

#define kModePre 0
#define kModeNum 1
#define kModeAbc 2

static byte s_asciiToNumTable[256];

void setAsciiToNumTableBatch(const char *chars, byte value) {
	while (*chars) {
		s_asciiToNumTable[tolower(*chars)] = value;
		s_asciiToNumTable[toupper(*chars)] = value;
		chars++;
	}
}

void initAsciiToNumTable() {
	memset(s_asciiToNumTable, 0, sizeof(s_asciiToNumTable));

	setAsciiToNumTableBatch("1'-.&", 1);
	setAsciiToNumTableBatch("2abc", 2);
	setAsciiToNumTableBatch("3def", 3);
	setAsciiToNumTableBatch("4ghi", 4);
	setAsciiToNumTableBatch("5jkl", 5);
	setAsciiToNumTableBatch("6mno", 6);
	setAsciiToNumTableBatch("7pqrs", 7);
	setAsciiToNumTableBatch("8tuv", 8);
	setAsciiToNumTableBatch("9wxyz", 9);
}

class SearchTree {
public:
	//byte val;
	//SearchTree *parent;	// TODO: Could be used to speed up re-searches
	SearchTree *children[10];
	Common::StringList words;
	
	SearchTree() {
		memset(children, 0, sizeof(children));
	}
	
	SearchTree *getChild(byte val) {
		assert(val < 10);
		if (children[val] == 0) {
			children[val] = new SearchTree();
		}
		return children[val];
	}

	SearchTree *findChildWithWords() {
		if (!words.empty())
			return this;
		
		SearchTree *child = 0;
		for (int i = 0; i < 10 && !child; ++i) {
			if (children[i])
				child = children[i]->findChildWithWords();
		}
	
		return child;
	}
	
};


void AgiEngine::insertSearchNode(const char *word) {
	// Insert the word into the tree
	SearchTree *tree = _searchTreeRoot;
	assert(tree);
	for (int i = 0; word[i] != 0; ++i) {
		byte key = s_asciiToNumTable[(int)word[i]];
		if (key == 0)
			return;	// abort!
		tree = tree->getChild(key);
	}

	// TODO: Sort words, remove duplicates... ?
	tree->words.push_back(word);
}

bool AgiEngine::predictiveDialog(void) {
	int key = 0, active = -1, lastactive = -1;
	bool rc = false;
	int x;
	int y;
	int bx[17], by[17];
	String prefix;
	char temp[MAXWORDLEN + 1], repeatcount[MAXWORDLEN];
	AgiBlock tmpwindow;

	_predictiveDialogRunning = true;
	
	// FIXME: Move this to a more appropriate place.
	initAsciiToNumTable();

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

	if (!_searchTreeRoot) {
		loadDict();
		if (!_searchTreeRoot)
			return false;
	}

	memset(repeatcount, 0, MAXWORDLEN);

	// show the predictive dialog.
	// if another window is already in display, save its state into tmpwindow
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

	/* clear key queue */
	while (_gfx->keypress()) {
		_gfx->getKey();
	}

	prefix.clear();
	_currentCode.clear();
	_currentWord.clear();
	_wordNumber = 0;
	_activeTreeNode = 0;

	int mode = kModePre;

	bool needRefresh = true;

	for (;;) {
		if (needRefresh) {
			for (int i = 0; buttons[i]; i++) {
				int color1 = colors[i * 2];
				int color2 = colors[i * 2 + 1];

				if (i == 9 && !((mode != kModeAbc && _activeTreeNode && _activeTreeNode->words.size() > 1) || 
							(mode == kModeAbc && _currentWord.size() && _currentWord.lastChar() != ' '))) { // Next
					color2 = 7;
				}
				
				bool _addIsActive = false; // FIXME
				if (i == 10 && !_addIsActive) { // Add
					color2 = 7;
				}
				if (i == 14) {
					_gfx->drawButton(bx[i], by[i], modes[mode], i == active, 0, color1, color2);
				} else {
					_gfx->drawButton(bx[i], by[i], buttons[i], i == active, 0, color1, color2);
				}
			}

				temp[MAXWORDLEN] = 0;

			strncpy(temp, prefix.c_str(), MAXWORDLEN);
			strncat(temp, _currentWord.c_str(), MAXWORDLEN);

			for (int i = prefix.size() + _currentCode.size(); i < MAXWORDLEN; i++)
				temp[i] = ' ';

			printText(temp, 0, 8, 7, MAXWORDLEN, 15, 0);
			_gfx->flushBlock(62, 54, 249, 66);

			if (active >= 0 && key != 9) {
				active = -1;
				needRefresh = true;
			} else
				needRefresh = false;

			_gfx->doUpdate();
		}

		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = doPollKeyboard();
		switch (key) {
		case KEY_ENTER:
			rc = true;
			goto press;
		case KEY_ESCAPE:
			rc = false;
			goto getout;
		case BUTTON_LEFT:
			for (int i = 0; buttons[i]; i++) {
				if (_gfx->testButton(bx[i], by[i], buttons[i])) {
					active = i;
				}
			}
processkey:
			if (active >= 0) {
				needRefresh = true;
				lastactive = active;
				if (active == 15 && mode != kModeNum) { // Space
					strncpy(temp, _currentWord.c_str(), _currentCode.size());

					temp[_currentCode.size()] = 0;

					prefix += temp;
					prefix += " ";
					_currentCode.clear();
					memset(repeatcount, 0, MAXWORDLEN);
					break;
				} if (active < 9 || active == 11 || active == 15) { // number or backspace
					if (active == 11) { // backspace
						if (_currentCode.size()) {
							repeatcount[_currentCode.size() - 1] = 0;
							_currentCode.deleteLastChar();
						} else {
							if (prefix.size())
								prefix.deleteLastChar();
						}
					} else if (active == 15) { // zero
						_currentCode += buttonStr[9];
					} else {
						_currentCode += buttonStr[active];
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
						break;
					case kModeAbc:
						for (x = 0; x < _currentCode.size(); x++)
							if (_currentCode[x] >= '1')
								temp[x] = buttons[_currentCode[x] - '1'][3 + repeatcount[x]];
						temp[_currentCode.size()] = 0;
						_currentWord = temp;
					}
				} else if (active == 9) { // next
					if (mode != kModeAbc) {
						int totalWordsNumber = _activeTreeNode ? _activeTreeNode->words.size() : 0;
						if (totalWordsNumber > 0) {
							_wordNumber = (_wordNumber + 1) % totalWordsNumber;
							_currentWord = String(_activeTreeNode->words[_wordNumber].c_str(), _currentCode.size());
						}
					} else {
						x = _currentCode.size();
						if (x) {
							if (_currentCode.lastChar() == '1' || _currentCode.lastChar() == '7' || _currentCode.lastChar() == '9')
								repeatcount[x - 1] = (repeatcount[x - 1] + 1) % 4;
							else
								repeatcount[x - 1] = (repeatcount[x - 1] + 1) % 3;
							if (_currentCode.lastChar() >= '1')
								_currentWord[x - 1] = buttons[_currentCode[x - 1] - '1'][3 + repeatcount[x - 1]];
						}
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
			break;
		case KEY_BACKSPACE:
			active = 11;
			goto processkey;
		case '#':
			active = 9;
			goto processkey;
		case '*':
			active = 14;
			goto processkey;
		case 0x09:	/* Tab */
			debugC(3, kDebugLevelText, "Focus change");
			lastactive = active = lastactive + 1;
			active %= ARRAYSIZE(buttons) - 1;
			needRefresh = true;
			break;
		default:
			// handle numeric buttons
			if (key >= '1' && key <= '9') {
				active = key - '1';
				goto processkey;
			} else if (key == '0') {
				active = 15;
				goto processkey;
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

	_predictiveDialogRunning = false;

	return rc;
}

#define MAXLINELEN 80

void AgiEngine::loadDict(void) {
	Common::File in;
	char buf[MAXLINELEN];
	int words = 0, lines = 0;

	if (!in.open("pred.txt"))
		return;

	_searchTreeRoot = new SearchTree();
	words = 0;

	while (!in.eos() && in.readLine(buf, MAXLINELEN)) {
		// Skip leading & trailing whitespaces
		char *word = Common::trim(buf);

		// Skip empty lines
		if (*word == 0)
			continue;
		
		lines++;

		// The lines are of the form:  "1234 word word"
		// I.e. first comes a T9 number, then a space separated list of
		// words with that T9 code. We simply ignore the T9 code, and then
		// insert the words in order of occurance.
		char *tok = strtok(word, " \t");
		if (tok) {
			while ((tok = strtok(NULL, " ")) != NULL) {
				insertSearchNode(tok);
				words++;
			}
		}
	}

	debug(0, "Loaded %d lines with %d words", lines, words);
}

bool AgiEngine::matchWord(void) {
	if (_currentCode.empty()) {
		return false;
	}
	
	// Lookup word in the search tree
	SearchTree *tree = _searchTreeRoot;
	assert(tree);
	for (uint i = 0; i < _currentCode.size(); ++i) {
		int key = _currentCode[i] - '0';
		if (key < 1 || key > 9) {
			tree = 0;
			break;	// Invalid key/code value, abort!
		}
		tree = tree->children[key];
		if (!tree)
			break;	// No matching entry in the search tree, abort!
	}
	
	if (tree)
		tree = tree->findChildWithWords();

	_wordNumber = 0;
	_activeTreeNode = tree;
	_currentWord.clear();


	if (tree) {
		_currentWord = String(_activeTreeNode->words[_wordNumber].c_str(), _currentCode.size());
	}

	return tree != 0;
}

} // End of namespace Agi
