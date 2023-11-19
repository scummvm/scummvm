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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "common/language.h"
#include "constants.h"

#include "en_constants.h"
#include "ru_constants.h"
#include "de_constants.h"

const char *const LangConstants::TERRAIN_TYPES[6] = {
	"town", "cave", "towr", "cstl", "dung", "scfi"
};

const char *const LangConstants::OUTDOORS_WALL_TYPES[16] = {
	nullptr, "mount", "ltree", "dtree", "grass", "snotree", "dsnotree",
	"snomnt", "dedltree", "mount", "lavamnt", "palm", "dmount", "dedltree",
	"dedltree", "dedltree"
};

const char *const LangConstants::SURFACE_NAMES[16] = {
	"water.srf", "dirt.srf", "grass.srf", "snow.srf", "swamp.srf",
	"lava.srf", "desert.srf", "road.srf", "dwater.srf", "tflr.srf",
	"sky.srf", "croad.srf", "sewer.srf", "cloud.srf", "scortch.srf",
	"space.srf"
};

const byte LangConstants::SYMBOLS[20][64] = {
	{ // 0
		0x00, 0x00, 0xA8, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x00, 0xA8, 0x9E, 0x9C, 0x9C, 0x9E, 0x9E, 0x9E,
		0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0x9A, 0x9A, 0x9A, 0xAC, 0x9E, 0xAC, 0xA8, 0xA8, 0xA6, 0x97, 0x98,
		0xAC, 0xA0, 0xAC, 0xAC, 0xA4, 0xA6, 0x98, 0x99, 0x00, 0xAC, 0xA0, 0xA0, 0xA8, 0xAC, 0x9A, 0x9A,
		0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xA4, 0x9B, 0x9A, 0x00, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9B, 0x9B,
	},
	{ // 1
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x99, 0x98, 0x98, 0x98, 0x97, 0x97, 0x97, 0x97, 0x97,
		0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9A, 0x9B, 0x9B, 0x9C, 0x9B, 0x9A, 0x9C, 0x9A, 0x9B, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x9A, 0x9B,
	},
	{ // 2
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x98, 0x98, 0x99, 0x98, 0x98, 0x97, 0x98, 0x98,
		0x99, 0x98, 0x98, 0x98, 0x99, 0x99, 0x98, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9B, 0x9B, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9B, 0x99, 0x9A, 0x9B, 0x9B, 0x9A, 0x9A, 0x99, 0x9A,
	},
	{ // 3
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x99, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x9A, 0x98, 0x98, 0x97, 0x97, 0x98, 0x98, 0x98, 0x98,
		0x99, 0x99, 0x98, 0x99, 0x98, 0x98, 0x99, 0x99, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9B, 0x9C, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x9A,
	},
	{ // 4
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E,
		0x9A, 0x9A, 0x9A, 0x99, 0x99, 0x99, 0x99, 0x9A, 0x97, 0x97, 0x97, 0x97, 0x97, 0x98, 0x98, 0x98,
		0x99, 0x99, 0x98, 0x99, 0x99, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A,
		0x9A, 0x9C, 0x9B, 0x9B, 0x9C, 0x9B, 0x9B, 0x9B, 0x9A, 0x99, 0x9B, 0x9B, 0x9A, 0x99, 0x9A, 0x9A,
	},
	{ // 5
		0xA4, 0xA4, 0xA8, 0xA8, 0x00, 0x00, 0x00, 0x00, 0x9E, 0x9E, 0x9E, 0xA0, 0xA8, 0xAC, 0x00, 0x00,
		0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9E, 0xAC, 0x00, 0x97, 0x97, 0x97, 0x98, 0x9C, 0x9C, 0xA0, 0xAC,
		0x99, 0x98, 0x99, 0x99, 0x99, 0x9B, 0xA0, 0xAC, 0x9A, 0x9A, 0x9A, 0x9A, 0x9A, 0x9B, 0xA0, 0xAC,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x9A, 0x9A, 0x9B, 0x9B, 0xA4, 0xAC, 0x00,
	},
	{ // 6
		0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9B, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x98, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A, 0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99,
	},
	{ // 7
		0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x9C, 0x99, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x00, 0x00, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99,
		0x00, 0x00, 0xAC, 0xA0, 0x9B, 0xA0, 0x9E, 0x9C, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9C, 0x99, 0x99,
	},
	{ // 8
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9B, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x9C, 0x99, 0x00, 0xAC, 0xA4, 0x9C, 0x99, 0x9E, 0x9C, 0x99,
	},
	{ // 9
		0x00, 0x00, 0xAC, 0xA4, 0xA0, 0x9C, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x9C, 0xA0, 0x9C, 0x9A,
		0xAC, 0xA4, 0x9C, 0x9A, 0x99, 0x99, 0x99, 0x99, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99,
		0xAC, 0xA4, 0x9C, 0x99, 0x99, 0x99, 0x99, 0x99, 0x00, 0xAC, 0xA0, 0x9C, 0x99, 0x99, 0x99, 0x99,
		0x00, 0xAC, 0xA4, 0x9C, 0x9A, 0x9C, 0x99, 0x99, 0x00, 0x00, 0xAC, 0xA0, 0x9C, 0x9A, 0x99, 0x99,
	},
	{ // 10
		0x99, 0x99, 0x99, 0x9A, 0xA0, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x99, 0x9C, 0x9E, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9C, 0x99, 0x9C, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x99, 0x99, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9A, 0x9B, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
	},
	{ // 11
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0x9E, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x9C, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x9E, 0x9E, 0x9C, 0x9C, 0xA0, 0xAC, 0x00,
	},
	{ // 12
		0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x9B, 0x9C, 0x9E, 0x9C, 0x9C, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9C, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xA4, 0xAC, 0x00,
		0x99, 0x99, 0x9C, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
	},
	{ // 13
		0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC, 0x00, 0x00,
		0x99, 0x9B, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0x00, 0x99, 0x99, 0x9A, 0x99, 0x9C, 0xA0, 0xAC, 0x00,
		0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x99, 0x99, 0x9C, 0xA0, 0xAC,
		0x99, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x99, 0x99, 0x99, 0x9A, 0x9C, 0xA4, 0xAC, 0x00,
	},
	{ // 14
		0x00, 0x00, 0xAC, 0x9E, 0x9C, 0x9C, 0x9C, 0x9B, 0x00, 0xAC, 0x9C, 0xA0, 0x9E, 0xA4, 0xA4, 0xA4,
		0xAC, 0x9C, 0xA4, 0xAC, 0xAC, 0xAC, 0x9C, 0x9E, 0xAC, 0xA0, 0xAC, 0xA8, 0x9E, 0xA8, 0xAC, 0x99,
		0xAC, 0x9E, 0xAC, 0xA8, 0xAC, 0x9E, 0xA4, 0xAC, 0xAC, 0xA4, 0xA0, 0xAC, 0xAC, 0xA0, 0xA4, 0xAC,
		0x00, 0xAC, 0xA4, 0xA0, 0xA0, 0xA4, 0xAC, 0xA4, 0x00, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 15
		0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x99, 0x99, 0x99, 0x99, 0x99, 0x98, 0x99, 0x98,
		0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E, 0x9E, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 16
		0x9B, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0x9C, 0x9C, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x99, 0x99, 0x99, 0x99,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xA0, 0xA0, 0x9E, 0xA0, 0x9E, 0x9E, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 17
		0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0x9C, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x9E, 0x98, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0x99, 0x98,
		0x9C, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9C, 0xA0, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0x9E,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 18
		0x9B, 0x9B, 0x9C, 0x9C, 0x9C, 0x9B, 0x9B, 0x9B, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4,
		0x9E, 0x9E, 0x9E, 0x9E, 0x9C, 0x9C, 0x9C, 0x9E, 0x98, 0x98, 0x98, 0x98, 0x9A, 0x9A, 0x98, 0x99,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9B, 0x9C, 0x9E, 0x9E, 0x9E, 0x9E, 0x9E, 0xA0, 0xA0, 0xA0,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC,
	},
	{ // 19
		0x9C, 0x9B, 0x9C, 0x9C, 0xA0, 0xA4, 0xAC, 0x00, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0x00,
		0x9E, 0x9E, 0x9C, 0x9C, 0x9E, 0xA0, 0xAC, 0x00, 0x99, 0x98, 0x98, 0x99, 0x9A, 0x9A, 0xA0, 0xAC,
		0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0x9C, 0xA0, 0xAC, 0xA0, 0xA0, 0x9E, 0xA0, 0xA0, 0xA0, 0xA0, 0xAC,
		0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xA4, 0xAC, 0x00, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0xAC, 0x00, 0x00,
	}
};

