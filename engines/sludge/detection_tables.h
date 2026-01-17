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

namespace Sludge {

static const PlainGameDescriptor sludgeGames[] = {
	{ "sludge",			"Sludge Game" },
	{ "welcome",		"Welcome Example" },
	{ "verbcoin",		"Verb Coin" },
	{ "verbcoin2",		"Verb Coin 2"},
	{ "parallax",		"Parallax Demo" },
	{ "robinsrescue",	"Robin's Rescue" },
	{ "outoforder",		"Out Of Order" },
	{ "frasse",			"Frasse and the Peas of Kejick" },
	{ "interview",		"The Interview" },
	{ "life",			"Life Flashes By" },
	{ "tgttpoacs",		"The Game That Takes Place on a Cruise Ship" },
	{ "mandy",			"Mandy Christmas Adventure" },
	{ "cubert",			"Cubert Badbone, P.I." },
	{ "gjgagsas",		"The Game Jam Game About Games, Secrets and Stuff" },
	{ "tsotc",			"The Secret of Tremendous Corporation" },
	{ "nsc",			"Nathan's Second Chance" },
	{ "atw",			"Above The Waves" },
	{ "leptonsquest",	"Lepton's Quest" },
	{ "otto",			"Otto Experiment" },
	{ 0, 0 }
};

#define GAME1l(t, e, f1, m1, s1, lang, pl, langId) 	{ { t, e, AD_ENTRY1s(f1, m1, s1), lang, pl, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI) }, langId }
#define GAME1(t, e, f1, m1, s1) GAME1l(t, e, f1, m1, s1, Common::EN_ANY, Common::kPlatformUnknown, 0)
#define GAME2l(t, e, f1, m1, s1, f2, m2, s2, lang, pl, langId) 	{ { t, e, AD_ENTRY2s(f1, m1, s1, f2, m2, s2), lang, pl, ADGF_NO_FLAGS, GUIO1(GUIO_NOMIDI) }, langId }
#define GAME2(t, e, f1, m1, s1, f2, m2, s2) GAME2l(t, e, f1, m1, s1, f2, m2, s2, Common::EN_ANY, Common::kPlatformUnknown, 0)


static const SludgeGameDescription gameDescriptions[] = {
	GAME1("welcome", "", "Welcome.slg", "50445503761cf6684fe3270d0860a4c3", 51736),

	GAME1("verbcoin", "", "Verb Coin.slg", "e39ec315dcbf3a1137481f0a5fe1617d", 980270),
	GAME1l("verbcoin", "", "Verb Coin.slg", "e39ec315dcbf3a1137481f0a5fe1617d", 980270, Common::DE_DEU, Common::kPlatformUnknown, 1),
	GAME1("verbcoin2", "", "Verb Coin.slg", "483b315990309c718617c7c47fa132d8", 1067575),
	GAME1l("verbcoin2", "", "Verb Coin.slg", "483b315990309c718617c7c47fa132d8", 1067575, Common::DE_DEU, Common::kPlatformUnknown, 1),

	GAME1("parallax", "", "Parallax_demo.slg", "daae3f75c6695bed47e5e633cd406a47", 65881),

	GAME1("robinsrescue", "", "robins_rescue.slg", "16cbf2bf916ed89f9c1b14fab133cf96", 14413769),
	GAME1("robinsrescue", "v1.0", "Gamedata.slg", "16cbf2bf916ed89f9c1b14fab133cf96", 14413754),
	GAME1l("robinsrescue", "v1.0", "Gamedata.slg", "16cbf2bf916ed89f9c1b14fab133cf96", 14413754, Common::EN_ANY, Common::kPlatformLinux, 0),

	GAME2l("outoforder", "v1.0", "gamedata",         "4d72dbad0ff170169cd7e4e7e389a90d", 21122647,
								 "out-of-order.xpm", "407086751ac167884c6585c4cad5b664", 2601,
								 Common::EN_ANY, Common::kPlatformLinux, 0),
	GAME2l("outoforder", "v1.0", "gamedata",         "4d72dbad0ff170169cd7e4e7e389a90d", 21122647,	// 2003-02-08
								 "Out Of Order.exe", "064d221103ca4bb7f187432b69c70efd", 68096,
								 Common::EN_ANY, Common::kPlatformWindows, 0),
	GAME2l("outoforder", "v1.0", "gamedata",         "4d72dbad0ff170169cd7e4e7e389a90d", 21122647,	// 2003-06-05
								 "Out Of Order.exe", "ebc386dd0cb77df68dd12d72742eb310", 68608,
								 Common::EN_ANY, Common::kPlatformWindows, 0),

	GAME2("frasse", "v1.03", "gamedata",   "5a985d772f9909a8cc98e1e9edf0875d", 38186227,
							 "frasse.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968),
	GAME2("frasse", "v1.04", "gamedata",   "13934872c16391de3ddd6644e3bfcd15", 38154596,
							 "frasse.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968),
	GAME1l("frasse", "v2.02", "Gamedata.slg", "25e4a63ae10f69f5032c58ad2fd51fac", 88582783, Common::EN_ANY, Common::kPlatformMacintosh, 0),
	GAME1l("frasse", "v2.03", "Gamedata.slg", "e4eb4eca6117bb9b77870bb74af453b4", 88582819, Common::EN_ANY, Common::kPlatformWindows, 0),

	GAME2("interview", "", "gamedata",	    "6ca8f6e44f30d09bd68e008be4c20e8d", 2570140,
					       "interview.exe", "7974f71566c423c3a344862dcbb827dd", 83968),

	GAME1("life", "", "LifeFlashesBy.slg", "a471759e071e5d2c0e8e6887607df778", 163794266),
	GAME2("life", "", "gamedata",    "a471759e071e5d2c0e8e6887607df778", 163794266,
					  "sludge.bmp", "69db99963fb7e93af6d48dfd7f4246ee", 13846),
	GAME2("life", "", "gamedata",            "a471759e071e5d2c0e8e6887607df778", 163794266,
					  "Life Flashes By.exe", "d33c957eefa85defde8b8c29a0bb5a9b", 972800),

	GAME2l("tgttpoacs", "", "gamedata", "d5ec4d7d8440f7744335d25d25e1e943", 40368,
						    "gert.ico", "b76b5b38e8d5cd6843636085947bfd29", 3638,
							Common::EN_ANY, Common::kPlatformWindows, 0),
	GAME1l("tgttpoacs", "", "tgttpoacs.dat", "e61d3d050793689d55487d3ad01b6693", 23817174, Common::EN_ANY, Common::kPlatformLinux, 0),

	GAME2l("mandy", "v1.2", "data",      "df4a0c113b93b89ff2fe7991fb018bae", 7099447,
							"Mandy.exe", "596478e1a7b4445fc0bd7f5ec2696125", 71168,
							Common::SK_SVK, Common::kPlatformWindows, 0),
	GAME2l("mandy", "v1.2", "data",      "df4a0c113b93b89ff2fe7991fb018bae", 7099447,
							"Mandy.exe", "596478e1a7b4445fc0bd7f5ec2696125", 71168,
							Common::EN_ANY, Common::kPlatformWindows, 1),
	GAME2l("mandy", "v1.3", "data",      "b732ffe04367c787c6ce70fbcb7aa6aa", 7100976,
							"Mandy.exe", "596478e1a7b4445fc0bd7f5ec2696125", 71168,
							Common::SK_SVK, Common::kPlatformWindows, 0),
	GAME2l("mandy", "v1.3", "data",      "b732ffe04367c787c6ce70fbcb7aa6aa", 7100976,
							"Mandy.exe", "596478e1a7b4445fc0bd7f5ec2696125", 71168,
							Common::EN_ANY, Common::kPlatformWindows, 1),
	GAME2l("mandy", "v1.3", "data",      "b732ffe04367c787c6ce70fbcb7aa6aa", 7100976,
							"Mandy.exe", "596478e1a7b4445fc0bd7f5ec2696125", 71168,
							Common::IT_ITA, Common::kPlatformWindows, 2),
	GAME2l("mandy", "v1.4", "data",      "705f6ca5f5da0c40c1f547231dd5139f", 7141292,
							"Mandy.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968,
							Common::SK_SVK, Common::kPlatformWindows, 0),
	GAME2l("mandy", "v1.4", "data",      "705f6ca5f5da0c40c1f547231dd5139f", 7141292,
							"Mandy.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968,
							Common::EN_ANY, Common::kPlatformWindows, 1),
	GAME2l("mandy", "v1.4", "data",      "705f6ca5f5da0c40c1f547231dd5139f", 7141292,
							"Mandy.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968,
							Common::IT_ITA, Common::kPlatformWindows, 2),
	GAME2l("mandy", "v1.4", "data",      "705f6ca5f5da0c40c1f547231dd5139f", 7141292,
							"Mandy.exe", "7016ef8ab67133a1d6fce20b8b70cd1d", 83968,
							Common::PL_POL, Common::kPlatformWindows, 3),

	// August 4, 2002
	GAME2l("cubert", "v1.1", "Gamedata",   "dfb7f8012a29631349a14351ba1cfd49", 27303861,
							 "Cubert.exe", "055b5b5c30265ba32e4235b012eb90bb", 64000,
							 Common::EN_ANY, Common::kPlatformWindows, 0),
	// June 11, 2003
	GAME2l("cubert", "v1.1", "Gamedata",		   "dfb7f8012a29631349a14351ba1cfd49", 27303861,
							 "Cubert Badbone.exe", "055b5b5c30265ba32e4235b012eb90bb", 64000,
							 Common::EN_ANY, Common::kPlatformWindows, 0),
	// August 16, 2002
	GAME2l("cubert", "v1.2", "Gamedata",   "245b043e4b2ade16f56118f8d98fb940", 27304149,
							 "Cubert.exe", "055b5b5c30265ba32e4235b012eb90bb", 64000,
							 Common::EN_ANY, Common::kPlatformWindows, 0),
	// September 5, 2002
	GAME2l("cubert", "v1.25", "gamedata",   "d1d9b27d0c43a37952c1bef7bc848623", 27306453,
							  "Cubert.exe", "acc9fb7b4be7e7824a003c88942d778d", 67072,
							  Common::EN_ANY, Common::kPlatformWindows, 0),
	// Mar 9, 2004
	GAME1l("cubert", "", "cubert.dat", "cabc424d4e39ecdba4b0afd4033b5ea8", 19696514, Common::EN_ANY, Common::kPlatformWindows, 0),
	GAME1l("cubert", "", "cubert.dat", "cabc424d4e39ecdba4b0afd4033b5ea8", 19696514, Common::IT_ITA, Common::kPlatformWindows, 1),
	GAME1l("cubert", "", "cubert.dat", "cabc424d4e39ecdba4b0afd4033b5ea8", 19696514, Common::SV_SWE, Common::kPlatformWindows, 2),
	GAME1l("cubert", "", "cubert.dat", "cabc424d4e39ecdba4b0afd4033b5ea8", 19696514, Common::DE_DEU, Common::kPlatformWindows, 3),
	// Dec 15, 2005
	GAME1l("cubert", "", "cubert.dat", "e70050692a0ab96e8753109793157ccd", 19677815, Common::EN_ANY, Common::kPlatformWindows, 0),
	GAME1l("cubert", "", "cubert.dat", "e70050692a0ab96e8753109793157ccd", 19677815, Common::IT_ITA, Common::kPlatformWindows, 1),
	GAME1l("cubert", "", "cubert.dat", "e70050692a0ab96e8753109793157ccd", 19677815, Common::SV_SWE, Common::kPlatformWindows, 2),
	GAME1l("cubert", "", "cubert.dat", "e70050692a0ab96e8753109793157ccd", 19677815, Common::DE_DEU, Common::kPlatformWindows, 3),

	GAME1("nsc",  "v1.03", "gamedata.slg", "57f318cc09e93a1e0685b790a956ebdc", 12733871),
	GAME2l("nsc", "v1.03", "gamedata",                   "57f318cc09e93a1e0685b790a956ebdc", 12733871,
						   "Nathan's Second Chance.exe", "9bb4492fc7f7fc38bc1772bb9f15f787", 71680,
						   Common::EN_ANY, Common::kPlatformWindows, 0),

	GAME1("gjgagsas", "", "gamedata.slg", "f438946f2ee79d52918f44c4a67eb37b", 27527984),

	GAME1("tsotc", "v6", "gamedata.slg", "7d677e79fb842df00c4602864da13829", 34740918),

	GAME1("atw", "", "atw.slg", "41ae22ac9fa5051e0499468a9fbe600e", 27808575),

	GAME1l("leptonsquest", "", "game.slg", "763d4020dcd55a4af4c01664f79584da", 71233234, Common::EN_ANY, Common::kPlatformWindows, 0),
	GAME1l("leptonsquest", "", "Gamedata.slg", "763d4020dcd55a4af4c01664f79584da", 71233239, Common::EN_ANY, Common::kPlatformMacintosh, 0),
	GAME1l("leptonsquest", "", "LeptonsQuest.slg", "763d4020dcd55a4af4c01664f79584da", 71233239, Common::EN_ANY, Common::kPlatformLinux, 0),

	GAME1("otto", "", "gamedata", "c93b1bd849cdb6a23fb8eb389f5c1955", 154427),

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace Wage
