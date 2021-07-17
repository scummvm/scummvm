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

#include "common/system.h"
#include "common/frac.h"
#include "common/tokenizer.h"

#include "gui/widgets/groupedlist.h"
#include "gui/widgets/scrollbar.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"

#include "gui/ThemeEval.h"
#define kGroupTag -2

namespace GUI {

GroupedListWidget::GroupedListWidget(Dialog *boss, const String &name, const Common::U32String &tooltip, uint32 cmd)
	: ListWidget(boss, name, tooltip, cmd) {

}

void GroupedListWidget::setList(const U32StringArray &list, const ColorList *colors, const U32StringArray *attrValues) {
	if (_editMode && _caretVisible)
		drawCaret(true);

	// Copy everything
	_dataList = list;
	_list = list;

	_filter.clear();
	_listIndex.clear();
	_listColors.clear();

	if (colors) {
		_listColors = *colors;
		assert(_listColors.size() == _dataList.size());
	}

	if (attrValues) {
		_attributeValues = *attrValues;
		assert(_attributeValues.size() == _dataList.size());
	}

	int size = list.size();
	if (_currentPos >= size)
		_currentPos = size - 1;
	if (_currentPos < 0)
		_currentPos = 0;
	_selectedItem = -1;
	_editMode = false;
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	groupByAttribute();
	scrollBarRecalc();
}

void GroupedListWidget::append(const String &s, ThemeEngine::FontColor color) {
	if (_dataList.size() == _listColors.size()) {
		// If the color list has the size of the data list, we append the color.
		_listColors.push_back(color);
	} else if (!_listColors.size() && color != ThemeEngine::kFontColorNormal) {
		// If it's the first entry to use a non default color, we will fill
		// up all other entries of the color list with the default color and
		// add the requested color for the new entry.
		for (uint i = 0; i < _dataList.size(); ++i)
			_listColors.push_back(ThemeEngine::kFontColorNormal);
		_listColors.push_back(color);
	}

	_dataList.push_back(s);
	_list.push_back(s);

	setFilter(_filter, false);

	scrollBarRecalc();
}

void GroupedListWidget::groupByAttribute() {
	if (_attributeValues.empty()) return;

	_groupExpanded.clear();
	_groupHeaders.clear();
	_groupValueIndex.clear();
	_itemsInGroup.clear();

	for (uint i = 0; i < _dataList.size(); ++i) {
		int groupID;
		U32StringArray::iterator attrVal = _attributeValues.begin() + i;
		if (!_groupValueIndex.contains(*attrVal)) {
			int newGroupID = _groupValueIndex.size();
			_groupValueIndex.setVal(*attrVal, newGroupID);
			_groupHeaders.push_back(*attrVal);
			_groupExpanded.push_back(true);
		}
		groupID = _groupValueIndex.getVal(*attrVal);
		
		_itemsInGroup[groupID].push_back(i);
	}

	sortGroups();
}

void GroupedListWidget::sortGroups() {
	int oldListSize = _list.size();
	_list.clear();
	_listIndex.clear();

	for (uint groupID = 0; groupID != _groupHeaders.size(); ++groupID) {
		_listColors.push_back(_listColors.front());
		_listIndex.push_back(kGroupTag - groupID);
		_list.push_back(_groupHeaders[groupID]);
		if (_groupExpanded[groupID]) {
			for (auto k = _itemsInGroup[groupID].begin(); k != _itemsInGroup[groupID].end(); ++k) {
				_list.push_back(Common::U32String("    ") + _dataList[*k]);
				_listIndex.push_back(*k);
			}
		}
	}
	checkBounds();
	scrollBarRecalc();
	// FIXME: Temporary solution to clear/display the background ofthe scrollbar when list 
	// grows too small or large during group toggle. We shouldn't have to redraw the top dialog, 
	// but not doing so the background of scrollbar isn't cleared.
	if (((_scrollBar->_entriesPerPage < oldListSize) && (_scrollBar->_entriesPerPage > _list.size())) ||
		((_scrollBar->_entriesPerPage > oldListSize) && (_scrollBar->_entriesPerPage < _list.size()))) {
		g_gui.scheduleTopDialogRedraw();
	} else {
		markAsDirty();
	}
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

void GroupedListWidget::handleMouseDown(int x, int y, int button, int clickCount) {
	if (!isEnabled())
		return;

	// First check whether the selection changed
	int newSelectedItem = findItem(x, y);
	int selectID = getSelected();
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

void GroupedListWidget::handleMouseUp(int x, int y, int button, int clickCount) {
	// If this was a double click and the mouse is still over
	// the selected item, send the double click command
	if (clickCount == 2 && (_selectedItem == findItem(x, y))) {
		int selectID = getSelected();
		if (selectID >= 0) {
			sendCommand(kListItemDoubleClickedCmd, _selectedItem);
		} else if (selectID <= kGroupTag) {
			int groupID = -selectID + kGroupTag;
			toggleGroup(groupID);
			warning("%d", groupID);
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

void GroupedListWidget::reflowLayout() {
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

void GroupedListWidget::toggleGroup(int groupID) {
	// Shrink group if it is expanded
	if (_groupExpanded[groupID]) {
		_groupExpanded[groupID] = false;
		sortGroups();
	} else {
	// Expand group if it is shrunk
		_groupExpanded[groupID] = true;
		sortGroups();
	}
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
		const int fontHeight = kLineHeight;
		bool isGroupHeader = false;
		ThemeEngine::TextInversionState inverted = ThemeEngine::kTextInversionNone;

		// Draw the selected item inverted, on a highlighted background.
		if (_selectedItem == pos)
			inverted = _inversion;

		Common::Rect r(getEditRect());
		int pad = _leftPadding;
		int rtlPad = (_x + r.left + _leftPadding) - (_x + _hlLeftPadding);

		if (_listIndex[pos] <= kGroupTag) {
			int groupID = -_listIndex[pos] + kGroupTag;
			isGroupHeader = true;
			r.left += g_gui.getStringWidth(Common::U32String(" > "), ThemeEngine::kFontStyleConsole) + _leftPadding;
			// TODO: Placeholder for expansion / contraction icons (triangles).
			if (_groupExpanded[groupID])
				buffer = Common::String(" v ");
			else
				buffer = Common::String(" > ");
			g_gui.theme()->drawText(Common::Rect(_x + _hlLeftPadding, y, _x + r.left + _leftPadding, y + fontHeight - 2),
									buffer, _state, _drawAlign, inverted, _leftPadding, true, ThemeEngine::kFontStyleConsole);
			pad = 0;
		}

		// If in numbering mode & not in RTL based GUI, we first print a number prefix
		if (_numberingMode != kListNumberingOff && g_gui.useRTL() == false) {
			buffer = Common::String::format("%2d. ", (pos + _numberingMode));
			g_gui.theme()->drawText(Common::Rect(_x + _hlLeftPadding, y, _x + r.left + _leftPadding, y + fontHeight - 2),
									buffer, _state, _drawAlign, inverted, _leftPadding, true);
			pad = 0;
		}

		ThemeEngine::FontColor color = ThemeEngine::kFontColorNormal;

		if (!_listColors.empty()) {
			if (_filter.empty() || _selectedItem == -1)
				color = _listColors[pos];
			else
				color = _listColors[_listIndex[pos]];
		}

		Common::Rect r1(_x + r.left, y, _x + r.right, y + fontHeight - 2);

		if (g_gui.useRTL()) {
			if (_scrollBar->isVisible()) {
				r1.translate(_scrollBarWidth, 0);
			}

			if (_numberingMode != kListNumberingOff) {
				r1.translate(-rtlPad, 0);
			}
		}

		if (_selectedItem == pos && _editMode) {
			buffer = _editString;
			color = _editColor;
			adjustOffset();
		} else {
			buffer = _list[pos];
		}
		if (isGroupHeader)
			g_gui.theme()->drawText(r1, buffer, _state,
								_drawAlign, inverted, pad, true, ThemeEngine::kFontStyleBold, color);
		else
			g_gui.theme()->drawText(r1, buffer, _state,
								_drawAlign, inverted, pad, true, ThemeEngine::kFontStyleNormal, color);

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

void GroupedListWidget::scrollToCurrent() {
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

} // End of namespace GUI
