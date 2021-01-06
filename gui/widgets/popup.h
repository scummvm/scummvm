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

#ifndef GUI_WIDGETS_POPUP_H
#define GUI_WIDGETS_POPUP_H

#include "gui/dialog.h"
#include "gui/widget.h"
#include "common/str.h"
#include "common/array.h"

namespace GUI {

/**
 * Popup or dropdown widget which, when clicked, "pop up" a list of items and
 * lets the user pick on of them.
 *
 * Implementation wise, when the user selects an item, then the specified command
 * is broadcast, with data being equal to the tag value of the selected entry.
 */
class PopUpWidget : public Widget, public CommandSender {
	typedef Common::String String;
	typedef Common::U32String U32String;

	struct Entry {
		U32String	name;
		uint32		tag;
	};
	typedef Common::Array<Entry> EntryList;
protected:
	EntryList		_entries;
	int				_selectedItem;

	int				_leftPadding;
	int				_rightPadding;
	uint32			_cmd;

public:
	PopUpWidget(GuiObject *boss, const String &name, const U32String &tooltip = U32String(), uint32 cmd = 0);
	PopUpWidget(GuiObject *boss, int x, int y, int w, int h, const U32String &tooltip = U32String(), uint32 cmd = 0);

	void handleMouseDown(int x, int y, int button, int clickCount) override;
	void handleMouseWheel(int x, int y, int direction) override;

	void appendEntry(const U32String &entry, uint32 tag = (uint32)-1);
	void appendEntry(const String &entry, uint32 tag = (uint32)-1);
	void clearEntries();
	int numEntries() { return _entries.size(); }

	/** Select the entry at the given index. */
	void setSelected(int item);

	/** Select the first entry matching the given tag. */
	void setSelectedTag(uint32 tag);

	int getSelected() const						{ return _selectedItem; }
	uint32 getSelectedTag() const				{ return (_selectedItem >= 0) ? _entries[_selectedItem].tag : (uint32)-1; }
//	const String& getSelectedString() const		{ return (_selectedItem >= 0) ? _entries[_selectedItem].name : String::emptyString; }

	void handleMouseEntered(int button) override	{ if (_selectedItem != -1) read(_entries[_selectedItem].name); setFlags(WIDGET_HILITED); markAsDirty(); }
	void handleMouseLeft(int button) override	{ clearFlags(WIDGET_HILITED); markAsDirty(); }

	void reflowLayout() override;
protected:
	void drawWidget() override;
};

/**
 * A small dialog showing a list of items and allowing the user to chose one of them
 *
 * Used by PopUpWidget and DropdownButtonWidget.
 */
class PopUpDialog : public Dialog {
protected:
	Widget		*_boss;
	int			_clickX, _clickY;
	int			_selection;
	int			_initialSelection;
	uint32		_openTime;
	bool		_twoColumns;
	int			_entriesPerColumn;

	int			_leftPadding;
	int			_rightPadding;
	int			_lineHeight;

	int			_lastRead;

	typedef Common::Array<Common::U32String> EntryList;
	EntryList		_entries;

public:
	PopUpDialog(Widget *boss, const Common::String &name, int clickX, int clickY);

	void open() override;
	void reflowLayout() override;
	void drawDialog(DrawLayer layerToDraw) override;

	void handleMouseUp(int x, int y, int button, int clickCount) override;
	void handleMouseWheel(int x, int y, int direction) override;	// Scroll through entries with scroll wheel
	void handleMouseMoved(int x, int y, int button) override;	// Redraw selections depending on mouse position
	void handleMouseLeft(int button) override;
	void handleKeyDown(Common::KeyState state) override;	// Scroll through entries with arrow keys etc.

	void setPosition(int x, int y);
	void setPadding(int left, int right);
	void setLineHeight(int lineHeight);
	void setWidth(uint16 width);

	void appendEntry(const Common::U32String &entry);
	void clearEntries();
	void setSelection(int item);

protected:
	void drawMenuEntry(int entry, bool hilite);

	int findItem(int x, int y) const;
	bool isMouseDown();

	void moveUp();
	void moveDown();
	void read(const Common::U32String &str);
};

} // End of namespace GUI

#endif
