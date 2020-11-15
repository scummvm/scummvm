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

#include "common/translation.h"
#include "gui/chooser.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

ChooserDialog::ChooserDialog(const U32String &title, String dialogId)
	: Dialog(dialogId) {

	// Headline
	new StaticTextWidget(this, dialogId + ".Headline", title);

	// Add choice list
	_list = new ListWidget(this, dialogId + ".List");
	_list->setNumberingMode(kListNumberingOff);
	_list->setEditable(false);

	// Buttons
	new ButtonWidget(this, dialogId + ".Cancel", _("Cancel"), Common::U32String(), kCloseCmd);
	_chooseButton = new ButtonWidget(this, dialogId + ".Choose", _("Choose"), Common::U32String(), kChooseCmd);
	_chooseButton->setEnabled(false);
}

void ChooserDialog::setList(const U32StringArray &list) {
	_list->setList(list);
}

void ChooserDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	int item = _list->getSelected();
	switch (cmd) {
	case kChooseCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd:
		_list->endEditMode();
		setResult(item);
		close();
		break;
	case kListSelectionChangedCmd:
		_chooseButton->setEnabled(item >= 0);
		_chooseButton->markAsDirty();
		break;
	case kCloseCmd:
		setResult(-1);
		// Fall through
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
