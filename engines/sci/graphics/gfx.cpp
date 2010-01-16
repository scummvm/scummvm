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

#include "common/util.h"
#include "common/stack.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/graphics/gfx.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/font.h"
#include "sci/graphics/picture.h"
#include "sci/graphics/view.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/text.h"

namespace Sci {

Gfx::Gfx(ResourceManager *resMan, SegManager *segMan, Kernel *kernel, Screen *screen, SciPalette *palette)
	: _resMan(resMan), _segMan(segMan), _kernel(kernel), _screen(screen), _palette(palette) {
}

Gfx::~Gfx() {
	purgeCache();

	delete _mainPort;
	delete _menuPort;
}

void Gfx::init(Text *text) {
	_text = text;

	// _mainPort is not known to windowmanager, that's okay according to sierra sci
	//  its not even used currently in our engine
	_mainPort = new Port(0);
	SetPort(_mainPort);
	OpenPort(_mainPort);

	// _menuPort has actually hardcoded id 0xFFFF. Its not meant to be known to windowmanager according to sierra sci
	_menuPort = new Port(0xFFFF);
	OpenPort(_menuPort);
	_text->SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->getWidth(), _screen->getHeight());
	_menuBarRect = Common::Rect(0, 0, _screen->getWidth(), 9);

	_EGAdrawingVisualize = false;
}

void Gfx::purgeCache() {
	for (ViewCache::iterator iter = _cachedViews.begin(); iter != _cachedViews.end(); ++iter) {
		delete iter->_value;
		iter->_value = 0;
	}

	_cachedViews.clear();
}

View *Gfx::getView(GuiResourceId viewNum) {
	if (_cachedViews.size() >= MAX_CACHED_VIEWS)
		purgeCache();

	if (!_cachedViews.contains(viewNum))
		_cachedViews[viewNum] = new View(_resMan, _screen, _palette, viewNum);

	return _cachedViews[viewNum];
}

Port *Gfx::SetPort(Port *newPort) {
	Port *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

Port *Gfx::GetPort() {
	return _curPort;
}

void Gfx::SetOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void Gfx::MoveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void Gfx::Move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

void Gfx::OpenPort(Port *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	Port *tmp = _curPort;
	_curPort = port;
	_text->SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->greyedOutput = false;
	port->penClr = 0;
	port->backClr = 255;
	port->penMode = 0;
	port->rect = _bounds;
}

void Gfx::PenColor(int16 color) {
	_curPort->penClr = color;
}

void Gfx::BackColor(int16 color) {
	_curPort->backClr = color;
}

void Gfx::PenMode(int16 mode) {
	_curPort->penMode = mode;
}

void Gfx::TextGreyedOutput(bool state) {
	_curPort->greyedOutput = state;
}

int16 Gfx::GetPointSize() {
	return _curPort->fontHeight;
}

void Gfx::ClearScreen(byte color) {
	FillRect(_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void Gfx::InvertRect(const Common::Rect &rect) {
	int16 oldpenmode = _curPort->penMode;
	_curPort->penMode = 2;
	FillRect(rect, 1, _curPort->penClr, _curPort->backClr);
	_curPort->penMode = oldpenmode;
}

void Gfx::EraseRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->backClr);
}

void Gfx::PaintRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->penClr);
}

void Gfx::FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
	Common::Rect r = rect;
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to fill
		return;

	int16 oldPenMode = _curPort->penMode;
	OffsetRect(r);
	int16 x, y;
	byte curVisual;

	// Doing visual first
	if (drawFlags & SCI_SCREEN_MASK_VISUAL) {
		if (oldPenMode == 2) { // invert mode
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					curVisual = _screen->getVisual(x, y);
					if (curVisual == clrPen) {
						_screen->putPixel(x, y, 1, clrBack, 0, 0);
					} else if (curVisual == clrBack) {
						_screen->putPixel(x, y, 1, clrPen, 0, 0);
					}
				}
			}
		} else { // just fill rect with ClrPen
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					_screen->putPixel(x, y, 1, clrPen, 0, 0);
				}
			}
		}
	}

	if (drawFlags < 2)
		return;
	drawFlags &= SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_CONTROL;

	if (oldPenMode != 2) {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, clrBack, bControl);
			}
		}
	} else {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, !_screen->getPriority(x, y), !_screen->getControl(x, y));
			}
		}
	}
}

