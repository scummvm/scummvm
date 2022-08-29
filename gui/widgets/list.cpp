/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "common/frac.h"
#include "common/tokenizer.h"

#include "gui/widgets/list.h"
#include "gui/widgets/scrollbar.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

namespace GUI {

bool ListWidgetDefaultMatcher(void *, int, const Common::U32String &item, Common::U32String token) {
	return item.contains(token);
}

ListWidget::ListWidget(Dialog *boss, const Common::String &name, const Common::U32String &tooltip, uint32 cmd)
	: EditableWidget(boss, name, tooltip), _cmd(cmd) {

	_entriesPerPage = 0;
	_scrollBarWidth = 0;

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth, 0, _scrollBarWidth, _h);
	_scrollBar->setTarget(this);

	setFlags(WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS | WIDGET_WANT_TICKLE | WIDGET_TRACK_MOUSE);
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

	_quickSelect = true;
	_editColor = ThemeEngine::kFontColorNormal;
	_dictionarySelect = false;

	_filterMatcher = ListWidgetDefaultMatcher;
	_filterMatcherArg = nullptr;

	_lastRead = -1;

	_hlLeftPadding = _hlRightPadding = 0;
	_leftPadding = _rightPadding = 0;
	_topPadding = _bottomPadding = 0;
}

ListWidget::ListWidget(Dialog *boss, int x, int y, int w, int h, const Common::U32String &tooltip, uint32 cmd)
	: EditableWidget(boss, x, y, w, h, tooltip), _cmd(cmd) {

	_entriesPerPage = 0;
	_scrollBarWidth = 0;

	_scrollBar = new ScrollBarWidget(this, _w - _scrollBarWidth, 0, _scrollBarWidth, _h);
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

	_quickSelect = true;
	_editColor = ThemeEngine::kFontColorNormal;
	_dictionarySelect = false;

	_filterMatcher = ListWidgetDefaultMatcher;
	_filterMatcherArg = nullptr;

	_lastRead = -1;

	_hlLeftPadding = _hlRightPadding = 0;
	_leftPadding = _rightPadding = 0;
	_topPadding = _bottomPadding = 0;

	_scrollBarWidth = 0;
}

void ListWidget::copyListData(const Common::U32StringArray &list) {
	Common::U32String stripped;

	_dataList.clear();
	_cleanedList.clear();

	for (uint i = 0; i < list.size(); ++i) {
		stripped = stripGUIformatting(list[i]);

		_dataList.push_back(ListData(list[i], stripped));
		_cleanedList.push_back(stripped);
	}
}


bool ListWidget::containsWidget(Widget *w) const {
	if (w == _scrollBar || _scrollBar->containsWidget(w))
		return true;
	return false;
}

Widget *ListWidget::findWidget(int x, int y) {
	if (x >= _w - _scrollBarWidth)
		return _scrollBar;

	return this;
}

void ListWidget::setSelected(int item) {
	// HACK/FIXME: If our _listIndex has a non zero size,
	// we will need to look up, whether the user selected
	// item is present in that list
	if (!_filter.empty()) {
		int filteredItem = -1;

		for (uint i = 0; i < _listIndex.size(); ++i) {
			if (_listIndex[i] == item) {
				filteredItem = i;
				break;
			}
		}

		item = filteredItem;
	}

	assert(item >= -1 && item < (int)_list.size());

	// We only have to do something if the widget is enabled and the selection actually changes
	if (isEnabled() && _selectedItem != item) {
		if (_editMode)
			abortEditMode();

		_selectedItem = item;

		// Notify clients that the selection changed.
		sendCommand(kListSelectionChangedCmd, _selectedItem);

		_currentPos = _selectedItem - _entriesPerPage / 2;
		scrollToCurrent();
		markAsDirty();
	}
}

void ListWidget::setList(const Common::U32StringArray &list) {
	if (_editMode && _caretVisible)
		drawCaret(true);

	// Copy everything
	copyListData(list);
	_list = list;
	_filter.clear();
	_listIndex.clear();

	int size = list.size();
	if (_currentPos >= size)
		_currentPos = size - 1;
	if (_currentPos < 0)
		_currentPos = 0;
	_selectedItem = -1;
	_editMode = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	scrollBarRecalc();
}

