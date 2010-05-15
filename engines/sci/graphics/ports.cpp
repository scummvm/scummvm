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

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/ports.h"

namespace Sci {

// window styles
enum {
	SCI_WINDOWMGR_STYLE_TRANSPARENT = (1 << 0),
	SCI_WINDOWMGR_STYLE_NOFRAME     = (1 << 1),
	SCI_WINDOWMGR_STYLE_TITLE       = (1 << 2),
	SCI_WINDOWMGR_STYLE_TOPMOST     = (1 << 3),
	SCI_WINDOWMGR_STYLE_USER        = (1 << 7)
};

GfxPorts::GfxPorts(SegManager *segMan, GfxScreen *screen)
	: _segMan(segMan), _screen(screen) {
}

GfxPorts::~GfxPorts() {
	// TODO: Clear _windowList and delete all stuff in it?
	delete _menuPort;
}

void GfxPorts::init(bool usesOldGfxFunctions, SciGui *gui, GfxPaint16 *paint16, GfxText16 *text16, Common::String gameId) {
	int16 offTop = 10;

	_usesOldGfxFunctions = usesOldGfxFunctions;
	_gui = gui;
	_paint16 = paint16;
	_text16 = text16;

	// _menuPort has actually hardcoded id 0xFFFF. Its not meant to be known to windowmanager according to sierra sci
	_menuPort = new Port(0xFFFF);
	openPort(_menuPort);
	setPort(_menuPort);
	_text16->SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->getWidth(), _screen->getHeight());
	_menuBarRect = Common::Rect(0, 0, _screen->getWidth(), 9);
	_menuRect = Common::Rect(0, 0, _screen->getWidth(), 10);
	_menuLine = Common::Rect(0, 9, _screen->getWidth(), 10);

	_wmgrPort = new Port(1);
	_windowsById.resize(2);
	_windowsById[0] = _wmgrPort; // wmgrPort is supposed to be accessible via id 0
	_windowsById[1] = _wmgrPort; //  but wmgrPort may not actually have id 0, so we assign id 1 (as well)
	// Background: sierra sci replies with the offset of curPort on kGetPort calls. If we reply with 0 there most games
	//				will work, but some scripts seem to check for 0 and initialize the variable again in that case
	//				resulting in problems.

	if (getSciVersion() >= SCI_VERSION_1_LATE)
		_styleUser = SCI_WINDOWMGR_STYLE_USER;
	else
		_styleUser = SCI_WINDOWMGR_STYLE_USER | SCI_WINDOWMGR_STYLE_TRANSPARENT;

	// Jones, Slater and Hoyle 3 were called with parameter -Nw 0 0 200 320.
	// Mother Goose (SCI1) uses -Nw 0 0 159 262. The game will later use SetPort so we don't need to set the other fields.
	// This actually meant not skipping the first 10 pixellines in windowMgrPort
	if (gameId == "jones" || gameId == "slater" || gameId == "hoyle3" || (gameId == "mothergoose" && getSciVersion() == SCI_VERSION_1_EARLY))
		offTop = 0;

	openPort(_wmgrPort);
	setPort(_wmgrPort);
	// SCI0 games till kq4 (.502 - not including) did not adjust against _wmgrPort in kNewWindow
	//  We leave _wmgrPort top at 0, so the adjustment wont get done
	if (!g_sci->_features->usesOldGfxFunctions())
		setOrigin(0, offTop);
	_wmgrPort->rect.bottom = _screen->getHeight() - offTop;
	_wmgrPort->rect.right = _screen->getWidth();
	_wmgrPort->rect.moveTo(0, 0);
	_wmgrPort->curTop = 0;
	_wmgrPort->curLeft = 0;
	_windowList.push_front(_wmgrPort);

	_picWind = newWindow(Common::Rect(0, offTop, _screen->getWidth(), _screen->getHeight()), 0, 0, SCI_WINDOWMGR_STYLE_TRANSPARENT | SCI_WINDOWMGR_STYLE_NOFRAME, 0, true);
	// For SCI0 games till kq4 (.502 - not including) we set _picWind top to offTop instead
	//  Because of the menu/status bar
	if (g_sci->_features->usesOldGfxFunctions())
		_picWind->top = offTop;

	priorityBandsMemoryActive = false;

	kernelInitPriorityBands();
}

void GfxPorts::kernelSetActive(uint16 portId) {
	switch (portId) {
	case 0:
		setPort(_wmgrPort);
		break;
	case 0xFFFF:
		setPort(_menuPort);
		break;
	default:
		setPort(getPortById(portId));
	};
}

