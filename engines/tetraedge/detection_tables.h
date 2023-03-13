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
		GUIO1(GAMEOPTION_CORRECT_MOVIE_ASPECT)
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

	AD_TABLE_END_MARKER
};

} // namespace Tetraedge
