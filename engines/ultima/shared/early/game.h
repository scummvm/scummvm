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

namespace Ultima {
namespace Shared {

class GameView;
class GameState;
class FontResources;

class Game : public GameBase {
	DECLARE_MESSAGE_MAP;
protected:
	GameView *_gameView;
	FontResources *_fontResources;
protected:
	/**
	 * Sets up EGA palette
	 */
	void setEGAPalette();

	/**
	 * Sets up a CGA palette
	 */
	void setCGAPalette();

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
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	Game();
	
	/**
	 * Destructor
	 */
	virtual ~Game();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
