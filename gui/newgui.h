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

#ifndef NEWGUI_H
#define NEWGUI_H

#include "scummsys.h"
#include "system.h"	// For events

class Dialog;

#define hline(x, y, x2, color) line(x, y, x2, y, color);
#define vline(x, y, y2, color) line(x, y, x, y2, color);

enum {
	kTextAlignLeft,
	kTextAlignCenter,
	kTextAlignRight,
};

enum {
	kDoubleClickDelay = 500,    // milliseconds
	kCursorAnimateDelay = 500
};

// Extremly simple stack class, doesn't even do any error checking (for now)
class DialogStack {
protected:
	Dialog	*_stack[10];	// Anybody nesting dialogs deeper than 4 is mad anyway
	int		_size;
public:
	DialogStack() : _size(0) {}
	
	bool	empty() const		{ return _size <= 0; }
	void	push(Dialog *d)		{ _stack[_size++] = d; }
	Dialog	*top() const		{ return _stack[_size-1]; }
	void	pop()				{ if (_size > 0) _stack[--_size] = 0; }
	int		size() const		{ return _size; }
	Dialog	*operator [](int i)	{ return _stack[i]; }
};

// This class hopefully will replace the old Gui class completly one day 
class NewGui {
	friend class Dialog;
public:

	// Main entry for the GUI: this will start an event loop that keeps running
	// until no dialogs are active anymore.
	void runLoop();

	bool isActive()	{ return ! _dialogStack.empty(); }

	NewGui(OSystem *system);

protected:
	OSystem		*_system;
	int16		*_screen;
	int			_screen_pitch;
	
	bool		_use_alpha_blending;
	bool		_need_redraw;
	DialogStack	_dialogStack;
	
	// for continuous events (keyDown)
	int			_currentKeyDown, _currentKeyDownFlags;
	int			_keyRepeatLoopCount;
	int			_keyRepeatEvenCount;
	
	// position and time of last mouse click (used to detect double clicks)
	struct {
		int16 x, y;	// Position of mouse when the click occured
		uint32 time;	// Time
		int count;	// How often was it already pressed?
	} _lastClick;
	
	// mouse cursor state
	bool		_oldCursorMode;
	int			_cursorAnimateCounter;
	int			_cursorAnimateTimer;
	byte		_cursor[2048];

	void saveState();
	void restoreState();
	
	void openDialog(Dialog *dialog);
	void closeTopDialog();
	
	void loop();

	void animateCursor();

public:
	// Theme colors
	int16 _color, _shadowcolor;
	int16 _bgcolor;
	int16 _textcolor;
	int16 _textcolorhi;

	// Drawing primitives
	int16 *getBasePtr(int x, int y);
	void box(int x, int y, int width, int height);
	void line(int x, int y, int x2, int y2, int16 color);
	void blendRect(int x, int y, int w, int h, int16 color);
	void fillRect(int x, int y, int w, int h, int16 color);
	void checkerRect(int x, int y, int w, int h, int16 color);
	void frameRect(int x, int y, int w, int h, int16 color);
	void addDirtyRect(int x, int y, int w, int h);
	void drawChar(char c, int x, int y, int16 color);
	int getStringWidth(const char *str);
	int getCharWidth(char c);
	void drawString(const char *str, int x, int y, int w, int16 color, int align = kTextAlignLeft);

	void drawBitmap(uint32 bitmap[8], int x, int y, int16 color);
};

#endif
