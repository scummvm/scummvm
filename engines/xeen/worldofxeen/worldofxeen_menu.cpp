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
		if (g_vm->getIsCD())
			menu = new WorldOfXeenCDMainMenuContainer();
		else
			menu = new WorldOfXeenMainMenuContainer();
		break;
	default:
		error("Invalid game");
	}

	menu->execute();
	delete menu;
}

MainMenuContainer::MainMenuContainer(const char *spritesName1, const char *spritesName2, const char *spritesName3) :
		_animateCtr(0), _dialog(nullptr) {
	g_vm->_files->setGameCc(g_vm->getGameID() == GType_Clouds ? 0 : 1);

	_backgroundSprites.resize(1 + (spritesName2 ? 1 : 0) + (spritesName3 ? 1 : 0));
	_backgroundSprites[0].load(spritesName1);
	if (spritesName2)
		_backgroundSprites[1].load(spritesName2);
	if (spritesName3)
		_backgroundSprites[2].load(spritesName3);

	_frameCount = 0;
	for (uint idx = 0; idx < _backgroundSprites.size(); ++idx)
		_frameCount += _backgroundSprites[idx].size();
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

	// Draw the next background frame
	uint frameNum = _animateCtr;
	for (uint idx = 0; idx < _backgroundSprites.size(); ++idx) {
		if (frameNum < _backgroundSprites[idx].size()) {
			_backgroundSprites[idx].draw(0, frameNum);
			return;
		} else {
			frameNum -= _backgroundSprites[idx].size();
		}
	}
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
			display();
			screen.doScroll(false, true);
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
				PendingEvent pe;
				if (events.getEvent(pe) && pe._keyState.keycode == Common::KEYCODE_ESCAPE)
					g_vm->_gameMode = GMODE_QUIT;

				events.clearEvents();
				showMenuDialog();
			}
		}
	}
}

/*------------------------------------------------------------------------*/

CloudsMainMenuContainer::CloudsMainMenuContainer() : MainMenuContainer("intro.vga") {
}

void CloudsMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 0;
	files.setGameCc(0);

	screen.loadPalette("mm4.pal");
	screen.loadBackground("intro.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("inn.m");
}

void CloudsMainMenuContainer::showMenuDialog() {
	setOwner(new CloudsMenuDialog(this));
}

/*------------------------------------------------------------------------*/

DarkSideMainMenuContainer::DarkSideMainMenuContainer() : MainMenuContainer("title2a.int") {
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	screen.loadPalette("dark.pal");
	screen.fadeIn(0x81);

	sound._musicSide = 1;
	sound.playSong("newbrigh.m");

	_background.load("title2.int");
}

void DarkSideMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	_background.draw(0, 0, Common::Point(0, 0));
	_background.draw(0, 1, Common::Point(160, 0));

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void DarkSideMainMenuContainer::showMenuDialog() {
	setOwner(new DarkSideMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenMainMenuContainer::WorldOfXeenMainMenuContainer() : MainMenuContainer("world.int") {
}

void WorldOfXeenMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void WorldOfXeenMainMenuContainer::showMenuDialog() {
	setOwner(new WorldMenuDialog(this));
}

/*------------------------------------------------------------------------*/

WorldOfXeenCDMainMenuContainer::WorldOfXeenCDMainMenuContainer() : MainMenuContainer("world0.int", "world1.int", "world2.int") {
}

void WorldOfXeenCDMainMenuContainer::display() {
	FileManager &files = *g_vm->_files;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;

	sound._musicSide = 1;
	files.setGameCc(1);

	screen.loadPalette("dark.pal");
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (!sound.isMusicPlaying())
		sound.playSong("newbrigh.m");
}

void WorldOfXeenCDMainMenuContainer::showMenuDialog() {
	setOwner(new WorldMenuDialog(this));
}

/*------------------------------------------------------------------------*/

bool MainMenuDialog::handleEvents() {
	FileManager &files = *g_vm->_files;
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

	case Common::KEYCODE_l: {
		// Load existing game
		int ccNum = files._ccNum;
		g_vm->_saves->newGame();
		if (!g_vm->_saves->loadGame()) {
			files.setGameCc(ccNum);
			return true;
		}

		g_vm->_gameMode = GMODE_PLAY_GAME;
		break;
	}

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
	w.writeString(Common::String::format(Res.OPTIONS_MENU, Res.GAME_NAMES[0], g_vm->_gameWon[0] ? 117 : 92, 1992));
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
			WOX_VM._sound->stopAllAudio();
			WOX_VM.showCloudsEnding(g_vm->_finalScore);
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
	case Common::KEYCODE_o: {
		// Show other options dialog
		// Remove this dialog
		MainMenuContainer *owner = _owner;
		delete this;

		// Set the new options dialog
		owner->setOwner(new OtherOptionsDialog(owner));
		return true;
	}

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
	w.writeString(Common::String::format(Res.OPTIONS_MENU, Res.GAME_NAMES[2], 117, 1993));
	drawButtons(&w);
}

bool WorldMenuDialog::handleEvents() {
	if (MainMenuDialog::handleEvents())
		return true;

	switch (_buttonValue) {
	case Common::KEYCODE_o: {
		// Show other options dialog
		// Remove this dialog
		MainMenuContainer *owner = _owner;
		delete this;

		// Set the new options dialog
		owner->setOwner(new OtherOptionsDialog(owner));
		return true;
	}

	default:
		break;
	}

	return false;
}

/*------------------------------------------------------------------------*/

OtherOptionsDialog::OtherOptionsDialog(MainMenuContainer *owner) : MenuContainerDialog(owner) {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	int height = (g_vm->getGameID() == GType_WorldOfXeen ? 25 : 0)
		+ (g_vm->getGameID() == GType_WorldOfXeen && g_vm->_gameWon[0] ? 25 : 0)
		+ (g_vm->_gameWon[1] ? 25 : 0)
		+ (g_vm->_gameWon[2] ? 25 : 0)
		+ 75;

	w.setBounds(Common::Rect(72, 25, 248, 25 + height));
	w.open();

	loadButtons();
}

OtherOptionsDialog::~OtherOptionsDialog() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];
	w.close();
}

void OtherOptionsDialog::loadButtons() {
	_buttonSprites.load("special.icn");
	Common::Rect r(93, 53, 227, 73);

	// View Darkside Intro
	addButton(r, Common::KEYCODE_d, &_buttonSprites);
	r.translate(0, 25);

	// View Clouds Intro
	if (g_vm->getGameID() == GType_WorldOfXeen) {
		addButton(r, Common::KEYCODE_c, &_buttonSprites);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID);
	}

	// View Darkside End
	if (g_vm->_gameWon[1]) {
		addButton(r, Common::KEYCODE_e, &_buttonSprites);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID);
	}

	// View Clouds End
	if (g_vm->_gameWon[0]) {
		addButton(r, Common::KEYCODE_v, &_buttonSprites);
		r.translate(0, 25);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID);
	}

	// View World End
	if (g_vm->_gameWon[2]) {
		addButton(r, Common::KEYCODE_w, &_buttonSprites);
	} else {
		addButton(Common::Rect(), Common::KEYCODE_INVALID);
	}
}

void OtherOptionsDialog::draw() {
	Windows &windows = *g_vm->_windows;
	Window &w = windows[GAME_WINDOW];

	w.frame();
	w.writeString(Common::String::format(Res.OPTIONS_MENU,
		Res.GAME_NAMES[g_vm->getGameID() == GType_WorldOfXeen ? 2 : 1],
		w.getBounds().height() - 33, 1993));
	drawButtons(&w);
}

bool OtherOptionsDialog::handleEvents() {
	Sound &sound = *g_vm->_sound;
	checkEvents(g_vm);

	switch (_buttonValue) {
	case Common::KEYCODE_d:
		delete this;
		sound.stopAllAudio();
		WOX_VM._events->hideCursor();
		WOX_VM.showDarkSideIntro(false);
		WOX_VM._events->showCursor();
		break;

	case Common::KEYCODE_c:
		if (g_vm->getGameID() == GType_WorldOfXeen) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showCloudsIntro();
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_e:
		if (g_vm->_gameWon[1]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showDarkSideEnding(g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_v:
		if (g_vm->_gameWon[0]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showCloudsEnding(g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_w:
		if (g_vm->_gameWon[2]) {
			delete this;
			sound.stopAllAudio();
			WOX_VM._events->hideCursor();
			WOX_VM.showWorldOfXeenEnding(NON_GOOBER, g_vm->_finalScore);
			WOX_VM._events->showCursor();
		}
		break;

	case Common::KEYCODE_ESCAPE:
		// Exit dialog
		delete this;
		break;

	default:
		return false;
	}

	return true;
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
