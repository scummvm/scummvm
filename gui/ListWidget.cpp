/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/system.h"
#include "gui/ListWidget.h"
#include "gui/ScrollBarWidget.h"
#include "gui/dialog.h"
#include "gui/newgui.h"

namespace GUI {

ListWidget::ListWidget(GuiObject *boss, int x, int y, int w, int h, WidgetSize ws)
	: EditableWidget(boss, x, y, w, h, ws), CommandSender(boss) {

	if (ws == kBigWidgetSize) {
		_w = w - kBigScrollBarWidth;
	} else {
		_w = w - kNormalScrollBarWidth;
	}

	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE;
	_type = kListWidget;
	_editMode = false;
	_numberingMode = kListNumberingOne;
	_entriesPerPage = (_h - 2) / kLineHeight;
	_currentPos = 0;
	_selectedItem = -1;
	_scrollBar = new ScrollBarWidget(boss, _x + _w, _y, (ws == kBigWidgetSize ? kBigScrollBarWidth : kNormalScrollBarWidth), _h);
	_scrollBar->setTarget(this);
	_currentKeyDown = 0;

	_quickSelectTime = 0;

	// The item is selected, thus _bgcolor is used to draw the caret and _textcolorhi to erase it
	_caretInverse = true;

	// FIXME: This flag should come from widget definition
	_editable = true;
}

ListWidget::~ListWidget() {
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
	int newSelectedItem;
	newSelectedItem = findItem(x, y);
	if (newSelectedItem > (int)_list.size() - 1)
		newSelectedItem = -1;

	if (_selectedItem != newSelectedItem) {
		if (_editMode)
			abortEditMode();
		_selectedItem = newSelectedItem;
		sendCommand(kListSelectionChangedCmd, _selectedItem);
	}

	// TODO: Determine where inside the string the user clicked and place the
	// caret accordingly. See _editScrollOffset and EditTextWidget::handleMouseDown.
	draw();

}

void ListWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// If this was a double click and the mouse is still over the selected item,
	// send the double click command
	if (clickCount == 2 && (_selectedItem == findItem(x, y))) {
		sendCommand(kListItemDoubleClickedCmd, _selectedItem);
	}
}

void ListWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}


int ListWidget::findItem(int x, int y) const {
	return (y - 1) / kLineHeight + _currentPos;
}

static int matchingCharsIgnoringCase(const char *x, const char *y, bool &stop) {
	int match = 0;
	while (*x && *y && toupper(*x) == toupper(*y)) {
		++x;
		++y;
		++match;
	}
	stop = !*y || (*x && (toupper(*x) >= toupper(*y)));
	return match;
}

bool ListWidget::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	bool handled = true;
	bool dirty = false;
	int oldSelectedItem = _selectedItem;

	if (!_editMode && isprint((char)ascii)) {
		// Quick selection mode: Go to first list item starting with this key
		// (or a substring accumulated from the last couple key presses).
		// Only works in a useful fashion if the list entries are sorted.
		// TODO: Maybe this should be off by default, and instead we add a
		// method "enableQuickSelect()" or so ?
		uint32 time = getMillis();
		if (_quickSelectTime < time) {
			_quickSelectStr = (char)ascii;
		} else {
			_quickSelectStr += (char)ascii;
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
		handled = EditableWidget::handleKeyDown(ascii, keycode, modifiers);
	} else {
		// not editmode

		switch (keycode) {
		case '\n':	// enter/return
		case '\r':
			if (_selectedItem >= 0) {
				// override continuous enter keydown
				if (_editable && (_currentKeyDown != '\n' && _currentKeyDown != '\r')) {
					dirty = true;
					startEditMode();
				} else
					sendCommand(kListItemActivatedCmd, _selectedItem);
			}
			break;
		case 256+17:	// up arrow
			if (_selectedItem > 0)
				_selectedItem--;
			break;
		case 256+18:	// down arrow
			if (_selectedItem < (int)_list.size() - 1)
				_selectedItem++;
			break;
		case 256+24:	// pageup
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;
		case 256+25:	// pagedown
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= (int)_list.size() )
				_selectedItem = _list.size() - 1;
			break;
		case 256+22:	// home
			_selectedItem = 0;
			break;
		case 256+23:	// end
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
	_currentKeyDown = keycode;
#endif

	return handled;
}

bool ListWidget::handleKeyUp(uint16 ascii, int keycode, int modifiers) {
	if (keycode == _currentKeyDown)
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

void ListWidget::drawWidget(bool hilite) {
	NewGui *gui = &g_gui;
	int i, pos, len = _list.size();
	Common::String buffer;
	int deltax;

	// Draw a thin frame around the list.
	gui->hLine(_x, _y, _x + _w - 1, gui->_color);
	gui->hLine(_x, _y + _h - 1, _x + _w - 1, gui->_shadowcolor);
	gui->vLine(_x, _y, _y + _h - 1, gui->_color);

	// Draw the list items
	for (i = 0, pos = _currentPos; i < _entriesPerPage && pos < len; i++, pos++) {
		const OverlayColor textColor = (_selectedItem == pos && _hasFocus) ? gui->_bgcolor : gui->_textcolor;
		const int y = _y + 2 + kLineHeight * i;

		// Draw the selected item inverted, on a highlighted background.
		if (_selectedItem == pos) {
			if (_hasFocus)
				gui->fillRect(_x + 1, _y + 1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
			else
				gui->frameRect(_x + 1, _y + 1 + kLineHeight * i, _w - 1, kLineHeight, gui->_textcolorhi);
		}

		// If in numbering mode, we first print a number prefix
		if (_numberingMode != kListNumberingOff) {
			char temp[10];
			sprintf(temp, "%2d. ", (pos + _numberingMode));
			buffer = temp;
			gui->drawString(buffer, _x + 2, y, _w - 4, textColor);
		}

		Common::Rect r(getEditRect());
		if (_selectedItem == pos && _editMode) {

			buffer = _editString;
			adjustOffset();
			deltax = -_editScrollOffset;

			gui->drawString(buffer, _x + r.left, y, r.width(), textColor, kTextAlignLeft, deltax, false);
		} else {
			buffer = _list[pos];
			deltax = 0;
			gui->drawString(buffer, _x + r.left, y, r.width(), textColor);
		}
	}
}

Common::Rect ListWidget::getEditRect() const {
	Common::Rect r(2, 1, _w - 2 , kLineHeight);
	const int offset = (_selectedItem - _currentPos) * kLineHeight;
	r.top += offset;
	r.bottom += offset;

	if (_numberingMode != kListNumberingOff) {
		char temp[10];
		// FIXME: Assumes that all digits have the same width.
		sprintf(temp, "%2d. ", (_list.size() - 1 + _numberingMode));
		r.left += g_gui.getStringWidth(temp);
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

} // End of namespace GUI
