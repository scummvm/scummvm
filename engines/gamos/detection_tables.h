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

namespace Gamos {

const PlainGameDescriptor gamosGames[] = {
	{ "gamos", "Gamos" },
	{ "solgamer", "21 Solitaire" },
	{ "pilots", "Pilots 1" },
	{ "pilots2", "Pilots 2" },
	{ "wild", "WildSnakes"},
	{ 0, 0 }
};

const GamosGameDescription gameDescriptions[] = {
	{
		{
			"solgamer",
			0,
			AD_ENTRY1s("solgamer.exe", "6049dd1645071da1b60cdd395e6999ba", 24658521),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"solgamer.exe",
		0x80000018
	},
	{
		{
			"pilots",
			0,
			AD_ENTRY1s("pilots.exe", "152f751d3c1b325e91411dd75de54e95", 48357155),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"pilots.exe",
		0x80000016
	},
	{
		{
			"pilots2",
			0,
			AD_ENTRY1s("pilots2.exe", "a0353dfb46043d1b2d1ef8ab6c204b33", 582283983),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"pilots2.exe",
		0x80000018
	},
	{
		{
			"wild",
			0,
			AD_ENTRY1s("wildus.exe", "6049dd1645071da1b60cdd395e6999ba", 11475754),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		"wildus.exe",
		0x80000018
	},
	{
		AD_TABLE_END_MARKER,
		"",
		0
	}
};

} // End of namespace Gamos
