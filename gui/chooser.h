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

#ifndef CHOOSER_DIALOG_H
#define CHOOSER_DIALOG_H

#include "common/str.h"
#include "common/list.h"
#include "gui/dialog.h"

namespace GUI {

class ButtonWidget;
class ListWidget;

/*
 * A dialog that allows the user to choose between a selection of items
 */

class ChooserDialog : public Dialog {
	typedef Common::String String;
	typedef Common::StringList StringList;
protected:
	ListWidget		*_list;
	ButtonWidget	*_chooseButton;

public:
	ChooserDialog(const String &title, const String &buttonLabel = "Choose", int height = 140);

	void setList(const StringList& list);

	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);
};

} // End of namespace GUI

#endif
