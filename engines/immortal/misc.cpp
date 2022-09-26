/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "immortal/immortal.h"

namespace Immortal {

/* 
 *
 * -----                -----
 * ----- Main Functions -----
 * -----                -----
 *
 */

void ImmortalEngine::miscInit() {
	// In the source, this is where the seed for the rng is set, but we don't need to do that as we used _randomSource
	_lastGauge = 0;
}

void ImmortalEngine::setRandomSeed() {}
void ImmortalEngine::getRandom() {}

/* 
 *
 * -----               -----
 * ----- Text Printing -----
 * -----               -----
 *
 */

// myFadeOut and myFadeIn are just RTS in the source, but they are called quite a lot
void ImmortalEngine::myFadeOut() {
	return;
}

void ImmortalEngine::myFadeIn() {
	return;
}

bool ImmortalEngine::textPrint(Str s, int n) {
	_slowText = 0;
	_formatted = 0;
	_collumn = 0;
	_row = 0;
	playTextSong();
	clearScreen();
	return textSub(s, kTextFadeIn, n);
}

bool ImmortalEngine::textBeginning(Str s, int n) {
	_slowText = 0;
	_formatted = 0;
	_collumn = 0;
	_row = 0;
	playTextSong();
	clearScreen();
	return textSub(s, kTextDontFadeIn, n);
}

void ImmortalEngine::textEnd(Str s, int n) {
	textSub(s, kTextFadeIn, n);
}

void ImmortalEngine::textMiddle(Str s, int n) {
	textSub(s, kTextDontFadeIn, n);
}

bool ImmortalEngine::textSub(Str s, FadeType f, int n) {
	bool done = false;

	char chr = 0;
	int index = 0;
	Common::String text = _strPtrs[s];

	while (done == false) {
		switch (text[index]) {
			case '@':
			case '=':
			case char(0):
				done = true;
				// This is so the while loop can be a little cleaner
				index--;
				break;
			case '&':
				textCR();
				break;
			case '$':
				printByte(n);
				copyToScreen();
				break;
			case '_':
				myFadeIn();
				_slowText = 1;
				break;
			case '<':
				_slowText = 0;
				break;
			case '>':
				_formatted = 0;
				break;
			case '\\':
				normalFadeOut();
				break;
			case '/':
				slowFadeOut();
				break;
			case '|':
				normalFadeIn();
				break;
			case '}':
				_formatted = 1;
				break;
			case ']':
				myDelay(40);
				break;
			case '{':
				index++;
				myDelay(text[index]);
				break;
			case '*':
				textPageBreak(text, index);
				break;
			case '[':
				textAutoPageBreak();
				break;
			case '#':
				index++;
				drawIcon(text[index]);
				break;
			case '~':
				text = _strPtrs[(int)text[index + 1]];
				index = -1;
				break;
			case '^':
				center();
				break;
			case '%':
				return yesNo();
			case '+':
				chr = 0x27;
				break;
			case '(':
				chr = 0x60;
				break;
			default:
				chr = text[index];
				_collumn++;
				if (chr == ' ') {
					if (text[index + 1] == '~') {
						text = _strPtrs[(int)text[index + 2]];
						index = -1;
					}
					textDoSpace(text, index);
				
				} else {
					printChr(chr);
					// We need this to show up now, not when the frame ends, so we have to update the screen here
					copyToScreen();
					if (_slowText != 0) {
						myDelay(5);
						switch (chr) {
							case '?':
							case ':':
								myDelay(13);
							case '.':
								myDelay(13);
							case ',':
								myDelay(13);
							default:
								break;
						}
					}
				}
				break;
		}
		if (index == 0xFF) {
			debug("String too long!");
			return false;
		}
		index++;
	}
	
	chr = text[index];

	if (f != kTextFadeIn) {
		return false;
	}

	// If we need to display an 'OK' message
	if (chr != '=') {
		setPen(_penX, kYesNoY);
		center();
		drawIcon(kOkayFrame);
		copyToScreen();
		if (_slowText == 0) {
			myFadeIn();
		}
		waitClick();
		standardBeep();
		textBounceDelay();

	} else if (_slowText == 0) {
		myFadeIn();
	}

	return false;
}

void ImmortalEngine::textCR() {
	carriageReturn();
	_row++;
	_collumn = 0;
}

void ImmortalEngine::textPageBreak(Common::String s, int &index) {
	_collumn = 0;
	_row = 0;
	if (_slowText == 0) {
		myFadeIn();
	}

	index++;
	myDelay((int) s[index]);
	myFadeOut();
	clearScreen();

	if (_slowText != 0) {
		myFadeIn();
	}
}

void ImmortalEngine::textAutoPageBreak() {
	_collumn = 0;
	_row = 0;
	if (_slowText == 0) {
		myFadeIn();
	}

	myDelay(140);
	myFadeOut();
	clearScreen();

	if (_slowText != 0) {
		myFadeIn();
	}
}

void ImmortalEngine::textDoSpace(Common::String s, int index) {
	// If text is formatted, then check if the space between here and the end of the string will fit, if not, use a newline or pagebreak
	if (_formatted != 0) {
		bool foundEnd = false;
		int start = index;
		while (foundEnd == false) {
			index++;
			switch (s[index]) {
				case '=':
				case '@':
				case '%':
				case '[':
				case ' ':
					foundEnd = true;
				default:
					break;
			}
		}
		if (((index - start) + _collumn) >= kMaxCollumns) {
			if (_row < kMaxRows) {
				textCR();
			
			} else {
				textAutoPageBreak();
			}
			return;
		}
	}
	printChr(' ');
}

void ImmortalEngine::textBounceDelay() {
	Utilities::delay(7);
}

bool ImmortalEngine::yesNo() {
	uint8 tyes[9] = {0, 1, 1, 1, 0, 0, 0, 0, 0};

	getInput();

	if (tyes[_heldDirection] == 0) {
		noOn();
		_lastYes = 0;
	
	} else {
		yesOn();
		_lastYes = 1;
	}

	while (buttonPressed() || firePressed()) {
		// If they have not pressed a button yet, we get the input after a delay
		Utilities::delay(4);
		getInput();

		// And then if they have changed direction, we play a sound and update the direction and button gfx
		if (tyes[_heldDirection] != _lastYes) {
			_lastYes = tyes[_heldDirection];
			standardBeep();
			if (_lastYes == 0) {
				noOn();
			
			} else {
				yesOn();
			}
			// Since we need this to show up right during the text sequence, we need to update the screen
			copyToScreen();
		}
	}

	standardBeep();
	textBounceDelay();

	// In source this is done weirdly so that it can use a result in A, except it never uses that result, so it's just weird.
	return (!(bool) _lastYes);
}

void ImmortalEngine::noOn() {
	// Draw the No icon as on, and the Yes icon as off
	setPen(kYesNoX1, kYesNoY);
	drawIcon(kNoIconOn);
	setPen(kYesNoX2, kYesNoY);
	drawIcon(kYesIconOff);
}

void ImmortalEngine::yesOn() {
	// Draw the No icon as off, and the Yes icon as on
	setPen(kYesNoX1, kYesNoY);
	drawIcon(kNoIconOff);
	setPen(kYesNoX2, kYesNoY);
	drawIcon(kYesIconOn);
}

void ImmortalEngine::myDelay(int j) {
	int type = 0;

	// Update input
	getInput();

	// 0 = neither button held, 1 = one held, 2 = both held
	if (_heldAction & kActionButton) {
		type++;
	}

	if (_heldAction & kActionFire) {
		type++;
	}

	do {
		// If the button was *pressed* and not held, then skip any delay
		if (!buttonPressed()) {
			return;
		}

		if (!firePressed()) {
			return;
		}

		// Otherwise, we delay by different amounts based on what's held down
		switch (type) {
			case 1:
				Utilities::delay4(1);
				break;
			case 0:
				Utilities::delay(1);
			case 2:
			default:
				break;
		}

		j--;
	} while (j != 0);
}

/* 
 *
 * -----               -----
 * ----- Input Related -----
 * -----               -----
 *
 */

bool ImmortalEngine::buttonPressed() {
	// Returns false if the button was pressed, but not held or up
	getInput();
	
	if (_heldAction == kActionButton) {
		// Zero just the button0held bit
		_myButton &= (0xFF - kButton0Held);
	
	} else if ((_myButton & kButton0Held) == 0) {
		_myButton |= kButton0Held;
		return false;
	}

	return true;
}

bool ImmortalEngine::firePressed() {
	// Returns false if the button was pressed, but not held or up
	getInput();
	
	if (_heldAction == kActionFire) {
		_myButton &= (0xFF - kButton1Held);
	
	} else if ((_myButton & kButton1Held) == 0) {
		_myButton |= kButton1Held;
		return false;
	}

	return true;
}


/* 
 *
 * -----                -----
 * ----- Screen Related -----
 * -----                -----
 *
 */


/*
 *
 * -----			   -----
 * ----- Sound Related -----
 * -----			   -----
 *
 */

void ImmortalEngine::standardBeep() {
	//playNote(4, 5, 0x4C);
}

} // namespace Immortal
