void ListWidget::append(const Common::String &s) {
	Common::U32String stripped = stripGUIformatting(s);
	_dataList.push_back(ListData(s, stripped));
	_cleanedList.push_back(stripped);
	_list.push_back(s);

	setFilter(_filter, false);

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
		checkBounds();
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
	_scrollBar->handleTickle();
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
	markAsDirty();

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

void ListWidget::handleMouseMoved(int x, int y, int button) {
	if (!isEnabled())
		return;

	// Determine if we are inside the widget
	if (x < 0 || x > _w)
		return;

	// First check whether the selection changed
	int item = findItem(x, y);

	if (item != -1) {
		if(_lastRead != item) {
			read(stripGUIformatting(_list[item]));
			_lastRead = item;
		}
	}
	else
		_lastRead = -1;
}

void ListWidget::handleMouseLeft(int button) {
	_lastRead = -1;
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

static int matchingCharsIgnoringCase(const char *x, const char *y, bool &stop, bool dictionary) {
	int match = 0;
	if (dictionary) {
		x = scumm_skipArticle(x);
		y = scumm_skipArticle(y);
	}
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

	if (!_editMode && state.keycode <= Common::KEYCODE_z && Common::isPrint(state.ascii)) {
		// Quick selection mode: Go to first list item starting with this key
		// (or a substring accumulated from the last couple key presses).
		// Only works in a useful fashion if the list entries are sorted.
		uint32 time = g_system->getMillis();
		if (_quickSelectTime < time) {
			_quickSelectStr = (char)state.ascii;
		} else {
			_quickSelectStr += (char)state.ascii;
		}
		_quickSelectTime = time + 300;	// TODO: Turn this into a proper constant (kQuickSelectDelay ?)

		if (_quickSelect) {
			// FIXME: This is bad slow code (it scans the list linearly each time a
			// key is pressed); it could be much faster. Only of importance if we have
			// quite big lists to deal with -- so for now we can live with this lazy
			// implementation :-)
			int newSelectedItem = 0;
			int bestMatch = 0;
			bool stop;
			for (Common::U32StringArray::const_iterator i = _list.begin(); i != _list.end(); ++i) {
				const int match = matchingCharsIgnoringCase(stripGUIformatting(*i).encode().c_str(), _quickSelectStr.c_str(), stop, _dictionarySelect);
				if (match > bestMatch || stop) {
					_selectedItem = newSelectedItem;
					bestMatch = match;
					if (stop)
						break;
				}
				newSelectedItem++;
			}

			scrollToCurrent();
		} else {
			sendCommand(_cmd, 0);
		}
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

		// Keypad & special keys
		//   - if num lock is set, we do not handle the keypress
		//   - if num lock is not set, we either fall down to the special key case
		//     or ignore the key press for 0, 4, 5 and 6

		case Common::KEYCODE_KP_PERIOD:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_BACKSPACE:
		case Common::KEYCODE_DELETE:
			if (_selectedItem >= 0) {
				if (_editable) {
					// Ignore delete and backspace when the list item is editable
				} else {
					sendCommand(kListItemRemovalRequestCmd, _selectedItem);
				}
			}
			break;

		case Common::KEYCODE_KP1:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_END:
			_selectedItem = _list.size() - 1;
			break;


		case Common::KEYCODE_KP2:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_DOWN:
			if (_selectedItem < (int)_list.size() - 1)
				_selectedItem++;
			break;

		case Common::KEYCODE_KP3:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_PAGEDOWN:
			_selectedItem += _entriesPerPage - 1;
			if (_selectedItem >= (int)_list.size() )
				_selectedItem = _list.size() - 1;
			break;

		case Common::KEYCODE_KP7:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_HOME:
			_selectedItem = 0;
			break;

		case Common::KEYCODE_KP8:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_UP:
			if (_selectedItem > 0)
				_selectedItem--;
			break;

		case Common::KEYCODE_KP9:
			if (state.flags & Common::KBD_NUM) {
				handled = false;
				break;
			}
			// fall through
		case Common::KEYCODE_PAGEUP:
			_selectedItem -= _entriesPerPage - 1;
			if (_selectedItem < 0)
				_selectedItem = 0;
			break;

		default:
			handled = false;
		}

		scrollToCurrent();
	}

	if (dirty || _selectedItem != oldSelectedItem)
		markAsDirty();

	if (_selectedItem != oldSelectedItem) {
		sendCommand(kListSelectionChangedCmd, _selectedItem);
		// also draw scrollbar
		_scrollBar->markAsDirty();
	}

	return handled;
}

bool ListWidget::handleKeyUp(Common::KeyState state) {
	if (state.keycode == _currentKeyDown)
		_currentKeyDown = 0;
	return true;
}

void ListWidget::receivedFocusWidget() {
	_inversion = ThemeEngine::kTextInversionFocus;

	// Redraw the widget so the selection color will change
	markAsDirty();
}

void ListWidget::lostFocusWidget() {
	_inversion = ThemeEngine::kTextInversion;

	// If we lose focus, we simply forget the user changes
	_editMode = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	drawCaret(true);
	markAsDirty();
}

void ListWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSetPositionCmd:
		if (_currentPos != (int)data) {
			_currentPos = data;
			checkBounds();
			markAsDirty();

			// Scrollbar actions cause list focus (which triggers a redraw)
			// NOTE: ListWidget's boss is always GUI::Dialog
			((GUI::Dialog *)_boss)->setFocusWidget(this);
		}
		break;
	default:
		break;
	}
}

