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
#include "graphics/macgui/maceditabletext.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

enum {
	kConWOverlap = 20,
	kConHOverlap = 20,
	kConWPadding = 3,
	kConHPadding = 4,
	kConOverscan = 3,
	kConScrollStep = 12,

	kCursorHeight = 12
};

static void cursorTimerHandler(void *refCon);

MacEditableText::MacEditableText(int w, int h, MacWindowManager *wm, Common::U32String s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) :
		MacWidget(w, h, true), MacText(s, wm, macFont, fgcolor, bgcolor, maxWidth, textAlignment, interlinear) {

	_maxWidth = maxWidth;

	init();
}

MacEditableText::MacEditableText(int w, int h, MacWindowManager *wm, const Common::String &s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear) :
		MacWidget(w, h, true), MacText(s, wm, macFont, fgcolor, bgcolor, maxWidth, textAlignment, interlinear) {

	_maxWidth = maxWidth;

	init();
}

void MacEditableText::init() {
	_inputTextHeight = 0;

	_inputIsDirty = true;
	_inTextSelection = false;

	_scrollPos = 0;
	_editable = true;
	_selectable = true;

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	_cursorDirty = true;

	_scrollbarIsDirty = false;

	_cursorRect = new Common::Rect(0, 0, 1, kCursorHeight);

	_cursorSurface = new ManagedSurface(1, kCursorHeight);
	_cursorSurface->fillRect(*_cursorRect, _wm->_colorBlack);

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "macEditableText");
}

MacEditableText::~MacEditableText() {
	delete _cursorRect;
	delete _cursorSurface;

	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
}

void MacEditableText::resize(int w, int h) {
	if (_surface->w == w && _surface->h == h)
		return;

	undrawInput();

	_maxWidth = w;
	MacText::setMaxWidth(_maxWidth);
}

void MacEditableText::appendText(Common::U32String str, const MacFont *macFont, bool skipAdd) {
	MacText::appendText(str, macFont->getId(), macFont->getSize(), macFont->getSlant(), skipAdd);

	_contentIsDirty = true;

	if (_editable) {
		_scrollPos = MAX(0, MacText::getTextHeight() - getDimensions().height());

		updateCursorPos();
	}
}

void MacEditableText::appendText(const Common::String &str, const MacFont *macFont, bool skipAdd) {
	appendText(Common::U32String(str), macFont, skipAdd);
}

void MacEditableText::clearText() {
	MacText::clearText();

	_contentIsDirty = true;
	_scrollbarIsDirty = true;

	updateCursorPos();
}

void MacEditableText::setTextFont(const MacFont *font) {
	_font = font;

	_fontRef = _wm->_fontMan->getFont(*font);

	MacText::setDefaultFormatting(font->getId(), font->getSlant(), font->getSize(), 0, 0, 0);
}

const MacFont *MacEditableText::getTextFont() {
	return _font;
}

bool MacEditableText::draw(ManagedSurface *g, bool forceRedraw) {
	if (!_scrollbarIsDirty && !_contentIsDirty && !_cursorDirty && !_inputIsDirty && !forceRedraw)
		return false;

	if (_scrollbarIsDirty || forceRedraw) {
		drawScrollbar();

		_composeSurface.clear(_wm->_colorWhite);
	}

	if (_inputIsDirty || forceRedraw) {
		drawInput();
		_inputIsDirty = false;
	}

	_contentIsDirty = false;

	// Compose
	MacText::draw(&_composeSurface, 0, _scrollPos, _surface->w - 2, _scrollPos + _surface->h - 2, kConWOverlap - 2, kConWOverlap - 2);

	if (_cursorState)
		_composeSurface.blitFrom(*_cursorSurface, *_cursorRect, Common::Point(_cursorX + kConWOverlap - 2, _cursorY + kConHOverlap - 2));

	if (_selectedText.endY != -1)
		drawSelection();

	//_composeSurface.transBlitFrom(_borderSurface, kColorGreen);

	g->transBlitFrom(_composeSurface, _composeSurface.getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), kColorGreen2);

	return true;
}