const byte LangConstants::TEXT_COLORS[40][4] = {
	{0x00, 0x19, 0x19, 0x19},
	{0x00, 0x08, 0x08, 0x08},
	{0x00, 0x0F, 0x0F, 0x0F},
	{0x00, 0x15, 0x15, 0x15},
	{0x00, 0x01, 0x01, 0x01},
	{0x00, 0x21, 0x21, 0x21},
	{0x00, 0x26, 0x26, 0x26},
	{0x00, 0x2B, 0x2B, 0x2B},
	{0x00, 0x31, 0x31, 0x31},
	{0x00, 0x36, 0x36, 0x36},
	{0x00, 0x3D, 0x3D, 0x3D},
	{0x00, 0x41, 0x41, 0x41},
	{0x00, 0x46, 0x46, 0x46},
	{0x00, 0x4C, 0x4C, 0x4C},
	{0x00, 0x50, 0x50, 0x50},
	{0x00, 0x55, 0x55, 0x55},
	{0x00, 0x5D, 0x5D, 0x5D},
	{0x00, 0x60, 0x60, 0x60},
	{0x00, 0x65, 0x65, 0x65},
	{0x00, 0x6C, 0x6C, 0x6C},
	{0x00, 0x70, 0x70, 0x70},
	{0x00, 0x75, 0x75, 0x75},
	{0x00, 0x7B, 0x7B, 0x7B},
	{0x00, 0x80, 0x80, 0x80},
	{0x00, 0x85, 0x85, 0x85},
	{0x00, 0x8D, 0x8D, 0x8D},
	{0x00, 0x90, 0x90, 0x90},
	{0x00, 0x97, 0x97, 0x97},
	{0x00, 0x9D, 0x9D, 0x9D},
	{0x00, 0xA4, 0xA4, 0xA4},
	{0x00, 0xAB, 0xAB, 0xAB},
	{0x00, 0xB0, 0xB0, 0xB0},
	{0x00, 0xB6, 0xB6, 0xB6},
	{0x00, 0xBD, 0xBD, 0xBD},
	{0x00, 0xC0, 0xC0, 0xC0},
	{0x00, 0xC6, 0xC6, 0xC6},
	{0x00, 0xCD, 0xCD, 0xCD},
	{0x00, 0xD0, 0xD0, 0xD0},
	{0x00, 0xD6, 0xD6, 0xD6},
	{0x00, 0xDB, 0xDB, 0xDB},
};

const byte LangConstants::TEXT_COLORS_STARTUP[40][4] = {
	{0x00, 0x19, 0x19, 0x19},
	{0x00, 0x08, 0x08, 0x08},
	{0x00, 0x0F, 0x0F, 0x0F},
	{0x00, 0x15, 0x15, 0x15},
	{0x00, 0x01, 0x01, 0x01},
	{0x00, 0x1F, 0x1F, 0x1F},
	{0x00, 0x26, 0x26, 0x26},
	{0x00, 0x2B, 0x2B, 0x2B},
	{0x00, 0x31, 0x31, 0x31},
	{0x00, 0x36, 0x36, 0x36},
	{0x00, 0x3D, 0x3D, 0x3D},
	{0x00, 0x42, 0x42, 0x42},
	{0x00, 0x46, 0x46, 0x46},
	{0x00, 0x4C, 0x4C, 0x4C},
	{0x00, 0x50, 0x50, 0x50},
	{0x00, 0x55, 0x55, 0x55},
	{0x00, 0x5D, 0x5D, 0x5D},
	{0x00, 0x60, 0x60, 0x60},
	{0x00, 0x65, 0x65, 0x65},
	{0x00, 0x6C, 0x6C, 0x6C},
	{0x00, 0x70, 0x70, 0x70},
	{0x00, 0x75, 0x75, 0x75},
	{0x00, 0x7B, 0x7B, 0x7B},
	{0x00, 0x80, 0x80, 0x80},
	{0x00, 0x85, 0x85, 0x85},
	{0x00, 0x8D, 0x8D, 0x8D},
	{0x00, 0x90, 0x90, 0x90},
	{0x00, 0x97, 0x97, 0x97},
	{0x00, 0x9D, 0x9D, 0x9D},
	{0x00, 0xA4, 0xA4, 0xA4},
	{0x00, 0xAB, 0xAB, 0xAB},
	{0x00, 0xB0, 0xB0, 0xB0},
	{0x00, 0xB6, 0xB6, 0xB6},
	{0x00, 0xBD, 0xBD, 0xBD},
	{0x00, 0xC0, 0xC0, 0xC0},
	{0x00, 0xC6, 0xC6, 0xC6},
	{0x00, 0xCD, 0xCD, 0xCD},
	{0x00, 0xD0, 0xD0, 0xD0},
	{0x00, 0x19, 0x19, 0x19},
	{0x00, 0x31, 0x31, 0x31}
};

const int LangConstants::RACE_HP_BONUSES[5] = {0, -2, 1, -1, 2};

const int LangConstants::RACE_SP_BONUSES[5][2] = {
	{0, 0}, {2, 0}, {-1, -1}, {1, 1}, {-2, -2}
};

const int LangConstants::CLASS_EXP_LEVELS[10] = {
	1500, 2000, 2000, 1500, 2000, 1000, 1500, 1500, 1500, 2000
};

const int LangConstants::CONDITION_COLORS[17] = {
	9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
};

const int LangConstants::FACE_CONDITION_FRAMES[17] = {
	2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0
};

const int LangConstants::CHAR_FACES_X[6] = {10, 45, 81, 117, 153, 189};

const int LangConstants::HP_BARS_X[6] = {13, 50, 86, 122, 158, 194};

const byte LangConstants::DARKNESS_XLAT[3][256] = {
	{
	0, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	}, {
	0, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0, 0, 0, 0, 0,
	56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0, 0, 0, 0, 0,
	72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0, 0, 0, 0, 0,
	88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0, 0, 0, 0, 0,
	104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0, 0, 0, 0, 0,
	120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0, 0, 0, 0, 0,
	136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0, 0, 0, 0, 0,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0, 0, 0, 0, 0,
	200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0, 0, 0, 0, 0,
	216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0, 0, 0, 0, 0,
	232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0, 0, 0, 0, 0,
	248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0, 0, 0, 0, 0
	}, {
	0, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25, 26, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 0,
	36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 0, 0, 0, 0,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 0, 0, 0, 0,
	68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 0, 0, 0, 0,
	84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 0, 0, 0, 0,
	100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 0, 0, 0, 0,
	116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 0, 0, 0, 0,
	132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 0, 0, 0, 0,
	152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175, 0, 0, 0, 0, 0, 0, 0, 0,
	180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 0, 0, 0, 0,
	196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 0, 0, 0, 0,
	212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 0, 0, 0, 0,
	228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 0, 0, 0, 0,
	244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 0, 0, 0, 0
	}
};

const int LangConstants::SCREEN_POSITIONING_X[4][48] = {
	{
	-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
	-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
	-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
	-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
	}, {
	 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
	}, {
	 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
	 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
	 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
	 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
	}, {
	 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
	-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
	}
};

const int LangConstants::SCREEN_POSITIONING_Y[4][48] = {
	{
	 0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  0,  1
	}, {
	 1,  0,  0,  0, -1,  1,  0,  0,  0, -1,  2,  1,
	 1,  0,  0,  0, -1, -1, -2,  4,  3,  3,  2,  2,
	 1,  1,  0,  0,  0, -1, -1, -2, -2, -3, -3, -4,
	 3,  2,  1,  0,  0, -1, -2, -3,  4, -4,  0,  0
	}, {
	 0,  0,  0,  0,  0, -1, -1, -1, -1, -1, -2, -2,
	-2, -2, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3,
	-3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
	-4, -4, -4, -4, -4, -4, -4, -4, -4, -4,  0, -1
	}, {
	-1,  0,  0,  0,  1, -1,  0,  0,  0,  1, -2, -1,
	-1,  0,  0,  0,  1,  1,  2, -4, -3, -3, -2, -2,
	-1, -1,  0,  0,  0,  1,  1,  2,  2,  3,  3,  4,
	-3, -2, -1,  0,  0,  1,  2,  3, -4,  4,  0,  0
	}
};

const int LangConstants::MONSTER_GRID_BITMASK[12] = {
	0xC, 8, 4, 0, 0xF, 0xF000, 0xF00, 0xF0, 0xF00, 0xF0, 0x0F, 0xF000
};

const int LangConstants::INDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -65, 49, -9, -34, 16, -58, 40 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -14, -98, 16 }
};

