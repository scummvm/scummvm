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
 */

#include "common/system.h"
#include "common/events.h"
#include "gui/ListWidget.h"
#include "gui/ScrollBarWidget.h"
#include "gui/dialog.h"
#include "gui/GuiManager.h"

#include "gui/ThemeEval.h"

namespace GUI {

ListWidget::ListWidget(GuiObject *boss, const String &name)
	: EditableWidget(boss, name), CommandSender(boss) {

	_scrollBar = NULL;
	_textWidth = NULL;

	// This ensures that _entriesPerPage is properly initialised.
	reflowLayout();

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth + 1, 0, _scrollBarWidth, _h);
	_scrollBar->setTarget(this);

	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kListWidget;
	_editMode = false;
	_numberingMode = kListNumberingOne;
	_currentPos = 0;
	_selectedItem = -1;
	_currentKeyDown = 0;

	_quickSelectTime = 0;

	// The item is selected, thus _bgcolor is used to draw the caret and _textcolorhi to erase it
	_caretInverse = true;

	// FIXME: This flag should come from widget definition
	_editable = true;
}

ListWidget::ListWidget(GuiObject *boss, int x, int y, int w, int h)
	: EditableWidget(boss, x, y, w, h), CommandSender(boss) {

	_scrollBar = NULL;
	_textWidth = NULL;

	// This ensures that _entriesPerPage is properly initialised.
	reflowLayout();

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth + 1, 0, _scrollBarWidth, _h);
	_scrollBar->setTarget(this);

	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE);
	_type = kListWidget;
	_editMode = false;
	_numberingMode = kListNumberingOne;
	_currentPos = 0;
	_selectedItem = -1;
	_currentKeyDown = 0;

	_quickSelectTime = 0;

	// The item is selected, thus _bgcolor is used to draw the caret and _textcolorhi to erase it
	_caretInverse = true;

	// FIXME: This flag should come from widget definition
	_editable = true;
}

ListWidget::~ListWidget() {
	delete[] _textWidth;
}

Widget *ListWidget::findWidget(int x, int y) {
	if (x >= _w - _scrollBarWidth)
		return _scrollBar;

	return this;
}

void ListWidget::setSelected(int item) {
	assert(item >= -1 && item < (int)_list.size());

	if (isEnabled() && _selectedItem != item) {
		if (_editMode)
			abortEditMode();

		_selectedItem = item;
		sendCommand(kListSelectionChangedCmd, _selectedItem);

		_currentPos = _selectedItem - _entriesPerPage / 2;
		scrollToCurrent();
		draw();
	}
}

void ListWidget::setList(const StringList &list) {
	if (_editMode && _caretVisible)
		drawCaret(true);
	int size = list.size();
	_list = list;
	if (_currentPos >= size)
		_currentPos = size - 1;
	if (_currentPos < 0)
		_currentPos = 0;
	_selectedItem = -1;
	_editMode = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	scrollBarRecalc();
}

void ListWidget::append(const String &s) {
	_list.push_back(s);
	scrollBarRecalc();
}

void ListWidget::scrollTo(int item) {
	int size = _list.size();
	if (item >= size)
		item = size - 1;
	if (item < 0)
		item = 0;

	if (_currentPos != item) {
		_currentPos = item;
		scrollBarRecalc();
	}
}

void ListWidget::scrollBarRecalc() {
	_scrollBar->_numEntries = _list.size();
	_scrollBar->_entriesPerPage = _entriesPerPage;
	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

void ListWidget::handleTickle() {
	if (_editMode)
		EditableWidget::handleTickle();
}

void ListWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (!isEnabled())
		return;

	// First check whether the selection changed
	int newSelectedItem = findItem(x, y);

	if (_selectedItem != newSelectedItem && newSelectedItem != -1) {
		if (_editMode)
			abortEditMode();
		_selectedItem = newSelectedItem;
		sendCommand(kListSelectionChangedCmd, _selectedItem);
	}

	// TODO: Determine where inside the string the user clicked and place the
	// caret accordingly.
	// See _editScrollOffset and EditTextWidget::handleMouseDown.
	draw();

}

void ListWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// If this was a double click and the mouse is still over
	// the selected item, send the double click command
	if (clickCount == 2 && (_selectedItem == findItem(x, y)) &&
		_selectedItem >= 0) {
		sendCommand(kListItemDoubleClickedCmd, _selectedItem);
	}
}

void ListWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}


int ListWidget::findItem(int x, int y) const {
	if (y < _topPadding) return -1;
	int item = (y - _topPadding) / kLineHeight + _currentPos;
	if (item >= _currentPos && item < _currentPos + _entriesPerPage &&
		item < (int)_list.size())
		return item;
	else
		return -1;
}

static int matchingCharsIgnoringCase(const char *x, const char *y, bool &stop) {
	int match = 0;
	while (*x && *y && tolower(*x) == tolower(*y)) {
		++x;
		++y;
		++match;
	}
	stop = !*y || (*x && (tolower(*x) >= tolower(*y)));
	return match;
}

