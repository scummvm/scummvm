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

#ifndef CHOOSER_DIALOG_H
#define CHOOSER_DIALOG_H

#include "common/array.h"
#include "common/str.h"
#include "gui/dialog.h"

namespace GUI {

class ButtonWidget;
class CommandSender;
class ListWidget;

/*
 * A dialog that allows the user to choose between a selection of items
 */

class ChooserDialog : public Dialog {
protected:
	ListWidget		*_list;
	ButtonWidget	*_chooseButton;

public:
	ChooserDialog(const Common::U32String &title, Common::String dialogId = "Browser");

	void setList(const Common::U32StringArray &list);

	void handleCommand(CommandSender *sender, uint32 cmd, uint32 data) override;
};

} // End of namespace GUI

#endif