Common::Rect GfxPorts::kernelGetPicWindow(int16 &picTop, int16 &picLeft) {
	picTop = _picWind->top;
	picLeft = _picWind->left;
	return _picWind->rect;
}

void GfxPorts::kernelSetPicWindow(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag) {
	_picWind->rect = rect;
	_picWind->top = picTop;
	_picWind->left = picLeft;
	if (initPriorityBandsFlag)
		kernelInitPriorityBands();
}

reg_t GfxPorts::kernelGetActive() {
	return make_reg(0, getPort()->id);
}

reg_t GfxPorts::kernelNewWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title) {
	Window *wnd = NULL;

	if (restoreRect.top != 0 && restoreRect.left != 0 && restoreRect.height() != 0 && restoreRect.width() != 0)
		wnd = newWindow(dims, &restoreRect, title, style, priority, false);
	else
		wnd = newWindow(dims, NULL, title, style, priority, false);
	wnd->penClr = colorPen;
	wnd->backClr = colorBack;
	drawWindow(wnd);

	return make_reg(0, wnd->id);
}

void GfxPorts::kernelDisposeWindow(uint16 windowId, bool reanimate) {
	Window *wnd = (Window *)getPortById(windowId);
	disposeWindow(wnd, reanimate);
}

int16 GfxPorts::isFrontWindow(Window *pWnd) {
	return _windowList.back() == pWnd;
}

void GfxPorts::beginUpdate(Window *wnd) {
	Port *oldPort = setPort(_wmgrPort);
	PortList::iterator it = _windowList.reverse_begin();
	const PortList::iterator end = Common::find(_windowList.begin(), _windowList.end(), wnd);
	while (it != end) {
		// FIXME: We also store Port objects in the window list.
		// We should add a check that we really only pass windows here...
		updateWindow((Window *)*it);
		--it;
	}
	setPort(oldPort);
}

void GfxPorts::endUpdate(Window *wnd) {
	Port *oldPort = setPort(_wmgrPort);
	const PortList::iterator end = _windowList.end();
	PortList::iterator it = Common::find(_windowList.begin(), end, wnd);

	// wnd has to be in _windowList
	assert(it != end);

	while (++it != end) {
		// FIXME: We also store Port objects in the window list.
		// We should add a check that we really only pass windows here...
		updateWindow((Window *)*it);
	}

	setPort(oldPort);
}

Window *GfxPorts::newWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw) {
	// Find an unused window/port id
	uint id = 1;
	while (id < _windowsById.size() && _windowsById[id]) {
		++id;
	}
	if (id == _windowsById.size())
		_windowsById.push_back(0);
	assert(0 < id && id < 0xFFFF);

	Window *pwnd = new Window(id);
	Common::Rect r;

	if (!pwnd) {
		warning("Can't open window!");
		return 0;
	}

	_windowsById[id] = pwnd;
	if (style & SCI_WINDOWMGR_STYLE_TOPMOST)
		_windowList.push_front(pwnd);
	else
		_windowList.push_back(pwnd);
	openPort(pwnd);

	r = dims;
	if (r.width() > _screen->getWidth()) {
		// We get invalid dimensions at least at the end of sq3 (script bug!)
		warning("fixing too large window, given left&right was %d, %d", dims.left, dims.right);
		r.left = 0;
		r.right = _screen->getWidth() - 1;
		if ((style != _styleUser) && !(style & SCI_WINDOWMGR_STYLE_NOFRAME))
			r.right--;
	}
	pwnd->rect = r;
	if (restoreRect)
		pwnd->restoreRect = *restoreRect;

	pwnd->wndStyle = style;
	pwnd->hSaved1 = pwnd->hSaved2 = NULL_REG;
	pwnd->bDrawn = false;
	if ((style & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0)
		pwnd->saveScreenMask = (priority == -1 ? GFX_SCREEN_MASK_VISUAL : GFX_SCREEN_MASK_VISUAL | GFX_SCREEN_MASK_PRIORITY);

	if (title && (style & SCI_WINDOWMGR_STYLE_TITLE)) {
		pwnd->title = title;
	}

	r = pwnd->rect;
	if ((style != _styleUser) && !(style & SCI_WINDOWMGR_STYLE_NOFRAME)) {
		r.grow(1);
		if (style & SCI_WINDOWMGR_STYLE_TITLE) {
			r.top -= 10;
			r.bottom++;
		}
	}

	pwnd->dims = r;
	const Common::Rect *wmprect = &_wmgrPort->rect;
	int16 oldtop = pwnd->dims.top;
	int16 oldleft = pwnd->dims.left;
	if (wmprect->top > pwnd->dims.top)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->top);

	if (wmprect->bottom < pwnd->dims.bottom)
		pwnd->dims.moveTo(pwnd->dims.left, wmprect->bottom - pwnd->dims.bottom + pwnd->dims.top);

	if (wmprect->right < pwnd->dims.right)
		pwnd->dims.moveTo(wmprect->right + pwnd->dims.left - pwnd->dims.right, pwnd->dims.top);

	if (wmprect->left > pwnd->dims.left)
		pwnd->dims.moveTo(wmprect->left, pwnd->dims.top);

	pwnd->rect.moveTo(pwnd->rect.left + pwnd->dims.left - oldleft, pwnd->rect.top + pwnd->dims.top - oldtop);
	if (restoreRect == 0)
		pwnd->restoreRect = pwnd->dims;

	if (draw)
		drawWindow(pwnd);
	setPort((Port *)pwnd);
	// All SCI0 games till kq4 .502 (not including) did not adjust against _wmgrPort, we set _wmgrPort->top to 0 in that case
	setOrigin(pwnd->rect.left, pwnd->rect.top + _wmgrPort->top);
	pwnd->rect.moveTo(0, 0);
	return pwnd;
}

