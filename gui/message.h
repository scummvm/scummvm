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

#ifndef MESSAGE_DIALOG_H
#define MESSAGE_DIALOG_H

#include "gui/dialog.h"
#include "common/str.h"

namespace GUI {

enum {
	kMessageOK = 1,
	kMessageCancel = 0
};


/**
 * Simple message dialog ("alert box"): presents a text message in a dialog with up to two buttons.
 */
class MessageDialog : public Dialog {
public:
	MessageDialog(const Common::String &message, const char *defaultButton = "OK", const char *altButton = 0);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

protected:
	int addLine(Common::StringList &lines, const char *line, int size);
};

/**
 * Timed message dialog: displays a message to the user for brief time period.
 */
class TimedMessageDialog : public MessageDialog {
public:
	TimedMessageDialog(const Common::String &message, uint32 duration);

	void handleTickle();

protected:
	uint32 _timer;
};

} // End of namespace GUI

#endif
