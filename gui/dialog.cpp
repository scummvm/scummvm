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
	_gui->setAreaDirty(_x, _y, _w, _h);

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


void Dialog::handleCommand(uint32 cmd)
{
	switch (cmd) {
	case kCloseCmd:
		close();
		break;
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
	_gui->closeTopDialog();
}

void Dialog::addResText(int x, int y, int w, int h, int resID)
{
	// Get the string
	const char *str = _gui->queryResString(resID);
	if (!str)
		str = "Dummy!";
	new StaticTextWidget(this, x, y, w, h, str);
}

void Dialog::addButton(int x, int y, int w, int h, char hotkey, const char *label, uint32 cmd)
{
	new ButtonWidget(this, x, y, w, h, label, cmd);
	// TODO - handle hotkey
}

#pragma mark -


enum {
	kSaveCmd = 'SAVE',
	kLoadCmd = 'LOAD',
	kPlayCmd = 'PLAY',
	kOptionsCmd = 'OPTN',
	kQuitCmd = 'QUIT'
};

SaveLoadDialog::SaveLoadDialog(NewGui *gui)
	: Dialog (gui, 30, 20, 260, 124)
{
	addResText(10, 7, 240, 16, 1);
//  addResText(10, 7, 240, 16, 2);
//  addResText(10, 7, 240, 16, 3);

	addButton(200, 20, 54, 16, 'S', RES_STRING(4), kSaveCmd);	// Save
	addButton(200, 40, 54, 16, 'L', RES_STRING(5), kLoadCmd);	// Load
	addButton(200, 60, 54, 16, 'P', RES_STRING(6), kPlayCmd);	// Play
	addButton(200, 80, 54, 16, 'O', CUSTOM_STRING(17), kOptionsCmd);	// Options
	addButton(200, 100, 54, 16, 'Q', RES_STRING(8), kQuitCmd);	// Quit
	
	// FIXME - test
	new CheckboxWidget(this, 50, 20, 100, 16, "Toggle me", 0);
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
		_gui->optionsDialog();
		break;
	case kQuitCmd:
		exit(1);
		break;
	default:
		Dialog::handleCommand(cmd);
	}
}


#pragma mark -

enum {
	kSoundCmd = 'SOUN',
	kKeysCmd = 'KEYS',
	kAboutCmd = 'ABOU',
	kMiscCmd = 'OPTN'
};

OptionsDialog::OptionsDialog(NewGui *gui)
	: Dialog (gui, 50, 80, 210, 60)
{
	addButton( 10, 10, 40, 15, 'S', CUSTOM_STRING(5), kSoundCmd);	// Sound
	addButton( 80, 10, 40, 15, 'K', CUSTOM_STRING(6), kKeysCmd);	// Keys
	addButton(150, 10, 40, 15, 'A', CUSTOM_STRING(7), kAboutCmd);	// About
	addButton( 10, 35, 40, 15, 'M', CUSTOM_STRING(18), kMiscCmd);	// Misc
	addButton(150, 35, 40, 15, 'C', CUSTOM_STRING(23), kCloseCmd);	// Close dialog - FIXME
}

void OptionsDialog::handleCommand(uint32 cmd)
{
	switch (cmd) {
	case kSoundCmd:
		break;
	case kKeysCmd:
		break;
	case kAboutCmd:
		_gui->aboutDialog();
		break;
	case kMiscCmd:
		break;
	default:
		Dialog::handleCommand(cmd);
	}
}


#pragma mark -


PauseDialog::PauseDialog(NewGui *gui)
	: Dialog (gui, 50, 80, 220, 16)
{
	addResText(4, 4, 220, 16, 10);
}
