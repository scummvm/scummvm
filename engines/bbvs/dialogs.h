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

#ifndef BBVS_DIALOGS_H
#define BBVS_DIALOGS_H

#include "bbvs/bbvs.h"
#include "gui/dialog.h"

namespace GUI {
class ButtonWidget;
class CommandSender;
}

namespace Bbvs {

enum {
	// Main menu
	kCmdNewGame = 'NEWG',
	kCmdContinue = 'CONT',
	kCmdOptions = 'OPTN',
	kCmdMiniGames = 'MINI',
	kCmdQuit = 'QUIT',
	// Options
	kCmdUninstall = 0,
	kCmdCredits = 'CRED',
	kCmdOpening = 'OPEN',
	kCmdChicksNStuff = 'CHIC',
	// Minigames
	kCmdHockALoogie = 'HOCK',
	kCmdBugJustice = 'BUGJ',
	kCmdCourtChaos = 'CORT',
	kCmdAirGuitar = 'AIRG',
	kCmdBack = 'BACK'
};

enum {
	kMainMenuScr        = 0,
	kOptionsMenuScr     = 1,
	kMiniGamesMenuScr   = 2
};

class MainMenu : public GUI::Dialog {
public:
	MainMenu(BbvsEngine *vm);
	~MainMenu() override;

	void reflowLayout() override;
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override;

protected:
	BbvsEngine *_vm;

	void init();

	GUI::ButtonWidget *_buttons[5];

	void gotoMenuScreen(int index);
	bool canContinue();
	void gotoScene(int sceneNum);

};

}

#endif // BBVS_DIALOGS_H
