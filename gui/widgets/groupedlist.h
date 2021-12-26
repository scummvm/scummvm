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

#ifndef GUI_WIDGETS_GROUPEDLIST_H
#define GUI_WIDGETS_GROUPEDLIST_H

#include "gui/widgets/list.h"

namespace GUI {

class ListWidget;

/* GroupedListWidget */
class GroupedListWidget : public ListWidget {
protected:
	Common::String								_groupingAttribute;
	Common::HashMap<Common::U32String, int>		_groupValueIndex;
	Common::Array<bool>							_groupExpanded;
	Common::U32String							_groupHeaderPrefix;
	Common::U32String							_groupHeaderSuffix;
	Common::U32StringArray						_groupHeaders;
	Common::U32StringArray						_attributeValues;
	Common::StringMap							_metadataNames;
	Common::HashMap<int, Common::Array<int> >	_itemsInGroup;
	bool _groupsVisible;

public:
	GroupedListWidget(Dialog *boss, const Common::String &name, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0);
	GroupedListWidget(Dialog *boss, int x, int y, int w, int h, const Common::U32String &tooltip = Common::U32String(), uint32 cmd = 0);

	void setList(const Common::U32StringArray &list, const ColorList *colors = nullptr);
	void setAttributeValues(const Common::U32StringArray &attrValues);
	void setMetadataNames(const Common::StringMap &metadata);
	const Common::U32StringArray &getList() const { return _dataList; }

	void append(const Common::String &s, ThemeEngine::FontColor color = ThemeEngine::kFontColorNormal);
	void setGroupHeaderFormat(const Common::U32String &prefix, const Common::U32String &suffix);
	void groupByAttribute();

	void setSelected(int item);
	int getSelected() const { return (_selectedItem == -1) ? _selectedItem : _listIndex[_selectedItem]; }

	void setFilter(const Common::U32String &filter, bool redraw = true);

	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseWheel(int x, int y, int direction) override;
	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;

	void reflowLayout() override;

	void setGroupsVisibility(bool val) { _groupsVisible = val; }

	void startEditMode() override { error("Edit mode is not supported for Grouped Lists"); }

protected:
	void sortGroups();
	void toggleGroup(int groupID);
	void drawWidget() override;

	void scrollToCurrent();
};

} // End of namespace GUI

#endif
