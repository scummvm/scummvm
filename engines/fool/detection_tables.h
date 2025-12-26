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
	{ "fool", "The Fool's Errand" },
	{ "atc", "At The Carnival" },
	{ "3inthree", "3 in Three" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	// The Fool's Errand - Version 1.1 (1987-07-30)
	{
		"fool",
		"v1.1",
		AD_ENTRY1s("The Fool's Errand", "rt:00000000000000000000000000000000", 0),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// The Fool's Errand - Version 2.0 (1988-07-25)
	{
		"fool",
		"v2.0",
		AD_ENTRY1s("The Fool's Errand", "rt:00000000000000000000000000000000", 0),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// The Fool's Errand - Version 3.0 (1998-04-01)
	{
		"fool",
		"v3.0",
		AD_ENTRY1s("The Fool's Errand", "rt:00000000000000000000000000000000", 0),
		Common::EN_ANY,
		Common::kPlatformMacintosh,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},


	AD_TABLE_END_MARKER
};

} // End of namespace Fool
