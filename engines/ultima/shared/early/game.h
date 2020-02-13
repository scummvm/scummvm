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

#ifndef ULTIMA_SHARED_EARLY_GAME_H
#define ULTIMA_SHARED_EARLY_GAME_H

#include "ultima/shared/early/game_base.h"
#include "ultima/shared/core/party.h"

namespace Ultima {
namespace Shared {

class GameView;
class GameState;
class FontResources;

namespace Maps {
class Map;
}

/**
 * More specialized base class for earlier Ultima games
 */
class Game : public GameBase {
	DECLARE_MESSAGE_MAP;
	bool EndOfTurnMsg(CEndOfTurnMsg &msg);
protected:
	GameView *_gameView;
	FontResources *_fontResources;
protected:
	/**
	 * Sets up EGA palette
	 */
	void setEGAPalette();

	/**
	 * Loads the Ultima 6 palette
	 */
	void loadU6Palette();
public:
	byte _edgeColor;
	byte _borderColor;
	byte _highlightColor;
	byte _textColor;
	byte _color1;
	byte _bgColor;
	byte _whiteColor;

	/**
	 * Player party. In the earlier Ultima games, this is a single character
	 */
	Party *_party;

	/**
	 * Pointer to the map manager for the game
	 */
	Maps::Map *_map;

	/**
	 * Stores the base random seed used for generating deterministic dungeon levels
	 */
	uint32 _randomSeed;

public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Game();
	
	/**
	 * Destructor
	 */
	~Game() override;

	/**
	 * Play a sound effect
	 */
	void playFX(uint effectId);

	/**
	 * Sets an EGA palette based on a 16-byte EGA RGB indexes
	 */
	void setEGAPalette(const byte *palette);

	/**
	 * Sets up a CGA palette
	 */
	void setCGAPalette();

	/**
	 * Returns the map
	 */
	Maps::Map *getMap() const override { return _map; }

	/**
	 * Handles loading and saving games
	 */
	void synchronize(Common::Serializer &s) override;

	/**
	 * Signal an end of turn
	 */
	void endOfTurn();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
