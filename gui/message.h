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
 */

#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"

namespace GUI {

class CommandSender;

enum {
	kMessageOK = 1,
	kMessageCancel = 0
};


/**
 * Simple message dialog ("alert box"): presents a text message in a dialog with up to two buttons.
 */
class MessageDialog : public Dialog {
public:
	MessageDialog(const Common::U32String &message, const Common::U32String &defaultButton = Common::U32String("OK"), const Common::U32String &altButton = Common::U32String(), Graphics::TextAlign alignment = Graphics::kTextAlignCenter, const char *url = nullptr);
	MessageDialog(const Common::String &message, const Common::String &defaultButton = "OK", const Common::String &altButton = "", Graphics::TextAlign alignment = Graphics::kTextAlignCenter, const char *url = nullptr);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
private:
	const char *_url;
	void init(const Common::U32String &message, const Common::U32String &defaultButton, const Common::U32String &altButton, Graphics::TextAlign alignment, const char *url);
};

/**
 * Timed message dialog: displays a message to the user for brief time period.
 */
class TimedMessageDialog : public MessageDialog {
public:
	TimedMessageDialog(const Common::U32String &message, uint32 duration);

	void handleTickle() override;

protected:
	uint32 _timer;
};

/**
 * Message dialog with button to open a specified URL
 */
class MessageDialogWithURL : public MessageDialog {
public:
	MessageDialogWithURL(const Common::U32String &message, const char *url, const Common::U32String &defaultButton = Common::U32String("OK"), Graphics::TextAlign alignment = Graphics::kTextAlignCenter);
	MessageDialogWithURL(const Common::String &message, const char *url, const char *defaultButton = "OK", Graphics::TextAlign alignment = Graphics::kTextAlignCenter);
};



} // End of namespace GUI

#endif
