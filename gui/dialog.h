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

#ifndef DIALOG_H
#define DIALOG_H

#include "scummsys.h"
#include "widget.h"

class NewGui;

#define RES_STRING(id)		_gui->queryResString(id)
#define CUSTOM_STRING(id)	_gui->queryCustomString(id)

// Some "common" commands sent to handleCommand()
enum {
	kCloseCmd = 'clos'
};

class Dialog : public CommandReceiver {
	friend class Widget;
protected:
	NewGui	*_gui;
	Widget	*_firstWidget;
	int16	_x, _y;
	uint16	_w, _h;
	Widget	*_mouseWidget;
	byte	*_screenBuf;

public:
	Dialog(NewGui *gui, int x, int y, int w, int h)
		: _gui(gui), _firstWidget(0), _x(x), _y(y), _w(w), _h(h), _mouseWidget(0), _screenBuf(0)
		{}
	virtual ~Dialog();

	virtual void draw();

	//virtual void handleIdle(); // Called periodically
	virtual void handleMouseDown(int x, int y, int button);
	virtual void handleMouseUp(int x, int y, int button);
	virtual void handleKeyDown(char key, int modifiers); // modifiers = alt/shift/ctrl etc.
	virtual void handleKeyUp(char key, int modifiers); // modifiers = alt/shift/ctrl etc.
	virtual void handleMouseMoved(int x, int y, int button);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	
	NewGui	*getGui()	{ return _gui; }
	
	void	setupScreenBuf();
	void	teardownScreenBuf();

protected:
	Widget* findWidget(int x, int y); // Find the widget at pos x,y if any
	void close();

	void addResText(int x, int y, int w, int h, int resID);
	void addButton(int x, int y, int w, int h, const char *label, uint32 cmd, char hotkey);
};


class SaveLoadDialog : public Dialog {
public:
	SaveLoadDialog(NewGui *gui);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
protected:
	Widget* _savegameList;
};


class SoundDialog;
class KeysDialog;
class MiscDialog;

class OptionsDialog : public Dialog {
protected:
	SoundDialog		*_soundDialog;
	KeysDialog		*_keysDialog;
	MiscDialog		*_miscDialog;

public:
	OptionsDialog(NewGui *gui);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

class PauseDialog : public Dialog {
public:
	PauseDialog(NewGui *gui);

	virtual void handleMouseDown(int x, int y, int button)
		{ close(); }
	virtual void handleKeyDown(char key, int modifiers)
		{
			if (key == 32)
				close();
			else
				Dialog::handleKeyDown(key, modifiers);
		}
};

#endif
