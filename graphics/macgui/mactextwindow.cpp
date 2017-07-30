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


#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactextwindow.h"

namespace Graphics {

MacTextWindow::MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment) :
		MacWindow(wm->getLastId(), true, true, true, wm) {

	_font = font;
	_mactext = new MacText("", _wm, font, fgcolor, bgcolor, maxWidth, textAlignment);

	_fontRef = wm->_fontMan->getFont(*font);

	_inputTextHeight = 0;
	_maxWidth = maxWidth;
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
}

const MacFont *MacTextWindow::getTextWindowFont() {
	return _font;
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
}

} // End of namespace Graphics
