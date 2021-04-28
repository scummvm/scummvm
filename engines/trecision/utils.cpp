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

#include <common/system.h>



#include "graphics.h"
#include "trecision/nl/define.h"
#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/logic.h"
#include "trecision/trecision.h"
#include "trecision/actor.h"

namespace Trecision {

void TrecisionEngine::initNames() {
	// Initial value of useful system texts.
	// Those values are overwritten by the ones from the game files later
	_sysText[kMessageSavePosition] = "SAVE POSITION";
	_sysText[kMessageEmptySpot] = "EMPTY SLOT";
	_sysText[kMessageLoadPosition] = "LOAD POSITION";
	_sysText[kMessageConfirmExit] = "Are you sure that you want to quit (y/n)?";
	_sysText[kMessageDemoOver] = "This demo is over.";
	_sysText[kMessageError] = "ERROR!";
	_sysText[kMessageUse] = "Use ";
	_sysText[kMessageWith] = " with ";
	_sysText[kMessageGoto] = "Go to ";
	_sysText[kMessageGoto2] = "Go to ... ";

	_sentence[0] = "          "; // Use it like a buffer !!!!
	_objName[0] = " ";
}

char *TrecisionEngine::getNextSentence() {
	while (*_textPtr) {
		*_textPtr = ~(*_textPtr);
		_textPtr++;
	}

	_textPtr++;
	return _textPtr;
}

void TrecisionEngine::addText(uint16 x, uint16 y, const char *text, uint16 tcol, uint16 scol) {
	_textStackTop++;
	if (_textStackTop >= MAXTEXTSTACK) {
		warning("MAXTEXTSTACK Reached!");
		return;
	}

	_textStack[_textStackTop].x = x;
	_textStack[_textStackTop].y = y;
	_textStack[_textStackTop].tcol = tcol;
	_textStack[_textStackTop].scol = scol;
	_textStack[_textStackTop].clear = false;
	strcpy(_textStack[_textStackTop].text, text);
}

void TrecisionEngine::clearText() {
	if (_textStackTop >= 0) {
		// The stack isn't empty
		if (!_textStack[_textStackTop].clear)
			// The previous is a string to write, return
			_textStackTop--;
	} else {
		// the stack is empty
		_textStackTop = 0;
		_textStack[_textStackTop].clear = true;
	}
}

void TrecisionEngine::drawString() {
	for (int16 i = 0; i <= _textStackTop; i++) {
		if (_textStack[i].clear)
			doClearText();
		else
			_textStack[i].doText();
	}
}

void TrecisionEngine::redrawString() {
	if (!_flagDialogActive && !_flagDialogMenuActive && !_flagSomeoneSpeaks && !_flagscriptactive && isCursorVisible()) {
		if (isInventoryArea(_mouseY))
			doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
		else {
			checkMask(_mouseX, _mouseY);
			ShowObjName(_curObj, true);
		}
	}
}

void StackText::doText() {
	curString._rect.left = x;
	curString._rect.top = y;
	curString._rect.right = g_vm->textLength(text, 0) + curString._rect.left;
	int16 w = curString._rect.width();

	if ((y == MAXY - CARHEI) && (w > 600))
		w = w * 3 / 5;
	else if ((y != MAXY - CARHEI) && (w > 960))
		w = w * 2 / 5;
	else if ((y != MAXY - CARHEI) && (w > 320))
		w = w * 3 / 5;

	curString._rect.right = w + curString._rect.left;

	curString.text = text;
	curString._subtitleRect.left = 0;
	curString._subtitleRect.top = 0;
	curString._subtitleRect.right = curString._rect.width();
	uint16 hstring = curString.checkDText();
	curString._subtitleRect.bottom = hstring;
	curString._rect.bottom = hstring + curString._rect.top;
	curString.tcol = tcol;
	curString.scol = scol;

	if (curString._rect.top <= hstring)
		curString._rect.top += hstring;
	else
		curString._rect.top -= hstring;

	if (curString._rect.top <= VIDEOTOP)
		curString._rect.top = VIDEOTOP + 1;

	TextStatus |= TEXT_DRAW;
}

void TrecisionEngine::doClearText() {
	if (!oldString.text && curString.text) {
		oldString.set(curString);
		curString.text = nullptr;

		TextStatus |= TEXT_DEL;
	}
}

void TrecisionEngine::setRoom(uint16 r, bool b) {
	_logicMgr->setupAltRoom(r, b);
	RegenRoom();
}

/*-------------------------------------------------
 * Compute string length from character 0 to num
 *-------------------------------------------------*/
uint16 TrecisionEngine::textLength(const char *text, uint16 num) {
	if (text == nullptr)
		return 0;

	uint16 len = (num == 0) ? strlen(text) : num;

	uint16 retVal = 0;
	for (uint16 c = 0; c < len; c++)
		retVal += _font[(uint8)text[c] * 3 + 2];

	return retVal;
}

char TrecisionEngine::getKey() {
	Common::KeyCode key = _curKey;
	uint16 ascii = _curAscii;
	_curKey = Common::KEYCODE_INVALID;
	_curAscii = 0;

	switch (key) {
	case Common::KEYCODE_SPACE:
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_CLEAR:
	case Common::KEYCODE_BACKSPACE:
		return key;
	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		return 0x3B + key - Common::KEYCODE_F1;
	default:
		if (ascii) {
			return ascii;
		}

		return 0;
	}
}

char TrecisionEngine::waitKey() {
	while (_curKey == Common::KEYCODE_INVALID)
		checkSystem();

	Common::KeyCode t = _curKey;
	_curKey = Common::KEYCODE_INVALID;

	return t;
}

void TrecisionEngine::waitDelay(uint32 val) {
	uint32 sv = readTime();

	while (sv + val > readTime())
		checkSystem();
}

void TrecisionEngine::freeKey() {
	_curKey = Common::KEYCODE_INVALID;
}

uint32 TrecisionEngine::readTime() {
	return (g_system->getMillis() * 3) / 50;
}

bool TrecisionEngine::checkMask(uint16 mx, uint16 my) {
	for (int8 a = MAXOBJINROOM - 1; a >= 0; a--) {
		uint16 checkedObj = _room[_curRoom]._object[a];
		Common::Rect lim = _obj[checkedObj]._lim;
		lim.translate(0, TOP);
		// Trecision includes the bottom and right coordinates
		lim.right++;
		lim.bottom++;

		if (checkedObj && (_obj[checkedObj]._mode & OBJMODE_OBJSTATUS)) {
			if (lim.contains(mx, my)) {

				if ((_obj[checkedObj]._mode & OBJMODE_FULL) || (_obj[checkedObj]._mode & OBJMODE_LIM)) {
					_curObj = checkedObj;
					return true;
				}

				if (_obj[checkedObj]._mode & OBJMODE_MASK) {
					uint8 *mask = _maskPointers[a];
					int16 d = _obj[checkedObj]._rect.left;
					uint16 max = _obj[checkedObj]._rect.bottom;

					for (uint16 b = _obj[checkedObj]._rect.top; b < max; b++) {
						bool insideObj = false;
						int16 e = 0;
						while (e < _obj[checkedObj]._rect.right) {
							if (!insideObj) { // not inside an object
								if (b + TOP == my) {
									if ((mx >= d + e) && (mx < d + e + *mask)) {
										_curObj = 0;
									}
								}

								e += *mask;
								mask++;
								insideObj = true;
							} else { // inside an object
								if (b + TOP == my) {
									if ((mx >= d + e) && (mx < d + e + *mask)) {
										_curObj = checkedObj;
										return true;
									}
								}

								e += *mask;
								mask++;
								insideObj = false;
							}
						}
					}
				}
			}
		}
	}
	_curObj = 0;
	return false;
}

Graphics::Surface *TrecisionEngine::convertScummVMThumbnail(Graphics::Surface *thumbnail) {
	Graphics::Surface *thumbnailConverted = thumbnail->convertTo(g_system->getScreenFormat());
	Graphics::Surface *result = thumbnailConverted->scale(ICONDX, ICONDY);

	thumbnailConverted->free();
	delete thumbnailConverted;
	thumbnail->free();
	delete thumbnail;

	return result;
}

void TrecisionEngine::resetZBuffer(int x1, int y1, int x2, int y2) {
	if (x1 > x2 || y1 > y2)
		return;

	int size = (x2 - x1) * (y2 - y1);
	if (size * 2 > ZBUFFERSIZE)
		warning("Warning: _zBuffer size %d!\n", size * 2);

	int16 *d = _zBuffer;
	for (int i = 0; i < size; ++i)
		*d++ = 0x7FFF;
}

void TrecisionEngine::openSys() {
	// head
	int32 idx = 0;
	FTexture[idx]._dx = 300 / 2;
	FTexture[idx]._dy = 208 / 2;
	FTexture[idx]._angle = 0;
	FTexture[idx]._texture = _textureArea;
	FTexture[idx]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// body
	idx = 1;
	FTexture[idx]._dx = 300;
	FTexture[idx]._dy = 300;
	FTexture[idx]._angle = 0;
	FTexture[idx]._texture = FTexture[0]._texture + (300 * 208) / 4;
	FTexture[idx]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	// arms
	idx = 2;
	FTexture[idx]._dx = 300;
	FTexture[idx]._dy = 150;
	FTexture[idx]._angle = 0;
	FTexture[idx]._texture = FTexture[1]._texture + 300 * 300;
	FTexture[idx]._flag = TEXTUREACTIVE + TEXTURECYLIND;

	delete _actor;
	_actor = new Actor(this);
	_actor->readActor("jm.om");

	_actor->_light = (SLight *)&VLight;
	_actor->_camera = (SCamera *)&FCamera;
	_actor->_texture = (STexture *)&FTexture[0];

	TextArea = new char[MAXTEXTAREA];

	// zbuffer
	_zBuffer = new int16[ZBUFFERSIZE / 2];
	for (int c = 0; c < ZBUFFERSIZE / 2; ++c)
		_zBuffer[c] = 0x7FFF;

	_screenBuffer = new uint16[MAXX * MAXY];
	memset(_screenBuffer, 0, MAXX * MAXY * 2);

	_graphicsMgr->clearScreen();

	hideCursor();

	for (int i = 0; i < MAXOBJINROOM; ++i) {
		OldObjStatus[i] = false;
		VideoObjStatus[i] = false;
	}
}

float TrecisionEngine::sinCosAngle(float sinus, float cosinus) {
	if (sinus == 0 && cosinus == 0)
		return 0;

	float t = (float)sqrt((double)(sinus * sinus) + (double)(cosinus * cosinus));
	cosinus /= t;
	sinus /= t;

	// 1e3 & 2e4 quad
	if (sinus >= 0)
		// 1 & 2 quad
		return (float)acos(cosinus);

	// 3 quad
	return PI2 - (float)acos(cosinus);
}

void TrecisionEngine::ProcessTime() {
	static uint8 OldRegInvSI = 0xFF;
	static uint8 OldRegInvSL = 0xFF;
	static uint8 OldLightIcon = 0xFF;

	TheTime = readTime();
	_animMgr->refreshAllAnimations();

	if (TheTime >= _nextRefresh) {
		drawString();

		if (_inventoryStatus == INV_PAINT || _inventoryStatus == INV_DEPAINT)
			rollInventory(_inventoryStatus);

		if (_inventoryStatus != INV_OFF && (OldRegInvSI != _inventoryRefreshStartIcon || OldRegInvSL != _inventoryRefreshStartLine || OldLightIcon != _lightIcon)) {
			refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			OldRegInvSI = _inventoryRefreshStartIcon;
			OldRegInvSL = _inventoryRefreshStartLine;
			OldLightIcon = _lightIcon;
		}

		PaintScreen(false);
		_textStackTop = -1;

		_flagScreenRefreshed = true;
		uint32 paintTime = readTime();
		if (paintTime - TheTime >= 5)
			_nextRefresh = paintTime + 1;
		else
			_nextRefresh = TheTime + 5;
	}
}

void TrecisionEngine::ProcessMouse() {
	static bool MaskMouse;
	static uint16 oldMouseX;
	static uint16 oldMouseY;
	static bool lastMouseOn = true;
	int16 mx = _mouseX;
	int16 my = _mouseY;

	if (lastMouseOn && !isCursorVisible()) {
		oldMouseX = 0; // Switch off
		oldMouseY = 0;
	} else if (!lastMouseOn && isCursorVisible()) {
		oldMouseX = 0; // Switch on
		oldMouseY = 0;
	}

	lastMouseOn = isCursorVisible();
	checkSystem();

	if (!isCursorVisible())
		return;

	if (_mouseLeftBtn || _mouseRightBtn) {
		if (!MaskMouse) {
			doEvent(MC_MOUSE, _mouseRightBtn ? ME_MRIGHT : ME_MLEFT, MP_DEFAULT, mx, my, 0, 0);
			MaskMouse = true;
		}
	} else {
		MaskMouse = false;

		if (!_flagscriptactive) {
			if (mx != oldMouseX || my != oldMouseY) {
				doEvent(MC_MOUSE, ME_MMOVE, MP_DEFAULT, mx, my, 0, 0);
				oldMouseX = mx;
				oldMouseY = my;
			}
		}
	}
}

} // End of namespace Trecision