void Gfx::FrameRect(const Common::Rect &rect) {
	Common::Rect r;
	// left
	r = rect;
	r.right = rect.left + 1;
	PaintRect(r);
	// right
	r.right = rect.right;
	r.left = rect.right - 1;
	PaintRect(r);
	//top
	r.left = rect.left;
	r.bottom = rect.top + 1;
	PaintRect(r);
	//bottom
	r.bottom = rect.bottom;
	r.top = rect.bottom - 1;
	PaintRect(r);
}

void Gfx::OffsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

void Gfx::OffsetLine(Common::Point &start, Common::Point &end) {
	start.x += _curPort->left;
	start.y += _curPort->top;
	end.x += _curPort->left;
	end.y += _curPort->top;
}

void Gfx::BitsShow(const Common::Rect &rect) {
	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_curPort->rect);
	if (workerRect.isEmpty()) // nothing to show
		return;

	OffsetRect(workerRect);
	_screen->copyRectToScreen(workerRect);
}

void Gfx::BitsShowHires(const Common::Rect &rect) {
	_screen->copyDisplayRectToScreen(rect);
}

reg_t Gfx::BitsSave(const Common::Rect &rect, byte screenMask) {
	reg_t memoryId;
	byte *memoryPtr;
	int size;

	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_curPort->rect);
	if (workerRect.isEmpty()) // nothing to save
		return NULL_REG;

	if (screenMask == SCI_SCREEN_MASK_DISPLAY) {
		// Adjust rect to upscaled hires, but dont adjust according to port
		workerRect.top *= 2; workerRect.bottom *= 2; workerRect.bottom++;
		workerRect.left *= 2; workerRect.right *= 2; workerRect.right++;
	} else {
		OffsetRect(workerRect);
	}

	// now actually ask _screen how much space it will need for saving
	size = _screen->bitsGetDataSize(workerRect, screenMask);

	memoryId = kalloc(_segMan, "SaveBits()", size);
	memoryPtr = kmem(_segMan, memoryId);
	_screen->bitsSave(workerRect, screenMask, memoryPtr);
	return memoryId;
}

void Gfx::BitsGetRect(reg_t memoryHandle, Common::Rect *destRect) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_segMan, memoryHandle);

		if (memoryPtr) {
			_screen->bitsGetRect(memoryPtr, destRect);
		}
	}
}

void Gfx::BitsRestore(reg_t memoryHandle) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_segMan, memoryHandle);

		if (memoryPtr) {
			_screen->bitsRestore(memoryPtr);
			kfree(_segMan, memoryHandle);
		}
	}
}

void Gfx::BitsFree(reg_t memoryHandle) {
	if (!memoryHandle.isNull()) {
		kfree(_segMan, memoryHandle);
	}
}

void Gfx::setEGAdrawingVisualize(bool state) {
	_EGAdrawingVisualize = state;
}

void Gfx::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId) {
	SciGuiPicture *picture = new SciGuiPicture(_resMan, this, _screen, _palette, pictureId, _EGAdrawingVisualize);

	// do we add to a picture? if not -> clear screen with white
	if (!addToFlag)
		ClearScreen(_screen->getColorWhite());

	picture->draw(animationNr, mirroredFlag, addToFlag, paletteId);
	delete picture;
}

// This one is the only one that updates screen!
void Gfx::drawCelAndShow(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	View *view = getView(viewId);
	Common::Rect celRect;
	
	if (view) {
		celRect.left = leftPos;
		celRect.top = topPos;
		celRect.right = celRect.left + view->getWidth(loopNo, celNo);
		celRect.bottom = celRect.top + view->getHeight(loopNo, celNo);

		drawCel(view, loopNo, celNo, celRect, priority, paletteNo, scaleX, scaleY);

		if (getSciVersion() >= SCI_VERSION_1_1) {
			if (!_screen->_picNotValidSci11) {
				BitsShow(celRect);
			}
		} else {
			if (!_screen->_picNotValid)
				BitsShow(celRect);
		}
	}
}

