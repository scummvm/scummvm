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

#include "engines/advancedDetector.h"
#include "common/language.h"

namespace Glk {
namespace Scott {

/**
 * Game descriptor for Scott Adams games
 */
struct ScottGame {
	const char *_md5;
	const char *_gameId;
	int32 _filesize;
	const char *_desc;
};

extern const ScottGame SCOTT_GAMES[];
extern const char *const ADVENTURELAND_DESC;
extern const char *const PIRATE_ADVENTURE_DESC;
extern const char *const MISSION_IMPOSSIBLE_DESC;
extern const char *const VOODOO_CASTLE_DESC;
extern const char *const THE_COUNT_DESC;
extern const char *const STRANGE_ODYSSEY_DESC;
extern const char *const MYSTERY_FUN_HOUSE_DESC;
extern const char *const PYRAMID_OF_DOOM_DESC;
extern const char *const GHOST_TOWN_DESC;
extern const char *const SAVAGE_ISLAND1_DESC;
extern const char *const SAVAGE_ISLAND2_DESC;
extern const char *const THE_GOLDEN_VOYAGE_DESC;
extern const char *const ADVENTURE13_DESC;
extern const char *const ADVENTURE14_DESC;
extern const char *const BUCKAROO_BANZAI_DESC;
extern const char *const MARVEL_ADVENTURE_DESC;
extern const char *const MINI_SAMPLER_DESC;

} // End of namespace Frotz
} // End of namespace Glk
