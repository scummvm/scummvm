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

#include "gui/download-games-dialog.h"
#include "gui/message.h"
#include "gui/widget.h"
#include "gui/widgets/list.h"
#include "gui/gui-manager.h"
#include "common/translation.h"
#include "backends/dlc/dlcmanager.h"

namespace GUI {

DownloadGamesDialog::DownloadGamesDialog(LauncherDialog *launcher)
	: Dialog("DownloadGames"), _launcher(launcher) {
	
	// Set target (Command Receiver) for Command Sender
	DLCMan.setTarget(this);

	new StaticTextWidget(this, "DownloadGames.Headline", _("Download Freeware Games"));

	// Add list with downloadable game titles
	_gamesList = new ListWidget(this, "DownloadGames.List");
	_gamesList->setNumberingMode(kListNumberingOff);
	_gamesList->setEditable(false);

	if (!DLCMan._fetchDLCs) {
		DLCMan.getAllDLCs();
		DLCMan._fetchDLCs = true;
		Common::U32StringArray fetchingText = {
			_("Fetching DLCs..."),
		};
		_gamesList->setList(fetchingText);
	} else {
		refreshDLCList();
	}

	new ButtonWidget(this, "DownloadGames.Back", _("Back"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "DownloadGames.Download", _("Download"), Common::U32String(), kDownloadSelectedCmd);
}

DownloadGamesDialog::~DownloadGamesDialog() {
	DLCMan.setTarget(nullptr);
}

void DownloadGamesDialog::refreshDLCList() {
	// Populate the ListWidget
	Common::U32StringArray games;
	for (int i = 0; i < DLCMan._dlcs.size(); ++i) {
		games.push_back(DLCMan._dlcs[i]->name);
	}

	// Gray out already downloaded packages
	for (Common::ConfigManager::DomainMap::iterator domain = ConfMan.beginGameDomains(); domain != ConfMan.endGameDomains(); ++domain) {
		if (domain->_value.contains("download")) {
			Common::String id = domain->_value.getVal("download");
			int idx = DLCMan.getDLCIdxFromId(id);
			if (idx != -1 && idx < games.size()) {
				games[idx] = "\001C{alternate}" + games[idx];
			}
		}
	}

	_gamesList->setList(games);
	g_gui.scheduleTopDialogRedraw();
}

void DownloadGamesDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kDownloadSelectedCmd: {
		MessageDialog dialog("Downloading: " + _gamesList->getSelectedString());
		dialog.runModal();
		DLCMan.addDownload(_gamesList->getSelected());
		}
		break;
	case kRefreshDLCList: {
		refreshDLCList();
		}
		break;
	case kRefreshLauncher: {
		_launcher->rebuild();
		}
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
