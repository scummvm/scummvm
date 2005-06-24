/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "common/str.h"
#include "common/system.h"
#include "gui/message.h"
#include "gui/newgui.h"
#include "gui/widget.h"

namespace GUI {

enum {
	kOkCmd = 'OK  ',
	kCancelCmd = 'CNCL'
};

// TODO: The default button should be visibly distinct from the alternate button

MessageDialog::MessageDialog(const Common::String &message, const char *defaultButton, const char *altButton)
	: Dialog(30, 20, 260, 124) {
	
	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	GUI::WidgetSize ws;
	int buttonWidth, buttonHeight;

	if (screenW >= 400 && screenH >= 300) {
		ws = GUI::kBigWidgetSize;
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
	} else {
		ws = GUI::kNormalWidgetSize;
		buttonWidth = kButtonWidth;
		buttonHeight = kButtonHeight;
	}
	
	// First, determine the size the dialog needs. For this we have to break
	// down the string into lines, and taking the maximum of their widths.
	// Using this, and accounting for the space the button(s) need, we can set
	// the real size of the dialog
	Common::StringList lines;
	int lineCount, okButtonPos, cancelButtonPos;
	int maxlineWidth = g_gui.getFont().wordWrapText(message, screenW - 2 * 20, lines);

	// Calculate the desired dialog size (maxing out at 300*180 for now)
	_w = maxlineWidth + 20;
	lineCount = lines.size();

	_h = 16;
	if (defaultButton || altButton)
		_h += buttonHeight + 8;

	// Limit the number of lines so that the dialog still fits on the screen.
	if (lineCount > (screenH - 20 - _h) / kLineHeight) {
		lineCount = (screenH - 20 - _h) / kLineHeight;
	}
	_h += lineCount * kLineHeight;

	// Center the dialog
	_x = (screenW - _w) / 2;
	_y = (screenH - _h) / 2;

	// Each line is represented by one static text item.
	for (int i = 0; i < lineCount; i++) {
		new StaticTextWidget(this, 10, 10 + i * kLineHeight, maxlineWidth, kLineHeight,
								lines[i], kTextAlignCenter, ws);
	}

	// FIXME - allow for more than two buttons, and return in runModal() which one
	// was selected.
	if (defaultButton && altButton) { 
		okButtonPos = (_w - (buttonWidth * 2)) / 2;
		cancelButtonPos = ((_w - (buttonWidth * 2)) / 2) + buttonWidth + 10;
	} else {
		okButtonPos = cancelButtonPos = (_w - buttonWidth) / 2;
	}

	if (defaultButton)
		addButton(this, okButtonPos, _h - buttonHeight - 8, defaultButton, kOkCmd, '\n', ws);	// Confirm dialog

	if (altButton)
		addButton(this, cancelButtonPos, _h - buttonHeight - 8, altButton, kCancelCmd, '\27', ws);	// Cancel dialog
}

void MessageDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	// FIXME: It's a really bad thing that we use two arbitrary constants 
	if (cmd == kOkCmd) {
		setResult(kMessageOK);
		close();
	} else if (cmd == kCancelCmd) {
		setResult(kMessageCancel);
		close();
	} else {
		Dialog::handleCommand(sender, cmd, data);
	}
}

TimedMessageDialog::TimedMessageDialog(const Common::String &message, uint32 duration)
	: MessageDialog(message, 0, 0) {
	_timer = getMillis() + duration;
}

void TimedMessageDialog::handleTickle() {
	MessageDialog::handleTickle();
	if (getMillis() > _timer)
		close();
}

} // End of namespace GUI
