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

#include <ctype.h>

#include "stdafx.h"
#include "newgui.h"
#include "dialog.h"
#include "widget.h"
#include "ListWidget.h"

Dialog::~Dialog()
{
	teardownScreenBuf();
}

void Dialog::setupScreenBuf()
{
	// Create _screenBuf if it doesn't already exist
	if (!_screenBuf)
		_screenBuf = new byte[320*200];
	
	// Draw the fixed parts of the dialog: background and border.
	_gui->blendRect(_x, _y, _w, _h, _gui->_bgcolor);
	_gui->box(_x, _y, _w, _h);

	// Draw a bgcolor rectangle for all widgets which have WIDGET_CLEARBG set. 
	Widget *w = _firstWidget;
	while (w) {
		if (w->_flags & WIDGET_CLEARBG)
			_gui->fillRect(_x + w->_x, _y + w->_y, w->_w, w->_h, _gui->_bgcolor);
		// FIXME - should we also draw borders here if WIDGET_BORDER is set?
		w = w->_next;
	}
	
	// Finally blit this to _screenBuf
	_gui->blitTo(_screenBuf, _x, _y, _w, _h); 
}

void Dialog::teardownScreenBuf()
{
	if (_screenBuf) {
		delete [] _screenBuf;
		_screenBuf = 0;
	}
}

void Dialog::draw()
{
	Widget *w = _firstWidget;

	if (_screenBuf) {
		_gui->blitFrom(_screenBuf, _x, _y, _w, _h); 
	} else {
		_gui->fillRect(_x, _y, _w, _h, _gui->_bgcolor);
		_gui->box(_x, _y, _w, _h);
	}
	_gui->addDirtyRect(_x, _y, _w, _h);

	while (w) {
		w->draw();
		w = w->_next;
	}
}

void Dialog::handleClick(int x, int y, int button)
{
	Widget *w = findWidget(x - _x, y - _y);
	if (w)
		w->handleClick(x - _x - w->_x, y - _y - w->_y, button);
}

void Dialog::handleKey(char key, int modifiers)
{
	// ESC closes all dialogs by default
	if (key == 27)
		close();
	
	// Hotkey handling
	Widget *w = _firstWidget;
	key = toupper(key);
	while (w) {
		if (w->_type == kButtonWidget && key == toupper(((ButtonWidget *)w)->_hotkey)) {
			w->handleClick(0, 0, 1);
			break;
		}
		w = w->_next;
	}
	
	// TODO - introduce the notion of a "focused" widget which receives
	// key events (by calling its handleKey method). Required for editfields
	// and also for an editable list widget.
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

	if (!w)
		return;
	
	if (w->getFlags() & WIDGET_TRACK_MOUSE) {
		w->handleMouseMoved(x - _x - w->_x, y - _y - w->_y, button);
	}
}


void Dialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
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

void Dialog::addButton(int x, int y, int w, int h, const char *label, uint32 cmd, char hotkey)
{
	new ButtonWidget(this, x, y, w, h, label, cmd, hotkey);
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

	addButton(200, 20, 54, 16, RES_STRING(4), kSaveCmd, 'S');	// Save
	addButton(200, 40, 54, 16, RES_STRING(5), kLoadCmd, 'L');	// Load
	addButton(200, 60, 54, 16, RES_STRING(6), kPlayCmd, 'P');	// Play
	addButton(200, 80, 54, 16, CUSTOM_STRING(17), kOptionsCmd, 'O');	// Options
	addButton(200, 100, 54, 16, RES_STRING(8), kQuitCmd, 'Q');	// Quit
	
	// FIXME - test
	new CheckboxWidget(this, 10, 20, 90, 16, "Toggle me", 0);

	// FIXME - test
	new SliderWidget(this, 110, 20, 80, 16, "Volume", 0);
	
	// FIXME - test
	_savegameList = new ListWidget(this, 10, 40, 180, 74);
}

void SaveLoadDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
{
	switch (cmd) {
	case kSaveCmd:
		//printf("Saving game in slot %d\n", _savegameList->getSelected());
		break;
	case kLoadCmd:
		//printf("Loading game in slot %d\n", _savegameList->getSelected());
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
		Dialog::handleCommand(sender, cmd, data);
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
	addButton( 10, 10, 40, 15, CUSTOM_STRING(5), kSoundCmd, 'S');	// Sound
	addButton( 80, 10, 40, 15, CUSTOM_STRING(6), kKeysCmd, 'K');	// Keys
	addButton(150, 10, 40, 15, CUSTOM_STRING(7), kAboutCmd, 'A');	// About
	addButton( 10, 35, 40, 15, CUSTOM_STRING(18), kMiscCmd, 'M');	// Misc
	addButton(150, 35, 40, 15, CUSTOM_STRING(23), kCloseCmd, 'C');	// Close dialog - FIXME
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data)
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
		Dialog::handleCommand(sender, cmd, data);
	}
}


#pragma mark -


PauseDialog::PauseDialog(NewGui *gui)
	: Dialog (gui, 50, 80, 220, 16)
{
	addResText(4, 4, 220, 16, 10);
}
