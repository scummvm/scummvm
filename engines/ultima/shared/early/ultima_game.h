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

#ifndef ULTIMA_SHARED_EARLY_ULTIMA_GAME_H
#define ULTIMA_SHARED_EARLY_ULTIMA_GAME_H

#include "ultima/shared/early/game.h"

namespace Ultima {
namespace Shared {

class GameView;
class GameState;
class FontResources;

class UltimaGame : public Game {
	DECLARE_MESSAGE_MAP;
private:
	GameView *_gameView;
	GameState *_gameState;
	FontResources *_fontResources;
private:
	/**
	 * Sets up game palette
	 */
	void setPalette();
public:
	byte _foregroundColor;
	byte _backgroundColor;
	byte _highlightColor;
	byte _textColor;
	byte _color1;
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	UltimaGame();
	
	/**
	 * Destructor
	 */
	virtual ~UltimaGame();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
