/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2003 The ScummVM project
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
#include "common/str.h"
#include "common/list.h"
#include "gui/message.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kOkCmd = 'OK  ',
	kCancelCmd = 'CNCL'
};

// TODO: The default button should be visibly distinct from the alternate button

MessageDialog::MessageDialog(const String &message, const char *defaultButton, const char *altButton)
	: Dialog(30, 20, 260, 124) {
	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	StringList lines;
	const char *str = message.c_str();
	const char *start = str;
	int lineWidth, maxlineWidth = 0;
	int lineCount, okButtonPos, cancelButtonPos;

	while (*str) {
		if (*str == '\n') {
			lineWidth = addLine(lines, start, str - start);
			if (maxlineWidth < lineWidth)
				maxlineWidth = lineWidth;
			start = str + 1;
		}
		++str;
	}

	// Add the last line
	lineWidth = addLine(lines, start, str - start);
	if (maxlineWidth < lineWidth)
		maxlineWidth = lineWidth;

	// Calculate the desired dialog size (maxing out at 300*180 for now)
	_w = maxlineWidth + 20;
	lineCount = lines.size();
	_h = lineCount * kLineHeight + 16;
	if (defaultButton || altButton)
		_h += 24;

	if (_h > 180) {
		lineCount = (180 - 34) / kLineHeight;
		_h = lineCount * kLineHeight + 34;
	}
	_x = (320 - _w) / 2;
	_y = (200 - _h) / 2;

	for (int i = 0; i < lineCount; i++) {
		new StaticTextWidget(this, 10, 10 + i * kLineHeight, maxlineWidth, kLineHeight,
								lines[i], kTextAlignCenter);
	}

	// FIXME - allow for multiple buttons, and return in runModal() which one
	// was selected.
	if (defaultButton && altButton) { 
		okButtonPos = (_w - (kButtonWidth * 2)) / 2;
		cancelButtonPos = ((_w - (kButtonWidth * 2)) / 2) + kButtonWidth + 10;
	} else {
		okButtonPos = cancelButtonPos = (_w-kButtonWidth) / 2;
	}

	if (defaultButton)
		addButton(okButtonPos, _h - 24, defaultButton, kOkCmd, '\n');	// Confirm dialog

	if (altButton)
		addButton(cancelButtonPos, _h - 24, altButton, kCancelCmd, '\27');	// Cancel dialog
}

int MessageDialog::addLine(StringList &lines, const char *line, int size) {
	int width = 0, maxWidth = 0;
	const char *start = line, *pos = line, *end = start + size;
	String tmp;
	NewGui *gui = &g_gui;

	while (pos < end) {
		int w = gui->getCharWidth(*pos);

		// Check if we exceed the maximum line width, if so, split the line.
		// If possible we split at whitespaces.
		if (width + w > 280) {
			// Scan backward till we find a space or we get back to the start
			const char *newPos = pos;
			while (newPos > start && !isspace(*newPos))
				newPos--;
			if (newPos > start)
				pos = newPos;

			// Add the substring from intervall [start, i-1]
			tmp = String(start, pos - start);
			lines.push_back(tmp);

			// Determine the width of the string, and adjust maxWidth accordingly
			width = gui->getStringWidth(tmp);
			if (maxWidth < width)
				maxWidth = width;

			start = pos;
			width = 0;
		} else {
			width += w;
			pos++;
		}
	}

	if (maxWidth < width)
		maxWidth = width;

	if (start < pos) {
		tmp = String(start, pos - start);
		lines.push_back(tmp);
	}
	return maxWidth;
}

void MessageDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kOkCmd) {
		setResult(1);
		close();
	} else if (cmd == kCancelCmd) {
		setResult(2);
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

TimedMessageDialog::TimedMessageDialog(const Common::String &message, uint32 duration)
	: MessageDialog(message, 0, 0) {
	_timer = g_system->get_msecs() + duration;
}

void TimedMessageDialog::handleTickle() {
	MessageDialog::handleTickle();
	if (g_system->get_msecs() > _timer)
		close();
}

} // End of namespace GUI
