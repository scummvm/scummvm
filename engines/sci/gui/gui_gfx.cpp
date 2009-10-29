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
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_animate.h"
#include "sci/gui/gui_font.h"
#include "sci/gui/gui_picture.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_text.h"

namespace Sci {

SciGuiGfx::SciGuiGfx(EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette)
	: _s(state), _screen(screen), _palette(palette) {
}

SciGuiGfx::~SciGuiGfx() {
	delete _mainPort;
	delete _menuPort;
}

void SciGuiGfx::init(SciGuiText *text) {
	_text = text;

	// _mainPort is not known to windowmanager, that's okay according to sierra sci
	//  its not even used currently in our engine
	_mainPort = new GuiPort(0);
	SetPort(_mainPort);
	OpenPort(_mainPort);

	// _menuPort has actually hardcoded id 0xFFFF. Its not meant to be known to windowmanager according to sierra sci
	_menuPort = new GuiPort(0xFFFF);
	OpenPort(_menuPort);
	_text->SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->_width, _screen->_height);
	_menuRect = Common::Rect(0, 0, _screen->_width, 9);
}

GuiPort *SciGuiGfx::SetPort(GuiPort *newPort) {
	GuiPort *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

GuiPort *SciGuiGfx::GetPort(void) {
	return _curPort;
}

void SciGuiGfx::SetOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void SciGuiGfx::MoveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void SciGuiGfx::Move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

void SciGuiGfx::OpenPort(GuiPort *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	GuiPort *tmp = _curPort;
	_curPort = port;
	_text->SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->textFace = 0;
	port->penClr = 0;
	port->backClr = 255;
	port->penMode = 0;
	port->rect = _bounds;
}

void SciGuiGfx::PenColor(int16 color) {
	_curPort->penClr = color;
}

void SciGuiGfx::BackColor(int16 color) {
	_curPort->backClr = color;
}

void SciGuiGfx::PenMode(int16 mode) {
	_curPort->penMode = mode;
}

void SciGuiGfx::TextFace(int16 textFace) {
	_curPort->textFace = textFace;
}

int16 SciGuiGfx::GetPointSize(void) {
	return _curPort->fontHeight;
}

void SciGuiGfx::ClearScreen(byte color) {
	FillRect(_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void SciGuiGfx::InvertRect(const Common::Rect &rect) {
	int16 oldpenmode = _curPort->penMode;
	_curPort->penMode = 2;
	FillRect(rect, 1, _curPort->penClr, _curPort->backClr);
	_curPort->penMode = oldpenmode;
}

void SciGuiGfx::EraseRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->backClr);
}

void SciGuiGfx::PaintRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->penClr);
}

void SciGuiGfx::FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
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

void SciGuiGfx::FrameRect(const Common::Rect &rect) {
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

void SciGuiGfx::OffsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

void SciGuiGfx::OffsetLine(Common::Point &start, Common::Point &end) {
	start.x += _curPort->left;
	start.y += _curPort->top;
	end.x += _curPort->left;
	end.y += _curPort->top;
}

void SciGuiGfx::BitsShow(const Common::Rect &rect) {
	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_curPort->rect);
	if (workerRect.isEmpty()) // nothing to show
		return;

	OffsetRect(workerRect);
	_screen->copyRectToScreen(workerRect);
}

GuiMemoryHandle SciGuiGfx::BitsSave(const Common::Rect &rect, byte screenMask) {
	GuiMemoryHandle memoryId;
	byte *memoryPtr;
	int size;

	Common::Rect workerRect(rect.left, rect.top, rect.right, rect.bottom);
	workerRect.clip(_curPort->rect);
	if (workerRect.isEmpty()) // nothing to save
		return NULL_REG;

	OffsetRect(workerRect);

	// now actually ask _screen how much space it will need for saving
	size = _screen->bitsGetDataSize(workerRect, screenMask);

	memoryId = kalloc(_s->_segMan, "SaveBits()", size);
	memoryPtr = kmem(_s->_segMan, memoryId);
	_screen->bitsSave(workerRect, screenMask, memoryPtr);
	return memoryId;
}

void SciGuiGfx::BitsGetRect(GuiMemoryHandle memoryHandle, Common::Rect *destRect) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_s->_segMan, memoryHandle);;

		if (memoryPtr) {
			_screen->bitsGetRect(memoryPtr, destRect);
		}
	}
}

void SciGuiGfx::BitsRestore(GuiMemoryHandle memoryHandle) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_s->_segMan, memoryHandle);;

		if (memoryPtr) {
			_screen->bitsRestore(memoryPtr);
			kfree(_s->_segMan, memoryHandle);
		}
	}
}

void SciGuiGfx::BitsFree(GuiMemoryHandle memoryHandle) {
	if (!memoryHandle.isNull()) {
		kfree(_s->_segMan, memoryHandle);
	}
}

