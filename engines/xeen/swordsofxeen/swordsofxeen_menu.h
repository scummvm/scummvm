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

#ifndef XEEN_SWORDSOFXEEN_MENU_H
#define XEEN_SWORDSOFXEEN_MENU_H

#include "xeen/dialogs/dialogs.h"

namespace Xeen {
namespace SwordsOfXeen {

class MainMenu : public ButtonContainer {
private:
	SpriteResource _start;

	/**
	 * Constructor
	 */
	MainMenu(XeenEngine *vm);

	/**
	 * Shows the menu
	 */
	void execute();

	/**
	 * Loads buttons for the menu
	 */
	void loadButtons();
public:
	/**
	 * Shows the main menu
	 * @param vm		Engine reference
	 */
	static void show(XeenEngine *vm);
};

} // End of namespace SwordsOfXeen
} // End of namespace Xeen

#endif /* XEEN_SWORDSOFXEEN_MENU_H */
