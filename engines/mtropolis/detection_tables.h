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

#include "mtropolis/detection.h"

namespace MTropolis {

static const MTropolisGameDescription gameDescriptions[] = {
	{
		// Obsidian Macintosh
		{
			"obsidian",
			"V1.0, 1/13/97, installed, CD",
			{
				{ "Obsidian Installer", 0, "1c272c23dc50b771970cabe8410c9349", 9250304 },
				{ "Obsidian Data 2", 0, "1e590e3154c1af09efb951a07abc48b8", 563287808 },
				{ "Obsidian Data 3", 0, "48e514a594b7a7ad190351d6d32d5d33", 617413632 },
				{ "Obsidian Data 4", 0, "8dfa726c675aae3778951ddd18e4484c", 599297536 },
				{ "Obsidian Data 5", 0, "6f085578b13b3db99543b969c9009b17", 583581056 },
				{ "Obsidian Data 6", 0, "120ddcb1780be0f6380d708041733406", 558315648 },
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
				{ "Obsidian.exe", 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				{ "Obsidian.c95", 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				{ "RSGKit.r95", 0, "071dc9098f9610fcec45c96342b1b69a", 625152 },
				{ "MCURSORS.C95", 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
				//{ "Start Obsidian", 0, "51a4980089bb35da0f7f6381382d2889", -1 },
				{ "Obsidian Data 1.MPL", 0, "9531162c32272c33837074be4646422a", 14755456 },
				{ "Obsidian Data 2.MPX", 0, "c13c9be0ab0482a952532fa647a67a7a", 558175757 },
				{ "Obsidian Data 3.MPX", 0, "35d8332221a7236b122b43233428f5dc", 614504412 },
				{ "Obsidian Data 4.MPX", 0, "263fe824a1dd6f91390bce447c01e54c", 597911854 },
				{ "Obsidian Data 5.MPX", 0, "894e4712a7bfb1b3c54086d43e6f3bb7", 576841795 },
				{ "Obsidian Data 6.MPX", 0, "f491955b858e1a41d25efbb060424833", 554803689 },
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
	{
		// Obsidian, German Windows, installed
		// Released via the "ProSieben Mystery" series
		{
			"obsidian",
			"installed, CD",
			{
				{ "Obsidian.exe", 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				{ "Obsidian.c95", 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				{ "MCURSORS.C95", 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
				{ "Obsidian Data 1.MPL", 0, "f96fc3a3a0a645009265c74c5fcb2c6a", 18972392 },
				{ "Obsidian Data 2.MPX", 0, "b42a5a7bc36b2de2f9882e8a05435857", 559682181 },
				{ "Obsidian Data 3.MPX", 0, "d4cb1a43d129019f8c2172a09cbedf2a", 614519546 },
				{ "Obsidian Data 4.MPX", 0, "ae3095e5ac0a3a8984758ee76420e9b1", 591403514 },
				{ "Obsidian Data 5.MPX", 0, "e8939423008a47c77735e16d7391a947", 578314080 },
				{ "Obsidian Data 6.MPX", 0, "1295c1fe1a9113dbf2764b7024bf759d", 552452074 },
				AD_LISTEND
			},
			Common::DE_DEU,
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

} // End of namespace MTropolisEngine

#endif
