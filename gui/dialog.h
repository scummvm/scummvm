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
#include "widget.h"	// For CommandReceiver

class NewGui;

// Some "common" commands sent to handleCommand()
enum {
	kCloseCmd = 'clos'
};

class Dialog : public CommandReceiver {
	friend class Widget;
	friend class NewGui;
protected:
	NewGui	*_gui;
	int16	_x, _y;
	uint16	_w, _h;
	Widget	*_firstWidget;
	Widget	*_mouseWidget;
	Widget  *_focusedWidget;
	bool	_visible;

public:
	Dialog(NewGui *gui, int x, int y, int w, int h)
		: _gui(gui), _x(x), _y(y), _w(w), _h(h), _firstWidget(0),
		  _mouseWidget(0), _focusedWidget(0), _visible(false)
		{}
	virtual ~Dialog() {};

	virtual void open();
	virtual void close();

	virtual void draw();

	virtual void handleTickle(); // Called periodically (in every guiloop() )
	virtual void handleMouseDown(int x, int y, int button, int clickCount);
	virtual void handleMouseUp(int x, int y, int button, int clickCount);
	virtual void handleKeyDown(char key, int modifiers);
	virtual void handleKeyUp(char key, int modifiers);
	virtual void handleMouseMoved(int x, int y, int button);
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
	
	NewGui	*getGui()	{ return _gui; }
	
	bool isVisible() const		{ return _visible; }

protected:
	Widget* findWidget(int x, int y); // Find the widget at pos x,y if any

	void addButton(int x, int y, int w, int h, const char *label, uint32 cmd, char hotkey);
};

#endif