void GfxPorts::drawWindow(Window *pWnd) {
	if (pWnd->bDrawn)
		return;
	Common::Rect r;
	int16 wndStyle = pWnd->wndStyle;

	pWnd->bDrawn = true;
	Port *oldport = setPort(_wmgrPort);
	penColor(0);
	if ((wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT) == 0) {
		pWnd->hSaved1 = _paint16->bitsSave(pWnd->restoreRect, GFX_SCREEN_MASK_VISUAL);
		if (pWnd->saveScreenMask & GFX_SCREEN_MASK_PRIORITY) {
			pWnd->hSaved2 = _paint16->bitsSave(pWnd->restoreRect, GFX_SCREEN_MASK_PRIORITY);
			if ((wndStyle & SCI_WINDOWMGR_STYLE_USER) == 0)
				_paint16->fillRect(pWnd->restoreRect, GFX_SCREEN_MASK_PRIORITY, 0, 15);
		}
	}

	// drawing frame,shadow and title
	if ((getSciVersion() >= SCI_VERSION_1_LATE) ? !(wndStyle & _styleUser) : wndStyle != _styleUser) {
		r = pWnd->dims;
		if (!(wndStyle & SCI_WINDOWMGR_STYLE_NOFRAME)) {
			r.top++;
			r.left++;
			_paint16->frameRect(r);// draw shadow
			r.translate(-1, -1);
			_paint16->frameRect(r);// draw actual window frame

			if (wndStyle & SCI_WINDOWMGR_STYLE_TITLE) {
				if (getSciVersion() <= SCI_VERSION_0_LATE) {
					// draw a black line between titlebar and actual window content for SCI0
					r.bottom = r.top + 10;
					_paint16->frameRect(r);
				}
				r.grow(-1);
				if (getSciVersion() <= SCI_VERSION_0_LATE)
					_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, 8); // grey titlebar for SCI0
				else
					_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, 0); // black titlebar for SCI01+
				if (!pWnd->title.empty()) {
					int16 oldcolor = getPort()->penClr;
					penColor(_screen->getColorWhite());
					_text16->Box(pWnd->title.c_str(), 1, r, SCI_TEXT16_ALIGNMENT_CENTER, 0);
					penColor(oldcolor);
				}

				r.grow(+1);
				r.bottom = pWnd->dims.bottom - 1;
				r.top += 9;
			}

			r.grow(-1);
		}

		if (!(wndStyle & SCI_WINDOWMGR_STYLE_TRANSPARENT))
			_paint16->fillRect(r, GFX_SCREEN_MASK_VISUAL, pWnd->backClr);

		_paint16->bitsShow(pWnd->restoreRect);
	}
	setPort(oldport);
}

void GfxPorts::disposeWindow(Window *pWnd, bool reanimate) {
	setPort(_wmgrPort);
	_paint16->bitsRestore(pWnd->hSaved1);
	_paint16->bitsRestore(pWnd->hSaved2);
	if (!reanimate)
		_paint16->bitsShow(pWnd->restoreRect);
	else
		_paint16->kernelGraphRedrawBox(pWnd->restoreRect);
	_windowList.remove(pWnd);
	setPort(_windowList.back());
	_windowsById[pWnd->id] = 0;
	delete pWnd;
}

