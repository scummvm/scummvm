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
#include "xeen/files.h"
#include "xeen/xeen.h"
#include "xeen/worldofxeen/worldofxeen_resources.h"

namespace Xeen {

Resources *g_resources;

Resources *Resources::init(XeenEngine *vm) {
	if (vm->getGameID() == GType_Clouds || vm->getGameID() == GType_DarkSide
		|| vm->getGameID() == GType_WorldOfXeen)
		g_resources = new WorldOfXeen::WorldOfXeenResources();
	else
		g_resources = new Resources();

	return g_resources;
}

Resources::Resources() {
	g_resources = this;
	g_vm->_files->setGameCc(1);
	
	_globalSprites.load("global.icn");

	File f("mae.xen");
	while (f.pos() < f.size())
		_maeNames.push_back(f.readString());
	f.close();
}

/*------------------------------------------------------------------------*/

const char *const Resources::CREDITS =
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

const char *const Resources::OPTIONS_TITLE =
	"\x0D\x01\003""c\014""dMight and Magic Options\n"
	"World of Xeen\x02\n"
	"\v117Copyright (c) 1993 NWC, Inc.\n"
	"All Rights Reserved\x01";

const char *const Resources::THE_PARTY_NEEDS_REST = "\x0B""012The Party needs rest!";

const char *const Resources::WHO_WILL = "\x03""c\x0B""000\x09""000%s\x0A\x0A"
	"Who will\x0A%s?\x0A\x0B""055F1 - F%d";

const char *const Resources::HOW_MUCH = "\x3""cHow Much\n\n";

const char *const Resources::WHATS_THE_PASSWORD = "What's the Password?";

const char *const Resources::IN_NO_CONDITION = "\x0B""007%s is not in any condition to perform actions!";

const char *const Resources::NOTHING_HERE = "\x03""c\x0B""010Nothing here.";

const char *const Resources::TERRAIN_TYPES[6] = {
	"town", "cave", "towr", "cstl", "dung", "scfi"
};

const char *const Resources::OUTDOORS_WALL_TYPES[16] = {
	nullptr, "mount", "ltree", "dtree", "grass", "snotree", "dsnotree",
	"snomnt", "dedltree", "mount", "lavamnt", "palm", "dmount", "dedltree",
	"dedltree", "dedltree"
};

const char *const Resources::SURFACE_NAMES[16] = {
	"water.srf", "dirt.srf", "grass.srf", "snow.srf", "swamp.srf",
	"lava.srf", "desert.srf", "road.srf", "dwater.srf", "tflr.srf",
	"sky.srf", "croad.srf", "sewer.srf", "cloud.srf", "scortch.srf",
	"space.srf"
};

const char *const Resources::WHO_ACTIONS[32] = {
	"search", "open", "drink", "mine", "touch", "read", "learn", "take",
	"bang", "steal", "bribe", "pay", "sit", "try", "turn", "bathe",
	"destroy", "pull", "descend", "toss a coin", "pray", "join", "act",
	"play", "push", "rub", "pick", "eat", "sign", "close", "look", "try"
};

const char *const Resources::WHO_WILL_ACTIONS[4] = {
	"Open Grate", "Open Door", "Open Scroll", "Select Char"
};

const byte Resources::SYMBOLS[20][64] = {
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

const byte Resources::TEXT_COLORS[40][4] = {
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

const char *const Resources::DIRECTION_TEXT_UPPER[4] = { "NORTH", "EAST", "SOUTH", "WEST" };

const char *const Resources::DIRECTION_TEXT[4] = { "North", "East", "South", "West" };

const char *const Resources::RACE_NAMES[5] = { "Human", "Elf", "Dwarf", "Gnome", "H-Orc" };

const int Resources::RACE_HP_BONUSES[5] = { 0, -2, 1, -1, 2 };

const int Resources::RACE_SP_BONUSES[5][2] = {
	{ 0, 0 }, { 2, 0 }, { -1, -1 }, { 1, 1 }, { -2, -2 }
};

const char *const Resources::ALIGNMENT_NAMES[3] = { "Good", "Neutral", "Evil" };

const char *const Resources::SEX_NAMES[2] = { "Male", "Female" };

const char *const Resources::SKILL_NAMES[18] = {
	"Thievery\t100", "Arms Master", "Astrologer", "Body Builder", "Cartographer",
	"Crusader", "Direction Sense", "Linguist", "Merchant", "Mountaineer",
	"Navigator", "Path Finder", "Prayer Master", "Prestidigitator",
	"Swimmer", "Tracker", "Spot Secret Door", "Danger Sense"
};

const char *const Resources::CLASS_NAMES[11] = {
	"Knight", "Paladin", "Archer", "Cleric", "Sorcerer", "Robber",
	"Ninja", "Barbarian", "Druid", "Ranger", nullptr
};

const uint Resources::CLASS_EXP_LEVELS[10] = {
	1500, 2000, 2000, 1500, 2000, 1000, 1500, 1500, 1500, 2000
};

const char *const Resources::CONDITION_NAMES[17] = {
	"Cursed", "Heart Broken", "Weak", "Poisoned", "Diseased",
	"Insane", "In Love", "Drunk", "Asleep", "Depressed", "Confused",
	"Paralyzed", "Unconscious", "Dead", "Stone", "Eradicated", "Good"
};

const int Resources::CONDITION_COLORS[17] = {
	9, 9, 9, 9, 9, 9, 9, 9, 32, 32, 32, 32, 6, 6, 6, 6, 15
};

const char *const Resources::GOOD = "Good";

const char *const Resources::BLESSED = "\n\t020Blessed\t095%+d";

const char *const Resources::POWER_SHIELD = "\n\t020Power Shield\t095%+d";

const char *const Resources::HOLY_BONUS = "\n\t020Holy Bonus\t095%+d";

const char *const Resources::HEROISM = "\n\t020Heroism\t095%+d";

const char *const Resources::IN_PARTY = "\014""15In Party\014""d";

const char *const Resources::PARTY_DETAILS = "\015\003l\002\014""00"
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
const char *const Resources::PARTY_DIALOG_TEXT =
	"%s\x2\x3""c\v106\t013Up\t048Down\t083\f37D\fdel\t118\f37R\fdem"
	"\t153\f37C\fdreate\t188E\f37x\fdit\x1";

const int Resources::FACE_CONDITION_FRAMES[17] = {
	2, 2, 2, 1, 1, 4, 4, 4, 3, 2, 4, 3, 3, 5, 6, 7, 0
};

const int Resources::CHAR_FACES_X[6] = { 10, 45, 81, 117, 153, 189 };

const int Resources::HP_BARS_X[6] = { 13, 50, 86, 122, 158, 194 };

const char *const Resources::NO_ONE_TO_ADVENTURE_WITH = "You have no one to adventure with";

const char *const Resources::YOUR_ROSTER_IS_FULL = "Your Roster is full!";

const byte Resources::DARKNESS_XLAT[3][256] = {
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

const char *const Resources::PLEASE_WAIT = "\014""d\003""c\011""000"
	"\013""002Please Wait...";

const char *const Resources::OOPS = "\003""c\011""000\013""002Oops...";

const int8 Resources::SCREEN_POSITIONING_X[4][48] = {
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

const int8 Resources::SCREEN_POSITIONING_Y[4][48] = {
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

const int Resources::MONSTER_GRID_BITMASK[12] = {
	0xC, 8, 4, 0, 0xF, 0xF000, 0xF00, 0xF0, 0xF00, 0xF0, 0x0F, 0xF000
};

const int Resources::INDOOR_OBJECT_X[2][12] = {
	{ 5, -7, -112, 98, -8, -65, 49, -9, -34, 16, -58, 40 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -14, -98, 16 }
};

const int Resources::MAP_OBJECT_Y[2][12] = {
	{ 2, 25, 25, 25, 50, 50, 50, 58, 58, 58, 58, 58 },
	{ -65, -6, -6, -6, 36, 36, 36, 54, 54, 54, 54, 54 }
};

const int Resources::INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

const int Resources::OUTDOOR_OBJECT_X[2][12] = {
	{ -5, -7, -112, 98, -8, -77, 61, -9, -43, 25, -74, 56 },
	{ -35, -35, -142, 68, -35, -95, 19, -35, -62, -24, -98, 16 }
};

const int Resources::OUTDOOR_MONSTER_INDEXES[26] = {
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 69, 70,
	71, 72, 73, 74, 75, 90, 91, 92, 93, 94, 112, 115, 118
};

const int Resources::OUTDOOR_MONSTERS_Y[26] = {
	59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 59, 53, 53,
	53, 53, 53, 53, 53, 34, 34, 34, 34, 34, 2, 2, 2
};

const int Resources::DIRECTION_ANIM_POSITIONS[4][4] = {
	{ 0, 1, 2, 3 }, { 3, 0, 1, 2 }, { 2, 3, 0, 1 }, { 1, 2, 3, 0 }
};

const byte Resources::WALL_SHIFTS[4][48] = {
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

const int Resources::DRAW_NUMBERS[25] = {
	36, 37, 38, 43, 42, 41,
	39, 20, 22, 24, 33, 31,
	29, 26, 10, 11, 18, 16,
	13, 5, 9, 6, 0, 4, 1
};

const int Resources::DRAW_FRAMES[25][2] = {
	{ 18, 24 }, { 19, 23 }, { 20, 22 }, { 24, 18 }, { 23, 19 }, { 22, 20 },
	{ 21, 21 }, { 11, 17 }, { 12, 16 }, { 13, 15 }, { 17, 11 }, { 16, 12 },
	{ 15, 13 }, { 14, 14 }, { 6, 10 }, { 7, 9 }, { 10, 6 }, { 9, 7 },
	{ 8, 8 }, { 3, 5 }, { 5, 3 }, { 4, 4 }, { 0, 2 }, { 2, 0 },
	{ 1, 1 }
};

const int Resources::COMBAT_FLOAT_X[8] = { -2, -1, 0, 1, 2, 1, 0, -1 };

const int Resources::COMBAT_FLOAT_Y[8] = { -2, 0, 2, 0, -1, 0, 2, 0 };

const int Resources::MONSTER_EFFECT_FLAGS[15][8] = {
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

const uint Resources::SPELLS_ALLOWED[3][40] = {
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

const int Resources::BASE_HP_BY_CLASS[10] = { 10, 8, 7, 5, 4, 8, 7, 12, 6, 9 };

const int Resources::AGE_RANGES[10] = { 1, 6, 11, 18, 36, 51, 76, 101, 201, 0xffff };

const int Resources::AGE_RANGES_ADJUST[2][10] = {
	{ -250, -50, -20, -10, 0, -2, -5, -10, -20, -50 },
	{ -250, -50, -20, -10, 0, 2, 5, 10, 20, 50 }
};

const uint Resources::STAT_VALUES[24] = {
	3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 25, 30, 35, 40,
	50, 75, 100, 125, 150, 175, 200, 225, 250,
};

const int Resources::STAT_BONUSES[24] = {
	-5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6,
	7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 20
};

const int Resources::ELEMENTAL_CATEGORIES[6] = { 8, 15, 20, 25, 33, 36 };

const int Resources::ATTRIBUTE_CATEGORIES[10] = {
	9, 17, 25, 33, 39, 45, 50, 56, 61, 72 };

const int Resources::ATTRIBUTE_BONUSES[72] = {
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

const int Resources::ELEMENTAL_RESISTENCES[37] = {
	0, 5, 7, 9, 12, 15, 20, 25, 30, 5, 7, 9, 12, 15, 20, 25,
	5, 10, 15, 20, 25, 10, 15, 20, 25, 40, 5, 7, 9, 11, 13, 15, 20, 25,
	5, 10, 20
};

const int Resources::ELEMENTAL_DAMAGE[37] = {
	0, 2, 3, 4, 5, 10, 15, 20, 30, 2, 3, 4, 5, 10, 15, 20, 2, 4, 5, 10, 20,
	2, 4, 8, 16, 32, 2, 3, 4, 5, 10, 15, 20, 30, 5, 10, 25
};

const int Resources::WEAPON_DAMAGE_BASE[35] = {
	0, 3, 2, 3, 2, 2, 4, 1, 2, 4, 2, 3,
	2, 2, 1, 1, 1, 1, 4, 4, 3, 2, 4, 2,
	2, 2, 5, 3, 3, 3, 3, 5, 4, 2, 6
};

const int Resources::WEAPON_DAMAGE_MULTIPLIER[35] = {
	0, 3, 3, 4, 5, 4, 2, 3, 3, 3, 3, 3,
	2, 4, 10, 6, 8, 9, 4, 3, 6, 8, 5, 6,
	4, 5, 3, 5, 6, 7, 2, 2, 2, 2, 4
};

const int Resources::METAL_DAMAGE[22] = {
	-3, -6, -4, -2, 2, 4, 6, 8, 10, 0, 1,
	1, 2, 2, 3, 4, 5, 12, 15, 20, 30, 50
};

const int Resources::METAL_DAMAGE_PERCENT[22] = {
	253, 252, 3, 2, 1, 2, 3, 4, 6, 0, 1,
	1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10
};

const int Resources::METAL_LAC[9] = { -3, 0, -2, -1, 1, 2, 4, 6, 8 };

const int Resources::ARMOR_STRENGTHS[14] = { 0, 2, 4, 5, 6, 7, 8, 10, 4, 2, 1, 1, 1, 1 };

const int Resources::MAKE_ITEM_ARR1[6] = { 0, 8, 15, 20, 25, 33 };

const int Resources::MAKE_ITEM_ARR2[6][7][2] = {
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 7 }, { 7, 7 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 1 }, { 1, 1 }, { 1, 2 }, { 2, 2 }, { 2, 3 }, { 3, 3 } }
};

const int Resources::MAKE_ITEM_ARR3[10][7][2] = {
	{ { 0, 0 }, { 1, 4 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 6, 10 }, { 10, 10 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 3 }, { 2, 5 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 8, 8 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 5 }, { 5, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 5 }, { 4, 6 }, { 6, 6 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 3 }, { 2, 4 }, { 3, 4 }, { 4, 5 }, { 5, 5 } },
	{ { 0, 0 }, { 1, 2 }, { 1, 4 }, { 3, 6 }, { 5, 8 }, { 7, 10 }, { 10, 10 } }
};

const int Resources::MAKE_ITEM_ARR4[2][7][2] = {
	{ { 0, 0 }, { 1, 4 }, { 3, 7 }, { 4, 8 }, { 5, 9 }, { 8, 9 }, { 9, 9 } },
	{ { 0, 0 }, { 1, 4 }, { 2, 6 }, { 4, 7 }, { 6, 10 }, { 9, 13 }, { 13, 13 } }
};


const int Resources::MAKE_ITEM_ARR5[8][2] = {
	{ 0, 0 }, { 1, 15 }, { 16, 30 }, { 31, 40 }, { 41, 50 },
	{ 51, 60 }, { 61, 73 }, { 61, 73 }
};

const int Resources::OUTDOOR_DRAWSTRUCT_INDEXES[44] = {
	37, 38, 39, 40, 41, 44, 42, 43, 47, 45, 46,
	48, 49, 52, 50, 51, 66, 67, 68, 69, 70, 71,
	72, 75, 73, 74, 87, 88, 89, 90, 91, 94, 92,
	93, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120
};

const int Resources::TOWN_MAXES[2][11] = {
	{ 23, 13, 32, 16, 26, 16, 16, 16, 16, 16, 16 },
	{ 26, 19, 48, 27, 26, 37, 16, 16, 16, 16, 16 }
};

const char *const Resources::TOWN_ACTION_MUSIC[2][7] = {
	{ "bank.m", "smith.m", "guild.m", "tavern.m",
	"temple.m", "grounds.m", "endgame.m" },
	{ "bank.m", "sf09.m", "guild.m", "tavern.m",
	"temple.m", "smith.m", "endgame.m" }
};

const char *const Resources::TOWN_ACTION_SHAPES[7] = {
	"bnkr", "blck", "gild", "tvrn", "tmpl", "trng", "eface08"
};

const int Resources::TOWN_ACTION_FILES[2][7] = {
	{ 3, 2, 4, 2, 4, 2, 1 }, { 5, 3, 7, 5, 4, 6, 1 }
};

const char *const Resources::BANK_TEXT = "\x0D\x02\x03""c\x0B""122\x09""013"
	"\x0C""37D\x0C""dep\x09""040\x0C""37W\x0C""dith\x09""067ESC"
	"\x01\x09""000\x0B""000Bank of Xeen\x0B""015\n"
	"Bank\x03l\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s\x03""c\n"
	"\n"
	"Party\x03l\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s";

const char *const Resources::BLACKSMITH_TEXT = "\x01\x0D\x03""c\x0B""000\x09""000"
	"Store Options for\x09""039\x0B""027%s\x03""l\x0B""046\n"
	"\x09""011\x0C""37B\x0C""drowse\n"
	"\x09""000\x0B""090Gold\x03r\x09""000%s"
	"\x02\x03""c\x0B""122\x09""040ESC\x01";

const char *const Resources::GUILD_NOT_MEMBER_TEXT =
	"\n\nYou have to be a member to shop here.";

const char *const Resources::GUILD_TEXT = "\x03""c\x0B""027\x09""039%s"
	"\x03l\x0B""046\n"
	"\x09""012\x0C""37B\x0C""duy Spells\n"
	"\x09""012\x0C""37S\x0C""dpell Info";

const char *const Resources::TAVERN_TEXT =
	"\x0D\x03""c\x0B""000\x09""000Tavern Options for\x09""039"
	"\x0B""027%s%s\x03l\x09""000"
	"\x0B""090Gold\x03r\x09""000%s\x02\x03""c\x0B""122"
	"\x09""021\x0C""37S\x0C""dign in\x09""060ESC\x01";

const char *const Resources::FOOD_AND_DRINK =
	"\x03l\x09""017\x0B""046\x0C""37D\x0C""drink\n"
	"\x09""017\x0C""37F\x0C""dood\n"
	"\x09""017\x0C""37T\x0C""dip\n"
	"\x09""017\x0C""37R\x0C""dumors";

const char *const Resources::GOOD_STUFF = "\n"
	"\n"
	"Good Stuff\n"
	"\n"
	"Hit a key!";

const char *const Resources::HAVE_A_DRINK = "\n\nHave a Drink\n\nHit a key!";

const char *const Resources::YOURE_DRUNK = "\n\nYou're Drunk\n\nHit a key!";

const int Resources::TAVERN_EXIT_LIST[2][6][5][2] = {
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

const char *const Resources::TEMPLE_TEXT =
	"\x0D\x03""c\x0B""000\x09""000Temple Options for"
	"\x09""039\x0B""027%s\x03l\x09""000\x0B""046"
	"\x0C""37H\x0C""deal\x03r\x09""000%lu\x03l\n"
	"\x0C""37D\x0C""donation\x03r\x09""000%lu\x03l\n"
	"\x0C""37U\x0C""dnCurse\x03r\x09""000%s"
	"\x03l\x09""000\x0B""090Gold\x03r\x09""000%s"
	"\x02\x03""c\x0B""122\x09""040ESC\x01";

const char *const Resources::EXPERIENCE_FOR_LEVEL =
	"%s needs %lu experience for level %u.";

const char *const Resources::LEARNED_ALL = "%s has learned all we can teach!";

const char *const Resources::ELIGIBLE_FOR_LEVEL = "%s is eligible for level %d.";

const char *const Resources::TRAINING_TEXT =
	"\x0D\x03""cTraining Options\n"
	"\n"
	"%s\x03l\x0B""090\x09""000Gold\x03r\x09"
	"000%s\x02\x03""c\x0B""122\x09""021"
	"\x0C""37T\x0C""drain\x09""060ESC\x01";

const char *const Resources::GOLD_GEMS =
	"\x03""c\x0B""000\x09""000%s\x03l\n"
	"\n"
	"Gold\x03r\x09""000%s\x03l\n"
	"Gems\x03r\x09""000%s\x02\x03""c\x0B""096\x09""013G"
	"\x0C""37o\x0C""dld\x09""040G\x0C\x03""7e"
	"\x0C""dms\x09""067ESC\x01";

const char *const Resources::GOLD_GEMS_2 =
	"\x09""000\x0B""000\x03""c%s\x03l\n"
	"\n"
	"\x04""077Gold\x03r\x09""000%s\x03l\n"
	"\x04""077Gems\x03r\x09""000%s\x03l\x09""000\x0B""051\x04""077\n"
	"\x04""077";

const char *const Resources::DEPOSIT_WITHDRAWL[2] = { "Deposit", "Withdrawl" };

const char *const Resources::NOT_ENOUGH_X_IN_THE_Y =
	"\x03""c\x0B""012Not enough %s in the %s!\x03l";

const char *const Resources::NO_X_IN_THE_Y = "\x03""c\x0B""012No %s in the %s!\x03l";

const char *const Resources::STAT_NAMES[16] = {
	"Might", "Intellect", "Personality", "Endurance", "Speed",
	"Accuracy", "Luck", "Age", "Level", "Armor Class", "Hit Points",
	"Spell Points", "Resistances", "Skills", "Awards", "Experience"
};

const char *const Resources::CONSUMABLE_NAMES[4] = { "Gold", "Gems", "Food", "Condition" };

const char *const Resources::WHERE_NAMES[2] = { "Party", "Bank" };

const char *const Resources::AMOUNT = "\x03""c\x09""000\x0B""051Amount\x03l\n";

const char *const Resources::FOOD_PACKS_FULL = "\v007Your food packs are already full!";

const char *const Resources::BUY_SPELLS =
	"\x03""c\x0B""027\x09""039%s\x03l\x0B""046\n"
	"\x09""012\x0C""37B\x0C""duy Spells\n"
	"\x09""012\x0C""37S\x0C""dpell Info";

const char *const Resources::GUILD_OPTIONS =
	"\x0D\x0C""00\x03""c\x0B""000\x09""000Guild Options for%s"
	"\x03l\x09""000\x0B""090Gold"
	"\x03r\x09""000%s\x02\x03""c\x0B""122\x09""040ESC\x01";

const int Resources::MISC_SPELL_INDEX[74] = {
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

const int Resources::SPELL_COSTS[77] = {
	8, 1, 5, -2, 5, -2, 20, 10, 12, 8, 3,
	- 3, 75, 40, 12, 6, 200, 10, 100, 30, -1, 30,
	15, 25, 10, -2, 1, 2, 7, 20, -2, -2, 100,
	15, 5, 100, 35, 75, 5, 20, 4, 5, 1, -2,
	6, 2, 75, 40, 60, 6, 4, 25, -2, -2, 60,
	- 1, 50, 15, 125, 2, -1, 3, -1, 200, 35, 150,
	15, 5, 4, 10, 8, 30, 4, 5, 7, 5, 0
};

const int Resources::DARK_SPELL_RANGES[12][2] = {
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 },
	{ 0, 17 }, { 14, 34 }, { 26, 37 }, { 29, 39 },
	{ 0, 20 }, { 16, 35 }, { 27, 37 }, { 29, 39 }
};

const int Resources::CLOUDS_SPELL_OFFSETS[5][20] = {
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

const uint Resources::DARK_SPELL_OFFSETS[3][39] = {
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

const int Resources::SPELL_GEM_COST[77] = {
	0, 0, 2, 1, 2, 4, 5, 0, 0, 0, 0, 10, 10, 10, 0, 0, 20, 4, 10, 20, 1, 10,
	5, 5, 4, 2, 0, 0, 0, 10, 3, 1, 20, 4, 0, 20, 10, 10, 1, 10, 0, 0, 0, 2,
	2, 0, 10, 10, 10, 0, 0, 10, 3, 2, 10, 1, 10, 10, 20, 0, 0, 1, 1, 20, 5, 20,
	5, 0, 0, 0, 0, 5, 1, 2, 0, 2, 0
};

const char *const Resources::NOT_A_SPELL_CASTER = "Not a spell caster...";

const char *const Resources::SPELLS_FOR = "\xD\xC""d%s\x2\x3""c\x9""000\xB""002Spells for %s";

const char *const Resources::SPELL_LINES_0_TO_9 =
	"\x2\x3l\xB""015\x9""0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";

const char *const Resources::SPELLS_DIALOG_SPELLS = "\x3l\xB""015"
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

const char *const Resources::SPELL_PTS = "Spell Pts";

const char *const Resources::GOLD = "Gold";

const char *const Resources::SPELLS_PRESS_A_KEY =
	"\x3""c\xC""09%s\xC""d\x3l\n"
	"\n"
	"%s\x3""c\x9""000\xB""100Press a Key!";

const char *const Resources::SPELLS_PURCHASE =
	"\x3l\xB""000\x9""000\xC""d%s  Do you wish to purchase "
	"\xC""09%s\xC""d for %u?";

const char *const Resources::MAP_TEXT =
	"\x3""c\xB""000\x9""000%s\x3l\xB""139"
	"\x9""000X = %d\x3r\x9""000Y = %d\x3""c\x9""000%s";

const char *const Resources::LIGHT_COUNT_TEXT = "\x3l\n\n\t024Light\x3r\t124%d";

const char *const Resources::FIRE_RESISTENCE_TEXT = "%c%sFire%s%u";

const char *const Resources::ELECRICITY_RESISTENCE_TEXT = "%c%sElectricity%s%u";

const char *const Resources::COLD_RESISTENCE_TEXT = "c%sCold%s%u";

const char *const Resources::POISON_RESISTENCE_TEXT = "%c%sPoison/Acid%s%u";

const char *const Resources::CLAIRVOYANCE_TEXT = "%c%sClairvoyance%s";

const char *const Resources::LEVITATE_TEXT = "%c%sLevitate%s";

const char *const Resources::WALK_ON_WATER_TEXT = "%c%sWalk on Water";

const char *const Resources::GAME_INFORMATION =
	"\xD\x3""c\x9""000\xB""001\xC""37%s of Xeen\xC""d\n"
	"Game Information\n"
	"\n"
	"Today is \xC""37%ssday\xC""d\n"
	"\n"
	"\x9""032Time\x9""072Day\x9""112Year\n"
	"\x9""032\xC""37%d:%02d%c\x9""072%u\x9""112%u\xC""d%s";

const char *const Resources::WORLD_GAME_TEXT = "World";
const char *const Resources::DARKSIDE_GAME_TEXT = "Darkside";
const char *const Resources::CLOUDS_GAME_TEXT = "Clouds";
const char *const Resources::SWORDS_GAME_TEXT = "Swords";

const char *const Resources::WEEK_DAY_STRINGS[10] = {
	"Ten", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"
};

const char *const Resources::CHARACTER_DETAILS =
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

const char *const Resources::PARTY_GOLD = "Party Gold";

const char *const Resources::PLUS_14 = "14+";

const char *const Resources::CHARACTER_TEMPLATE =
	"\x1\xC""00\xD\x3l\x9""029\xB""018Mgt\x9""080Acy\x9""131H.P.\x9""196Experience"
	"\x9""029\xB""041Int\x9""080Lck\x9""131S.P.\x9""029\xB""064Per\x9""080Age"
	"\x9""131Resis\x9""196Party Gems\x9""029\xB""087End\x9""080Lvl\x9""131Skills"
	"\x9""196Party Food\x9""029\xB""110Spd\x9""080AC\x9""131Awrds\x9""196Condition\x3""c"
	"\x9""290\xB""025\xC""37I\xC""dtem\x9""290\xB""057\xC""37Q"
	"\xC""duick\x9""290\xB""089\xC""37E\xC""dxch\x9""290\xB""121Exit\x3l%s";

const char *const Resources::EXCHANGING_IN_COMBAT = "\x3""c\xB""007\x9""000Exchanging in combat is not allowed!";

const char *const Resources::CURRENT_MAXIMUM_RATING_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%lu\x3l\x9""058/ %lu\n"
	"\x3""cRating: %s";

const char *const Resources::CURRENT_MAXIMUM_TEXT = "\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%u\x3l\x9""058/ %u";

const char *const Resources::RATING_TEXT[24] = {
	"Nonexistant", "Very Poor", "Poor", "Very Low", "Low", "Averarage", "Good",
	"Very Good", "High", "Very High", "Great", "Super", "Amazing", "Incredible",
	"Gigantic", "Fantastic", "Astoundig", "Astonishing", "Monumental", "Tremendous",
	"Collosal", "Awesome", "AweInspiring", "aUltimate"
};

const char *const Resources::AGE_TEXT = "\x2\x3""c%s\n"
	"Current / Natural\n"
	"\x3r\x9""057%u\x3l\x9""061/ %u\n"
	"\x3""cBorn: %u / %u\x1";

const char *const Resources::LEVEL_TEXT =
	"\x2\x3""c%s\n"
	"Current / Maximum\n"
	"\x3r\x9""054%u\x3l\x9""058/ %u\n"
	"\x3""c%u Attack%s/Round\x1";

const char *const Resources::RESISTENCES_TEXT =
	"\x2\x3""c%s\x3l\n"
	"\x9""020Fire\x9""100%u\n"
	"\x9""020Cold\x9""100%u\n"
	"\x9""020Electricity\x9""100%u\n"
	"\x9""020Poison\x9""100%u\n"
	"\x9""020Energy\x9""100%u\n"
	"\x9""020Magic\x9""100%u";

const char *const Resources::NONE = "\n\x9""020";

const char *const Resources::EXPERIENCE_TEXT = "\x2\x3""c%s\x3l\n"
	"\x9""010Current:\x9""070%lu\n"
	"\x9""010Next Level:\x9""070%s\x1";

const char *const Resources::ELIGIBLE = "\xC""12Eligible\xC""d";

const char *const Resources::IN_PARTY_IN_BANK =
	"\x2\x3""cParty %s\n"
	"%lu on hand\n"
	"%lu in bank\x1\x3l";

const char *const Resources::FOOD_TEXT =
	"\x2\x3""cParty %s\n"
	"%u on hand\n"
   "Enough for %u day%s\x3l";

const char *const Resources::EXCHANGE_WITH_WHOM = "\t010\v005Exchange with whom?";

const char *const Resources::QUICK_REF_LINE =
	"\xB%3d\x9""007%u)\x9""027%s\x9""110%c%c%c\x3r\x9""160\xC%02u%u\xC""d"
	"\x3l\x9""170\xC%02u%d\xC""d\x9""208\xC%02u%u\xC""d\x9""247\xC"
	"%02u%u\xC""d\x9""270\xC%02u%c%c%c%c\xC""d";

const char *const Resources::QUICK_REFERENCE =
	"\xD\x3""cQuick Reference Chart\xB""012\x3l"
	"\x9""007#\x9""027Name\x9""110Cls\x9""140Lvl\x9""176H.P."
	"\x9""212S.P.\x9""241A.C.\x9""270Cond"
	"%s%s%s%s%s%s%s%s"
	"\xB""110\x9""064\x3""cGold\x9""144Gems\x9""224Food\xB""119"
	"\x9""064\xC""15%lu\x9""144%lu\x9""224%u day%s\xC""d";

const uint Resources::BLACKSMITH_MAP_IDS[2][4] = { { 28, 30, 73, 49 }, { 29, 31, 37, 43 } };

const char *const Resources::ITEMS_DIALOG_TEXT1 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153%s\t187%s\t221%s"
	"\t255%s\t289Exit";
const char *const Resources::ITEMS_DIALOG_TEXT2 =
	"\r\x2\x3""c\v021\t017\f37W\fdeap\t051\f37A\fdrmor\t085A"
	"\f37c\fdces\t119\f37M\fdisc\t153\f37%s\t289Exit";
const char *const Resources::ITEMS_DIALOG_LINE1 = "\x3r\f%02u\f023%2d)\x3l\t028%s\n";
const char *const Resources::ITEMS_DIALOG_LINE2 = "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%lu\n";

const char *const Resources::BTN_BUY = "\f37B\fduy";
const char *const Resources::BTN_SELL = "\f37S\fdell";
const char *const Resources::BTN_IDENTIFY = "\f37I\fddentify";
const char *const Resources::BTN_FIX = "\f37F\fdix";
const char *const Resources::BTN_USE = "\f37U\fdse";
const char *const Resources::BTN_EQUIP = "\f37E\fdquip";
const char *const Resources::BTN_REMOVE = "\f37R\fdem";
const char *const Resources::BTN_DISCARD = "\f37D\fdisc";
const char *const Resources::BTN_QUEST = "\f37Q\fduest";
const char *const Resources::BTN_ENCHANT = "E\fdnchant";
const char *const Resources::BTN_RECHARGE = "R\fdechrg";
const char *const Resources::BTN_GOLD = "G\fdold";

const char *const Resources::ITEM_BROKEN = "\f32broken ";
const char *const Resources::ITEM_CURSED = "\f09cursed ";
const char *const Resources::BONUS_NAMES[7] = {
	"", "Dragon Slayer", "Undead Eater", "Golem Smasher",
	"Bug Zapper", "Monster Masher", "Beast Bopper"
};

const char *const Resources::WEAPON_NAMES[35] = {
	nullptr, "long sword ", "short sword ", "broad sword ", "scimitar ",
	"cutlass ", "sabre ", "club ", "hand axe ", "katana ", "nunchakas ",
	"wakazashi ", "dagger ", "mace ", "flail ", "cudgel ", "maul ", "spear ",
	"bardiche ", "glaive ", "halberd ", "pike ", "flamberge ", "trident ",
	"staff ", "hammer ", "naginata ", "battle axe ", "grand axe ", "great axe ",
	"short bow ", "long bow ", "crossbow ", "sling ", "Xeen Slayer Sword"
};

const char *const Resources::ARMOR_NAMES[14] = {
	nullptr, "robes ", "sale armor ", "ring mail ", "chain mail ",
	"splint mail ", "plate mail ", "plate armor ", "shield ",
	"helm ", "boots ", "cloak ", "cape ", "gauntlets "
};

const char *const Resources::ACCESSORY_NAMES[11] = {
	nullptr, "ring ", "belt ", "broach ", "medal ", "charm ", "cameo ",
	"scarab ", "pendant ", "necklace ", "amulet "
};

const char *const Resources::MISC_NAMES[22] = {
	nullptr, "rod ", "jewel ", "gem ", "box ", "orb ", "horn ", "coin ",
	"wand ", "whistle ", "potion ", "scroll ", "RogueVM",
	"bogusg", "bogus", "bogus", "bogus", "bogus",
	"bogus", "bogus", "bogus", "bogus"
};

const char *const *Resources::ITEM_NAMES[4] = {
	&Resources::WEAPON_NAMES[0], &Resources::ARMOR_NAMES[0],
	&Resources::ACCESSORY_NAMES[0], &Resources::MISC_NAMES[0]
};

const char *const Resources::ELEMENTAL_NAMES[6] = {
	"Fire", "Elec", "Cold", "Acid/Poison", "Energy", "Magic"
};

const char *const Resources::ATTRIBUTE_NAMES[10] = {
	"might", "Intellect", "Personality", "Speed", "accuracy", "Luck",
	"Hit Points", "Spell Points", "Armor Class", "Thievery"
};

const char *const Resources::EFFECTIVENESS_NAMES[7] = {
	nullptr, "Dragons", "Undead", "Golems", "Bugs", "Monsters", "Beasts"
};

const char *const Resources::QUEST_ITEM_NAMES[85] = {
	"Deed to New Castle",
	"Crystal Key to Witch Tower",
	"Skeleton Key to Darzog's Tower",
	"Enchanted Key to Tower of High Magic",
	"Jeweled Amulet of the Northern Sphinx",
	"Stone of a Thousand Terrors",
	"Golem Stone of Admittance",
	"Yak Stone of Opening",
	"Xeen's Scepter of Temporal Distortion",
	"Alacorn of Falista",
	"Elixir of Restoration",
	"Wand of Faery Magic",
	"Princess Roxanne's Tiara",
	"Holy Book of Elvenkind",
	"Scarab of Imaging",
	"Crystals of Piezoelectricity",
	"Scroll of Insight",
	"Phirna Root",
	"Orothin's Bone Whistle",
	"Barok's Magic Pendant",
	"Ligono's Missing Skull",
	"Last Flower of Summer",
	"Last Raindrop of Spring",
	"Last Snowflake of Winter",
	"Last Leaf of Autumn",
	"Ever Hot Lava Rock",
	"King's Mega Credit",
	"Excavation Permit",
	"Cupie Doll",
	"Might Doll",
	"Speed Doll",
	"Endurance Doll",
	"Accuracy Doll",
	"Luck Doll",
	"Widget",
	"Pass to Castleview",
	"Pass to Sandcaster",
	"Pass to Lakeside",
	"Pass to Necropolis",
	"Pass to Olympus",
	"Key to Great Western Tower",
	"Key to Great Southern Tower",
	"Key to Great Eastern Tower",
	"Key to Great Northern Tower",
	"Key to Ellinger's Tower",
	"Key to Dragon Tower",
	"Key to Darkstone Tower",
	"Key to Temple of Bark",
	"Key to Dungeon of Lost Souls",
	"Key to Ancient Pyramid",
	"Key to Dungeon of Death",
	"Amulet of the Southern Sphinx",
	"Dragon Pharoah's Orb",
	"Cube of Power",
	"Chime of Opening",
	"Gold ID Card",
	"Silver ID Card",
	"Vulture Repellant",
	"Bridle",
	"Enchanted Bridle",
	"Treasure Map (Goto E1 x1, y11)",
	"",
	"Fake Map",
	"Onyx Necklace",
	"Dragon Egg",
	"Tribble",
	"Golden Pegasus Statuette",
	"Golden Dragon Statuette",
	"Golden Griffin Statuette",
	"Chalice of Protection",
	"Jewel of Ages",
	"Songbird of Serenity",
	"Sandro's Heart",
	"Ector's Ring",
	"Vespar's Emerald Handle",
	"Queen Kalindra's Crown",
	"Caleb's Magnifying Glass",
	"Soul Box",
	"Soul Box with Corak inside",
	"Ruby Rock",
	"Emerald Rock",
	"Sapphire Rock",
	"Diamond Rock",
	"Monga Melon",
	"Energy Disk"
};

const int Resources::WEAPON_BASE_COSTS[35] = {
	0, 50, 15, 100, 80, 40, 60, 1, 10, 150, 30, 60, 8, 50,
	100, 15, 30, 15, 200, 80, 250, 150, 400, 100, 40, 120,
	300, 100, 200, 300, 25, 100, 50, 15, 0
};
const int Resources::ARMOR_BASE_COSTS[14] = {
	0, 20, 100, 200, 400, 600, 1000, 2000, 100, 60, 40, 250, 200, 100
};
const int Resources::ACCESSORY_BASE_COSTS[11] = {
	0, 100, 100, 250, 100, 50, 300, 200, 500, 1000, 2000
};
const int Resources::MISC_MATERIAL_COSTS[22] = {
	0, 50, 1000, 500, 10, 100, 20, 10, 50, 10, 10, 100,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const int Resources::MISC_BASE_COSTS[76] = {
	0, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
	100, 100, 100, 100, 200, 200, 200, 200, 200, 200, 200, 200,
	200, 200, 200, 200, 200, 200, 200, 300, 300, 300, 300, 300,
	300, 300, 300, 300, 300, 400, 400, 400, 400, 400, 400, 400,
	400, 400, 400, 500, 500, 500, 500, 500, 500, 500, 500, 500,
	500, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600, 600,
	600, 600, 600, 600
};
const int Resources::METAL_BASE_MULTIPLIERS[22] = {
	10, 25, 5, 75, 2, 5, 10, 20, 50, 2, 3, 5, 10, 20, 30, 40,
	50, 60, 70, 80, 90, 100
};
const int Resources::ITEM_SKILL_DIVISORS[4] = { 1, 2, 100, 10 };

const int Resources::RESTRICTION_OFFSETS[4] = { 0, 35, 49, 60 };

const int Resources::ITEM_RESTRICTIONS[86] = {
	0, 86, 86, 86, 86, 86, 86, 0, 6, 239, 239, 239, 2, 4, 4, 4, 4,
	6, 70, 70, 70, 70, 94, 70, 0, 4, 239, 86, 86, 86, 70, 70, 70, 70,
	0, 0, 0, 68, 100, 116, 125, 255, 255, 85, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const char *const Resources::NOT_PROFICIENT =
	"\t000\v007\x3""c%ss are not proficient with a %s!";

const char *const Resources::NO_ITEMS_AVAILABLE = "\x3""c\n"
	"\t000No items available.";

const char *const Resources::CATEGORY_NAMES[4] = { "Weapons", "Armor", "Accessories", "Miscellaneous" };

const char *const Resources::X_FOR_THE_Y =
	"\x1\fd\r%s\v000\t000%s for %s the %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";

const char *const Resources::X_FOR_Y =
	"\x1\xC""d\r\x3l\v000\t000%s for %s\x3r\t000%s\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\xC""d";

const char *const Resources::X_FOR_Y_GOLD =
	"\x1\fd\r\x3l\v000\t000%s for %s\t150Gold - %lu%s\x3l\v011"
	"\x2%s%s%s%s%s%s%s%s%s\x1\fd";

const char *const Resources::FMT_CHARGES = "\x3rr\t000Charges\x3l";

const char *const Resources::AVAILABLE_GOLD_COST =
	"\x1\fd\r\x3l\v000\t000Available %s\t150Gold - %lu\x3r\t000Cost"
	"\x3l\v011\x2%s%s%s%s%s%s%s%s%s\x1\xC""d";

const char *const Resources::CHARGES = "Charges";

const char *const Resources::COST = "Cost";

const char *const Resources::ITEM_ACTIONS[7] = {
	"Equip", "Remove", "Use", "Discard", "Enchant", "Recharge", "Gold"
};
const char *const Resources::WHICH_ITEM = "\t010\v005%s which item?";

const char *const Resources::WHATS_YOUR_HURRY = "\v007What's your hurry?\n"
	"Wait till you get out of here!";

const char *const Resources::USE_ITEM_IN_COMBAT =
	"\v007To use an item in Combat, invoke the Use command on your turn!";

const char *const Resources::NO_SPECIAL_ABILITIES = "\v005\x3""c%s\fdhas no special abilities!";

const char *const Resources::CANT_CAST_WHILE_ENGAGED = "\x3""c\v007Can't cast %s while engaged!";

const char *const Resources::EQUIPPED_ALL_YOU_CAN = "\x3""c\v007You have equipped all the %ss you can!";
const char *const Resources::REMOVE_X_TO_EQUIP_Y = "\x3""c\v007You must remove %sto equip %s\x8!";
const char *const Resources::RING = "ring";
const char *const Resources::MEDAL = "medal";

const char *const Resources::CANNOT_REMOVE_CURSED_ITEM = "\x3""You cannot remove a cursed item!";

const char *const Resources::CANNOT_DISCARD_CURSED_ITEM = "\3x""cYou cannot discard a cursed item!";

const char *const Resources::PERMANENTLY_DISCARD = "\v000\t000\x03lPermanently discard %s\fd?";

const char *const Resources::BACKPACK_IS_FULL = "\v005\x3""c\fd%s's backpack is full.";

const char *const Resources::CATEGORY_BACKPACK_IS_FULL[4] = {
	"\v010\t000\x3""c%s's weapons backpack is full.",
	"\v010\t000\x3""c%s's armor backpack is full.",
	"\v010\t000\x3""c%s's accessories backpack is full.",
	"\v010\t000\x3""c%s's miscellaneous backpack is full."
};

const char *const Resources::BUY_X_FOR_Y_GOLD = "\x3l\v000\t000\fdBuy %s\fd for %lu gold?";

const char *const Resources::SELL_X_FOR_Y_GOLD = "\x3l\v000\t000\fdSell %s\fd for %lu gold?";

const char *const Resources::NO_NEED_OF_THIS = "\v005\x3""c\fdWe have no need of this %s\f!";

const char *const Resources::NOT_RECHARGABLE = "\v012\x3""c\fdNot Rechargeable.  %s";

const char *const Resources::NOT_ENCHANTABLE = "\v012\t000\x3""cNot Enchantable.  %s";

const char *const Resources::SPELL_FAILED = "Spell Failed!";

const char *const Resources::ITEM_NOT_BROKEN =  "\fdThat item is not broken!";

const char *const Resources::FIX_IDENTIFY[2] = { "Fix", "Identify" };

const char *const Resources::FIX_IDENTIFY_GOLD = "\x3l\v000\t000%s %s\fd for %lu gold?";

const char *const Resources::IDENTIFY_ITEM_MSG = "\fd\v000\t000\x3""cIdentify Item\x3l\n"
	"\n"
	"\v012%s\fd\n"
	"\n"
	"%s";

const char *const Resources::ITEM_DETAILS =
	"Proficient Classes\t132:\t140%s\n"
	"to Hit Modifier\t132:\t140%s\n"
	"Physical Damage\t132:\t140%s\n"
	"Elemental Damage\t132:\t140%s\n"
	"Elemental Resistance\t132:\t140%s\n"
	"Armor Class Bonus\t132:\t140%s\n"
	"Attribute Bonus\t132:\t140%s\n"
	"Special Power\t132:\t140%s";

const char *const Resources::ALL = "All";
const char *const Resources::FIELD_NONE = "None";
const char *const Resources::DAMAGE_X_TO_Y = "%d to %d";
const char *const Resources::ELEMENTAL_XY_DAMAGE = "%+d %s Damage";
const char *const Resources::ATTR_XY_BONUS = "%+d %s";
const char *const Resources::EFFECTIVE_AGAINST = "x3 vs %s";

const char *const Resources::QUESTS_DIALOG_TEXT =
	"\r\x2\x3""c\v021\t017\f37I\fdtems\t085\f37Q\fduests\t153"
	"\f37A\fduto Notes	221\f37U\fdp\t255\f37D\fdown"
	"\t289Exit";
const char *const Resources::CLOUDS_OF_XEEN_LINE = "\b \b*-- \f04Clouds of Xeen\fd --";
const char *const Resources::DARKSIDE_OF_XEEN_LINE = "\b \b*-- \f04Darkside of Xeen\fd --";

const char *const Resources::NO_QUEST_ITEMS =
	"\r\x3""c\v000	000Quest Items\x3l\x2\n"
	"\n"
	"\x3""cNo Quest Items";
const char *const Resources::NO_CURRENT_QUESTS =
	"\x3""c\v000\t000\n"
	"\n"
	"No Current Quests";
const char *const Resources::NO_AUTO_NOTES = "\x3""cNo Auto Notes";

const char *const Resources::QUEST_ITEMS_DATA =
	"\r\x1\fd\x3""c\v000\t000Quest Items\x3l\x2\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s";
const char *const Resources::CURRENT_QUESTS_DATA =
	"\r\x1\fd\x3""c\t000\v000Current Quests\x3l\x2\n"
	"%s\n"
	"\n"
	"%s\n"
	"\n"
	"%s";
const char *const Resources::AUTO_NOTES_DATA =
	"\r\x1\fd\x3""c\t000\v000Auto Notes\x3l\x2\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l\n"
	"%s\x3l";

const char *const Resources::REST_COMPLETE =
	"\v000\t0008 hours pass.  Rest complete.\n"
	"%s\n"
	"%d food consumed.";
const char *const Resources::PARTY_IS_STARVING = "\f07The Party is Starving!\fd";
const char *const Resources::HIT_SPELL_POINTS_RESTORED = "Hit Pts and Spell Pts restored.";
const char *const Resources::TOO_DANGEROUS_TO_REST = "Too dangerous to rest here!";
const char *const Resources::SOME_CHARS_MAY_DIE = "Some Chars may die. Rest anyway?";

const char *const Resources::CANT_DISMISS_LAST_CHAR = "You cannot dismiss your last character!";

const char *const Resources::REMOVE_DELETE[2] = { "Remove", "Delete" };

const char *const Resources::REMOVE_OR_DELETE_WHICH = "\x3l\t010\v005%s which character?";

const char *const Resources::YOUR_PARTY_IS_FULL = "\v007Your party is full!";

const char *const Resources::HAS_SLAYER_SWORD =
	"\v000\t000This character has the Xeen Slayer Sword and cannot be deleted!";
const char *const Resources::SURE_TO_DELETE_CHAR =
	"Are you sure you want to delete %s the %s?";

const char *const Resources::CREATE_CHAR_DETAILS =
	"\f04\x3""c\x2\t144\v119\f37R\f04oll\t144\v149\f37C\f04reate"
	"\t144\v179\f37ESC\f04\x3l\x1\t195\v021\f37M\f04gt"
	"\t195\v045\f37I\f04nt\t195\v069\f37P\f04er\t195\v093\f37E\f04nd"
	"\t195\v116\f37S\f04pd\t195\v140\f37A\f04cy\t195\v164\f37L\f04ck%s";

const char *const Resources::NEW_CHAR_STATS =
	"\f04\x3l\t022\v148Race\t055: %s\n"
	"\t022Sex\t055: %s\n"
	"\t022Class\t055:\n"
	"\x3r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
	"\t215\v151%d\t215\v175%d\x3l\t242\v020\f%2dKnight\t242\v031\f%2d"
	"Paladin\t242\v042\f%2dArcher\t242\v053\f%2dCleric\t242\v064\f%2d"
	"Sorcerer\t242\v075\f%2dRobber\t242\v086\f%2dNinja\t242\v097\f%2d"
	"Barbarian\t242\v108\f%2dDruid\t242\v119\f%2dRanger\f04\x3""c"
	"\t265\v142Skills\x3l\t223\v155%s\t223\v170%s%s";

const char *const Resources::NAME_FOR_NEW_CHARACTER =
	"\x3""cEnter a Name for this Character";
const char *const Resources::SELECT_CLASS_BEFORE_SAVING =
	"\v006\x3""cSelect a Class before saving.\x3l";
const char *const Resources::EXCHANGE_ATTR_WITH = "Exchange %s with...";

const int Resources::NEW_CHAR_SKILLS[10] = { 1, 5, -1, -1, 4, 0, 0, -1, 6, 11 };
const int Resources::NEW_CHAR_SKILLS_LEN[10] = { 11, 8, 0, 0, 12, 8, 8, 0, 9, 11 };
const int Resources::NEW_CHAR_RACE_SKILLS[10] = { 14, -1, 17, 16, -1, 0, 0, 0, 0, 0 };

const int Resources::RACE_MAGIC_RESISTENCES[5] = { 7, 5, 20, 0, 0 };
const int Resources::RACE_FIRE_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int Resources::RACE_ELECTRIC_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int Resources::RACE_COLD_RESISTENCES[5] = { 7, 0, 2, 5, 10 };
const int Resources::RACE_ENERGY_RESISTENCES[5] = { 7, 5, 2, 5, 0 };
const int Resources::RACE_POISON_RESISTENCES[5] = { 7, 0, 2, 20, 0 };
const int Resources::NEW_CHARACTER_SPELLS[10][4] = {
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

const char *const Resources::COMBAT_DETAILS = "\r\f00\x3""c\v000\t000\x2""Combat%s%s%s\x1";

const char *Resources::NOT_ENOUGH_TO_CAST = "\x3""c\v010Not enough %s to Cast %s";
const char *Resources::SPELL_CAST_COMPONENTS[2] = { "Spell Points", "Gems" };

const char *const Resources::CAST_SPELL_DETAILS =
	"\r\x2\x3""c\v122\t013\f37C\fdast\t040\f37N\fdew"
	"\t067ESC\x1\t000\v000\x3""cCast Spell\n"
	"\n"
	"%s\x3l\n"
	"\n"
	"Spell Ready:\x3""c\n"
	"\n"
	"\f09%s\fd\x2\x3l\n"
	"\v082Cost\x3r\t000%u/%u\x3l\n"
	"Cur SP\x3r\t000%u\x1";

const char *const Resources::PARTY_FOUND =
	"\x3""cThe Party Found:\n"
	"\n"
	"\x3r\t000%lu Gold\n"
	"%lu Gems";

const char *const Resources::BACKPACKS_FULL_PRESS_KEY =
	"\v007\f12Warning!  BackPacks Full!\fd\n"
	"Press a Key";

const char *const Resources::HIT_A_KEY = "\x3l\v120\t000\x4""077\x3""c\f37Hit a key\xC""d";

const char *const Resources::GIVE_TREASURE_FORMATTING =
	"\x3l\v060\t000\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077\n"
	"\x4""077";

const char *const Resources::X_FOUND_Y = "\v060\t000\x3""c%s found: %s";

const char *const Resources::ON_WHO = "\x3""c\v009On Who?";

const char *const Resources::WHICH_ELEMENT1 =
	"\r\x3""c\x1Which Element?\x2\v034\t014\f15F\fdire\t044"
	"\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";

const char *const Resources::WHICH_ELEMENT2 =
	"\r\x3""cWhich Element?', 2, 0Bh, '034\t014\f15F\fdire\t044"
	"\f15E\fdlec\t074\f15C\fdold\t104\f15A\fdcid\x1";

const char *const Resources::DETECT_MONSTERS = "\x3""cDetect Monsters";

const char *const Resources::LLOYDS_BEACON =
	"\r\x3""c\v000\t000\x1Lloyd's Beacon\n"
	"\n"
	"Last Location\n"
	"\n"
	"%s\x3l\n"
	"x = %d\x3r\t000y = %d\x3""c\x2\v122\t021\f15S\fdet\t060\f15R\fdeturn\x1";

const char *const Resources::HOW_MANY_SQUARES = "\x3""cTeleport\nHow many squares %s (1-9)";

const char *const Resources::TOWN_PORTAL =
	"\x3""cTown Portal\x3l\n"
	"\n"
	"\t0101. %s\n"
	"\t0102. %s\n"
	"\t0103. %s\n"
	"\t0104. %s\n"
	"\t0105. %s\x3""c\n"
	"\n"
	"To which Town (1-5)\n"
	"\n";

const int Resources::TOWN_MAP_NUMBERS[2][5] = {
	{ 28, 29, 30, 31, 32 }, { 29, 31, 33, 35, 37 }
};

const char *const Resources::MONSTER_DETAILS =
	"\x3l\n"
	"%s\x3""c\t100%s\t140%u\t180%u\x3r\t000%s";

const char *const Resources::MONSTER_SPECIAL_ATTACKS[23] = {
	"None", "Magic", "Fire", "Elec", "Cold", "Poison", "Energy", "Disease",
	"Insane", "Asleep", "CurseItm", "InLove", "DrnSPts", "Curse", "Paralys",
	"Uncons", "Confuse", "BrkWpn", "Weak", "Erad", "Age+5", "Dead", "Stone"
};

const char *const Resources::IDENTIFY_MONSTERS =
	"Name\x3""c\t100HP\t140AC\t177#Atks\x3r\t000Special%s%s%s";

const char *const Resources::EVENT_SAMPLES[6] = {
	"ahh.voc", "whereto.voc", "gulp.voc", "null.voc", "scream.voc", "laff1.voc"
};

const char *const Resources::MOONS_NOT_ALIGNED =
"\x3""c\xB""012\t000The moons are not aligned. Passage to the %s is unavailable";

const char *const Resources::AWARDS_FOR =
	"\r\x1\fd\x3""c\v000\t000Awards for %s the %s\x3""l\x2\n"
	"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";

const char *const Resources::AWARDS_TEXT =
	"\r\x2\x3""c\xB""021\t221\xC""37U\xC""dp\t255\xC""37D\xC""down\t289Exit";

const char *const Resources::NO_AWARDS = "\x3""cNo Awards";

const char *const Resources::WARZONE_BATTLE_MASTER = "The Warzone\n\t125Battle Master";

const char *const Resources::WARZONE_MAXED = "What!  You again?  Go pick on someone your own size!";

const char *const Resources::WARZONE_LEVEL = "What level of monsters? (1-10)\n";

const char *const Resources::WARZONE_HOW_MANY = "How many monsters? (1-20)\n";

const char *const Resources::PICKS_THE_LOCK = "\x3""c\xB""010%s picks the lock!\nPress any key.";

const char *const Resources::UNABLE_TO_PICK_LOCK = "\x3""c\v010%s was unable to pick the lock!\nPress any key.";

const char *const Resources::CONTROL_PANEL_TEXT =
	"\x1\xC""00\x3""c\xB""000\t000Control Panel\x3r"
	"\xB""022\t045\xC""06E\xC""dfx:\t124\xC""06S\xC""dave:"
	"\xB""041\t045\xC""06M\xC""dusic:\t124S\xC""06a\xC""dve:"
	"\xB""060\t045\xC""06L\xC""doad:\t124\xC""06Q\xC""duit:"
	"\xB""080\t084Mr \xC""06W\xC""dizard:%s\t000";
const char *const Resources::CONTROL_PANEL_BUTTONS =
	"\x3""c\xB""022\t062load\t141%s"
	"\xB""041\t062save\t141%s"
	"\xB""060\t062exit"
	"\xB""079\t102Help\xC""d";
const char *const Resources::ON = "on";
const char *const Resources::OFF = "off";
const char *const Resources::CONFIRM_QUIT = "Are you sure you want to quit?";
const char *const Resources::MR_WIZARD =
	"Are you sure you want Mr.Wizard''s Help ?";
const char *const Resources::NO_LOADING_IN_COMBAT =
	"No Loading Allowed in Combat!";
const char *const Resources::NO_SAVING_IN_COMBAT =
	"No Saving Allowed in Combat!";

} // End of namespace Xeen
