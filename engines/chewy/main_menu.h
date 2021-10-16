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

#ifndef CHEWY_MAIN_MENU_H
#define CHEWY_MAIN_MENU_H

#include "chewy/events_base.h"

namespace Chewy {

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

	/**
	 * Screen rendering function for the main menu
	 */
	static void screenFunc();

	/**
	 * Animates the contents of the main menu,
	 * and checks for any option selection
	 */
	static void animate();

	static int16 creditsFn(int16 key);
public:
	/**
	 * Displays the menu
	 */
	static void execute();
};

} // namespace Chewy

#endif
