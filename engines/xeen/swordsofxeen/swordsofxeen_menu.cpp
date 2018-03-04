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

#include "xeen/swordsofxeen/swordsofxeen_menu.h"
#include "xeen/dialogs/credits_screen.h"
#include "xeen/dialogs/dialogs_difficulty.h"
#include "xeen/xeen.h"

namespace Xeen {
namespace SwordsOfXeen {

void MainMenu::show(XeenEngine *vm) {
	MainMenu *dlg = new MainMenu(vm);
	dlg->execute();
	delete dlg;
}

MainMenu::MainMenu(XeenEngine *vm) : ButtonContainer(vm) {
	loadButtons();
	_start.load("start.int");
}

void MainMenu::execute() {
	EventsManager &events = *_vm->_events;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	int difficulty;

	events.setCursor(0);
	events.showCursor();
	sound.playSong("newbrigh.m");

	do {
		// Draw the screen
		screen.fadeOut();
		screen.loadPalette("scr.pal");
		_start.draw(0, 0, Common::Point(0, 0));
		_start.draw(0, 1, Common::Point(160, 0));
		screen.fadeIn(129);

		bool redrawFlag = false;
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);

			// Handle keypress
			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
				// Exit game
				_vm->_gameMode = GMODE_QUIT;
				break;
			case Common::KEYCODE_c:
			case Common::KEYCODE_v:
				// Show credits
				CreditsScreen::show(_vm);
				redrawFlag = true;
				break;
			case Common::KEYCODE_s:
				// Start new game
				difficulty = DifficultyDialog::show(_vm);
				if (difficulty != -1) {
					// Load a new game state and set the difficulty
					_vm->_saves->newGame();
					_vm->_party->_difficulty = (Difficulty)difficulty;
					_vm->_gameMode = GMODE_PLAY_GAME;
				}
				break;
			case Common::KEYCODE_l:
				_vm->_saves->newGame();
				if (_vm->_saves->loadGame())
					_vm->_gameMode = GMODE_PLAY_GAME;
				break;
			default:
				break;
			}
		} while (!_vm->shouldExit() && _vm->_gameMode == GMODE_NONE && !redrawFlag);
	} while (!_vm->shouldExit() && _vm->_gameMode == GMODE_NONE);

	screen.loadPalette("dark.pal");
}

void MainMenu::loadButtons() {
	addButton(Common::Rect(93, 87, 227, 97), Common::KEYCODE_s);
	addButton(Common::Rect(93, 98, 227, 108), Common::KEYCODE_l);
	addButton(Common::Rect(93, 110, 227, 120), Common::KEYCODE_v);
}

} // End of namespace SwordsOfXeen
} // End of namespace Xeen