void ListWidget::drawWidget() {
	int i, pos, len = _list.size();
	Common::U32String buffer;

	// Draw a thin frame around the list.
	g_gui.theme()->drawWidgetBackground(Common::Rect(_x, _y, _x + _w, _y + _h),
	                                    ThemeEngine::kWidgetBackgroundBorder);

	// Draw the list items
	for (i = 0, pos = _currentPos; i < _entriesPerPage && pos < len; i++, pos++) {
		const int y = _y + _topPadding + kLineHeight * i;
		const int fontHeight = g_gui.getFontHeight();
		ThemeEngine::TextInversionState inverted = ThemeEngine::kTextInversionNone;

		// Draw the selected item inverted, on a highlighted background.
		if (_selectedItem == pos)
			inverted = _inversion;

		Common::Rect r(getEditRect());
		int pad = _leftPadding;
		int rtlPad = (_x + r.left + _leftPadding) - (_x + _hlLeftPadding);

		// If in numbering mode & not in RTL based GUI, we first print a number prefix
		if (_numberingMode != kListNumberingOff && g_gui.useRTL() == false) {
			buffer = Common::String::format("%2d. ", (pos + _numberingMode));
			g_gui.theme()->drawText(Common::Rect(_x + _hlLeftPadding, y, _x + r.left + _leftPadding, y + fontHeight),
									buffer, _state, _drawAlign, inverted, _leftPadding, true);
			pad = 0;
		}

		Common::Rect r1(_x + r.left, y, _x + r.right, y + fontHeight);

		if (g_gui.useRTL()) {
			if (_scrollBar->isVisible()) {
				r1.translate(_scrollBarWidth, 0);
			}

			if (_numberingMode != kListNumberingOff) {
				r1.translate(-rtlPad, 0);
			}
		}

		ThemeEngine::FontColor color = ThemeEngine::kFontColorFormatting;

		if (_selectedItem == pos && _editMode) {
			buffer = _editString;
			color = _editColor;
			adjustOffset();
		} else {
			buffer = _list[pos];
		}

		drawFormattedText(r1, buffer, _state, _drawAlign, inverted, pad, true, color);

		// If in numbering mode & using RTL layout in GUI, we print a number suffix after drawing the text
		if (_numberingMode != kListNumberingOff && g_gui.useRTL()) {
			buffer = Common::String::format(" .%2d", (pos + _numberingMode));

			Common::Rect r2 = r1;

			r2.left = r1.right;
			r2.right = r1.right + rtlPad;

			g_gui.theme()->drawText(r2, buffer, _state, _drawAlign, inverted, _leftPadding, true);
		}
	}
}