const int LangConstants::MAP_OBJECT_Y[2][12] = {
	{ 2, 25, 25, 25, 50, 50, 50, 58, 58, 58, 58, 58 },
	{ -65, -6, -6, -6, 36, 36, 36, 54, 54, 54, 54, 54 }
};

const int LangConstants::INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

const int LangConstants::OUTDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -77, 61, -9, -43, 25, -74, 56 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -24, -98, 16 }
};

const int LangConstants::OUTDOOR_MONSTER_INDEXES[26] = {
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 69, 70,
	71, 72, 73, 74, 75, 90, 91, 92, 93, 94, 112, 115, 118
};

const int LangConstants::OUTDOOR_MONSTERS_Y[26] = {
	59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53, 53,
	53, 53, 53, 53, 53, 34, 34, 34, 34, 34, 2, 2, 2
};

const int LangConstants::DIRECTION_ANIM_POSITIONS[4][4] = {
	{ 0, 1, 2, 3 }, { 3, 0, 1, 2 }, { 2, 3, 0, 1 }, { 1, 2, 3, 0 }
};

const byte LangConstants::WALL_SHIFTS[4][48] = {
	{
		12, 0, 12, 8, 12, 12, 0, 12, 8, 12, 12, 0,
		12, 0, 12, 8, 12, 8, 12, 12, 0, 12, 0, 12,
		0, 12, 0, 12, 8, 12, 8, 12, 8, 12, 8, 12,
		0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 4, 4
	}, {
		8, 12, 8, 4, 8, 8, 12, 8, 4, 8, 8, 12,
		8, 12, 8, 4, 8, 4, 8, 8, 12, 8, 12, 8,
		12, 8, 12, 8, 4, 8, 4, 8, 4, 8, 4, 8,
		12, 12, 12, 12, 4, 4, 4, 4, 0, 0, 0, 0
	}, {
		4, 8, 4, 0, 4, 4, 8, 4, 0, 4, 4, 8,
		4, 8, 4, 0, 4, 0, 4, 4, 8, 4, 8, 4,
		8, 4, 8, 4, 0, 4, 0, 4, 0, 4, 0, 4,
		8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 12, 12
	}, {
		0, 4, 0, 12, 0, 0, 4, 0, 12, 0, 0, 4,
		0, 4, 0, 12, 0, 12, 0, 0, 4, 0, 4, 0,
		4, 0, 4, 0, 12, 0, 12, 0, 12, 0, 12, 0,
		4, 4, 4, 4, 12, 12, 12, 12, 0, 0, 8, 8
	}
};

const int LangConstants::DRAW_NUMBERS[25] = {
	36, 37, 38, 43, 42, 41,
	39, 20, 22, 24, 33, 31,
	29, 26, 10, 11, 18, 16,
	13, 5, 9, 6, 0, 4, 1
};

const int LangConstants::DRAW_FRAMES[25][2] = {
	{ 18, 24 }, { 19, 23 }, { 20, 22 }, { 24, 18 }, { 23, 19 }, { 22, 20 },
	{ 21, 21 }, { 11, 17 }, { 12, 16 }, { 13, 15 }, { 17, 11 }, { 16, 12 },
	{ 15, 13 }, { 14, 14 }, { 6, 10 }, { 7, 9 }, { 10, 6 }, { 9, 7 },
	{ 8, 8 }, { 3, 5 }, { 5, 3 }, { 4, 4 }, { 0, 2 }, { 2, 0 },
	{ 1, 1 }
};

const int LangConstants::COMBAT_FLOAT_X[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };

const int LangConstants::COMBAT_FLOAT_Y[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

const int LangConstants::MONSTER_EFFECT_FLAGS[15][8] = {
	{ 0x104, 0x105, 0x106, 0x107, 0x108, 0x109, 0x10A, 0x10B },
	{ 0x10C, 0x10D, 0x10E, 0x10F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x110, 0x111, 0x112, 0x113, 0x0, 0x0, 0x0, 0x0 },
	{ 0x114, 0x115, 0x116, 0x117, 0x0, 0x0, 0x0, 0x0 },
	{ 0x200, 0x201, 0x202, 0x203, 0x0, 0x0, 0x0, 0x0 },
	{ 0x300, 0x301, 0x302, 0x303, 0x400, 0x401, 0x402, 0x403 },
	{ 0x500, 0x501, 0x502, 0x503, 0x0, 0x0, 0x0, 0x0 },
	{ 0x600, 0x601, 0x602, 0x603, 0x0, 0x0, 0x0, 0x0 },
	{ 0x604, 0x605, 0x606, 0x607, 0x608, 0x609, 0x60A, 0x60B },
	{ 0x60C, 0x60D, 0x60E, 0x60F, 0x0, 0x0, 0x0, 0x0 },
	{ 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100, 0x100 },
	{ 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101, 0x101 },
	{ 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102, 0x102 },
	{ 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103, 0x103 },
	{ 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108, 0x108 }
};

const int LangConstants::SPELLS_ALLOWED[3][40] = {
	{
		0, 1, 2, 3, 5, 6, 7, 8, 9, 10,
		12, 14, 16, 23, 26, 27, 28, 30, 31, 32,
		33, 42, 46, 48, 49, 50, 52, 55, 56, 58,
		59, 62, 64, 65, 67, 68, 71, 73, 74, 76
	}, {
		1, 4, 11, 13, 15, 17, 18, 19, 20, 21,
		22, 24, 25, 29, 34, 35, 36, 37, 38, 39,
		40, 41, 42, 43, 44, 45, 47, 51, 53, 54,
		57, 60, 61, 63, 66, 69, 70, 72, 75, 76
	}, {
		0, 1, 2, 3, 4, 5, 7, 9, 10, 20,
		25, 26, 27, 28, 30, 31, 34, 38, 40, 41,
		42, 43, 44, 45, 49, 50, 52, 53, 55, 59,
		60, 61, 62, 67, 68, 72, 73, 74, 75, 76
	}
};

const int LangConstants::BASE_HP_BY_CLASS[10] = { 10, 8, 7, 5, 4, 8, 7, 12, 6, 9 };

const int LangConstants::AGE_RANGES[10] = { 1, 6, 11, 18, 36, 51, 76, 101, 201, 0xffff };

const int LangConstants::AGE_RANGES_ADJUST[2][10] = {
	{ -250, -50, -20, -10, 0, -2, -5, -10, -20, -50 },
	{ -250, -50, -20, -10, 0, 2, 5, 10, 20, 50 }
};

const int LangConstants::STAT_VALUES[24] = {
	3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 30, 35, 40,
	50, 75, 100, 125, 150, 175, 200, 225, 250, 65535
};

const int LangConstants::STAT_BONUSES[24] = {
	-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20
};

//                                                fire elec ice acid energy magic
const int LangConstants::ELEMENTAL_CATEGORIES[6] = { 8, 15, 20, 25, 33, 36 };

const int LangConstants::ATTRIBUTE_CATEGORIES[10] = {
	9, 17, 25, 33, 39, 45, 50, 56, 61, 72
};

const int LangConstants::ATTRIBUTE_BONUSES[72] = {
	2, 3, 5, 8, 12, 17, 23, 30, 38, 47,	// Might bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// int LangConstants::bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// PER bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// SPD bonus
	3, 5, 10, 15, 20, 30,				// ACC bonus
	5, 10, 15, 20, 25, 30,				// LUC bonus
	4, 6, 10, 20, 50,					// HP bonus
	4, 8, 12, 16, 20, 25,				// SP bonus
	2, 4, 6, 10, 16,					// AC bonus
	4, 6, 8, 10, 12, 14, 16, 18, 20, 25	// Thievery bonus
};

const int LangConstants::ELEMENTAL_RESISTENCES[37] = {
	0, 5, 7, 9, 12, 15, 20, 25, 30, 5, 7, 9, 12, 15, 20, 25,
	5, 10, 15, 20, 25, 10, 15, 20, 25, 40, 5, 7, 9, 11, 13, 15, 20, 25,
	5, 10, 20
};

const int LangConstants::ELEMENTAL_DAMAGE[37] = {
	0, 2, 3, 4, 5, 10, 15, 20, 30, 2, 3, 4, 5, 10, 15, 20, 2, 4, 5, 10, 20,
	2, 4, 8, 16, 32, 2, 3, 4, 5, 10, 15, 20, 30, 5, 10, 25
};

const int LangConstants::WEAPON_DAMAGE_BASE[35] = {
	0, 3, 2, 3, 2, 2, 4, 1, 2, 4, 2, 3,
	2, 2, 1, 1, 1, 1, 4, 4, 3, 2, 4, 2,
	2, 2, 5, 3, 3, 3, 3, 5, 4, 2, 6
};

const int LangConstants::WEAPON_DAMAGE_MULTIPLIER[35] = {
	0, 3, 3, 4, 5, 4, 2, 3, 3, 3, 3, 3,
	2, 4, 10, 6, 8, 9, 4, 3, 6, 8, 5, 6,
	4, 5, 3, 5, 6, 7, 2, 2, 2, 2, 4
};

const int LangConstants::METAL_DAMAGE[22] = {
	-3, -6, -4, -2, 2, 4, 6, 8, 10, 0, 1,
	1, 2, 2, 3, 4, 5, 12, 15, 20, 30, 50
};

const int LangConstants::METAL_DAMAGE_PERCENT[22] = {
	-3, -4, 3, 2, 1, 2, 3, 4, 6, 0, 1,
	1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const int LangConstants::METAL_LAC[22] = {
	-3, 0, -2, -1, 1, 2, 4, 6, 8, 0, 1,
	1, 2, 2, 3, 4, 5, 10, 12, 14, 16, 20
};

const int LangConstants::ARMOR_STRENGTHS[14] = { 0, 2, 4, 5, 6, 7, 8, 10, 4, 2, 1, 1, 1, 1 };

const int LangConstants::MAKE_ITEM_ARR1[6] = { 0, 8, 15, 20, 25, 33 };

const int LangConstants::MAKE_ITEM_ARR2[6][7][2] = {
	//  L1        L2         L3        L4        L5        L6        L7
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } }, // fire
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 7 }, { 7, 7 } }, // electricity
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 5, 5 } }, // frost
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } }, // acid
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } }, // energy
	{ { 0, 0 }, { 1, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 3 }, { 3, 3 } }  // magic
};