void MacEditableText::drawSelection() {
	if (_selectedText.endY == -1)
		return;

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startX, s.endX);
		SWAP(s.startY, s.endY);
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	int lastLineStart = s.endY;
	s.endY += MacText::getLineHeight(s.endRow);

	int start = s.startY - _scrollPos;
	start = MAX(0, start);

	if (start > getDimensions().height())
		return;

	int end = s.endY - _scrollPos;

	if (end < 0)
		return;

	end = MIN((int)getDimensions().height(), end);

	int numLines = 0;
	int x1 = 0, x2 = 0;

	for (int y = start; y < end; y++) {
		if (!numLines) {
			x1 = 0;
			x2 = getDimensions().width() - 1;

			if (y + _scrollPos == s.startY && s.startX > 0) {
				numLines = MacText::getLineHeight(s.startRow);
				x1 = s.startX;
			}
			if (y + _scrollPos >= lastLineStart) {
				numLines = MacText::getLineHeight(s.endRow);
				x2 = s.endX;
			}
		} else {
			numLines--;
		}

		byte *ptr = (byte *)_composeSurface.getBasePtr(x1 + kConWOverlap - 2, y + kConWOverlap - 2);

		for (int x = x1; x < x2; x++, ptr++)
			if (*ptr == _wm->_colorBlack)
				*ptr = _wm->_colorWhite;
			else
				*ptr = _wm->_colorBlack;
	}
}

Common::U32String MacEditableText::getSelection(bool formatted, bool newlines) {
	if (_selectedText.endY == -1)
		return Common::U32String("");

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	return MacText::getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, formatted, newlines);
}

void MacEditableText::clearSelection() {
	_selectedText.endY = _selectedText.startY = -1;
}

bool MacEditableText::isCutAllowed() {
	if (_selectedText.startRow >= (int)(MacText::getLineCount() - _inputTextHeight) &&
			_selectedText.endRow  >= (int)(MacText::getLineCount() - _inputTextHeight))
		return true;

	return false;
}

Common::U32String MacEditableText::cutSelection() {
	if (!isCutAllowed())
		return Common::U32String("");

	SelectedText s = _selectedText;

	if (s.startY > s.endY || (s.startY == s.endY && s.startX > s.endX)) {
		SWAP(s.startRow, s.endRow);
		SWAP(s.startCol, s.endCol);
	}

	Common::U32String selection = MacText::getTextChunk(s.startRow, s.startCol, s.endRow, s.endCol, false, false);

	uint32 selPos = _inputText.find(selection);

	if (selPos == Common::U32String::npos) {
		//warning("Cannot find substring '%s' in '%s'", selection.c_str(), _inputText.c_str()); // Needed encode method

		return Common::U32String("");
	}

	Common::U32String newInput = Common::U32String(_inputText.c_str(), selPos) + Common::U32String(_inputText.c_str() + selPos + selection.size());

	clearSelection();
	clearInput();
	appendInput(newInput);

	return selection;
}

bool MacEditableText::processEvent(Common::Event &event) {
	WindowClick click = kBorderInner; //_parent->isInBorder(event.mouse.x, event.mouse.y);

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!_editable)
			return false;

		// Make the parent window active
		_wm->setActive(_parent->getId());

		if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
			return false;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (!_inputText.empty()) {
				_inputText.deleteLastChar();
				_inputIsDirty = true;
			}
			return true;

		case Common::KEYCODE_RETURN:
			undrawInput();
			return false; // Pass it to the higher level for processing

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				_inputText += (char)event.kbd.ascii;
				_inputIsDirty = true;

				return true;
			}

			break;
		}
	}

	if (hasAllFocus())
		return _parent->processEvent(event);	// Pass it to upstream

	if (event.type == Common::EVENT_WHEELUP) {
		scroll(-2);
		return true;
	}

	if (event.type == Common::EVENT_WHEELDOWN) {
		scroll(2);
		return true;
	}

	if (click == kBorderScrollUp || click == kBorderScrollDown) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			int consoleHeight = getDimensions().height();
			int textFullSize = MacText::getTextHeight();
			float scrollPos = (float)_scrollPos / textFullSize;
			float scrollSize = (float)consoleHeight / textFullSize;

			setScroll(scrollPos, scrollSize);

			return true;
		} else if (event.type == Common::EVENT_LBUTTONUP) {
			switch (click) {
			case kBorderScrollUp:
				scroll(-1);
				break;
			case kBorderScrollDown:
				scroll(1);
				break;
			default:
				return false;
			}

			return true;
		}

		return false;
	}

	if (click == kBorderInner) {
		if (!_selectable)
			return false;

		if (event.type == Common::EVENT_LBUTTONDOWN) {
			startMarking(event.mouse.x, event.mouse.y);

			return true;
		} else if (event.type == Common::EVENT_LBUTTONUP && _menu) {
			if (_inTextSelection) {
				_inTextSelection = false;

				if (_selectedText.endY == -1 ||
						(_selectedText.endX == _selectedText.startX && _selectedText.endY == _selectedText.startY)) {
					_selectedText.startY = _selectedText.endY = -1;
					_contentIsDirty = true;
					_menu->enableCommand("Edit", "Copy", false);
				} else {
					_menu->enableCommand("Edit", "Copy", true);

					bool cutAllowed = isCutAllowed();

					_menu->enableCommand("Edit", "Cut", cutAllowed);
					_menu->enableCommand("Edit", "Clear", cutAllowed);
				}
			}

			return true;
		} else if (event.type == Common::EVENT_MOUSEMOVE) {
			if (_inTextSelection) {
				updateTextSelection(event.mouse.x, event.mouse.y);
				return true;
			}
		}

		return false;
	}

	return _parent->processEvent(event);
}

