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

#include "dreamweb/dreamweb.h"

namespace DreamGen {

const uint16 kKeypadx = 36+112;
const uint16 kKeypady = 72;

void DreamBase::getUnderMenu() {
	multiGet(_underTimedText, kMenux, kMenuy, 48, 48);
}

void DreamBase::putUnderMenu() {
	multiPut(_underTimedText, kMenux, kMenuy, 48, 48);
}

void DreamBase::singleKey(uint8 key, uint16 x, uint16 y) {
	if (key == _graphicPress) {
		key += 11;
		if (_pressCount < 8)
			key -= 11;
	}
	key -= 20;
	showFrame(_tempGraphics, x, y, key, 0);
}

void DreamBase::loadKeypad() {
	loadIntoTemp("DREAMWEB.G02");
}

void DreamBase::showKeypad() {
	singleKey(22, kKeypadx+9,  kKeypady+5);
	singleKey(23, kKeypadx+31, kKeypady+5);
	singleKey(24, kKeypadx+53, kKeypady+5);
	singleKey(25, kKeypadx+9,  kKeypady+23);
	singleKey(26, kKeypadx+31, kKeypady+23);
	singleKey(27, kKeypadx+53, kKeypady+23);
	singleKey(28, kKeypadx+9,  kKeypady+41);
	singleKey(29, kKeypadx+31, kKeypady+41);
	singleKey(30, kKeypadx+53, kKeypady+41);
	singleKey(31, kKeypadx+9,  kKeypady+59);
	singleKey(32, kKeypadx+31, kKeypady+59);
	if (_lightCount) {
		--_lightCount;
		uint8 frameIndex;
		uint16 y;
		if (_vars._lockStatus) {
			frameIndex = 36;
			y = kKeypady-1+63;
		} else {
			frameIndex = 41;
			y = kKeypady+4+63;
		}
		if ((_lightCount >= 60) && (_lightCount < 100))
			--frameIndex;
		showFrame(_tempGraphics, kKeypadx+60, y, frameIndex, 0);
	}
}

bool DreamBase::isItRight(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3) {
	
	return digit0 == _pressList[0] && digit1 == _pressList[1]
		&& digit2 == _pressList[2] && digit3 == _pressList[3];
}

void DreamBase::addToPressList() {
	if (_pressPointer == 5)
		return;
	uint8 pressed = _pressed;
	if (pressed == 10)
		pressed = 0;

	_pressList[_pressPointer] = pressed;
	++_pressPointer;
}

void DreamBase::enterCode(uint8 digit0, uint8 digit1, uint8 digit2, uint8 digit3) {
	RectWithCallback<DreamBase> keypadList[] = {
		{ kKeypadx+9,kKeypadx+30,kKeypady+9,kKeypady+22,&DreamBase::buttonOne },
		{ kKeypadx+31,kKeypadx+52,kKeypady+9,kKeypady+22,&DreamBase::buttonTwo },
		{ kKeypadx+53,kKeypadx+74,kKeypady+9,kKeypady+22,&DreamBase::buttonThree },
		{ kKeypadx+9,kKeypadx+30,kKeypady+23,kKeypady+40,&DreamBase::buttonFour },
		{ kKeypadx+31,kKeypadx+52,kKeypady+23,kKeypady+40,&DreamBase::buttonFive },
		{ kKeypadx+53,kKeypadx+74,kKeypady+23,kKeypady+40,&DreamBase::buttonSix },
		{ kKeypadx+9,kKeypadx+30,kKeypady+41,kKeypady+58,&DreamBase::buttonSeven },
		{ kKeypadx+31,kKeypadx+52,kKeypady+41,kKeypady+58,&DreamBase::buttonEight },
		{ kKeypadx+53,kKeypadx+74,kKeypady+41,kKeypady+58,&DreamBase::buttonNine },
		{ kKeypadx+9,kKeypadx+30,kKeypady+59,kKeypady+73,&DreamBase::buttonNought },
		{ kKeypadx+31,kKeypadx+74,kKeypady+59,kKeypady+73,&DreamBase::buttonEnter },
		{ kKeypadx+72,kKeypadx+86,kKeypady+80,kKeypady+94,&DreamBase::quitKey },
		{ 0,320,0,200,&DreamBase::blank },
		{ 0xFFFF,0,0,0,0 }
	};

	getRidOfReels();
	loadKeypad();
	createPanel();
	showIcon();
	showOuterPad();
	showKeypad();
	readMouse();
	showPointer();
	workToScreen();
	delPointer();
	_pressPointer = 0;
	_getBack = 0;
	while (true) {
		delPointer();
		readMouse();
		showKeypad();
		showPointer();
		vSync();
		if (_pressCount == 0) {
			_pressed = 255;
			_graphicPress = 255;
			vSync();
		} else
			--_pressCount;

		dumpPointer();
		dumpKeypad();
		dumpTextLine();
		checkCoords(keypadList);
		if (_quitRequested || (_getBack == 1))
			break;
		if (_lightCount == 1) {
			if (_vars._lockStatus == 0)
				break;
		} else {
			if (_pressCount == 40) {
				addToPressList();
				if (_pressed == 11) {
					if (isItRight(digit0, digit1, digit2, digit3))
						_vars._lockStatus = 0;
					playChannel1(11);
					_lightCount = 120;
					_pressPointer = 0;
				}
			}
		}
	}
	_manIsOffScreen = 0;
	getRidOfTemp();
	restoreReels();
	redrawMainScrn();
	workToScreenM();
}

void DreamBase::buttonOne() {
	buttonPress(1);
}

void DreamBase::buttonTwo() {
	buttonPress(2);
}

void DreamBase::buttonThree() {
	buttonPress(3);
}

void DreamBase::buttonFour() {
	buttonPress(4);
}

void DreamBase::buttonFive() {
	buttonPress(5);
}

void DreamBase::buttonSix() {
	buttonPress(6);
}

void DreamBase::buttonSeven() {
	buttonPress(7);
}

void DreamBase::buttonEight() {
	buttonPress(8);
}

void DreamBase::buttonNine() {
	buttonPress(9);
}

void DreamBase::buttonNought() {
	buttonPress(10);
}

void DreamBase::buttonEnter() {
	buttonPress(11);
}

void DreamBase::buttonPress(uint8 buttonId) {
	uint8 commandType = 100 + buttonId;
	if (_commandType != commandType) {
		_commandType = commandType;
		commandOnly(buttonId + 4);
	}
	if ((_mouseButton & 1) && (_mouseButton != _oldButton)) {
		_pressed = buttonId;
		_graphicPress = buttonId + 21;
		_pressCount = 40;
		if (buttonId != 11)
			playChannel1(10);
	}
}

void DreamBase::showOuterPad() {
	showFrame(_tempGraphics, kKeypadx-3, kKeypady-4, 1, 0);
	showFrame(_tempGraphics, kKeypadx+74, kKeypady+76, 37, 0);
}

void DreamBase::dumpKeypad() {
	multiDump(kKeypadx - 3, kKeypady - 4, 120, 90);
}

void DreamBase::dumpSymbol() {
	_newTextLine = 0;
	multiDump(kSymbolx, kSymboly + 20, 104, 60);
}

void DreamBase::dumpSymBox() {
	if (_dumpX != 0xFFFF) {
		multiDump(_dumpX, _dumpY, 30, 77);
		_dumpX = 0xFFFF;
	}
}

void DreamBase::quitSymbol() {
	if (_symbolTopX != 24 || _symbolBotX != 24) {
		blank();
		return;
	};

	if (_commandType != 222) {
		_commandType = 222;
		commandOnly(18);
	}

	if (_mouseButton == _oldButton)
		return;	// notqs

	if (!(_mouseButton & 1))
		return;

	_getBack = 1;
}

} // End of namespace DreamGen
