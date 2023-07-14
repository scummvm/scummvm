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
#include "common/translation.h"
#include "backends/dlc/dlcmanager.h"

namespace GUI {

enum {
	kDownloadSelectedCmd = 'DWNS',
};

DownloadGamesDialog::DownloadGamesDialog()
	: Dialog("DownloadGames") {

	new StaticTextWidget(this, "DownloadGames.Headline", _("Download Freeware Games"));

	// Add list with downloadable game titles
	_gamesList = new ListWidget(this, "DownloadGames.List");
	_gamesList->setNumberingMode(kListNumberingOff);
	_gamesList->setEditable(false);

	// Populate the ListWidget
	Common::U32StringArray games;
	for (int i = 0; i < DLCMan._dlcs.size(); ++i) {
		games.push_back(DLCMan._dlcs[i]->name);
	}

	// Gray out already downloaded packages
	for (Common::ConfigManager::DomainMap::iterator domain = ConfMan.beginGameDomains(); domain != ConfMan.endGameDomains(); ++domain) {
		if (domain->_value.contains("download")) {
			Common::String val(domain->_value.getVal("download"));
			uint64 id = val.asUint64();
			if (id < games.size()) {
				games[id] = "\001C{alternate}" + games[id];
			}
		}
	}

	_gamesList->setList(games);

	new ButtonWidget(this, "DownloadGames.Back", _("Back"), Common::U32String(), kCloseCmd);
	new ButtonWidget(this, "DownloadGames.Download", _("Download"), Common::U32String(), kDownloadSelectedCmd);
}

void DownloadGamesDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	//Search for typed-in directory
	case kDownloadSelectedCmd: {
		MessageDialog dialog("Downloading: " + _gamesList->getSelectedString());
		dialog.runModal();
		DLCMan.addDownload(_gamesList->getSelected());
		}
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace GUI
