/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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

/* TODO:
 * - draw an (unselectable) sepeator line for items that start with a '-'
 * - handle long lists by allowing scrolling (a lot of work if done right, 
 *   so I will probably only implement if we really need it)
 * - ...
 */

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

const ScummVM::String PopUpWidget::emptyStr;

class PopUpDialog : public Dialog {
protected:
	PopUpWidget	*_popUpBoss;
	int			_clickX, _clickY;
	byte		*_buffer;
	int			_selection;
public:
	PopUpDialog(PopUpWidget *boss, int clickX, int clickY);
	
	void drawDialog();

	void handleMouseDown(int x, int y, int button, int clickCount);
	void handleMouseUp(int x, int y, int button, int clickCount);
//	void handleMouseWheel(int x, int y, int direction);	// Scroll through entries with scroll wheel
	void handleMouseMoved(int x, int y, int button);	// Redraw selections depending on mouse position
//	bool handleKeyDown(uint16 ascii, int keycode, int modifiers);	// Scroll through entries with arrow keys etc.
//	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
//	void backupMenuBackground();
//	void restoreMenuBackground();
	
	void drawMenuEntry(int entry, bool hilite);
	
	int findItem(int x, int y) const;
};

PopUpDialog::PopUpDialog(PopUpWidget *boss, int clickX, int clickY)
	: Dialog(boss->_boss->getGui(), 0, 0, 16, 16),
	_popUpBoss(boss)
{
	// Calculate real popup dimensions
	_x = _popUpBoss->_boss->getX() + _popUpBoss->_x;
	_y = _popUpBoss->_boss->getY() + _popUpBoss->_y - _popUpBoss->_selectedItem * kLineHeight;
	_h = _popUpBoss->_entries.size() * kLineHeight + 2;
	_w = _popUpBoss->_w - 10;
	
	// Copy the selection index
	_selection = _popUpBoss->_selectedItem;
	
	// TODO - perform clipping / switch to scrolling mode if we don't fit on the screen
	
	// TODO - backup background here

	// Remember original mouse position
	_clickX = clickX - _x;
	_clickY = clickY - _y;
}

void PopUpDialog::drawDialog()
{
	// Draw the menu border
//	_gui->box(_x, _y, _w, _h);
	_gui->hline(_x, _y, _x+_w-1, _gui->_color);
	_gui->hline(_x, _y+_h-1, _x+_w-1, _gui->_shadowcolor);
	_gui->vline(_x, _y, _y+_h-1, _gui->_color);
	_gui->vline(_x+_w-1, _y, _y+_h-1, _gui->_shadowcolor);

	// Draw the entries
	int count = _popUpBoss->_entries.size();
	for (int i = 0; i < count; i++) {
		drawMenuEntry(i, i == _selection);
	}

	_gui->addDirtyRect(_x, _y, _w, _h);
}

void PopUpDialog::handleMouseDown(int x, int y, int button, int clickCount)
{
}


void PopUpDialog::handleMouseUp(int x, int y, int button, int clickCount)
{
	// Mouse was released. If it wasn't moved much since the original mouse down, 
	// let the popup stay open. If it did move, assume the user made his selection.
	int dist = (_clickX - x) * (_clickX - x) + (_clickY - y) * (_clickY - y);
	if (dist > 3*3) {
		setResult(_selection);
		close();
	}
	_clickX = -1;
	_clickY = -1;
}

void PopUpDialog::handleMouseMoved(int x, int y, int button)
{
	// Compute over which item
	int item = findItem(x, y);
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

int PopUpDialog::findItem(int x, int y) const
{
	if (x >= 0 && x < _w && y >= 0 && y < _h) {
		return (y-2) / kLineHeight;
	}
	return _popUpBoss->_selectedItem;
}

/*
void PopUpWidget::backupMenuBackground()
{
	NewGui	*gui = _boss->getGui();

	assert(_menu.buffer);
	gui->blitToBuffer(_menu.x1, _menu.y1, _menu.w, _menu.h, _menu.buffer, _menu.w * 2);
}

void PopUpWidget::restoreMenuBackground()
{
	NewGui	*gui = _boss->getGui();

	assert(_menu.buffer);
	gui->blitFromBuffer(_menu.x1, _menu.y1, _menu.w, _menu.h, _menu.buffer, _menu.w * 2);
	gui->addDirtyRect(_menu.x1, _menu.y1, _menu.w, _menu.h);
	draw();
}
*/

void PopUpDialog::drawMenuEntry(int entry, bool hilite)
{
	// Draw one entry of the popup menu, including selection
	assert(entry >= 0);
	int x = _x + 1;
	int y = _y + 1 + kLineHeight * entry;
	int w = _w - 2;

	_gui->fillRect(x, y, w, kLineHeight,
						hilite ? _gui->_textcolorhi : _gui->_bgcolor);
	_gui->drawString(_popUpBoss->_entries[entry].name, x+1, y+1, w-2,
						hilite ? _gui->_bgcolor : _gui->_textcolor);
	_gui->addDirtyRect(x, y, w, kLineHeight);
}

//
//
//
//
//


PopUpWidget::PopUpWidget(Dialog *boss, int x, int y, int w, int h)
	: Widget(boss, x, y-1, w, h+2), CommandSender(boss)
{
	_flags = WIDGET_ENABLED | WIDGET_CLEARBG | WIDGET_RETAIN_FOCUS;
	_type = 'POPU';

	_selectedItem = -1;
}

void PopUpWidget::handleMouseDown(int x, int y, int button, int clickCount)
{
	PopUpDialog popupDialog(this, x + _x + _boss->getX(), y + _y + _boss->getY());
	int newSel = popupDialog.runModal();
	if (newSel != -1 && _selectedItem != newSel) {
		_selectedItem = newSel;
		sendCommand(kPopUpItemSelectedCmd, _selectedItem);
	}
}

void PopUpWidget::appendEntry(const String &entry, uint32 tag)
{
	Entry e;
	e.name = entry;
	e.tag = tag;
	_entries.push_back(e);
}

void PopUpWidget::clearEntries()
{
	_entries.clear();
	_selectedItem = -1;
}

void PopUpWidget::setSelected(int item)
{
	// FIXME
	if (item != _selectedItem) {
		if (item >= 0 && item < _entries.size()) {
			_selectedItem = item;
		} else {
			_selectedItem = -1;
		}
	}
}

void PopUpWidget::drawWidget(bool hilite)
{
	NewGui	*gui = _boss->getGui();

	// Draw a thin frame around us.
	// TODO - should look different than the EditTextWidget fram
	gui->hline(_x, _y, _x+_w-1, gui->_color);
	gui->hline(_x, _y+_h-1, _x+_w-1, gui->_shadowcolor);
	gui->vline(_x, _y, _y+_h-1, gui->_color);
	gui->vline(_x+_w-1, _y, _y+_h-1, gui->_shadowcolor);
	
	// Draw an arrow pointing down at the right end to signal this is a dropdown/popup
	gui->drawBitmap(up_down_arrows, _x+_w - 10, _y+2, hilite ? gui->_textcolorhi : gui->_textcolor);
	
	// Draw the selected entry, if any
	if (_selectedItem >= 0) {
		int align = (gui->getStringWidth(_entries[_selectedItem].name) > _w-6) ? kTextAlignRight : kTextAlignLeft;
		gui->drawString(_entries[_selectedItem].name, _x+2, _y+3, _w-6, gui->_textcolor, align);
	}
}
