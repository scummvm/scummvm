/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * $URL$
 * $Id$
 */

#include "common/events.h"
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


enum {
	kButtonWidth = 72,	// FIXME: Get rid of this
	kButtonHeight = 16,	// FIXME: Get rid of this

	kBigButtonWidth = 108,	// FIXME: Get rid of this
	kBigButtonHeight = 24	// FIXME: Get rid of this
};


// TODO: The default button should be visibly distinct from the alternate button

MessageDialog::MessageDialog(const Common::String &message, const char *defaultButton, const char *altButton)
	: Dialog(30, 20, 260, 124) {

	const int screenW = g_system->getOverlayWidth();
	const int screenH = g_system->getOverlayHeight();

	int buttonWidth, buttonHeight;

	if (g_gui.getWidgetSize() == kBigWidgetSize) {
		buttonWidth = kBigButtonWidth;
		buttonHeight = kBigButtonHeight;
	} else {
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
								lines[i], kTextAlignCenter);
	}

	if (defaultButton && altButton) {
		okButtonPos = (_w - (buttonWidth * 2)) / 2;
		cancelButtonPos = ((_w - (buttonWidth * 2)) / 2) + buttonWidth + 10;
	} else {
		okButtonPos = cancelButtonPos = (_w - buttonWidth) / 2;
	}

	if (defaultButton)
		addButton(this, okButtonPos, _h - buttonHeight - 8, defaultButton, kOkCmd, Common::ASCII_RETURN);	// Confirm dialog

	if (altButton)
		addButton(this, cancelButtonPos, _h - buttonHeight - 8, altButton, kCancelCmd, Common::ASCII_ESCAPE);	// Cancel dialog
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

ButtonWidget *MessageDialog::addButton(GuiObject *boss, int x, int y, const Common::String &label, uint32 cmd, char hotkey) {
	// FIXME: Get rid of this method: Use theme stuff instead.
	int w, h;

	if (g_gui.getWidgetSize() == kBigWidgetSize) {
		w = kBigButtonWidth;
		h = kBigButtonHeight;
	} else {
		w = kButtonWidth;
		h = kButtonHeight;
	}

	return new ButtonWidget(boss, x, y, w, h, label, cmd, hotkey);
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
