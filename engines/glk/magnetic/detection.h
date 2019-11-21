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

#ifndef GLK_MAGNETIC_DETECTION
#define GLK_MAGNETIC_DETECTION

#include "common/fs.h"
#include "common/hash-str.h"
#include "engines/game.h"
#include "glk/detection.h"

namespace Glk {
namespace Magnetic {

/**
 * The following game database is built from Generic/games.txt, and is used
 * to identify the game being run.  Magnetic Scrolls games don't generally
 * supply a status line, so this data can be used instead.
 */
struct gms_game_table_t {
	const uint32 undo_size;   ///< Header word at offset 0x22
	const uint32 undo_pc;     ///< Header word at offset 0x26
	const char *const name;   ///< Game title and platform
};
typedef const gms_game_table_t *gms_game_tableref_t;

/**
 * Meta engine for Magnetic interpreter
 */
class MagneticMetaEngine {
public:
	/**
	 * Get a list of supported games
	 */
	static void getSupportedGames(PlainGameList &games);

	/**
	 * Returns a game description for the given game Id, if it's supported
	 */
	static GameDescriptor findGame(const char *gameId);

	/**
	 * Detect supported games
	 */
	static bool detectGames(const Common::FSList &fslist, DetectedGames &gameList);

	/**
	 * Check for game Id clashes with other sub-engines
	 */
	static void detectClashes(Common::StringMap &map);
};

/**
 * Look up and return the game table entry given a game's undo size and
 * undo pc values.  Returns the entry, or NULL if not found.
 */
extern const gms_game_table_t *gms_gameid_lookup_game(uint32 undo_size, uint32 undo_pc);

} // End of namespace Magnetic
} // End of namespace Glk

#endif
