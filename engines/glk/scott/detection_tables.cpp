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

#include "glk/scott/detection_tables.h"

namespace Glk {
namespace Scott {
	
const char *const ADVENTURELAND_DESC = "Adventureland";
const char *const PIRATE_ADVENTURE_DESC = "Pirate Adventure";
const char *const MISSION_IMPOSSIBLE_DESC = "Mission Impossible";
const char *const VOODOO_CASTLE_DESC = "Voodoo Castle";
const char *const THE_COUNT_DESC = "The Count";
const char *const STRANGE_ODYSSEY_DESC = "Strange Odyssey";
const char *const MYSTERY_FUN_HOUSE_DESC = "Mystery Fun House";
const char *const PYRAMID_OF_DOOM_DESC = "Pyramid Of Doom";
const char *const GHOST_TOWN_DESC = "Ghost Town";
const char *const SAVAGE_ISLAND1_DESC = "Savage Island, Part 1";
const char *const SAVAGE_ISLAND2_DESC = "Savage Island, Part 2";
const char *const THE_GOLDEN_VOYAGE_DESC = "The Golden Voyage";
const char *const ADVENTURE13_DESC = "Adventure 13";
const char *const ADVENTURE14_DESC = "Adventure 14";
const char *const BUCKAROO_BANZAI_DESC = "Buckaroo Banzai";
const char *const MARVEL_ADVENTURE_DESC = "Marvel Adventure #1";
const char *const MINI_SAMPLER_DESC = "Adventure International's Mini-Adventure Sampler";

const ScottGame SCOTT_GAMES[] = {
	// PC game versions
	{ "7c6f495d757a54e73d259efc718d8024", "adventureland",     15896, ADVENTURELAND_DESC },
	{ "ea535fa7684508410151b4561de1f323", "pirateadventure",   16325, PIRATE_ADVENTURE_DESC },
	{ "379c77a9a483886366b3b5c425e56410", "missionimpossible", 15275, MISSION_IMPOSSIBLE_DESC },
	{ "a530a6857d1092eaa177eee575c94c71", "voodoocastle",      15852, VOODOO_CASTLE_DESC },
	{ "5ebb4ade985670bb2eac54f8fa202214", "thecount",          17476, THE_COUNT_DESC },
	{ "c57bb6df04dc77a2b232bc5bcab6e417", "strangeodyssey",    17489, STRANGE_ODYSSEY_DESC },
	{ "ce2931ac3d5cbc270a5cb7be9e614f6e", "mysteryfunhouse",   17165, MYSTERY_FUN_HOUSE_DESC },
	{ "4e6127fad6b5d75eccd3f3b101f8c9c8", "pyramidofdoom",     17673, PYRAMID_OF_DOOM_DESC },
	{ "2c08327ab06d5490bd9e367ddaeca627", "ghosttown",         17831, GHOST_TOWN_DESC },
	{ "8feb77f11d32e9567ce2fc7d435eaf44", "savageisland1",     19533, SAVAGE_ISLAND1_DESC },
	{ "20c40a349f7a214ac515fb1d63c30a87", "savageisland2",     18367, SAVAGE_ISLAND2_DESC },
	{ "e2a8f956ab215012d1495550c4c11ee8", "goldenvoyage",      18513, THE_GOLDEN_VOYAGE_DESC },
	{ "f986d7e1ee074f65b6c1d00461c9b3c3", "adventure13",       19232, ADVENTURE13_DESC },
	{ "6d98f422cc986d959a3c74351785aea3", "adventure14",       19013, ADVENTURE14_DESC },
	{ "aadcc04e6b37eb9d30a58b5bc775842e", "marveladventure",   18876, MARVEL_ADVENTURE_DESC },
	{ "d569a769f304dc02b3062d97458ddd01", "scottsampler",      13854, MINI_SAMPLER_DESC },

	// PDA game versions
	{ "ae541fc1085da2f7d561b72ed20a6bc1", "adventureland",     18003, ADVENTURELAND_DESC },
	{ "cbd47ab4fcfe00231ffd71d52378d410", "pirateadventure",   18482, PIRATE_ADVENTURE_DESC },
	{ "9251ab2c64e63559d8a6e9e6246760a5", "missionimpossible", 17227, MISSION_IMPOSSIBLE_DESC },
	{ "be849c5747c7fc3b201984afb4403b8e", "voodoocastle",      18140, VOODOO_CASTLE_DESC },
	{ "85b75b6079b5ee572b5259b29a0e5d21", "thecount",          19999, THE_COUNT_DESC },
	{ "c423cae841ac1927b5b2e503607b21bc", "strangeodyssey",    20115, STRANGE_ODYSSEY_DESC },
	{ "326b98b991d401605074e64d474ce566", "mysteryfunhouse",   19700, MYSTERY_FUN_HOUSE_DESC },
	{ "8ef9010399f055da9adb15ce7745a11c", "pyramidofdoom",     20320, PYRAMID_OF_DOOM_DESC },
	{ "fcdcca8b2acf76ba2d0006cefa3630a1", "ghosttown",         20687, GHOST_TOWN_DESC },
	{ "c8aaa80f07c40fa8e4b17432644919dc", "savageisland1",     22669, SAVAGE_ISLAND1_DESC },
	{ "2add0f28d9b236c866890cdf8d86ee60", "savageisland2",     21169, SAVAGE_ISLAND2_DESC },
	{ "675126bd0477e8ed9230ad3db5afc45f", "goldenvoyage",      21401, THE_GOLDEN_VOYAGE_DESC },
	{ "0ef0def798d895ed766041fa99dd28a0", "adventure13",       22346, ADVENTURE13_DESC },
	{ "0bf1bcc649422798332a38c88588fdff", "adventure14",       22087, ADVENTURE14_DESC },
	{ "a0a5423967287dae9cbeb9abe8324479", "buckaroobanzai",    21038, BUCKAROO_BANZAI_DESC },
	{ nullptr, nullptr, 0, nullptr }
};

} // End of namespace Frotz
} // End of namespace Glk
