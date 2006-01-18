/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace GUI {

class AboutDialog : public Dialog {
	typedef Common::StringList StringList;
protected:
	int			_scrollPos;
	uint32		_scrollTime;
	StringList	_lines;
	uint32		_lineHeight;
	byte		_modifiers;
	bool		_willClose;
	Graphics::Surface	_canvas;

	int xOff, yOff;

	void addLine(const char *str);

public:
	AboutDialog();

	void open();
	void close();
	void drawDialog();
	void handleTickle();
	void handleScreenChanged();
	void handleMouseUp(int x, int y, int button, int clickCount);
	void handleKeyDown(uint16 ascii, int keycode, int modifiers);
	void handleKeyUp(uint16 ascii, int keycode, int modifiers);
};

} // End of namespace GUI

#endif
