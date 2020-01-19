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

class Game;

enum VideoMode {
	UNSET = -1, CGA = 0, EGA = 1, TGA = 2, VGA_ENHANCED = 3
};

class GameState {
protected:
	Game *_game;
public:
	/**
	 * Position in the world map. This is stored separately from the map so that the same point can
	 * be returned to when leaving locations
	 */
	Point _worldMapPos;

	/**
	 * Characters in the party. In the earlier Ultima games, this is a single character
	 */
	CharacterArray _characters;

	/**
	 * Currently active character. In earlier Ultima games, this is the single party member
	 */
	Character *_currentCharacter;

	/**
	 * Pointer to the map manager for the game
	 */
	Map *_map;

	/**
	 * Game type Id
	 */
	uint _gameId;

	/**
	 * Video mode
	 */
	VideoMode _videoMode;

	/**
	 * The number of hit points to generate when a dungeon is left
	 */
	uint _dungeonExitHitPoints;

	/**
	 * Stores the base random seed used for generating deterministic dungeon levels
	 */
	uint32 _randomSeed;
public:
	/**
	 * Constructor
	 */
	GameState(Game *game);

	/**
	 * Destructor
	 */
	virtual ~GameState();

	/**
	 * Setup the initial game state
	 */
	virtual void setup() {}

	/**
	 * Returns true if the party is dead
	 */
	bool isPartyDead() const;

	/**
	 * Returns true if the party has no food
	 */
	bool isPartyFoodless() const;
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
