/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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

#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include "gui/widget.h"
#include "common/str.h"
#include "common/list.h"

namespace GUI {

class ScrollBarWidget;

enum NumberingMode {
	kListNumberingOff	= -1,
	kListNumberingZero	= 0,
	kListNumberingOne	= 1
};

// Some special commands
enum {
	kListItemDoubleClickedCmd	= 'LIdb',	// double click on item - 'data' will be item index
	kListItemActivatedCmd		= 'LIac',	// item activated by return/enter - 'data' will be item index
	kListSelectionChangedCmd	= 'Lsch'	// selection changed - 'data' will be item index
};

/* ListWidget */
class ListWidget : public Widget, public CommandSender {
	typedef Common::StringList StringList;
	typedef Common::String String;
protected:
	StringList		_list;
	bool			_editable;
	bool			_editMode;
	NumberingMode	_numberingMode;
	int				_currentPos;
	int				_entriesPerPage;
	int				_selectedItem;
	ScrollBarWidget	*_scrollBar;
	int				_currentKeyDown;
	String			_backupString;
	bool			_caretVisible;
	uint32			_caretTime;
public:
	ListWidget(GuiObject *boss, int x, int y, int w, int h);
	virtual ~ListWidget();
	
	void setList(const StringList& list);
	const StringList& getList()	const			{ return _list; }
	int getSelected() const						{ return _selectedItem; }
	const String& getSelectedString() const		{ return _list[_selectedItem]; }
	void setNumberingMode(NumberingMode numberingMode)	{ _numberingMode = numberingMode; }
	bool isEditable() const						{ return _editable; }
	void setEditable(bool editable)				{ _editable = editable; }
	void scrollTo(int item);
	
	virtual void handleTickle();
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleMouseWheel(int x, int y, int direction);
	virtual bool handleKeyDown(uint16 ascii, int keycode, int modifiers);
	virtual bool handleKeyUp(uint16 ascii, int keycode, int modifiers);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

	virtual bool wantsFocus() { return true; };

	void scrollBarRecalc();
	
	void startEditMode();
	void abortEditMode();

protected:
	void drawWidget(bool hilite);
	void drawCaret(bool erase);
	void lostFocusWidget();
	void scrollToCurrent();
};

} // End of namespace GUI

#endif
