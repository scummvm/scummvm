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
#include "common/translation.h"
#include "common/config-manager.h"

#include "gui/widgets/groupedlist.h"
#include "gui/widgets/scrollbar.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"

#define kGroupTag -2
#define isGroupHeader(x) (_groupsVisible && ((x) <= kGroupTag))
#define indexToGroupID(x) (kGroupTag - x)

namespace GUI {

GroupedListWidget::GroupedListWidget(Dialog *boss, const Common::String &name, const Common::U32String &tooltip, uint32 cmd)
	: ListWidget(boss, name, tooltip, cmd) {
	_groupsVisible = true;
}

void GroupedListWidget::setList(const Common::U32StringArray &list) {
	ListWidget::setList(list);

	_attributeValues.clear();	// Regenerate attributes for the new list
	groupByAttribute();
	scrollBarRecalc();
}

void GroupedListWidget::setAttributeValues(const Common::U32StringArray &attrValues) {
	_attributeValues = attrValues;
	// Make sure we always have the attribute values for all the entries of the _dataList.
	// This is not foolproof, but can prevent accidentally passing attributes for the wrong
	// list of entries, such as after adding / removing games.
	if (!attrValues.empty())
		assert(_attributeValues.size() == _dataList.size());
}

void GroupedListWidget::setMetadataNames(const Common::StringMap &metadata) {
	_metadataNames = metadata;
}

void GroupedListWidget::setGroupHeaderFormat(const Common::U32String &prefix, const Common::U32String &suffix) {
	_groupHeaderPrefix = prefix;
	_groupHeaderSuffix = suffix;
}

void GroupedListWidget::groupByAttribute() {
	_groupExpanded.clear();
	_groupHeaders.clear();
	_groupValueIndex.clear();
	_itemsInGroup.clear();

	if (_attributeValues.empty()) {
		_groupExpanded.push_back(true);
		// I18N: Group for All items
		_groupHeaders.push_back(_("All"));
		_groupValueIndex.setVal(Common::String("All"), 0);
		for (uint i = 0; i < _dataList.size(); ++i) {
			_itemsInGroup[0].push_back(i);
		}
		sortGroups();
		return;
	}

	for (uint i = 0; i < _dataList.size(); ++i) {
		Common::U32StringArray::iterator attrVal = _attributeValues.begin() + i;
		if (!_groupValueIndex.contains(*attrVal)) {
			int newGroupID = _groupValueIndex.size();
			_groupValueIndex.setVal(*attrVal, newGroupID);
			_groupHeaders.push_back(*attrVal);
			_groupExpanded.push_back(true);
		}
		int groupID = _groupValueIndex.getVal(*attrVal);

		_itemsInGroup[groupID].push_back(i);
	}

	sortGroups();
}

void GroupedListWidget::sortGroups() {
	uint oldListSize = _list.size();
	_list.clear();
	_listIndex.clear();

	Common::sort(_groupHeaders.begin(), _groupHeaders.end(),
		[](const Common::String &first, const Common::String &second) {
			return first.empty() ? 0 : second.empty() ? 1 : first < second;
		}

	);

	for (uint i = 0; i != _groupHeaders.size(); ++i) {
		Common::U32String header = _groupHeaders[i];
		Common::U32String displayedHeader;
		if (_metadataNames.contains(header)) {
			displayedHeader = _metadataNames[header];
		} else {
			displayedHeader = header;
		}
		uint groupID = _groupValueIndex[header];

		if (_groupsVisible) {
			_listIndex.push_back(kGroupTag - groupID);

			displayedHeader.toUppercase();

			_list.push_back(_groupHeaderPrefix + displayedHeader + _groupHeaderSuffix);
		}

		if (_groupExpanded[groupID]) {
			for (int *k = _itemsInGroup[groupID].begin(); k != _itemsInGroup[groupID].end(); ++k) {
				_list.push_back(_dataList[*k].orig);
				_listIndex.push_back(*k);
			}
		}
	}
	checkBounds();
	scrollBarRecalc();
	// FIXME: Temporary solution to clear/display the background ofthe scrollbar when list
	// grows too small or large during group toggle. We shouldn't have to redraw the top dialog,
	// but not doing so the background of scrollbar isn't cleared.
	if ((((uint)_scrollBar->_entriesPerPage < oldListSize) && ((uint)_scrollBar->_entriesPerPage > _list.size())) ||
		(((uint)_scrollBar->_entriesPerPage > oldListSize) && ((uint)_scrollBar->_entriesPerPage < _list.size()))) {
		g_gui.scheduleTopDialogRedraw();
	} else {
		markAsDirty();
	}
}

void GroupedListWidget::loadClosedGroups(const Common::U32String &groupName) {
	// Recalls what groups were closed from the config
	if (ConfMan.hasKey("group_" + groupName, ConfMan.kApplicationDomain)) {
		const Common::String &val = ConfMan.get("group_" + groupName, ConfMan.kApplicationDomain);
		Common::StringTokenizer hiddenGroups(val);

		for (Common::String tok = hiddenGroups.nextToken(); tok.size(); tok = hiddenGroups.nextToken()) {
			// See if the hidden group is in our group headers still, if so, hide it
			for (Common::U32StringArray::size_type i = 0; i < _groupHeaders.size(); ++i) {
				if (_groupHeaders[i] == tok || (tok == "unnamed" && _groupHeaders[i].size() == 0)) {
					_groupExpanded[i] = false;
					break;
				}
			}
		}
		sortGroups();
	}
}

void GroupedListWidget::saveClosedGroups(const Common::U32String &groupName) {
	// Save the hidden groups to the config
	Common::String hiddenGroups;
	for (Common::U32StringArray::size_type i = 0; i < _groupHeaders.size(); ++i) {
		if (!_groupExpanded[i]) {
			if (_groupHeaders[i].size()) {
				hiddenGroups += _groupHeaders[i];
			} else {
				hiddenGroups += "unnamed";
			}
			hiddenGroups += ' ';
		}
	}
	ConfMan.set("group_" + groupName, hiddenGroups, ConfMan.kApplicationDomain);
	ConfMan.flushToDisk();
}

void GroupedListWidget::setSelected(int item) {
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

	if (item < -1 || item >= (int)_list.size())
		return;

	// We only have to do something if the widget is enabled and the selection actually changes
	if (isEnabled() && (_selectedItem == -1 || _selectedItem >= (int)_list.size() || _listIndex[_selectedItem] != item)) {
		if (_editMode)
			abortEditMode();

		if (!_filter.empty()) {
			_selectedItem = item;
		} else {
			_selectedItem = -1;
			for (uint i = 0; i < _listIndex.size(); ++i) {
				if (_listIndex[i] == item) {
					_selectedItem = i;
					break;
				}
			}
		}

		// Notify clients that the selection changed.
		sendCommand(kListSelectionChangedCmd, _selectedItem);

		_currentPos = _selectedItem - _entriesPerPage / 2;
		scrollToCurrent();
		markAsDirty();
	}
}

void GroupedListWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (!isEnabled())
		return;

