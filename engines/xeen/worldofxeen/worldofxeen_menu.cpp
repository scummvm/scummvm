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

#include "common/scummsys.h"
#include "xeen/worldofxeen/worldofxeen_menu.h"
#include "xeen/worldofxeen/worldofxeen.h"
#include "xeen/dialogs/credits_screen.h"
#include "xeen/dialogs/dialogs_difficulty.h"
#include "xeen/resources.h"

namespace Xeen {
namespace WorldOfXeen {

void MainMenuContainer::show() {
	MainMenuContainer *menu;

	switch (g_vm->getGameID()) {
	case GType_Clouds:
		menu = new CloudsMainMenuContainer();
		break;
	case GType_DarkSide:
		menu = new DarkSideMainMenuContainer();
		break;
	case GType_WorldOfXeen:
		menu = new WorldOfXeenMainMenuContainer();
		break;
	default:
		error("Invalid game");
	}

	menu->execute();
	delete menu;
}

MainMenuContainer::MainMenuContainer(const Common::String &spritesName) : _animateCtr(0), _dialog(nullptr) {
	_backgroundSprites.load(spritesName);
}

MainMenuContainer::~MainMenuContainer() {
	delete _dialog;
	g_vm->_windows->closeAll();
	g_vm->_sound->stopAllAudio();
	g_vm->_events->clearEvents();
}

void MainMenuContainer::draw() {
	g_vm->_screen->restoreBackground();
	_animateCtr = (_animateCtr + 1) % 9;
	_backgroundSprites.draw(0, _animateCtr);
}

void MainMenuContainer::execute() {
	EventsManager &events = *g_vm->_events;
	bool showFlag = false;

	// Show the cursor
	events.clearEvents();
	events.setCursor(0);
	events.showCursor();

	while (!g_vm->shouldExit() && g_vm->_gameMode == GMODE_NONE) {
		// Draw the menu
		draw();
		if (_dialog)
			_dialog->draw();

		// Fade/scroll in screen if first frame
		if (!showFlag) {
			loadBackground();
			// TODO: doScroll(false, false);
			showFlag = true;
		}

		// Check for events
		events.updateGameCounter();
		
		if (events.wait(4, true)) {
			if (_dialog) {
				// There's a dialog active, so let it handle the event
				_dialog->handleEvents();

				// If dialog was removed as a result of the event, flag screen for re-showing
				// if the menu screen isn't being left
				if (!_dialog)
					showFlag = false;
			} else {
				// No active dialog. If Escape pressed, exit game entirely. Otherwise,
				// open up the main menu dialog
				if (events.isKeyPending()) {
					Common::KeyState key;
					if (events.getKey(key) && key.keycode == Common::KEYCODE_ESCAPE)
						g_vm->_gameMode = GMODE_QUIT;
				}

				events.clearEvents();
				showMenuDialog();
			}
		}
	}
}

/*------------------------------------------------------------------------*/

CloudsMainMenuContainer::CloudsMainMenuContainer() : MainMenuContainer("intro.vga") {
	g_vm->_sound->playSong("inn.m");
}

void CloudsMainMenuContainer::loadBackground() {
	Screen &screen = *g_vm->_screen;
	screen.loadPalette("mm4.pal");
	screen.loadBackground("intro.raw");
	screen.saveBackground();
}

void CloudsMainMenuContainer::showMenuDialog() {
	setOwner(new CloudsMenuDialog(this));
}

/*------------------------------------------------------------------------*/

DarkSideMainMenuContainer::DarkSideMainMenuContainer() : MainMenuContainer("intro.vga") {
	g_vm->_sound->playSong("inn.m");
}

void DarkSideMainMenuContainer::loadBackground() {
	Screen &screen = *g_vm->_screen;
	screen.loadPalette("mm4.pal");
	screen.loadBackground("intro.raw");
	screen.saveBackground();
}

void DarkSideMainMenuContainer::showMenuDialog() {
	setOwner(new CloudsMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenMainMenuContainer::WorldOfXeenMainMenuContainer() : MainMenuContainer("intro.vga") {
	g_vm->_sound->playSong("inn.m");
}

void WorldOfXeenMainMenuContainer::loadBackground() {
	Screen &screen = *g_vm->_screen;
	screen.loadPalette("mm4.pal");
	screen.loadBackground("intro.raw");
	screen.saveBackground();
}

void WorldOfXeenMainMenuContainer::showMenuDialog() {
	setOwner(new CloudsMenuDialog(this));
}

/*------------------------------------------------------------------------*/

bool MainMenuDialog::handleEvents() {
	checkEvents(g_vm);
	int difficulty;

	switch (_buttonValue) {
	case Common::KEYCODE_s:
		// Start new game
		difficulty = DifficultyDialog::show(g_vm);
		if (difficulty == -1)
			return true;

		// Load a new game state and set the difficulty
		g_vm->_saves->newGame();
		g_vm->_party->_difficulty = (Difficulty)difficulty;
		g_vm->_gameMode = GMODE_PLAY_GAME;
		break;

	case Common::KEYCODE_l:
		// Load existing game
		g_vm->_saves->newGame();
		if (!g_vm->_saves->loadGame())
			return true;

		g_vm->_gameMode = GMODE_PLAY_GAME;
		break;

	case Common::KEYCODE_c:
	case Common::KEYCODE_v:
		// Show credits
		CreditsScreen::show(g_vm);
		break;

	default:
		return false;
	}

	// If this point is reached, delete the dialog itself, which will return the main menu
	// to it's default "No dialog showing" state
	delete this;
	return true;
}

/*------------------------------------------------------------------------*/

CloudsMenuDialog::CloudsMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, g_vm->_gameWon[0] ? 175 : 150));
	w.open();

	loadButtons();
}

CloudsMenuDialog::~CloudsMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void CloudsMenuDialog::loadButtons() {
	_buttonSprites.load("start.icn");
	addButton(Common::Rect(93, 53, 227, 73), Common::KEYCODE_s, &_buttonSprites);
	addButton(Common::Rect(93, 78, 227, 98), Common::KEYCODE_l, &_buttonSprites);
	addButton(Common::Rect(93, 103, 227, 123), Common::KEYCODE_c, &_buttonSprites);
	if (g_vm->_gameWon[0])
		addButton(Common::Rect(93, 128, 227, 148), Common::KEYCODE_e, &_buttonSprites);
}

void CloudsMenuDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	
	w.frame();
	w.writeString(Common::String::format(Res.CLOUDS_MAIN_MENU, g_vm->_gameWon[0] ? 117 : 92));
	drawButtons(&w);
}

bool CloudsMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_e:
		if (g_vm->_gameWon[0]) {
			// Close the window
			delete this;

			// Show clouds ending
			WOX_VM.showCloudsEnding(g_vm->_finalScore[0]);
			return true;
		}
		break;

	default:
		break;
	}

	return false;
}

/*------------------------------------------------------------------------*/


} // End of namespace WorldOfXeen
} // End of namespace Xeen
