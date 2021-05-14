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

#include "3d.h"
#include "common/system.h"

#include "trecision/scheduler.h"
#include "trecision/text.h"
#include "trecision/actor.h"
#include "trecision/defines.h"
#include "trecision/graphics.h"
#include "trecision/logic.h"
#include "trecision/trecision.h"
#include "trecision/struct.h"

namespace Trecision {

char *TrecisionEngine::getNextSentence() {
	while (*_textPtr) {
		*_textPtr = ~(*_textPtr);
		_textPtr++;
	}

	_textPtr++;
	return _textPtr;
}

void TrecisionEngine::setRoom(uint16 r, bool b) {
	_logicMgr->setupAltRoom(r, b);
}

/*-------------------------------------------------
 * Compute string length from character begin to end
 *-------------------------------------------------*/
uint16 TrecisionEngine::textLength(const Common::String &text, uint16 begin, uint16 end) {
	if (text.empty())
		return 0;

	if (end == 0)
		end = text.size();

	uint16 retVal = 0;
	for (uint16 c = begin; c < end; c++)
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
	for (int8 a = MAXOBJINROOM - 1; a >= 0; a--) {
		uint16 checkedObj = _room[_curRoom]._object[a];
		Common::Rect lim = _obj[checkedObj]._lim;
		lim.translate(0, TOP);
		// Trecision includes the bottom and right coordinates
		lim.right++;
		lim.bottom++;

		if (checkedObj && isObjectVisible(checkedObj)) {
			if (lim.contains(pos)) {

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
						while (e < _obj[checkedObj]._rect.width()) {
							if (!insideObj) { // not inside an object
								if (b + TOP == pos.y) {
									if ((pos.x >= d + e) && (pos.x < d + e + *mask)) {
										_curObj = 0;
									}
								}

								e += *mask;
								mask++;
								insideObj = true;
							} else { // inside an object
								if (b + TOP == pos.y) {
									if ((pos.x >= d + e) && (pos.x < d + e + *mask)) {
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

void TrecisionEngine::openSys() {
	delete _actor;
	_actor = new Actor(this);
	_actor->initTextures();
	_actor->readModel("jm.om");

	_actor->_light = (SLight *)&_lightArea;
	_actor->_camera = (SCamera *)&_cameraArea;

	_textArea = new char[MAXTEXTAREA];

	_graphicsMgr->clearScreen();
	_graphicsMgr->hideCursor();
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

void TrecisionEngine::processTime() {
	static uint8 OldRegInvSI = 0xFF;
	static uint8 OldRegInvSL = 0xFF;
	static uint8 OldLightIcon = 0xFF;

	_curTime = readTime();
	_animMgr->refreshAllAnimations();

	if (_curTime >= _nextRefresh) {
		_textMgr->drawTexts();

		if (_inventoryStatus == INV_PAINT || _inventoryStatus == INV_DEPAINT)
			rollInventory(_inventoryStatus);

		if (_inventoryStatus != INV_OFF && (OldRegInvSI != _inventoryRefreshStartIcon || OldRegInvSL != _inventoryRefreshStartLine || OldLightIcon != _lightIcon)) {
			refreshInventory(_inventoryRefreshStartIcon, _inventoryRefreshStartLine);
			OldRegInvSI = _inventoryRefreshStartIcon;
			OldRegInvSL = _inventoryRefreshStartLine;
			OldLightIcon = _lightIcon;
		}

		_renderer->paintScreen(false);
		_textMgr->clearTextStack();

		uint32 paintTime = readTime();
		if (paintTime - _curTime >= 5)
			_nextRefresh = paintTime + 1;
		else
			_nextRefresh = _curTime + 5;
	}
}

void TrecisionEngine::processMouse() {
	static bool maskMouse;
	static Common::Point oldMousePos;
	static bool lastMouseOn = true;
	int16 mx = _mousePos.x;
	int16 my = _mousePos.y;

	if (lastMouseOn && !_graphicsMgr->isCursorVisible()) {
		oldMousePos = Common::Point(0, 0); // Switch off
	} else if (!lastMouseOn && _graphicsMgr->isCursorVisible()) {
		oldMousePos = Common::Point(0, 0); // Switch on
	}

	lastMouseOn = _graphicsMgr->isCursorVisible();
	checkSystem();

	if (!_graphicsMgr->isCursorVisible())
		return;

	if (_mouseLeftBtn || _mouseRightBtn) {
		if (!maskMouse) {
			_scheduler->doEvent(MC_MOUSE, _mouseRightBtn ? ME_MRIGHT : ME_MLEFT, MP_DEFAULT, mx, my, 0, 0);
			maskMouse = true;
		}
	} else {
		maskMouse = false;

		if (!_flagscriptactive && (mx != oldMousePos.x || my != oldMousePos.y)) {
			_scheduler->doEvent(MC_MOUSE, ME_MMOVE, MP_DEFAULT, mx, my, 0, 0);
			oldMousePos = Common::Point(mx, my);
		}
	}
}

/*------------------------------------------------
		Fake distance between two 2D points
--------------------------------------------------*/
float TrecisionEngine::dist2D(float x1, float y1, float x2, float y2) {
	double d1 = fabs((double)(x1 - x2));
	double d2 = fabs((double)(y1 - y2));

	return (float)sqrt(d1 * d1 + d2 * d2);
}

/*------------------------------------------------
			Distance between two 3D points
--------------------------------------------------*/
float TrecisionEngine::dist3D(float x1, float y1, float z1, float x2, float y2, float z2) {
	double d1 = fabs((double)(x1 - x2));
	double d2 = fabs((double)(y1 - y2));
	double d3 = fabs((double)(z1 - z2));
	return (float)sqrt(d1 * d1 + d2 * d2 + d3 * d3);
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
	for (uint16 index = 0; index < MAXOBJINROOM; index++) {
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
void SDText::set(SDText org) {
	set(org._rect, org._subtitleRect, org.tcol, org.scol, org.text);
}

void SDText::set(Common::Rect rect, Common::Rect subtitleRect, uint16 ptcol, uint16 pscol, const Common::String &pText) {
	_rect = rect;
	_subtitleRect = subtitleRect;
	tcol = ptcol;
	scol = pscol;
	text = pText;
}

/*-------------------------------------------------------------
   checkDText - Computes and returns the dy of the given DText
--------------------------------------------------------------*/
uint16 SDText::checkDText() {
	if (text.empty())
		return 0;

	uint8 curLine = 0;
	if (g_vm->textLength(text) <= _rect.width()) {
		strcpy((char *)g_vm->_drawTextLines[curLine], text.c_str());
		return CARHEI;
	}

	uint16 a = 0;
	uint16 tmpDy = 0;
	uint16 lastSpace = 0;
	uint16 curInit = 0;

	while (a < text.size()) {
		a++;
		if (a < text.size() && text[a] == ' ') {
			if (g_vm->textLength(text, curInit, a) <= _rect.width())
				lastSpace = a;
			else if (g_vm->textLength(text, curInit, lastSpace) <= _rect.width()) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					g_vm->_drawTextLines[curLine][b - curInit] = text[b];

				g_vm->_drawTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;

				tmpDy += CARHEI;
				a = curInit;
			} else
				return 0;
		} else if (a == text.size()) {
			if (g_vm->textLength(text, curInit, a) <= _rect.width()) {
				uint16 b;
				for (b = curInit; b < a; b++)
					g_vm->_drawTextLines[curLine][b - curInit] = text[b];
				g_vm->_drawTextLines[curLine][b - curInit] = '\0';

				tmpDy += CARHEI;

				return tmpDy;
			}

			if (g_vm->textLength(text, curInit, lastSpace) <= _rect.width()) {
				uint16 b;
				for (b = curInit; b < lastSpace; b++)
					g_vm->_drawTextLines[curLine][b - curInit] = text[b];

				g_vm->_drawTextLines[curLine][b - curInit] = '\0';
				curLine++;

				curInit = lastSpace + 1;
				tmpDy += CARHEI;

				if (curInit < text.size()) {
					for (b = curInit; b < text.size(); b++)
						g_vm->_drawTextLines[curLine][b - curInit] = text[b];

					g_vm->_drawTextLines[curLine][b - curInit] = '\0';

					tmpDy += CARHEI;
				}
				return tmpDy;
			}
			return 0;
		}
	}
	return 0;
}

void SDText::DText(uint16 *frameBuffer) {
	uint16 tmpTCol = tcol;
	uint16 tmpSCol = scol;
	g_vm->_graphicsMgr->updatePixelFormat(&tmpTCol, 1);
	if (scol != MASKCOL)
		g_vm->_graphicsMgr->updatePixelFormat(&tmpSCol, 1);

	if (text.empty())
		return;

	uint16 *buffer = (frameBuffer == nullptr) ? g_vm->_graphicsMgr->getScreenBufferPtr() : frameBuffer;
	uint16 curDy = checkDText();

	for (uint16 b = 0; b < (curDy / CARHEI); b++) {
		char *curText = (char *)g_vm->_drawTextLines[b];
		uint16 inc = (_rect.width() - g_vm->textLength(curText)) / 2;
		uint16 len = strlen(curText);

		if (len >= MAXCHARS) {
			strcpy(curText, g_vm->_sysText[kMessageError]);
			len = strlen(curText);
		}

		for (uint16 c = 0; c < len; c++) {
			byte curChar = curText[c]; /* reads the first part of the font */

			const uint16 charOffset = g_vm->_font[curChar * 3] + (uint16)(g_vm->_font[curChar * 3 + 1] << 8);
			uint16 fontDataOffset = 768;
			const uint16 charWidth = g_vm->_font[curChar * 3 + 2];

			if (c == len - 1 && g_vm->_blinkLastDTextChar != MASKCOL)
				tmpTCol = g_vm->_blinkLastDTextChar;

			for (uint16 a = b * CARHEI; a < (b + 1) * CARHEI; a++) {
				uint16 curPos = 0;
				uint16 CurColor = tmpSCol;

				while (curPos <= charWidth - 1) {
					if (a >= _subtitleRect.top && a < _subtitleRect.bottom) {
						if (CurColor != MASKCOL && (g_vm->_font[charOffset + fontDataOffset])) {
							const uint16 charLeft = inc + curPos;
							const uint16 charRight = charLeft + g_vm->_font[charOffset + fontDataOffset];
							uint16 *dst1 = buffer + _rect.left + charLeft + (_rect.top + a) * MAXX;
							uint16 *dst2 = buffer + _rect.left + _subtitleRect.left + (_rect.top + a) * MAXX;
							uint16 *dst = nullptr;
							uint16 size = 0;

							if (charLeft >= _subtitleRect.left && charRight < _subtitleRect.right) {
								dst = dst1;
								size = charRight - charLeft;
							} else if (charLeft < _subtitleRect.left && charRight < _subtitleRect.right && charRight > _subtitleRect.left) {
								dst = dst2;
								size = charRight - _subtitleRect.left;
							} else if (charLeft >= _subtitleRect.left && charRight >= _subtitleRect.right && _subtitleRect.right > charLeft) {
								dst = dst1;
								size = _subtitleRect.right - charLeft;
							} else if (charLeft < _subtitleRect.left && charRight >= _subtitleRect.right && _subtitleRect.right > charLeft) {
								dst = dst2;
								size = _subtitleRect.right - _subtitleRect.left;
							}

							if (dst && size > 0) {
								uint16 *d = dst;
								for (uint32 i = 0; i < size; i++)
									*d++ = CurColor;
							}
						}
					}

					curPos += g_vm->_font[charOffset + fontDataOffset];
					fontDataOffset++;

					if (CurColor == tmpSCol)
						CurColor = 0;
					else if (CurColor == 0)
						CurColor = tmpTCol;
					else if (CurColor == tmpTCol)
						CurColor = tmpSCol;
				}
			}
			inc += charWidth;
		}
	}
}

} // End of namespace Trecision
