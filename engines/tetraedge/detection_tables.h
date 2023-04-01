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

namespace Tetraedge {

const PlainGameDescriptor GAME_NAMES[] = {
	{ "amerzone", "Amerzone" },
	{ "syberia", "Syberia" },
	{ "syberia2", "Syberia II" },
	{ 0, 0 }
};

const ADGameDescription GAME_DESCRIPTIONS[] = {
	// Amerzone GOG release
	{
		"amerzone",
		nullptr,
		AD_ENTRY1s("MacOS/Amerzone", "d:cde4144aeea5a99602ee903554585178", 6380272),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO0()
	},

	// GOG and Steam releases
	// Note: Full sum of GOG and Steam are different,
	// but size and first 5000 bytes are the same.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("MacOS/Syberia", "d:6951fb8f71fe06f34684564625f73cd8", 10640592),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_TESTING,
		GUIO2(GAMEOPTION_CORRECT_MOVIE_ASPECT, GAMEOPTION_RESTORE_SCENES)
	},

	// iOS "free" release v1.1.3.  Not supported as we can't properly support
	// the in-app purchase to enable the full game.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("Syberia", "d:be658efbcf4541f56b656f92a05d271a", 15821120),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSUPPORTED | ADGF_DEMO,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// iOS paid release v1.2.  Not yet tested.
	{
		"syberia",
		nullptr,
		AD_ENTRY1s("Syberia", "d:1425707556476013e859979562c5d753", 15794272),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSTABLE,
		GUIO2(GAMEOPTION_CORRECT_MOVIE_ASPECT, GAMEOPTION_RESTORE_SCENES)
	},

	// GOG release
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("MacOS/Syberia 2", "d:c447586a3cb3d46d6127b467e7fb9a86", 12021136),
		Common::UNK_LANG,
		Common::kPlatformMacintosh,
		ADGF_TESTING,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	// iOS release v1.0.1.  Not yet tested.
	{
		"syberia2",
		nullptr,
		AD_ENTRY1s("Syberia 2", "d:17d0ded9b87b5096207117bf0cfb5138", 15881248),
		Common::UNK_LANG,
		Common::kPlatformIOS,
		ADGF_UNSTABLE,
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
	},

	AD_TABLE_END_MARKER
};

} // namespace Tetraedge