Common::Rect ListWidget::getEditRect() const {
	const int scrollbarW = (_scrollBar && _scrollBar->isVisible()) ? _scrollBarWidth : 0;
	int editWidth = _w - _hlLeftPadding - _hlRightPadding - scrollbarW;
	// Ensure r will always be a valid rect
	if (editWidth < 0) {
		editWidth = 0;
	}
	Common::Rect r(_hlLeftPadding, 0, _hlLeftPadding + editWidth, g_gui.getFontHeight());
	const int offset = (_selectedItem - _currentPos) * kLineHeight + _topPadding;
	r.top += offset;
	r.bottom += offset;

	if (_numberingMode != kListNumberingOff) {
		// FIXME: Assumes that all digits have the same width.
		Common::String temp = Common::String::format("%2d. ", (_list.size() - 1 + _numberingMode));
		r.left += g_gui.getStringWidth(temp) + _leftPadding;
		// Make sure we don't go farther than right
		if (r.right < r.left) {
			r.right = r.left;
		}
	}

	return r;
}

int ListWidget::getCaretOffset() const {
	Common::U32String substr(_editString.begin(), _editString.begin() + _caretPos);
	Common::U32String stripped = stripGUIformatting(substr);
	return g_gui.getStringWidth(stripped, _font) - _editScrollOffset;
}

void ListWidget::checkBounds() {
	if (_currentPos < 0 || _entriesPerPage > (int)_list.size())
		_currentPos = 0;
	else if (_currentPos + _entriesPerPage > (int)_list.size())
		_currentPos = _list.size() - _entriesPerPage;
}

void ListWidget::scrollToCurrent() {
	// Only do something if the current item is not in our view port
	if (_selectedItem != -1 && _selectedItem < _currentPos) {
		// it's above our view
		_currentPos = _selectedItem;
	} else if (_selectedItem >= _currentPos + _entriesPerPage ) {
		// it's below our view
		_currentPos = _selectedItem - _entriesPerPage + 1;
	}

	checkBounds();
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
	_scrollBar->markAsDirty();
}

