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

#include "stdafx.h"
#include "PopUpWidget.h"
#include "dialog.h"
#include "newgui.h"
#include "base/engine.h"

#define UP_DOWN_BOX_HEIGHT	10

// Little up/down arrow
static uint32 up_down_arrows[8] = {
	0x00000000,
	0x00001000,
	0x00011100,
	0x00111110,
	0x00000000,
	0x00111110,
	0x00011100,
	0x00001000,
};

//
// PopUpDialog
//

class PopUpDialog : public Dialog {
protected:
	PopUpWidget	*_popUpBoss;
	int			_clickX, _clickY;
	byte		*_buffer;
	int			_selection;
	uint32		_openTime;
public:
	PopUpDialog(PopUpWidget *boss, int clickX, int clickY);
	
	void drawDialog();

	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleMouseWheel(int x, int y, int direction);	// Scroll through entries with scroll wheel
	void handleMouseMoved(int x, int y, int button);	// Redraw selections depending on mouse position
	void handleKeyDown(uint16 ascii, int keycode, int modifiers);	// Scroll through entries with arrow keys etc.

protected:
	void drawMenuEntry(int entry, bool hilite);
	
	int findItem(int x, int y) const;
	void setSelection(int item);
	bool isMouseDown();
	
	void moveUp();
	void moveDown();
};

PopUpDialog::PopUpDialog(PopUpWidget *boss, int clickX, int clickY)
	: Dialog(0, 0, 16, 16),
	_popUpBoss(boss) {
	// Copy the selection index
	_selection = _popUpBoss->_selectedItem;

	// Calculate real popup dimensions
	_x = _popUpBoss->getAbsX() + _popUpBoss->_labelWidth;
	_y = _popUpBoss->getAbsY() - _popUpBoss->_selectedItem * kLineHeight;
	_h = _popUpBoss->_entries.size() * kLineHeight + 2;
	_w = _popUpBoss->_w - 10 - _popUpBoss->_labelWidth;
	
	// Perform clipping / switch to scrolling mode if we don't fit on the screen
	// FIXME - hard coded screen height 200. We really need an API in OSystem to query the
	// screen height, and also OSystem should send out notification messages when the screen
	// resolution changes... we could generalize CommandReceiver and CommandSender.
	if (_h >= 200)
		_h = 199;
	if (_y < 0)
		_y = 0;
	else if (_y + _h >= 200)
		_y = 199 - _h;

	// TODO - implement scrolling if we had to move the menu, or if there are too many entries

	// Remember original mouse position
	_clickX = clickX - _x;
	_clickY = clickY - _y;

	// Time the popup was opened
	_openTime = g_system->get_msecs();
}

void PopUpDialog::drawDialog() {
	// Draw the menu border
	g_gui.hLine(_x, _y, _x+_w - 1, g_gui._color);
	g_gui.hLine(_x, _y + _h - 1, _x + _w - 1, g_gui._shadowcolor);
	g_gui.vLine(_x, _y, _y+_h - 1, g_gui._color);
	g_gui.vLine(_x + _w - 1, _y, _y + _h - 1, g_gui._shadowcolor);

	// Draw the entries
	int count = _popUpBoss->_entries.size();
	for (int i = 0; i < count; i++) {
		drawMenuEntry(i, i == _selection);
	}

	g_gui.addDirtyRect(_x, _y, _w, _h);
}

void PopUpDialog::handleMouseUp(int x, int y, int button, int clickCount) {
	// Mouse was released. If it wasn't moved much since the original mouse down, 
	// let the popup stay open. If it did move, assume the user made his selection.
	int dist = (_clickX - x) * (_clickX - x) + (_clickY - y) * (_clickY - y);
	if (dist > 3 * 3 || g_system->get_msecs() - _openTime > 300) {
		setResult(_selection);
		close();
	}
	_clickX = -1;
	_clickY = -1;
	_openTime = (uint32)-1;
}

void PopUpDialog::handleMouseWheel(int x, int y, int direction) {
	if (direction < 0)
		moveUp();
	else if (direction > 0)
		moveDown();
}

void PopUpDialog::handleMouseMoved(int x, int y, int button) {
	// Compute over which item the mouse is...
	int item = findItem(x, y);

	if (item >= 0 && _popUpBoss->_entries[item].name.size() == 0)
		item = -1;

	if (item == -1 && !isMouseDown())
		return;

	// ...and update the selection accordingly
	setSelection(item);
}

void PopUpDialog::handleKeyDown(uint16 ascii, int keycode, int modifiers) {
	if (keycode == 27) {	// escape
		close();
		return;
	}

	if (isMouseDown())
		return;

	switch (keycode) {
	case '\n':		// enter/return
	case '\r':
		setResult(_selection);
		close();
		break;
	case 256+17:	// up arrow
		moveUp();
		break;
	case 256+18:	// down arrow
		moveDown();
		break;
	case 256+22:	// home
		setSelection(0);
		break;
	case 256+23:	// end
		setSelection(_popUpBoss->_entries.size()-1);
		break;
	}
}

int PopUpDialog::findItem(int x, int y) const {
	if (x >= 0 && x < _w && y >= 0 && y < _h) {
		return (y-2) / kLineHeight;
	}
	return -1;
}

