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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_HERO_H
#define PRINCE_HERO_H

#include "common/scummsys.h"
#include "common/array.h"

#include "graphics/surface.h"

namespace Prince {

class Animation;

class Hero {
public:
	enum State {
		STAY = 0,
		TURN = 1,
		MOVE = 2,
		BORE = 3, 
		SPEC = 4,
		TALK = 5,
		MVAN = 6,
		TRAN = 7,
		RUN  = 8,
		DMOVE = 9
	};

	enum Direction {
		LEFT = 1,
		RIGHT = 2,
		UP = 3, 
		DOWN = 4
	};

	enum MoveSet {
		Move_SL,
		Move_SR,
		Move_SU,
		Move_SD,
		Move_ML,
		Move_MR,
		Move_MU,
		Move_MD,
		Move_TL,
		Move_TR,
		Move_TU,
		Move_TD,
		Move_MLU,
		Move_MLD,
		Move_MLR,
		Move_MRU,
		Move_MRD,
		Move_MRL,
		Move_MUL,
		Move_MUR,
		Move_MUD,
		Move_MDL,
		Move_MDR,
		Move_MDU,
		Move_BORED1,
		Move_BORED2
	};

	Hero();

	bool loadAnimSet(uint32 heroAnimNumber);

	const Graphics::Surface * getSurface();

	void setPos(int16 x, int16 y) { _middleX = x; _middleX = y; }
	void setVisible(bool flag) { _visible = flag; }

//private:
	uint16 _number;
	uint16 _visible;
	State _state;
	int16 _middleX;
	int16 _middleY;
	int16 _moveSetType;
	int16 _frame;

	// Coords array of coordinates
	// DirTab array of directions
	// CurrCoords current coordinations
	// CurrDirTab current direction
	// LastDir previous move direction
	// DestDir 
	// LeftRight previous left/right direction
	// UpDown previous up/down direction
	// Phase animation phase
	// Step x/y step size depends on direction
	// MaxBoredom stand still timeout
	// Boredom current boredom time in frames
	uint16 _boreNum; // Bore anim frame
	// TalkTime time of talk anim
	// SpecAnim additional anim

	uint16 _currHeight; // height of current anim phase

	// Inventory array of items
	// Inventory2 array of items
	// Font subtitiles font
	// Color subtitiles color
	// AnimSet number of animation set
	Common::Array<Animation *> _moveSet; // MoveAnims MoveSet
	// TurnAnim ??
	
	uint32 _moveDelay;
	uint32 _shadMinus; //??
};

}

#endif

/* vim: set tabstop=4 noexpandtab: */
