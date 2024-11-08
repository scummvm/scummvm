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
			ADGF_NO_FLAGS,
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
			ADGF_NO_FLAGS,
			GUIO3(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS, GAMEOPTION_SOUND_EFFECT_SUBTITLES)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
	},
	{ // Obsidian Japanese Macintosh, dumped
		{
			"obsidian",
			"V1.0, 1/13/97, CD",
			{
				{"Obsidian Data 2", 0, "d3b4746dd05adba87e15f83a1599c6fe", 570855424},
				//{"Obsidian Data 3", 0, "96bb5f6ab893aca9e8e3ce19d19974fc", 616954880},
				//{"Obsidian Data 4", 0, "54cf1745ea19e0cc33d76905d0b9fc41", 593788928},
				//{"Obsidian Data 5", 0, "adc938b08ba2d2f0a08c934e6105b0f9", 583581824},
				//{"Obsidian Data 6", 0, "09de40ab9001eda30291421d2736fa76", 551191680},
				//{"xn--u9j9ecg0a2fsa1io6k6jkdc2k", 0, "af62516a9a9bd16bc5c01e755a9f7329", 9186432},
				AD_LISTEND
			},
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_MAC_JP,
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
			ADGF_NO_FLAGS,
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
				{ "Obsidian.exe",		 0, "0b50a779136ae6c9cc8bcfa3148c1127", 762368 },
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
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE_INSTALLED,
	},
	{
		// Obsidian, German Windows, CD
		// Released via the "ProSieben Mystery" series
		{
			"obsidian",
			"CD",
			{
				{ "_SETUP.1",			 0, "79c3a087043eb540a6e796d89a5add47", 8586799 },
				{ "OBSIDIAN DATA 1.MPL", 0, "f96fc3a3a0a645009265c74c5fcb2c6a", 18972392 },
				//{ "OBSIDIAN DATA 2.MPX", 0, "b42a5a7bc36b2de2f9882e8a05435857", 559682181 },
				//{ "OBSIDIAN DATA 3.MPX", 0, "d4cb1a43d129019f8c2172a09cbedf2a", 614519546 },
				//{ "OBSIDIAN DATA 4.MPX", 0, "ae3095e5ac0a3a8984758ee76420e9b1", 591403514 },
				//{ "OBSIDIAN DATA 5.MPX", 0, "e8939423008a47c77735e16d7391a947", 578314080 },
				//{ "OBSIDIAN DATA 6.MPX", 0, "1295c1fe1a9113dbf2764b7024bf759d", 552452074 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_RETAIL_WIN_DE_DISC,
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
			ADGF_NO_FLAGS,
			GUIO2(GAMEOPTION_WIDESCREEN_MOD, GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS)
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
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
			ADGF_DEMO,
			GUIO1(GAMEOPTION_WIDESCREEN_MOD)
		},
		GID_OBSIDIAN,
		0,
		MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,
	},

	{ // Muppet Treasure Island English Macintosh Retail
		{
			"mti",
			"",
			{
				{"xn--MTI1-8b7a", 0, "57a7f8df27c736b4248e2806139c8432", 28512101},
				//{"MTI2", 0, "02d4188353a44f120c4263128258d364", 432112070},
				//{"MTI3", 0, "629e6399517982b95abf111cf9402756", 306671165},
				//{"MTI4", 0, "cc666572fde4e56de3d90e2e885b6ad8", 410213632},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_MAC,
	},

	{ // Muppet Treasure Island English Windows Retail
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
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN,
	},
	{ // Muppet Treasure Island English Windows Retail DVD (OEM pack-in)
		{
			"mti",
			"",
			{
				{"MTI1.MPL", 0, "caff9457a120dd08a9a089071f3f8645", 28316059},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DVD | MTGF_WANT_MPEG_VIDEO | MTGF_WANT_MPEG_AUDIO,
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
			ADGF_NO_FLAGS,
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
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN,
	},

	{ // Muppet Treasure Island (Russian)
		{
			"mti",
			"CD",
			{
				{ "DATA1.CAB",		0, "381cd741e3d47796d0ac9c4c867fa91f", 17639913 },
				//{ "DATA1.HDR",		0, "62d2d807878352e2d775d9a4261f0a10", 5104 },
				{ "MTI1.MPL",		0, "38685e63d09ac835db3c826bebd521c0", 28500187 },
				//{ "MTI2.MPX",		0, "299929afb890398c385b13ee1446ece1", 431981661 },
				//{ "MTI3.MPX",		0, "90bd8dd40fcc65579f723eb75ad92799", 306575085 },
				//{ "MTI4.MPX",		0, "108628b01feb4d61ce40c9424de41b42", 201095285 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN_RU_DISC,
	},

	{ // Muppet Treasure Island (Russian, installed)
		{
			"mti",
			"installed, CD",
			{
				{ "MTPLAY32.EXE",	0, "aad51b462d0961fb02d9c1422a41937f", 840192 },
				//{ "GROUP3.R95",		0, "3b01850e511727aa270aff1d6cb1fcf8", 89088 },
				//{ "MTIKIT.R95",		0, "f7183d9ff845a3a607f764920fc23b18", 101376 },
				{ "MTI1.MPL",		0, "38685e63d09ac835db3c826bebd521c0", 28500187 },
				//{ "MTI2.MPX",		0, "299929afb890398c385b13ee1446ece1", 431981661 },
				//{ "MTI3.MPX",		0, "90bd8dd40fcc65579f723eb75ad92799", 306575085 },
				//{ "MTI4.MPX",		0, "108628b01feb4d61ce40c9424de41b42", 201095285 },
				AD_LISTEND
			},
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_RETAIL_WIN_RU_INSTALLED,
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
			ADGF_DEMO,
			GUIO0()
		},
		GID_MTI,
		0,
		MTBOOT_MTI_DEMO_WIN,
	},

	{ // Uncle Albert's Magical Album (German, Windows)
	  // Original title: Onkel Alberts geheimnisvolles Notizbuch
	  // Published by Tivola, 1998
		{
			"albert1",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Albert.exe",   0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "album411.MPL", 0, "08a742f5087d25e8ee45c2fcd57ad9a6", 17979781 },
				// { "album412.MPX", 0, "79fabc94dafd0e0f3ab93c138c4c8c82", 298762355 },
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT1,
		0,
		MTBOOT_ALBERT1_WIN_DE,
	},

	{ // Uncle Albert's Magical Album (English, Windows)
	  // Published by VTech, 1999
		{
			"albert1",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"ALBUM.EXE",    0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"album411.MPL", 0, "ad34822a1d7a4fd2c69487566c383b2e", 17658069},
				// {"album412.MPX", 0, "a2d01cdbed904f875fde71734b25f8d7", 308697831},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT1,
		0,
		MTBOOT_ALBERT1_WIN_EN,
	},

	{ // Uncle Albert's Magical Album (French, Windows)
	  // Original title: L'Album secret de l'oncle Ernest
	  // Published by Emme, 1998
		{
			"albert1",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"album421.MPL", 0, "2f3cc30f13ecc99e8e4818353cef1d34", 18879823},
				// {"album422.MPX", 0, "132db6e709ff40b49e5c702c6d14bd95", 293417240},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
			 AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT1,
		0,
		MTBOOT_ALBERT1_WIN_FR,
	},

	{ // Uncle Albert's Magical Album (Dutch, Windows)
	  // Original title: Het Magische Boek Van Oom Ernest
	  // Published by Emme, 1999
		{
			"albert1",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"Boek.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"boek1.MPL", 0, "4d10ee9943bdf9d8b751c7527e1c3a13", 18053917},
				// {"boek2.MPX", 0, "53f3df424a09b957da89dbf84828d92b", 297560609},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
			 	AD_LISTEND
			},
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT1,
		0,
		MTBOOT_ALBERT1_WIN_NL,
	},

	{ // Uncle Albert's Fabulous Voyage (German, Windows)
	  // Original title: Alberts abenteuerliche Reise
	  // Published by Tivola, 2000
		{
			"albert2",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "reise.exe",    0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "voyage1.MPL",  0, "fe202ad897cb6cf5303f2f046e8123d5", 26122297 },
				// { "voyage2.MPX",  0, "6554893511e08bf2e0d5770b7854589f", 261888657 },
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT2,
		0,
		MTBOOT_ALBERT2_WIN_DE,
	},

	{ // Uncle Albert's Fabulous Voyage (English, Windows)
	  // Published by Emme, 2000
		{
			"albert2",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"voyage.exe",  0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"voyage1.MPL", 0, "1859b9507c118f52402ecb248fa4ae9c", 25972983},
				// {"voyage2.MPX", 0, "639428214b71e057c825f41402d3694e", 254333940},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT2,
		0,
		MTBOOT_ALBERT2_WIN_EN,
	},

	{ // Uncle Albert's Fabulous Voyage (French, Windows)
	  // Original title: Le Fabuleux Voyage de l'oncle Ernest
	  // Published by Emme, 1999
		{
			"albert2",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"voyage.exe", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"voyage1.MPL", 0, "174683d29a1c8511830262940223cc8a", 27889879},
				// {"voyage2.MPX", 0, "e7204f9afcd19ad883ae33e63f199525", 240409364},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT2,
		0,
		MTBOOT_ALBERT2_WIN_FR,
	},

	{ // Uncle Albert's Fabulous Voyage (Dutch, Windows)
	  // Original title: De Fabelachtige Reis Van Oom Ernest
	  // Published by Emme, 1999
		{
			"albert2",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"reis.exe", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"voyage1.MPL", 0, "1bda2dd6149cc398296031439dc3d25b", 25618601},
				// {"voyage2.MPX", 0, "a02b7be7dd5a2324bd691299c66e4477", 246786921},
				// { "BASIC.X95",    0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",   0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",  0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",   0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95", 0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT2,
		0,
		MTBOOT_ALBERT2_WIN_NL,
	},

	{ // Uncle Albert's Mysterious Island (German, Windows)
	  // Original title: Alberts mysteriöse Insel
	  // Published by Tivola, 2001
		{
			"albert3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "insel.exe",      0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "ile_myst1.MPL",  0, "44ac2e944a61303f4028408165bafeb4", 24080377 },
				// { "ile_myst2.MPX",  0, "ecb1d2c8b99eb5b2ac6be55893b97f50", 283748531 },
				// { "BASIC.X95",     0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",    0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",   0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",    0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95",  0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT3,
		0,
		MTBOOT_ALBERT3_WIN_DE,
	},

	{ // Uncle Albert's Mysterious Island (English, Windows)
	  // Published by Emme, 2000
		{
			"albert3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"Ile_myst.exe",  0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"ile_myst1.MPL", 0, "90a71952e2336bddd30d0bacd59aa510", 22167812},
				// {"ile_myst2.MPX", 0, "aa49de0c8337ec97045c6af57ead02a3", 277412342},
				// { "BASIC.X95",     0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",    0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",   0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",    0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95",  0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT3,
		0,
		MTBOOT_ALBERT3_WIN_EN,
	},

	{ // Uncle Albert's Mysterious Island (French, Windows)
	  // Original title: L'Île Mystérieuse de l'oncle Ernest
	  // Published by Emme, 2000
		{
			"albert3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"Ile_myst.exe", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"ile_myst1.MPL", 0, "426868b7ea8fa555fa9240594c5e9903", 22181982},
				// {"ile_myst2.MPX", 0, "c4343b14271ad3c73aed1114bab704ef", 264549665},
				// { "BASIC.X95",     0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",    0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",   0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",    0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95",  0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT3,
		0,
		MTBOOT_ALBERT3_WIN_FR,
	},

	{ // Uncle Albert's Mysterious Island (Dutch, Windows)
	  // Original title: Het Mysterieuze Eiland Van Oom Ernest
	  // Published by Emme, 2001
		{
			"albert3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"Eiland.exe", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"ile_myst1.MPL", 0, "d851ca7a48c7adc98445624e154dc0bb", 21866179},
				// {"ile_myst2.MPX", 0, "637470c04417dafb55dee53e954857d8", 277233382},
				// { "BASIC.X95",     0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",    0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",   0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",    0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95",  0, "c271f786f028c0076635bea49a2be890", 66560 },
			 	AD_LISTEND
			},
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT3,
		0,
		MTBOOT_ALBERT3_WIN_NL,
	},

	{ // Uncle Albert's Mysterious Island (Catalan, Windows)
	  // Original title: L'Illa Misteriosa de l'oncle Albert
	  // Published by Emme, 2001
		{
			"albert3",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"Ile_Myst.exe", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760},
				{"ile_myst1.MPL", 0, "9ca7bddd55bf755df6508dd4aca3fa69", 21280759},
				// {"ile_myst2.MPX", 0, "a0f49ee642ecc3b2bc18221f19855de1", 271726516},
				// { "BASIC.X95",     0, "a1b474e90cc285bfdcfa87355f890b9e", 242688 },
				// { "BITMAP.R95",    0, "2bf7305eca9df63dbc75f55093b84cc0", 81408 },
				// { "CURSORS.C95",   0, "c1e2eebe7183b5ea33de7872a8ea4d2b", 87040 },
				// { "EXTRAS.R95",    0, "c5830771609c774de9dbeaa5dff69b3e", 185344 },
				// { "ROTATORK.R95",  0, "c271f786f028c0076635bea49a2be890", 66560 },
				AD_LISTEND
			},
			Common::CA_ESP,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ALBERT3,
		0,
		MTBOOT_ALBERT3_WIN_CA,
	},

	{ // SPQR: The Empire's Darkest Hour Windows CD-ROM
		{
			"spqr",
			"",
			{
				{ "SPQR.MPL", 0, "da778ae00dd3e7596ba970a4dccd0a44", 3292503 },
				// { "S_6842.MPX", 0, "506c04751bee9482f76333b0cf3c0e52", 577095784 },
				{ "SPQR32.EXE", 0, "af2a7889779243c0d25392598e893137", 759296 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_SPQR,
		0,
		MTBOOT_SPQR_RETAIL_WIN,
	},

	{ // SPQR: The Empire's Darkest Hour Macintosh CD-ROM
		{
			"spqr",
			"",
			{
				{ "Install.vct", 0, "df8b7e964bf2ce4d32a38091836bdd75", 3539712 },
				{ "S_6772", 0, "8f8e503fd5b6d0f7651149ff8d92a7a2", 576188928 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_SPQR,
		0,
		MTBOOT_SPQR_RETAIL_MAC,
	},

	{ // Star Trek: The Game Show demo
		{
			"sttgs",
			"Demo",
			{
				// { "MTPLAY95.EXE", 0, "3ce9559e8c8047243fb7393a90d8b8f5", 756736 },
				{ "Trektriv.mpl", 0, "2dfab3665f9d7a24473f7cd17a0615ba", 43003276 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_STTGS,
		0,
		MTBOOT_STTGS_DEMO_WIN,
	},

	{ // Star Trek: The Game Show
		{
			"sttgs",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				// { "MTPLAY95.EXE", 0, "3ce9559e8c8047243fb7393a90d8b8f5", 756736 },
				{"Trektriv.mpl", 0, "1a3e920c9334f4ddb02a0fca0a55b8e2", 540573701},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_STTGS,
		0,
		MTBOOT_STTGS_RETAIL_WIN,
	},

	{ // Unit: Rebooted (Music Videos)
		{
			"unit",
			"",
			{
				// { "UNIT32.EXE", 0, "c23dccd2b7a525a9f7bb8505f7c7f2d4", 1085952 },
				{ "DATA.MFX", 0, "9a3a0c2f11173c7af3f16d42a2b7c1b7", 194625739 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_UNIT,
		0,
		MTBOOT_UNIT_REBOOTED_WIN
	},

	{ // Mind Gym (English, Windows)
	  // Published by Macmillan, 1996
		{
			"mindgym",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
			 	{ "MINDGYM1.MPL", 0, "a183294df65173d4a16e51005b72e92b", 21260398 },
			 	// { "MINDGYM2.MPX", 0, "c8fb5c2c8dbf175c206b9f1d0ef4582d", 623947336 },
			 	AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MINDGYM,
		0,
		MTBOOT_MINDGYM_WIN_EN,
	},

	{ // Mind Gym (German, Windows)
	  // Original Title: Mind Gym
	  // Published by Ravensburger, 1997
		{
			"mindgym",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "MINDGYM1.MPL", 0, "8fd1e5e8d269da5d442b77023a2854d0", 21632226 },
				// { "MINDGYM2.MPX", 0, "7db9dc6b76bcca4d0629c37fb8c352c3", 621175049 },
		  		AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MINDGYM,
		0,
		MTBOOT_MINDGYM_WIN_DE,
	},

	{ // Fun With Architecture (English, Windows)
	  // Published by The Voyager Company, 1997
		{
			"architecture",
			"",
			{
				{ "MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
			 	{ "FWA1041.MPL", 0, "cb108fec620eff4108f194b1a730ec16", 50695771 },
			 	AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_ARCHITECTURE,
		0,
		MTBOOT_ARCHITECTURE_EN,
	},

	{ // The Magic World of Beatrix Potter (English, Windows)
	  // Published by Europress Software, 1997
		{
			"beatrix",
			"",
			{
				{ "POTTER95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
			 	{ "DATA.MPL", 0, "1dc4d2bf656ebb5a89e5f4a750a8cec0", 259660228 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO0()
		},
		GID_BEATRIX,
		0,
		MTBOOT_BEATRIX_EN,
	},

	{ // The Magic World of Beatrix Potter - Demo (English, Windows)
	  // Published by Europress Software, 1997
		{
			"beatrix",
			"Demo",
			{
				{ "PLAY.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "PLAY.MPL", 0, "93b4c5329a4752e26e3ba5a737e68430", 38415262 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DEMO,
			GUIO0()
		},
		GID_BEATRIX,
		0,
		MTBOOT_BEATRIX_DEMO_EN,
	},

	{ // Whitetail Impact (English, Windows)
	  // AKA: Whitetail Fever
	  // Published by Valusoft, 1998
		{
			"wtimpact",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "WTIMPACT.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
			 	{ "DATA.MPL", 0, "b86b35aa87a487e8af996d17cfe5f1b1", 285839431 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_WTIMPACT,
		0,
		MTBOOT_WT_IMPACT_EN,
	},

	{ // The Day The World Broke (English, Windows)
	  // Published by Houghton Mifflin Interactive, 1997
		{
			"worldbroke",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "TDTWB.EXE", 0, "ead983d2002f2b7b18da7f2f173fd425", 757760 },
			 	{ "TDTWB1.MPL", 0, "18d48974dbaff13829addbd47a5c374b", 5883239 },
			 	// { "TDTWB2.MPX", 0, "05c607388c8abcd70b1209088729311f", 663745644 },
			 	AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_WORLDBROKE,
		0,
		MTBOOT_WORLDBROKE_EN,
	},

	{
		// The Totally Techie World of Young Dilbert: Hi-Tech Hijinks (English, Windows)
		// Published by KnowWonder, 1997
		{
			"dilbert",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Hijinks.exe", 0, "418335f719bebcab41af9f23dd3ae27c", 30104 },
				// { "ALIENS2.MPX", 0, "9cc257dd254860bd20fd4204e7a2fb87", 46348146 },
				// { "Aliens1.mpl", 0, "330dae4abc82efa8c7bf48d5a5e4dcf3", 307527 },
				// { "BOB2.MPX", 0, "58357fb97c4424fedacc2775b39ad9a6", 63452005 },
				// { "Bob1.mpl", 0, "4b6dd1a3ee888fdf0bc587ce89bd713b", 216795 },
				// { "HIJINKS2.MPX", 0, "551d67b730f4fcd674e4166431038e8e", 52615395 },
				{ "Hijinks1.mpl", 0, "9552749a9d6031c10125cea155dfee57", 36070890 },
				// { "STROID2.MPX", 0, "f9b34f867a3d039b7e16e394e3fcb4d7", 44764602 },
				// { "Stroid1.mpl", 0, "c2763a45614c0fd566002417a976f5a5", 176178 },
				// { "VIRUS2.MPX", 0, "7047796bbd385ca22d205e24abcf9f99", 41056147 },
				// { "Virus1.mpl", 0, "86dbc04255eb1e7c46809d8590536df1", 195399 },
			 AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_DILBERT,
		0,
		MTBOOT_DILBERT_WIN_EN,
	},

	{ // Free Willy Activity Center (English, Windows)
	  // Published by Sound Source Interactive, 1997
		{
			"freewilly",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY95.EXE", 0, "3ce9559e8c8047243fb7393a90d8b8f5", 756736 },
				{ "FWAC.MPL", 0, "0b847e5ac895fcdc669ea40f8930b0c6", 77218688 },
			 	AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_FREEWILLY,
		0,
		MTBOOT_FREEWILLY_WIN_EN,
	},

	{ // Hercules & Xena Learning Adventure: Quest for the Scrolls (English, Windows)
	  // Published by Sound Source Interactive, 1997
		{
			"hercules",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY95.EXE", 0, "3ce9559e8c8047243fb7393a90d8b8f5", 756736 },
		 		{ "HERCXENA.MPL", 0, "549f965bc589a86d149c24f2ebd15bdc", 169595018 },
			 	AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_HERCULES,
		0,
		MTBOOT_HERCULES_WIN_EN,
	},

	{ // I Can Be a Dinosaur Finder (English, Windows)
	  // Published by Macmillan, 1997
		{
			"idino",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "WBDFR1.C9A", 0, "e6ec553f1f5ff0a420a7aed8b0543123", 584374209 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_IDINO,
		0,
		MTBOOT_IDINO_RETAIL_EN,
	},

	{ // I Can Be an Animal Doctor (English, Windows)
	  // Published by Macmillan, 1998
		{
			"idoctor",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "VET.C9A", 0, "6409415e9879c9287d0f72f900cfb4c7", 537078248 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_IDOCTOR,
		0,
		MTBOOT_IDOCTOR_RETAIL_EN,
	},

	{ // Cloud 9 CD Sampler Volume 2 - Demo (English, Windows)
	  // Published by Cloud 9 Interactive, 1997
		{
			"c9sampler",
			"Demo",
			{
				{ "RUNSAMP.C9A", 0, "349d9318b181ac4f6e0326a4b8fe7238", 5954633 },
				//{ "VETS.C9A", 0, "836765ba5202c146c5228933309e90cd", 149747132 },
				//{ "WBDFS.C9A", 0, "d28310a331ae07801e4e2fdc30a323b6", 172110125 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			GUIO0()
		},
		GID_C9SAMPLER,
		0,
		MTBOOT_C9SAMPLER_WIN_EN,
	},

	{ // How to Draw the Marvel Way (English, Windows)
	  // Published by Marvel Studios, 1996
		{
			"drawmarvelway",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{"MDRAW.C9A", 0, "601539f3e9f9e169aa6b2fac5a4e82c7", 369901410},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_DRAWMARVELWAY,
		0,
		MTBOOT_DRAWMARVELWAY_WIN_EN,
	},

	{ // FairyTale: A True Story - Activity Center (English, Windows)
	  // Published by Knowledge Adventure, 1999
		{
			"ftts",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "FAIRIES.EXE", 0, "b37ecc3aa2b1f24ddfbf27841f15442e", 758272 },
			 	{ "FT_1.MPL", 0, "76282d2f5a8f0eea5c73289e065d66d2", 800203 },
			 	// { "FT_2.MPX", 0, "08208838a8c5c4c95beae41e5208328b", 149851712 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_FTTS,
		0,
		MTBOOT_FTTS_WIN_EN,
	},

	{ // Purple Moon Sampler (English, Windows)
	  // Published by Purple Moon, 1997
		{
			"purplemoon",
			"Demo",
			{
				{ "MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
			 	{ "0SAMPLER.MPL", 0, "27899cf914b6d1b3099a3e5bb9108122", 311323 },
			 	// { "MAINMENU.MPL", 0, "476e9c774d30eda319cabec8e5b9e19c", 1517692 },
			 	// { "MSDAY1.MPL", 0, "0d5d7d0fc0c224b08eb183e2d498d23f", 73801500 },
			 	// { "MSDAY2.MPL", 0, "dbac1163593b11c13d9a496db96143c6", 8633775 },
			 	// { "SPATH.MPL", 0, "82ae96e0eac36bfd34e2f3e5fd1dfa6e", 57841860 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			GUIO0()
		},
		GID_PURPLEMOON,
		0,
		MTBOOT_PURPLEMOON_WIN_EN,
	},

	{ // Chomp! The Video Game (English, Windows)
	  // Published by Nabisco, 1996
		{
			"chomp",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MPLAY_95.EXE", 0, "369e485a92c4aaf1d7037214fc641848", 721920 },
			 	{ "MINIGAME.MPL", 0, "ad79f779eb6f87da5eecee7718fba483", 7768522 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_CHOMP,
		0,
		MTBOOT_CHOMP_RETAIL_WIN_EN,
	},

	{ // Chomp! The Video Game Demo (English, Windows)
	  // Published by Nabisco, 1996
		{
			"chomp",
			"Demo",
			{
				{ "MTPLAY32.EXE", 0, "5bc345c80c615e7c8c633468ba9bd7b4", 681984 },
				{ "JUMBLE.MPL", 0, "6f1c9e3e83e0eb87506b500b71a6e73c", 6473576 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			GUIO0()
		},
		GID_CHOMP,
		0,
		MTBOOT_CHOMP_DEMO_WIN_EN,
	},

	{ // 24 Hours in Cyberspace (English, Windows)
		{
		 	"cyber24",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
		 	{
				{ "ABC32.EXE", 0, "b5efd541d37b23ab6a587e0929b05c2e", 1218048 },
				{ "DATA1.MPL", 0, "7fb0b50e9db45dd069a1ca96efdefa5a", 1227933 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_CYBER24,
		0,
		MTBOOT_CYBER24_WIN_EN,
	},

	{ // IVOCLAR (English, Windows)
	  // A presentation of some dental medicine technology
		{
			"ivoclar",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
		 		{ "RUN95NT.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "DATA.MPL", 0, "d963537b9996b969492e095dca4c5b2a", 1717658 },
				AD_LISTEND
			},
		 	Common::EN_ANY,
		 	Common::kPlatformWindows,
		 	ADGF_UNSUPPORTED,
		 	GUIO0()
		},
	 	GID_IVOCLAR,
	 	0,
	 	MTBOOT_IVOCLAR_WIN_EN,
	},

	{ // Real Wild Child! Australian Rock Music 1950s-90s (English, Windows)
		{
			"realwild",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
		 		{ "RWC.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
		  		{ "RWC_DATA.MPL", 0, "7c271bed9ef7ea012896860358590791", 90144089 },
				AD_LISTEND
			},
		 	Common::EN_ANY,
		 	Common::kPlatformWindows,
		 	ADGF_UNSUPPORTED,
		 	GUIO0()
		},
	 	GID_REALWILD,
	 	0,
	 	MTBOOT_REALWILD_WIN_EN,
	},

	{ // How to Build a Telemedicine Program (English, Windows)
		{
			"telemed",
			"",
			{
		 		{ "TELEMED.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "TELEMED.MPL", 0, "cd73ec482fb21508a34daeff6773623c", 44022984 },
				AD_LISTEND
			},
		 	Common::EN_ANY,
		 	Common::kPlatformWindows,
		 	ADGF_UNSTABLE,
		 	GUIO0()
		},
	 	GID_TELEMED,
	 	0,
	 	MTBOOT_TELEMED_WIN_EN,
	},

	{ // Rugrats: Totally Angelica Boredom Buster (English, Windows)
	  // Published by Mattel Interactive, 2000
		{
			"angelica",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "BedRm.mfx", 0, "ff59f93ec9d61f43be11fa05f339495b", 13919113 },
				// { "HighScore.mfx", 0, "35742a1cdc503ff73897f8617d328759", 2613743 },
				// { "Journal.mfx", 0, "a48bc254d2c58075ca3433b80111db55", 10815621 },
				// { "LivRm.mfx", 0, "5e3f918f54e4d1343df7e6144b505895", 23964970 },
				// { "Makeup.mfx", 0, "4c0b8a760d525e066ec977dad685d9bd", 18024760 },
				// { "MatchAc.mfx", 0, "d097776158f8df266a6e66a1a3d36008", 19040502 },
				// { "OnIce.mfx", 0, "99f062b610f2533abe2a56f9fcf4ce15", 1668905 },
				// { "PBedRm.mfx", 0, "54438f3b8a42819e1166a43fe6116530", 13934291 },
				// { "SignIn.mfx", 0, "a7bfc234e36ddcb38798973984215395", 5260294 },
				{ "Splash.mfx", 0, "a95eb9155418e5fd2d9e7e7b855c8242", 1770003 },
				// { "TV.mfx", 0, "471e95b81207b56338defd4faf53ebc5", 44146820 },
				// { "WRobe.mfx", 0, "0ec2865d77e935e0932854201b5d1e24", 110314929 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ANGELICA,
		0,
		MTBOOT_ANGELICA_WIN_EN,
	},

	{ // Babe and Friends: Animated Early Reader (English, Windows)
	  // Published by Sound Source Interactive, 1999
		{
			"babe",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "BabeER.exe", 0, "a77039aa17f3f81f6b06e34bd6cc62db", 1041920 },
				{ "BabeER.mfx", 0, "890b294288581b20abac226c4ab37748", 15113800 },
				// { "BabeER2.mxx", 0, "3cce691bbdd9b1247aceb75ed3d567cb", 245599825 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_BABE,
		0,
		MTBOOT_BABE_WIN_EN,
	},

	{ // Biologia Cellulare Evoluzione E Varietà Della Vita (Italian, Windows)
		{
			"biocellevo",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "BIO.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "BIO1.mfx", 0, "7ae8e3384da3860e4b6b3a2de40d231a", 542088601 },
				AD_LISTEND
			},
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_BIOCELLEVO,
		0,
		MTBOOT_BIOCELLEVO_WIN_IT,
	},

	{ // Easy-Bake Kitchen (English, Windows)
	  // Published by Hasbro, 1999
		{
			"easybake",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay32.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "BAKESESS.MFX", 0, "1aa7b078b614b21e91022dab97d167b9", 78386865 },
				// { "DECOSESS.MFX", 0, "4f92c1f3caab95818bbf4ede17ddd960", 38170867 },
				// { "DOUGH.MFX", 0, "bec653daa80d92585eac670d1315f1e9", 11102671 },
				// { "GAMEHUB.MFX", 0, "6f1c8d3951128bc446f1e8981b2806bf", 8544387 },
				// { "ICING.MFX", 0, "2f35e759f6a686c3fdb1ab5874f54c19", 12424423 },
				{ "INTRO.MFX", 0, "8a7e999c047583f8ae6157210aef256e", 36640375 },
				// { "JAR.MFX", 0, "0f46d4d19fda1513b08803cb9c14df76", 58458020 },
				// { "MUSIC.MFX", 0, "b53fd2981dfb008356aa1828c5e586c4", 24862538 },
				// { "RECIPSEL.MFX", 0, "fc0fead02ab3c00b3aa15282927850f4", 7397433 },
				// { "SKILL.MFX", 0, "b22b5bd3c9322300da1f68c6195c5d4f", 43974421 },
				// { "TPARTY.MFX", 0, "089cfcc56296b47b9d3088280aa4e30a", 30451868 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_EASYBAKE,
		0,
		MTBOOT_EASYBAKE_WIN_EN,
	},

	{ // The Forgotten: It Begins (English, Windows)
	  // Published by DreamCatcher Interactive, 1999
		{
			"forgotten",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Forgot.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "Forgotten.mfw", 0, "d54d5b0d7e6e2d000fa3284d6bb3b1b7", 102093801 },
				// { "Forgotten2.mxw", 0, "60576ac2f3b7d4b506d8934deb54ad00", 269910629 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_FORGOTTEN,
		0,
		MTBOOT_FORGOTTEN_WIN_EN,
	},

	{ // The Mystery at Greveholm 2: The Journey to Planutus (Swedish, Windows)
	  // Published by Young Genius Software, 1998
		{
			"greveholm2",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Planutus.exe", 0, "15c9a577478fdee6bfc7aa32e0edc10b", 1085952 },
				{ "pData.mfw", 0, "9f17d1caab9be5078639e73a2c326969", 10690754 },
				// { "pData2.mxw", 0, "b7888606944d8fadee9c807eaaedaf19", 50885620 },
				// { "pData3.mxw", 0, "515d9ceb1fb58121d7c5b7c296590efd", 127879332 },
				// { "pData4.mxw", 0, "32461427fbc3e3a5e71ebc0c8f95b46b", 4581550 },
				// { "pData5.mxw", 0, "d453c37e5624f1c57a11c330162e03ba", 8140530 },
				// { "pData6.mxw", 0, "c7ac100c8afb2f40503cd04c065aa1c2", 3120251 },
				// { "pData7.mxw", 0, "ebb27c171261c17fb52af43ffab9ce2a", 8626512 },
				// { "pData8.mxw", 0, "3ca49943418521ed8608382480994ac4", 3123415 },
				// { "pData9.mxw", 0, "6562e4a7cc83dd134e01e179a6478d4d", 17303805 },
				// { "pData10.mxw", 0, "fc0e391cc891b30a6d41969aaeeaf116", 9759220 },
				// { "pData11.mxw", 0, "2be469dd466c3ad29adbcf22a052eff4", 5742569 },
				// { "pData12.mxw", 0, "780394367788facc5251ff038e9fd94b", 3330452 },
				// { "pData13.mxw", 0, "71d23e427ad999d9d75580731562c862", 3954444 },
				// { "pData15.mxw", 0, "fcfed4619b6c757df20c90ba9b111b59", 14658903 },
				// { "pData16.mxw", 0, "e2a89640876608487769e722699646f1", 26378754 },
				// { "pData17.mxw", 0, "997d05a8ad34d4fd17dfecfad17f8981", 17060297 },
				// { "pData18.mxw", 0, "62cebd98a9bf2f17fa9a3352ab952b6d", 10892439 },
				// { "pData19.mxw", 0, "695f75f8bdc65aadfc95970f1a2e16e6", 4582666 },
				// { "pData20.mxw", 0, "02aa4a3bfbfba7c21aa7cd388334d060", 15462415 },
				AD_LISTEND
			},
			Common::SE_SWE,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_GREVEHOLM2,
		0,
		MTBOOT_GREVEHOLM2_WIN_SE,
	},

	{ // Itacante: La Cité des Robots (French, Windows)
	  // Published by Emme Interactive, 1999
		{
			"itacante",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "itacante.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "machina.mfx", 0, "d500d15fb68cbc96988bbd868df316d0", 37537889 },
				// { "machina2.mxx", 0, "74f775d98a17277897940e58b69c5e7f", 214246774 },
				AD_LISTEND
			},
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ITACANTE,
		0,
		MTBOOT_ITACANTE_WIN_FR,
	},

	{ // King of Dragon Pass (English, Windows)
	  // Published by A-Sharp, 1999
		{
			"kingofdragonpass",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay95.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "opal.mfx", 0, "cb1d4f044a0293458e3816185a50a2b6", 36377208 },
				// { "KODP5182.mxx", 0, "5f1318edd0a90837f7a48295c6debf81", 144497671 },
				// { "KODP5183.mxx", 0, "05cd0df24f866ff2b942d7e39d91f400", 310970688 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_KINGOFDRAGONPASS,
		0,
		MTBOOT_KINGOFDRAGONPASS_WIN_EN,
	},

	{ // The Times: Key Stage 1 English (English, Windows)
		{
			"ks1eng",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "POWER.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "PmdataT.mfx", 0, "9eda23aece0732e7b69b9b9234301164", 86965804 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_KS1ENG,
		0,
		MTBOOT_KS1ENG_WIN_EN,
	},

	{ // Maisy's Playhouse (English, Windows)
	  // Published by Simon & Schuster, 1999
		{
			"maisy",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Maisy.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "data.mfx", 0, "04f20a0a5e721ff0c4f84ee900c99530", 4588716 },
				// { "data2.mxx", 0, "de7b7bf70ec8f628922f7ceb2fc004f1", 294743286 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MAISY,
		0,
		MTBOOT_MAISY_WIN_EN,
	},

	{ // The Magic School Bus Explores the World of Animals (English, Windows)
	  // Published by Microsoft, 1999
		{
			"msb_animal",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952},
				// { "reports.mfw", 0, "bb588986e16f9e9ae12816c6d9f51268", 35712973},
				{ "Startup.mfw", 0, "f52d750ffe3072e488023dedcdfbfd0b", 2000203},
				// { "Arctic.mfx", 0, "a595b7b712ad777bde46d08b6f583ff4", 32374648},
				// { "BackBus.mfx", 0, "ca6d7948723f862e28c491ede89c895b", 12526142 },
				// { "Bones.mfx", 0, "2489291991bc9ce84823137d25979991", 9826379 },
				// { "Bus.mfx", 0, "56038d8eb1d6aab4ad5abd1e01c24326", 49004547 },
				// { "camo.mfx", 0, "58248080d2c63e3c6265765371440cda", 3926076 },
				// { "cert.mfx", 0, "1c6c752f52293951473d9eba5d8ac89e", 4234176 },
				// { "Class.mfx", 0, "7a8bf0f67ba72678df5f012379b218e7", 22494146 },
				// { "crtr.mfx", 0, "d0c0506fa234d54c80c3e3b3cbd9015b", 4969456 },
				// { "Desert.mfx", 0, "851247a3200f368a3a3cd8d359e074b8", 26331602 },
				// { "EcoPost.mfx", 0, "5f3521ed0ca386d91bfb74112a0e4b9c", 10845589 },
				// { "Expmnts.mfx", 0, "cbf8509512a7769f4836daab4edabcc2", 22450663 },
				// { "frank.mfx", 0, "d602a3ac418a0983457192530116848a", 6414082 },
				// { "Himlaya.mfx", 0, "7cc63328f93b8326d31d8272b7cf21bb", 20834694 },
				// { "Pacific.mfx", 0, "db572822c9cdb26f66e35adfe6490c45", 29213317 },
				// { "Penquin.mfx", 0, "6974867092197e9d62cfb67f04638810", 5584817 },
				// { "picPuz.mfx", 0, "279cc6a2de8442afe6af51fb94a77787", 9257620 },
				// { "Rain.mfx", 0, "a7e6dafddf1b0a6df52ec386a3038e5b", 27309278 },
				// { "roar.mfx", 0, "353077cd53891b121228b4c7021f3c96", 3758103 },
				// { "Savanna.mfx", 0, "21dc6a27b4d264dba8758255aaf1e515", 26035394 },
				// { "Scat.mfx", 0, "75105868539b68f814c41efb641c5c82", 19246689 },
				// { "shelx.mfx", 0, "af1a4cf3f196226339de5845e8177cba", 7223058 },
				// { "Snake.mfx", 0, "273ea44cf95946f5246a2c564bec6545", 6231640 },
				// { "SThing.mfx", 0, "5d719c15d43a10a2feb5f0edbdbf2b86", 5659555 },
				// { "Swamp.mfx", 0, "2c3eed7ffce09c584f5ca1bfc7f53a6f", 26466355 },
				// { "Tag.mfx", 0, "0a71dfd5d8a8c491a60f08435daef832", 10795860 },
				// { "Tile.mfx", 0, "8e23fa37e32640b778d3675e337adc19", 5915594 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_ANIMAL,
		0,
		MTBOOT_MSB_ANIMAL_WIN_EN,
	},

	{ // The Magic School Bus Explores Bugs (English, Windows)
	  // Published by Microsoft, 2000
		{
			"msb_bugs",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "Bus.mfw", 0, "cdbe91cfaa4634c5920413a3ee56b1dd", 19193104 },
				// { "AntMaze.mfx", 0, "0ef81926da5ca6c205651c7f744f49c0", 4183723 },
				// { "BackBus.mfx", 0, "96cf4976106fdf9a501a3b35b4bc10d4", 13208304 },
				// { "BugEat.mfx", 0, "c773d6779394ab12ce89c339c9f3afb4", 8425720 },
				// { "BuildBug.mfx", 0, "a37b009c051318e4e4442d7d28434fea", 3774580 },
				// { "Bus.mfx", 0, "31841ec5be05f7eb18e77e9ff5040c05", 67852118 },
				// { "cert.mfx", 0, "3e7496a5ead768bcfcc88983c3342e56", 4369317 },
				// { "EBT.mfx", 0, "43d916c710c05ebd2e56d8668a8697fa", 11946840 },
				// { "EcoPost.mfx", 0, "ec441b4b965026ef2dd9e748a7a3ffe8", 8354730 },
				// { "FireFly.mfx", 0, "210fef0df8888bda3b5b733f1d7993c4", 4547310 },
				// { "Flight.mfx", 0, "fd1d071032c9cdf83cc6f0933fd0865e", 4465039 },
				// { "Forest.mfx", 0, "b2067495ef209f547942af829a90c491", 24773402 },
				// { "GameShow.mfx", 0, "c73ec09cbf9def72e0bd2378292ad699", 23056010 },
				// { "GBE.mfx", 0, "2b77b64fd1a2f0302ced71eb15fd3951", 8302512 },
				// { "Jungle.mfx", 0, "d65aacf5c3e46027300c4f451da593d3", 30996636 },
				// { "KidMorph.mfx", 0, "ae6cf12fad8901be85fabca592fa834c", 4760950 },
				// { "License.mfx", 0, "078f0f2eda2e94b0a06b69be301931ba", 4249248 },
				// { "meadow.mfx", 0, "09c56fc553efbbdeb2bbeb46623b4393", 22720812 },
				// { "Pond.mfx", 0, "b2030dac8053f792b5166960bfb1fb8b", 24635938 },
				// { "RBE.mfx", 0, "5d8bdb9044de697ca78c7571c9618384", 5495682 },
				// { "RBT.mfx", 0, "083152f6af3d40e823d067b676543116", 5500961 },
				// { "RCM.mfx", 0, "479e2709dcbef49b7ed2ebf42f014177", 5782712 },
				// { "RFL.mfx", 0, "a5cf594438743a533704beb0d9259380", 7978091 },
				// { "RGB.mfx", 0, "e43cb6267fb859d0d0c2e300703040d9", 5519579 },
				// { "RMT.mfx", 0, "8d639c2d43c14aa0d80c068f686c9b8f", 6107984 },
				// { "RPL.mfx", 0, "ae617deabe3a654c513a85bab18d9fe6", 6331374 },
				// { "RWB.mfx", 0, "a3931939f91c64ab2cb38286c146d8ca", 6844343 },
				// { "school.mfx", 0, "d03459adf342651da0eee1fc9f4185e5", 23830537 },
				{ "SignIn.mfx", 0, "a33f52fb5cc96809b1e9eafac2f28045", 2246830 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_BUGS,
		0,
		MTBOOT_MSB_BUGS_WIN_EN,
	},

	{ // The Magic School Bus In Concert (English, Windows)
	  // Published by Microsoft, 2000
		{
			"msb_concert",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "Acoust.mfx", 0, "112ea6c2a4a8b3a49770443ad9859190", 17246635 },
				// { "backbus.mfx", 0, "586e2e0981325935711cfab0bea3a06a", 15183980 },
				// { "Concert.mfx", 0, "bf644854cf510ea1316083f772384196", 26730066 },
				// { "gallery.mfx", 0, "ca949f69f12c63ce09ccf6258f88cbc1", 8769879 },
				// { "GameShow.mfx", 0, "b29f513d5b64a5d33717d73cb5337137", 54962750 },
				{ "SignInC.mfx", 0, "c03d48eb2af46e2111e46b64fd90b4e1", 3846548 },
				// { "simonsnd.mfx", 0, "fdfb9334ab5aa7bc3e9e550c85304727", 8483542 },
				// { "sndmixer.mfx", 0, "01021dc6ffb6fa6f3907c37d80264ed4", 12449504 },
				// { "SndPuzz.mfx", 0, "a3b20a084aec9818fe08b3de58b6fd84", 52351464 },
				// { "sndwave.mfx", 0, "051b36665d23497b11961fac85da1408", 13706241 },
				// { "stgpass.mfx", 0, "e4df5e2802d2346e8a9291fbec8579a3", 5680467 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_CONCERT,
		0,
		MTBOOT_MSB_CONCERT_WIN_EN,
	},

	{ // The Magic School Bus Discovers Flight (English, Windows)
	  // Published by Microsoft, 2000
		{
			"msb_flight",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "balloon.mfx", 0, "bafca52d8e8bcbe6f9b5f7bfeb2be29b", 4827052 },
				// { "buildaplane.mfx", 0, "53047c70893c012e7c81e645820ef433", 8152708 },
				// { "Bus.mfx", 0, "f16da286a05a5ced1b800ee74e5f9727", 36580418 },
				// { "fgs.mfx", 0, "44d34ce1f59bd4b0c19dd6b10df73a1e", 33408756 },
				// { "FPass.mfx", 0, "15d69a865e05791f58751fca3c41c0c3", 3370535 },
				// { "FPuzzle.mfx", 0, "bd5a9edb1b7c99d455b012508903145e", 10775749 },
				// { "FWT.mfx", 0, "af94af6733b98fe719daed303c3e92fa", 7788077 },
				// { "poster.mfx", 0, "2a28d92eb4fa78c796c01e7c0c412724", 5317029 },
				{ "SignInF.mfx", 0, "232be4bc0540c1bd025f9abeeef14135", 2770575 },
				// { "smkjump.mfx", 0, "d6b7a569fd8afcbe9e290415b6a4684c", 4627390 },
				// { "Zone.mfx", 0, "2110b284cbc2fec5d11381ff06c6bea9", 42565436 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_FLIGHT,
		0,
		MTBOOT_MSB_FLIGHT_WIN_EN,
	},

	{ // The Magic School Bus Lands on Mars (English, Windows)
	  // Published by Microsoft, 2000
		{
			"msb_mars",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay32.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "Bus.mfx", 0, "b72a82877c8dfe8d0dd1f96f8f9259e2", 51951667 },
				// { "CrewPass.mfx", 0, "e230104d86d9e38cd5eb8bd21837e5b3", 3844705 },
				// { "EBR.mfx", 0, "1603a2de3ace14768e7885b2ab676c38", 53142318 },
				// { "GameShow.mfx", 0, "bb69c4f7cd72a5c4c6a3c14d292c92cb", 24045815 },
				// { "Lander.mfx", 0, "5c24f1a4148dec627dd08f453d4b361e", 9645963 },
				// { "MPuzzle.mfx", 0, "3b0b6423431daba4d74fdd27b50d0201", 16779619 },
				// { "Postcard.mfx", 0, "72cbed8ea0c8286f903475060af8ca5b", 5170443 },
				// { "Pressure.mfx", 0, "147022e8e5cbc9c80bdf12565d57b4e0", 11582143 },
				// { "RRace.mfx", 0, "4349471749723d02f0e5865f180cec2e", 11464016 },
				{ "SignInM.mfx", 0, "a5aafcc2676bd1f0297623b053e0f1b4", 2984038 },
				// { "Zone.mfx", 0, "4d9f4dc6252996103fc1300f2910395b", 23110033 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_MARS,
		0,
		MTBOOT_MSB_MARS_WIN_EN,
	},

	{ // The Magic School Bus Volcano Adventure (English, Windows)
	  // Published by Microsoft, 2001
		{
			"msb_volcano",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay32.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "Art.mfx", 0, "672cf2da2dd8233074746e1e657c2f62", 7297320 },
				// { "BOB.mfx", 0, "2a5666aeb650c93374bee1b4d13b84ed", 24617929 },
				// { "crewpass.mfx", 0, "409384a1e43a70bf480162573da07d67", 2794311 },
				// { "Ex1Erupt.mfx", 0, "e99be79f854965b34c16720278122f4e", 49765988 },
				// { "ex2Cmpr.mfx", 0, "a50865d73eb030f26f6df54341df8930", 19028980 },
				// { "LLava.mfx", 0, "05614efcb330c8f856980b2515007252", 5178433 },
				// { "Puzzle.mfx", 0, "d835730da23e09fb539c4a77293f7337", 13281112 },
				// { "SaveTown.mfx", 0, "10518372621f35dcd9559cfad68678f5", 5819650 },
				{ "SignInV.mfx", 0, "04d2f5d7182d7e91ff3e207a6d7e5cde", 2617580 },
				// { "SubRace.mfx", 0, "1c3415b492f7b7b6b6a3eebc8b09e37b", 7764830 },
				// { "Zone.mfx", 0, "80c23a93bddfa77454ba86d3ddf886f6", 31418461 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_VOLCANO,
		0,
		MTBOOT_MSB_VOLCANO_WIN_EN,
	},

	{ // The Magic School Bus Whales & Dolphins (English, Windows)
	  // Published by Microsoft, 2001
		{
			"msb_whales",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay32.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "BOB.mfx", 0, "d4d6abc27f96febfcd70258d05be39be", 20627391 },
				// { "CrewPass.mfx", 0, "7725162d0305af00458b866a0e90ab90", 3102954 },
				// { "gameshow.mfx", 0, "d26c52d3725fb442e5a4a906b6e90a39", 14590001 },
				// { "Maze.mfx", 0, "1f2d5e47009fdc101413c27ccd64d70c", 10024159 },
				// { "Scales.mfx", 0, "fb792fe3f15a7f4d05ae1a0e83146f80", 6135250 },
				{ "SignInW.mfx", 0, "c92ca5d5ef4b0d2ea34a477871ea3b27", 2038047 },
				// { "SpyHop.mfx", 0, "1e2856e9101b49d2f544b6782a0be56f", 6200975 },
				// { "Teria.mfx", 0, "5349254b0fbbaf58ec9c4c06c6fae579", 6343211 },
				// { "WPoster.mfx", 0, "e416437248335e1739fcd4a5a3306b78", 4294222 },
				// { "wpuzzle.mfx", 0, "b209404a7a46af109b93bee1964bb19e", 31800918 },
				// { "Zone.mfx", 0, "ccaa81d3542b27991e35b28ec5c847b6", 59535774 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MSB_WHALES,
		0,
		MTBOOT_MSB_WHALES_WIN_EN,
	},

	{ // Mykropolis: Planet der Roboter (German, Windows)
	  // Original title: Itacante: La Cité des Robots
	  // Published by Tivola, 2000
		{
			"mykropolis",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "mykropolis.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "machina.mfx", 0, "4cb398466b078280a237b9437c7a934a", 39111217 },
				// { "machina2.mxx", 0, "99a55b1cb02a761d29b75f75fbf4dc4b", 210044061 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MYKROPOLIS,
		0,
		MTBOOT_MYKROPOLIS_WIN_DE,
	},

	{ // Your Notebook (with help from Amelia) (English, Windows)
	  // Published by Pleasant Company, 1999
		{
			"notebook",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "Mtplay32.exe", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				// { "Bedroom.mfx", 0, "994009fcad48d713f4168503c2ebcb63", 54491655 },
				{ "Hallway.mfx", 0, "2b5991e1c00b7cc9ba2d80a9fa277020", 7664235 },
				// { "HallwayM.mfx", 0, "df60d2694d908d9686cb8fccebe1826d", 7698531 },
				// { "Journal.mfx", 0, "e57cd06e55c46e552154377d3ca73d27", 54028999 },
				// { "tutorial.mfx", 0, "41e1816fc1d475a93a0bef9f95c0a15b", 77214074 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_NOTEBOOK,
		0,
		MTBOOT_NOTEBOOK_WIN_EN,
	},

	{ // Pferd & Pony Lass uns reiten (German, Windows)
	  // Published by dtp entertainment, 2003
		{
			"pferdpony",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "hast.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "Start.mfw", 0, "66d0b37f3580d2d66f84ae100690aef6", 13609 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_PFERDPONY,
		0,
		MTBOOT_PFERDPONY_WIN_DE,
	},

	{ // Pinnacle Systems miroVideo Studio DC10 Plus (German, Windows)
		{
			"mirodc10",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "c23dccd2b7a525a9f7bb8505f7c7f2d4", 1085952 },
				{ "Studio Guided Tour.mfw", 0, "67f8ebfde1fe329b5c282f35fc885a8b", 54496799 },
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MIRODC10,
		0,
		MTBOOT_MIRODC10_WIN_DE,
	},

	{ // Pinnacle Systems miroVideo Studio DC10 Plus (English, Windows)
		{
			"mirodc10",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "MTPLAY32.EXE", 0, "c23dccd2b7a525a9f7bb8505f7c7f2d4", 1085952 },
				{ "Studio Guided Tour.mfw", 0, "47263c3d7f5bebba2d5dbcb4ed8aab1a", 32270329 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_MIRODC10,
		0,
		MTBOOT_MIRODC10_WIN_EN,
	},

	{ // Poser 3 content sampler on the sampler disc "Stuff for Poser" (English, Windows)
		{
			"poser3_sampler_zygote",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "ClickMe.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "summer98.mfw", 0, "c079c98d39833ea733669e88e42cae03", 96352413 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_POSER3_SAMPLER_ZYGOTE,
		0,
		MTBOOT_POSER3_SAMPLER_ZYGOTE_WIN_EN,
	},

	{ // Poser 3 content sampler on the disc for Poser 3 (English, Windows)
		{
			"poser3_zygote",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "CLICKME.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "ZYGOTE.MFW", 0, "7b98d86857bd412af291cae3e214d331", 18642284 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_POSER3_ZYGOTE,
		0,
		MTBOOT_POSER3_ZYGOTE_WIN_EN,
	},

	{ // Whitetail Extreme (English, Windows)
	  // Published by ValuSoft, 1999
		{
			"wtextreme",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "WTX.EXE", 0, "ed776bfe63d17a7d57d9625c3efe5a0a", 1085952 },
				{ "DATA.MFW", 0, "aa1d4694b3400f01e4fd55530331794d", 220904580 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_WTEXTREME,
		0,
		MTBOOT_WT_EXTREME_WIN_EN,
	},

	{ // Adobe 24 Hours Tools Sampler (English, Windows)
	  // Published by Against All Odds Productions, 1996
		{
			"adobe24",
			MetaEngineDetection::GAME_NOT_IMPLEMENTED,
			{
				{ "ADOBE32.EXE", 0, "b5efd541d37b23ab6a587e0929b05c2e", 1218048 },
				{ "DATA.MPL", 0, "e5d4f39ab5685556115ba0b34103bc2e", 27625660 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO0()
		},
		GID_ADOBE24,
		0,
		MTBOOT_ADOBE24_WIN_EN,
	},

	{ // Byzantine: The Betrayal - Demo (English, Windows)
	  // Published by Discovery Communications, 1997
		{
			"byzantine",
			"Demo",
			{
				{ "MTPLAY95.EXE", 0, "0e513dac9d2a0d7cfcdc670cab2a9bda", 757760 },
				{ "BYZPROJV.MPL", 0, "4f805af9af615f00482299746ff2474d", 62407900 },
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED | ADGF_DEMO,
			GUIO0()
		},
		GID_BYZANTINE,
		0,
		MTBOOT_BYZANTINE_DEMO_WIN_EN,
	},

	{ AD_TABLE_END_MARKER, 0, 0, MTBOOT_INVALID }
};

} // End of namespace MTropolis

#endif
