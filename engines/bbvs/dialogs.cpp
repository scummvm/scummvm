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

#include "bbvs/dialogs.h"
#include "common/events.h"
#include "gui/widget.h"
#include "engines/advancedDetector.h"

namespace GUI {
class CommandSender;
}

namespace Bbvs {

struct MenuButton {
	const char *label;
	uint32 cmd;
};

static const MenuButton kMenuButtons[] = {
	// Main menu
	{"New Game", kCmdNewGame},
	{"Continue", kCmdContinue},
	{"Options", kCmdOptions},
	{"Mini Games", kCmdMiniGames},
	{"Quit", kCmdQuit},
	// Options
	{"Uninstall", kCmdUninstall},
	{"Credits", kCmdCredits},
	{"Opening", kCmdOpening},
	{"Chicks 'n' Stuff", kCmdChicksNStuff},
	{"Back ..", kCmdBack},
	// Minigames
	{"Hock-A-Loogie", kCmdHockALoogie},
	{"Bug Justice", kCmdBugJustice},
	{"Court Chaos", kCmdCourtChaos},
	{"Air Guitar", kCmdAirGuitar},
	{"Back ..", kCmdBack}
};

static const MenuButton kMenuButtonsRu[] = {
	// Main menu
	{"\xBD\xDE\xD2\xD0\xEF \xD8\xD3\xE0\xD0", kCmdNewGame},
	{"\xBF\xE0\xDE\xD4\xDE\xDB\xD6\xD8\xE2\xEC", kCmdContinue},
	{"\xB5\xE9\xD5 ..", kCmdOptions},
	{"\xBC\xD8\xDD\xD8 \xB8\xD3\xE0\xEB", kCmdMiniGames},
	{"\xB2\xEB\xE5\xDE\xD4", kCmdQuit},
	// Options
	{"\xB4\xD5\xD8\xDD\xE1\xE2\xD0\xDB\xDB\xEF\xE6\xD8\xEF", kCmdUninstall},
	{"\xB0\xD2\xE2\xDE\xE0\xEB", kCmdCredits},
	{"\xBF\xE0\xDE\xDB\xDE\xD3", kCmdOpening},
	{"\xC0\xD5\xDA\xDB\xD0\xDC\xD0", kCmdChicksNStuff},
	{"\xBD\xD0\xD7\xD0\xD4 ..", kCmdBack},
	// Minigames
	{"\xC1\xDD\xD0\xD9\xDF\xD5\xE0", kCmdHockALoogie},
	{"\xB6\xE3\xDA\xDE\xD6\xD0\xE0\xDA\xD0", kCmdBugJustice},
	{"\xBF\xE2\xD5\xDD\xD8\xE1", kCmdCourtChaos},
	{"\xB6\xD8\xD0\xDE\xD9 \xB7\xD2\xE3\xDA", kCmdAirGuitar},
	{"\xBD\xD0\xD7\xD0\xD4 ..", kCmdBack}
};

MainMenu::MainMenu(BbvsEngine *vm) : Dialog(0, 0, 1, 1), _vm(vm) {
	init();
}

MainMenu::~MainMenu() {
}

void MainMenu::init() {
	_buttons[0] = new GUI::ButtonWidget(this, 0, 0, 1, 1, "", 0, 0);
	_buttons[1] = new GUI::ButtonWidget(this, 0, 0, 1, 1, "", 0, 0);
	_buttons[2] = new GUI::ButtonWidget(this, 0, 0, 1, 1, "", 0, 0);
	_buttons[3] = new GUI::ButtonWidget(this, 0, 0, 1, 1, "", 0, 0);
	_buttons[4] = new GUI::ButtonWidget(this, 0, 0, 1, 1, "", 0, 0);
	gotoMenuScreen(kMainMenuScr);
}

void MainMenu::reflowLayout() {
	const int screenW = _vm->_system->getOverlayWidth();
	const int screenH = _vm->_system->getOverlayHeight();

	const int buttonWidth = screenW * 70 / 320;
	const int buttonHeight = screenH * 14 / 240;
	const int buttonPadding = screenW * 3 / 320;

	_w = 2 * buttonWidth  + buttonPadding;
	_h = 3 * buttonHeight + 3 * buttonPadding;
	_x = (screenW - _w) / 2;
	_y = screenH - _h - 2;

	int x = 0, y = 0;

	x = 0;
	y = 0;
	_buttons[0]->resize(x, y, buttonWidth, buttonHeight);
	x += buttonWidth + buttonPadding;
	_buttons[1]->resize(x, y, buttonWidth, buttonHeight);

	x = 0;
	y += buttonHeight + buttonPadding;
	_buttons[2]->resize(x, y, buttonWidth, buttonHeight);
	x += buttonWidth + buttonPadding;
	_buttons[3]->resize(x, y, buttonWidth, buttonHeight);

	x = (_w - buttonWidth) / 2; // Center the last button
	y += buttonHeight + buttonPadding;
	_buttons[4]->resize(x, y, buttonWidth, buttonHeight);

	GUI::Dialog::reflowLayout();

}

void MainMenu::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	switch (command) {
	// Main menu
	case kCmdNewGame:
		close();
		_vm->newGame();
		break;
	case kCmdContinue:
		close();
		_vm->continueGameFromQuickSave();
		break;
	case kCmdOptions:
		gotoMenuScreen(kOptionsMenuScr);
		break;
	case kCmdMiniGames:
		gotoMenuScreen(kMiniGamesMenuScr);
		break;
	case kCmdQuit:
		close();
		_vm->quitGame();
		break;
	// Options menu
	case kCmdUninstall:
		break;
	case kCmdCredits:
		gotoScene(45);
		break;
	case kCmdOpening:
		gotoScene(43);
		break;
	case kCmdChicksNStuff:
		gotoScene(41);
		break;
	// Minigames menu
	case kCmdHockALoogie:
		gotoScene(27);
		break;
	case kCmdBugJustice:
		gotoScene(29);
		break;
	case kCmdCourtChaos:
		gotoScene(28);
		break;
	case kCmdAirGuitar:
		gotoScene(30);
		break;
	case kCmdBack:
		gotoMenuScreen(kMainMenuScr);
		break;
	default:
		Dialog::handleCommand(sender, command, data);
	}
}

void MainMenu::gotoMenuScreen(int screen) {
	for (int i = 0; i < 5; ++i) {
		const MenuButton *btn;

		if (_vm->_gameDescription->language == Common::RU_RUS) {
			btn = &kMenuButtonsRu[screen * 5 + i];
		} else {
			btn = &kMenuButtons[screen * 5 + i];
		}
		_buttons[i]->setLabel(btn->label);
		_buttons[i]->setCmd(btn->cmd);
		_buttons[i]->setEnabled(btn->cmd != 0);
	}
	// Enable the "Continue" button if a savegame at slot 0 exists
	if (screen == kMainMenuScr)
		_buttons[1]->setEnabled(canContinue());
}

bool MainMenu::canContinue() {
	return _vm->existsSavegame(0);
}

void MainMenu::gotoScene(int sceneNum) {
	close();
	_vm->setNewSceneNum(sceneNum);
}

} // End of namespace Hugo
