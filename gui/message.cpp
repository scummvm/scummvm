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
#include "message.h"
#include "newgui.h"

#include "common/list.h"


MessageDialog::MessageDialog(NewGui *gui, const String &message)
	: Dialog(gui, 30, 20, 260, 124)
{
	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	ScummVM::StringList lines;
	String tmp;
	const char *str = message.c_str();
	const char *start = str;
	int lineWidth, maxlineWidth = 0;
	
	while (*str) {
		if (*str == '\n') {
			tmp = String(start, str - start);
			lines.push_back(tmp);
			lineWidth = _gui->getStringWidth(tmp);
			if (maxlineWidth < lineWidth)
				maxlineWidth = lineWidth;
			start = str + 1;
		}
		
		++str;
	}

	// Add in the last line
	tmp = String(start, str - start);
	lines.push_back(tmp);
	lineWidth = _gui->getStringWidth(tmp);
	if (maxlineWidth < lineWidth)
		maxlineWidth = lineWidth;
	
	// TODO - we should probably check for over/underflows here
	_w = maxlineWidth + 20;
	_h = lines.size() * kLineHeight + 34;
	_x = (320 - _w) / 2;
	
	for (int i = 0; i < lines.size(); i++) {
		new StaticTextWidget(this, 10, 10+i*kLineHeight, maxlineWidth, kLineHeight,
								lines[i], kTextAlignCenter);
	}

	// FIXME - the vertical position has to be adjusted
	addButton((_w - kButtonWidth)/2, _h - 24, "OK", kCloseCmd, '\n');	// Confirm dialog
}
