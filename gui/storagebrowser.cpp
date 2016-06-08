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

#include "gui/storagebrowser.h"
#include "gui/widgets/list.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"

#include "common/translation.h"
#ifdef USE_CLOUD
#include "backends/cloud/cloudmanager.h"
#endif

namespace GUI {

enum {
	kChooseCmd = 'Chos'
};

StorageBrowser::StorageBrowser() : Dialog("Browser") {
	new StaticTextWidget(this, "Browser.Headline", _("Select a Storage"));

	// Add storages list
	_storagesList = new ListWidget(this, "Browser.List");
	_storagesList->setNumberingMode(kListNumberingOff);
	_storagesList->setEditable(false);

	_backgroundType = GUI::ThemeEngine::kDialogBackgroundPlain;

	// Buttons
	new ButtonWidget(this, "Browser.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "Browser.Choose", _("Choose"), 0, kChooseCmd);
}

void StorageBrowser::open() {
	// Always refresh storages list
	updateListing();

	// Call super implementation
	Dialog::open();
}

void StorageBrowser::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseCmd:
	case kListItemActivatedCmd:
	case kListItemDoubleClickedCmd: {		
		int selection = _storagesList->getSelected();
		if (selection < 0)
			break;
		_selectionIndex = selection;
		setResult(1);
		close();
		break;
	}
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void StorageBrowser::updateListing() {
	Common::StringArray list; 
	uint32 currentStorageIndex = 0;
#ifdef USE_CLOUD
	list = CloudMan.listStorages();
	currentStorageIndex = CloudMan.getStorageIndex();
#endif

	_storagesList->setList(list);
	_storagesList->scrollTo(0);
	_storagesList->setSelected(currentStorageIndex);

	// Finally, redraw
	draw();
}

} // End of namespace GUI
