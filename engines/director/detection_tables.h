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
#define MACGAME_l(t,e,f,m,s,l,v) GENGAME_(t,e,f,m,s,l,Common::kPlatformMacintosh,ADGF_MACRESFORK,v)
#define WINGAME_l(t,e,f,m,s,l,v) GENGAME_(t,e,f,m,s,l,Common::kPlatformWindows,ADGF_NO_FLAGS,v)
#define MACDEMO_l(t,e,f,m,s,l,v) GENGAME_(t,e,f,m,s,l,Common::kPlatformMacintosh,(ADGF_MACRESFORK|ADGF_DEMO),v)
#define WINDEMO_l(t,e,f,m,s,l,v) GENGAME_(t,e,f,m,s,l,Common::kPlatformWindows,ADGF_DEMO,v)

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
		400
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
		400
	},

	// Generic D2 Mac entry
	MACGAME("director", "D2-mac", "D2-mac", 0, -1, 200),
	// Generic D3 Mac entry
	MACGAME("director", "D3-mac", "D3-mac", 0, -1, 300),
	// Generic D4 Mac entry
	MACGAME("director", "D4-mac", "D4-mac", 0, -1, 400),
	// Generic D3 Win entry
	WINGAME("director", "D3-win", "D3-win", 0, -1, 300),
	// Generic D4 Win entry
	WINGAME("director", "D4-win", "D4-win", 0, -1, 400),

	MACGAME("theapartment", "D2", "Main Menu", "fc56c179cb8c6d4938e61ee61fd0032c", 48325, 200), // Original name is "•Main Menu"
	MACGAME("theapartment", "D3", "Main Menu", "9e838fe1a6af7992d656ca325e38dee5", 47911, 300), // Original name is "•Main Menu"
	MACGAME("theapartment", "D4", "Main Menu", "ff86181f03fe6eb060f65a985ca0580d", 160612, 400), // Original name is "•Main Menu"

////////////////////////////////////////
//
// VideoWorks Player 1.00 (pre-director)
//
////////////////////////////////////////

	MACGAME_l("madmac", "", "VW Player", "16034c0db4619552e4fe7e108cb98dae", 17070, Common::FR_FRA, 0),

////////////////////////////////////////
//
// MacroMind Director v2
//
////////////////////////////////////////

	MACGAME_l("alice", "", "Alice", "e54ec74aeb4355b0acd46320327c1bed", 274274, Common::JA_JPN, 200),
	MACGAME_l("tri3dtrial", "", "Tri-3D-Trial", "cfa68a1bc49251497ebde18e5fc9c217", 271223, Common::JA_JPN, 200),

	MACGAME("warlock", "v1.0", "Spaceship Warlock", "cfa68a1bc49251497ebde18e5fc9c217", 271093, 200),
	MACGAME("warlock", "v1.1.1", "Spaceship Warlock", "cfa68a1bc49251497ebde18e5fc9c217", 271107, 200),
	MACDEMO("warlock", "Demo", "Spaceship Warlock Demo", "cfa68a1bc49251497ebde18e5fc9c217", 271099, 200),