// This version of drawCel is not supposed to call BitsShow()!
void Gfx::drawCel(GuiResourceId viewId, int16 loopNo, int16 celNo, Common::Rect celRect, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	drawCel(getView(viewId), loopNo, celNo, celRect, priority, paletteNo, scaleX, scaleY);
}

// This version of drawCel is not supposed to call BitsShow()!
void Gfx::drawCel(View *view, int16 loopNo, int16 celNo, Common::Rect celRect, byte priority, uint16 paletteNo, uint16 scaleX, uint16 scaleY) {
	Common::Rect clipRect = celRect;
	clipRect.clip(_curPort->rect);
	if (clipRect.isEmpty()) // nothing to draw
		return;

	Common::Rect clipRectTranslated = clipRect;
	OffsetRect(clipRectTranslated);
	if (scaleX == 128 && scaleY == 128) {
		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo, false);
	} else {
		view->drawScaled(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, scaleX, scaleY);
	}
}

// This is used as replacement for drawCelAndShow() when hires-cels are drawn to screen
//  Hires-cels are available only SCI 1.1+
void Gfx::drawHiresCelAndShow(GuiResourceId viewId, int16 loopNo, int16 celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo, reg_t upscaledHiresHandle, uint16 scaleX, uint16 scaleY) {
	View *view = getView(viewId);
	Common::Rect celRect, curPortRect, clipRect, clipRectTranslated;
	Common::Point curPortPos;
	bool upscaledHiresHack = false;
	
	if (view) {
		if ((leftPos == 0) && (topPos == 0)) {
			// HACK: in kq6, we get leftPos&topPos == 0 SOMETIMES, that's why we need to get coordinates from upscaledHiresHandle
			//  I'm not sure if this is what we are supposed to do or if there is some other bug that actually makes
			//  coordinates to be 0 in the first place
			byte *memoryPtr = NULL;
			memoryPtr = kmem(_segMan, upscaledHiresHandle);
			if (memoryPtr) {
				Common::Rect upscaledHiresRect;
				_screen->bitsGetRect(memoryPtr, &upscaledHiresRect);
				leftPos = upscaledHiresRect.left;
				topPos = upscaledHiresRect.top;
				upscaledHiresHack = true;
			}
		}

		celRect.left = leftPos;
		celRect.top = topPos;
		celRect.right = celRect.left + view->getWidth(loopNo, celNo);
		celRect.bottom = celRect.top + view->getHeight(loopNo, celNo);
		// adjust curPort to upscaled hires
		clipRect = celRect;
		curPortRect = _curPort->rect;
		curPortRect.top *= 2; curPortRect.bottom *= 2; curPortRect.bottom++;
		curPortRect.left *= 2; curPortRect.right *= 2; curPortRect.right++;
		clipRect.clip(curPortRect);
		if (clipRect.isEmpty()) // nothing to draw
			return;

		clipRectTranslated = clipRect;
		if (!upscaledHiresHack) {
			curPortPos.x = _curPort->left * 2; curPortPos.y = _curPort->top * 2;
			clipRectTranslated.top += curPortPos.y; clipRectTranslated.bottom += curPortPos.y;
			clipRectTranslated.left += curPortPos.x; clipRectTranslated.right += curPortPos.x;
		}

		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo, true);
		if (!_screen->_picNotValidSci11) {
			_screen->copyDisplayRectToScreen(clipRectTranslated);
		}
	}
}

uint16 Gfx::onControl(uint16 screenMask, Common::Rect rect) {
	Common::Rect outRect(rect.left, rect.top, rect.right, rect.bottom);
	int16 x, y;
	uint16 result = 0;

	outRect.clip(_curPort->rect);
	if (outRect.isEmpty()) // nothing to control
		return 0;
	OffsetRect(outRect);

	if (screenMask & SCI_SCREEN_MASK_PRIORITY) {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getPriority(x, y);
			}
		}
	} else {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getControl(x, y);
			}
		}
	}
	return result;
}

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

