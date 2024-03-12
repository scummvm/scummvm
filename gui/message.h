/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"
#include "common/str-array.h"

namespace GUI {

class CommandSender;
class StaticTextWidget;
class ButtonWidget;

enum {
	kMessageOK = 0,
	kMessageAlt = 1
};


/**
 * Simple message dialog ("alert box"): presents a text message in a dialog with up to two buttons.
 */
class MessageDialog : public Dialog {
public:
	MessageDialog(const Common::U32String &message,
				  const Common::U32String &defaultButton = Common::U32String("OK"),
				  const Common::U32String &altButton = Common::U32String(),
				  Graphics::TextAlign alignment = Graphics::kTextAlignCenter,
				  const char *url = nullptr,
				  const Common::U32String &extraMessage = Common::U32String());
	MessageDialog(const Common::String &message,
				  const Common::String &defaultButton = "OK",
				  const Common::String &altButton = Common::String(),
				  Graphics::TextAlign alignment = Graphics::kTextAlignCenter,
				  const char *url = nullptr);
	MessageDialog(const Common::U32String &message,
				  const Common::U32String &defaultButton,
				  const Common::U32StringArray &altButtons,
				  Graphics::TextAlign alignment = Graphics::kTextAlignCenter);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
	void reflowLayout() override;

private:
	const char *_url;
	void init(const Common::U32String &message,
			  const Common::U32String &defaultButton,
			  const Common::U32StringArray &altButtons,
			  Graphics::TextAlign alignment,
			  const char *url,
			  const Common::U32String &extraMessage);

protected:
	Common::U32String _message;
	Graphics::TextAlign _alignment;
	Common::Array<StaticTextWidget *> _lines;
	Common::Array<ButtonWidget *> _buttons;
	StaticTextWidget *_extraMessage;
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
 * Timed message dialog: displays a message with a countdown.
 */
class CountdownMessageDialog : public MessageDialog {
public:
	CountdownMessageDialog(const Common::U32String &message,
				  uint32 duration,
				  const Common::U32String &defaultButton = Common::U32String("OK"),
				  const Common::U32String &altButton = Common::U32String(),
				  Graphics::TextAlign alignment = Graphics::kTextAlignCenter,
				  const Common::U32String &countdownMessage = Common::U32String(""));

	void handleTickle() override;

protected:
	void updateCountdown();

	uint32 _timer;
	uint32 _startTime;
	Common::U32String _countdownMessage;
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
