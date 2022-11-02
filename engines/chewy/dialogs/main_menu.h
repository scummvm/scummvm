/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CHEWY_MAIN_MENU_H
#define CHEWY_MAIN_MENU_H

#include "chewy/events.h"

namespace Chewy {
namespace Dialogs {

enum MainMenuSelection {
	MM_START_GAME = 0,
	MM_VIEW_INTRO = 1,
	MM_LOAD_GAME = 2,
	MM_CINEMA = 3,
	MM_QUIT = 4,
	MM_CREDITS = 5
};

class MainMenu {
private:
	static int _selection;
	static int _personAni[3];

	/**
	 * Screen rendering function for the main menu
	 */
	static void screenFunc();

	/**
	 * Animates the contents of the main menu,
	 * and checks for any option selection
	 */
	static void animate();

	/**
	 * Starts a new game
	 */
	static void startGame();

	/**
	 * Starts a new game
	 */
	static bool loadGame();

	/**
	 * Saves the personAni array
	 */
	static void savePersonAni();

	/**
	 * Restores the personAni array
	 */
	static void restorePersonAni();

public:
	/**
	 * Displays the menu
	 */
	static void execute();

	/**
	 * Plays the game
	 */
	static void playGame();
};

} // namespace Dialogs
} // namespace Chewy

#endif
