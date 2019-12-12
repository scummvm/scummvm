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

// md5's of the [efg]usecode.flx file of various versions of
// Ultima 8, Crusader: No Remorse and Crusader: No Regret

#include "ultima/ultima8/games/game_info.h"

namespace Ultima8 {
namespace Pentagram {

struct GameMD5Entry {
	GameInfo::GameType type;
	GameInfo::GameLanguage language;
	int version;
	const char *md5;
};


static GameMD5Entry md5table[] = {
	// U8, 2.10
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_ENGLISH, 210,
		"da13af19e2b7f4e6a5efbaffff371030"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_FRENCH, 210,
		"05f867efd6eebd2deac05d50464ac1e5"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_GERMAN, 210,
		"cf6815f9b65b7530b6c43d7f23a11afc"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_SPANISH, 210,
		"d075ac78cd75529142325ab0b80ed813"
	},

	// U8, 2.12
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_ENGLISH, 212,
		"34cd320c4b780e59ae7fab40ab4338f1"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_FRENCH, 212,
		"9675900bd1a9c9f72b331e84a6cdbc23"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_GERMAN, 212,
		"473991399cfb176b0df6f088d30301c5"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_JAPANESE, 212,
		"304a315f7a436d18397c4ec544585f7e"
	},

	// U8, 2.13
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_ENGLISH, 213,
		"09e6916fdd404ce910eb19b6acf96383"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_FRENCH, 213,
		"2dd7355009021ebc85803b7227fcea4c"
	},
	{
		GameInfo::GAME_U8, GameInfo::GAMELANG_GERMAN, 213,
		"7aeab4a5f84dd6264d2ff80c5bdcebd4"
	},


	// Remorse, 1.01
	{
		GameInfo::GAME_REMORSE, GameInfo::GAMELANG_ENGLISH, 101,
		"e3b09cbe08b18b45767739a37ee5c239"
	},

	// Remorse, 1.07
	{
		GameInfo::GAME_REMORSE, GameInfo::GAMELANG_GERMAN, 107,
		"a82ecab20a8f0f79a1fb2e803ac67b0d"
	},

	// Remorse, 1.10
	{
		GameInfo::GAME_REMORSE, GameInfo::GAMELANG_ENGLISH, 110,
		"d41c4bd1401aaf3a1a47522fdebfd7c7"
	},

	// Remorse, 1.21
	{
		GameInfo::GAME_REMORSE, GameInfo::GAMELANG_ENGLISH, 121,
		"a3c0950f44693eb5983227b4e834a3b6"
	},


	// Regret, 1.01
	{
		GameInfo::GAME_REGRET, GameInfo::GAMELANG_ENGLISH, 101,
		"ddd81cd425cd77632c3878ca97d46e75"
	},

	// Regret, 1.06
	{
		GameInfo::GAME_REGRET, GameInfo::GAMELANG_ENGLISH, 106,
		"ce7b4ed2081e8bfb34bf74920a1ec254"
	},
	{
		GameInfo::GAME_REGRET, GameInfo::GAMELANG_GERMAN, 106,
		"0579a250c486a022f7336c6d02dff93d"
	},

	{ GameInfo::GAME_UNKNOWN, GameInfo::GAMELANG_UNKNOWN, 0, 0 }
};

} // End of namespace Pentagram
} // End of namespace Ultima8

