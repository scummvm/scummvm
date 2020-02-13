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

#ifndef SHERLOCK_TATTOO_H
#define SHERLOCK_TATTOO_H

#include "sherlock/sherlock.h"
#include "sherlock/tattoo/tattoo_darts.h"
#include "sherlock/tattoo/widget_foolscap.h"

namespace Sherlock {

namespace Tattoo {

enum {
	INV_FOREGROUND		= 167,
	INV_BACKGROUND		= 1,
	INFO_FOREGROUND		= 233,
	INFO_BACKGROUND		= 239,
	INFO_TOP			= 185,
	INFO_MIDDLE			= 186,
	INFO_BOTTOM			= 188,
	MENU_BACKGROUND		= 225,
	COMMAND_FOREGROUND	= 15,
	COMMAND_HIGHLIGHTED	= 254,
	COMMAND_NULL		= 193,
	PEN_COLOR			= 248,
	PEN_HIGHLIGHT_COLOR	= 129
};

enum {
	FLAG_PLAYER_IS_HOLMES	= 76,
	FLAG_ALT_MAP_MUSIC		= 525
};

class TattooEngine : public SherlockEngine {
private:
	Darts _darts;
	WidgetFoolscap _foolscapWidget;

	/**
	 * Loads the initial palette for the game
	 */
	void loadInitialPalette();

	/**
	 * Load the initial inventory
	 */
	void loadInventory();
protected:
	/**
	 * Initialize the engine
	 */
	void initialize() override;

	void showOpening() override;

	/**
	 * Starting a scene within the game
	 */
	void startScene() override;

	/**
	 * Load configuration options
	 */
	void loadConfig() override;
public:
	bool _runningProlog;
	bool _fastMode, _allowFastMode;
	bool _transparentMenus;
	bool _textWindowsOn;
public:
	TattooEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	~TattooEngine() override;

	/**
	 * Shows the foolscap puzzle
	 */
	void doFoolscapPuzzle();

	/**
	 * Save the game configuration
	 */
	void saveConfig() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently() override;

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
