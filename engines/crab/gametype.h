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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#pragma once

namespace Crab {

#define STATNAME_HEALTH "health"
#define STATNAME_ATTACK "attack"
#define STATNAME_DEFENSE "defense"
#define STATNAME_SPEED "speed"
// #define STATNAME_CHARISMA "charisma"
// #define STATNAME_INTELLIGENCE "intelligence"

namespace pyrodactyl {

namespace stat {

enum StatType {
	STAT_HEALTH,
	STAT_ATTACK,
	STAT_DEFENSE,
	STAT_SPEED,
	/*STAT_CHARISMA,
	STAT_INTELLIGENCE,*/
	STAT_TOTAL
};

} // End of namespace stat

} // End of namespace pyrodactyl

enum Align { ALIGN_LEFT,
			 ALIGN_CENTER,
			 ALIGN_RIGHT };

enum Direction {
	// An invalid direction, used for collisions
	DIRECTION_NONE = -1,

	// South
	DIRECTION_DOWN,

	// North
	DIRECTION_UP,

	// West
	DIRECTION_LEFT,

	// East
	DIRECTION_RIGHT,

	// Also an invalid direction, used for animations
	DIRECTION_TOTAL
};

enum TextureFlipType {
	// Draw texture normally
	FLIP_NONE,

	// Flipped horizontally
	FLIP_X,

	// Flipped vertically
	FLIP_Y,

	// Flipped horizontally and vertically
	FLIP_XY,

	// Flipped anti-diagonally, where anti-diagonal is from top right corner to bottom left
	FLIP_D,

	// Flipped both anti-diagonally and horizontally
	FLIP_DX,

	// Flipped both anti-diagonally and vertically
	FLIP_DY,

	// Flipped anti-diagonally and horizontally and vertically - MAXIMUM FLIP
	FLIP_XYD
};

} // End of namespace Crab
