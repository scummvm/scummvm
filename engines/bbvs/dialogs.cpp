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
	{"\xD0\x9D\xD0\xBE\xD0\xB2\xD0\xB0\xD1\x8F \xD0\xB8\xD0\xB3\xD1\x80\xD0\xB0", kCmdNewGame},
	{"\xD0\x9F\xD1\x80\xD0\xBE\xD0\xB4\xD0\xBE\xD0\xBB\xD0\xB6\xD0\xB8\xD1\x82\xD1\x8C", kCmdContinue},
	{"\xD0\x95\xD1\x89\xD0\xB5 ..", kCmdOptions},
	{"\xD0\x9C\xD0\xB8\xD0\xBD\xD0\xB8 \xD0\x98\xD0\xB3\xD1\x80\xD1\x8B", kCmdMiniGames},
	{"\xD0\x92\xD1\x8B\xD1\x85\xD0\xBE\xD0\xB4", kCmdQuit},
	// Options
	{"\xD0\x94\xD0\xB5\xD0\xB8\xD0\xBD\xD1\x81\xD1\x82\xD0\xB0\xD0\xBB\xD0\xBB\xD1\x8F\xD1\x86\xD0\xB8\xD1\x8F", kCmdUninstall},
	{"\xD0\x90\xD0\xB2\xD1\x82\xD0\xBE\xD1\x80\xD1\x8B", kCmdCredits},
	{"\xD0\x9F\xD1\x80\xD0\xBE\xD0\xBB\xD0\xBE\xD0\xB3", kCmdOpening},
	{"\xD0\xA0\xD0\xB5\xD0\xBA\xD0\xBB\xD0\xB0\xD0\xBC\xD0\xB0", kCmdChicksNStuff},
	{"\xD0\x9D\xD0\xB0\xD0\xB7\xD0\xB0\xD0\xB4 ..", kCmdBack},
	// Minigames
	{"\xD0\xA1\xD0\xBD\xD0\xB0\xD0\xB9\xD0\xBF\xD0\xB5\xD1\x80", kCmdHockALoogie},
	{"\xD0\x96\xD1\x83\xD0\xBA\xD0\xBE\xD0\xB6\xD0\xB0\xD1\x80\xD0\xBA\xD0\xB0", kCmdBugJustice},
	{"\xD0\x9F\xD1\x82\xD0\xB5\xD0\xBD\xD0\xB8\xD1\x81", kCmdCourtChaos},
	{"\xD0\x96\xD0\xB8\xD0\xB2\xD0\xBE\xD0\xB9 \xD0\x97\xD0\xB2\xD1\x83\xD0\xBA", kCmdAirGuitar},
	{"\xD0\x9D\xD0\xB0\xD0\xB7\xD0\xB0\xD0\xB4 ..", kCmdBack}
};

MainMenu::MainMenu(BbvsEngine *vm) : Dialog(0, 0, 1, 1), _vm(vm) {
	init();
}

MainMenu::~MainMenu() {
}

void MainMenu::init() {
	_buttons[0] = new GUI::ButtonWidget(this, 0, 0, 1, 1, Common::U32String());
	_buttons[1] = new GUI::ButtonWidget(this, 0, 0, 1, 1, Common::U32String());
	_buttons[2] = new GUI::ButtonWidget(this, 0, 0, 1, 1, Common::U32String());
	_buttons[3] = new GUI::ButtonWidget(this, 0, 0, 1, 1, Common::U32String());
	_buttons[4] = new GUI::ButtonWidget(this, 0, 0, 1, 1, Common::U32String());
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
		_buttons[i]->setLabel(Common::U32String(btn->label, Common::kUtf8));
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
