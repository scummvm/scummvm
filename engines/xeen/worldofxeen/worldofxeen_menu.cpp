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
#include "xeen/dialogs_difficulty.h"
#include "xeen/resources.h"

namespace Xeen {
namespace WorldOfXeen {

void WorldOfXeenMenu::show(XeenEngine *vm) {
	WorldOfXeenMenu *menu;

	switch (vm->getGameID()) {
	case GType_Clouds:
		menu = new CloudsOptionsMenu(vm);
		break;
	case GType_DarkSide:
		menu = new DarkSideOptionsMenu(vm);
		break;
	case GType_WorldOfXeen:
		menu = new WorldOptionsMenu(vm);
		break;
	default:
		error("Unsupported game");
		break;
	}

	menu->execute();
	delete menu;
}

void WorldOfXeenMenu::execute() {
	_vm->_files->setGameCc(1);
	SpriteResource special("special.icn");
	Windows &windows = *_vm->_windows;
	EventsManager &events = *_vm->_events;

	File newBright("newbrigh.m");
	_vm->_sound->playSong(newBright);

	windows[GAME_WINDOW].setBounds(Common::Rect(72, 25, 248, 175));

	Common::String title1, title2;
	startup(title1, title2);
	SpriteResource title1Sprites(title1), title2Sprites(title2);

	bool firstTime = true, doFade = true;
	while (!_vm->shouldExit()) {
		setBackground(doFade);

		if (firstTime) {
			firstTime = false;
			events.setCursor(0);
			events.showCursor();
		}

		showTitles1(title1Sprites);
		showTitles2();

		clearButtons();
		setupButtons(&title2Sprites);
		openWindow();

		while (!_vm->shouldExit()) {
			// Show the dialog with a continually animating background
			while (!_vm->shouldExit() && !_buttonValue)
				showContents(title1Sprites, true);
			if (_vm->shouldExit())
				return;

			// Handle keypress
			int key = toupper(_buttonValue);
			_buttonValue = 0;

			if (key == 27) {
				// Hide the options menu
				closeWindow();
				break;
			} else if (key == 'C' || key == 'V') {
				// Show credits
				closeWindow();
				CreditsScreen::show(_vm);
				break;
			} else if (key == 'S') {
				// Start new game
				int result = DifficultyDialog::show(_vm);
				if (result == -1)
					break;

				_vm->_party->_difficulty = (Difficulty)result;
				WOX_VM._pendingAction = WOX_PLAY_GAME;
				closeWindow();
				return;
			}
		}
	}
}

void WorldOfXeenMenu::showTitles1(SpriteResource &sprites) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	int frameNum = 0;
	while (!_vm->shouldExit() && !events.isKeyMousePressed()) {
		events.updateGameCounter();

		frameNum = (frameNum + 1) % (_vm->getGameID() == GType_WorldOfXeen ? 5 : 10);
		screen.restoreBackground();
		sprites.draw(0, frameNum);

		events.wait(4);
	}
}

void WorldOfXeenMenu::showTitles2() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;

	SpriteResource titleSprites("title2b.raw");
	SpriteResource kludgeSprites("kludge.int");
	SpriteResource title2Sprites[8] = {
		SpriteResource("title2b.int"), SpriteResource("title2c.int"),
		SpriteResource("title2d.int"), SpriteResource("title2e.int"),
		SpriteResource("title2f.int"), SpriteResource("title2g.int"),
		SpriteResource("title2h.int"), SpriteResource("title2i.int"),
	};

	kludgeSprites.draw(0, 0);
	screen.saveBackground();
	sound.playSound("elect.voc");

	for (int i = 0; i < 30 && !_vm->shouldExit(); ++i) {
		events.updateGameCounter();
		screen.restoreBackground();
		title2Sprites[i / 4].draw(0, i % 4);
		windows[0].update();

		if (i == 19)
			sound.stopSound();

		while (!_vm->shouldExit() && events.timeElapsed() < 2)
			events.pollEventsAndWait();
	}

	screen.restoreBackground();
	windows[0].update();
}

void WorldOfXeenMenu::setupButtons(SpriteResource *buttons) {
	addButton(Common::Rect(124, 87, 124 + 53, 87 + 10), 'S');
	addButton(Common::Rect(126, 98, 126 + 47, 98 + 10), 'L');
	addButton(Common::Rect(91, 110, 91 + 118, 110 + 10), 'C');
	addButton(Common::Rect(85, 121, 85 + 131, 121 + 10), 'O');
}

void WorldOptionsMenu::setupButtons(SpriteResource *buttons) {
	addButton(Common::Rect(93, 53, 93 + 134, 53 + 20), 'S', buttons);
	addButton(Common::Rect(93, 78, 93 + 134, 78 + 20), 'L', buttons);
	addButton(Common::Rect(93, 103, 93 + 134, 103 + 20), 'C', buttons);
	addButton(Common::Rect(93, 128, 93 + 134, 128 + 20), 'O', buttons);
}

/*------------------------------------------------------------------------*/

void CloudsOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "title1.int";
	title2 = "title1a.int";
}

/*------------------------------------------------------------------------*/

void DarkSideOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "title2.int";
	title2 = "title2a.int";
}

void WorldOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "world.int";
	title2 = "start.icn";

	Screen &screen = *_vm->_screen;
	screen.fadeOut();
	screen.loadPalette("dark.pal");
	_vm->_events->clearEvents();
}

void WorldOptionsMenu::setBackground(bool doFade) {
	Screen &screen = *_vm->_screen;
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (doFade)
		screen.fadeIn();
}

void WorldOptionsMenu::openWindow() {
	Windows &windows = *_vm->_windows;
	windows[GAME_WINDOW].open();
}

void WorldOptionsMenu::closeWindow() {
	Windows &windows = *_vm->_windows;
	windows[GAME_WINDOW].close();
}

void WorldOptionsMenu::showContents(SpriteResource &title1, bool waitFlag) {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Windows &windows = *_vm->_windows;
	events.updateGameCounter();

	// Draw the background frame in a continous cycle
	_bgFrame = (_bgFrame + 1) % 5;
	title1.draw(windows[0], _bgFrame);

	// Draw the basic frame for the optitons menu and title text
	windows[GAME_WINDOW].frame();
	windows[GAME_WINDOW].writeString(Res.OPTIONS_TITLE);

	drawButtons(&windows[0]);
	screen.update();

	if (waitFlag) {
		while (!_vm->shouldExit() && !_buttonValue && events.timeElapsed() < 3) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}
	}
}

} // End of namespace WorldOfXeen
} // End of namespace Xeen