void PopUpDialog::setSelection(int item) {
	if (item != _selection) {
		// Undraw old selection
		if (_selection >= 0)
			drawMenuEntry(_selection, false);

		// Change selection
		_selection = item;

		// Draw new selection
		if (item >= 0)
			drawMenuEntry(item, true);
	}
}

bool PopUpDialog::isMouseDown() {
	// TODO/FIXME - need a way to determine whether any mouse buttons are pressed or not.
	// Sure, we could just count mouse button up/down events, but that is cumbersome and
	// error prone. Would be much nicer to add an API to OSystem for this...
	
	return false;
}

void PopUpDialog::moveUp() {
	if (_selection < 0) {
		setSelection(_popUpBoss->_entries.size() - 1);
	} else if (_selection > 0) {
		int item = _selection;
		do {
			item--;
		} while (item >= 0 && _popUpBoss->_entries[item].name.size() == 0);
		if (item >= 0)
			setSelection(item);
	}
}

void PopUpDialog::moveDown() {
	int lastItem = _popUpBoss->_entries.size() - 1;

	if (_selection < 0) {
		setSelection(0);
	} else if (_selection < lastItem) {
		int item = _selection;
		do {
			item++;
		} while (item <= lastItem && _popUpBoss->_entries[item].name.size() == 0);
		if (item <= lastItem)
			setSelection(item);
	}
}

void PopUpDialog::drawMenuEntry(int entry, bool hilite) {
	// Draw one entry of the popup menu, including selection
	assert(entry >= 0);
	int x = _x + 1;
	int y = _y + 1 + kLineHeight * entry;
	int w = _w - 2;
	Common::String &name = _popUpBoss->_entries[entry].name;

	g_gui.fillRect(x, y, w, kLineHeight, hilite ? g_gui._textcolorhi : g_gui._bgcolor);
	if (name.size() == 0) {
		// Draw a separator
		g_gui.hLine(x - 1, y + kLineHeight / 2, x + w, g_gui._shadowcolor);
		g_gui.hLine(x, y + 1 + kLineHeight / 2, x + w, g_gui._color);
	} else {
		g_gui.drawString(name, x + 1, y + 2, w - 2, hilite ? g_gui._bgcolor : g_gui._textcolor);
	}
	g_gui.addDirtyRect(x, y, w, kLineHeight);
}


#pragma mark -

//
// PopUpWidget
//

PopUpWidget::PopUpWidget(GuiObject *boss, int x, int y, int w, int h, const String &label, uint labelWidth)
	: Widget(boss, x, y - 1, w, h + 2), CommandSender(boss), _label(label), _labelWidth(labelWidth) {
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS;
	_type = kPopUpWidget;

	_selectedItem = -1;

	if (!_label.isEmpty() && _labelWidth == 0)
		_labelWidth = g_gui.getStringWidth(_label);
}

void PopUpWidget::handleMouseDown(int x, int y, int button, int clickCount) {

	if (isEnabled()) {
		PopUpDialog popupDialog(this, x + getAbsX(), y + getAbsY());
		int newSel = popupDialog.runModal();
		if (newSel != -1 && _selectedItem != newSel) {
			_selectedItem = newSel;
			sendCommand(kPopUpItemSelectedCmd, _entries[_selectedItem].tag);
		}
	}
}

void PopUpWidget::appendEntry(const String &entry, uint32 tag) {
	Entry e;
	e.name = entry;
	e.tag = tag;
	_entries.push_back(e);
}

void PopUpWidget::clearEntries() {
	_entries.clear();
	_selectedItem = -1;
}

void PopUpWidget::setSelected(int item) {
	// FIXME
	if (item != _selectedItem) {
		if (item >= 0 && item < _entries.size()) {
			_selectedItem = item;
		} else {
			_selectedItem = -1;
		}
	}
}

void PopUpWidget::drawWidget(bool hilite) {
	NewGui	*gui = &g_gui;
	int x = _x + _labelWidth;
	int w = _w - _labelWidth;

	// Draw the label, if any
	if (_labelWidth > 0)
		gui->drawString(_label, _x, _y + 3, _labelWidth, isEnabled() ? gui->_textcolor : gui->_color, kTextAlignRight);

	// Draw a thin frame around us.
	gui->hLine(x, _y, x + w - 1, gui->_color);
	gui->hLine(x, _y +_h-1, x + w - 1, gui->_shadowcolor);
	gui->vLine(x, _y, _y+_h-1, gui->_color);
	gui->vLine(x + w - 1, _y, _y +_h - 1, gui->_shadowcolor);

	// Draw an arrow pointing down at the right end to signal this is a dropdown/popup
	gui->drawBitmap(up_down_arrows, x+w - 10, _y+2, !isEnabled() ? gui->_color : hilite ? gui->_textcolorhi : gui->_textcolor);

	// Draw the selected entry, if any
	if (_selectedItem >= 0) {
		int align = (gui->getStringWidth(_entries[_selectedItem].name) > w-6) ? kTextAlignRight : kTextAlignLeft;
		gui->drawString(_entries[_selectedItem].name, x+2, _y+3, w-6, !isEnabled() ? gui->_color : gui->_textcolor, align);
	}
}