void SciGuiGfx::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId) {
	SciGuiPicture *picture;

	picture = new SciGuiPicture(_s->resMan, this, _screen, _palette, pictureId);
	// do we add to a picture? if not -> clear screen
	if (!addToFlag) {
		ClearScreen(_screen->_colorClearScreen);
	}
	picture->draw(animationNr, mirroredFlag, addToFlag, paletteId);
}

// This one is the only one that updates screen!
void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	Common::Rect rect;
	Common::Rect clipRect;
	if (view) {
		rect.left = leftPos;
		rect.top = topPos;
		rect.right = rect.left + view->getWidth(loopNo, celNo);
		rect.bottom = rect.top + view->getHeight(loopNo, celNo);
		clipRect = rect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) {	// nothing to draw
			delete view;
			return;
		}

		Common::Rect clipRectTranslated = clipRect;
		OffsetRect(clipRectTranslated);
		view->draw(rect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);
		if (!_screen->_picNotValid)
			BitsShow(rect);
	}

	delete view;
}

// This version of drawCel is not supposed to call BitsShow()!
void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	Common::Rect clipRect;
	if (view) {
		clipRect = celRect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) { // nothing to draw
			delete view;
			return;
		}

		Common::Rect clipRectTranslated = clipRect;
		OffsetRect(clipRectTranslated);
		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);
	}

	delete view;
}

// This version of drawCel is not supposed to call BitsShow()!
void SciGuiGfx::drawCel(SciGuiView *view, GuiViewLoopNo loopNo, GuiViewCelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo) {
	Common::Rect clipRect;
	clipRect = celRect;
	clipRect.clip(_curPort->rect);
	if (clipRect.isEmpty()) // nothing to draw
		return;

	Common::Rect clipRectTranslated = clipRect;
	OffsetRect(clipRectTranslated);
	view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);
}

uint16 SciGuiGfx::onControl(uint16 screenMask, Common::Rect rect) {
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

void SciGuiGfx::PriorityBandsInit(int16 bandCount, int16 top, int16 bottom) {
	int16 y;
	int32 bandSize;

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
	// We fill space that is left over with the highest band
	for (y = _priorityBottom; y < _screen->_height; y++)
		_priorityBands[y] = _priorityBandCount;
}

void SciGuiGfx::PriorityBandsInit(byte *data) {
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

byte SciGuiGfx::CoordinateToPriority(int16 y) {
	if (y < _priorityTop) 
		return _priorityBands[_priorityTop];
	if (y > _priorityBottom)
		return _priorityBands[_priorityBottom];
	return _priorityBands[y];
}

int16 SciGuiGfx::PriorityToCoordinate(byte priority) {
	int16 y;
	if (priority <= _priorityBandCount) {
		for (y = 0; y <= _priorityBottom; y++)
			if (_priorityBands[y] == priority)
				return y;
	}
	return _priorityBottom;
}

bool SciGuiGfx::CanBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = GET_SEL32V(segMan, curObject, signal);
			if ((signal & (kSignalIgnoreActor | kSignalRemoveView | kSignalNoUpdate)) == 0) {
				curRect.left = GET_SEL32V(segMan, curObject, brLeft);
				curRect.top = GET_SEL32V(segMan, curObject, brTop);
				curRect.right = GET_SEL32V(segMan, curObject, brRight);
				curRect.bottom = GET_SEL32V(segMan, curObject, brBottom);
				// Check if curRect is within checkRect
				if (curRect.right > checkRect.left && curRect.left < checkRect.right && curRect.bottom > checkRect.top && curRect.top < checkRect.bottom) {
					return false;
				}
			}
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
	return true;
}

void SciGuiGfx::SetNowSeen(reg_t objectReference) {
	SegManager *segMan = _s->_segMan;
	SciGuiView *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(segMan, objectReference, view);
	GuiViewLoopNo loopNo = sign_extend_byte((GuiViewLoopNo)GET_SEL32V(segMan, objectReference, loop));
	GuiViewCelNo celNo = sign_extend_byte((GuiViewCelNo)GET_SEL32V(segMan, objectReference, cel));
	int16 x = (int16)GET_SEL32V(segMan, objectReference, x);
	int16 y = (int16)GET_SEL32V(segMan, objectReference, y);
	int16 z = 0;
	if (_s->_kernel->_selectorCache.z > -1) {
		z = (int16)GET_SEL32V(segMan, objectReference, z);
	}

	// now get cel rectangle
	view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_s->_segMan, objectReference, _s->_kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(segMan, objectReference, nsLeft, celRect.left);
		PUT_SEL32V(segMan, objectReference, nsRight, celRect.right);
		PUT_SEL32V(segMan, objectReference, nsTop, celRect.top);
		PUT_SEL32V(segMan, objectReference, nsBottom, celRect.bottom);
	}

	delete view;
}

} // End of namespace Sci
