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
	kConScrollStep = 12,

	kCursorHeight = 12
};

static void cursorTimerHandler(void *refCon);

MacEditableText::MacEditableText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 border, uint16 gutter, uint16 boxShadow, uint16 textShadow) :
	MacWidget(parent, x, y, w, h, true, border, gutter, boxShadow), MacText(s, wm, macFont, fgcolor, bgcolor, maxWidth, textAlignment, interlinear, textShadow) {

	_maxWidth = maxWidth;

	init();

	setDefaultFormatting(macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);

	MacText::render();
	setAlignOffset(_textAlignment);
	updateCursorPos();
}

MacEditableText::MacEditableText(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::String &s, const MacFont *macFont, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, int interlinear, uint16 border, uint16 gutter, uint16 boxShadow, uint16 textShadow) :
	MacWidget(parent, x, y, w, h, true, border, gutter, boxShadow), MacText(s, wm, macFont, fgcolor, bgcolor, maxWidth, textAlignment, interlinear, textShadow) {

	_maxWidth = maxWidth;

	init();

	setDefaultFormatting(macFont->getId(), macFont->getSlant(), macFont->getSize(), 0, 0, 0);

	MacText::render();
	setAlignOffset(_textAlignment);
	updateCursorPos();
}

void MacEditableText::init() {
	_inTextSelection = false;

	_scrollPos = 0;
	_editable = false;
	_selectable = false;

	_editableRow = 0;

	_menu = nullptr;

	_cursorX = 0;
	_cursorY = 0;
	_cursorState = false;
	_cursorOff = false;

	_cursorRow = getLineCount() - 1;
	_cursorCol = getLineCharWidth(_cursorRow);

	_cursorRect = new Common::Rect(0, 0, 1, kCursorHeight);

	_cursorSurface = new ManagedSurface(1, kCursorHeight);
	_cursorSurface->clear(_wm->_colorBlack);
}

MacEditableText::~MacEditableText() {
	_wm->setActiveWidget(nullptr);

	delete _cursorRect;
	delete _cursorSurface;
	delete _composeSurface;
}

void MacEditableText::setAlignOffset(TextAlign align) {
	switch(align) {
	case kTextAlignLeft:
	default:
		_alignOffset = Common::Point(0, 0);
		break;
	case kTextAlignCenter:
		_alignOffset = Common::Point((_maxWidth / 2) - (_surface->w / 2), 0);
		break;
	case kTextAlignRight:
		_alignOffset = Common::Point(_maxWidth - (_surface->w + 1), 0);
		break;
	}
}

Common::Point MacEditableText::calculateOffset() {
	return Common::Point(_alignOffset.x + _border + _gutter + 1, _alignOffset.y + _border + _gutter/2);
}

void MacEditableText::setActive(bool active) {
	if (_active == active)
		return;

	MacWidget::setActive(active);

	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
	if (_active) {
		g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "macEditableText");
	}

	if (!_cursorOff && _cursorState == true)
		undrawCursor();
}

void MacEditableText::setEditable(bool editable) {
	if (editable == _editable)
		return;

	_editable = editable;
	_cursorOff = !editable;

	if (editable) {
		// TODO: Select whole region. This is done every time the text is set from
		// uneditable to editable.
		setActive(editable);
		_wm->setActiveWidget(this);
	} else {
		undrawCursor();
	}
}

void MacEditableText::resize(int w, int h) {
	if (_surface->w == w && _surface->h == h)
		return;

	_maxWidth = w;
	MacText::setMaxWidth(_maxWidth);
}

void MacEditableText::appendText(const Common::U32String &str, const MacFont *macFont, bool skipAdd) {
	MacText::appendTextDefault(str, skipAdd);

	_contentIsDirty = true;

	if (_editable) {
		_scrollPos = MAX(0, MacText::getTextHeight() - getDimensions().height());

		_cursorRow = getLineCount();
		_cursorCol = getLineCharWidth(_cursorRow);

		updateCursorPos();
	}
}

void MacEditableText::appendText(const Common::String &str, const MacFont *macFont, bool skipAdd) {
	appendText(Common::U32String(str), macFont, skipAdd);
}

void MacEditableText::clearText() {
	MacText::clearText();

	_contentIsDirty = true;

	_cursorRow = _cursorCol = 0;

	updateCursorPos();
}

bool MacEditableText::draw(bool forceRedraw) {
	if (!_contentIsDirty && !_cursorDirty && !forceRedraw)
		return false;

	if (!_surface) {
		warning("MacEditableText::draw: Null surface");
		return false;
	}

	_composeSurface->clear(_bgcolor);

	_contentIsDirty = false;
	_cursorDirty = false;

	// Compose
	for (int bb = 0; bb < _shadow; bb ++) {
		_composeSurface->hLine(_shadow, _composeSurface->h - _shadow + bb, _composeSurface->w, 0);
		_composeSurface->vLine(_composeSurface->w - _shadow + bb, _shadow, _composeSurface->h - _shadow, 0);
	}

	for (int bb = 0; bb < _border; bb++) {
		Common::Rect borderRect(bb, bb, _composeSurface->w - _shadow - bb, _composeSurface->h - _shadow - bb);
		_composeSurface->frameRect(borderRect, 0);
	}

	Common::Point offset(calculateOffset());
	MacText::draw(_composeSurface, 0, _scrollPos, _surface->w, _scrollPos + _surface->h, offset.x, offset.y);

	if (_cursorState)
		_composeSurface->blitFrom(*_cursorSurface, *_cursorRect, Common::Point(_cursorX, _cursorY + offset.y + 1));

	if (_selectedText.endY != -1)
		drawSelection();

	return true;
}