const int LangConstants::MAKE_ITEM_ARR3[10][7][2] = {
	//  L1        L2         L3        L4        L5        L6        L7
	{ { 0, 0 }, { 1, 4 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 10 }, { 10, 10 } }, // str
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },    // int
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },    // per
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },    // spd
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },    // acc
	{ { 0, 0 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 6 } },    // luc
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },    // hp
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },    // sp
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },    // ac
	{ { 0, 0 }, { 1, 2 }, { 1, 4 }, { 3, 6 }, { 5, 8 }, { 7, 10 }, { 10, 10 } }  // thievery
};

const int LangConstants::MAKE_ITEM_ARR4[2][7][2] = {
	//  L1        L2         L3        L4        L5        L6        L7
	{ { 0, 0 }, { 1, 4 }, { 3, 7 }, { 4, 8 }, { 5, 9 }, { 8, 9 }, { 9, 9 } },      // common
	{ { 0, 0 }, { 1, 4 }, { 2, 6 }, { 4, 7 }, { 6, 10 }, { 9, 13 }, { 13, 13 } }   // rare and precious
};

const int LangConstants::MAKE_ITEM_ARR5[8][2] = {
	//           L1        L2         L3             L4
	{ 0, 0 }, { 1, 15 }, { 16, 30 }, { 31, 40 }, { 41, 50 },
	//   L5        L6           L7
	{ 51, 60 }, { 61, 73 }, { 61, 73 }
};

const int LangConstants::OUTDOOR_DRAWSTRUCT_INDEXES[44] = {
	37, 38, 39, 40, 41, 44, 42, 43, 47, 45, 46,
	48, 49, 52, 50, 51, 66, 67, 68, 69, 70, 71,
	72, 75, 73, 74, 87, 88, 89, 90, 91, 94, 92,
	93, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120
};

const int LangConstants::TOWN_MAXES[2][11] = {
	{ 23, 13, 32, 16, 26, 16, 16, 16, 16, 16, 16 },
	{ 26, 19, 48, 27, 26, 37, 16, 16, 16, 16, 16 }
};

const char *const LangConstants::TOWN_ACTION_MUSIC[2][7] = {
	{ "bank.m", "smith.m", "guild.m", "tavern.m",
	"temple.m", "grounds.m", "endgame.m" },
	{ "bank.m", "sf09.m", "guild.m", "tavern.m",
	"temple.m", "smith.m", "endgame.m" }
};

const char *const LangConstants::TOWN_ACTION_SHAPES[7] = {
	"bnkr", "blck", "gild", "tvrn", "tmpl", "trng", "eface08"
};

const int LangConstants::TOWN_ACTION_FILES[2][7] = {
	{ 3, 2, 4, 2, 4, 2, 1 }, { 5, 3, 7, 5, 4, 6, 1 }
};

