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

#ifndef ULTIMA_SHARED_CORE_GAME_STATE_H
#define ULTIMA_SHARED_CORE_GAME_STATE_H

#include "ultima/shared/core/rect.h"
#include "ultima/shared/core/character.h"
#include "ultima/shared/core/map.h"

namespace Ultima {
namespace Shared {

enum VideoMode {
	UNSET = -1, CGA = 0, EGA = 1, TGA = 2, VGA_ENHANCED = 3
};

class GameState {
private:
	/**
	 * Enhanced game versions can have multiple tiles per tile of the original game. This constant
	 * specifies the maximum number of tiles a map can have versus the original
	 */
	const Point MAX_TILES_PER_ORIGINAL;

	/**
	 * Position in the world map. This is in the context of the whatever overworld map is used,
	 * so would have to be divided by MAX_TILES_PER_ORIGINAL to get original game co-ordinates
	 */
	Point _worldMapPos;

	/**
	 * Pointer to the map manager for the game
	 */
	Map *_map;
public:
	/**
	 * Characters in the party. In the earlier Ultima games, this is a single character
	 */
	CharacterArray _characters;

	/**
	 * Game type Id
	 */
	uint _gameId;

	/**
	 * Video mode
	 */
	VideoMode _videoMode;
public:
	/**
	 * Constructor
	 */
	GameState();

	/**
	 * Destructor
	 */
	~GameState();
};

} // End of namespace Shared
} // End of namespace Xeen

#endif
