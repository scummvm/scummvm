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

#include "common/platform.h"

namespace Fool {

const PlainGameDescriptor foolGames[] = {
	{ "foolserrand", "The Fool's Errand" },
	//{ "atc", "At The Carnival" },
	//{ "3inthree", "3 in Three" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	// The Fool's Errand - Version 1.1 (1987-07-30)
	{
		"foolserrand",
		"v1.1",
		AD_ENTRY1s("xn--The Fool's Errand-306j", "rt:44749119459beb47f37a02bd629ed9e9", 175275),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// The Fool's Errand - Version 2.0 (1988-07-25)
	{
		"foolserrand",
		"v2.0",
		AD_ENTRY1s("The Fool's Errand", "rt:7319cc059fde4535a196a744761b293f", 178663),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// The Fool's Errand - Version 3.0 (1998-04-01)
	{
		"foolserrand",
		"v3.0",
		AD_ENTRY1s("The Fool's Errand", "rt:b36443d20490fa45c6a58fd2d7be5ffb", 186482),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},


	AD_TABLE_END_MARKER
};

} // End of namespace Fool
