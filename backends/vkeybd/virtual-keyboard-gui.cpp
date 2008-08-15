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

#include "backends/vkeybd/virtual-keyboard-gui.h"
#include "graphics/cursorman.h"
#include "gui/newgui.h"

namespace Common {

VirtualKeyboardGUI::VirtualKeyboardGUI(VirtualKeyboard *kbd)
	: _kbd(kbd), _displaying(false), _needRedraw(false), _drag(false),
	_drawCaret(false), _refreshDisplay(false), _displayEnabled(false),
	_firstRun(true), _cursorAnimateTimer(0), _cursorAnimateCounter(0) {
	
	assert(_kbd);
	assert(g_system);
	_system = g_system;
	
	_lastScreenChanged = _system->getScreenChangeID();
	
	memset(_cursor, 0xFF, sizeof(_cursor));
}

VirtualKeyboardGUI::~VirtualKeyboardGUI() {
	_overlayBackup.free();
	_dispSurface.free();
}

void VirtualKeyboardGUI::initMode(VirtualKeyboard::Mode *mode) {
	_kbdSurface = mode->image;
	_kbdTransparentColor = mode->transparentColor;
	_kbdBound.setWidth(_kbdSurface->w + 1);
	_kbdBound.setHeight(_kbdSurface->h + 1);
	_needRedraw = true;

	_dispSurface.free();
	_displayEnabled = false;
	if (!mode->displayArea)
		return;
	Rect r = *(mode->displayArea);

	// choose font
	_dispFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	if (!fontIsSuitable(_dispFont, r)) {
		_dispFont = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		if (!fontIsSuitable(_dispFont, r))
			return;
	}
	_dispX = r.left;
	_dispY = r.top + (r.height() + 1 - _dispFont->getFontHeight()) / 2;
	_dispSurface.create(r.width() + 1, _dispFont->getFontHeight(), sizeof(OverlayColor));
	_dispI = 0;
	_dispForeColor = mode->displayFontColor;
	_dispBackColor = _dispForeColor + 0xFF;
	_displayEnabled = true;
}

bool VirtualKeyboardGUI::fontIsSuitable(const Graphics::Font *font, const Rect& rect) {
	return (font->getMaxCharWidth() < rect.width() &&
			font->getFontHeight() < rect.height());
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
	resetDirtyRect();

	setupCursor();

	_displaying = true;
	mainLoop();

	removeCursor();

	_system->copyRectToOverlay((OverlayColor*)_overlayBackup.pixels, _overlayBackup.w, 0, 0, _overlayBackup.w, _overlayBackup.h);
	if (!g_gui.isActive()) _system->hideOverlay();

	_overlayBackup.free();
	_dispSurface.free();
}

void VirtualKeyboardGUI::close() {
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
	// add old position to dirty area
	extendDirtyRect(_kbdBound);
	_needRedraw = true;

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
		if (_displayEnabled) {
			if (_kbd->_keyQueue.hasStringChanged())
				_refreshDisplay = true;
			animateCaret();
			if (_refreshDisplay) updateDisplay();
		}
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
				if (_drag)
					move(event.mouse.x - _dragPoint.x, 
						event.mouse.y - _dragPoint.y);
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

void VirtualKeyboardGUI::extendDirtyRect(const Rect &r) {
	if (_dirtyRect.isValidRect()) {
		_dirtyRect.extend(r);
	} else {
		_dirtyRect = r;
	}
	_dirtyRect.clip(Rect(0, 0, _overlayBackup.w, _overlayBackup.h));
}

void VirtualKeyboardGUI::resetDirtyRect() {
	_dirtyRect.setWidth(-1);
}

void VirtualKeyboardGUI::redraw() {
	assert(_kbdSurface);

	extendDirtyRect(_kbdBound);

	Graphics::SurfaceKeyColored surf;
	surf.create(_dirtyRect.width(), _dirtyRect.height(), sizeof(OverlayColor));

	OverlayColor *dst = (OverlayColor *)surf.pixels;
	const OverlayColor *src = (OverlayColor *) _overlayBackup.getBasePtr(_dirtyRect.left, _dirtyRect.top);
	int16 h = surf.h;

	while (h--) {
		memcpy(dst, src, surf.w * sizeof(OverlayColor));
		dst += surf.w;
		src += _overlayBackup.w;
	}

	int16 keyX = _kbdBound.left - _dirtyRect.left;
	int16 keyY = _kbdBound.top - _dirtyRect.top;
	surf.blit(_kbdSurface, keyX, keyY, _kbdTransparentColor);
	if (_displayEnabled) surf.blit(&_dispSurface, keyX + _dispX, keyY + _dispY, _dispBackColor);
	_system->copyRectToOverlay((OverlayColor*)surf.pixels, surf.w,
		_dirtyRect.left, _dirtyRect.top, surf.w, surf.h);

	surf.free();

	_needRedraw = false;
	
	resetDirtyRect();
}

uint VirtualKeyboardGUI::calculateEndIndex(const String& str, uint startIndex) {
	int16 w = 0;
	while (w <= _dispSurface.w && startIndex < str.size()) {
		w += _dispFont->getCharWidth(str[startIndex++]);
	}
	if (w > _dispSurface.w) startIndex--;
	return startIndex;
}

void VirtualKeyboardGUI::animateCaret() {
	if (_system->getMillis() % kCaretBlinkTime < kCaretBlinkTime / 2) {
		if (!_drawCaret) {
			_drawCaret = true;
			_refreshDisplay = true;
		}
	} else {
		if (_drawCaret) {
			_drawCaret = false;
			_refreshDisplay = true;
		}
	}
}

void VirtualKeyboardGUI::updateDisplay() {
	if (!_displayEnabled) return;

	// calculate the text to display
	uint cursorPos = _kbd->_keyQueue.getInsertIndex();
	String wholeText = _kbd->_keyQueue.getString();
	uint dispTextEnd;
	if (_dispI > cursorPos)
		_dispI = cursorPos;
	
	dispTextEnd = calculateEndIndex(wholeText, _dispI);
	while (cursorPos > dispTextEnd)
		dispTextEnd = calculateEndIndex(wholeText, ++_dispI);
	
	String dispText = String(wholeText.c_str() + _dispI, wholeText.c_str() + dispTextEnd);

	// draw to display surface
	_dispSurface.fillRect(Rect(0, 0, _dispSurface.w, _dispSurface.h), _dispBackColor);
	_dispFont->drawString(&_dispSurface, dispText, 0, 0, _dispSurface.w, _dispForeColor);
	if (_drawCaret) {
		String beforeCaret(wholeText.c_str() + _dispI, wholeText.c_str() + cursorPos);
		int16 caretX = _dispFont->getStringWidth(beforeCaret);
		_dispSurface.drawLine(caretX, 0, caretX, _dispSurface.h, _dispForeColor);
	}

	_needRedraw = true;
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
