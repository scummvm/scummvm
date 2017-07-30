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

#include "common/timer.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactextwindow.h"

namespace Graphics {

static void cursorTimerHandler(void *refCon);

MacTextWindow::MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment) :
		MacWindow(wm->getLastId(), true, true, true, wm) {

	_font = font;
	_mactext = new MacText("", _wm, font, fgcolor, bgcolor, maxWidth, textAlignment);

	_fontRef = wm->_fontMan->getFont(*font);

	_inputTextHeight = 0;
	_maxWidth = maxWidth;

	_scrollPos = 0;

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "textWindowCursor");
}

void MacTextWindow::drawText(ManagedSurface *g, int x, int y, int w, int h, int xoff, int yoff) {
	_mactext->draw(g, x, y, w, h, xoff, yoff);
}

void MacTextWindow::appendText(Common::String str, int id, int size, int slant) {
	_mactext->appendText(str, id, size, slant);
}

void MacTextWindow::appendText(Common::String str, const MacFont *macFont) {
	_mactext->appendText(str, macFont->getId(), macFont->getSize(), macFont->getSlant());
}

void MacTextWindow::clearText() {
	_mactext->clearText();
}

void MacTextWindow::setSelection(int selStartX, int selStartY, int selEndX, int selEndY) {
	_selectedText.startX = selStartX;
	_selectedText.startY = selStartY;
	_selectedText.endX = selEndX;
	_selectedText.endY = selEndY;
}

MacTextWindow::~MacTextWindow() {
	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
}

void MacTextWindow::setTextWindowFont(const MacFont *font) {
	_font = font;

	_fontRef = _wm->_fontMan->getFont(*font);
}

const MacFont *MacTextWindow::getTextWindowFont() {
	return _font;
}

bool MacTextWindow::draw(ManagedSurface *g, bool forceRedraw) {
	if (!_borderIsDirty && !_contentIsDirty && !_cursorDirty && !forceRedraw)
		return false;

	if (_borderIsDirty || forceRedraw)
		drawBorder();

	_contentIsDirty = false;

	// Compose
	_composeSurface.blitFrom(_surface, Common::Rect(0, 0, _surface.w - 2, _surface.h - 2), Common::Point(2, 2));
	_composeSurface.transBlitFrom(_borderSurface, kColorGreen);

	g->transBlitFrom(_composeSurface, _composeSurface.getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), kColorGreen2);

	return true;
}

bool MacTextWindow::processEvent(Common::Event &event) {
	WindowClick click = isInBorder(event.mouse.x, event.mouse.y);

	if (event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (!_inputText.empty()) {
				_inputText.deleteLastChar();
				drawInput();
			}
			break;

		case Common::KEYCODE_RETURN:
			return false; // Pass it to the higher level for processing

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				_inputText += (char)event.kbd.ascii;
				drawInput();
			}

			break;
		}
	}

	return false;
}

void MacTextWindow::drawInput() {
	// First, we kill previous input text
	for (uint i = 0; i < _inputTextHeight; i++)
		_mactext->removeLastLine();

	Common::Array<Common::String> text;

	// Now recalc new text height
	_fontRef->wordWrapText(_inputText, _maxWidth, text);
	_inputTextHeight = MAX(1u, text.size()); // We always have line to clean

	// And add new input line to the text
	appendText(_inputText, _font);

	_cursorX = _fontRef->getStringWidth(text[_inputTextHeight - 1]);

	if (_scrollPos)
		_cursorY = _mactext->getTextHeight() - kCursorHeight * 2;
	else
		_cursorY = _mactext->getTextHeight() - kCursorHeight;

}

//////////////////
// Cursor stuff
static void cursorTimerHandler(void *refCon) {
	MacTextWindow *w = (MacTextWindow *)refCon;

	int x = w->_cursorX;
	int y = w->_cursorY;

	if (x == 0 && y == 0)
		return;

	x += w->getInnerDimensions().left;
	y += w->getInnerDimensions().top;
	int h = kCursorHeight;

	if (y + h > w->getInnerDimensions().bottom) {
		h = w->getInnerDimensions().bottom - y;
	}

	if (h > 0)
		w->getSurface()->vLine(x, y, y + h, w->_cursorState ? kColorBlack : kColorWhite);

	if (!w->_cursorOff)
		w->_cursorState = !w->_cursorState;

	w->_cursorRect.left = x;
	w->_cursorRect.right = MIN<uint16>(x + 1, w->getInnerDimensions().right);
	w->_cursorRect.top = MIN<uint16>(y - 1, w->getInnerDimensions().top);
	w->_cursorRect.bottom = MIN<uint16>(y + h, w->getInnerDimensions().bottom);

	w->_cursorDirty = true;
}

void MacTextWindow::undrawCursor() {
	_cursorOff = true;
	_cursorState = false;
	cursorTimerHandler(this);
	_cursorOff = false;
}


} // End of namespace Graphics