void MacEditableText::scroll(int delta) {
	int oldScrollPos = _scrollPos;

	_scrollPos += delta * kConScrollStep;

	if (_editable)
		_scrollPos = CLIP<int>(_scrollPos, 0, MacText::getTextHeight() - kConScrollStep);
	else
		_scrollPos = CLIP<int>(_scrollPos, 0, MAX(0, MacText::getTextHeight() - getDimensions().height()));

	undrawCursor();
	_cursorY -= (_scrollPos - oldScrollPos);
	_contentIsDirty = true;
	_scrollbarIsDirty = true;
}

void MacEditableText::startMarking(int x, int y) {
	x -= getDimensions().left - 2;
	y -= getDimensions().top;

	y += _scrollPos;

	MacText::getRowCol(x, y, &_selectedText.startX, &_selectedText.startY, &_selectedText.startRow, &_selectedText.startCol);

	_selectedText.endY = -1;

	_inTextSelection = true;
}

void MacEditableText::updateTextSelection(int x, int y) {
	x -= getDimensions().left - 2;
	y -= getDimensions().top;

	y += _scrollPos;

	MacText::getRowCol(x, y, &_selectedText.endX, &_selectedText.endY, &_selectedText.endRow, &_selectedText.endCol);

	debug(3, "s: %d,%d (%d, %d) e: %d,%d (%d, %d)", _selectedText.startX, _selectedText.startY,
			_selectedText.startRow, _selectedText.startCol, _selectedText.endX,
			_selectedText.endY, _selectedText.endRow, _selectedText.endCol);

	_contentIsDirty = true;
}

void MacEditableText::undrawInput() {
	for (uint i = 0; i < _inputTextHeight; i++)
		MacText::removeLastLine();

	if (_inputTextHeight)
		appendText("\n", _font, true);

	_inputTextHeight = 0;
}

void MacEditableText::drawInput() {
	undrawInput();

	Common::Array<Common::U32String> text;

	// Now recalc new text height
	_fontRef->wordWrapText(_inputText, _maxWidth, text);
	_inputTextHeight = MAX((uint)1, text.size()); // We always have line to clean

	// And add new input line to the text
	appendText(_inputText, _font, true);

	_cursorX = _inputText.empty() ? 0 : _fontRef->getStringWidth(text[_inputTextHeight - 1]);

	updateCursorPos();

	_contentIsDirty = true;
}

void MacEditableText::clearInput() {
	undrawCursor();

	_cursorX = 0;
	_inputText.clear();
}

void MacEditableText::appendInput(Common::U32String str) {
	_inputText += str;

	drawInput();
}

void MacEditableText::appendInput(const Common::String &str) {
	appendInput(Common::U32String(str));
}

//////////////////
// Cursor stuff
static void cursorTimerHandler(void *refCon) {
	MacEditableText *w = (MacEditableText *)refCon;

	if (!w->_cursorOff)
		w->_cursorState = !w->_cursorState;

	w->_cursorDirty = true;
}

void MacEditableText::updateCursorPos() {
	_cursorY = MacText::getTextHeight() - _scrollPos - kCursorHeight;

	_cursorDirty = true;
}

void MacEditableText::undrawCursor() {
	_cursorState = false;
	_cursorDirty = true;
}

void MacEditableText::setScroll(float scrollPos, float scrollSize) {
}

void MacEditableText::drawScrollbar() {
}

} // End of namespace Graphics
