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

#ifndef XEEN_SAVES_H
#define XEEN_SAVES_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "xeen/party.h"

namespace Xeen {

enum {
	kAutoSaveSlot = 0
};

struct XeenSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class SavesManager {
private:
	Common::String _targetName;
private:
	/**
	 * Initializes a new savegame
	 */
	void reset();
public:
	bool _wonWorld;
	bool _wonDarkSide;
public:
	SavesManager(const Common::String &targetName);
	~SavesManager();

	/**
	 * Read in a savegame header
	 */
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, XeenSavegameHeader &header, bool skipThumbnail = true);

	/**
	 * Write out a savegame header
	 */
	void writeSavegameHeader(Common::OutSaveFile *out, XeenSavegameHeader &header);

	/**
	 * Load a savegame
	 */
	Common::Error loadGameState(int slot);

	/**
	 * Save the game
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);

	/**
	 * Does an autosave
	 */
	void doAutosave();

	/**
	 * Sets up a new game
	 */
	void newGame();

	/**
	 * Shows the load game dialog, and lets the user load a game
	 * @returns		True if a savegame was loaded
	 */
	bool loadGame();

	/**
	 * Shows the save game dialog, and lets the user save their game
	 * @returns		True if a savegame was saved
	 */
	bool saveGame();
};

} // End of namespace Xeen

#endif	/* XEEN_SAVES_H */
