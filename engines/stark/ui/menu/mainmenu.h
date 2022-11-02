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

#ifndef STARK_UI_MENU_MAIN_MENU_H
#define STARK_UI_MENU_MAIN_MENU_H

#include "engines/stark/ui/menu/locationscreen.h"

namespace Stark {

/**
 * The main menu of the game when it is opened
 */
class MainMenuScreen : public StaticLocationScreen {
public:
	MainMenuScreen(Gfx::Driver *gfx, Cursor *cursor);
	virtual ~MainMenuScreen();

	// StaticLocationScreen API
	void open() override;

private:
	enum HelpTextIndex {
		kNewGame = 7,
		kContinue = 8,
		kOption = 6,
		kBox = 9,
		kQuit = 10,
		kCredits = 12
	};

	template<HelpTextIndex N>
	void helpTextHandler(StaticLocationWidget &widget, const Common::Point &mousePos);

	void newGameHandler();
	void loadHandler();
	void creditsHandler();
	void settingsHandler();
	void boxHandler();
	void quitHandler();
};

/**
 * The version info text
 */
class VersionInfoText : public StaticLocationWidget {
public:
	VersionInfoText();
	virtual ~VersionInfoText() {}

private:
	static const char _copyrightSymbol = char(0xA9);
	static const int _posX = 16, _posY = 419;
};

} // End of namespace Stark

#endif // STARK_UI_MENU_MAIN_MENU_H
