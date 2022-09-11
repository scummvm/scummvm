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

#define GAMEOPTION_WIDESCREEN_MOD				GUIO_GAMEOPTIONS1
#define GAMEOPTION_DYNAMIC_MIDI					GUIO_GAMEOPTIONS2
#define GAMEOPTION_LAUNCH_DEBUG					GUIO_GAMEOPTIONS3
#define GAMEOPTION_SOUND_EFFECT_SUBTITLES		GUIO_GAMEOPTIONS4
#define GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS		GUIO_GAMEOPTIONS5
#define GAMEOPTION_ENABLE_SHORT_TRANSITIONS		GUIO_GAMEOPTIONS6

namespace MTropolis {

static const MTropolisGameDescription gameDescriptions[] = {

	{ // Obsidian Macintosh, dumped
		{
			"obsidian",
			"V1.0, 1/13/97, CD",
			{
				{ "Obsidian Installer", 0, "1c272c23dc50b771970cabe8410c9349", 9250304 },
				//{ "Obsidian Data 2",	0, "1e590e3154c1af09efb951a07abc48b8", 563287808 },
				//{ "Obsidian Data 3",	0, "48e514a594b7a7ad190351d6d32d5d33", 617413632 },
				//{ "Obsidian Data 4",	0, "8dfa726c675aae3778951ddd18e4484c", 599297536 },
				//{ "Obsidian Data 5",	0, "6f085578b13b3db99543b969c9009b17", 583581056 },
				//{ "Obsidian Data 6",	0, "120ddcb1780be0f6380d708041733406", 558315648 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUIO3(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS, GAMEOPTION_SOUND_EFFECT_SUBTITLES)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
	},
	{ // Obsidian Macintosh, data forks only
		{
			"obsidian",
			"V1.0, 1/13/97, CD",
			{
				{ "Obsidian Installer", 0,	"c8859ba831a202a112eaffc5aee3ddf5", 9138050 },
				//{ "Obsidian Data 2",	0,	"a07c8ba79b9cb1de5496345dbe168527", 563284971 },
				//{ "Obsidian Data 3",	0,	"7cd809daa365b478ed96acbd6434966b", 617410816 },
				//{ "Obsidian Data 4",	0,	"ee67b2032f27133800f50c8b5cf08129", 599294667 },
				//{ "Obsidian Data 5",	0,	"13a221b93471b7d551316735cec21e7f", 583578222 },
				//{ "Obsidian Data 6",	0,	"5388ee329d1f5621333249f2f09cfb0c", 558312729 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUIO3(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS, GAMEOPTION_SOUND_EFFECT_SUBTITLES)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
	},

	{ // Obsidian Windows, installed
		{
			"obsidian",
			"V1.0, 1/13/97, installed, CD",
			{
				//{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				//{ "Obsidian.c95",		 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				//{ "RSGKit.r95",			 0, "071dc9098f9610fcec45c96342b1b69a", 625152 },
				//{ "MCURSORS.C95",		 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
				{ "Obsidian Data 1.MPL", 0, "9531162c32272c33837074be4646422a", 14755456 },
				//{ "Obsidian Data 2.MPX", 0, "c13c9be0ab0482a952532fa647a67a7a", 558175757 },
				//{ "Obsidian Data 3.MPX", 0, "35d8332221a7236b122b43233428f5dc", 614504412 },
				//{ "Obsidian Data 4.MPX", 0, "263fe824a1dd6f91390bce447c01e54c", 597911854 },
				//{ "Obsidian Data 5.MPX", 0, "894e4712a7bfb1b3c54086d43e6f3bb7", 576841795 },
				//{ "Obsidian Data 6.MPX", 0, "f491955b858e1a41d25efbb060424833", 554803689 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO3(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS, GAMEOPTION_SOUND_EFFECT_SUBTITLES)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_WIN_EN,
	},
	{
		// Obsidian, German Windows, installed
		// Released via the "ProSieben Mystery" series
		{
			"obsidian",
			"installed, CD",
			{
				//{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				//{ "Obsidian.c95",		 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				//{ "MCURSORS.C95",		 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
				{ "Obsidian Data 1.MPL", 0, "f96fc3a3a0a645009265c74c5fcb2c6a", 18972392 },
				//{ "Obsidian Data 2.MPX", 0, "b42a5a7bc36b2de2f9882e8a05435857", 559682181 },
				//{ "Obsidian Data 3.MPX", 0, "d4cb1a43d129019f8c2172a09cbedf2a", 614519546 },
				//{ "Obsidian Data 4.MPX", 0, "ae3095e5ac0a3a8984758ee76420e9b1", 591403514 },
				//{ "Obsidian Data 5.MPX", 0, "e8939423008a47c77735e16d7391a947", 578314080 },
				//{ "Obsidian Data 6.MPX", 0, "1295c1fe1a9113dbf2764b7024bf759d", 552452074 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE,
	},
	{
		// Obsidian, Italian Windows, installed
		{
			"obsidian",
			"installed, CD",
			{
				//{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
				//{ "Obsidian.c95",		 0, "fea68ff30ff319cdab30b79d2850a480", 145920 },
				//{ "MCURSORS.C95",		 0, "dcbe480913eebf233d0cdc33809bf048", 87040 },
				{ "Obsidian Data 1.MPL", 0, "7ea3e4ded35faebe254ed8f2d3693e82", 16306671 },
				//{ "Obsidian Data 2.MPX", 0, "7d395964b968632548b66066454b35f1", 559967533 },
				//{ "Obsidian Data 3.MPX", 0, "29091d607daf7f10e6ef910ebb539ad6", 614784639 },
				//{ "Obsidian Data 4.MPX", 0, "384d2c4fc502f213764eb1796be43df0", 591490294 },
				//{ "Obsidian Data 5.MPX", 0, "af2763c9deb312faf83d3621dd6cc60d", 578972797 },
				//{ "Obsidian Data 6.MPX", 0, "9fdbff8188021c132fcee15ed8a6e936", 546307095 },
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_WIN_IT,
	},

	{ // Obsidian Macintosh demo from standalone CD titled "Demo v1.0 January 1997"
		{
			"obsidian",
			"Demo",
			{
				//{ "Obsidian Demo",	   0, "abd1b5e7ac133f4c4b8c45ac67a4c44d", 920832 },
				//{ "Basic.rPP",		   0, "cb567ec1423a35903d8d5f458409e681", 210432 },
				//{ "Experimental.rPP",	   0, "26aa5fe1a6a152ade74e23a706673c50", 102016 },
				//{ "Extras.rPP",		   0, "c0e4c0401f2107ba3a3b7d282a76d99b", 377600 },
				//{ "mCursors.cPP",		   0, "a52e2aaf3b1a5c7d93a2949693bca694", 13312 },
				//{ "mNet.rPP",			   0, "ed5d998e7db6daae1f24bb124cc269aa", 134784 },
				//{ "Obsidian.cPP",		   0, "6da7babae9725a716f27f9f4ea382e92", 7552 },
				//{ "RSGKit.rPP",		   0, "c359e3c932b09280d1ccf21f8fb52bd7", 668160 },
				{ "Obs Demo Large w Sega", 0, "4672fe8ba459811dea0744cf90063a35", 98954240 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_MAC_EN,
	},

	{ // Obsidian PC demo [1996-10-03], found on:
	  // - PC Magazine Ultimate Utilities for Windows 95 (1996)
	  // - PC Magazine Super Shareware Annual Award Winners (1996)
	  // - PC Magazine Gaming Megapac (1997)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE",			0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				//{ "OBSIDIAN.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "TEXTWORK.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "EXPRMNTL.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "MCURSORS.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "643a989213b42cbac319d04676447624", 29096880 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_1,
	},

	{ // Obsidian PC demo (same as above, with 8.3 file names), found on PC Gamer Disc 2.12 (1997-01)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE",	0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				//{ "OBSIDIAN.R95",	0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "TEXTWORK.R95",	0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "EXPRMNTL.R95",	0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "MCURSORS.C95",	0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDI~1.MPL",	0, "643a989213b42cbac319d04676447624", 29096880 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_2,
	},

	{ // Obsidian PC demo [1996-10-11/22] found on:
	  // - Level 25 (1997-02)
	  // - Score 38 (1997-02)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN DEMO.EXE",		0, "1bac38af354fd79ae3285e6c737705b7", 751104 },
				//{ "OBSIDIAN1.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "OBSIDIAN2.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "OBSIDIAN3.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "OBSIDIAN4.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "77d04f62825c9f424baba46922ffb60f", 29552976 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_3,
	},

	{ // Obsidian PC cinematic demo found on:
	  // - Segasoft Demonstration Disk (Fall 1996)
	  // - CD Review #67 (1997) [1996-10-03]
		{
			"obsidian",
			"Demo",
			{
				//{ "OBSIDIAN.EXE", 0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				//{ "OBSIDIAN.R95", 0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "TEXTWORK.R95", 0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "EXPRMNTL.R95", 0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "MCURSORS.C95", 0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_4,
	},

	{ // Obsidian PC cinematic demo found on Multimedia Live (PC World) (v2.11, May 1997) [1996-10-03]
		{
			"obsidian",
			"Demo",
			{
				//{ "OBSIDI~1.EXE", 0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				//{ "OBSIDIAN.R95", 0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "TEXTWORK.R95", 0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "EXPRMNTL.R95", 0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "MCURSORS.C95", 0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDI~1.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO0()
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_5,
	},

	{ // Obsidian PC cinematic demo (identical to the above except for EXE name)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN.EXE",			0, "b6fb0e0df88c1524bcd0c5de9f5e882c", 750080 },
				//{ "OBSIDIAN.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "TEXTWORK.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "EXPRMNTL.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "MCURSORS.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_6,
	},

	{ // Obsidian PC cinematic demo (identical to above, but different player version and renamed extensions)
		{
			"obsidian",
			"Demo",
			{
				{ "OBSIDIAN DEMO.EXE",		0, "1bac38af354fd79ae3285e6c737705b7", 751104 },
				//{ "OBSIDIAN1.R95",			0, "5361ef93e36d722665594b724e0018fd", 183296 },
				//{ "OBSIDIAN2.R95",			0, "96346d39c4bb04f525edbf06ffe047e0", 148992 },
				//{ "OBSIDIAN3.R95",			0, "aa0431c2be37e33883747c61d3e980ff", 108544 },
				//{ "OBSIDIAN4.C95",			0, "47cf6abb95f3c43cdcbdf7ea1de3478d", 145920 },
				{ "OBSIDIAN DEMO DATA.MPL", 0, "4d557cd0a5f2311685d213053ebbd567", 116947911 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_TESTING,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,
	},

	{ // Muppet Treasure Island
		{
			"mti",
			"",
			{
				//{ "MTPLAY32.EXE",	0, "aad51b462d0961fb02d9c1422a41937f", 840192 },
				//{ "GROUP3.R95",		0, "3b01850e511727aa270aff1d6cb1fcf8", 89088 },
				//{ "MTIKIT.R95",		0, "f7183d9ff845a3a607f764920fc23b18", 101376 },
				{ "MTI1.MPL",		0, "cd0e1cd198fa2971371f42bb92b44972", 28500187 },
				//{ "MTI2.MPX",		0, "299929afb890398c385b13ee1446ece1", 431981661 },
				//{ "MTI3.MPX",		0, "90bd8dd40fcc65579f723eb75ad92799", 306575085 },
				//{ "MTI4.MPX",		0, "108628b01feb4d61ce40c9424de41b42", 201095285 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN,
	},

	{ // Los Muppets en la Isla del Tesoro (Mexican) [identical to Los Teleñecos en la Isla del Tesoro?]
		{
			"mti",
			"",
			{
				//{ "MTPLAY32.EXE",	0, "aad51b462d0961fb02d9c1422a41937f", 840192 },
				//{ "GROUP3.R95",		0, "3b01850e511727aa270aff1d6cb1fcf8", 89088 },
				//{ "MTIKIT.R95",		0, "f7183d9ff845a3a607f764920fc23b18", 101376 },
				{ "MTI1.MPL",		0, "1a951860380f7a0a0e1b9abe6be45ccd", 28605614 },
				//{ "MTI2.MPX",		0, "29b21afc024dc56dc99b7eb1f056fe65", 433252845 },
				//{ "MTI3.MPX",		0, "a880d87116b787b6e8a39f7d522c723c", 306257034 },
				//{ "MTI4.MPX",		0, "9b41ca763935f288b1b97c4025568e1d", 201507336 },
				AD_LISTEND
			},
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN,
	},

	{ // I Muppet nell'Isola del Tesoro
		{
			"mti",
			"",
			{
				//{ "MTPLAY32.EXE",	0, "aad51b462d0961fb02d9c1422a41937f", 840192 },
				//{ "GROUP3.R95",		0, "3b01850e511727aa270aff1d6cb1fcf8", 89088 },
				//{ "MTIKIT.R95",		0, "f7183d9ff845a3a607f764920fc23b18", 101376 },
				{ "MTI1.MPL",		0, "49883a0d8c76db739449ac8b0c6bc0a9", 28280783 },
				//{ "MTI2.MPX",		0, "5b69bbcceaf221e6b4200a5a76f8373e", 431268421 },
				//{ "MTI3.MPX",		0, "43a145a5a498640804a2116f8418cbf6", 307241744 },
				//{ "MTI4.MPX",		0, "30cd478975003466c7dadf5f6c6f5408", 201100471 },
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN,
	},

	{ // Muppet Treasure Island PC demo found on Score 38 (1997-02) [1996-07-17/19]
		{
			"mti",
			"Demo",
			{
				//{ "MTIWIN95.EXE", 0, "aad51b462d0961fb02d9c1422a41937f", 840192 },
				//{ "GROUP3.R95",	  0, "3b01850e511727aa270aff1d6cb1fcf8", 89088 },
				//{ "MTIKIT.R95",	  0, "f7183d9ff845a3a607f764920fc23b18", 101376 },
				{ "MUP_DATA.MPL", 0, "aea8ca15455991278213d09674a183ed", 51678610 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_DEMO_WIN,
	},

	{ AD_TABLE_END_MARKER, 0, 0, MTBOOT_INVALID }
};

} // End of namespace MTropolis

#endif
