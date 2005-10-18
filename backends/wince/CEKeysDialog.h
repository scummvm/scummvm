/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef CEKEYSDIALOG
#define CEKEYSDIALOG

#include "gui/newgui.h"
#include "gui/dialog.h"
#include "gui/ListWidget.h"
#include "common/str.h"

class CEKeysDialog : public GUI::Dialog {
public:
	CEKeysDialog(const Common::String &title = "Choose an action to map");

	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	virtual void handleKeyDown(uint16 ascii, int keycode, int modifiers);

protected:

	GUI::ListWidget		 *_actionsList;
	GUI::StaticTextWidget *_actionTitle;
	GUI::StaticTextWidget *_keyMapping;
	int				 _actionSelected;
};

#endif