	// First check whether the selection changed
	int newSelectedItem = findItem(x, y);
	if (_selectedItem != newSelectedItem && newSelectedItem != -1) {
		if (_listIndex[newSelectedItem] > -1) {
			if (_editMode)
				abortEditMode();
			_selectedItem = newSelectedItem;
			sendCommand(kListSelectionChangedCmd, _selectedItem);
		} else if (isGroupHeader(_listIndex[newSelectedItem])) {
			int groupID = indexToGroupID(_listIndex[newSelectedItem]);
			_selectedItem = -1;
			toggleGroup(groupID);
		}
	}

	// TODO: Determine where inside the string the user clicked and place the
	// caret accordingly.
	// See _editScrollOffset and EditTextWidget::handleMouseDown.
	markAsDirty();

}

void GroupedListWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// If this was a double click and the mouse is still over
	// the selected item, send the double click command
	if (clickCount == 2 && (_selectedItem == findItem(x, y))) {
		int selectID = getSelected();
		if (selectID >= 0) {
			sendCommand(kListItemDoubleClickedCmd, _selectedItem);
		}
	}
}

void GroupedListWidget::handleMouseWheel(int x, int y, int direction) {
	_scrollBar->handleMouseWheel(x, y, direction);
}

void GroupedListWidget::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
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

void GroupedListWidget::toggleGroup(int groupID) {
	_groupExpanded[groupID] = !_groupExpanded[groupID];
	sortGroups();
}

void GroupedListWidget::drawWidget() {
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
#if 0
		ThemeEngine::FontStyle bold = ThemeEngine::kFontStyleBold;
#endif

		// Draw the selected item inverted, on a highlighted background.
		if (_selectedItem == pos)
			inverted = _inversion;

		Common::Rect r(getEditRect());
		int pad = _leftPadding;
		int rtlPad = (_x + r.left + _leftPadding) - (_x + _hlLeftPadding);

		if (isGroupHeader(_listIndex[pos])) {
			int groupID = indexToGroupID(_listIndex[pos]);
#if 0
			bold = ThemeEngine::kFontStyleBold;
#endif
			r.left += fontHeight + _leftPadding;
			g_gui.theme()->drawFoldIndicator(Common::Rect(_x + _hlLeftPadding + _leftPadding, y, _x + fontHeight + _leftPadding, y + fontHeight), _groupExpanded[groupID]);
			pad = 0;
		} else if (_groupsVisible) {
			r.left += fontHeight + _leftPadding;
			r.right -= _rightPadding;
			pad = 0;
		}

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

void GroupedListWidget::setFilter(const Common::U32String &filter, bool redraw) {
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
		sortGroups();
	} else {
		// Restrict the list to everything which contains all words in _filter
		// as substrings, ignoring case.

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

} // End of namespace GUI