bool ListWidget::handleKeyDown(Common::KeyState state) {
	bool handled = true;
	bool dirty = false;
	int oldSelectedItem = _selectedItem;

	if (!_editMode && isprint((char)state.ascii)) {
		// Quick selection mode: Go to first list item starting with this key
		// (or a substring accumulated from the last couple key presses).
		// Only works in a useful fashion if the list entries are sorted.
		// TODO: Maybe this should be off by default, and instead we add a
		// method "enableQuickSelect()" or so ?
		uint32 time = getMillis();
		if (_quickSelectTime < time) {
			_quickSelectStr = (char)state.ascii;
		} else {
			_quickSelectStr += (char)state.ascii;
		}
		_quickSelectTime = time + 300;	// TODO: Turn this into a proper constant (kQuickSelectDelay ?)


		// FIXME: This is bad slow code (it scans the list linearly each time a
		// key is pressed); it could be much faster. Only of importance if we have
		// quite big lists to deal with -- so for now we can live with this lazy
		// implementation :-)
		int newSelectedItem = 0;
		int bestMatch = 0;
		bool stop;
		for (StringList::const_iterator i = _list.begin(); i != _list.end(); ++i) {
			const int match = matchingCharsIgnoringCase(i->c_str(), _quickSelectStr.c_str(), stop);
			if (match > bestMatch || stop) {
				_selectedItem = newSelectedItem;
				bestMatch = match;
				if (stop)
					break;
			}
			newSelectedItem++;
		}

		scrollToCurrent();
	} else if (_editMode) {
		// Class EditableWidget handles all text editing related key presses for us
		handled = EditableWidget::handleKeyDown(state);
	} else {
		// not editmode

		switch (state.keycode) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			if (_selectedItem >= 0) {
				// override continuous enter keydown
				if (_editable && (_currentKeyDown != Common::KEYCODE_RETURN && _currentKeyDown != Common::KEYCODE_KP_ENTER)) {
					dirty = true;
					startEditMode();
				} else
					sendCommand(kListItemActivatedCmd, _selectedItem);
			}
			break;
		case Common::KEYCODE_BACKSPACE:
		case Common::KEYCODE_KP_PERIOD:
		case Common::KEYCODE_DELETE:
			if (_selectedItem >= 0) {
				if (_editable) {
					// Ignore delete and backspace when the list item is editable
				} else {
					sendCommand(kListItemRemovalRequestCmd, _selectedItem);
				}
			}
			break;
		case Common::KEYCODE_UP:
			if (_selectedItem > 0)
				_selectedItem--;
			break;
		case Common::KEYCODE_DOWN:
			if (_selectedItem < (int)_list.size() - 1)
				_selectedItem++;
			break;
		case Common::KEYCODE_PAGEUP:
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;
		case Common::KEYCODE_PAGEDOWN:
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= (int)_list.size() )
				_selectedItem = _list.size() - 1;
			break;
		case Common::KEYCODE_HOME:
			_selectedItem = 0;
			break;
		case Common::KEYCODE_END:
			_selectedItem = _list.size() - 1;
			break;
		default:
			handled = false;
		}

		scrollToCurrent();
	}

	if (dirty || _selectedItem != oldSelectedItem)
		draw();

	if (_selectedItem != oldSelectedItem) {
		sendCommand(kListSelectionChangedCmd, _selectedItem);
		// also draw scrollbar
		_scrollBar->draw();
	}

#if !defined(PALMOS_MODE)
	// not done on PalmOS because keyboard is emulated and keyup is not generated
	_currentKeyDown = state.keycode;
#endif

	return handled;
}

bool ListWidget::handleKeyUp(Common::KeyState state) {
	if (state.keycode == _currentKeyDown)
		_currentKeyDown = 0;
	return true;
}

void ListWidget::lostFocusWidget() {
	// If we loose focus, we simply forget the user changes
	_editMode = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	drawCaret(true);
	draw();
}

void ListWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		if (_currentPos != (int)data) {
			_currentPos = data;
			draw();
		}
		break;
	}
}

