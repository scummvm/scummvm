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

#ifndef WIDGET_H
#define WIDGET_H

#include "scummsys.h"


class Dialog;

enum {
	WIDGET_ENABLED		= 1 << 0,
	WIDGET_INVISIBLE	= 1 << 1,
	WIDGET_BORDER		= 1 << 2,
	WIDGET_CLEARBG		= 1 << 3,
	WIDGET_WANT_TICKLE	= 1 << 4,
};

/* Widget */
class Widget {
friend class Dialog;
protected:
	Dialog		*_boss;
	Widget		*_next;
	int16		_x, _y;
	uint16		_w, _h;
	uint16		_id;
	int			_flags;
public:
	Widget(Dialog *boss, int x, int y, int w, int h);

	virtual void handleClick(int button) {}
	void draw();

	void setFlags(int flags)	{ _flags |= flags; }
	void clearFlags(int flags)	{ _flags &= ~flags; }
	int getFlags() const		{ return _flags; }

protected:
	virtual void drawWidget(bool hilite) {}
};


/* StaticTextWidget */
class StaticTextWidget : public Widget {
protected:
	const char	*_text;
public:
	StaticTextWidget(Dialog *boss, int x, int y, int w, int h, const char *text);
	void setText(const char *text);
	const char *getText();

protected:
	void drawWidget(bool hilite);
};


/* ButtonWidget */
class ButtonWidget : public StaticTextWidget {
protected:
	uint8	_hotkey;
	uint32	_cmd;
public:
	ButtonWidget(Dialog *boss, int x, int y, int w, int h, const char *label, uint32 cmd);
	void setCmd(uint32 cmd);
	uint32 getCmd();
	void handleClick(int button);
};


#endif
