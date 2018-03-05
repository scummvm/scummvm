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

MainMenuContainer::MainMenuContainer(const Common::String &spritesName, uint frameCount) :
		_frameCount(frameCount), _animateCtr(0), _dialog(nullptr) {
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
	_animateCtr = (_animateCtr + 1) % _frameCount;
	_backgroundSprites.draw(0, _animateCtr);
}

void MainMenuContainer::execute() {
	EventsManager &events = *g_vm->_events;
	Screen &screen = *g_vm->_screen;
	bool showFlag = false;

	// Show the cursor
	events.clearEvents();
	events.setCursor(0);
	events.showCursor();

	screen.doScroll(true, false);

	while (!g_vm->shouldExit() && g_vm->_gameMode == GMODE_NONE) {
		// Draw the menu
		draw();
		if (_dialog)
			_dialog->draw();

		// Fade/scroll in screen if first frame showing screen
		if (!showFlag) {
			loadBackground();
			screen.doScroll(false, false);
			showFlag = true;
		}

		// Check for events
		events.updateGameCounter();
		
		if (events.wait(4, true)) {
			if (_dialog) {
				// There's a dialog active, so let it handle the event
				_dialog->handleEvents();

				// If dialog was removed as a result of the event, flag screen for re-showing,
				// such as returning to main menu from the Credits screen
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

CloudsMainMenuContainer::CloudsMainMenuContainer() : MainMenuContainer("intro.vga", 9) {
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

DarkSideMainMenuContainer::DarkSideMainMenuContainer() : MainMenuContainer("title2a.int", 10) {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	screen.loadPalette("dark.pal");
	screen.fadeIn(0x81);
	sound.playSong("newbrigh.m");

	_background.load("title2.int");
}

void DarkSideMainMenuContainer::loadBackground() {
	Screen &screen = *g_vm->_screen;
	_background.draw(0, 0, Common::Point(0, 0));
	_background.draw(0, 1, Common::Point(160, 0));

	screen.loadPalette("dark.pal");
	screen.saveBackground();
}

void DarkSideMainMenuContainer::showMenuDialog() {
	setOwner(new DarkSideMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenMainMenuContainer::WorldOfXeenMainMenuContainer() : MainMenuContainer("world.int", 5) {
	Sound &sound = *g_vm->_sound;
	sound.playSong("newbrigh.m");
}

void WorldOfXeenMainMenuContainer::loadBackground() {
	Screen &screen = *g_vm->_screen;
	screen.loadPalette("dark.pal");
	screen.loadBackground("world.raw");
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

	case Common::KEYCODE_ESCAPE:
		// Exit dialog (returning to just the animated background)
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

DarkSideMenuDialog::DarkSideMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner), _firstDraw(true) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, 150));
	w.open();

	loadButtons();
}

DarkSideMenuDialog::~DarkSideMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void DarkSideMenuDialog::loadButtons() {
	addButton(Common::Rect(124, 87, 177, 97), Common::KEYCODE_s);
	addButton(Common::Rect(126, 98, 173, 108), Common::KEYCODE_l);
	addButton(Common::Rect(91, 110, 209, 120), Common::KEYCODE_c);
	addButton(Common::Rect(85, 121, 216, 131), Common::KEYCODE_o);
}

void DarkSideMenuDialog::draw() {
	Screen &screen = *g_vm->_screen;
	EventsManager &events = *g_vm->_events;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	if (!_firstDraw)
		return;

	SpriteResource kludgeSprites("kludge.int");
	SpriteResource title2Sprites[8] = {
		SpriteResource("title2b.int"), SpriteResource("title2c.int"),
		SpriteResource("title2d.int"), SpriteResource("title2e.int"),
		SpriteResource("title2f.int"), SpriteResource("title2g.int"),
		SpriteResource("title2h.int"), SpriteResource("title2i.int"),
	};

	screen.loadBackground("title2b.raw");
	kludgeSprites.draw(0, 0, Common::Point(85, 86));
	screen.saveBackground();
	sound.playSound("elect.voc");

	for (int i = 0; i < 30 && !g_vm->shouldExit(); ++i) {
		events.updateGameCounter();
		screen.restoreBackground();
		title2Sprites[i / 4].draw(0, i % 4);
		windows[0].update();

		if (i == 19)
			sound.stopSound();

		if (events.wait(2))
			break;
	}

	events.clearEvents();
	sound.stopSound();

	screen.restoreBackground();
	windows[0].update();
	_firstDraw = false;
}

bool DarkSideMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_o:
		// Show other options dialog
		// TODO
		break;

	default:
		break;
	}

	return false;
}

/*------------------------------------------------------------------------*/

WorldMenuDialog::WorldMenuDialog(MainMenuContainer *owner) : MainMenuDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.setBounds(Common::Rect(72, 25, 248, 175));
	w.open();

	loadButtons();
}

WorldMenuDialog::~WorldMenuDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void WorldMenuDialog::loadButtons() {
	_buttonSprites.load("start.icn");
	addButton(Common::Rect(93, 53, 227, 73), Common::KEYCODE_s, &_buttonSprites);
	addButton(Common::Rect(93, 78, 227, 98), Common::KEYCODE_l, &_buttonSprites);
	addButton(Common::Rect(93, 103, 227, 123), Common::KEYCODE_c, &_buttonSprites);
	addButton(Common::Rect(93, 128, 227, 148), Common::KEYCODE_o, &_buttonSprites);
}

void WorldMenuDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	w.frame();
	w.writeString(Common::String::format(Res.CLOUDS_MAIN_MENU, g_vm->_gameWon[0] ? 117 : 92));
	drawButtons(&w);
}

bool WorldMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_o:
		// Show other options dialog
		// TODO
		break;

	default:
		break;
	}

	return false;
}


} // End of namespace WorldOfXeen
} // End of namespace Xeen