bool MacEditableText::draw(ManagedSurface *g, bool forceRedraw) {
	if (!MacEditableText::draw(forceRedraw))
		return false;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), kColorGreen2);

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

		byte *ptr = (byte *)_composeSurface->getBasePtr(x1, y);

		for (int x = x1; x < x2; x++, ptr++)
			if (*ptr == _fgcolor)
				*ptr = _bgcolor;
			else
				*ptr = _fgcolor;
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
	if (_selectedText.startRow >= _editableRow &&
			_selectedText.endRow  >= _editableRow)
		return true;

	return false;
}

Common::U32String MacEditableText::getEditedString() {
	return getTextChunk(_editableRow, 0, -1, -1);
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

	// TODO: Remove the actual text

	clearSelection();

	return selection;
}

bool MacEditableText::processEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		if (!_editable)
			return false;

		setActive(true);

		if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
			return false;
		}

		int ncol;

		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (_cursorRow > 0 || _cursorCol > 0) {
				deletePreviousChar(&_cursorRow, &_cursorCol);
				updateCursorPos();
				_contentIsDirty = true;
			}
			return true;

		case Common::KEYCODE_RETURN:
			addNewLine(&_cursorRow, &_cursorCol);
			updateCursorPos();
			_contentIsDirty = true;
			return true;

		case Common::KEYCODE_LEFT:
			if (_cursorCol == 0) {
				if (_cursorRow == 0) { // Nowhere to go
					return true;
				}
				_cursorRow--;
				_cursorCol = getLineCharWidth(_cursorRow) - 1;
			} else {
				_cursorCol--;
			}
			updateCursorPos();

			return true;

		case Common::KEYCODE_RIGHT:
			if (_cursorCol >= getLineCharWidth(_cursorRow)) {
				if (_cursorRow == getLineCount() - 1) { // Nowhere to go
					return true;
				}
				_cursorRow++;
				_cursorCol = 0;
			} else {
				_cursorCol++;
			}
			updateCursorPos();

			return true;

		case Common::KEYCODE_UP:
			if (_cursorRow == 0)
				return true;

			_cursorRow--;

			if (_cursorCol > 0) {
				getRowCol(_cursorX, _textLines[_cursorRow].y, nullptr, nullptr, nullptr, &ncol);
				_cursorCol = ncol + 1;
			}

			updateCursorPos();

			return true;

		case Common::KEYCODE_DOWN:
			if (_cursorRow == getLineCount() - 1)
				return true;

			_cursorRow++;
			if (_cursorCol > 0) {
				getRowCol(_cursorX, _textLines[_cursorRow].y, nullptr, nullptr, nullptr, &ncol);
				_cursorCol = ncol + 1;
			}

			updateCursorPos();

			return true;

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				insertChar((byte)event.kbd.ascii, &_cursorRow, &_cursorCol);
				updateCursorPos();
				_contentIsDirty = true;

				return true;
			}

			break;
		}
	}

	if (event.type == Common::EVENT_WHEELUP) {
		scroll(-2);
		return true;
	}

	if (event.type == Common::EVENT_WHEELDOWN) {
		scroll(2);
		return true;
	}

	if (!_selectable)
		return false;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		_wm->setActiveWidget(this);

		startMarking(event.mouse.x, event.mouse.y);

		return true;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_inTextSelection) {
			_inTextSelection = false;

			if (_selectedText.endY == -1 ||
					(_selectedText.endX == _selectedText.startX && _selectedText.endY == _selectedText.startY)) {
				_selectedText.startY = _selectedText.endY = -1;
				_contentIsDirty = true;

				if (_menu)
					_menu->enableCommand("Edit", "Copy", false);

				int x = event.mouse.x - getDimensions().left;
				int y = event.mouse.y - getDimensions().top + _scrollPos;

				MacText::getRowCol(x, y, nullptr, nullptr, &_cursorRow, &_cursorCol);
				updateCursorPos();
			} else {
				if (_menu) {
					_menu->enableCommand("Edit", "Copy", true);

					bool cutAllowed = isCutAllowed();

					_menu->enableCommand("Edit", "Cut", cutAllowed);
					_menu->enableCommand("Edit", "Clear", cutAllowed);
				}
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

//////////////////
// Cursor stuff
static void cursorTimerHandler(void *refCon) {
	MacEditableText *w = (MacEditableText *)refCon;

	if (!w->_cursorOff)
		w->_cursorState = !w->_cursorState;

	w->_cursorDirty = true;
}

void MacEditableText::updateCursorPos() {
	if (_textLines.empty()) {
		_cursorX = _cursorY = 0;
	} else {
		_cursorRow = MIN<int>(_cursorRow, _textLines.size() - 1);

		Common::Point offset(calculateOffset());

		int alignOffset = 0;
		if (_textAlignment == kTextAlignRight)
			alignOffset = _textMaxWidth - getLineWidth(_cursorRow);
		else if (_textAlignment == kTextAlignCenter)
			alignOffset = (_textMaxWidth / 2) - (getLineWidth(_cursorRow) / 2);

		_cursorY = _textLines[_cursorRow].y + offset.y - 2;
		_cursorX = getLineWidth(_cursorRow, false, _cursorCol) + alignOffset + offset.x - 1;
	}

	_cursorDirty = true;
}

void MacEditableText::undrawCursor() {
	_cursorState = false;
	_cursorDirty = true;
}

} // End of namespace Graphics