void Gfx::PriorityBandsInit(int16 bandCount, int16 top, int16 bottom) {
	int16 y;
	int32 bandSize;

	// This code is for 320x200 games only
	if (_screen->getHeight() != 200)
		return;

	if (bandCount != -1)
		_priorityBandCount = bandCount;

	_priorityTop = top;
	_priorityBottom = bottom;

	// Do NOT modify this algo or optimize it anyhow, sierra sci used int32 for calculating the
	//  priority bands and by using double or anything rounding WILL destroy the result
	bandSize = ((_priorityBottom - _priorityTop) * 2000) / _priorityBandCount;

	memset(_priorityBands, 0, sizeof(byte) * _priorityTop);
	for (y = _priorityTop; y < _priorityBottom; y++)
		_priorityBands[y] = 1 + (((y - _priorityTop) * 2000) / bandSize);
	if (_priorityBandCount == 15) {
		// When having 15 priority bands, we actually replace band 15 with band 14, cause the original sci interpreter also
		//  does it that way as well
		y = _priorityBottom;
		while (_priorityBands[--y] == _priorityBandCount)
			_priorityBands[y]--;
	}
	// We fill space that is left over with the highest band (hardcoded 200 limit, because this algo isnt meant to be used on hires)
	for (y = _priorityBottom; y < 200; y++)
		_priorityBands[y] = _priorityBandCount;
}

void Gfx::PriorityBandsInit(byte *data) {
	int i = 0, inx;
	byte priority = 0;

	for (inx = 0; inx < 14; inx++) {
		priority = *data++;
		while (i < priority)
			_priorityBands[i++] = inx;
	}
	while (i < 200)
		_priorityBands[i++] = inx;
}

byte Gfx::CoordinateToPriority(int16 y) {
	if (y < _priorityTop) 
		return _priorityBands[_priorityTop];
	if (y > _priorityBottom)
		return _priorityBands[_priorityBottom];
	return _priorityBands[y];
}

int16 Gfx::PriorityToCoordinate(byte priority) {
	int16 y;
	if (priority <= _priorityBandCount) {
		for (y = 0; y <= _priorityBottom; y++)
			if (_priorityBands[y] == priority)
				return y;
	}
	return _priorityBottom;
}

bool Gfx::CanBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list) {
	reg_t curAddress = list->first;
	Node *curNode = _segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = GET_SEL32V(_segMan, curObject, signal);
			if ((signal & (kSignalIgnoreActor | kSignalRemoveView | kSignalNoUpdate)) == 0) {
				curRect.left = GET_SEL32V(_segMan, curObject, brLeft);
				curRect.top = GET_SEL32V(_segMan, curObject, brTop);
				curRect.right = GET_SEL32V(_segMan, curObject, brRight);
				curRect.bottom = GET_SEL32V(_segMan, curObject, brBottom);
				// Check if curRect is within checkRect
				if (curRect.right > checkRect.left && curRect.left < checkRect.right && curRect.bottom > checkRect.top && curRect.top < checkRect.bottom) {
					return false;
				}
			}
		}
		curAddress = curNode->succ;
		curNode = _segMan->lookupNode(curAddress);
	}
	return true;
}

void Gfx::SetNowSeen(reg_t objectReference) {
	View *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(_segMan, objectReference, view);
	int16 loopNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, loop));
	int16 celNo = sign_extend_byte((int16)GET_SEL32V(_segMan, objectReference, cel));
	int16 x = (int16)GET_SEL32V(_segMan, objectReference, x);
	int16 y = (int16)GET_SEL32V(_segMan, objectReference, y);
	int16 z = 0;
	if (_kernel->_selectorCache.z > -1)
		z = (int16)GET_SEL32V(_segMan, objectReference, z);

	// now get cel rectangle
	view = getView(viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_segMan, objectReference, _kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(_segMan, objectReference, nsLeft, celRect.left);
		PUT_SEL32V(_segMan, objectReference, nsRight, celRect.right);
		PUT_SEL32V(_segMan, objectReference, nsTop, celRect.top);
		PUT_SEL32V(_segMan, objectReference, nsBottom, celRect.bottom);
	}
}

} // End of namespace Sci
