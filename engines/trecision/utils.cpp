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

#include "common/system.h"

#include "trecision/scheduler.h"
#include "trecision/text.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/trecision.h"
#include "trecision/struct.h"

namespace Trecision {

char *TrecisionEngine::getNextSentence() {
	while (*_textPtr) {
		*_textPtr = ~(*_textPtr);
		++_textPtr;
	}

	++_textPtr;
	return _textPtr;
}

/**
 * Compute string length from character begin to end
 */
uint16 TrecisionEngine::textLength(const Common::String &text, uint16 begin, uint16 end) {
	if (text.empty())
		return 0;

	if (end == 0)
		end = text.size();

	uint16 retVal = 0;
	for (uint16 c = begin; c < end; ++c)
		retVal += _graphicsMgr->getCharWidth(text[c]);

	return retVal;
}

uint16 TrecisionEngine::getKey() {
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
		if (ascii)
			return ascii;

		return 0;
	}
}

Common::KeyCode TrecisionEngine::waitKey() {
	_graphicsMgr->hideCursor();
	while (_curKey == Common::KEYCODE_INVALID)
		checkSystem();
	_graphicsMgr->showCursor();

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

bool TrecisionEngine::checkMask(Common::Point pos) {
	for (int8 i = MAXOBJINROOM - 1; i >= 0; --i) {
		uint16 checkedObj = _room[_curRoom]._object[i];
		Common::Rect lim = _obj[checkedObj]._area;
		lim.translate(0, TOP);
		// Trecision includes the bottom and right coordinates
		++lim.right;
		++lim.bottom;

		if (checkedObj && isObjectVisible(checkedObj)) {
			if (lim.contains(pos)) {

				if (_obj[checkedObj].isModeFull() || _obj[checkedObj].isModeLim()) {
					_curObj = checkedObj;
					return true;
				}

				if (_obj[checkedObj].isModeMask()) {
					uint8 *mask = _objectGraphics[i].mask;
					int16 d = _obj[checkedObj]._rect.left;
					uint16 max = _obj[checkedObj]._rect.bottom;

					for (uint16 j = _obj[checkedObj]._rect.top; j < max; ++j) {
						bool insideObj = false;
						int16 e = 0;
						while (e < _obj[checkedObj]._rect.width()) {
							if (!insideObj) { // not inside an object
								if (j + TOP == pos.y) {
									if ((pos.x >= d + e) && (pos.x < d + e + *mask)) {
										_curObj = 0;
									}
								}

								e += *mask;
								++mask;
								insideObj = true;
							} else { // inside an object
								if (j + TOP == pos.y) {
									if ((pos.x >= d + e) && (pos.x < d + e + *mask)) {
										_curObj = checkedObj;
										return true;
									}
								}

								e += *mask;
								++mask;
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

float TrecisionEngine::sinCosAngle(float sinus, float cosinus) {
	if (floatComp(sinus, 0.0f) == 0 && floatComp(cosinus, 0.0f) == 0)
		return 0;

	float t = (float)sqrt((double)(sinus * sinus) + (double)(cosinus * cosinus));
	cosinus /= t;
	sinus /= t;

	// 1e3 & 2e4 quad
	if (floatComp(sinus, 0.0f) >= 0)
		// 1 & 2 quad
		return (float)acos(cosinus);

	// 3 quad
	return PI2 - (float)acos(cosinus);
}

void TrecisionEngine::processTime() {
	_curTime = readTime();

	if (_curTime >= _nextRefresh) {
		if (_inventoryStatus == INV_PAINT || _inventoryStatus == INV_DEPAINT)
			rollInventory(_inventoryStatus);

		if (_inventoryStatus != INV_OFF) {
			refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
		}

		_textMgr->drawTexts();
		_graphicsMgr->paintScreen(false);
		_textMgr->clearTextStack();

		uint32 paintTime = readTime();
		if (paintTime - _curTime >= 5)
			_nextRefresh = paintTime + 1;
		else
			_nextRefresh = _curTime + 5;
	}
}

void TrecisionEngine::processMouse() {
	int16 mx = _mousePos.x;
	int16 my = _mousePos.y;

	checkSystem();

	if (!_graphicsMgr->isCursorVisible())
		return;

	if (_mouseLeftBtn) {
		_scheduler->leftClick(mx, my);
		_mouseLeftBtn = false;
	} else if (_mouseRightBtn) {
		_scheduler->rightClick(mx, my);
		_mouseRightBtn = false;
	} else {
		if (!_flagScriptActive && _mouseMoved) {
			processMouseMovement();
			_mouseMoved = false;
		}
	}
}

/**
 *		Fake distance between two 2D points
 */
float TrecisionEngine::dist2D(float x1, float y1, float x2, float y2) {
	const double dx = x1 - x2;
	const double dy = y1 - y2;

	return (float)sqrt(dx * dx + dy * dy);
}

/**
 *			Distance between two 3D points
 */
float TrecisionEngine::dist3D(float x1, float y1, float z1, float x2, float y2, float z2) {
	const double dx = x1 - x2;
	const double dy = y1 - y2;
	const double dz = z1 - z2;
	return (float)sqrt(dx * dx + dy * dy + dz * dz);
}

bool TrecisionEngine::isBetween(int a, int x, int b) {
	return x >= a && x <= b;
}

bool TrecisionEngine::isGameArea(Common::Point pos) {
	return isBetween(TOP, pos.y, TOP + AREA - 1);
}

bool TrecisionEngine::isInventoryArea(Common::Point pos) {
	return pos.y >= TOP + AREA;
}

bool TrecisionEngine::isIconArea(Common::Point pos) {
	return pos.y >= TOP + AREA && pos.y < MAXY && pos.x >= ICONMARGSX && pos.x <= MAXX - ICONMARGDX;
}

int TrecisionEngine::getRoomObjectIndex(uint16 objectId) {
	for (uint16 index = 0; index < MAXOBJINROOM; ++index) {
		const uint16 curObjId = _room[_curRoom]._object[index];
		if (curObjId == 0)
			return -1;
		if (curObjId == objectId)
			return index;
	}

	return -1;
}

/************************************************
 * SDText
 ************************************************/
void SDText::set(SDText *org) {
	set(org->_rect, org->_subtitleRect, org->_textColor, org->_text);
}

void SDText::set(Common::Rect rect, Common::Rect subtitleRect, uint16 textCol, const Common::String &text) {
	_rect = rect;
	_subtitleRect = subtitleRect;
	_textColor = textCol;
	_text = text;

	// Clean output buffer
	for (int i = 0; i < MAXDTEXTLINES; ++i)
		_drawTextLines[i] = "";
}

/**
 *   calcHeight - Computes and returns the dy of the current text
 */
uint16 SDText::calcHeight(TrecisionEngine *vm) {
	if (_text.empty())
		return 0;

	uint8 curLine = 0;
	if (vm->textLength(_text) <= _rect.width()) {
		_drawTextLines[curLine] = _text;
		return CARHEI;
	}

	uint16 index = 0;
	uint16 tmpDy = 0;
	uint16 lastSpace = 0;
	uint16 curInit = 0;

	while (index < _text.size()) {
		++index;
		if (index < _text.size() && _text[index] == ' ') {
			if (vm->textLength(_text, curInit, index) <= _rect.width())
				lastSpace = index;
			else if (vm->textLength(_text, curInit, lastSpace) <= _rect.width()) {
				_drawTextLines[curLine] = _text.substr(curInit, lastSpace - curInit);

				++curLine;
				curInit = lastSpace + 1;

				tmpDy += CARHEI;
				index = curInit;
			} else
				return 0;
		} else if (index == _text.size()) {
			if (vm->textLength(_text, curInit, index) <= _rect.width()) {
				_drawTextLines[curLine] = _text.substr(curInit, index - curInit);

				tmpDy += CARHEI;
				return tmpDy;
			}

			if (vm->textLength(_text, curInit, lastSpace) <= _rect.width()) {
				_drawTextLines[curLine] = _text.substr(curInit, lastSpace - curInit);

				++curLine;
				curInit = lastSpace + 1;
				tmpDy += CARHEI;

				if (curInit < _text.size()) {
					_drawTextLines[curLine] = _text.substr(curInit);
					tmpDy += CARHEI;
				}
				return tmpDy;
			}
			return 0;
		}
	}
	return 0;
}

void SDText::draw(TrecisionEngine *vm, bool hideLastChar, Graphics::Surface *externalSurface) {
	uint16 textColor = vm->_graphicsMgr->convertToScreenFormat(_textColor);
	
	if (_text.empty())
		return;

	const uint16 curDy = calcHeight(vm);

	for (uint16 line = 0; line < curDy / CARHEI; ++line) {
		Common::String curText = _drawTextLines[line];
		uint16 inc = (_rect.width() - vm->textLength(curText)) / 2;

		if (curText.size() >= MAXCHARS) {
			curText = vm->_sysText[kMessageError];
		}

		for (uint index = 0; index < curText.size(); ++index) {
			const byte curChar = curText[index];

			if (index == curText.size() - 1 && hideLastChar)
				textColor = vm->_graphicsMgr->convertToScreenFormat(0);

			vm->_graphicsMgr->drawChar(curChar, textColor, line, _rect, _subtitleRect, inc, externalSurface);

			inc += vm->_graphicsMgr->getCharWidth(curChar);
		}
	}
}

int TrecisionEngine::floatComp(float f1, float f2) const {
	static const float epsilon = 1.0e-05f;

	const float diff = f1 - f2;
	if (ABS(diff) < epsilon)
		// equality
		return 0;

	if (f1 > f2)
		return 1;

	return -1;
}
} // End of namespace Trecision
