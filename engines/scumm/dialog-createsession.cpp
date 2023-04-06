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

#include "common/config-manager.h"
#include "common/translation.h"

#include "scumm/dialog-createsession.h"

namespace Scumm {

enum {
	kHostCmd = 'HOST',
	kCancelCmd = 'CNCL'
};

CreateSessionDialog::CreateSessionDialog() : Dialog("CreateSession") {

	// I18N: Creates new online session for multiplayer
	new GUI::StaticTextWidget(this, "CreateSession.CreateSessionTitle", _("Create a new game session"));

	// I18N: Name of the online game session
	new GUI::StaticTextWidget(this, "CreateSession.SessionNameLabel", _("Game Name:"));
	_sessionName = new GUI::EditTextWidget(this, "CreateSession.SessionName", ConfMan.get("game_session_name"));

	// I18N: The user's name for online
	new GUI::StaticTextWidget(this, "CreateSession.PlayerNameLabel", _("Your Name:"));
	_playerName = new GUI::EditTextWidget(this, "CreateSession.PlayerName", ConfMan.get("network_player_name"));

	new GUI::ButtonWidget(this, "CreateSession.Cancel", _("Cancel"), Common::U32String(), kCancelCmd, Common::ASCII_ESCAPE);
	// I18N: Button, start hosting online multiplayer game
	new GUI::ButtonWidget(this, "CreateSession.Host", _("Host"), Common::U32String(), kHostCmd, Common::ASCII_RETURN);
}

void CreateSessionDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kHostCmd:
		// Save our game configuration.
		ConfMan.set("game_session_name", _sessionName->getEditString());
		ConfMan.set("network_player_name", _playerName->getEditString());
		ConfMan.flushToDisk();

		setResult(1);
		close();
		break;
	case kCancelCmd:
		// User cancelled, so we don't do anything and just leave.
		setResult(0);
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

} // End of namespace Scumm