void GfxPorts::updateWindow(Window *wnd) {
	reg_t handle;

	if (wnd->saveScreenMask && wnd->bDrawn) {
		handle = _paint16->bitsSave(wnd->restoreRect, GFX_SCREEN_MASK_VISUAL);
		_paint16->bitsRestore(wnd->hSaved1);
		wnd->hSaved1 = handle;
		if (wnd->saveScreenMask & GFX_SCREEN_MASK_PRIORITY) {
			handle = _paint16->bitsSave(wnd->restoreRect, GFX_SCREEN_MASK_PRIORITY);
			_paint16->bitsRestore(wnd->hSaved2);
			wnd->hSaved2 = handle;
		}
	}
}

Port *GfxPorts::getPortById(uint16 id) {
	if (id > _windowsById.size())
		error("getPortById() received invalid id");
	return _windowsById[id];
}



Port *GfxPorts::setPort(Port *newPort) {
	Port *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

Port *GfxPorts::getPort() {
	return _curPort;
}

void GfxPorts::setOrigin(int16 left, int16 top) {
	// This looks fishy, but it's exactly what sierra did. They removed last bit of left in their interpreter
	//  It seems sierra did it for EGA byte alignment (EGA uses 1 byte for 2 pixels) and left it in their interpreter even
	//  when going VGA.
	_curPort->left = left & 0x7FFE;
	_curPort->top = top;
}

void GfxPorts::moveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void GfxPorts::move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

void GfxPorts::openPort(Port *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	Port *tmp = _curPort;
	_curPort = port;
	_text16->SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->greyedOutput = false;
	port->penClr = 0;
	port->backClr = _screen->getColorWhite();
	port->penMode = 0;
	port->rect = _bounds;
}

void GfxPorts::penColor(int16 color) {
	_curPort->penClr = color;
}

void GfxPorts::backColor(int16 color) {
	_curPort->backClr = color;
}

void GfxPorts::penMode(int16 mode) {
	_curPort->penMode = mode;
}

void GfxPorts::textGreyedOutput(bool state) {
	_curPort->greyedOutput = state;
}

int16 GfxPorts::getPointSize() {
	return _curPort->fontHeight;
}

void GfxPorts::offsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

void GfxPorts::offsetLine(Common::Point &start, Common::Point &end) {
	start.x += _curPort->left;
	start.y += _curPort->top;
	end.x += _curPort->left;
	end.y += _curPort->top;
}

void GfxPorts::priorityBandsInit(int16 bandCount, int16 top, int16 bottom) {
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

void GfxPorts::priorityBandsInit(byte *data) {
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

// Gets used by picture class to remember priority bands data from sci1.1 pictures that need to get applied when
//  transitioning to that picture
void GfxPorts::priorityBandsRemember(byte *data) {
	int bandNo;
	for (bandNo = 0; bandNo < 14; bandNo++) {
		priorityBandsMemory[bandNo] = READ_LE_UINT16(data);
		data += 2;
	}
	priorityBandsMemoryActive = true;
}

void GfxPorts::priorityBandsRecall() {
	if (priorityBandsMemoryActive) {
		priorityBandsInit((byte *)&priorityBandsMemory);
		priorityBandsMemoryActive = false;
	}
}

void GfxPorts::kernelInitPriorityBands() {
	if (_usesOldGfxFunctions) {
		priorityBandsInit(15, 42, 200);
	} else {
		if (getSciVersion() >= SCI_VERSION_1_1)
			priorityBandsInit(14, 0, 190);
		else
			priorityBandsInit(14, 42, 190);
	}
}

void GfxPorts::kernelGraphAdjustPriority(int top, int bottom) {
	if (_usesOldGfxFunctions) {
		priorityBandsInit(15, top, bottom);
	} else {
		priorityBandsInit(14, top, bottom);
	}
}

byte GfxPorts::kernelCoordinateToPriority(int16 y) {
	if (y < _priorityTop)
		return _priorityBands[_priorityTop];
	if (y > _priorityBottom)
		return _priorityBands[_priorityBottom];
	return _priorityBands[y];
}

int16 GfxPorts::kernelPriorityToCoordinate(byte priority) {
	int16 y;
	if (priority <= _priorityBandCount) {
		for (y = 0; y <= _priorityBottom; y++)
			if (_priorityBands[y] == priority)
				return y;
	}
	return _priorityBottom;
}

} // End of namespace Sci
