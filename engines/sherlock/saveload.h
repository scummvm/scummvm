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

#ifndef SHERLOCK_SAVELOAD_H
#define SHERLOCK_SAVELOAD_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "engines/savestate.h"
#include "graphics/surface.h"

namespace Sherlock {

#define MAX_SAVEGAME_SLOTS 99
#define ONSCREEN_FILES_COUNT 5

enum {
	CURRENT_SAVEGAME_VERSION = 4,
	MINIMUM_SAVEGAME_VERSION = 4
};

enum SaveMode { SAVEMODE_NONE = 0, SAVEMODE_LOAD = 1, SAVEMODE_SAVE = 2 };

extern const char *const EMPTY_SAVEGAME_SLOT;

struct SherlockSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class SherlockEngine;

typedef Common::Serializer Serializer;

class SaveManager {
protected:
	SherlockEngine *_vm;
	Common::String _target;
	Graphics::Surface *_saveThumb;

	/**
	 * Build up a savegame list, with empty slots given an explicit Empty message
	 */
	void createSavegameList();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);
public:
	Common::StringArray _savegames;
	int _savegameIndex;
	bool _justLoaded;
public:
	static SaveManager *init(SherlockEngine *vm, const Common::String &target);
	SaveManager(SherlockEngine *vm, const Common::String &target);
	virtual ~SaveManager();

	/**
	 * Creates a thumbnail for the current on-screen contents
	 */
	void createThumbnail();

	/**
	 * Load a list of savegames
	 */
	static SaveStateList getSavegameList(const Common::String &target);

	/**
	 * Support method that generates a savegame name
	 * @param slot		Slot number
	 */
	Common::String generateSaveName(int slot);

	/**
	 * Write out the header information for a savegame
	 */
	void writeSavegameHeader(Common::OutSaveFile *out, SherlockSavegameHeader &header);

	/**
	 * Read in the header information for a savegame
	 */
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, SherlockSavegameHeader &header, bool skipThumbnail = true);

	/**
	 * Return the index of the button the mouse is over, if any
	 */
	int getHighlightedButton() const;

	/**
	 * Handle highlighting buttons
	 */
	void highlightButtons(int btnIndex);

	/**
	 * Load the game in the specified slot
	 */
	void loadGame(int slot);

	/**
	 * Save the game in the specified slot with the given name
	 */
	void saveGame(int slot, const Common::String &name);

	/**
	 * Returns true if the given save slot is empty
	 */
	bool isSlotEmpty(int slot) const;
};

} // End of namespace Sherlock

#endif
