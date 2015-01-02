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
#include "xeen/dialogs_options.h"
#include "xeen/resources.h"

namespace Xeen {

void OptionsMenu::show(XeenEngine *vm) {
	OptionsMenu *menu;

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

void OptionsMenu::execute() {
	SpriteResource special("special.icn");
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	
	File newBright("newbrigh.m");
	_vm->_sound->playMusic(newBright);

	screen._windows[28].setBounds(Common::Rect(72, 25, 248, 175));

	Common::String title1, title2;
	startup(title1, title2);
	SpriteResource title1Sprites(title1), title2Sprites(title2);

	bool firstTime = true, doFade = true;
	while (!_vm->shouldQuit()) {
		setBackground(doFade);
		events.setCursor(0);

		if (firstTime) {
			firstTime = false;
			warning("TODO: Read existing save file");
		}

		showTitles1(title1Sprites);
		showTitles2();

		clearButtons();
		setupButtons(&title2Sprites);
		openWindow();

		while (!_vm->shouldQuit()) {
			// Show the dialog with a continually animating background
			while (!_vm->shouldQuit() && _key == '\0')
				showContents(title1Sprites, true);
			if (_vm->shouldQuit())
				return;

			// Handle keypress
			char key = toupper(_key);
			_key = '\0';

			if (key == 'C' || key == 'V') {
				// Show credits
				CreditsScreen::show(_vm);
				break;
			} else if (key == 27) {
				// Hide the options menu
				break;
			}
		}
	}
}

void OptionsMenu::showTitles1(SpriteResource &sprites) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;

	int frameNum = 0;
	while (!_vm->shouldQuit() && !events.isKeyMousePressed()) {
		events.updateGameCounter();

		frameNum = ++frameNum % (_vm->getGameID() == GType_WorldOfXeen ? 5 : 10);
		screen.restoreBackground();
		sprites.draw(screen, frameNum);

		events.wait(4, true);
	}
}

void OptionsMenu::showTitles2() {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	SoundManager &sound = *_vm->_sound;

	File voc("elect.voc");
	SpriteResource titleSprites("title2b.raw");
	SpriteResource kludgeSprites("kludge.int");
	SpriteResource title2Sprites[8] = {
		SpriteResource("title2b.int"), SpriteResource("title2c.int"),
		SpriteResource("title2d.int"), SpriteResource("title2e.int"),
		SpriteResource("title2f.int"), SpriteResource("title2g.int"),
		SpriteResource("title2h.int"), SpriteResource("title2i.int"),
	};

	kludgeSprites.draw(screen, 0);
	screen.saveBackground();
	sound.playSample(&voc, 0);

	for (int i = 0; i < 30 && !_vm->shouldQuit(); ++i) {
		events.updateGameCounter();
		screen.restoreBackground();
		title2Sprites[i / 4].draw(screen, i % 4);
		screen._windows[0].update();

		if (i == 19)
			sound.playSample(nullptr, 0);

		while (!_vm->shouldQuit() && events.timeElapsed() < 2)
			events.pollEventsAndWait();
	}

	screen.restoreBackground();
	screen._windows[0].update();
}

void OptionsMenu::setupButtons(SpriteResource *buttons) {
	addButton(Common::Rect(124, 87, 124 + 53, 87 + 10), 'S', buttons, false);
	addButton(Common::Rect(126, 98, 126 + 47, 98 + 10), 'L', buttons, false);
	addButton(Common::Rect(91, 110, 91 + 118, 110 + 10), 'C', buttons, false);
	addButton(Common::Rect(85, 121, 85 + 131, 121 + 10), 'O', buttons, false);
}

void WorldOptionsMenu::setupButtons(SpriteResource *buttons) {
	addButton(Common::Rect(93, 53, 93 + 134, 53 + 20), 'S', buttons, true);
	addButton(Common::Rect(93, 78, 93 + 134, 78 + 20), 'L', buttons, true);
	addButton(Common::Rect(93, 103, 93 + 134, 103 + 20), 'C', buttons, true);
	addButton(Common::Rect(93, 128, 93 + 134, 128 + 20), 'O', buttons, true);
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
	screen.fadeOut(4);
	screen.loadPalette("dark.pal");
	_vm->_events->clearEvents();
}

void WorldOptionsMenu::setBackground(bool doFade) {
	Screen &screen = *_vm->_screen;
	screen.loadBackground("world.raw");
	screen.saveBackground();

	if (doFade)
		screen.fadeIn(4);	
}

void WorldOptionsMenu::openWindow() {
	_vm->_screen->_windows[28].open();
}

void WorldOptionsMenu::showContents(SpriteResource &title1, bool waitFlag) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	events.updateGameCounter();
	
	// Draw the background frame in a continous cycle
	_bgFrame = ++_bgFrame % 5;
	title1.draw(screen._windows[0], _bgFrame);

	// Draw the basic frame for the optitons menu and title text
	screen._windows[28].frame();
	screen._windows[28].writeString(OPTIONS_TITLE);

	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		DialogButton &btn = _buttons[btnIndex];
		if (btn._draw) {
			btn._sprites->draw(screen._windows[0], btnIndex * 2,
				Common::Point(btn._bounds.left, btn._bounds.top));
		}
	}

	if (waitFlag) {
		screen._windows[0].update();

		while (!_vm->shouldQuit() && _key == Common::KEYCODE_INVALID &&
				events.timeElapsed() < 3) {
			checkEvents();
		}
	}
}

} // End of namespace Xeen
