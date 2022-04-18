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

#ifndef MTROPOLIS_DETECTION_TABLES_H
#define MTROPOLIS_DETECTION_TABLES_H

#include "engines/advancedDetector.h"

namespace MTropolis {

static const MTropolisGameDescription gameDescriptions[] = {
	{
		// Obsidian Macintosh
		{
			"obsidian",
			"V1.0, 1/13/97, installed, CD",
			{
				{ "Obsidian Installer", 0, "1c272c23dc50b771970cabe8410c9349", -1 },
				{ "Obsidian Data 2", 0, "1e590e3154c1af09efb951a07abc48b8", -1 },
				{ "Obsidian Data 3", 0, "48e514a594b7a7ad190351d6d32d5d33", -1 },
				{ "Obsidian Data 4", 0, "8dfa726c675aae3778951ddd18e4484c", -1 },
				{ "Obsidian Data 5", 0, "6f085578b13b3db99543b969c9009b17", -1 },
				{ "Obsidian Data 6", 0, "120ddcb1780be0f6380d708041733406", -1 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		0,
	},
	{
		// Obsidian Windows, installed
		{
			"obsidian",
			"V1.0, 1/13/97, installed, CD",
			{
				{ "Obsidian.c95", 0, "fea68ff30ff319cdab30b79d2850a480", -1 },
				{ "RSGKit.r95", 0, "071dc9098f9610fcec45c96342b1b69a", -1 },
				{ "MCURSORS.C95", 0, "dcbe480913eebf233d0cdc33809bf048", -1 },
				//{ "Start Obsidian", 0, "51a4980089bb35da0f7f6381382d2889", -1 },
				{ "Obsidian Data 1.MPL", 0, "9531162c32272c33837074be4646422a", -1 },
				{ "Obsidian Data 2.MPX", 0, "c13c9be0ab0482a952532fa647a67a7a", -1 },
				{ "Obsidian Data 3.MPX", 0, "35d8332221a7236b122b43233428f5dc", -1 },
				{ "Obsidian Data 4.MPX", 0, "263fe824a1dd6f91390bce447c01e54c", -1 },
				{ "Obsidian Data 5.MPX", 0, "894e4712a7bfb1b3c54086d43e6f3bb7", -1 },
				{ "Obsidian Data 6.MPX", 0, "f491955b858e1a41d25efbb060424833", -1 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

/**
 * The fallback game descriptor used by the mTropolis engine's fallbackDetector.
 * Contents of this struct are to be overwritten by the fallbackDetector.
 */
static MTropolisGameDescription g_fallbackDesc = {
	{
		"",
		"",
		AD_ENTRY1(0, 0), // This should always be AD_ENTRY1(0, 0) in the fallback descriptor
		Common::UNK_LANG,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},
	0,
	0,
	0,
};

} // End of namespace MTropolisEngine

#endif