void ListWidget::drawWidget() {
	int i, pos, len = _list.size();
	Common::String buffer;

	// Draw a thin frame around the list.
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h), 0, ThemeEngine::kWidgetBackgroundBorder);
	const int scrollbarW = (_scrollBar && _scrollBar->isVisible()) ? _scrollBarWidth : 0;

	// Draw the list items
	for (i = 0, pos = _currentPos; i < _entriesPerPage && pos < len; i++, pos++) {
		const int y = _y + _topPadding + kLineHeight * i;
		const int fontHeight = kLineHeight;
		bool inverted = false;

		// Draw the selected item inverted, on a highlighted background.
		if (_selectedItem == pos) {
			if (_hasFocus)
				inverted = true;
			else
				g_gui.theme()->drawWidgetBackground(Common::Rect(_x, y - 1, _x + _w - 1, y + fontHeight - 1),
													0, ThemeEngine::kWidgetBackgroundBorderSmall);
		}

		Common::Rect r(getEditRect());
		int pad = _leftPadding;

		// If in numbering mode, we first print a number prefix
		if (_numberingMode != kListNumberingOff) {
			char temp[10];
			sprintf(temp, "%2d. ", (pos + _numberingMode));
			buffer = temp;
			g_gui.theme()->drawText(Common::Rect(_x, y, _x + r.left + _leftPadding, y + fontHeight - 2),
									buffer, _state, Graphics::kTextAlignLeft, inverted, _leftPadding, true);
			pad = 0;
		}

		int width;

		if (_selectedItem == pos && _editMode) {
			buffer = _editString;
			adjustOffset();
			width = _w - r.left - _hlRightPadding - _leftPadding - scrollbarW;
			g_gui.theme()->drawText(Common::Rect(_x + r.left, y, _x + r.left + width, y + fontHeight - 2),
									buffer, _state, Graphics::kTextAlignLeft, inverted, pad, true);
		} else {
			buffer = _list[pos];
			width = _w - r.left - scrollbarW;
			g_gui.theme()->drawText(Common::Rect(_x + r.left, y, _x + r.left + width, y + fontHeight - 2),
									buffer, _state, Graphics::kTextAlignLeft, inverted, pad, true);
		}

		_textWidth[i] = width;
	}
}

Common::Rect ListWidget::getEditRect() const {
	Common::Rect r(_hlLeftPadding, 0, _w - _hlLeftPadding - _hlRightPadding, kLineHeight - 1);
	const int offset = (_selectedItem - _currentPos) * kLineHeight + _topPadding;
	r.top += offset;
	r.bottom += offset;

	if (_numberingMode != kListNumberingOff) {
		char temp[10];
		// FIXME: Assumes that all digits have the same width.
		sprintf(temp, "%2d. ", (_list.size() - 1 + _numberingMode));
		r.left += g_gui.getStringWidth(temp) + _leftPadding;
	}

	return r;
}

void ListWidget::scrollToCurrent() {
	// Only do something if the current item is not in our view port
	if (_selectedItem < _currentPos) {
		// it's above our view
		_currentPos = _selectedItem;
	} else if (_selectedItem >= _currentPos + _entriesPerPage ) {
		// it's below our view
		_currentPos = _selectedItem - _entriesPerPage + 1;
	}

	if (_currentPos < 0 || _entriesPerPage > (int)_list.size())
		_currentPos = 0;
	else if (_currentPos + _entriesPerPage > (int)_list.size())
		_currentPos = _list.size() - _entriesPerPage;

	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
}

void ListWidget::scrollToEnd() {
	if (_currentPos + _entriesPerPage < (int)_list.size()) {
		_currentPos = _list.size() - _entriesPerPage;
	} else {
		return;
	}

	_scrollBar->_currentPos = _currentPos;
	_scrollBar->recalc();
	_scrollBar->draw();
}

void ListWidget::startEditMode() {
	if (_editable && !_editMode && _selectedItem >= 0) {
		_editMode = true;
		setEditString(_list[_selectedItem]);
		draw();
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	}
}

void ListWidget::endEditMode() {
	if (!_editMode)
		return;
	// send a message that editing finished with a return/enter key press
	_editMode = false;
	_list[_selectedItem] = _editString;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	sendCommand(kListItemActivatedCmd, _selectedItem);
}

void ListWidget::abortEditMode() {
	// undo any changes made
	assert(_selectedItem >= 0);
	_editMode = false;
	//drawCaret(true);
	//draw();
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void ListWidget::reflowLayout() {
	Widget::reflowLayout();

	_leftPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.Padding.Left", 0);
	_rightPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.Padding.Right", 0);
	_topPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.Padding.Top", 0);
	_bottomPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.Padding.Bottom", 0);
	_hlLeftPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.hlLeftPadding", 0);
	_hlRightPadding = g_gui.xmlEval()->getVar("Globals.ListWidget.hlRightPadding", 0);

	_scrollBarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 0);

	// HACK: Once we take padding into account, there are times where
	// integer rounding leaves a big chunk of white space in the bottom
	// of the list.
	// We do a rough rounding on the decimal places of Entries Per Page,
	// to add another entry even if it goes a tad over the padding.
	_entriesPerPage = ((_h - _topPadding - _bottomPadding) << 16) / kLineHeight;

	if ((uint)(_entriesPerPage & 0xFFFF) >= 0xF000)
		_entriesPerPage += (1 << 16);

	_entriesPerPage >>= 16;

	assert(_entriesPerPage > 0);

	delete[] _textWidth;
	_textWidth = new int[_entriesPerPage];

	for (int i = 0; i < _entriesPerPage; i++)
		_textWidth[i] = 0;

	if (_scrollBar) {
		_scrollBar->resize(_w - _scrollBarWidth + 1, 0, _scrollBarWidth, _h);
		scrollBarRecalc();
		scrollToCurrent();
	}
}

} // End of namespace GUI
