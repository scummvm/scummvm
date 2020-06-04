/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_DETECTION_TABLES_H
#define DIRECTOR_DETECTION_TABLES_H

namespace Director {

#define GENGAME_(t,e,f,m,s,l,p,fl,v) 	{ { t, e, AD_ENTRY1s(f, m, s), l, p, fl, GUIO1(GUIO_NOASPECT) }, GID_GENERIC, v }

#define MACGAME(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformMacintosh,ADGF_MACRESFORK,v)
#define WINGAME(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformWindows,ADGF_NO_FLAGS,v)
#define MACDEMO(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformMacintosh,(ADGF_MACRESFORK|ADGF_DEMO),v)
#define WINDEMO(t,e,f,m,s,v) 	GENGAME_(t,e,f,m,s,Common::EN_ANY,Common::kPlatformWindows,ADGF_DEMO,v)

static const DirectorGameDescription gameDescriptions[] = {
	// Execute all *.lingo files in game directory
	{
		{
			"directortest",
			"",
			AD_ENTRY1("lingotests", 0),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_TEST,
		4
	},

	// Execute all movies in directory
	{
		{
			"directortest-all",
			"",
			AD_ENTRY1("lingotests-all", 0),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_TESTALL,
		4
	},

	// Generic D2 Mac entry
	MACGAME("director", "D2-mac", "D2-mac", 0, -1, 2),
	// Generic D3 Mac entry
	MACGAME("director", "D3-mac", "D3-mac", 0, -1, 3),
	// Generic D4 Mac entry
	MACGAME("director", "D4-mac", "D4-mac", 0, -1, 4),
	// Generic D3 Win entry
	WINGAME("director", "D3-win", "D3-win", 0, -1, 3),
	// Generic D4 Win entry
	WINGAME("director", "D4-win", "D4-win", 0, -1, 4),

	MACGAME("theapartment", "D2", "Main Menu", "fc56c179cb8c6d4938e61ee61fd0032c", 48325, 2), // Original name is "•Main Menu"
	MACGAME("theapartment", "D3", "Main Menu", "9e838fe1a6af7992d656ca325e38dee5", 47911, 3), // Original name is "•Main Menu"
	MACGAME("theapartment", "D4", "Main Menu", "ff86181f03fe6eb060f65a985ca0580d", 160612, 4), // Original name is "•Main Menu"

	WINGAME("gundam0079", "", "Gundam0079.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 5),
	MACGAME("gundam0079", "", "Gundam0079", "4c38a51a21a1ad231f218c4786ff771d", -1, 5),

	MACGAME("ironhelix", "", "Iron Helix Minimum", "1ae45c23586b41997ba52e2e7c771c4c", 1652052, 3),

	WINGAME("jewels", "", "JEWELS.EXE", "bb6d81471d166088260090472c6c3a87", -1, 4),
	WINGAME("jewels", "", "Jewels.exe", "c1a2e8b7e41fa204009324a9c7db1030", -1, 7),
	WINDEMO("jewels", "Two-Minute Demo", "DEMO.EXE", "ebee52d3c4280674c600177df5b09da0", -1, 4),

	// Note: There are four versions of the binary included on the disc.
	// 5.6, 6, and 9 Meg variants all exist too.
	MACGAME("jewels", "", "Jewels 11 Meg", "339c89a148c4ff2c5c815c62ac006325", -1, 4),
	MACDEMO("jewels", "Two-Minute Demo", "Two-Minute Demo", "01be45e7241194dad07938e7059b88e3", -1, 4),

	MACGAME("jewels", "", "Jewels of the Oracle", "fa52f0136cde568a46249ce74f01a324", -1, 7),
	WINDEMO("jewels", "Demo", "JEWELS.EXE", "abcc448c035e88d4edb4a29034fd1e34", -1, 4),

	MACGAME("jman", "", "Journeyman.16 MB", "1ae45c23586b41997ba52e2e7c771c4c", 437743, 3),
	WINGAME("jman", "", "JMAN.EXE", "7c8230a804abf9353b05627a675b5ffb", 375282, 3),
	WINDEMO("jman", "", "JMDEMO.EXE", "7c8230a804abf9353b05627a675b5ffb", 375305, 3),

	{
		{
			"jman",
			"",
			AD_ENTRY1("JOURNEY.EXE", "65d06b5fef155a2473434571aff5bc29"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	MACGAME("jman", "Turbo!", "JMP Turbo\xE2\x84\xA2", "cc3321069072b90f091f220bba16e4d4", -1, 4), // Trademark symbol (UTF-8)

	WINGAME("majestic", "", "MAJESTIC.EXE", "624267f70253e5327981003a6fc0aeba", 23437631, 4),

	MACGAME("majestic", "", "Majestic", "01be45e7241194dad07938e7059b88e3", 483774, 4),

	// Meet Mediaband
	WINGAME("mediaband", "", "MEDIABND.EXE", "0cfb9b4762e33ab56d656a0eb146a048", 717921, 4),

	{
		{
			// Masters of the Elements - English (from lotharsm)
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			{
				{"CHECK.DXR", 0, "c31ee30eebd24a8cf31691fc9926daa4", 901820},
				{"MVM.EXE", 0, 0, 2565921},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - English (from lotharsm)

			"melements", "",
			{
				{"check.dxr", 0, "36f42340e819d1532c850880afe16581", 898206},
				{"Masters of the Elements", 0, 0, 1034962},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - German (from lotharsm)
			// Released in Germany as "Meister Zufall und die Herrscher der Elemente"
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			{
				{"CHECK.DXR", 0, "d1cd0ed95b0e30597e0089bf3e5caf0f", 575414},
				{"MVM.EXE", 0, 0, 1512503},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Masters of the Elements - German (from lotharsm)

			"melements", "",
			{
				{"check.dxr", 0, "9c81934b7616ab077f44825b8afaa83e", 575426},
				{"Meister Zufall", 0, 0, 1034962},
				AD_LISTEND
			},
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	WINGAME("spyclub", "", "SPYCLUB.EXE", "65d06b5fef155a2473434571aff5bc29", -1, 3),

	WINGAME("amber", "", "amber_jb.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 5),

	WINGAME("vvvampire", "", "VAMPIRE.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", -1, 3),
	MACGAME("vvvampire", "", "The Vampire's Coffin", "d41d8cd98f00b204e9800998ecf8427e", -1, 3),
	WINGAME("vvdinosaur", "", "DINOSAUR.EXE", "4e6303630f4dd588e730d09241cf7e76", -1, 3),
	MACGAME("vvdinosaur", "", "Start Game", "d41d8cd98f00b204e9800998ecf8427e", -1, 3),
	WINGAME("vvcyber", "", "CYBER.EXE", "65d06b5fef155a2473434571aff5bc29", 371139, 3),
	MACGAME("vvcyber", "", "Start Game", "7f443f2e63fd497a9ad85b10dc880a91", 384846, 3),
	WINDEMO("vvcyber", "Demo", "CYBER.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", 639439, 3),
	MACDEMO("vvcyber", "Demo", "CyberPlasm", "2ce360c9ea2da80a2c6d1040b0ad92dd", 504157, 3),
	WINGAME("vvharp", "", "HYPNOTIC.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", 370272, 3),
	MACGAME("vvharp", "", "Start Adventure", "7f443f2e63fd497a9ad85b10dc880a91", 383406, 3),

	MACGAME("warlock", "V1.0", "Spaceship Warlock", "cfa68a1bc49251497ebde18e5fc9c217", 271093, 2),
	MACGAME("warlock", "V1.1.1", "Spaceship Warlock", "cfa68a1bc49251497ebde18e5fc9c217", 271107, 2),
	WINGAME("warlock", "", "SSWARLCK.EXE", "65d06b5fef155a2473434571aff5bc29", 370867, 2),

	WINDEMO("warlock", "", "SSWDEMO.EXE", "65d06b5fef155a2473434571aff5bc29", 370934, 2),

	{
		{
			"ernie",
			"Demo",
			AD_ENTRY1s("ERNIE.EXE", "1a7acbba10a7246ba58c1d53fc7203f5", 1417371),
			Common::SE_SWE,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ernie",
			"",
			AD_ENTRY1s("Ernie.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1417481),
			Common::SE_SWE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"gadget",
			"",
			AD_ENTRY1s("GADGET.EXE", "d62438566e44826960fc16c5c23dbe43", 2212541),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS, //ADGF_HICOLOR,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	WINGAME("id4p1", "iD4 Mission Disk 1 - Alien Supreme Commander", "SUPREME.EXE", "629eb9a5d991a2dbe380804e8c37043a", 1664965, 4),
	WINGAME("id4p2", "iD4 Mission Disk 2 - Alien Science Officer", "SCIENCE.EXE", "812a4b81b70e61e547c14dbbd507b402", 1766499, 4),
	WINGAME("id4p3", "iD4 Mission Disk 3 - Warrior Alien", "WARRIOR.EXE", "387245092ce0583c6fd0c54000b1502a", 1751593, 4),
	WINGAME("id4p4", "iD4 Mission Disk 4 - Alien Navigator", "NAV.EXE", "29de2d1fd34029b3c97ce852a7fc665e", 1834037, 4),
	WINGAME("id4p5", "iD4 Mission Disk 5 - Captain Steve Hiller", "STEVE.EXE", "14f19b724dd6361e4bf3cfddbac87d3f", 1797301, 4),
	WINGAME("id4p6", "iD4 Mission Disk 6 - Dave's Computer", "DAVE.EXE", "237f9db2ea9a38fa6f7292974539f494", 1774167, 4),
	WINGAME("id4p7", "iD4 Mission Disk 7 - President Whitmore", "PREZ.EXE", "e7d03a6d749d65dbcea7171ec4627e9c", 1666293, 4),
	WINGAME("id4p8", "iD4 Mission Disk 8 - Alien Attack Fighter", "ALIEN_F.EXE", "ee2eb92900d515ed7872c57f3b89a408", 1754975, 4),
	WINGAME("id4p9", "iD4 Mission Disk 9 - FA-18 Fighter Jet", "F18.EXE", "7a0292909a5103c89297c40cce1d836c", 1691893, 4),
	WINGAME("id4p10", "iD4 Mission Disk 10 - Alien Bomber", "BOMBER.EXE", "17758a9f425f7f3e7a926951e6c770f4", 1844189, 4),
	WINGAME("id4p11", "iD4 Mission Disk 11 - Area 51", "AREA51.EXE", "78be40f9c7e8e1770c388cc16a522aaf", 1776077, 4),

	WINGAME("chopsuey", "Chop Suey", "CHOPSUEY.EXE", "785e26240153a028549e8a66c2e904bf", 772382, 4),

	{
		{
			// Uses VideoWorks Player 1.00 (pre-director)

			"madmac", "",
			AD_ENTRY1s("VW Player", "16034c0db4619552e4fe7e108cb98dae", 17070),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		0
	},

	{
		{
			"alexworld", "",
			AD_ENTRY1s("ALEX.EXE", "65d06b5fef155a2473434571aff5bc29", 369996),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"alexworld", "",
			AD_ENTRY1s("AlexWorld", "f5277c53bacd27936158dd3867e587e2", 391576),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"alice", "",
			AD_ENTRY1s("Alice", "e54ec74aeb4355b0acd46320327c1bed", 274274),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		2
	},

	WINGAME("alice", "", "ALICE.EXE", "c67ece9b657ee80355520ac37a25fe55", 684733, 4),
	MACGAME("alice", "", "Alice", "3b61149c922f0fd815ca29686e4f554a", 304714, 4),
	MACGAME("amandastories", "3.0", "AmandaStories", "1687f456d7f2bcf98e84fec6d3a03c17", 353985, 3),
	WINGAME("beyondthewall", "", "WALL.EXE", "5c6862a40a48881913ee7c50d25cd984", 225253, 3),
	MACGAME("beyondthewall", "", "Beyond The Wall Of Stars", "107c6a6d3cce957eaa26d427e672000c", 465523, 3),

	// original file name is The C.H.A.O.S. Continuum™
	MACGAME("chaos", "", "The C.H.A.O.S. Continuum", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384481, 3),
	WINGAME("chaos", "", "CHAOS.EXE", "7c8230a804abf9353b05627a675b5ffb", 375294, 3),
	MACGAME("maze", "", "Maze", "c4917547b9d9fda87567a9b3544489c4", 384534, 3),

	// original file name is Who Killed Sam Rupert?
	MACGAME("murdersam", "", "Who Killed Sam Rupert", "243ed9ef948de907401784d0938a4ed5", 518996, 3),
	WINGAME("murdersam", "", "SAM.EXE", "65d06b5fef155a2473434571aff5bc29", 370016, 3),
	MACGAME("murdermagic", "", "The Magic Death", "a8788e8b79dccc582b9818ec63734bed", 736754, 3),
	WINGAME("murdermagic", "", "MAGIC.EXE", "7c8230a804abf9353b05627a675b5ffb", 375298, 3),
	MACGAME("murderbrett", "", "The Environmental Surfer", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384469, 3),
	WINGAME("murderbrett", "", "BRETTP.EXE", "65d06b5fef155a2473434571aff5bc29", 370010, 3),
	MACGAME("murdertaylor", "", "Taylor French", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384469, 3),
	WINGAME("murdertaylor", "", "TAYLORF.EXE", "65d06b5fef155a2473434571aff5bc29", 370010, 3),

	WINGAME("snh", "", "SNH.EXE", "5a6e241994f671a1ebe664fbae394217", 378901, 3),
	MACGAME("snh", "", "A Silly Noisy House", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 357142, 3),

	{
		{
			"murderbrett", "",
			AD_ENTRY1s("BRAD.EXE", "e1c3cb4df694b887f3708f9ebb72302f", 934342),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"murderbrett", "",
			AD_ENTRY1s("Brad, le surfeur", "227fcce08de8028e2201f5f4eb3b3cc3", 720160),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"murdertaylor", "",
			AD_ENTRY1s("TF.EXE", "d7a97fbe1daf2bc5fdecee2725c297cd", 952741),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"murdertaylor", "",
			AD_ENTRY1s("Taylor French", "dc839fb9e2f8e6bfc4f4f995c57df495", 291683),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"lzone", "",
			AD_ENTRY1s("L_ZONE.EXE", "65d06b5fef155a2473434571aff5bc29", 370009),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"lzone", "V2.0",
			AD_ENTRY1s("L-ZONE", "276bee761e48a6fd709df77d5c2f60dd", 395344),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			// May contain English, French, Japanese text

			"gadget", "",
			AD_ENTRY1s("GADGET.EXE", "2cc9e2dacb90fb130f6ee9519b66c5a8", 369009),
			Common::EN_ANY,
			Common::kPlatformFMTowns,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"lzone", "",
			AD_ENTRY1s("L_ZONE.EXE", "65d06b5fef155a2473434571aff5bc29", 370009),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"lzone", "V2.0",
			AD_ENTRY1s("L-ZONE", "276bee761e48a6fd709df77d5c2f60dd", 395344),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"lzone", "Bandai Pippin",
			AD_ENTRY1s("L-ZONE", "07c3095bd36e5aed00ba12f17f761070", 60352),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			// original file name is ~LEGEND OF PSY•S CITY~

			"the7colors", "",
			AD_ENTRY1s("LEGEND OF PSYS CITY", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 384001),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			// English translation by Skye Sonomura (HomeStarRunnerTron)

			"the7colors", "",
			AD_ENTRY1s("T7C-R!", "d84feae6ba35340857f46c9cbd6bd1a6", 398981),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"tri3dtrial", "",
			AD_ENTRY1s("Tri-3D-Trial", "cfa68a1bc49251497ebde18e5fc9c217", 271223),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		2
	},

	{
		{
			"refixion1", "",
			AD_ENTRY1s("REFIXION", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 356245),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"refixion2", "",
			AD_ENTRY1s("Museum or Hospital", "1ae45c23586b41997ba52e2e7c771c4c", 375841),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"refixion3", "",
			AD_ENTRY1s("The Reindeer Story", "f5277c53bacd27936158dd3867e587e2", 392473),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	MACGAME("wrath", "", "Wrath of the Gods", "2ce360c9ea2da80a2c6d1040b0ad92dd", 413986, 3),

	{
		{
			"wrath", "",
			{
				{"000WRATH.EXE", 0, "3162423a1d0885eb1eb94f557a86b258", 372970},
				{"SHARDCST.MMM", 0, "22af72fba773cc5313d99c91b2e5aea9", 3432948},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"wrath", "Demo",
			{
				{"000WRATH.EXE", 0, "3162423a1d0885eb1eb94f557a86b258", 372970},
				{"SHARDCST.MMM", 0, "85d166abe18730d96e9d4137c66255a1", 3553330},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	MACGAME("ybr1", "", "YBR", "f5277c53bacd27936158dd3867e587e2", 414498, 3),

	{
		{
			"ybr1", "",
			{
				{"YBR.EXE", 0, "2cc9e2dacb90fb130f6ee9519b66c5a8", 369007},
				{"START.MMM", 0, "69012b34b3f0e6f1886133b807db4476", 2566872},
				AD_LISTEND
			},
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"ybr1", "",
			{
				{"YBR.EXE", 0, "2cc9e2dacb90fb130f6ee9519b66c5a8", 369007},
				{"START.MMM", 0, "9e02f41270708101b1d5d04cb822a784", 2563274},
				AD_LISTEND
			},
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"ybr1", "",
			{
				{"YBR", 0, "f5277c53bacd27936158dd3867e587e2", 414498},
				{"START", 0, "ca4a6650d8f8ce04b26bcbc941bf0a86", 2417242},
				AD_LISTEND
			},
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		3
	},

	{
		{
			"ybr2", "Demo",
			AD_ENTRY1s("YBR2DEMO.EXE", "25ecc053e02a0ef537d34d615119c814", 900973),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"ybr2", "",
			AD_ENTRY1s("YBR2.EXE", "dbe273c1df60305be98a1a582ddd2c3b", 860579),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"ybr2", "",
			AD_ENTRY1s("YBR2", "b797956eb3a4dabcc15bfadf6fc66591", 504153),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		4
	},

	{
		{
			"ybr3", "Single Version",
			AD_ENTRY1s("YBRSingl.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1410799),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ybr3", "Demo",
			AD_ENTRY1s("harapeco.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1693107),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ybr3", "Single Version",
			AD_ENTRY1s("YBR_Single", "6e7e31d05709e1d38d63f4df6a59eec0", 722547),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			// Original file name is non-ASCII Japanese characters

			"ybr3", "Demo",
			AD_ENTRY1s("harapeco", "4ab012350c6e1c39eda772e33df30cea", 723903),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ybr3", "",
			AD_ENTRY1s("YBRH", "4c38a51a21a1ad231f218c4786ff771d", 105855),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{
		{
			"ybr3", "",
			AD_ENTRY1s("YBRH.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1410805),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		5
	},

	{ AD_TABLE_END_MARKER, GID_GENERIC, 0 }
};

} // End of Namespace Director

#endif