const int LangConstants::TAVERN_EXIT_LIST[2][6][5][2] = {
	{
		{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
		{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
		{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
		{ { 18, 4 }, { 0, 0 }, { 19, 16 }, { 0, 0 }, { 11, 12 } },
		{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
		{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } },
	}, {
		{ { 21, 17 }, { 0, 0 }, { 20, 3 }, { 0, 0 }, { 0, 0 } },
		{ { 13, 4 }, { 0, 0 }, { 19, 9 }, { 0, 0 }, { 0, 0 } },
		{ { 20, 10 }, { 12, 8 }, { 5, 26 }, { 3, 4 }, { 7, 5 } },
		{ { 17, 24 }, { 14, 13 }, { 0, 0 }, { 0, 0 }, { 9, 4 } },
		{ { 15, 21 }, { 0, 0 }, { 13, 21 }, { 0, 0 }, { 0, 0 } },
		{ { 10, 8 }, { 0, 0 }, { 15, 12 }, { 0, 0 }, { 0, 0 } }
	}
};

const int LangConstants::MISC_SPELL_INDEX[74] = {
	NO_SPELL, MS_Light, MS_Awaken, MS_MagicArrow,
	MS_FirstAid, MS_FlyingFist, MS_EnergyBlast, MS_Sleep,
	MS_Revitalize, MS_CureWounds, MS_Sparks, MS_Shrapmetal,
	MS_InsectSpray, MS_ToxicCloud, MS_ProtFromElements, MS_Pain,
	MS_Jump, MS_BeastMaster, MS_Clairvoyance, MS_TurnUndead,
	MS_Levitate, MS_WizardEye, MS_Bless, MS_IdentifyMonster,
	MS_LightningBolt, MS_HolyBonus, MS_PowerCure, MS_NaturesCure,
	MS_LloydsBeacon, MS_PowerShield, MS_Heroism, MS_Hynotize,
	MS_WalkOnWater, MS_FrostBite, MS_DetectMonster, MS_Fireball,
	MS_ColdRay, MS_CurePoison, MS_AcidSpray, MS_TimeDistortion,
	MS_DragonSleep, MS_CureDisease, MS_Teleport, MS_FingerOfDeath,
	MS_CureParalysis, MS_GolemStopper, MS_PoisonVolley, MS_DeadlySwarm,
	MS_SuperShelter, MS_DayOfProtection, MS_DayOfSorcery, MS_CreateFood,
	MS_FieryFlail, MS_RechargeItem, MS_FantasticFreeze, MS_TownPortal,
	MS_StoneToFlesh, MS_RaiseDead, MS_Etheralize, MS_DancingSword,
	MS_MoonRay, MS_MassDistortion, MS_PrismaticLight, MS_EnchantItem,
	MS_Incinerate, MS_HolyWord, MS_Resurrection, MS_ElementalStorm,
	MS_MegaVolts, MS_Inferno, MS_SunRay, MS_Implosion,
	MS_StarBurst, MS_DivineIntervention
};

const int LangConstants::SPELL_COSTS[77] = {
	8, 1, 5, -2, 5, -2, 20, 10, 12, 8, 3,
	- 3, 75, 40, 12, 6, 200, 10, 100, 30, -1, 30,
	15, 25, 10, -2, 1, 2, 7, 20, -2, -2, 100,
	15, 5, 100, 35, 75, 5, 20, 4, 5, 1, -2,
	6, 2, 75, 40, 60, 6, 4, 25, -2, -2, 60,
	- 1, 50, 15, 125, 2, -1, 3, -1, 200, 35, 150,
	15, 5, 4, 10, 8, 30, 4, 5, 7, 5, 0
};

const int LangConstants::DARK_SPELL_RANGES[12][2] = {
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 },
	{ 0, 17 }, { 14, 34 }, { 26, 37 }, { 29, 39 },
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }
};

const int LangConstants::SWORDS_SPELL_RANGES[12][2] = {
	{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 },
	{ 0, 17 },{ 14, 34 },{ 26, 39 },{ 29, 39 },
	{ 0, 20 },{ 16, 35 },{ 27, 39 },{ 29, 39 }
};

const int LangConstants::CLOUDS_GUILD_SPELLS[5][20] = {
	{
		1, 10, 20, 26, 27, 38, 40, 42, 45, 50,
		55, 59, 60, 61, 62, 68, 72, 75, 77, 77
	}, {
		3, 4, 5, 14, 15, 25, 30, 31, 34, 41,
		49, 51, 53, 67, 73, 75, -1, -1, -1, -1
	}, {
		4, 8, 9, 12, 13, 22, 23, 24, 28, 34,
		41, 44, 52, 70, 73, 74, -1, -1, -1, -1
	}, {
		6, 7, 9, 11, 12, 13, 17, 21, 22, 24,
		29, 36, 56, 58, 64, 71, -1, -1, -1, -1
	}, {
		6, 7, 9, 11, 12, 13, 18, 21, 29, 32,
		36, 37, 46, 51, 56, 58, 69, -1, -1, -1
	}
};

const int LangConstants::DARK_SPELL_OFFSETS[3][39] = {
	{
		42, 1, 26, 59, 27, 10, 50, 68, 55, 62, 67, 73, 2,
		5, 3, 31, 30, 52, 49, 28, 74, 0, 9, 7, 14, 8,
		33, 6, 23, 71, 64, 56, 48, 46, 12, 32, 58, 65, 16
	}, {
		42, 1, 45, 61, 72, 40, 20, 60, 38, 41, 75, 34, 4,
		43, 25, 53, 44, 15, 70, 17, 24, 69, 22, 66, 57, 11,
		29, 39, 51, 21, 19, 36, 47, 13, 54, 37, 18, 35, 63
	}, {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
		13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
		26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38
	}
};

const int LangConstants::SPELL_GEM_COST[77] = {
	0, 0, 2, 1, 2, 4, 5, 0, 0, 0, 0, 10, 10, 10, 0, 0, 20, 4, 10, 20, 1, 10,
	5, 5, 4, 2, 0, 0, 0, 10, 3, 1, 20, 4, 0, 20, 10, 10, 1, 10, 0, 0, 0, 2,
	2, 0, 10, 10, 10, 0, 0, 10, 3, 2, 10, 1, 10, 10, 20, 0, 0, 1, 1, 20, 5, 20,
	5, 0, 0, 0, 0, 5, 1, 2, 0, 2, 0
};

const int LangConstants::BLACKSMITH_MAP_IDS[2][4] = { { 28, 30, 73, 49 }, { 29, 31, 37, 43 } };

const int LangConstants::WEAPON_BASE_COSTS[35] = {
	0, 50, 15, 100, 80, 40, 60, 1, 10, 150, 30, 60, 8, 50,
	100, 15, 30, 15, 200, 80, 250, 150, 400, 100, 40, 120,
	300, 100, 200, 300, 25, 100, 50, 15, 0
};
const int LangConstants::ARMOR_BASE_COSTS[14] = {
	0, 20, 100, 200, 400, 600, 1000, 2000, 100, 60, 40, 250, 200, 100
};
const int LangConstants::ACCESSORY_BASE_COSTS[11] = {
	0, 100, 100, 250, 100, 50, 300, 200, 500, 1000, 2000
};
const int LangConstants::MISC_MATERIAL_COSTS[22] = {
	0, 50, 1000, 500, 10, 100, 20, 10, 50, 10, 10, 100,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int LangConstants::MISC_BASE_COSTS[76] = {
	0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 200, 200, 200, 300, 300, 300, 300, 300,
	300, 300, 300, 300, 300, 400, 400, 400, 400, 400, 400, 400,
	400, 400, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
	600, 600, 600, 600
};
const int LangConstants::METAL_BASE_MULTIPLIERS[22] = {
	10, 25, 5, 75, 2, 5, 10, 20, 50, 2, 3, 5, 10, 20, 30, 40,
	50, 60, 70, 80, 90, 100
};
const int LangConstants::ITEM_SKILL_DIVISORS[4] = { 1, 2, 100, 10 };

const int LangConstants::RESTRICTION_OFFSETS[4] = { 0, 35, 49, 60 };

const int LangConstants::ITEM_RESTRICTIONS[86] = {
	0, 86, 86, 86, 86, 86, 86, 0, 6, 239, 239, 239, 2, 4, 4, 4, 4,
	6, 70, 70, 70, 70, 94, 70, 0, 4, 239, 86, 86, 86, 70, 70, 70, 70,
	0, 0, 0, 68, 100, 116, 125, 255, 255, 85, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int LangConstants::NEW_CHAR_SKILLS[10] = { 1, 5, -1, -1, 4, 0, 0, -1, 6, 11 };
const int LangConstants::NEW_CHAR_SKILLS_LEN[10] = { 11, 8, 0, 0, 12, 8, 8, 0, 9, 11 };
const int LangConstants::NEW_CHAR_RACE_SKILLS[10] = { 14, -1, 17, 16, -1, 0, 0, 0, 0, 0 };

const int LangConstants::RACE_MAGIC_RESISTENCES[5] = { 7, 5, 20, 0, 0 };
const int LangConstants::RACE_FIRE_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int LangConstants::RACE_ELECTRIC_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int LangConstants::RACE_COLD_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int LangConstants::RACE_ENERGY_RESISTENCES[5] = { 7, 5, 2, 5, 0 };
const int LangConstants::RACE_POISON_RESISTENCES[5] = { 7, 0, 2, 20, 0 };
const int LangConstants::NEW_CHARACTER_SPELLS[10][4] = {
	{ -1, -1, -1, -1 },
	{ 21, -1, -1, -1 },
	{ 22, -1, -1, -1 },
	{ 21, 1, 14, -1 },
	{ 22, 0, 25, -1 },
	{ -1, -1, -1, -1 },
	{ -1, -1, -1, -1 },
	{ -1, -1, -1, -1 },
	{ 20, 1, 11, 23 },
	{ 20, 1, -1, -1 }
};

const int LangConstants::TOWN_MAP_NUMBERS[3][5] = {
	{ 28, 29, 30, 31, 32 }, { 29, 31, 33, 35, 37 }, { 53, 92, 63, 0, 0 }
};

const char *const LangConstants::EVENT_SAMPLES[6] = {
	"ahh.voc", "whereto.voc", "gulp.voc", "null.voc", "scream.voc", "laff1.voc"
};

const char *const LangConstants::MUSIC_FILES1[5] = {
	"outdoors.m", "town.m", "cavern.m", "dungeon.m", "castle.m"
};

const char *const LangConstants::MUSIC_FILES2[6][7] = {
	{ "outday1.m", "outday2.m", "outday4.m", "outnght1.m",
	"outnght2.m", "outnght4.m", "daydesrt.m" },
	{ "townday1.m", "twnwlk.m", "newbrigh.m", "twnnitea.m",
	"twnniteb.m", "twnwlk.m", "townday1.m" },
	{ "cavern1.m", "cavern2.m", "cavern3a.m", "cavern1.m",
	"cavern2.m", "cavern3a.m", "cavern1.m" },
	{ "dngon1.m", "dngon2.m", "dngon3.m", "dngon1.m",
	"dngon2.m", "dngon3.m", "dngon1.m" },
	{ "cstl1rev.m", "cstl2rev.m", "cstl3rev.m", "cstl1rev.m",
	"cstl2rev.m", "cstl3rev.m", "cstl1rev.m" },
	{ "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m", "sf05.m" }
};


void LangConstants::writeConstants(Common::String num) {
	Common::MemFile file;
	file.syncString(CLOUDS_CREDITS());
	file.syncString(DARK_SIDE_CREDITS());
	file.syncString(SWORDS_CREDITS1());
	file.syncString(SWORDS_CREDITS2());
	file.syncString(OPTIONS_MENU());
	file.syncStrings(GAME_NAMES(), 3);
	file.syncString(THE_PARTY_NEEDS_REST());
	file.syncString(WHO_WILL());
	file.syncString(HOW_MUCH());
	file.syncString(WHATS_THE_PASSWORD());
	file.syncString(PASSWORD_INCORRECT());
	file.syncString(IN_NO_CONDITION());
	file.syncString(NOTHING_HERE());
	file.syncStrings(TERRAIN_TYPES, 6);
	file.syncStrings(OUTDOORS_WALL_TYPES, 16);
	file.syncStrings(SURFACE_NAMES, 16);
	file.syncStrings(WHO_ACTIONS(), 32);
	file.syncStrings(WHO_WILL_ACTIONS(), 4);
	file.syncBytes2D((const byte *)SYMBOLS, 20, 64);
	file.syncBytes2D((const byte *)TEXT_COLORS, 40, 4);
	file.syncBytes2D((const byte *)TEXT_COLORS_STARTUP, 40, 4);
	file.syncStrings(DIRECTION_TEXT_UPPER(), 4);
	file.syncStrings(DIRECTION_TEXT(), 4);
	file.syncStrings(RACE_NAMES(), 5);
	file.syncNumbers(RACE_HP_BONUSES, 5);
	file.syncNumbers2D((const int *)RACE_SP_BONUSES, 5, 2);
	file.syncStrings(CLASS_NAMES(), 11);
	file.syncNumbers(CLASS_EXP_LEVELS, 10);
	file.syncStrings(ALIGNMENT_NAMES(), 3);
	file.syncStrings(SEX_NAMES(), 2);
	file.syncStrings(SKILL_NAMES(), 18);
	file.syncStrings(CONDITION_NAMES_M(), 17);
	file.syncStrings(CONDITION_NAMES_F(), 17);
	file.syncNumbers(CONDITION_COLORS, 17);
	file.syncString(GOOD());
	file.syncString(BLESSED());
	file.syncString(POWER_SHIELD());
	file.syncString(HOLY_BONUS());
	file.syncString(HEROISM());
	file.syncString(IN_PARTY());
	file.syncString(PARTY_DETAILS());
	file.syncString(PARTY_DIALOG_TEXT());
	file.syncNumbers(FACE_CONDITION_FRAMES, 17);
	file.syncNumbers(CHAR_FACES_X, 6);
	file.syncNumbers(HP_BARS_X, 6);
	file.syncString(NO_ONE_TO_ADVENTURE_WITH());
	file.syncBytes2D((const byte *)DARKNESS_XLAT, 3, 256);
	file.syncString(YOUR_ROSTER_IS_FULL());
	file.syncString(PLEASE_WAIT());
	file.syncString(OOPS());
	file.syncNumbers2D((const int *)SCREEN_POSITIONING_X, 4, 48);
	file.syncNumbers2D((const int *)SCREEN_POSITIONING_Y, 4, 48);
	file.syncNumbers(MONSTER_GRID_BITMASK, 12);
	file.syncNumbers2D((const int *)INDOOR_OBJECT_X, 2, 12);
	file.syncNumbers2D((const int *)MAP_OBJECT_Y, 2, 12);
	file.syncNumbers(INDOOR_MONSTERS_Y, 4);
	file.syncNumbers2D((const int *)OUTDOOR_OBJECT_X, 2, 12);
	file.syncNumbers(OUTDOOR_MONSTER_INDEXES, 26);
	file.syncNumbers(OUTDOOR_MONSTERS_Y, 26);
	file.syncNumbers2D((const int *)DIRECTION_ANIM_POSITIONS, 4, 4);
	file.syncBytes2D((const byte *)WALL_SHIFTS, 4, 48);
	file.syncNumbers(DRAW_NUMBERS, 25);
	file.syncNumbers2D((const int *)DRAW_FRAMES, 25, 2);
	file.syncNumbers(COMBAT_FLOAT_X, 8);
	file.syncNumbers(COMBAT_FLOAT_Y, 8);
	file.syncNumbers2D((const int *)MONSTER_EFFECT_FLAGS, 15, 8);
	file.syncNumbers2D((const int *)SPELLS_ALLOWED, 3, 40);
	file.syncNumbers(BASE_HP_BY_CLASS, 10);
	file.syncNumbers(AGE_RANGES, 10);
	file.syncNumbers2D((const int *)AGE_RANGES_ADJUST, 2, 10);
	file.syncNumbers(STAT_VALUES, 24);
	file.syncNumbers(STAT_BONUSES, 24);
	file.syncNumbers(ELEMENTAL_CATEGORIES, 6);
	file.syncNumbers(ATTRIBUTE_CATEGORIES, 10);
	file.syncNumbers(ATTRIBUTE_BONUSES, 72);
	file.syncNumbers(ELEMENTAL_RESISTENCES, 37);
	file.syncNumbers(ELEMENTAL_DAMAGE, 37);
	file.syncNumbers(WEAPON_DAMAGE_BASE, 35);
	file.syncNumbers(WEAPON_DAMAGE_MULTIPLIER, 35);
	file.syncNumbers(METAL_DAMAGE, 22);
	file.syncNumbers(METAL_DAMAGE_PERCENT, 22);
	file.syncNumbers(METAL_LAC, 22);
	file.syncNumbers(ARMOR_STRENGTHS, 14);
	file.syncNumbers(MAKE_ITEM_ARR1, 6);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR2, 6, 7, 2);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR3, 10, 7, 2);
	file.syncNumbers3D((const int *)MAKE_ITEM_ARR4, 2, 7, 2);
	file.syncNumbers2D((const int *)MAKE_ITEM_ARR5, 8, 2);
	file.syncNumbers(OUTDOOR_DRAWSTRUCT_INDEXES, 44);
	file.syncNumbers2D((const int *)TOWN_MAXES, 2, 11);
	file.syncStrings2D((const char *const *)TOWN_ACTION_MUSIC, 2, 7);
	file.syncStrings(TOWN_ACTION_SHAPES, 7);
	file.syncNumbers2D((const int *)TOWN_ACTION_FILES, 2, 7);
	file.syncString(BANK_TEXT());
	file.syncString(BLACKSMITH_TEXT());
	file.syncString(GUILD_NOT_MEMBER_TEXT());
	file.syncString(GUILD_TEXT());
	file.syncString(TAVERN_TEXT());
	file.syncString(GOOD_STUFF());
	file.syncString(HAVE_A_DRINK());
	file.syncString(YOURE_DRUNK());
	file.syncNumbers4D((const int *)TAVERN_EXIT_LIST, 2, 6, 5, 2);
	file.syncString(FOOD_AND_DRINK());
	file.syncString(TEMPLE_TEXT());
	file.syncString(EXPERIENCE_FOR_LEVEL());
	file.syncString(TRAINING_LEARNED_ALL());
	file.syncString(ELIGIBLE_FOR_LEVEL());
	file.syncString(TRAINING_TEXT());
	file.syncString(GOLD_GEMS());
	file.syncString(GOLD_GEMS_2());
	file.syncStrings(DEPOSIT_WITHDRAWL(), 2);
	file.syncString(NOT_ENOUGH_X_IN_THE_Y());
	file.syncString(NO_X_IN_THE_Y());
	file.syncStrings(STAT_NAMES(), 16);
	file.syncStrings(CONSUMABLE_NAMES(), 4);
	file.syncStrings(CONSUMABLE_GOLD_FORMS(), 1);
	file.syncStrings(CONSUMABLE_GEM_FORMS(), 1);
	file.syncStrings(WHERE_NAMES(), 2);
	file.syncString(AMOUNT());
	file.syncString(FOOD_PACKS_FULL());
	file.syncString(BUY_SPELLS());
	file.syncString(GUILD_OPTIONS());
	file.syncNumbers((const int *)MISC_SPELL_INDEX, 74);
	file.syncNumbers((const int *)SPELL_COSTS, 77);
	file.syncNumbers2D((const int *)CLOUDS_GUILD_SPELLS, 5, 20);
	file.syncNumbers2D((const int *)DARK_SPELL_OFFSETS, 3, 39);
	file.syncNumbers2D((const int *)DARK_SPELL_RANGES, 12, 2);
	file.syncNumbers2D((const int *)SWORDS_SPELL_RANGES, 12, 2);
	file.syncNumbers((const int *)SPELL_GEM_COST, 77);
	file.syncString(NOT_A_SPELL_CASTER());
	file.syncString(SPELLS_LEARNED_ALL());
	file.syncString(SPELLS_FOR());
	file.syncString(SPELL_LINES_0_TO_9());
	file.syncString(SPELLS_DIALOG_SPELLS());
	file.syncString(SPELL_PTS());
	file.syncString(GOLD());
	file.syncString(SPELL_INFO());
	file.syncString(SPELL_PURCHASE());
	file.syncString(MAP_TEXT());
	file.syncString(LIGHT_COUNT_TEXT());
	file.syncString(FIRE_RESISTENCE_TEXT());
	file.syncString(ELECRICITY_RESISTENCE_TEXT());
	file.syncString(COLD_RESISTENCE_TEXT());
	file.syncString(POISON_RESISTENCE_TEXT());
	file.syncString(CLAIRVOYANCE_TEXT());
	file.syncString(LEVITATE_TEXT());
	file.syncString(WALK_ON_WATER_TEXT());
	file.syncString(GAME_INFORMATION());
	file.syncString(WORLD_GAME_TEXT());
	file.syncString(DARKSIDE_GAME_TEXT());
	file.syncString(CLOUDS_GAME_TEXT());
	file.syncString(SWORDS_GAME_TEXT());
	file.syncStrings(WEEK_DAY_STRINGS(), 10);
	file.syncString(CHARACTER_DETAILS());
	file.syncStrings(DAYS(), 3);
	file.syncString(PARTY_GOLD());
	file.syncString(PLUS_14());
	file.syncString(CHARACTER_TEMPLATE());
	file.syncString(EXCHANGING_IN_COMBAT());
	file.syncString(CURRENT_MAXIMUM_RATING_TEXT());
	file.syncString(CURRENT_MAXIMUM_TEXT());
	file.syncStrings(RATING_TEXT(), 24);
	file.syncStrings(BORN(), 2);
	file.syncString(AGE_TEXT());
	file.syncString(LEVEL_TEXT());
	file.syncString(RESISTENCES_TEXT());
	file.syncString(NONE());
	file.syncString(EXPERIENCE_TEXT());
	file.syncString(ELIGIBLE());
	file.syncString(IN_PARTY_IN_BANK());
	file.syncStrings(FOOD_ON_HAND(), 3);
	file.syncString(FOOD_TEXT());
	file.syncString(EXCHANGE_WITH_WHOM());
	file.syncString(QUICK_REF_LINE());
	file.syncString(QUICK_REFERENCE());
	file.syncNumbers2D((const int *)BLACKSMITH_MAP_IDS, 2, 4);
	file.syncString(ITEMS_DIALOG_TEXT1());
	file.syncString(ITEMS_DIALOG_TEXT2());
	file.syncString(ITEMS_DIALOG_LINE1());
	file.syncString(ITEMS_DIALOG_LINE2());
	file.syncString(BTN_BUY());
	file.syncString(BTN_SELL());
	file.syncString(BTN_IDENTIFY());
	file.syncString(BTN_FIX());
	file.syncString(BTN_USE());
	file.syncString(BTN_EQUIP());
	file.syncString(BTN_REMOVE());
	file.syncString(BTN_DISCARD());
	file.syncString(BTN_QUEST());
	file.syncString(BTN_ENCHANT());
	file.syncString(BTN_RECHARGE());
	file.syncString(BTN_GOLD());
	file.syncString(ITEM_BROKEN());
	file.syncString(ITEM_CURSED());
	file.syncString(ITEM_OF());
	file.syncStrings(BONUS_NAMES(), 7);
	file.syncStrings(WEAPON_NAMES(), 41);
	file.syncStrings(ARMOR_NAMES(), 14);
	file.syncStrings(ACCESSORY_NAMES(), 11);
	file.syncStrings(MISC_NAMES(), 22);
	file.syncStrings(SPECIAL_NAMES(), 74);
	file.syncStrings(ELEMENTAL_NAMES(), 6);
	file.syncStrings(ATTRIBUTE_NAMES(), 10);
	file.syncStrings(EFFECTIVENESS_NAMES(), 7);
	file.syncStrings(QUEST_ITEM_NAMES(), 85);
	file.syncStrings(QUEST_ITEM_NAMES_SWORDS(), 51);
	file.syncNumbers((const int *)WEAPON_BASE_COSTS, 35);
	file.syncNumbers((const int *)ARMOR_BASE_COSTS, 14);
	file.syncNumbers((const int *)ACCESSORY_BASE_COSTS, 11);
	file.syncNumbers((const int *)MISC_MATERIAL_COSTS, 22);
	file.syncNumbers((const int *)MISC_BASE_COSTS, 76);
	file.syncNumbers((const int *)METAL_BASE_MULTIPLIERS, 22);
	file.syncNumbers((const int *)ITEM_SKILL_DIVISORS, 4);
	file.syncNumbers((const int *)RESTRICTION_OFFSETS, 4);
	file.syncNumbers((const int *)ITEM_RESTRICTIONS, 86);
	file.syncString(NOT_PROFICIENT());
	file.syncString(NO_ITEMS_AVAILABLE());
	file.syncStrings(CATEGORY_NAMES(), 4);
	file.syncString(X_FOR_THE_Y());
	file.syncString(X_FOR_Y());
	file.syncString(X_FOR_Y_GOLD());
	file.syncString(FMT_CHARGES());
	file.syncString(AVAILABLE_GOLD_COST());
	file.syncString(CHARGES());
	file.syncString(COST());
	file.syncStrings(ITEM_ACTIONS(), 7);
	file.syncString(WHICH_ITEM());
	file.syncString(WHATS_YOUR_HURRY());
	file.syncString(USE_ITEM_IN_COMBAT());
	file.syncString(NO_SPECIAL_ABILITIES());
	file.syncString(CANT_CAST_WHILE_ENGAGED());
	file.syncString(EQUIPPED_ALL_YOU_CAN());
	file.syncString(REMOVE_X_TO_EQUIP_Y());
	file.syncString(RING());
	file.syncString(MEDAL());
	file.syncString(CANNOT_REMOVE_CURSED_ITEM());
	file.syncString(CANNOT_DISCARD_CURSED_ITEM());
	file.syncString(PERMANENTLY_DISCARD());
	file.syncString(BACKPACK_IS_FULL());
	file.syncStrings(CATEGORY_BACKPACK_IS_FULL(), 4);
	file.syncString(BUY_X_FOR_Y_GOLD());
	file.syncString(SELL_X_FOR_Y_GOLD());
	file.syncStrings(GOLDS(), 2);
	file.syncString(NO_NEED_OF_THIS());
	file.syncString(NOT_RECHARGABLE());
	file.syncString(SPELL_FAILED());
	file.syncString(NOT_ENCHANTABLE());
	file.syncString(ITEM_NOT_BROKEN());
	file.syncStrings(FIX_IDENTIFY(), 2);
	file.syncString(FIX_IDENTIFY_GOLD());
	file.syncString(IDENTIFY_ITEM_MSG());
	file.syncString(ITEM_DETAILS());
	file.syncString(ALL());
	file.syncString(FIELD_NONE());
	file.syncString(DAMAGE_X_TO_Y());
	file.syncString(ELEMENTAL_XY_DAMAGE());
	file.syncString(ATTR_XY_BONUS());
	file.syncString(EFFECTIVE_AGAINST());
	file.syncString(QUESTS_DIALOG_TEXT());
	file.syncString(CLOUDS_OF_XEEN_LINE());
	file.syncString(DARKSIDE_OF_XEEN_LINE());
	file.syncString(SWORDS_OF_XEEN_LINE());
	file.syncString(NO_QUEST_ITEMS());
	file.syncString(NO_CURRENT_QUESTS());
	file.syncString(NO_AUTO_NOTES());
	file.syncString(QUEST_ITEMS_DATA());
	file.syncString(CURRENT_QUESTS_DATA());
	file.syncString(AUTO_NOTES_DATA());
	file.syncString(REST_COMPLETE());
	file.syncString(PARTY_IS_STARVING());
	file.syncString(HIT_SPELL_POINTS_RESTORED());
	file.syncString(TOO_DANGEROUS_TO_REST());
	file.syncString(SOME_CHARS_MAY_DIE());
	file.syncString(DISMISS_WHOM());
	file.syncString(CANT_DISMISS_LAST_CHAR());
	file.syncString(DELETE_CHAR_WITH_ELDER_WEAPON());
	file.syncStrings(REMOVE_DELETE(), 2);
	file.syncString(REMOVE_OR_DELETE_WHICH());
	file.syncString(YOUR_PARTY_IS_FULL());
	file.syncString(HAS_SLAYER_SWORD());
	file.syncString(SURE_TO_DELETE_CHAR());
	file.syncString(CREATE_CHAR_DETAILS());
	file.syncString(NEW_CHAR_STATS());
	file.syncString(NAME_FOR_NEW_CHARACTER());
	file.syncString(SELECT_CLASS_BEFORE_SAVING());
	file.syncString(EXCHANGE_ATTR_WITH());
	file.syncNumbers((const int *)NEW_CHAR_SKILLS, 10);
	file.syncNumbers((const int *)NEW_CHAR_SKILLS_OFFSET(), 10);
	file.syncNumbers((const int *)NEW_CHAR_SKILLS_LEN, 10);
	file.syncNumbers((const int *)NEW_CHAR_RACE_SKILLS, 10);
	file.syncNumbers((const int *)RACE_MAGIC_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_FIRE_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_ELECTRIC_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_COLD_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_ENERGY_RESISTENCES, 5);
	file.syncNumbers((const int *)RACE_POISON_RESISTENCES, 5);
	file.syncNumbers2D((const int *)NEW_CHARACTER_SPELLS, 10, 4);
	file.syncString(COMBAT_DETAILS());
	file.syncString(NOT_ENOUGH_TO_CAST());
	file.syncStrings(SPELL_CAST_COMPONENTS(), 2);
	file.syncString(CAST_SPELL_DETAILS());
	file.syncString(PARTY_FOUND());
	file.syncString(BACKPACKS_FULL_PRESS_KEY());
	file.syncString(HIT_A_KEY());
	file.syncString(GIVE_TREASURE_FORMATTING());
	file.syncStrings(FOUND(), 2);
	file.syncString(X_FOUND_Y());
	file.syncString(ON_WHO());
	file.syncString(WHICH_ELEMENT1());
	file.syncString(WHICH_ELEMENT2());
	file.syncString(DETECT_MONSTERS());
	file.syncString(LLOYDS_BEACON());
	file.syncString(HOW_MANY_SQUARES());
	file.syncString(TOWN_PORTAL());
	file.syncString(TOWN_PORTAL_SWORDS());
	file.syncNumbers2D((const int *)TOWN_MAP_NUMBERS, 3, 5);
	file.syncString(MONSTER_DETAILS());
	file.syncStrings(MONSTER_SPECIAL_ATTACKS(), 23);
	file.syncString(IDENTIFY_MONSTERS());
	file.syncStrings(EVENT_SAMPLES, 6);
	file.syncString(MOONS_NOT_ALIGNED());
	file.syncString(AWARDS_FOR());
	file.syncString(AWARDS_TEXT());
	file.syncString(NO_AWARDS());
	file.syncString(WARZONE_BATTLE_MASTER());
	file.syncString(WARZONE_MAXED());
	file.syncString(WARZONE_LEVEL());
	file.syncString(WARZONE_HOW_MANY());
	file.syncString(PICKS_THE_LOCK());
	file.syncStrings(PICK_FORM(), 2);
	file.syncString(UNABLE_TO_PICK_LOCK());
	file.syncStrings(UNABLE_TO_PICK_FORM(), 2);
	file.syncString(CONTROL_PANEL_TEXT());
	file.syncString(CONTROL_PANEL_BUTTONS());
	file.syncString(ON());
	file.syncString(OFF());
	file.syncString(CONFIRM_QUIT());
	file.syncString(MR_WIZARD());
	file.syncString(NO_LOADING_IN_COMBAT());
	file.syncString(NO_SAVING_IN_COMBAT());
	file.syncString(QUICK_FIGHT_TEXT());
	file.syncStrings(QUICK_FIGHT_OPTIONS(), 4);
	file.syncStrings(WORLD_END_TEXT(), 9);
	file.syncString(WORLD_CONGRATULATIONS());
	file.syncString(WORLD_CONGRATULATIONS2());
	file.syncString(CLOUDS_CONGRATULATIONS1());
	file.syncString(CLOUDS_CONGRATULATIONS2());
	file.syncStrings(GOOBER(), 3);
	file.syncStrings(MUSIC_FILES1, 5);
	file.syncStrings2D((const char *const *)MUSIC_FILES2, 6, 7);
	file.syncString(DIFFICULTY_TEXT());
	file.syncString(SAVE_OFF_LIMITS());
	file.syncString(CLOUDS_INTRO1());
	file.syncString(DARKSIDE_ENDING1());
	file.syncString(DARKSIDE_ENDING2());
	file.syncString(PHAROAH_ENDING_TEXT1());
	file.syncString(PHAROAH_ENDING_TEXT2());
	file.syncStrings(CLOUDS_MAE_NAMES(), 131);
	file.syncStrings(CLOUDS_MIRROR_LOCATIONS(), 59);
	file.syncStrings(CLOUDS_MAP_NAMES(), 86);
	file.syncStrings(CLOUDS_MONSTERS(), 91);
	file.syncStrings(CLOUDS_SPELLS(), 77);

	Common::String fname = "CONSTANTS" + num;
	Common::File::write(fname.c_str(), file);

	Common::MemFile keys;
	keys.syncNumber(keyConstants()->dialogsCharInfo()->KEY_ITEM());
	keys.syncNumber(keyConstants()->dialogsCharInfo()->KEY_QUICK());
	keys.syncNumber(keyConstants()->dialogsCharInfo()->KEY_EXCHANGE());

	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_FXON());
	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_MUSICON());
	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_LOAD());
	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_SAVE());
	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_QUIT());
	keys.syncNumber(keyConstants()->dialogsControlPanel()->KEY_MRWIZARD());

	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_ROLL());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_CREATE());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_MGT());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_INT());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_PER());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_END());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_SPD());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_ACY());
	keys.syncNumber(keyConstants()->dialogsCreateChar()->KEY_LCK());

	keys.syncNumber(keyConstants()->dialogsDifficulty()->KEY_ADVENTURER());
	keys.syncNumber(keyConstants()->dialogsDifficulty()->KEY_WARRIOR());

	keys.syncNumber(keyConstants()->dialogsItems()->KEY_WEAPONS());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_ARMOR());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_ACCESSORY());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_MISC());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_ENCHANT());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_USE());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_BUY());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_SELL());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_IDENTIFY());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_FIX());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_EQUIP());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_REM());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_DISC());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_QUEST());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_RECHRG());
	keys.syncNumber(keyConstants()->dialogsItems()->KEY_GOLD());

	keys.syncNumber(keyConstants()->dialogsParty()->KEY_DELETE());
	keys.syncNumber(keyConstants()->dialogsParty()->KEY_REMOVE());
	keys.syncNumber(keyConstants()->dialogsParty()->KEY_CREATE());
	keys.syncNumber(keyConstants()->dialogsParty()->KEY_EXIT());

	keys.syncNumber(keyConstants()->dialogsQuests()->KEY_QUEST_ITEMS());
	keys.syncNumber(keyConstants()->dialogsQuests()->KEY_CURRENT_QUESTS());
	keys.syncNumber(keyConstants()->dialogsQuests()->KEY_AUTO_NOTES());

	keys.syncNumber(keyConstants()->dialogsQuickFight()->KEY_NEXT());

	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_CAST());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_NEW());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_FIRE());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_ELEC());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_COLD());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_ACID());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_SET());
	keys.syncNumber(keyConstants()->dialogsSpells()->KEY_RETURN());

	keys.syncNumber(keyConstants()->locations()->KEY_DEP());
	keys.syncNumber(keyConstants()->locations()->KEY_WITH());
	keys.syncNumber(keyConstants()->locations()->KEY_GOLD());
	keys.syncNumber(keyConstants()->locations()->KEY_GEMS());
	keys.syncNumber(keyConstants()->locations()->KEY_BROWSE());
	keys.syncNumber(keyConstants()->locations()->KEY_BUY_SPELLS());
	keys.syncNumber(keyConstants()->locations()->KEY_SPELL_INFO());
	keys.syncNumber(keyConstants()->locations()->KEY_SIGN_IN());
	keys.syncNumber(keyConstants()->locations()->KEY_DRINK());
	keys.syncNumber(keyConstants()->locations()->KEY_FOOD());
	keys.syncNumber(keyConstants()->locations()->KEY_TIP());
	keys.syncNumber(keyConstants()->locations()->KEY_RUMORS());
	keys.syncNumber(keyConstants()->locations()->KEY_HEAL());
	keys.syncNumber(keyConstants()->locations()->KEY_DONATION());
	keys.syncNumber(keyConstants()->locations()->KEY_UNCURSE());
	keys.syncNumber(keyConstants()->locations()->KEY_TRAIN());

	keys.syncNumber(keyConstants()->cloudsOfXeenMenu()->KEY_START_NEW_GAME());
	keys.syncNumber(keyConstants()->cloudsOfXeenMenu()->KEY_LOAD_GAME());
	keys.syncNumber(keyConstants()->cloudsOfXeenMenu()->KEY_SHOW_CREDITS());
	keys.syncNumber(keyConstants()->cloudsOfXeenMenu()->KEY_VIEW_ENDGAME());

	fname = "CONSTKEYS" + num;
	Common::File::write(fname.c_str(), keys);
}

void writeConstants() {
	EN eng;
	eng.writeConstants(Common::String::format("_%i", 7 /*Common::EN_ANY */));
	RU ru;
	ru.writeConstants(Common::String::format("_%i", 26 /* Common::RU_RUS */));
	DE de;
	de.writeConstants(Common::String::format("_%i", 4 /* Common::DE_DEU */));	
}
