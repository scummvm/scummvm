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
* $URL$
* $Id$
*
*/

#include "backends/common/virtual-keyboard-gui.h"
#include "backends/common/virtual-keyboard.h"
#include "graphics/cursorman.h"
#include "graphics/surface-keycolored.h"
#include "gui/newgui.h"

namespace Common {

VirtualKeyboardGUI::VirtualKeyboardGUI(VirtualKeyboard *kbd) {
	_kbd = kbd;

	assert(g_system);
	_system = g_system;

	_lastScreenChanged = _system->getScreenChangeID();

	memset(_cursor, 0xFF, sizeof(_cursor));

	_displaying = _needRedraw = _drag = false;
	_firstRun = true;
}

void VirtualKeyboardGUI::setKeyboardSurface(Graphics::Surface *sur, OverlayColor trans_color) {
	_kbdSurface = sur;
	_kbdTransparentColor = trans_color;
	_kbdBound.setWidth(_kbdSurface->w);
	_kbdBound.setHeight(_kbdSurface->h);
	_needRedraw = true;
}

void VirtualKeyboardGUI::run() {

	if (_lastScreenChanged != _system->getScreenChangeID())
		screenChanged();

	// TODO: set default position if position is somehow invalid
	if (_firstRun) {
		_firstRun = false;
		setDefaultPosition();
	}

	if (!g_gui.isActive()) {
		_system->showOverlay();
		_system->clearOverlay();
	}
	_overlayBackup.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	_system->grabOverlay((OverlayColor*)_overlayBackup.pixels, _overlayBackup.w);
	setupCursor();

	_displaying = true;
	mainLoop();

	removeCursor();
	_system->copyRectToOverlay((OverlayColor*)_overlayBackup.pixels, _overlayBackup.w, 0, 0, _overlayBackup.w, _overlayBackup.h);
	if (!g_gui.isActive()) _system->hideOverlay();
	_overlayBackup.free();
}

void VirtualKeyboardGUI::hide() {
	_displaying = false;
}

void VirtualKeyboardGUI::reset() {
	_kbdBound.left = _kbdBound.top
		= _kbdBound.right = _kbdBound.bottom = 0;
	_displaying = _drag = false;
	_firstRun = true;
	_lastScreenChanged = _system->getScreenChangeID();
	_kbdSurface = 0;
}

void VirtualKeyboardGUI::setDefaultPosition()
{
	int16 scrW = _system->getOverlayWidth(), scrH = _system->getOverlayHeight(); 
	int16 kbdW = _kbdBound.width(), kbdH = _kbdBound.height();
	int16 posX = 0, posY = 0;
	if (scrW != kbdW) {
		switch (_kbd->_hAlignment) {
		case VirtualKeyboard::kAlignLeft:
			posX = 0;
			break;
		case VirtualKeyboard::kAlignCentre:
			posX = (scrW - kbdW) / 2;
			break;
		case VirtualKeyboard::kAlignRight:
			posX = scrW - kbdW;
			break;
		}
	}
	if (scrH != kbdH) {
		switch (_kbd->_vAlignment) {
		case VirtualKeyboard::kAlignTop:
			posY = 0;
			break;
		case VirtualKeyboard::kAlignMiddle:
			posY = (scrH - kbdH) / 2;
			break;
		case VirtualKeyboard::kAlignBottom:
			posY = scrH - kbdH;
			break;
		}
	}
	_kbdBound.moveTo(posX, posY);
}

void VirtualKeyboardGUI::move(int16 x, int16 y) {
	// snap to edge of screen
	if (ABS(x) < SNAP_WIDTH)
		x = 0;
	int16 x2 = _system->getOverlayWidth() - _kbdBound.width();
	if (ABS(x - x2) < SNAP_WIDTH)
		x = x2;
	if (ABS(y) < SNAP_WIDTH)
		y = 0;
	int16 y2 = _system->getOverlayHeight() - _kbdBound.height();
	if (ABS(y - y2) < SNAP_WIDTH)
		y = y2;

	_kbdBound.moveTo(x, y);
}

void VirtualKeyboardGUI::screenChanged() {
	_lastScreenChanged = _system->getScreenChangeID();
	if (!_kbd->checkModeResolutions())
		_displaying = false;
}


void VirtualKeyboardGUI::mainLoop() {
	Common::EventManager *eventMan = _system->getEventManager();

	while (_displaying) {
		if (_needRedraw) redraw();

		animateCursor();
		_system->updateScreen();
		Common::Event event;
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				if (_kbdBound.contains(event.mouse)) {
					_kbd->handleMouseDown(event.mouse.x - _kbdBound.left,
										  event.mouse.y - _kbdBound.top);
				}
				break;
			case Common::EVENT_LBUTTONUP:
				if (_kbdBound.contains(event.mouse)) {
					_kbd->handleMouseUp(event.mouse.x - _kbdBound.left,
										event.mouse.y - _kbdBound.top);
				}
				break;
			case Common::EVENT_MOUSEMOVE:
				if (_drag) {
					move(event.mouse.x - _dragPoint.x, 
						event.mouse.y - _dragPoint.y);
					_needRedraw = true;
				}
				break;
			case Common::EVENT_SCREEN_CHANGED:
				screenChanged();
				break;
			case Common::EVENT_QUIT:
				_system->quit();
				return;
			default:
				break;
			}
		}
		// Delay for a moment
		_system->delayMillis(10);
	}
}

void VirtualKeyboardGUI::startDrag(int16 x, int16 y) {
	_drag = true;
	_dragPoint.x = x;
	_dragPoint.y = y;
}

void VirtualKeyboardGUI::endDrag() {
	_drag = false;
}


void VirtualKeyboardGUI::redraw() {
	Graphics::SurfaceKeyColored surf;
	assert(_kbdSurface);

	surf.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));

	memcpy(surf.pixels, _overlayBackup.pixels, surf.w * surf.h * sizeof(OverlayColor));
	surf.blit(_kbdSurface, _kbdBound.left, _kbdBound.top, _kbdTransparentColor);

	_system->copyRectToOverlay((OverlayColor*)surf.pixels, surf.w, 0, 0, surf.w, surf.h);

	surf.free();

	_needRedraw = false;
}

void VirtualKeyboardGUI::setupCursor() {
	const byte palette[] = {
		255, 255, 255, 0,
		255, 255, 255, 0,
		171, 171, 171, 0,
		87,  87,  87, 0
	};

	CursorMan.pushCursorPalette(palette, 0, 4);
	CursorMan.pushCursor(NULL, 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

void VirtualKeyboardGUI::animateCursor() {
	int time = _system->getMillis();
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
		for (int i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = _cursorAnimateCounter;
				_cursor[16 * i + 7] = _cursorAnimateCounter;
			}
		}

		CursorMan.replaceCursor(_cursor, 16, 16, 7, 7);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

void VirtualKeyboardGUI::removeCursor() {
	CursorMan.popCursor();
	CursorMan.popCursorPalette();
}

} // end of namespace Common
