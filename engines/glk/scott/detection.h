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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#ifndef GLK_SCOTT_DETECTION_H
#define GLK_SCOTT_DETECTION_H

#include "common/fs.h"
#include "common/hash-str.h"
#include "engines/game.h"
#include "glk/detection.h"

namespace Glk {
namespace Scott {

class ScottMetaEngine {
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

} // End of namespace Scott
} // End of namespace Glk

#endif
