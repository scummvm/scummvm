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
#include "dialog.h"
#include "widget.h"
#include "newgui.h"

void Dialog::draw()
{
	Widget *w = _firstWidget;

	_gui->clearArea(_x, _y, _w, _h);
	_gui->box(_x, _y, _w, _h);

	while (w) {
		w->draw();
		w = w->_next;
	}
}

void Dialog::handleClick(int x, int y, int button)
{
	Widget *w = findWidget(x - _x, y - _y);
	if (w)
		w->handleClick(button);
}

void Dialog::handleMouseMoved(int x, int y, int button)
{
	Widget *w = findWidget(x - _x, y - _y);
	if (_mouseWidget != w) {
		if (_mouseWidget)
			_mouseWidget->handleMouseLeft(button);
		if (w)
			w->handleMouseEntered(button);
		_mouseWidget = w;
	}
}


/*
 * Determine the widget at location (x,y) if any. Assumes the coordinates are
 * in the local coordinate system, i.e. relative to the top left of the dialog.
 */
Widget *Dialog::findWidget(int x, int y)
{
	Widget *w = _firstWidget;
	while (w) {
		// Stop as soon as we find a fidget that contains (x,y)
		if (x >= w->_x && x <= w->_x + w->_w && y >= w->_y && y <= w->_y + w->_h)
			break;
		w = w->_next;
	}
	return w;
}

void Dialog::close()
{
	// FIXME - this code should be inside the Gui class, and should be 
	// extended to support nested dialogs.
	_gui->restoreState();
	_gui->_active = false;
	_gui->_activeDialog = 0;
}

void Dialog::addResText(int x, int y, int w, int h, int resID)
{
	// Get the string
	const char *str = _gui->queryString(resID);
	if (!str)
		str = "Dummy!";
	new StaticTextWidget(this, x, y, w, h, str);
}

void Dialog::addButton(int x, int y, int w, int h, char hotkey, const char *label, uint32 cmd)
{
	new ButtonWidget(this, x, y, w, h, label, cmd);
	// TODO - handle hotkey
}

void Dialog::addButton(int x, int y, int w, int h, char hotkey, int resID, uint32 cmd)
{
	// Get the string
	const char *label = _gui->queryString(resID);
	if (!label)
		label = "Dummy!";
	addButton(x, y, w, h, hotkey, label, cmd);
}

#pragma mark -

enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kQuitCmd = 'QUIT'
};

SaveLoadDialog::SaveLoadDialog(NewGui * gui)
:Dialog(gui, 30, 20, 260, 124)
{
	addResText(10, 7, 240, 16, 1);
//  addResText(10, 7, 240, 16, 2);
//  addResText(10, 7, 240, 16, 3);

	addButton(200, 20, 54, 16, 'S', 4, kSaveCmd);	// Save
	addButton(200, 40, 54, 16, 'L', 5, kLoadCmd);	// Load
	addButton(200, 60, 54, 16, 'P', 6, kPlayCmd);	// Play
	addButton(200, 80, 54, 16, 'O', 17, kOptionsCmd);	// Options
	addButton(200, 100, 54, 16, 'Q', 8, kQuitCmd);	// Quit
}

void SaveLoadDialog::handleCommand(uint32 cmd)
{
	switch (cmd) {
	case kSaveCmd:
		break;
	case kLoadCmd:
		break;
	case kPlayCmd:
		close();
		break;
	case kOptionsCmd:
		break;
	case kQuitCmd:
		exit(1);
		break;
	}
}


#pragma mark -


PauseDialog::PauseDialog(NewGui * gui)
:Dialog(gui, 50, 80, 220, 16)
{
	addResText(2, 2, 220, 16, 10);
}
