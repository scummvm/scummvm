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
#include "xeen/menus.h"

namespace Xeen {

/**
 * Saves the current list of buttons
 */
void Dialog::saveButtons() {
	_savedButtons.push(_buttons);
}

/*
 * Clears the current list of defined buttons
 */
void Dialog::clearButtons() {
	_buttons.clear();
}

void Dialog::restoreButtons() {
	_buttons = _savedButtons.pop();
}

void Dialog::addButton(const Common::Rect &bounds, char c, SpriteResource *sprites, bool d) {
	_buttons.push_back(DialogButton(bounds, c, sprites, d));
}

void Dialog::checkEvents() {
	EventsManager &events = *_vm->_events;
	events.pollEventsAndWait();

	if (events._leftButton) {
		// Check whether any button is selected
		events.debounceMouse();
		Common::Point pt = events._mousePos;

		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(pt)) {
				_key = _buttons[i]._c;
				return;
			}
		}
	} else if (events.isKeyPending()) {
		Common::KeyState keyState;
		events.getKey(keyState);
		if (keyState.ascii >= 32 && keyState.ascii <= 127) {
			_key = keyState.ascii;
			return;
		}
	}
}

/*------------------------------------------------------------------------*/

void SettingsBaseDialog::showContents(SpriteResource &title1, bool waitFlag) {
	while (!_vm->shouldQuit() && _key == Common::KEYCODE_INVALID) {
		checkEvents();
	}
}

/*------------------------------------------------------------------------*/

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

	bool firstTime = true;
	while (!_vm->shouldQuit()) {
		setBackground();
		events.setCursor(0);

		if (firstTime) {
			firstTime = false;
			warning("TODO: Read existing save file");
		}

		for (;;) {
			showTitles1(title1Sprites);
			showTitles2();

		reopen:
			clearButtons();
			setupButtons(&title2Sprites);
			openWindow();

			while (!_vm->shouldQuit()) {
				showContents(title1Sprites, true);
				
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

		while (events.timeElapsed() == 0)
			events.pollEventsAndWait();
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
	addButton(Common::Rect(124, 87, 124 + 53, 87 + 10), 'S', buttons, true);
	addButton(Common::Rect(126, 98, 126 + 47, 98 + 10), 'L', buttons, true);
	addButton(Common::Rect(91, 110, 91 + 118, 110 + 10), 'C', buttons, true);
	addButton(Common::Rect(85, 121, 85 + 131, 121 + 10), 'O', buttons, true);
}

void WorldOptionsMenu::setupButtons(SpriteResource *buttons) {
	addButton(Common::Rect(93, 53, 93 + 134, 53 + 20), 'S', buttons, false);
	addButton(Common::Rect(93, 78, 93 + 134, 78 + 20), 'L', buttons, false);
	addButton(Common::Rect(93, 103, 93 + 134, 103 + 20), 'C', buttons, false);
	addButton(Common::Rect(93, 128, 93 + 134, 128 + 20), 'O', buttons, false);
}

/*------------------------------------------------------------------------*/

void CloudsOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "title1.int";
	title2 = "title1a.int";
}

/**
* Draws the scroll in the background
*/
void Dialog::doScroll(bool drawFlag, bool doFade) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	const int SCROLL_L[8] = { 29, 23, 15, 251, 245, 233, 207, 185 };
	const int SCROLL_R[8] = { 165, 171, 198, 218, 228, 245, 264, 281 };

	saveButtons();
	clearButtons();
	screen.saveBackground();

	// Load hand vga files
	SpriteResource *hand[16];
	for (int i = 0; i < 16; ++i) {
		Common::String name = Common::String::format("hand%02u.vga");
		hand[i] = new SpriteResource(name);
	}

	// Load marb vga files
	SpriteResource *marb[5];
	for (int i = 1; i < 5; ++i) {
		Common::String name = Common::String::format("marb%02u.vga");
		marb[i] = new SpriteResource(name);
	}

	if (drawFlag) {
		for (int i = 22; i > 0; --i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i > 0 && i <= 14) {
				hand[i - 1]->draw(screen, 0);
			} else {
				// TODO: Check '800h'.. horizontal reverse maybe?
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 14], 0));
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 14], 0));
			}

			if (i <= 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && _vm->_events->timeElapsed() == 0)
				_vm->_events->pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	} else {
		for (int i = 0; i < 22 && !events.isKeyMousePressed(); ++i) {
			events.updateGameCounter();
			screen.restoreBackground();

			if (i < 14) {
				hand[i]->draw(screen, 0);
			}
			else {
				// TODO: Check '800h'.. horizontal reverse maybe?
				hand[14]->draw(screen, 0, Common::Point(SCROLL_L[i - 7], 0));
				marb[15]->draw(screen, 0, Common::Point(SCROLL_R[i - 7], 0));
			}

			if (i < 20) {
				marb[i / 5]->draw(screen, i % 5);
			}

			while (!_vm->shouldQuit() && _vm->_events->timeElapsed() == 0)
				_vm->_events->pollEventsAndWait();

			screen._windows[0].update();
			if (i == 0 && doFade)
				screen.fadeIn(2);
		}
	}

	if (drawFlag) {
		hand[0]->draw(screen, 0);
		marb[0]->draw(screen, 0);
	} else {
		screen.restoreBackground();
	}

	screen._windows[0].update();
	restoreButtons();

	// Free resources
	for (int i = 1; i < 5; ++i)
		delete marb[i];
	for (int i = 0; i < 16; ++i)
		delete hand[i];
}

/*------------------------------------------------------------------------*/

void DarkSideOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "title2.int";
	title2 = "title2a.int";
}

/**
* Draws the scroll in the background
*/
void DarkSideOptionsMenu::doScroll(bool drawFlag, bool doFade) {
	if (doFade) {
		_vm->_screen->fadeIn(2);
	}
}

void WorldOptionsMenu::startup(Common::String &title1, Common::String &title2) {
	title1 = "world.int";
	title2 = "start.icn";

	Screen &screen = *_vm->_screen;
	screen.fadeOut(4);
	screen.loadPalette("dark.pal");
	screen.fadeIn(0x81);
	_vm->_events->clearEvents();
}

void WorldOptionsMenu::setBackground() {
	Screen &screen = *_vm->_screen;
	screen.loadBackground("world.raw");
	screen.saveBackground();
	screen.fadeIn(4);	
}

void WorldOptionsMenu::openWindow() {
	_vm->_screen->_windows[28].open();
}

void WorldOptionsMenu::showContents(SpriteResource &title1, bool waitFlag) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	
	events.updateGameCounter();
	_bgFrame = ++_bgFrame % 5;
	title1.draw(screen._windows[0], 0);
	screen._windows[28].frame();

	screen._windows[28].writeString("\r\x01\x03c\fdMight and Magic Options\n"
		"World of Xeen\x02\n"
		"117Copyright (c) 1993 NWC, Inc.\n"
		"All Rights Reserved\x01");

	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		DialogButton &btn = _buttons[btnIndex];
		if (btn._d) {
			btn._sprites->draw(screen._windows[0], btnIndex * 2,
				Common::Point(btn._bounds.left, btn._bounds.top));
		}
	}

	if (waitFlag) {
		screen._windows[0].update();
		SettingsBaseDialog::showContents(title1, true);
	}
}

} // End of namespace Xeen
