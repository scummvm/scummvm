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

class Scumm;
class Dialog;

class NewGui {
	friend class Dialog;
public:
	byte _color, _shadowcolor;
	byte _bgcolor;
	byte _textcolor;
	byte _textcolorhi;

	// Dialogs
	void pauseDialog();
	void saveloadDialog();
	void loop();

	bool isActive()	{ return _active; }

	NewGui(Scumm *s);

protected:
	Scumm		*_s;
	bool		_active;
	bool		_need_redraw;
	Dialog		*_activeDialog;
	
	Dialog		*_pauseDialog;
	Dialog		*_saveLoadDialog;

	// sound state
	bool		_old_soundsPaused;

	// mouse cursor state
	bool		_old_cursor_mode;
	int			_old_cursorHotspotX, _old_cursorHotspotY, _old_cursorWidth, _old_cursorHeight;
	byte		_old_grabbedCursor[2048];
	
	// mouse pos
	struct {
		int16 x,y;
	} _old_mouse;

	void saveState();
	void restoreState();

public:
	// Drawing
	byte *getBasePtr(int x, int y);
	void box(int x, int y, int width, int height);
    void line(int x, int y, int x2, int y2, byte color);
    void clearArea(int x, int y, int w, int h);
	void drawChar(const char c, int x, int y);
	void drawString(const char *str, int x, int y, int w, byte color);

	// 
	const char *queryString(int string);
};

#endif
