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

#ifndef SCI_ENGINE_SAVEGAME_H
#define SCI_ENGINE_SAVEGAME_H

#include "common/scummsys.h"
#include "common/str.h"

#include "sci/sci.h"

namespace Sci {

struct EngineState;

/*
 * Savegame format history:
 *
 * Version - new/changed feature
 * =============================
 *      45 - Sync stopAfterFading
 *      44 - GK2+SCI3 audio resource locks
 *      43 - stop saving SCI3 mustSetViewVisible array
 *      42 - SCI3 robots and VM objects
 *      41 - palette support for newer SCI2.1 games; stable SCI2/2.1 save games
 *      40 - always store palvary variables
 *      39 - Accurate SCI32 arrays/strings, score metadata, avatar metadata
 *      38 - SCI32 cursor
 *      37 - Segment entry data changed to pointers
 *      36 - SCI32 bitmap segment
 *      35 - SCI32 remap
 *      34 - SCI32 palettes, and store play time in ticks
 *      33 - new overridePriority flag in MusicEntry
 *      32 - new playBed flag in MusicEntry
 *      31 - priority for sound effects/music is now a signed int16, instead of a byte
 *      30 - synonyms
 *      29 - system strings
 *      28 - heap
 *      27 - script created windows
 *      26 - play time
 *      25 - palette intensity
 *      24 - palvary
 *      23 - script buffer and heap size
 *      22 - game signature
 *      21 - script local variables
 *      20 - exports/synonyms
 *      19 - exportsAreWide
 *      18 - SCI32 arrays/strings
 *      17 - sound
 *
 */

enum {
	CURRENT_SAVEGAME_VERSION = 45,
	MINIMUM_SAVEGAME_VERSION = 14
#ifdef ENABLE_SCI32
	,
	MINIMUM_SCI32_SAVEGAME_VERSION = 41
#endif
};

// Savegame metadata
struct SavegameMetadata {
	Common::String name;
	int version;
	Common::String gameVersion;
	int saveDate;
	int saveTime;
	uint32 playTime;
	uint16 gameObjectOffset;
	uint16 script0Size;

	// Used by Shivers 1
	uint16 lowScore;
	uint16 highScore;

	// Used by MGDX
	uint8 avatarId;
};

/**
* Saves a game state to the hard disk in a portable way.
* @param s			The state to save
* @param saveId		The id of the savegame
* @param savename	The description of the savegame
* @param version	The version string of the game
* @return true on success, false otherwise
*/
bool gamestate_save(EngineState *s, int saveId, const Common::String &savename, const Common::String &version);

/**
 * Saves a game state to the hard disk in a portable way.
 * @param s			The state to save
 * @param save		The stream to save to
 * @param savename	The description of the savegame
 * @param version	The version string of the game
 * @return true on success, false otherwise
 */
bool gamestate_save(EngineState *s, Common::WriteStream *save, const Common::String &savename, const Common::String &version);

// does a few fixups right after restoring a saved game
void gamestate_afterRestoreFixUp(EngineState *s, int savegameId);

/**
* Restores a game state from a directory.
* @param s			An older state from the same game
* @param saveId		The id of the savegame to restore from
* @return true on success, false otherwise
*/
bool gamestate_restore(EngineState *s, int saveId);

/**
 * Restores a game state from a directory.
 * @param s			An older state from the same game
 * @param save		The stream to restore from
 */
void gamestate_restore(EngineState *s, Common::SeekableReadStream *save);

/**
 * Read the header from a savegame.
 */
bool get_savegame_metadata(Common::SeekableReadStream *stream, SavegameMetadata &meta);

/**
 * Write the header to a savegame.
 */
void set_savegame_metadata(Common::Serializer &ser, Common::WriteStream *fh, const Common::String &savename, const Common::String &version);
void set_savegame_metadata(Common::WriteStream *fh, const Common::String &savename, const Common::String &version);

} // End of namespace Sci

#endif // SCI_ENGINE_SAVEGAME_H
