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

#define GAMEOPTION_WIDESCREEN_MOD			GUIO_GAMEOPTIONS1
#define GAMEOPTION_DYNAMIC_MIDI				GUIO_GAMEOPTIONS2
#define GAMEOPTION_LAUNCH_DEBUG				GUIO_GAMEOPTIONS3
//#define GAMEOPTION_LAUNCH_BREAK			GUIO_GAMEOPTIONS4	// Disabled due to not being functional
#define GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS	GUIO_GAMEOPTIONS5
#define GAMEOPTION_ENABLE_SHORT_TRANSITIONS GUIO_GAMEOPTIONS6

namespace MTropolis {

static const MTropolisGameDescription gameDescriptions[] = {

	{ // Obsidian Macintosh
		{
			"obsidian",
			"V1.0, 1/13/97, CD",
			{
				{ "Obsidian Installer", 0, "1c272c23dc50b771970cabe8410c9349", 9250304 },
				{ "Obsidian Data 2",	0, "1e590e3154c1af09efb951a07abc48b8", 563287808 },
				{ "Obsidian Data 3",	0, "48e514a594b7a7ad190351d6d32d5d33", 617413632 },
				{ "Obsidian Data 4",	0, "8dfa726c675aae3778951ddd18e4484c", 599297536 },
				{ "Obsidian Data 5",	0, "6f085578b13b3db99543b969c9009b17", 583581056 },
				{ "Obsidian Data 6",	0, "120ddcb1780be0f6380d708041733406", 558315648 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian Windows, installed
		{
			"obsidian",
			"V1.0, 1/13/97, installed, CD",
			{
				{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				{ "Obsidian.c95",		 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				{ "RSGKit.r95",			 0, "071dc9098f9610fcec45c96342b1b69a", 625152 },
				{ "MCURSORS.C95",		 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
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
			ADGF_UNSTABLE,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
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
				{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				{ "Obsidian.c95",		 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				{ "MCURSORS.C95",		 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
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
			ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian Macintosh demo from standalone CD titled "Demo v1.0 January 1997"
		{
			"obsidian",
			"Demo",
			{
				{ "Obsidian Demo",		   0, "abd1b5e7ac133f4c4b8c45ac67a4c44d", 920832 },
				{ "Basic.rPP",			   0, "cb567ec1423a35903d8d5f458409e681", 210432 },
				{ "Experimental.rPP",	   0, "26aa5fe1a6a152ade74e23a706673c50", 102016 },
				{ "Extras.rPP",			   0, "c0e4c0401f2107ba3a3b7d282a76d99b", 377600 },
				{ "mCursors.cPP",		   0, "a52e2aaf3b1a5c7d93a2949693bca694", 13312 },
				{ "mNet.rPP",			   0, "ed5d998e7db6daae1f24bb124cc269aa", 134784 },
				{ "Obsidian.cPP",		   0, "6da7babae9725a716f27f9f4ea382e92", 7552 },
				{ "RSGKit.rPP",			   0, "c359e3c932b09280d1ccf21f8fb52bd7", 668160 },
				{ "Obs Demo Large w Sega", 0, "4672fe8ba459811dea0744cf90063a35", 98954240 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo [1996-10-03], appears on:
	  // - PC Magazine Ultimate Utilities for Windows 95 (1996)
	  // - PC Magazine Super Shareware Annual Award Winners (1996)
	  // - PC Magazine Gaming Megapac (1997)
	  // - PC Gamer Disc 2.12 (1997-01) [with 8.3 names]
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE",			0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				{ "OBSIDIAN.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "TEXTWORK.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "EXPRMNTL.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "MCURSORS.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "643a989213b42cbac319d04676447624", 29096880 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo found on Level 25 (1997-02) [1996-10-11/22]
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN DEMO.EXE",		0, "1bac38af354fd79ae3285e6c737705b7", 751104 },
				{ "OBSIDIAN1.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "OBSIDIAN2.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "OBSIDIAN3.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "OBSIDIAN4.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "77d04f62825c9f424baba46922ffb60f", 29552976 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo found on:
	  // - Segasoft Demonstration Disk (Fall 1996)
	  // - CD Review #67 (1997) [1996-10-03]
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE", 0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				{ "OBSIDIAN.R95", 0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "TEXTWORK.R95", 0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "EXPRMNTL.R95", 0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "MCURSORS.C95", 0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo found on Multimedia Live (PC World) (v2.11, May 1997) [1996-10-03] [using 8.3 names]
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDI~1.EXE", 0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				{ "OBSIDIAN.R95", 0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "TEXTWORK.R95", 0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "EXPRMNTL.R95", 0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "MCURSORS.C95", 0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDI~1.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo (identical to the above except for EXE name)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE",			0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				{ "OBSIDIAN.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "TEXTWORK.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "EXPRMNTL.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "MCURSORS.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ // Obsidian PC demo
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN DEMO.EXE",		0, "1bac38af354fd79ae3285e6c737705b7", 751104 },
				{ "OBSIDIAN1.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				{ "OBSIDIAN2.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				{ "OBSIDIAN3.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				{ "OBSIDIAN4.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		0,
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

} // End of namespace MTropolis

#endif
