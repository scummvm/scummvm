/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/system.h"
#include "gui/chooser.h"
#include "gui/newgui.h"
#include "gui/ListWidget.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

ChooserDialog::ChooserDialog(const String &title, String prefix, const String &buttonLabel)
	: Dialog(prefix + "chooser") {

	// Headline
	new StaticTextWidget(this, prefix + "chooser_headline", title);

	// Add choice list
	// HACK: Subtracting -12 from the height makes the list look good when
	// it's used to list savegames in the 320x200 version of the GUI.
	_list = new ListWidget(this, prefix + "chooser_list");
	_list->setNumberingMode(kListNumberingOff);

	// Buttons
	new ButtonWidget(this, prefix + "chooser_cancel", "Cancel", kCloseCmd, 0);
	_chooseButton = new ButtonWidget(this, prefix + "chooser_ok", buttonLabel, kChooseCmd, 0);
	_chooseButton->setEnabled(false);
}

void ChooserDialog::setList(const StringList& list) {
	_list->setList(list);
}

void ChooserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();
	switch (cmd) {
	case kChooseCmd:
	case kListItemDoubleClickedCmd:
		_list->endEditMode();
		setResult(item);
		close();
		break;
	case kListSelectionChangedCmd:
		_chooseButton->setEnabled(item >= 0);
		_chooseButton->draw();
		break;
	case kCloseCmd:
		setResult(-1);
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