////////////////////////////////////////
//
// MacroMind / Macromedia Director v3
//
////////////////////////////////////////

	MACGAME_l("alexworld", "", "AlexWorld", "f5277c53bacd27936158dd3867e587e2", 391576, Common::JA_JPN, 300),
	WINGAME_l("alexworld", "", "ALEX.EXE", "65d06b5fef155a2473434571aff5bc29", 369996, Common::JA_JPN, 300),

	MACGAME("amandastories", "3.0", "AmandaStories", "1687f456d7f2bcf98e84fec6d3a03c17", 353985, 300),

	// From Isaac Asimov's The Ultimate Robot CD-ROM
	WINGAME("asimovrat", "", "TOOLKIT.EXE", "65d06b5fef155a2473434571aff5bc29", 1858955, 310), // confirm director version

	MACGAME("beyondthewall", "", "Beyond The Wall Of Stars", "107c6a6d3cce957eaa26d427e672000c", 465523, 300),
	WINGAME("beyondthewall", "", "WALL.EXE", "5c6862a40a48881913ee7c50d25cd984", 225253, 300),

	WINDEMO("bpmc", "", "BPMC.EXE", "1998188253fc8657198e3e78efe823a8", 370291, 313), // confirm director version

	MACGAME("chaos", "", "The C.H.A.O.S. Continuum", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384481, 300), // original name is The C.H.A.O.S. Continuum™
	WINGAME("chaos", "", "CHAOS.EXE", "7c8230a804abf9353b05627a675b5ffb", 375294, 300),

	// almost all folders and file names use Japanese encoding, including executable
	MACGAME_l("easternmind", "", "Tong Nou", "276bee761e48a6fd709df77d5c2f60dd", 395037, Common::JA_JPN, 300),

	MACGAME("encarta95", "", "Encarta Projector", "7f443f2e63fd497a9ad85b10dc880a91", 385985, 313),

	MACGAME("freakshow", "D3", "Freak Show Projector", "bb61dd0cc8356e51fe54c1a00f979b8f", 439483, 300),

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
		300
	},

	MACGAME("hhouse", "", "The Ultimate Haunted House", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384475, 313),

	MACGAME("ironhelix", "", "Iron Helix Minimum", "1ae45c23586b41997ba52e2e7c771c4c", 1652052, 300),
	MACDEMO("ironhelix", "Demo", "IRON HELIX DEMO", "7c80091b84d4d5d48f7c773b30b10598", 1634901, 300),

	MACGAME("jman", "", "Journeyman.16 MB", "1ae45c23586b41997ba52e2e7c771c4c", 437743, 300),
	MACDEMO("jman", "Sneak Preview", "Journeyman Sneak Preview", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 6222889, 300),
	MACDEMO("jman", "CD Demo", "Journeyman Demo", "424093b2bc04a1bfa4376c878f67a364", 3083188, 300),
	MACDEMO("jman", "Looping Demo", "Journeyman Looping Demo", "1ae45c23586b41997ba52e2e7c771c4c", 3077658, 300),
	WINGAME("jman", "", "JMAN.EXE", "7c8230a804abf9353b05627a675b5ffb", 375282, 300),
	WINGAME_l("jman", "", "JOURNEY.EXE", "65d06b5fef155a2473434571aff5bc29", -1, Common::JA_JPN, 300),
	WINDEMO("jman", "Demo", "JMDEMO.EXE", "7c8230a804abf9353b05627a675b5ffb", 375305, 300),

	MACGAME("jmmd", "", "Just Me & My Dad", "ea646eccc9a53f44ce082459d4809a06", 486966, 400),

	MACGAME("kyoto", "v1.01", "Cosmology of KYOTO", "f5277c53bacd27936158dd3867e587e2", 392505, 300),

	MACGAME("lzone", "", "L-ZONE", "f5277c53bacd27936158dd3867e587e2", 392484, 300),
	MACGAME("lzone", "v2", "L-ZONE", "276bee761e48a6fd709df77d5c2f60dd", 395344, 300),
	{
		{
			"lzone", "",
			AD_ENTRY2s("L_ZONE.EXE",	"65d06b5fef155a2473434571aff5bc29", 370009,
					   "SYNER_01.MMM",	"56b6f1c68e85a96bcdd01028bdec2d35", 460594),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},


	MACGAME("maze", "", "Maze", "c4917547b9d9fda87567a9b3544489c4", 384534, 300),

	MACGAME("murderbrett", "", "The Environmental Surfer", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384469, 300),
	WINGAME("murderbrett", "", "BRETTP.EXE", "65d06b5fef155a2473434571aff5bc29", 370010, 300),
	MACGAME("murdermagic", "", "The Magic Death", "a8788e8b79dccc582b9818ec63734bed", 736754, 300),
	WINGAME("murdermagic", "", "MAGIC.EXE", "7c8230a804abf9353b05627a675b5ffb", 375298, 300),
	MACGAME("murdersam", "", "Who Killed Sam Rupert", "243ed9ef948de907401784d0938a4ed5", 518996, 300), // original name is Who Killed Sam Rupert?
	WINGAME("murdersam", "", "SAM.EXE", "65d06b5fef155a2473434571aff5bc29", 370016, 300),
	MACGAME("murdertaylor", "", "Taylor French", "2ce360c9ea2da80a2c6d1040b0ad92dd", 384469, 300),
	WINGAME("murdertaylor", "", "TAYLORF.EXE", "65d06b5fef155a2473434571aff5bc29", 370010, 300),

	MACGAME("mylk", "", "Mylk DoubleClickMe!", "8a33471513b6896d3d13b8a9c8790d86", 356280, 300),

	MACGAME_l("refixion1", "", "REFIXION", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 356245, Common::JA_JPN, 300),
	MACGAME_l("refixion2", "", "Museum or Hospital", "1ae45c23586b41997ba52e2e7c771c4c", 375841, Common::JA_JPN, 300),
	MACGAME_l("refixion3", "", "The Reindeer Story", "f5277c53bacd27936158dd3867e587e2", 392473, Common::JA_JPN, 300),

	MACDEMO("rodney", "Demo", "Rodney's Funscreen demo", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 356866, 300), // full game is supported in MADE engine
	MACDEMO("screamingmetal", "Demo", "Screaming Metal Demo", "0a280df213c5bf3eca241f37151e1d56", 373190, 300), // full game was never released

	MACGAME("snh", "", "A Silly Noisy House", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 357142, 300),
	WINGAME("snh", "", "SNH.EXE", "5a6e241994f671a1ebe664fbae394217", 378901, 300),

	WINGAME("spyclub", "", "SPYCLUB.EXE", "65d06b5fef155a2473434571aff5bc29", -1, 300),

	MACGAME_l("the7colors", "", "LEGEND OF PSYS CITY", "9f0bb7ec7720e4f680ee3aa3d22c1c9d", 384001, Common::JA_JPN, 300), //original name is ~LEGEND OF PSY•S CITY~
	MACGAME("the7colors", "", "T7C-R!", "d84feae6ba35340857f46c9cbd6bd1a6", 398981, 300), // English translation by Skye Sonomura (HomeStarRunnerTron)

	MACGAME("vvcyber", "", "Start Game", "7f443f2e63fd497a9ad85b10dc880a91", 384846, 300),
	WINGAME("vvcyber", "", "CYBER.EXE", "65d06b5fef155a2473434571aff5bc29", 371139, 300),
	MACDEMO("vvcyber", "Demo", "CyberPlasm", "2ce360c9ea2da80a2c6d1040b0ad92dd", 504157, 300),
	WINDEMO("vvcyber", "Demo", "CYBER.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", 639439, 300),
	MACGAME("vvdinosaur", "", "Start Game", "d41d8cd98f00b204e9800998ecf8427e", -1, 300),
	WINGAME("vvdinosaur", "", "DINOSAUR.EXE", "4e6303630f4dd588e730d09241cf7e76", -1, 300),
	WINGAME("vvdinosaur", "v1.1", "DINOSAUR.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", 370049, 300),
	MACGAME("vvharp", "", "Start Adventure", "7f443f2e63fd497a9ad85b10dc880a91", 383406, 300),
	WINGAME("vvharp", "", "HYPNOTIC.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", 370272, 300),
	MACGAME("vvvampire", "", "The Vampire's Coffin", "d41d8cd98f00b204e9800998ecf8427e", -1, 300),
	WINGAME("vvvampire", "", "VAMPIRE.EXE", "88f4f7406f34ec36e751a64f7c76f2c4", -1, 300),

	WINGAME("warlock", "", "SSWARLCK.EXE", "65d06b5fef155a2473434571aff5bc29", 370867, 300),
	WINDEMO("warlock", "", "SSWDEMO.EXE", "65d06b5fef155a2473434571aff5bc29", 370934, 300),

	MACGAME("wrath", "", "Wrath of the Gods", "2ce360c9ea2da80a2c6d1040b0ad92dd", 413986, 300),

	{
		{
			"wrath", "",
			AD_ENTRY2s("000WRATH.EXE", "3162423a1d0885eb1eb94f557a86b258", 372970,
					   "SHARDCST.MMM", "22af72fba773cc5313d99c91b2e5aea9", 3432948),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},

	{
		{
			"wrath", "Demo",
			AD_ENTRY2s("000WRATH.EXE", "3162423a1d0885eb1eb94f557a86b258", 372970,
					   "SHARDCST.MMM", "85d166abe18730d96e9d4137c66255a1", 3553330),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},

	MACDEMO("xanthus", "Demo", "X A N T H U S", "1ae45c23586b41997ba52e2e7c771c4c", 375447, 300), // may be vaporware; original name is ∆ X A N T H U S ∆

	MACGAME("ybr1", "", "YBR", "f5277c53bacd27936158dd3867e587e2", 414498, 300),

	{
		{
			"ybr1", "",
			AD_ENTRY2s("YBR.EXE",	"2cc9e2dacb90fb130f6ee9519b66c5a8", 369007,
					   "START.MMM",	"69012b34b3f0e6f1886133b807db4476", 2566872),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},

	{
		{
			"ybr1", "",
			AD_ENTRY2s("YBR",	"f5277c53bacd27936158dd3867e587e2", 414498,
					   "START",	"ca4a6650d8f8ce04b26bcbc941bf0a86", 2417242),
			Common::JA_JPN,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},

	{
		{
			"ybr1", "",
			AD_ENTRY2s("YBR.EXE",	"2cc9e2dacb90fb130f6ee9519b66c5a8", 369007,
					   "START.MMM",	"9e02f41270708101b1d5d04cb822a784", 2563274),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		300
	},

////////////////////////////////////////
//
// Macromedia Director v4
//
////////////////////////////////////////

	MACGAME("9worlds", "", "Nine Worlds", "3930940b5b4b2ae2563cc29b812855a6", 482838, 400),
	WINGAME("9worlds", "", "9WORLDS.EXE", "9da9df906149916b340a0a8fbee5ca52", 741994, 400),
	MACGAME_l("9worlds", "", "Neun Welten", "3930940b5b4b2ae2563cc29b812855a6", 483774, Common::DE_DEU, 400),
	WINGAME_l("9worlds", "", "9WORLDS.EXE", "054e5af279a05ab64627ef664631fe37", 742002, Common::DE_DEU, 400),

	MACGAME("alice", "", "Alice", "3b61149c922f0fd815ca29686e4f554a", 304714, 400),
	WINGAME("alice", "", "ALICE.EXE", "c67ece9b657ee80355520ac37a25fe55", 684733, 400),

	WINGAME_l("ankh1", "", "ANKH.EXE", "ca3b036aed6f38beb3ee762e171b663e", 746959, Common::JA_JPN, 404),

	MACGAME("arcofdoom", "", "Arc Player (4mb)", "ea553e3f3fa123ad38fa5249621e8c8b", 295426, 400),
	MACDEMO("arcofdoom", "Demo", "ArcDemo", "cdb27c916044ae4dceb4b7326063fa03", 301925, 400),
	WINGAME("arcofdoom", "", "ARC.EXE", "fd3e9ad004597b01656b28c348a2ae7f", 687973, 400),

	MACGAME("artrageous", "", "Artrageous!", "17efee018a660458fae80de4364021ac", 483746, 404),
	WINGAME("artrageous", "", "ART.EXE", "926ca15bcb7bdccc8f9b14b052d05f83", 696827, 404),

	WINDEMO("ataripack", "Demo", "ACTDEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	// both Mac and Win versions require installation to extract executable
	MACGAME("badday", "", "BadDay (Universal)", "b7e69c37b7355022d400c14aa97c5d54", 483781, 400),
	WINGAME("badday", "", "BAD_DAY.EXE", "4ad339db5379a860b33cbb0fa64f18e2", 742342, 400),

	WINGAME("bowie", "", "JUMP.EXE", "1706ba3b999d724957198c134e508127", 684317, 400),

	WINDEMO("c64pack", "Demo", "C64DEMO.EXE", "0458e44d9ad4ae0d3a583ac4141d21ae", 1007517, 400), // full game is not Director

	WINGAME("chopsuey", "", "CHOPSUEY.EXE", "785e26240153a028549e8a66c2e904bf", 772382, 400),

	MACGAME_l("chuteng", "", "ChuTeng", "ea646eccc9a53f44ce082459d4809a06", 484351, Common::JA_JPN, 400), // executable file name is in Japanese characters
	WINGAME_l("chuteng", "", "CHUTENG.EXE", "aaef7b33829ff7b0243412c89869e011", 746971, Common::JA_JPN, 400),

	MACGAME("daedalus", "", "DAEDALUS.DAT", "4eb2fab14f1b932ae9f01d8b05a411de", 69794, 400),
	MACGAME_l("daedalus", "", "DAEDALUS.DAT", "2b9e22ee93e5d58f383ef389fa09e12b", 69794, Common::DE_DEU, 400),
	MACGAME_l("daedalus", "", "DAEDALUS.DAT", "2b9e22ee93e5d58f383ef389fa09e12b", 69794, Common::FR_FRA, 400),

	// both Mac and Win versions require installation to extract executable
	// Mac version has 68k, PowerPC, and a fat binary (default) as options in the installer
	MACGAME("darkeye", "", "The Dark Eye (Universal)", "67f572196550aedb1f9523d782022be0", 486334, 400),
	WINGAME("darkeye", "", "DARKEYE.EXE", "6190ac8ccef3fd073e912ec5d0850f94", 742326, 400),

	MACGAME("derratsorcerum", "", "Derrat Sorcerum v1.0.3", "a546ef2b921a9b48fd93928f1a505f22", 483762, 400), // original name is Derrat Sorcerum™ v1.0.3
	MACDEMO("derratsorcerum", "Demo", "Derrat Sorcerum Demo v1.0.3", "a546ef2b921a9b48fd93928f1a505f22", 483762, 400), // original name is Derrat Sorcerum™ Demo v1.0.3

	// both Mac and Win versions require installation to extract executable
	MACGAME("devo", "", "Smart Patrol", "01be45e7241194dad07938e7059b88e3", 478506, 400),
	WINGAME("devo", "", "SMRTPTRL.EXE", "6030100dbff520b8763f6e98d15389d7", 741904, 400),

	MACGAME_l("earthtia", "", "LATHUR'S LEGEND", "12345", 12345, Common::JA_JPN, 400), // many files and folders use only JP char encoding

	WINDEMO("earthwormjim", "Demo", "EWJDEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("easternmind", "", "Eastern Mind", "78138a77207907642744d0960c14d9e5", 483746, 400),
	WINGAME("easternmind", "", "ESTRNMND.EXE", "2597cfb5466d193b5a18fc7b0e3a3fa5", 685789, 400),

	WINGAME("einstein", "", "EINSTEIN.EXE", "5f2b54a1cbf0c62913d4f1fa07b5f3a9", 711759, 404),
	WINGAME("flw", "", "FLW.EXE", "7d02787f74f2adfc0c66f34646380c0e", 720683, 400),

	MACGAME("frankenstein", "", "Frankenstein", "01be45e7241194dad07938e7059b88e3", 483771, 403),
	WINGAME("frankenstein", "", "FRANKIE.EXE", "a9a4e3a07f3ecda7e59215aabb844532", 2229509, 404),
	WINDEMO("frankenstein", "Demo", "FRNKDEMO.EXE", "ed00aa67138b715a8bee6f41756852c5", 7365699, 400),
	WINGAME_l("frankenstein", "", "FRANKIE.EXE", "98beeaa18c58708b0ce82de320e3bdf7", 2196911, Common::DE_DEU, 404),

	MACGAME("freakshow", "D4", "FreakShow Projector", "dc5a87dda7a0daf46604515f7d2cca66", 300644, 400),
	WINGAME("freakshow", "", "FREAKS.EXE", "b2e1c5abc0e3c31ddd2089a55a27a0a5", 695893, 400),

	MACDEMO("gundam0079", "Demo", "Gundam Demo", "01be45e7241194dad07938e7059b88e3", 483376, 400), // full game is D5

	// Different releases of hamsterland1 allow choice of language on the opening screen
	// English is always available, along with one alternate language

	{
		{
			"hamsterland1", "Danish alt",
			AD_ENTRY2s("Hamsterland",	"bcd3c718db258701496b3c5bcb827ef2", 483774,
					   "AIRDK",			"f74f18a9d454db63465dac151a6b5706", 8425728),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"hamsterland1", "Swedish alt",
			AD_ENTRY2s("Hamsterland",	"bcd3c718db258701496b3c5bcb827ef2", 483774,
					   "AIRSW",			"ccf7c642796c5aeb9fee4c663bdd02d9", 7588992),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"hamsterland1", "Danish alt",
			AD_ENTRY2s("HAMSTER.EXE",	"2bb6e93d9aab502f46dbf163d51eb785", 785327,
					   "AIRDK",			"ad98bbbd724e31a8a125e284a74d0241", 8425546),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"hamsterland1", "Spanish alt",
			AD_ENTRY2s("HAMSTER.EXE",	"a0d2cfdf10ec14c00338329277bc8acc", 727653,
					   "AIRSP",			"9b5b82d488b96b2fcf5e78ec8769e29a", 8432260),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"hamsterland1", "Swedish alt",
			AD_ENTRY2s("HAMSTER.EXE",	"2bb6e93d9aab502f46dbf163d51eb785", 785327,
					   "AIRSW",			"410efb55564b8eb2042eea61d0f8025b", 7588828),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	// original file name is La Máquina del Tiempo
	MACGAME_l("hamsterland2", "", "La Mquina del Tiempo", "2c7d0c0d55cbe3f622c7d68cedb671e0", 483774, Common::ES_ESP, 400),
	WINGAME_l("hamsterland2", "", "HL2SP.EXE", "8dd22b8bde300c9fb5b66808905300f7", 875665, Common::ES_ESP, 400),
	MACGAME("hamsterland2", "", "The Time Machine", "2c7d0c0d55cbe3f622c7d68cedb671e0", 483774, 400),
	WINGAME("hamsterland2", "", "HL2EN.EXE", "4c812a63165c1f50242e879aae62718d", 859681, 400),

	WINGAME("hhouse", "", "HHOUSE.EXE", "e798a08ba6057705af028673f7a9a2ef", 3181345, 400),

	MACGAME("horrortour1", "", "ZEDDAS(7M)", "da0da5d543b237051975ad70bec129f4", 483443, 400),
	WINGAME("horrortour1", "", "ZEDDAS.EXE", "d3ce91f305dd0f4d2f5e9ff6d4e41a3b", 741818, 400),
	MACGAME_l("horrortour1", "", "HorrorTour(7M)", "b7e69c37b7355022d400c14aa97c5d54", 483443, Common::JA_JPN, 400),
	WINGAME_l("horrortour1", "", "HORROR.EXE", "b16ecf8ebc02142c742b8a9acc935d52", 752425, Common::JA_JPN, 400),
	MACGAME_l("horrortour2", "", "ZEDDAS PowerPC", "da7d3f1d85bdb99518b586c40d2a673e", 60013, Common::JA_JPN, 400),
	WINGAME_l("horrortour2", "", "HT2.EXE", "499d8545ee2325b18d3f09fb2c0fc26e", 698029, Common::JA_JPN, 400),

	WINDEMO("hyperblade", "Demo", "HYPER.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	WINGAME("id4p1", "iD4 Mission Disk 1 - Alien Supreme Commander", "SUPREME.EXE", "629eb9a5d991a2dbe380804e8c37043a", 1664965, 400),
	WINGAME("id4p2", "iD4 Mission Disk 2 - Alien Science Officer", "SCIENCE.EXE", "812a4b81b70e61e547c14dbbd507b402", 1766499, 400),
	WINGAME("id4p3", "iD4 Mission Disk 3 - Warrior Alien", "WARRIOR.EXE", "387245092ce0583c6fd0c54000b1502a", 1751593, 400),
	WINGAME("id4p4", "iD4 Mission Disk 4 - Alien Navigator", "NAV.EXE", "29de2d1fd34029b3c97ce852a7fc665e", 1834037, 400),
	WINGAME("id4p5", "iD4 Mission Disk 5 - Captain Steve Hiller", "STEVE.EXE", "14f19b724dd6361e4bf3cfddbac87d3f", 1797301, 400),
	WINGAME("id4p6", "iD4 Mission Disk 6 - Dave's Computer", "DAVE.EXE", "237f9db2ea9a38fa6f7292974539f494", 1774167, 400),
	WINGAME("id4p7", "iD4 Mission Disk 7 - President Whitmore", "PREZ.EXE", "e7d03a6d749d65dbcea7171ec4627e9c", 1666293, 400),
	WINGAME("id4p8", "iD4 Mission Disk 8 - Alien Attack Fighter", "ALIEN_F.EXE", "ee2eb92900d515ed7872c57f3b89a408", 1754975, 400),
	WINGAME("id4p9", "iD4 Mission Disk 9 - FA-18 Fighter Jet", "F18.EXE", "7a0292909a5103c89297c40cce1d836c", 1691893, 400),
	WINGAME("id4p10", "iD4 Mission Disk 10 - Alien Bomber", "BOMBER.EXE", "17758a9f425f7f3e7a926951e6c770f4", 1844189, 400),
	WINGAME("id4p11", "iD4 Mission Disk 11 - Area 51", "AREA51.EXE", "78be40f9c7e8e1770c388cc16a522aaf", 1776077, 400),

	// Play as either Shirley (disc 1) or Dolan (disc 2)
	{
		{
			"improv", "Dolan Game",
			AD_ENTRY2s("IMPROV (PowerPC)",	"096e0aacbc1670c80a354c3c49b3fabe", 60324,
					   "INTRO.DIR",			"dd667e45e402729581c70f278140f3a5", 3400832),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"improv", "Shirley Game",
			AD_ENTRY2s("IMPROV (PowerPC)",	"096e0aacbc1670c80a354c3c49b3fabe", 60324,
					   "INTRO.DIR",			"62695e977e1a29647e630f70349a3690", 2926976),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"improv", "Dolan Game",
			AD_ENTRY2s("IMPROV.EXE",	"097276acd3caee9bc79b85af79278643", 730079,
					   "INTRO.DIR",		"075c3c7bd7c059280ca86c6b906bf97b", 46366),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"improv", "Shirley Game",
			AD_ENTRY2s("IMPROV.EXE",	"097276acd3caee9bc79b85af79278643", 730079,
					   "INTRO.DIR",		"37f2d3c0b6f805e7a0f3a3ea62f8297c", 44532),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	MACGAME("improv", "Shirley Game", "IMPROV (PowerPC)", "da0da5d543b237051975ad70bec129f4", 483443, 400),
	MACGAME("improv", "Dolan Game", "IMPROV (PowerPC)", "da0da5d543b237051975ad70bec129f4", 483443, 400),
	WINGAME("improv", "Shirley Game", "IMPROV.EXE", "da0da5d543b237051975ad70bec129f4", 483443, 400),
	WINGAME("improv", "Dolan Game", "IMPROV.EXE", "da0da5d543b237051975ad70bec129f4", 483443, 400),

	MACGAME("isis", "", "ISIS", "dedfabf9ee7bf5aab003bdab4137bca6", 777886, 400),
	WINGAME("isis", "", "ISIS.EXE", "59c1cb960199dd3f041262ab5271bb23", 14747213, 400),
	MACGAME_l("isis", "", "ISIS", "dedfabf9ee7bf5aab003bdab4137bca6", 780610, Common::DE_DEU, 400),
	WINGAME_l("isis", "", "ISIS.EXE", "d7f7e89495ac22c75b3fc04c09675b6e", 16750643, Common::DE_DEU, 400),

	// Note: There are four versions of the binary included on the disc.
	// 5.6, 6, and 9 Meg variants all exist too.
	MACGAME("jewels", "", "Jewels 11 Meg", "339c89a148c4ff2c5c815c62ac006325", -1, 400),

	WINGAME("jewels", "", "JEWELS.EXE", "bb6d81471d166088260090472c6c3a87", -1, 400),
	WINDEMO("jewels", "Demo", "JEWELS.EXE", "abcc448c035e88d4edb4a29034fd1e34", -1, 400),

	MACGAME("jman", "Turbo!", "JMP Turbo", "cc3321069072b90f091f220bba16e4d4", -1, 400), // original name is JMP Turbo™
	MACGAME("jman", "Making Of", "The Journey", "cc3321069072b90f091f220bba16e4d4", -1, 400),
	MACDEMO("jman", "Director's Cut Demo", "Journeyman - Director's Cut", "01be45e7241194dad07938e7059b88e3", 484284, 400),

	MACGAME("jman2", "", "Buried in Time PowerPC", "71287376e445ab9c98f0d150bb0ed175", 80334, 400), // original name is Buried in Time™ PowerPC; also contains 68k binary
	MACGAME_l("jman2", "", "Buried in Time PowerPC", "71287376e445ab9c98f0d150bb0ed175", 86324, Common::DE_DEU, 400), // see English notes
	MACDEMO("jman2", "Demo", "Buried in Time Demo", "1ae45c23586b41997ba52e2e7c771c4c", 437743, 400),
	MACDEMO("jman2", "06/22/94 Demo", "Buried in Time Demo", "cdb27c916044ae4dceb4b7326063fa03", 328895, 400),
	MACDEMO("jman2", "Final Demo", "Buried in Time Demo", "cc3321069072b90f091f220bba16e4d4", 292731, 400),
	MACDEMO("jman2", "Gallery", "BIT Gallery", "01be45e7241194dad07938e7059b88e3", 484284, 400),
	MACDEMO("jman2", "Overview", "BIT Overview", "01be45e7241194dad07938e7059b88e3", 484284, 400),

	// Original Taiwan release is called 塔克拉玛干—敦煌传奇 (Taklamakan: Dunhuang Chuanqi)
	MACGAME("karma", "", "Karma", "ea646eccc9a53f44ce082459d4809a06", 485535, 400),
	WINGAME("karma", "", "KARMA.EXE", "e830af6b5dfca4964184e7d61039e120", 697047, 400),
	WINGAME_l("karma", "", "DH.EXE", "f917ac9c649bff9eaf538ae69432a145", 690991, Common::ZH_TWN, 400),

	MACGAME("kyoto", "v2.0", "Cosmology of KYOTO", "8719de9c083aca942fc0e5c0a127b6dc", 484370, 400),
	WINGAME("kyoto", "", "PLAY_E.EXE", "84cc8880f8c46b4da121b4c9f184418a", 691269, 400),
	MACGAME_l("kyoto", "v2.0", "Cosmology of KYOTO", "8719de9c083aca942fc0e5c0a127b6dc", 484445, Common::JA_JPN, 400),
	WINGAME_l("kyoto", "", "PLAY.EXE", "865b1b59e32e51a6670b9105ac8501ec", 741137, Common::JA_JPN, 400),

	MACDEMO("lion", "Demo", "Lion Demo", "01be45e7241194dad07938e7059b88e3", 484284, 400), // full game is DOS only, not Director

	MACGAME("louiscatorze", "", "Louis Catorze", "2c7d0c0d55cbe3f622c7d68cedb671e0", 486954, 400),
	WINGAME("louiscatorze", "", "LOUIS.EXE", "21610856ba0f75598624edd58291600b", 719433, 400),

	{
		{
			// Win/Mac versions are D3

			"lzone", "Pippin",
			AD_ENTRY1s("L-ZONE", "07c3095bd36e5aed00ba12f17f761070", 60352),
			Common::JA_JPN,
			Common::kPlatformMacintosh, //kPlatformPippin
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	MACGAME("majestic", "", "Majestic", "01be45e7241194dad07938e7059b88e3", 483774, 400),
	WINGAME("majestic", "", "MAJESTIC.EXE", "624267f70253e5327981003a6fc0aeba", 23437631, 400),

	MACGAME("martian", "", "The Martian Chronicles PPC CD", "db9b7f11aea52a294d2986a94a184000", 60328, 400),
	WINGAME("martian", "", "MC.EXE", "ee6267d276a18e8bfd6f0e4ecd07ba36", 1764151, 400),

	WINGAME("mediaband", "", "MEDIABND.EXE", "0cfb9b4762e33ab56d656a0eb146a048", 717921, 400),

	WINDEMO("mechwarrior2", "Demo", "MW2DEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("mirage", "", "Mirage", "0c7bbb4b24823e5ab871cb4c1d6f3710", 486954, 400),
	WINGAME("mirage", "", "MIRAGE.EXE", "2291ea234c14f1ccb8f2e67ae09bbfa3", 696867, 400),
	MACGAME("mirage", "Making Of", "The Making of Mirage", "78138a77207907642744d0960c14d9e5", 486982, 400),
	WINGAME("mirage", "Making Of", "MAKINGOF.EXE", "2458926212e5589c013b7e7c28b9bee0", 698407, 400),

	// both Mac and Win versions require installation to extract executable and some data files
	MACGAME("mummy", "", "MUMMY", "01be45e7241194dad07938e7059b88e3", 503560, 400),
	WINGAME("mummy", "", "MUMMY.EXE", "2ebe3a4ef8c128db46794333d4b61c36", 2939630, 400),

	WINDEMO("muppets", "Demo", "MUPPETS.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME_l("murdertaylor", "", "Taylor French", "dc839fb9e2f8e6bfc4f4f995c57df495", 291683, Common::DE_DEU, 400),
	WINGAME_l("murdertaylor", "", "TF.EXE", "d7a97fbe1daf2bc5fdecee2725c297cd", 952741, Common::DE_DEU, 400),

	WINGAME("mylk", "", "mylk.exe", "032b0860a694ece5f64f37cfe1ebdfa0", 904859, 400),

	MACGAME("mysteriousegypt", "", "Mysterious Egypt", "32cb3a881f2d47a7e525370b198b2c5c", 483746, 404),

	MACGAME("necrobius", "", "NCROBIUS.PPC", "ab6ba48edb4a93ca034c63905f615865", 605874, 400),
	WINGAME("necrobius", "", "N_BIUS.EXE", "d82ac6034122e1314157931f9df613f9", 692919, 400),
	WINDEMO("necrobius", "Demo", "N_BIUS.EXE", "d17330706956c2fda3ee482b39b0609f", 705049, 400),

	MACGAME("nile", "", "Nile Passage to Egypt", "9765ad17bce6a2b40dd7f48377e82436", 512990, 400),
	WINGAME("nile", "", "NILE.EXE", "9b00cbba14f0a2e0445784ebacf62c78", 2846777, 400),

	WINDEMO("noir", "Demo", "NOIRDEMO.EXE", "a9690aa58c6da36b7a6bf88fd4c81ffc", 10949309, 400),

	WINGAME("operafatal", "", "OPERA.EXE", "2b9da5566698a1f4b453e1a27e00ad29", 697739, 400),

	{
		{
			"operafatal", "",
			AD_ENTRY2s("OPERA FATAL",	"f5033f237ad1b1639fc46d01a82ac380", 285566,
					   "SPRINT.Dxr",	"3e86f01eeac5fa3349c5177378997a7f", 694912),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	{
		{
			"operafatal", "",
			AD_ENTRY2s("OPERA FATAL",	"f5033f237ad1b1639fc46d01a82ac380", 285566,
					   "SPRINT.Dxr",	"3eb6f5568c9ced258059e2cfd30751c5", 479616),
			Common::IT_ITA,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		400
	},

	MACGAME_l("phantasplanet", "", "phantasmagoria_PPC", "602e61f10c158183218405dd30a09b3f", 60352, Common::JA_JPN, 400),
	WINGAME_l("phantasplanet", "", "PHANTAS.EXE", "c2dd62dd0f9488ae8102970553eff170", 690449, Common::JA_JPN, 400),

	WINDEMO("pitfall", "Demo", "PITDEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("planetarizona", "", "Planet Arizona", "0c7bbb4b24823e5ab871cb4c1d6f3710", 488433, 400),
	WINGAME("planetarizona", "", "ARIZONA.EXE", "a23462ec87eff973d2cdb2ddfd4a59fa", 698651, 400),

	// original file name is Paradise*Rescue
	MACGAME_l("prescue", "", "ParadiseRescue", "bcd3c718db258701496b3c5bcb827ef2", 502965, Common::JA_JPN, 404),
	WINGAME_l("prescue", "", "P_RESCUE.EXE", "c66e73719aff15a2ba4f33cf2e21581a", 813265, Common::JA_JPN, 404),

	// original file name is Japanese characters
	MACGAME_l("sakin2", "", "STARTCD", "67f572196550aedb1f9523d782022be0", 484351, Common::JA_JPN, 404),
	WINGAME_l("sakin2", "", "STARTCD.EXE", "0c8a44b04aa6e8f1483b05f6b567fa3c", 749233, Common::JA_JPN, 404),

	WINDEMO("santafe1", "Demo", "ELKMOON.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("sciencesmart", "", "Science Smart", "e020c33ab426b167647fed5e7aea6659", 295454, 403),
	WINGAME("sciencesmart", "", "SCIENCE.EXE", "23e748085b255870650ae18b15dd986b", 759951, 404),

	WINDEMO("shanghai", "Demo", "SHGMDEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("skyborg", "", "SkyBorg 1.0", "bcd3c718db258701496b3c5bcb827ef2", 486954, 400),
	WINGAME("skyborg", "", "BORG.EXE", "855f305ba8ae70d5e5a53a9085b83d5b", 1887499, 400),

	WINDEMO("spycraft", "Demo", "SPYCRAFT.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is not Director

	MACGAME("superspy", "", "ssi english", "ccf864a8dc6e9d0d26eb73b4683e634b", 60324, 400),
	MACGAME_l("superspy", "", "SSI (Power Mac)", "5f036ea61efdcc5e30074a404dbf4d80", 61030, Common::DE_DEU, 400),
	MACGAME_l("superspy", "", "ssi hebrew", "49aa41e6b624e64e8e48850faa882cc8", 60324, Common::HE_ISR, 400),
	MACGAME_l("superspy", "", "SSI", "8f6a07fe6aa88428dd73557e1cd53d06", 58058, Common::IT_ITA, 400),
	WINGAME("superspy", "", "ssie.exe", "1f256591c24a3bef0875aee737114057", 2181003, 400),
	WINGAME_l("superspy", "", "Ssi.exe", "88272b3e24689c3986f4f4aa8c50e1f7", 2307489, Common::DE_DEU, 400),
	WINGAME_l("superspy", "", "ssih.exe", "f016f23131189b2b65669f5258538fb5", 2261091, Common::HE_ISR, 400),
	WINGAME_l("superspy", "", "SSI.EXE", "890c17a442dd3d96ab331f96d8c1eed8", 3648017, Common::IT_ITA, 400),

	WINGAME("teamxtreme1", "", "XTREME.EXE", "45ab80997efedec388ff5200722e673c", 690123, 400),
	WINGAME("teamxtreme2", "", "TX2.EXE", "50c80bd2add25e574494838772973beb", 2585471, 400),

	// original name is TD MAC/PPC
	MACGAME("totaldistortion", "", "TD MACPPC", "17efee018a660458fae80de4364021ac", 486074, 404),
	WINGAME("totaldistortion", "", "TOTAL_DN.EXE", "461b407c321e80487ae4882056310f9f", 700747, 404),

	MACGAME("trekborg", "", "Picard Dossier", "e1dc28c1dd8409c2d2e0d5d269df5ffc", 520069, 400),
	WINGAME("trekborg", "", "OMNIBORG.EXE", "1580ebad7599fb6c18815e244b0bfda3", 811363, 400),

	MACGAME("trekguideds9", "v1.0", "Omnipedia DS9 Episodes", "1c2e5371b835680e7c1ca8bcea008bef", 505317, 400),
	WINGAME("trekguideds9", "v1.0", "OMNI_DS9.EXE", "0058390ff77e527c7bb413081004c304", 805517, 400),

	MACGAME("trekguidetng", "v1.1", "TNG Episodes", "1c2e5371b835680e7c1ca8bcea008bef", 520149, 400),
	WINGAME("trekguidetng", "v1.1", "OMNI_TNG.EXE", "0049d72e2d5869408fc33860ec4b5c1f", 794201, 400),

	MACGAME("trekklingon", "", "Klingon Language Lab", "b797956eb3a4dabcc15bfadf6fc66591", 481482, 400),
	WINGAME("trekklingon", "", "TREK_WIN.EXE", "204417e63c0a2df18cf42d780b97ed58", 1425763, 400),

	MACGAME("trekomni", "v1.00", "Omnipedia", "b7e69c37b7355022d400c14aa97c5d54", 516791, 400),
	MACGAME("trekomni", "v1.1.2 Upgrade", "Omnipedia 2", "b7e69c37b7355022d400c14aa97c5d54", 613253, 400),
	WINGAME("trekomni", "Premier Edition", "ST_OMNI.EXE", "1a648d47632561f10a98782f1dde5738", 811827, 400),
	WINGAME("trekomni", "v1.1.2 Upgrade", "ST_OMNI2.EXE", "1d60aad73cc9c69776a878aa59e65230", 809781, 400),

	MACGAME("trektech", "", "STNG Technical Manual (PowerPC)", "a9bd9f3a82fcde6e4d151868e256aec7", 525946, 400),
	WINGAME("trektech", "", "NCC1701D.EXE", "02aec408cee26413a096aec9b1cb1878", 720249, 400),

	// original file names are Japanese characters
	MACGAME_l("twistynight1", "", "Twisty Night 1", "8ea9e9a699c61edddffd79ddd0c994d7", 290682, Common::JA_JPN, 402),
	MACGAME_l("twistynight2", "", "Twisty Night 2", "8ea9e9a699c61edddffd79ddd0c994d7", 290682, Common::JA_JPN, 402),
	MACGAME_l("twistynight3", "", "Twisty Night 3", "8ea9e9a699c61edddffd79ddd0c994d7", 290682, Common::JA_JPN, 402),

	WINGAME("ushistory", "", "HISTORY.EXE", "0ca49c80588dd50f985d5aa6192b973e", 703629, 404),

	MACGAME_l("ybr2", "", "YBR2", "b797956eb3a4dabcc15bfadf6fc66591", 504153, Common::JA_JPN, 400),
	WINGAME_l("ybr2", "", "YBR2.EXE", "dbe273c1df60305be98a1a582ddd2c3b", 860579, Common::JA_JPN, 400),
	WINDEMO_l("ybr2", "Demo", "YBR2DEMO.EXE", "25ecc053e02a0ef537d34d615119c814", 900973, Common::JA_JPN, 400),

	WINDEMO("znemesis", "Demo", "ZORKDEMO.EXE", "4a8fd0d74faef305bc935e1aac94d3e8", 712817, 400), // full game is supported in ZVision engine

////////////////////////////////////////
//
// Macromedia Director v5
//
////////////////////////////////////////

	WINGAME("amber", "", "amber_jb.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 500),

	// original file names are ' ANKH2 ' and ' ANKH3 '
	MACGAME_l("ankh2", "", "ANKH2", "779a44f65837ded641cc3fbbebe5ace1", 721432, Common::JA_JPN, 501),
	MACGAME_l("ankh3", "", "ANKH3", "f715a3036c1b54ab1121e90388948018", 720456, Common::JA_JPN, 501),

	WINGAME_l("ernie", "", "Ernie.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1417481, Common::SE_SWE, 500),
	WINDEMO_l("ernie", "Demo", "ERNIE.EXE", "1a7acbba10a7246ba58c1d53fc7203f5", 1417371, Common::SE_SWE, 500),

	WINGAME("gadget", "", "GADGET.EXE", "d62438566e44826960fc16c5c23dbe43", 2212541, 500),  //ADGF_HICOLOR,

	MACGAME("gundam0079", "", "Gundam0079", "4c38a51a21a1ad231f218c4786ff771d", -1, 500),
	WINGAME("gundam0079", "", "Gundam0079.exe", "1a7acbba10a7246ba58c1d53fc7203f5", -1, 500),

	// Win version must be installed in 32-bit Windows to extract executable
	WINGAME_l("horrortour3", "", "HT3.exe", "3460ad87d2ba57104e2810a77b53c220", 5128359, Common::JA_JPN, 500),
	MACGAME_l("horrortour3", "", "HT3", "b545372e9f6e9351d4119af9258d8de0", 108736, Common::JA_JPN, 500),

	{
		{
			// Masters of the Elements - English (from lotharsm)

			"melements", "",
			AD_ENTRY2s("check.dxr", "36f42340e819d1532c850880afe16581", 898206,
					   "Masters of the Elements", 0, 1034962),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		500
	},

	{
		{
			// Masters of the Elements - English (from lotharsm)
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			AD_ENTRY2s("CHECK.DXR", "c31ee30eebd24a8cf31691fc9926daa4", 901820,
					   "MVM.EXE", 0, 2565921),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		500
	},

	{
		{
			// Masters of the Elements - German (from lotharsm)

			"melements", "",
			AD_ENTRY2s("check.dxr", "9c81934b7616ab077f44825b8afaa83e", 575426,
					   "Meister Zufall", 0, 1034962),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		500
	},

	{
		{
			// Masters of the Elements - German (from lotharsm)
			// Released in Germany as "Meister Zufall und die Herrscher der Elemente"
			// Developed by IJsfontein, published by Tivola
			// File version of MVM.EXE is 6.0.2.32
			// The game disc is a hybrid CD-ROM containing both the Windows and the Macintosh release.

			"melements", "",
			AD_ENTRY2s("CHECK.DXR", "d1cd0ed95b0e30597e0089bf3e5caf0f", 575414,
					   "MVM.EXE", 0, 1512503),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GID_GENERIC,
		500
	},

	MACGAME_l("murderbrett", "", "Brad, le surfeur", "227fcce08de8028e2201f5f4eb3b3cc3", 720160, Common::FR_FRA, 500),
	WINGAME_l("murderbrett", "", "BRAD.EXE", "e1c3cb4df694b887f3708f9ebb72302f", 934342, Common::FR_FRA, 500),

	WINGAME("noir", "", "NOIR.EXE", "2e62abdad839e42068afdcd0644d7dcf", 1020879, 500),

	// Use 32-bit Windows executable in SUPPORT/WIN_95
	MACGAME("saillusion", "", "Illusion", "3f3285490a1764d200c9a82041bf4937", 719289, 501),
	MACGAME_l("saillusion", "", "Illusion", "06ed90580400cd9e263525f84c8075fd", 705673, Common::DE_DEU, 500),
	WINGAME("saillusion", "", "ILLUSION.EXE", "1a7acbba10a7246ba58c1d53fc7203f5", 1411175, 501),
	WINGAME_l("saillusion", "", "Illusion.exe", "1c1fa9c5797b8d2cdc04ad283d9e41d7", 1412949, Common::DE_DEU, 501),

	MACGAME("sauniverse", "", "UNIVPPC", "f488a0465587e739b67e53d224449b9a", 94329, 500),
	WINGAME("sauniverse", "", "UNIV32.EXE", "1a7acbba10a7246ba58c1d53fc7203f5", 1468873, 501),

	MACGAME("staytooned", "", "Power Mac Stay Tooned!", "d692140f81aaf86096538f66d6477404", 94301, 500),
	WINGAME("staytooned", "", "START32.EXE", "3460ad87d2ba57104e2810a77b53c220", 1394421, 500),

	MACGAME_l("teddybear", "", "Teddy Bear", "f117b8395c4859d11b9c02c741e22cea", 704793, Common::FR_FRA, 500),
	WINGAME_l("teddybear", "", "BEAR95.EXE", "3460ad87d2ba57104e2810a77b53c220", 1394051, Common::FR_FRA, 500),

	// Both Mac and Windows require HD install
	MACGAME("wishbone", "", "Odyssey", "15a4144726d15488814dab951ec7c3ff", 94329, 500),
	WINGAME("wishbone", "", "ODYSSEY.EXE", "2e62abdad839e42068afdcd0644d7dcf", 1026617, 500),

	MACGAME_l("ybr3", "", "YBRH", "4c38a51a21a1ad231f218c4786ff771d", 105855, Common::JA_JPN, 500),
	MACGAME_l("ybr3", "Single Version", "YBR_Single", "6e7e31d05709e1d38d63f4df6a59eec0", 722547, Common::JA_JPN, 500),
	MACDEMO_l("ybr3", "Demo", "harapeco", "4ab012350c6e1c39eda772e33df30cea", 723903, Common::JA_JPN, 500), // Original name is non-ASCII Japanese characters
	WINGAME_l("ybr3", "", "YBRH.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1410805, Common::JA_JPN, 500), // Original name is non-ASCII Japanese characters
	WINGAME_l("ybr3", "Single Version", "YBRSingl.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1410799, Common::JA_JPN, 500),
	WINDEMO_l("ybr3", "Demo", "harapeco.exe", "1a7acbba10a7246ba58c1d53fc7203f5", 1693107, Common::JA_JPN, 500),

////////////////////////////////////////
//
// Macromedia Director v6
//
////////////////////////////////////////

	MACGAME("trekpedia98", "", "Ency98 Player", "b0b0e36586489ca6070d20feeff52fa5", 1091806, 600),
	WINGAME("trekpedia98", "", "ENCY98.EXE", "35f1550747fcf691283b0f2486144e2b", 1707566, 600),

	// Disc 2 has no director content
	WINGAME("xfua", "", "X-FILES.EXE", "518a98696fe1122e08410b0f157f21bf", 1719387, 602),

////////////////////////////////////////
//
// Macromedia Director v7
//
////////////////////////////////////////

	MACGAME("jewels", "", "Jewels of the Oracle", "fa52f0136cde568a46249ce74f01a324", -1, 700),
	WINGAME("jewels", "", "Jewels.exe", "c1a2e8b7e41fa204009324a9c7db1030", -1, 700),

////////////////////////////////////////
//
// Macromedia Director v8
//
////////////////////////////////////////

	MACGAME_l("operafatal", "Classic OS", "Opera Classic", "78e78f91a6e42ece5a585c3a3dc81de3", 172200, Common::DE_DEU, 800),

////////////////////////////////////////
//
// Macromedia Director MX (v9)
//
////////////////////////////////////////

	MACGAME_l("operafatal", "OS X", "Opera OSX", "3ab7133c17cd4f175e5cb14c6750766e", 222001, Common::DE_DEU, 900),
	WINGAME_l("operafatal", "", "Opera.exe", "05e89a3d53d270cac3b53e0a6f9352f5", 2747652, Common::DE_DEU, 900),

	{ AD_TABLE_END_MARKER, GID_GENERIC, 0 }

};

} // End of namespace Director

#endif
