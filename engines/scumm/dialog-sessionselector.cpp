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

#include "engines/metaengine.h"
#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/taskbar.h"
#include "common/translation.h"

#include "engines/advancedDetector.h"

#include "scumm/dialog-sessionselector.h"

namespace Scumm {

enum {
	kOkCmd = 'OK  ',
	kCancelCmd = 'CNCL'
};

SessionSelectorDialog::SessionSelectorDialog(Scumm::ScummEngine_v90he *vm)
	: Dialog("SessionSelector"),
	_vm(vm),
	_joinButton(nullptr),
	_queryProgressText(nullptr) {

	_timestamp = 0;

	_queryProgressText = new GUI::StaticTextWidget(this, "SessionSelector.QueryProgressText",
						// I18N: Retrieving list of online multiplayer games
						_("Querying games..."));

	_queryProgressText->setAlign(Graphics::kTextAlignCenter);

	_list = new GUI::ListWidget(this, "SessionSelector.SessionList");
	_list->setEditable(false);
	_list->setNumberingMode(GUI::kListNumberingOff);

	// I18N: The user's name for online
	new GUI::StaticTextWidget(this, "SessionSelector.PlayerNameLabel", _("Your Name:"));
	_playerName = new GUI::EditTextWidget(this, "SessionSelector.PlayerName", ConfMan.get("network_player_name"));

	// I18N: Join online multiplayer game
	_joinButton = new GUI::ButtonWidget(this, "SessionSelector.Join", _("Join"), Common::U32String(), kOkCmd, Common::ASCII_RETURN);
	_joinButton->setEnabled(false);

	new GUI::ButtonWidget(this, "SessionSelector.Cancel", _("Cancel"), Common::U32String(), kCancelCmd, Common::ASCII_ESCAPE);
}

void SessionSelectorDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kListSelectionChangedCmd:
		_timestamp = g_system->getMillis();
		_joinButton->setEnabled(true);
		break;
	case GUI::kListItemDoubleClickedCmd:
	case kOkCmd:
		if (_list->getSelected() > -1) {
			// Save our name.
			ConfMan.set("network_player_name", _playerName->getEditString());
			ConfMan.flushToDisk();

			setResult(_list->getSelected());
			close();
		}
		break;
	case kCancelCmd:
		// User cancelled, so we don't do anything and just leave.
		setResult(-2);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void SessionSelectorDialog::handleTickle() {
	// Keep the connection active.
	_vm->_net->doNetworkOnceAFrame(15);

	// Query for new sessions every 5 seconds.
	if (!_timestamp || g_system->getMillis() - _timestamp > 5000) {
		int numSessions = _vm->_net->querySessions();

		// Clear list
		Common::U32StringArray l;
		_list->setList(l);

		for (int i = 0; i < numSessions; i++) {
			char name[MAX_SESSION_NAME];
			_vm->_net->getSessionName(i, name, MAX_SESSION_NAME);
			_list->append(name);
		}

		_joinButton->setEnabled(false);
		// Update the dialog
		// I18N: Online multiplayer games were found
		_queryProgressText->setLabel(Common::U32String::format(_("Found %d available games."), l.size()));

		_timestamp = g_system->getMillis();
	}

	drawDialog(GUI::kDrawLayerForeground);
}


} // End of namespace Scumm
