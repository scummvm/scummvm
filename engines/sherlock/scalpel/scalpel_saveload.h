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

#ifndef SHERLOCK_SCALPEL_SAVELOAD_H
#define SHERLOCK_SCALPEL_SAVELOAD_H

#include "sherlock/saveload.h"

namespace Sherlock {

namespace Scalpel {

extern const int ENV_POINTS[6][3];

class ScalpelSaveManager: public SaveManager {
public:
	SaveMode _envMode;

	Common::String _fixedTextExit;
	Common::String _fixedTextLoad;
	Common::String _fixedTextSave;
	Common::String _fixedTextUp;
	Common::String _fixedTextDown;
	Common::String _fixedTextQuit;

	byte _hotkeyExit;
	byte _hotkeyLoad;
	byte _hotkeySave;
	byte _hotkeyUp;
	byte _hotkeyDown;
	byte _hotkeyQuit;

	byte _hotkeysIndexed[6];

	Common::String _fixedTextQuitGameQuestion;
	Common::String _fixedTextQuitGameYes;
	Common::String _fixedTextQuitGameNo;

	byte _hotkeyQuitGameYes;
	byte _hotkeyQuitGameNo;

public:
	ScalpelSaveManager(SherlockEngine *vm, const Common::String &target);
	~ScalpelSaveManager() override {}

	/**
	 * Shows the in-game dialog interface for loading and saving games
	 */
	void drawInterface();

	/**
	 * Return the index of the button the mouse is over, if any
	 */
	int getHighlightedButton() const;

	/**
	 * Handle highlighting buttons
	 */
	void highlightButtons(int btnIndex);

	/**
	 * Make sure that the selected savegame is on-screen
	 */
	bool checkGameOnScreen(int slot);

	/**
	 * Prompts the user to enter a description in a given slot
	 */
	bool promptForDescription(int slot);

	/**
	 * Identifies a button number according to the key, that the user pressed
	 */
	int identifyUserButton(int key);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