void ListWidget::startEditMode() {
	if (_editable && !_editMode && _selectedItem >= 0) {
		_editMode = true;
		setEditString(stripGUIformatting(_list[_selectedItem]));
		_caretPos = _editString.size();	// Force caret to the *end* of the selection.
		_editColor = ThemeEngine::kFontColorNormal;
		markAsDirty();
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
	//markAsDirty();
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
	frac_t entriesPerPage = intToFrac(_h - _topPadding - _bottomPadding) / kLineHeight;

	// Our threshold before we add another entry is 0.9375 (0xF000 with FRAC_BITS being 16).
	const frac_t threshold = intToFrac(15) / 16;

	if ((frac_t)(entriesPerPage & FRAC_LO_MASK) >= threshold)
		entriesPerPage += FRAC_ONE;

	_entriesPerPage = fracToInt(entriesPerPage);
	assert(_entriesPerPage > 0);

	if (_scrollBar) {
		_scrollBar->resize(_w - _scrollBarWidth, 0, _scrollBarWidth, _h, false);
		scrollBarRecalc();
		scrollToCurrent();
	}
}

void ListWidget::setFilter(const Common::U32String &filter, bool redraw) {
	// FIXME: This method does not deal correctly with edit mode!
	// Until we fix that, let's make sure it isn't called while editing takes place
	assert(!_editMode);

	Common::U32String filt = filter;
	filt.toLowercase();

	if (_filter == filt) // Filter was not changed
		return;

	_filter = filt;

	if (_filter.empty()) {
		// No filter -> display everything

		_list.clear();

		for (uint i = 0; i < _dataList.size(); ++i)
			_list.push_back(_dataList[i].orig);

		_listIndex.clear();
	} else {
		// Restrict the list to everything which matches all tokens in _filter, ignoring case.

		Common::U32StringTokenizer tok(_filter);
		Common::U32String tmp;
		int n = 0;

		_list.clear();
		_listIndex.clear();

		for (auto i = _dataList.begin(); i != _dataList.end(); ++i, ++n) {
			tmp = i->clean;
			tmp.toLowercase();
			bool matches = true;
			tok.reset();
			while (!tok.empty()) {
				if (!_filterMatcher(_filterMatcherArg, n, tmp, tok.nextToken())) {
					matches = false;
					break;
				}
			}

			if (matches) {
				_list.push_back(i->orig);
				_listIndex.push_back(n);
			}
		}
	}

	_currentPos = 0;
	_selectedItem = -1;

	if (redraw) {
		scrollBarRecalc();
		// Redraw the whole dialog. This is annoying, as this might be rather
		// expensive when really only the list widget and its scroll bar area
		// to be redrawn. However, since the scrollbar might change its
		// visibility status, and the list its width, we cannot just redraw
		// the two.
		// TODO: A more efficient (and elegant?) way to handle this would be to
		// introduce a kind of "BoxWidget" or "GroupWidget" which defines a
		// rectangular region and subwidgets can be placed within it.
		// Such a widget could also (optionally) draw a border (or even different
		// kinds of borders) around the objects it groups; and also a 'title'
		// (I am borrowing these "ideas" from the NSBox class in Cocoa :).
		g_gui.scheduleTopDialogRedraw();
	}
}

Common::U32String ListWidget::getThemeColor(byte r, byte g, byte b) {
	return Common::U32String::format("\001c%02x%02x%02x", r, g, b);
}

Common::U32String ListWidget::getThemeColor(ThemeEngine::FontColor color) {
	switch (color) {
	case ThemeEngine::kFontColorNormal:
		return Common::U32String("\001C{normal}");
	case ThemeEngine::kFontColorAlternate:
		return Common::U32String("\001C{alternate}");
	default:
		return Common::U32String("\001C{unknown}");
	}
}

ThemeEngine::FontColor ListWidget::getThemeColor(Common::U32String color) {
	if (color == "normal")
		return ThemeEngine::kFontColorNormal;

	if (color == "alternate")
		return ThemeEngine::kFontColorAlternate;

	warning("ListWidget::getThemeColor(): Malformed color (\"%s\")", color.encode().c_str());

	return ThemeEngine::kFontColorNormal;
}

Common::U32String ListWidget::stripGUIformatting(const Common::U32String &str) {
	Common::U32String stripped;
	const uint32 *s = str.u32_str();

	while (*s) {
		if (*s != '\001') { // normal symbol
			stripped += *s++;
			continue;
		}

		s++; // skip \001
		switch (*s) {
		case '\001':  // \001\001 -> \001
			stripped += *s++;
			break;

		case 'c': // \001cRRGGBB
			s += 7; // check length?
			break;

		case 'C': // \001C{color-name}
			while (*s && *s++ != '}')
				;
			break;

		default:
			error("Wrong string format (%c)", *s);
		}
	}

	return stripped;
}

void ListWidget::drawFormattedText(const Common::Rect &r, const Common::U32String &str, ThemeEngine::WidgetStateInfo state,
				Graphics::TextAlign align, ThemeEngine::TextInversionState inverted, int deltax, bool useEllipsis,
				ThemeEngine::FontColor color) {
	Common::U32String chunk;
	const uint32 *s = str.u32_str();
	ThemeEngine::FontStyle curfont = ThemeEngine::kFontStyleBold;
	ThemeEngine::FontColor curcolor = ThemeEngine::kFontColorNormal;
	Common::U32String tmp;

	while (*s) {
		if (*s != '\001') { // normal symbol
			chunk += *s++;
			continue;
		}

		if (chunk.size()) {
			g_gui.theme()->drawText(r, chunk, state, align, inverted, deltax, true, curfont, curcolor);

			deltax += g_gui.theme()->getStringWidth(chunk, curfont);
			chunk.clear();
		}

		s++; // skip \001
		switch (*s) {
		case '\001':  // \001\001 -> \001
			chunk += *s++;
			break;

		case 'c': // \001cRRGGBB
			s += 7; // check length?
			break;

		case 'C': // \001C{color-name}
			tmp.clear();
			s++;
			if (*s == '{')
				s++;
			else
				error("ListWidget::drawFormattedText(): Malformatted \\001C color (%c)", *s);

			while (*s && *s != '}')
				tmp += *s++;

			if (*s == '}')	// skip the closing bracket
				s++;

			if (color == ThemeEngine::kFontColorFormatting)
				curcolor = getThemeColor(tmp);
			else
				curcolor = color;	// Ignore color and use the requested one

			break;

		default:
			error("ListWidget::drawFormattedText(): Wrong string format (\\001%c)", *s);
		}
	}

	if (chunk.size() || str.empty())
		g_gui.theme()->drawText(r, chunk, state, align, inverted, deltax, true, curfont, curcolor);
}

} // End of namespace GUI
