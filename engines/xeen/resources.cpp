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

#include "common/scummsys.h"
#include "xeen/resources.h"

namespace Xeen {

const char *const CREDITS =
	"\013""012\010""000\003""c\014""35Designed and Directed By:\n"
	"\014""17Jon Van Caneghem\003""l\n"
	"\n"
	"\t025\014""35Programming:\n"
	"\t035\014""17Mark Caldwell\n"
	"\t035Dave Hathaway\n"
	"\n"
	"\t025\014""35Sound System & FX:\n"
	"\t035\014""17Mike Heilemann\n"
	"\n"
	"\t025\014""35Music & Speech:\n"
	"\t035\014""17Tim Tully\n"
	"\n"
	"\t025\014""35Writing:\n"
	"\t035\014""17Paul Rattner\n"
	"\t035Debbie Van Caneghem\n"
	"\t035Jon Van Caneghem\013""012\n"
	"\n"
	"\n"
	"\t180\014""35Graphics:\n"
	"\t190\014""17Jonathan P. Gwyn\n"
	"\t190Bonita Long-Hemsath\n"
	"\t190Julia Ulano\n"
	"\t190Ricardo Barrera\n"
	"\n"
	"\t180\014""35Testing:\n"
	"\t190\014""17Benjamin Bent\n"
	"\t190Christian Dailey\n"
	"\t190Mario Escamilla\n"
	"\t190Marco Hunter\n"
	"\t190Robert J. Lupo\n"
	"\t190Clayton Retzer\n"
	"\t190David Vela\003""c";

const char *const OPTIONS_TITLE = 
	"\x0D\x01\003""c\014""dMight and Magic Options\n"
	"World of Xeen\x02\n"
	"\v117Copyright (c) 1993 NWC, Inc.\n"
	"All Rights Reserved\x01";

const char *const THE_PARTY_NEEDS_REST = "\x0B""012The Party needs rest!";

const char *const WHO_WILL = "\x03""c\x0B""000\x09""000%s\x0A\x0A"
	"Who will\x0A%s?\x0A\x0B""055F1 - F%d";

const char *const WHATS_THE_PASSWORD = "What's the Password?";

const char *const IN_NO_CONDITION = "\x0B""007%s is not in any condition to perform actions!";

const char *const NOTHING_HERE = "\x03""c\x0B""010Nothing here.";

const char *const TERRAIN_TYPES[6] = {
	"town", "cave", "towr", "cstl", "dung", "scfi"
};

const char *const SURFACE_TYPE_NAMES[15] = {
	nullptr, "mount", "ltree", "dtree", "grass", "snotree", "snomnt",
	"dedltree", "mount", "lavamnt", "palm", "dmount", "dedltree",
	"dedltree", "dedltree"
};

const char *const SURFACE_NAMES[16] = {
	"water.srf", "dirt.srf", "grass.srf", "snow.srf", "swamp.srf",
	"lava.srf", "desert.srf", "road.srf", "dwater.srf", "tflr.srf",
	"sky.srf", "croad.srf", "sewer.srf", "cloud.srf", "scortch.srf",
	"space.srf"
};

const char *const WHO_ACTIONS[32] = {
	"aSearch", "aOpen", "aDrink", "aMine", "aTouch", "aRead", "aLearn", "aTake",
	"aBang", "aSteal", "aBribe", "aPay", "aSit", "aTry", "aTurn", "aBathe",
	"aDestroy", "aPull", "aDescend", "aTossACoin", "aPray", "aJoin", "aAct",
	"aPlay", "aPush", "aRub", "aPick", "aEat", "aSign", "aClose", "aLook", "aTry"
};

const char *const WHO_WILL_ACTIONS[4] = {
	"Open Grate", "Open Door", "Open Scroll", "Select Char"
};

const byte SYMBOLS[20][64] = {
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

const byte TEXT_COLORS[40][4] = {
	{ 0x00, 0x19, 0x19, 0x19 },
	{ 0x00, 0x08, 0x08, 0x08 },
	{ 0x00, 0x0F, 0x0F, 0x0F },
	{ 0x00, 0x15, 0x15, 0x15 },
	{ 0x00, 0x01, 0x01, 0x01 },
	{ 0x00, 0x21, 0x21, 0x21 },
	{ 0x00, 0x26, 0x26, 0x26 },
	{ 0x00, 0x2B, 0x2B, 0x2B },
	{ 0x00, 0x31, 0x31, 0x31 },
	{ 0x00, 0x36, 0x36, 0x36 },
	{ 0x00, 0x3D, 0x3D, 0x3D },
	{ 0x00, 0x41, 0x41, 0x41 },
	{ 0x00, 0x46, 0x46, 0x46 },
	{ 0x00, 0x4C, 0x4C, 0x4C },
	{ 0x00, 0x50, 0x50, 0x50 },
	{ 0x00, 0x55, 0x55, 0x55 },
	{ 0x00, 0x5D, 0x5D, 0x5D },
	{ 0x00, 0x60, 0x60, 0x60 },
	{ 0x00, 0x65, 0x65, 0x65 },
	{ 0x00, 0x6C, 0x6C, 0x6C },
	{ 0x00, 0x70, 0x70, 0x70 },
	{ 0x00, 0x75, 0x75, 0x75 },
	{ 0x00, 0x7B, 0x7B, 0x7B },
	{ 0x00, 0x80, 0x80, 0x80 },
	{ 0x00, 0x85, 0x85, 0x85 },
	{ 0x00, 0x8D, 0x8D, 0x8D },
	{ 0x00, 0x90, 0x90, 0x90 },
	{ 0x00, 0x97, 0x97, 0x97 },
	{ 0x00, 0x9D, 0x9D, 0x9D },
	{ 0x00, 0xA4, 0xA4, 0xA4 },
	{ 0x00, 0xAB, 0xAB, 0xAB },
	{ 0x00, 0xB0, 0xB0, 0xB0 },
	{ 0x00, 0xB6, 0xB6, 0xB6 },
	{ 0x00, 0xBD, 0xBD, 0xBD },
	{ 0x00, 0xC0, 0xC0, 0xC0 },
	{ 0x00, 0xC6, 0xC6, 0xC6 },
	{ 0x00, 0xCD, 0xCD, 0xCD },
	{ 0x00, 0xD0, 0xD0, 0xD0 },
	{ 0x00, 0xD6, 0xD6, 0xD6 },
	{ 0x00, 0xDB, 0xDB, 0xDB },
};

const char *const DIRECTION_TEXT_UPPER[4] = { "NORTH", "EAST", "SOUTH", "WEST" };

const char *const DIRECTION_TEXT[4] = { "North", "East", "South", "West" };

const char *const RACE_NAMES[5] = { "Human", "Elf", "Dwarf", "Gnome", "H-Orc" };

const int RACE_HP_BONUSES[5] = { 0, -2, 1, -1, 2 };

const int RACE_SP_BONUSES[5][2] = {
	{ 0, 0 }, { 2, 0 }, { -1, -1 }, { 1, 1 }, { -2, -2 }
};

const char *const ALIGNMENT_NAMES[3] = { "Good", "Neutral", "Evil" };

const char *const SEX_NAMES[2] = { "Male", "Female" };

const char *const SKILL_NAMES[18] = {
	"Thievery", "Arms Master", "Astrologer", "Body Builder", "Cartographer",
	"Crusader", "Direction Sense", "Linguist", "Merchant", "Mountaineer", 
	"Navigator", "Path Finder", "Prayer Master", "Prestidigitator",
	"Swimmer", "Tracker", "Spot Secret Door", "Danger Sense"
};

const char *const CLASS_NAMES[11] = {
	"Knight", "Paladin", "Archer", "Cleric", "Sorcerer", "Robber", 
	"Ninja", "Barbarian", "Druid", "Ranger", nullptr
};

const uint CLASS_EXP_LEVELS[10] = {
	1500, 2000, 2000, 1500, 2000, 1000, 1500, 1500, 1500, 2000
};

const char *const CONDITION_NAMES[17] = {
	"Cursed", "Heart Broken", "Weak", "Poisoned", "Diseased", 
	"Insane", "In Love", "Drunk", "Asleep", "Depressed", "Confused", 
	"Paralyzed", "Unconscious", "Dead", "Stone", "Eradicated", "Good"
};

const int CONDITION_COLORS[17] = {
	9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
};

const char *const GOOD = "Good";

const char *const BLESSED = "\n\t020Blessed\t095%+d";

const char *const POWER_SHIELD = "\n\t020Power Shield\t095%+d";

const char *const HOLY_BONUS = "\n\t020Holy Bonus\t095%+d";

const char *const HEROISM = "\n\t020Heroism\t095%+d";

const char *const IN_PARTY = "\014""15In Party\014""d";

const char *const PARTY_DETAILS = "\015\003l\002\014""00"
	"\013""001""\011""035%s" 
	"\013""009""\011""035%s" 
	"\013""017""\011""035%s" 
	"\013""025""\011""035%s" 
	"\013""001""\011""136%s" 
	"\013""009""\011""136%s" 
	"\013""017""\011""136%s" 
	"\013""025""\011""136%s" 
	"\013""044""\011""035%s" 
	"\013""052""\011""035%s" 
	"\013""060""\011""035%s" 
	"\013""068""\011""035%s" 
	"\013""044""\011""136%s" 
	"\013""052""\011""136%s" 
	"\013""060""\011""136%s" 
	"\013""068""\011""136%s";

const int FACE_CONDITION_FRAMES[17] = { 
	2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0 
};

const int CHAR_FACES_X[6] = { 10, 45, 81, 117, 153, 189 };

const int HP_BARS_X[6] = { 13, 50, 86, 122, 158, 194 };

const char *const NO_ONE_TO_ADVENTURE_WITH = "You have no one to adventure with";

const char *const YOUR_ROSTER_IS_FULL = "Your Roster is full!";

const byte BACKGROUND_XLAT[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xF7, 0xFF, 0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF9, 0xFF, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF7, 0xFF, 0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF5, 0xFF, 0x0B, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xF3, 0xFF, 0x0D, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

const char *const PLEASE_WAIT = "\014""d\003""c\011""000"
	"\013""002Please Wait...";

const char *const OOPS = "\003""c\011""000\013""002Oops...";

const int8 SCREEN_POSITIONING_X[4][48] = {
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

const int8 SCREEN_POSITIONING_Y[4][48] = {
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

const int INDOOR_OBJECT_X[2][12] = {
	{ 5, -7, -112, 98, -8, -65, 49, -9, -34, 16, -58, 40 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -14, -98, 16 }
};

const int MAP_OBJECT_Y[2][12] = {
	{ 2, 25, 25, 25, 50, 50, 50, 58, 58, 58, 58, 58 },
	{ -65, -6, -6, -6, 36, 36, 36, 54, 54, 54, 54, 54 }
};

const int INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

const int OUTDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -77, 61, -9, -43, 25, -74, 56 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -24, -98, 16 }
};

const int OUTDOOR_MONSTER_INDEXES[26] = {
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 69, 70,
	71, 72, 73, 74, 75, 90, 91, 92, 93, 94, 112, 115, 118
};

const int OUTDOOR_MONSTERS_Y[26] = {
	59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53, 53,
	53, 53, 53, 53, 53, 34, 34, 34, 34, 34, 2, 2, 2
};

const int DIRECTION_ANIM_POSITIONS[4][4] = {
	{ 0, 1, 2, 3 }, { 3, 0, 1, 2 }, { 2, 3, 0, 1 }, { 1, 2, 3, 0 }
};

const byte WALL_SHIFTS[4][48] = {
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

const int DRAW_NUMBERS[25] = {
	36, 37, 38, 43, 42, 41, 
	39, 20, 22, 24, 33, 31, 
	29, 26, 10, 11, 18, 16, 
	13, 5, 9, 6, 0, 4, 1
};

const int DRAW_FRAMES[25][2] = {
	{ 18, 24 }, { 19, 23 }, { 20, 22 }, { 24, 18 }, { 23, 19 }, { 22, 20 },
	{ 21, 21 }, { 11, 17 }, { 12, 16 }, { 13, 15 }, { 17, 11 }, { 16, 12 },
	{ 15, 13 }, { 14, 14 }, { 6, 10 }, { 7, 9 }, { 10, 6 }, { 9, 7 },
	{ 8, 8 }, { 3, 5 }, { 5, 3 }, { 4, 4 }, { 0, 2 }, { 2, 0 },
	{ 1, 1 }
};

const int COMBAT_FLOAT_X[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };

const int COMBAT_FLOAT_Y[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

const int MONSTER_EFFECT_FLAGS[15][8] = {
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

const uint SPELLS_ALLOWED[3][40] = {
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

const int BASE_HP_BY_CLASS[10] = { 10, 8, 7, 5, 4, 8, 7, 12, 6, 9 };

const int AGE_RANGES[10] = { 1, 6, 11, 18, 36, 51, 76, 101, 201, 0xffff };

const int AGE_RANGES_ADJUST[2][10] = {
	{ -250, -50, -20, -10, 0, -2, -5, -10, -20, -50 },
	{ -250, -50, -20, -10, 0, 2, 5, 10, 20, 50 }
};

const uint STAT_VALUES[24] = {
	3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 30, 35, 40, 
	50, 75, 100, 125, 150, 175, 200, 225, 250, 
};

const int STAT_BONUSES[24] = {
	-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20
};

const int ELEMENTAL_CATEGORIES[6] = { 8, 15, 20, 25, 33, 36 };

const int ATTRIBUTE_CATEGORIES[10] = {
	9, 17, 25, 33, 39, 45, 50, 56, 61, 72 };

const int ATTRIBUTE_BONUSES[72] = {
	2, 3, 5, 8, 12, 17, 23, 30, 38, 47,	// Might bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// INT bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// PER bonus
	2, 3, 5, 8, 12, 17, 23, 30,			// SPD bonus
	3, 5, 10, 15, 20, 30,				// ACC bonus
	5, 10, 15, 20, 25, 30,				// LUC bonus
	4, 6, 10, 20, 50,					// HP bonus
	4, 8, 12, 16, 20, 25,				// SP bonus
	2, 4, 6, 10, 16,					// AC bonus
	4, 6, 8, 10, 12, 14, 16, 18, 20, 25	// Thievery bonus
};

const int ELEMENTAL_RESISTENCES[37] = {
	0, 5, 7, 9, 12, 15, 20, 25, 30, 5, 7, 9, 12, 15, 20, 25,
	5, 10, 15, 20, 25, 10, 15, 20, 25, 40, 5, 7, 9, 11, 13, 15, 20, 25,
	5, 10, 20
};

const int ELEMENTAL_DAMAGE[37] = {
	0, 2, 3, 4, 5, 10, 15, 20, 30, 2, 3, 4, 5, 10, 15, 20, 2, 4, 5, 10, 20,
	2, 4, 8, 16, 32, 2, 3, 4, 5, 10, 15, 20, 30, 5, 10, 25
};

const int METAL_LAC[9] = { -3, 0, -2, -1, 1, 2, 4, 6, 8 };

const int ARMOR_STRENGTHS[14] = { 0, 2, 4, 5, 6, 7, 8, 10, 4, 2, 1, 1, 1, 1 };

const int OUTDOOR_DRAWSTRCT_INDEXES[44] = {
	37, 38, 39, 40, 41, 44, 42, 43, 47, 45, 46,
	48, 49, 52, 50, 51, 66, 67, 68, 69, 70, 71,
	72, 75, 73, 74, 87, 88, 89, 90, 91, 94, 92,
	93, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120
};

const int TOWN_MAXES[2][11] = {
	{ 23, 13, 32, 16, 26, 16, 16, 16, 16, 16, 16 },
	{ 26, 19, 48, 27, 26, 37, 16, 16, 16, 16, 16 }
};

const char *const TOWN_ACTION_MUSIC[14] = {
	"bank.m", "smith.m", "guild.m", "tavern.m", "temple.m",
	"grounds.m", "endgame.m", "bank.m", "sf09.m", "guild.m",
	"tavern.m", "temple.m", "smith.m", "endgame.m"
};

const char *const TOWN_ACTION_SHAPES[4] = {
	"bankr", "blck", "gild", "tvrn"
};

const int TOWN_ACTION_FILES[2][7] = {
	{ 3, 2, 4, 2, 4, 2, 1 }, { 5, 3, 7, 5, 4, 6, 1 }
};

const char *const BANK_TEXT = "\x0D\x02\x03""c\x0B""122\x09""013"
	"\x0C""37D\x0C""dep\x09""040\x0C""37W\x0C""dith\x09""067ESC"
	"\x01\x09""000\x0B""000Bank of Xeen\x0B""015\n"
	"Bank\x03l\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s\x03""c\n"
	"\n"
	"Party\x03l\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s";

const char *const BLACKSMITH_TEXT = "\x01\x0D\x03""c\x0B""000\x09""000"
	"Store Options for\x09""039\x0B""027%s\x03""l\x0B""046\n"
	"\x09""011\x0C""37B\x0C""drowse\n"
	"\x09""000\x0B""090Gold\x03r\x09""000%s"
	"\x02\x03""c\x0B""122\x09""040ESC\x01";

const char *const GUILD_NOT_MEMBER_TEXT =
	"\n\nYou have to be a member to shop here.";

const char *const GUILD_TEXT = "\x03""c\x0B""027\x09""039%s"
	"\x03l\x0B""046\n"
	"\x09""012\x0C""37B\x0C""duy Spells\n"
	"\x09""012\x0C""37S\x0C""dpell Info";

const char *const TAVERN_TEXT =
	"\x0D\x03""c\x0B""000\x09""000Tavern Options for\x09""039"
	"\x0B""027%s%s\x03l\x09""000"
	"\x0B""090Gold\x03r\x09""000%s\x02\x03""c\x0B""122"
	"\x09""021\x0C""37S\x0C""dign in\x09""060ESC\x01";

const char *const FOOD_AND_DRINK =
	"\x03l\x09""017\x0B""046\x0C""37D\x0C""drink\n"
	"\x09""017\x0C""37F\x0C""dood\n"
	"\x09""017\x0C""37T\x0C""dip\n"
	"\x09""017\x0C""37R\x0C""dumors";

const char *const GOOD_STUFF = "\n"
	"\n"
	"Good Stuff\n"
	"\n"
	"Hit a key!";

const char *const HAVE_A_DRINK = "\n\nHave a Drink\n\nHit a key!";

const char *const YOURE_DRUNK = "\n\nYou're Drunk\n\nHit a key!";

const int TAVERN_EXIT_LIST[2][6][5][2] = {
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

const char *const TEMPLE_TEXT =
	"\x0D\x03""c\x0B""000\x09""000Temple Options for"
	"\x09""039\x0B""027%s\x03l\x09""000\x0B""046"
	"\x0C""37H\x0C""deal\x03r\x09""000%lu\x03l\n"
	"\x0C""37D\x0C""donation\x03r\x09""000%lu\x03l\n"
	"\x0C""37U\x0C""dnCurse\x03r\x09""000%s"
	"\x03l\x09""000\x0B""090Gold\x03r\x09""000%s"
	"\x02\x03""c\x0B""122\x09""040ESC\x01";

const char *const EXPERIENCE_FOR_LEVEL = 
	"%s needs %lu experience for level %u.";

const char *const LEARNED_ALL = "%s has learned all we can teach!";

const char *const ELIGIBLE_FOR_LEVEL = "%s is eligible for level %d.";

const char *const TRAINING_TEXT =
	"\x0D\x03""cTraining Options\n"
	"\n"
	"%s\x03l\x0B""090\x09""000Gold\x03r\x09"
	"000%s\x02\x03""c\x0B""122\x09""021"
	"\x0C""37T\x0C""drain\x09""060ESC\x01";

const char *const GOLD_GEMS =
	"\x03""c\x0B""000\x09""000%s\x03l\n"
	"\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s\x02\x03""c\x0B""096\x09""013G"
	"\x0C""37o\x0C""dld\x09""040G\x0C\x03""7e"
	"\x0C""dms\x09""067ESC\x01";

const char *const GOLD_GEMS_2 =
	"\x09""000\x0B""000\x03""c%s\x03l\n"
	"\n"
	"\x04""077Gold\x03r\x09""000%s\x03l\n"
	"\x04""077Gems\x03r\x09""000%s\x03l\x09""000\x0B""051\x04""077\n"
	"\x04""077";

const char *const DEPOSIT_WITHDRAWL[2] = { "Deposit", "Withdrawl" };

const char *const NOT_ENOUGH_X_IN_THE_Y =
	"\x03""c\x0B""012Not enough %s in the %s!\x03l";

const char *const NO_X_IN_THE_Y = "\x03""c\x0B""012No %s in the %s!\x03l";

const char *const STAT_NAMES[16] = {
	"Might", "Intellect", "Personality", "Endurance", "Speed",
	"Accuracy", "Luck", "Age", "Level", "Armor Class", "Hit Points",
	"Spell Points", "Resistances", "Skills", "Awards", "Experience"
};

const char *const CONSUMABLE_NAMES[4] = { "Gold", "Gems", "Food", "Condition" };

const char *const WHERE_NAMES[2] = { "Party", "Bank" };

const char *const AMOUNT = "\x03""c\x09""000\x0B""051Amount\x03l\n";

const char *const FOOD_PACKS_FULL = "\v007Your food packs are already full!";

const char *const BUY_SPELLS =
	"\x03""c\x0B""027\x09""039%s\x03l\x0B""046\n"
	"\x09""012\x0C""37B\x0C""duy Spells\n"
	"\x09""012\x0C""37S\x0C""dpell Info";

const char *const GUILD_OPTIONS = 
	"\x0D\x0C""00\x03""c\x0B""000\x09""000Guild Options for%s"
	"\x03l\x09""000\x0B""090Gold"
	"\x03r\x09""000%s\x02\x03""c\x0B""122\x09""040ESC\x01";

const int SPELL_COSTS[77] = {
	8, 1, 5, -2, 5, -2, 20, 10, 12, 8, 3,
	- 3, 75, 40, 12, 6, 200, 10, 100, 30, -1, 30,
	15, 25, 10, -2, 1, 2, 7, 20, -2, -2, 100,
	15, 5, 100, 35, 75, 5, 20, 4, 5, 1, -2,
	6, 2, 75, 40, 60, 6, 4, 25, -2, -2, 60,
	- 1, 50, 15, 125, 2, -1, 3, -1, 200, 35, 150,
	15, 5, 4, 10, 8, 30, 4, 5, 7, 5, 0
};

const int DARK_SPELL_RANGES[12][2] = {
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }, 
	{ 0, 17 }, { 14, 34 }, { 26, 37 }, { 29, 39 }, 
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }
};

const int CLOUDS_SPELL_OFFSETS[5][20] = {
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

const uint DARK_SPELL_OFFSETS[3][39] = {
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

const int SPELL_GEM_COST[77] = {
	0, 0, 2, 1, 2, 4, 5, 0, 0, 0, 0, 10, 10, 10, 0, 0, 20, 4, 10, 20, 1, 10,
	5, 5, 4, 2, 0, 0, 0, 10, 3, 1, 20, 4, 0, 20, 10, 10, 1, 10, 0, 0, 0, 2,
	2, 0, 10, 10, 10, 0, 0, 10, 3, 2, 10, 1, 10, 10, 20, 0, 0, 1, 1, 20, 5, 20,
	5, 0, 0, 0, 0, 5, 1, 2, 0, 2, 0
};

const char *const NOT_A_SPELL_CASTER = "Not a spell caster...";

const char *const SPELLS_FOR = "\xD\xC""d%s\x2\x3""c\x9""000\xB""002Spells for %s";

const char *const SPELL_LINES_0_TO_9 =
	"\x2\x3l\xB""015\x9""0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";

const char *const SPELLS_DIALOG_SPELLS = "\x3l\xB""015"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l\n"
	"\x9""010\xC""%2u%s\xC""d\x3l"
	"\x9""004\xB""110%s - %lu\x1";

const char *const SPELL_PTS = "Spell Pts";

const char *const GOLD = "Gold";

const char *const SPELLS_PRESS_A_KEY =
	"\x3""c\xC""09%s\xC""d\x3l\n"
	"\n"
	"%s\x3""c\x9""000\xB""100Press a Key!";

const char *const SPELLS_PURCHASE =
	"\x3l\xB""000\x9""000\xC""d%s  Do you wish to purchase "
	"\xC""09%s\xC""d for %u?"; 

const char *const SPELL_DETAILS =
	"\xD\x2\x3""c\xB""122\x9""013\xC""37C\xC""dast"
	"\x9""040\xC""37N\xC""dew\x9""067ESC\x1""000\xB""000\x3""cCast Spell\n"
	"\n"
	"%s\x3l\n"
	"\n"
	"Spell Ready:\x3""c\n"
	"\n"
	"\xC""09%s\xC""d\x2\x3l\n"
	"\xB""082Cost\x3r\x9""000%u/%u\x3l\n"
	"Cur SP\x3r\x9""000%u\x1";

const char *const MAP_TEXT =
	"\x3""c\xB""000\x9""000%s\x3l\xB""139"
	"\x9""000X = %d\x3r\x9""000Y = %d\x3""c\x9""000%s";

const char *const LIGHT_COUNT_TEXT = "\x3l\n\n\t024Light\x3r\t124%d";

const char *const FIRE_RESISTENCE_TEXT = "%c%sFire%s%u";

const char *const ELECRICITY_RESISTENCE_TEXT = "%c%sElectricity%s%u";

const char *const COLD_RESISTENCE_TEXT = "c%sCold%s%u";

const char *const POISON_RESISTENCE_TEXT = "%c%sPoison/Acid%s%u";

const char *const CLAIRVOYANCE_TEXT = "%c%sClairvoyance%s";

const char *const LEVITATE_TEXT = "%c%sLevitate%s";

const char *const WALK_ON_WATER_TEXT = "%c%sWalk on Water";

const char *const GAME_INFORMATION = 
	"\xD\x3""c\x9""000\xB""001\xC""37%s of Xeen\xC""d\n"
	"Game Information\n"
	"\n"
	"Today is \xC""37%ssday\xC""d\n"
	"\n"
	"\x9""032Time\x9""072Day\x9""112Year\n"
	"\x9""032\xC""37%d:%02d%c\x9""072%u\x9""112%u\xC""d%s";

const char *const WORLD_GAME_TEXT = "World";
const char *const DARKSIDE_GAME_TEXT = "Darkside";
const char *const CLOUDS_GAME_TEXT = "Clouds";
const char *const SWORDS_GAME_TEXT = "Swords";

const char *const WEEK_DAY_STRINGS[10] = {
	"Ten", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
};

const char *const CHARACTER_DETAILS =
	"\x3l\xB""041\x9""196%s\x9""000\xB""002%s : %s %s %s"
	"\x3r\x9""053\xB""028\xC%02u%u\xC""d\x9""103\xC""%02u%u\xC""d"
	"\x3l\x9""131\xC""%02u%d\xC""d\x9""196\xC""15%lu\xC""d\x3r"
	"\x9""053\xB""051\xC""%02u%u\xC""d\x9""103\xC""%02u%u\xC""d"
	"\x3l\x9""131\xC""%02u%u\xC""d\x9""196\xC""15%lu\xC""d"
	"\x3r\x9""053\xB""074\xC""%02u%u\xC""d\x9""103\xC""%02u%u\xC""d"
	"\x3l\x9""131\xC""15%u\xC""d\x9""196\xC""15%lu\xC""d"
	"\x3r\x9""053\xB""097\xC""%02u%u\xC""d\x9""103\xC""%02u%u\xC""d"
	"\x3l\x9""131\xC""15%u\xC""d\x9""196\xC""15%u day%c\xC""d"
	"\x3r\x9""053\xB""120\xC""%02u%u\xC""d\x9""103\xC""%02u%u\xC""d"
	"\x3l\x9""131\xC""15%u\xC""d\x9""196\xC""%02u%s\xC""d"
	"\x9""230%s%s%s%s\xC""d";

const char *const PARTY_GOLD = "Party Gold";

const char *const PLUS_14 = "14+";

const char *const CHARACTER_TEMPLATE =
	"\x1\xC""00\xD\x3l\x9""029\xB""018Mgt\x9""080Acy\x9""131H.P.\x9""196Experience"
	"\x9""029\xB""041Int\x9""080Lck\x9""131S.P.\x9""029\xB""064Per\x9""080Age"
	"\x9""131Resis\x9""196Party Gems\x9""029\xB""087End\x9""080Lvl\x9""131Skills"
	"\x9""196Party Food\x9""029\xB""110Spd\x9""080AC\x9""131Awrds\x9""196Condition\x3""c"
	"\x9""290\xB""025\xC""37I\xC""dtem\x9""290\xB""057\xC""37Q"
	"\xC""duick\x9""290\xB""089\xC""37E\xC""dxch\x9""290\xB""121Exit\x3l%s";

const char *const EXCHANGING_IN_COMBAT = "\x3""c\xB""007\x9""000Exchanging in combat is not allowed!";

const char *const CURRENT_MAXIMUM_RATING_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%lu\x3l\x9""058/ %lu\n"
	"\x3""cRating: %s";

const char *const CURRENT_MAXIMUM_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%u\x3l\x9""058/ %u";

const char *const RATING_TEXT[24] = {
	"Nonexistant", "Very Poor", "Poor", "Very Low", "Low", "Averarage", "Good",
	"Very Good", "High", "Very High", "Great", "Super", "Amazing", "Incredible",
	"Gigantic", "Fantastic", "Astoundig", "Astonishing", "Monumental", "Tremendous",
	"Collosal", "Awesome", "AweInspiring", "aUltimate"
};

const char *const AGE_TEXT = "\x2\x3""c%s\n"
	"Current / Natural\n"
	"\x3r\x9""057%u\x3l\x9""061/ %u\n"
	"\x3""cBorn: %u / %u\x1";

const char *const LEVEL_TEXT =
	"\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%u\x3l\x9""058/ %u\n"
	"\x3""c%u Attack%s/Round\x1";

const char *const RESISTENCES_TEXT = 
	"\x2\x3""c%s\x3l\n"
	"\x9""020Fire\x9""100%u\n"
	"\x9""020Cold\x9""100%u\n"
	"\x9""020Electricity\x9""100%u\n"
	"\x9""020Poison\x9""100%u\n"
	"\x9""020Energy\x9""100%u\n"
	"\x9""020Magic\x9""100%u";

const char *const NONE = "\n\x9""020";

const char *const EXPERIENCE_TEXT = "\x2\x3""c%s\x3l\n"
	"\x9""010Current:\x9""070%lu\n"
	"\x9""010Next Level:\x9""070%s\x1";

const char *const ELIGIBLE = "\xC""12Eligible\xC""d";

const char *const IN_PARTY_IN_BANK =
	"\x2\x3""cParty %s\n"
	"%lu on hand\n"
	"%lu in bank\x1\x3l";

const char *const FOOD_TEXT =
	"\x2\x3""cParty %s\n"
	"%u on hand\n"
   "Enough for %u day%s\x3l";

const char *const EXCHANGE_WITH_WHOM = "\t010\v005Exchange with whom?";

const char *const QUICK_REF_LINE = 
	"\xB%3d\x9""007%u)\x9""027%s\x9""110%c%c%c\x3r\x9""160\xC%02u%u\xC""d"
	"\x3l\x9""170\xC%02u%d\xC""d\x9""208\xC%02u%u\xC""d\x9""247\xC"
	"%02u%u\xC""d\x9""270\xC%02u%c%c%c%c\xC""d";

const char *const QUICK_REFERENCE =
	"\xD\x3""cQuick Reference Chart\xB""012\x3l"
	"\x9""007#\x9""027Name\x9""110Cls\x9""140Lvl\x9""176H.P."
	"\x9""212S.P.\x9""241A.C.\x9""270Cond"
	"%s%s%s%s%s%s%s%s"
	"\xB""110\x9""064\x3""cGold\x9""144Gems\x9""224Food\xB""119"
	"\x9""064\xC""15%lu\x9""144%lu\x9""224%u day%s\xC""d";

const uint BLACKSMITH_MAP_IDS[2][4] = { { 28, 30, 73, 49 }, { 29, 31, 37, 43 } };

const char *const ITEMS_DIALOG_TEXT1 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153%s\t187%s\t221%s"
	"\t255%s\t289Exit";
const char *const ITEMS_DIALOG_TEXT2 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153\f37%s\t289Exit";
const char *const ITEMS_DIALOG_LINE1 = "\x3r\f%02u\f023%2d)\x3l\t028%s\n";
const char *const ITEMS_DIALOG_LINE2 = "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%lu\n";

const char *const BTN_BUY = "\f37B\fduy";
const char *const BTN_SELL = "\f37S\fdell";
const char *const BTN_IDENTIFY = "\f37I\fddentify";
const char *const BTN_FIX = "\f37F\fdix";
const char *const BTN_USE = "\f37U\fdse";
const char *const BTN_EQUIP = "\f37E\fdquip";
const char *const BTN_REMOVE = "\f37R\fdem";
const char *const BTN_DISCARD = "\f37D\fdisc";
const char *const BTN_QUEST = "\f37Q\fduest";
const char *const BTN_ENCHANT = "E\fdnchant";
const char *const BTN_RECHARGE = "R\fdechrg";
const char *const BTN_GOLD = "G\fdold";

const char *const ITEM_BROKEN = "\f32broken ";
const char *const ITEM_CURSED = "\f09cursed ";
const char *const BONUS_NAMES[7] = {
	"", "Dragon Slayer", "Undead Eater", "Golem Smasher",
	"Bug Zapper", "Monster Masher", "Beast Bopper"
};
const char *const WEAPON_NAMES[35] = {
	nullptr, "long sword ", "short sword ", "broad sword ", "scimitar ", 
	"cutlass ", "sabre ", "club ", "hand axe ", "katana ", "nunchakas ", 
	"wakazashi ", "dagger ", "mace ", "flail ", "cudgel ", "maul ", "spear ", 
	"bardiche ", "glaive ", "halberd ", "pike ", "flamberge ", "trident ",
	"staff ", "hammer ", "naginata ", "battle axe ", "grand axe ", "great axe ", 
	"short bow ", "long bow ", "crossbow ", "sling ", "Xeen Slayer Sword"
};

const char *const ARMOR_NAMES[14] = {
	nullptr, "Robes ", "Scale rmor ", "ring mail ", "chain mail ",
	"splint mail ", "plate mail ", "plate armor ", "shield ",
	"helm ", "boots ", "cloak ", "cape ", "gauntlets "
};

const char *const ACCESSORY_NAMES[11] = {
	nullptr, "ring ", "belt ", "broach ", "medal ", "charm ", "cameo ",
	"scarab ", "pendant ", "necklace ", "amulet "
};

const char *const MISC_NAMES[22] = {
	nullptr, "rod ", "jewel ", "gem ", "box ", "orb ", "horn ", "coin ",
	"wand ", "whistle ", "potion ", "scroll ", "RogueVM", 
	"bogusg", "bogus", "bogus", "bogus", "bogus", 
	"bogus", "bogus", "bogus", "bogus"
};

const int WEAPON_BASE_COSTS[35] = {
	0, 50, 15, 100, 80, 40, 60, 1, 10, 150, 30, 60, 8, 50,
	100, 15, 30, 15, 200, 80, 250, 150, 400, 100, 40, 120,
	300, 100, 200, 300, 25, 100, 50, 15, 0
};
const int ARMOR_BASE_COSTS[25] = {
	0, 20, 100, 200, 400, 600, 1000, 2000, 100, 60, 40, 250, 200, 100
};
const int ACCESSORY_BASE_COSTS[11] = {
	0, 100, 100, 250, 100, 50, 300, 200, 500, 1000, 2000
};
const int MISC_MATERIAL_COSTS[22] = {
	0, 50, 1000, 500, 10, 100, 20, 10, 50, 10, 10, 100,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int MISC_BASE_COSTS[76] = {
	0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 200, 200, 200, 300, 300, 300, 300, 300,
	300, 300, 300, 300, 300, 400, 400, 400, 400, 400, 400, 400,
	400, 400, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
	600, 600, 600, 600
};
const int METAL_BASE_MULTIPLIERS[22] = {
	10, 25, 5, 75, 2, 5, 10, 20, 50, 2, 3, 5, 10, 20, 30, 40,
	50, 60, 70, 80, 90, 100
};
const int ITEM_SKILL_DIVISORS[4] = { 1, 2, 100, 10 };

const int RESTRICTION_OFFSETS[4] = { 0, 35, 49, 60 };

const int ITEM_RESTRICTIONS[86] = {
	0, 86, 86, 86, 86, 86, 86, 0, 6, 239, 239, 239, 2, 4, 4, 4, 4, 
	6, 70, 70, 70, 70, 94, 70, 0, 4, 239, 86, 86, 86, 70, 70, 70, 70, 
	0, 0, 0, 68, 100, 116, 125, 255, 255, 85, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char *const NOT_PROFICIENT =
	"\t000\v007\x3""c%ss are not proficient with a %s!";

const char *const NO_ITEMS_AVAILABLE = "\x3""c\n"
	"\t000No items available.";

const char *const CATEGORY_NAMES[4] = { "Weapons", "Armor", "Accessories", "Miscellaneous" };

const char *const X_FOR_THE_Y =
	"\x1\fd\r%s\v000\t000%s for %s the %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";

const char *const X_FOR_Y =
	"\x1\xC""d\r\x3l\v000\t000%s for %s\x3r\t000%s\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\xC""d";

const char *const X_FOR_Y_GOLD =
	"\x1\fd\r\x3l\v000\t000%s for %s\t150Gold - %lu%s\x3l\v011"
	"\x2%s%s%s%s%s%s%s%s%s\x1\fd";

const char *const FMT_CHARGES = "\x3rr\t000Charges\x3l";

const char *const AVAILABLE_GOLD_COST =
	"\x1\fd\r\x3l\v000\t000Available %s%s\t150Gold - %lu\x3r\t000Cost"
	"\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\xC""d";

const char *const CHARGES = "Charges";

const char *const COST = "Cost";

const char *const ITEM_ACTIONS[7] = {
	"Equip", "Remove", "Use", "Discard", "Enchant", "Recharge", "Gold"
};
const char *const WHICH_ITEM = "\t010\v005%s which item?";

const char *const WHATS_YOUR_HURRY = "\v007What's your hurry?\n"
	"Wait till you get out of here!";

const char *const USE_ITEM_IN_COMBAT = 
	"\v007To use an item in Combat, invoke the Use command on your turn!";

const char *const NO_SPECIAL_ABILITIES = "\v005\x3""c%s\fdhas no special abilities!";

const char *const CANT_CAST_WHILE_ENGAGED = "\x03c\v007Can't cast %s while engaged!";

const char *const EQUIPPED_ALL_YOU_CAN = "\x3""c\v007You have equipped all the %ss you can!";
const char *const REMOVE_X_TO_EQUIP_Y = "\x3""c\v007You must remove %sto equip %s\x8!";
const char *const RING = "ring";
const char *const MEDAL = "medal";

const char *const CANNOT_REMOVE_CURSED_ITEM = "\x3""You cannot remove a cursed item!";

const char *const CANNOT_DISCARD_CURSED_ITEM = "\3x""cYou cannot discard a cursed item!";

} // End of namespace Xeen
