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

namespace Mohawk {

#define GUI_OPTIONS_MYST                   GUIO4(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI)
#define GUI_OPTIONS_MYST_ME                GUIO5(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI, GAMEOPTION_ME)
#define GUI_OPTIONS_MYST_ME_25TH           GUIO6(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI, GAMEOPTION_ME, GAMEOPTION_25TH)
#define GUI_OPTIONS_MYST_DEMO              GUIO5(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GAMEOPTION_DEMO)
#define GUI_OPTIONS_MYST_MAKING_OF         GUIO5(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_NOLAUNCHLOAD)

#define GUI_OPTIONS_RIVEN                  GUIO4(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI)
#define GUI_OPTIONS_RIVEN_25TH             GUIO5(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI, GAMEOPTION_25TH)
#define GUI_OPTIONS_RIVEN_DEMO             GUIO6(GUIO_NOASPECT, GUIO_NOSUBTITLES, GUIO_NOSPEECH, GUIO_NOMIDI, GUIO_NOLAUNCHLOAD, GAMEOPTION_DEMO)

static const MohawkGameDescription gameDescriptions[] = {
	// Myst
	// English Windows 3.11
	// From clone2727
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "ae3258c9c90128d274aa6a790b3ad181"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// English Windows 3.11, v1.0.1
	// From lotharsm
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "4beb3366ed3f3b9bfb6e81a14a43bdcc"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// English Windows 3.11, v1.0
	// From vonLeheCreative, #9645
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "0e4b6fcbd2419d4371365314fb7443f8"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst Demo
	// English Windows 3.11
	// From CD-ROM Today July, 1994
	{
		{
			"myst",
			"Demo",
			AD_ENTRY1("DEMO.DAT", "c39303dd53fb5c4e7f3c23231c606cd0"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_DEMO
		},
		GType_MYST,
		GF_DEMO,
		0,
	},

	// Myst
	// German Windows 3.11
	// From clone2727
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "4beb3366ed3f3b9bfb6e81a14a43bdcc"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// German Windows 3.11
	// From LordHoto
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "e0937cca1ab125e48e30dc3cd5046ddf"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// Spanish Windows ?
	// From jvprat
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "f7e7d7ca69934f1351b5acd4fe4d44c2"),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// Italian Windows ?
	// From goodoldgeorg in bug #6895
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "a5795ce1751fc42525e4f9a1859181d5"),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// Japanese Windows 3.11
	// From clone2727
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "032c88e3b7e8db4ca475e7b7db9a66bb"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst
	// French Windows 3.11
	// From Strangerke
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "d631d42567a941c67c78f2e491f4ea58"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Myst - Russian
	// From AndyILC in bug #10303
	{
		{
			"myst",
			"",
			AD_ENTRY1("myst.dat", "196384f87e8bcb51731bce8416ab6a07"),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0,
	},

	// Making of Myst
	// English Windows 3.11
	// From clone2727
	{
		{
			"makingofmyst",
			"",
			AD_ENTRY1("making.dat", "f6387e8f0f7b8a3e42c95294315d6a0e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_MAKING_OF
		},
		GType_MAKINGOF,
		0,
		0,
	},

	// Making of Myst
	// Japanese Windows 3.11
	// From clone2727
	{
		{
			"makingofmyst",
			"",
			AD_ENTRY1("making.dat", "03ff62607e64419ab2b6ebf7b7bcdf63"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_MAKING_OF
		},
		GType_MAKINGOF,
		0,
		0,
	},

	// Myst Masterpiece Edition
	// English Windows
	// From clone2727
	{
		{
			"myst",
			"Masterpiece Edition",
			AD_ENTRY1("myst.dat", "c4cae9f143b5947262e6cb2397e1617e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME
		},
		GType_MYST,
		GF_ME,
		0,
	},

	// Myst Masterpiece Edition
	// German Windows
	// From DrMcCoy (Included in "Myst: Die Trilogie")
	{
		{
			"myst",
			"Masterpiece Edition",
			AD_ENTRY1("myst.dat", "f88e0ace66dbca78eebdaaa1d3314ceb"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME
		},
		GType_MYST,
		GF_ME,
		0,
	},

	// Myst Masterpiece Edition
	// French Windows
	// From gamin (Included in "Myst: La Trilogie")
	{
		{
			"myst",
			"Masterpiece Edition",
			AD_ENTRY1("myst.dat", "aea81633b2d2ae498f09072fb87263b6"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME
		},
		GType_MYST,
		GF_ME,
		0,
	},

	// Myst Masterpiece Edition
	// Polish Windows
	// From pykman (Included in "Myst: Antologia")
	{
		{
			"myst",
			"Masterpiece Edition",
			AD_ENTRY1("myst.dat", "4a05771b60f4a69869838d01e85c9e80"),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME
		},
		GType_MYST,
		GF_ME,
		0,
	},

	// Myst Masterpiece Edition - 25th Anniversary
	// Repacked by the ScummVM team
	{
		{
			"myst",
			"Masterpiece Edition - 25th Anniversary",
			{
				{"myst.dat", 0, "c4cae9f143b5947262e6cb2397e1617e", -1},
				{"myst_french.dat", 0, "7c8230be50ffcac588e7db8788ad7614", -1},
				{"myst_german.dat", 0, "3952554439960b22a360e8e006dfed58", -1},
				{"myst_polish.dat", 0, "9ca82ff26fcbfacf40e4164523a50854", -1},
				{"myst_spanish.dat", 0, "822ed3c0de912c10b877dcd2cc078493", -1},
				{"menu.dat", 0, "7dc23051084f79b1c2bccc84cdec0503", -1},
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME_25TH
		},
		GType_MYST,
		GF_ME | GF_25TH,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0 (5CD)
	// From clone2727
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "71145fdecbd68a0cfc292c2fbddf8e08"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.03 (5CD)
	// From ST
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "d8ccae34a0e3c709135a73f449b783be"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.? (5CD) - Spanish
	// From jvprat
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "249e8c995d191b03ee94c892c0eac775"),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0 (5CD), 1.02 (DVD, From "Myst: La Trilogie")
	// From gamin
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "aff2a384aaa9a0e0ec51010f708c5c04"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0 (5CD) - Italian
	// From dodomorandi on bug #6629
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "0e21e89df7788f32056b6521abf2e81a"),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0.0 (5CD) - Russian, Fargus
	{
		{
			"riven",
			"",
			AD_ENTRY1s("a_Data.MHK", "2a840ed74fe5dc3a388bced674d379d5", 12024358),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.1 (5CD) - Russian, Fargus
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "59bd2e3ccbae2f1faa1b23a18dc316eb"),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0J (5CD) - Japanese
	// From sev
	{
		{
			"riven",
			"",
			AD_ENTRY1s("a_Data.MHK", "3a2b4764979dc007a0e6ded64e4b7889", 10014314),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0 (5CD) - Brazilian
	// from trembyle
	{
		{
			"riven",
			"",
			AD_ENTRY1s("a_Data.MHK", "49fe3003d6da02b03c5c1f6028a7f719", 9892372),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.? (DVD, From "Myst 10th Anniversary Edition")
	// From Clone2727
	{
		{
			"riven",
			"DVD",
			AD_ENTRY1("a_Data.MHK", "08fcaa5d5a2a01d7a5a6960f497212fe"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		GF_DVD,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.1 (DVD, Pressing code rvd 2811 ab, RVD8AB-BI RVD2811AB)
	// From wouwehand in #10519
	{
		{
			"riven",
			"DVD",
			AD_ENTRY1("a_Data.MHK", "3370cd9a9696814365a2b7fd7a7b726e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		GF_DVD,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.0 (DVD, From "Myst: Die Trilogie")
	// From DrMcCoy
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "a5fe1c91a6033eb6ee54b287578b74b9"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.02 (DVD, From "Myst: Antologia")
	// From pykman
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "733a710cf5f848b441ec72d988ab8a3d"),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.1 (DVD), Russian, Fargus
	{
		{
			"riven",
			"DVD",
			AD_ENTRY1("a_Data.MHK", "b5f40e6e6b843bf3abea291faa0911f4"),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		GF_DVD,
		0,
	},

	// Riven: The Sequel to Myst - 25th anniversary
	// Created by the ScummVM team
	{
		{
			"riven",
			"25th Anniversary",
			{
				{ "a_data.mhk",          0, "08fcaa5d5a2a01d7a5a6960f497212fe", 10218888 },
				{ "a_data_french.mhk",   0, "ad7547ed7159a97be98a005f62862f85", 7088579  },
				{ "a_data_german.mhk",   0, "5ebd301bd4bf6fd7667c4a46eebf6532", 7098655  },
				{ "a_data_italian.mhk",  0, "9d53b178510ce90f10b32ad3ca967d38", 6677740  },
				{ "a_data_japanese.mhk", 0, "bf43cf8af21fefc5a02881f7cfb68f52", 7237370  },
				{ "a_data_polish.mhk",   0, "5c7cd4b1a1a4c63cc670485816b0b5ec", 14588293 },
				{ "a_data_russian.mhk",  0, "76e12906637f5274bb6af8ab42871c25", 14349136 },
				{ "a_data_spanish.mhk",  0, "6226a3e1748e64962971b2f6536ef283", 8133297  },
				AD_LISTEND
			},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUI_OPTIONS_RIVEN_25TH
		},
		GType_RIVEN,
		GF_DVD | GF_25TH,
		0,
	},

	// Riven: The Sequel to Myst
	// Version ? (Demo, From "Prince of Persia Collector's Edition")
	// From Clone2727
	{
		{
			"riven",
			"Demo",
			AD_ENTRY1("a_Data.MHK", "bae6b03bd8d6eb350d35fd13f0e3139f"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			(ADGF_DEMO | ADGF_DROPPLATFORM),
			GUI_OPTIONS_RIVEN_DEMO
		},
		GType_RIVEN,
		GF_DEMO,
		0,
	},

	{
		{
			"cstime",
			"",
			AD_ENTRY1("signin.mhk", "410b4ce8d1a8702971e4d1ffba9b965d"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_CSTIME,
		0,
		0
	},

	{
		{
			"cstime",
			"Demo",
			AD_ENTRY1("iface.mhk", "5c1203712a16513bd158dc3c1b6cebd7"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_CSTIME,
		GF_DEMO,
		0
	},

	// Harry and the Haunted House v1.0E
	// English Windows 3.11
	// From strangerke
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("HHHB.LB", "267bb6e3c8f237ca98b02c07b9c4013f"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Harry and the Haunted House v1.0E
	// French Windows 3.11
	// From strangerke
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("HHHF.LB", "7e5da86f19935bdf8fa89bbd39446543"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Harry and the Haunted House v1.0E
	// German Windows 3.11
	// From strangerke
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("HHHD.LB", "85c0a816efeb679739158789befb2be8"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("EnglishBO", "b63a7b67834de0cd4cdbf02cf40d8547"),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("GermanBO", "eb740102c1c8379c2c610cba14484ccb"),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("FrenchBO", "2118de914ab9eaec482c245c06145071"),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Harry and the Haunted House 1.1
	// From pacifist
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("HARRY.512", "8d786f0998f27e44603a2202d6786c25"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"HARRY.EXE"
	},

	// part of "Super Living Books" compilation
	// from rgemini, bug #3309343
	{
		{
			"harryhh",
			"",
			AD_ENTRY1("HARRY.512", "39d11399796dfa36d3f631d2d87e8b85"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"HARRY.EXE"
	},

	{
		{
			"carmentq",
			"",
			AD_ENTRY1("Outline.txt", "67abce5dcda969c23f367a98c90439bc"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV5,
		0,
		0
	},

	{
		{
			"carmentqc",
			"",
			AD_ENTRY1("Outline.txt", "6a281eefe72987afb0f8fb6cf84553f5"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV5,
		0,
		0
	},

	// From afholman in bug #3309306
	{
		{
			"lbsampler",
			"v3",
			AD_ENTRY1("outline", "8397cea6bed1ff90029f7602ef37684d"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		"Living Books Sampler"
	},

	{
		{
			"maggiesfa",
			"",
			AD_ENTRY1("Outline", "b7dc6e65fa9e80784a5bb8b557aa37c4"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"maggiesfa",
			"",
			AD_ENTRY1("BookOutline", "1ce006d7daaa26cf61040203856b88f1"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"greeneggs",
			"",
			AD_ENTRY1("GREEN.LB", "5df8438138186f89e71299d7b4f88d06"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// 32-bit version of the previous entry
	{
		{
			"greeneggs",
			"",
			AD_ENTRY1("GREEN32.LB", "5df8438138186f89e71299d7b4f88d06"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"greeneggs",
			"",
			AD_ENTRY1("BookOutline", "5500fa72a6d112b4b3d3573b26a31820"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"seussabc",
			"",
			AD_ENTRY1("ABC.LB", "1d56a9351974a7a70ace5274a4570b72"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// 32-bit version of the previous entry
	{
		{
			"seussabc",
			"",
			AD_ENTRY1("ABC32.LB", "1d56a9351974a7a70ace5274a4570b72"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	{
		{
			"seussabc",
			"",
			AD_ENTRY1("BookOutline", "64fbc7a3519de7db3f8c7ff650921eea"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	{
		{
			"seussabc",
			"Demo",
			AD_ENTRY1("BookOutline", "17d72660680ae32cd7c560d0cf04d2ef"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		GF_DEMO,
		0
	},

	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTOISE.512", "dfcf7bff3d0f187832c9897497efde0e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"TORTOISE.EXE"
	},

	// part of "Super Living Books" compilation
	// from rgemini, bug #3309343
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTOISE.512", "e9ec7a6bc6b451c9e85e5b4f072d5143"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"TORTOISE.EXE"
	},

	// From afholman in bug#3309308
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTB.LB", "83f6bfcf30c445d13e81e0faed9aa27b"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTB.LB", "9a80b66e7d95c7f59bdfd2c280e03e6e"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// Tortoise and the Hare Hebrew variant - From georgeqgreg on bug #3441928
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTB.LB", "23135777370cf1ff00aa7247e93642d3"),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From afholman in bug#3309308
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTD.LB", "21761e7de4e5f12298f43fa17c00f3e1"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTD.LB", "5d4d830116fe965ee35e328db85743b7"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From afholman in bug#3309308
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTF.LB", "9693043df217ffc0667a1f45f2849aa7"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("TORTF.LB", "89f23bc3a1d1797bfe07dd0eaa7c13c4"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From the Mac CD "La Tartaruga e la Lepre" (The Turtle and the Hare) [Italian, English]
	{
		{
			"tortoise",
			"",
			AD_ENTRY1("BookOutline", "82d0b24a6400bec8e94cde021a4c876b"),
			Common::IT_ITA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		"La Tartaruga e la Lepre"
	},

	{
		{
			"tortoise",
			"Demo v1.0",
			AD_ENTRY1("TORTOISE.512", "75d9a2f8339e423604a0c6e8177600a6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"TORTOISE.EXE"
	},

	{
		{
			"tortoise",
			"Demo v1.1",
			AD_ENTRY1("TORTOISE.512", "a38c99360e2bea3bfdec418469aef022"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"TORTOISE.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"tortoise",
			"Demo v1.1",
			AD_ENTRY1("TORTOISE.512", "14400a3358a3f1148e4d4b47bc3523c9"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"TORTOISE.EXE"
	},

	{
		{
			"tortoise",
			"Demo",
			AD_ENTRY1("The Tortoise and the Hare Demo", "35d571806838667743c7c15a133e9335"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	{
		{
			"arthur",
			"",
			AD_ENTRY1("PAGES.512", "1550a361454ec452fe7d2328aac2003c"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"ARTHUR.EXE"
	},

	// part of "Super Living Books" compilation
	// from rgemini, bug #3309343
	{
		{
			"arthur",
			"",
			AD_ENTRY1("PAGES.512", "cd995d20d0d7b4642476fd76044b4e5b"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"ARTHUR.EXE"
	},

	// From afholman in bug#3309308
	{
		{
			"arthur",
			"",
			AD_ENTRY1("BookOutline", "133750de1ceb9e7351599d79f99fee4d"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"Arthur's Teacher Trouble"
	},

	// From darthbo in bug#3301791
	{
		{
			"arthur",
			"",
			AD_ENTRY1("PAGES.512", "cd995d20d0d7b4642476fd76044b4e5b"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"ARTHUR.EXE"
	},

	{
		{
			"arthur",
			"Demo",
			AD_ENTRY1("PAGES.512", "a4d68cef197af1416921ca5b2e0c1e31"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO | GF_LB_10,
		"ARTHUR.EXE"
	},

	{
		{
			"arthur",
			"Demo v1.1",
			AD_ENTRY1("ARTHUR.512", "f19e824e0a2f2745ed698e6aaf44f838"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"ARTHUR.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"arthur",
			"Demo v1.1",
			AD_ENTRY1("ARTHUR.512", "dabdd466dea26ab5ecb9415cf73f8601"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"ARTHUR.EXE"
	},

	{
		{
			"arthur",
			"Demo",
			AD_ENTRY1("Bookoutline", "7e2691611ff4c7b89c05221736628059"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO | GF_LB_10,
		"Arthur's Teacher Trouble"
	},

	{
		{
			"arthur",
			"Demo",
			AD_ENTRY1("Arthur's Teacher Trouble Demo", "dcbd8af6bf25854df8ad36fd13665d08"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	// Just Grandma and Me 2.0
	// From pacifist
	{
		{
			"grandma",
			"v2.0",
			AD_ENTRY1("OUTLINE", "159c18b663c58d1aa17ad5e1ab1f0e12"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// Just Grandma and Me
	// From bug Trac #6745
	{
		{
			"grandma",
			"v1.3.0.5",
			AD_ENTRY1("outline", "33074daec5263ba209abcce3ee60ee38"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// Just Grandma and Me 2.0 Macintosh
	// From aluff in bug #3461368
	{
		{
			"grandma",
			"v2.0",
			AD_ENTRY1("BookOutline", "99fe5c8ace79f0542e6390bc3b58f25a"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// Just Grandma and Me 1.0
	// From scoriae
	{
		{
			"grandma",
			"v1.0",
			AD_ENTRY1("PAGES.512", "e694ac10f957dd2e20611350bf968da3"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"GRANDMA.EXE"
	},

	// Just Grandma and Me 1.0, Macintosh
	{
		{
			"grandma",
			"v1.0",
			AD_ENTRY1("BookOutline", "9162483da06179e76f4a082412245efa"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		0
	},

	// Just Grandma and Me 1.1 Mac
	// From eisnerguy1 in bug#3610725
	{
		{
			"grandma",
			"v1.1",
			AD_ENTRY1("BookOutline", "76eb265ec5fe42bc5b07f2bb418bd871"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		0
	},

	// from jjnryan in bug #3389857
	{
		{
			"grandma",
			"v1.0",
			AD_ENTRY1("PAGES.512", "613ca946bc8d91087fb7c10e9b84e88b"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_LB_10,
		"GRANDMA.EXE"
	},

	// From Torsten in bug#3422652
	{
		{
			"grandma",
			"",
			AD_ENTRY1("JGMB.LB", "400b68a6f23b88ca23a01cfd11c6ef9f"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"grandma",
			"",
			AD_ENTRY1("JGMD.LB", "c4fbfd73f805a2266cf9e15caa1b7462"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"grandma",
			"",
			AD_ENTRY1("JGMF.LB", "0c3305e109a027f8b62a6bd8c5d2addb"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	{
		{
			"grandma",
			"Demo v1.0",
			AD_ENTRY1("PAGES.512", "95d9f4b035bf5d15c57a9189f231b0f8"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO | GF_LB_10,
		"GRANDMA.EXE"
	},

	{
		{
			"grandma",
			"Demo v1.1",
			AD_ENTRY1("GRANDMA.512", "72a4d5fb1b3f06b5f75425635d42ce2e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"GRANDMA.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"grandma",
			"Demo v1.1",
			AD_ENTRY1("GRANDMA.512", "4f616647245bb4e37e6dab7557dad304"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"GRANDMA.EXE"
	},

	// From the Mac CD "La Tartaruga e la Lepre" (The Turtle and the Hare) [Italian, English]
	{
		{
			"grandma",
			"Demo v1.2",
			AD_ENTRY1("BookOutline", "599fece5d71cdd19c9726c28dbf634d8"),
			Common::IT_ITA,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		GF_DEMO,
		"Al mare con la Nonna"
	},

	{
		{
			"grandma",
			"Demo",
			AD_ENTRY1("Bookoutline", "553c93891b9631d1e1d269599e1efa6c"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO | GF_LB_10,
		"Just Grandma and Me"
	},

	{
		{
			"grandma",
			"Demo",
			AD_ENTRY1("Just Grandma and Me Demo", "552d8729fa77a4a83c88283c7d79bd31"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	// Ruff's Bone 1.0
	// From pacifist
	{
		{
			"ruff",
			"",
			AD_ENTRY1("RUFF.512", "3dbda0de6f47a64d1714d89f5a5f60d1"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"RUFF.EXE"
	},

	// From aluff in bug#3307785
	{
		{
			"ruff",
			"",
			AD_ENTRY1("BookOutline", "f625d4056c750b9aad6f94dd854f5abe"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Living Books Player"
	},

	{
		{
			"ruff",
			"Demo",
			AD_ENTRY1("RUFF.512", "2ba1aa65177c816e156db648c398d362"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"RUFF.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"ruff",
			"Demo",
			AD_ENTRY1("RUFF.512", "07b9d013e2400d61ca268892a76de4d2"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"RUFF.EXE"
	},

	{
		{
			"ruff",
			"Demo",
			AD_ENTRY1("Ruff's Bone Demo", "22553ac2ceb2a166bdf1def6ad348532"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	// From aluff in bug#3309981
	{
		{
			"newkid",
			"",
			AD_ENTRY1("NEWKID.512", "5135f24afa138ecdf5b52d955e9a9189"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"NEWKID.EXE"
	},

	// part of "Super Living Books" compilation
	// from rgemini, bug #3309343
	{
		{
			"newkid",
			"",
			AD_ENTRY1("NEWKID.512", "28a5aef3e6ef7e2ed7742485c25bdff6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"NEWKID.EXE"
	},

	// From aluff in bug#3309981
	{
		{
			"newkid",
			"",
			AD_ENTRY1("BookOutline", "6aa7c4720b922f4164584956be5ba9e5"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Living Books Player"
	},

	{
		{
			"newkid",
			"Demo v1.0",
			AD_ENTRY1("NEWKID.512", "2b9d94763a50d514c04a3af488934f73"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"NEWKID.EXE"
	},

	{
		{
			"newkid",
			"Demo v1.1",
			AD_ENTRY1("NEWKID.512", "41e975b7390c626f8d1058a34f9d9b2e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"NEWKID.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"newkid",
			"Demo v1.1",
			AD_ENTRY1("NEWKID.512", "de576f3481f62e84eda03b4d2307492b"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"NEWKID.EXE"
	},

	{
		{
			"newkid",
			"Demo",
			AD_ENTRY1("The New Kid on the Block Demo", "7d33237e0ea452a97f2a3acdfb9e1286"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	{
		{
			"arthurrace",
			"",
			AD_ENTRY1("RACE.LB", "1645f36bcb36e440d928e920aa48c373"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// 32-bit version of the previous entry
	{
		{
			"arthurrace",
			"",
			AD_ENTRY1("RACE32.LB", "292a05bc48c1dd9583821a4181a02ef2"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"arthurrace",
			"",
			AD_ENTRY1("BookOutline", "f0a9251824a648fce1b49cb7c1a0ba67"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// From zerep in bug #3287894
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("BIRTHDAY.512", "874f80ff363214d63593864e58c4a130"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"BIRTHDAY.EXE"
	},

	// From Maikel-Nait in bug Trac #10705
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("BIRTHDAY.512", "fd0c7b73b9f40400e145efd06240b84b"),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_ADDENGLISH,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"BIRTHDAY.EXE"
	},

	// From aluff in bug#3309936
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("BookOutline", "d631242b004720ecc615e4f855825860"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Living Books Player"
	},

	// From Matthew Winder in bug#6557
	// v1.0E, English, Windows
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1s("AB16B.LB", "c169be346de7b0bbfcd18761fc0a3e49", 3093),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("AB16B.LB", "54223967c507a48e572902802650412f"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("AB16D.LB", "1154bfdb9fe8c42ad8692e68071ec175"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// From Torsten in bug#3422652
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("AB16F.LB", "60f439aa56c22a400bda27c9574115a9"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0,
	},

	// Arthur Birthday (English) Version 2.0 Windows(R) August 8, 1997
	// From jacecen in bug #3413119
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("Outline", "3b793adf2b303722e0fb6c632f94e1fb"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// Arthur Birthday (English) Version 2.0 Macintosh
	// From aluff in bug #3461368
	{
		{
			"arthurbday",
			"",
			AD_ENTRY1("BookOutline", "8e4fddb5b761c8cf2a3b448dd38422be"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"arthurbday",
			"Demo",
			AD_ENTRY1("BIRTHDAY.512", "fb73e387cfec65c5c930db068a8f468a"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"BIRTHDAY.EXE"
	},

	// From Scarlatti in bug #3275626
	{
		{
			"arthurbday",
			"Demo",
			AD_ENTRY1("BIRTHDAY.512", "2946b1e06f59ea607b8b29dfc6ba8976"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"BIRTHDAY.EXE"
	},

	{
		{
			"arthurbday",
			"Demo",
			AD_ENTRY1("Arthur's Birthday Demo", "0d974ec635eea615475368e865f1b1c8"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"Living Books Player"
	},

	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("MONSTER.512", "e7b24bf8f59106b5c4df51b39eb8c0ef"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"MONSTER.EXE"
	},

	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("BookOutline", "970409f9d967d63c05e63113f8e78fe2"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Little Monster at School"
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("lmasb.lb", "18a4e82f2c5cc30f7a2f9bd95e8c1364"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("lmasd.lb", "422b94c0e663305869cb2d2f1109a0bc"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("lmasf.lb", "8c22e79c97a86827d56b4c596066dcea"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("EnglishBO", "7aa2a1694255000b72ff0cc179f8059f"),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From Matthew Winder in bug#6557
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1s("lmasf.lb", "fcb665df1713d0411a41515efb20bebc", 4136),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("GermanBO", "ff7ac4b1b4f2ded71ff3650f383fea48"),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"lilmonster",
			"",
			AD_ENTRY1("FrenchBO", "d13e5eae0f68cecc91a0dcfcceec7061"),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From Scarlatti in bug #3275626
	{
		{
			"lilmonster",
			"Demo",
			AD_ENTRY1("MONSTER.512", "029e57f1fc8dd1f93f6623a1841f0df2"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"MONSTER.EXE"
	},

	// From GeorgeQGreg
	{
		{
			"lilmonster",
			"Demo",
			AD_ENTRY1("MONSTER.512", "f603f04c1824d1034ec0366416a059c9"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"MONSTER.EXE"
	},

	{
		{
			"catinthehat",
			"",
			AD_ENTRY1("Outline", "0b5ab6dd7c08cf23066efa709fa48bbc"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"catinthehat",
			"",
			AD_ENTRY1("BookOutline", "e139903eee98f0b0c3f39247a23b8f10"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"rugrats",
			"",
			AD_ENTRY1("outline", "525be248363fe27d50d750632c1e759e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	{
		{
			"rugrats",
			"",
			AD_ENTRY1("BookOutline", "54a324ee6f8260258bff7043a05b0004"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	// Rugrats Adventure Game
	// French Windows
	// From Strangerke
	{
		{
			"rugrats",
			"",
			AD_ENTRY1("outline", "36225e0b4986a80135cfdd9643cc7030"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	// Rugrats Adventure Game
	// English Windows Demo
	// From GeorgeQGreg (Rugrats Movie Soundtrack)
	{
		{
			"rugrats",
			"Demo",
			AD_ENTRY1("outline", "adbd7ff6c5e1bdb7062c89879a4e39e6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	{
		{
			"lbsampler",
			"v1",
			AD_ENTRY1("DEMO.512", "27c3455b850e5981d2bf2a3fa489b91e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"DEMO.EXE"
	},

	{
		{
			"lbsampler",
			"v1",
			AD_ENTRY1("BookOutline", "d30e6026a9809dea579030bac380977d"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Living Books Demos"
	},

	{
		{
			"lbsampler",
			"v2",
			AD_ENTRY1("SAMPLER.LB", "6c77cba7326f975645d55fd17ad02d92"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// While this entry has a v2 interpreter, it still has v1 data
	{
		{
			"lbsampler",
			"v2",
			AD_ENTRY1("BookOutline", "48985306013164b128981883045f2c43"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Living Books Sampler"
	},

	{
		{
			"lbsampler",
			"v3",
			AD_ENTRY1("outline", "d239506f969ff68fa886f084082e9158"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0,
	},

	{
		{
			"lbsampler",
			"v3",
			AD_ENTRY1("BookOutline", "6dd1c0606f1db3b71207121b4370e487"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0,
	},

	{
		{
			"bearfight",
			"",
			AD_ENTRY1("FIGHT.512", "f6e6c77a216c400a69388c94a11766d2"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"FIGHT.EXE"
	},

	// The Berenstain Bears Get in a Fight (English and Spanish) Version 1.0 1995
	// From jacecen in bug #3413119
	{
		{
			"bearfight",
			"",
			AD_ENTRY1("FIGHT.512", "e313242a4cba2fffcd8cded5ca23c68a"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"FIGHT.EXE"
	},

	{
		{
			"bearfight",
			"",
			AD_ENTRY1("BookOutline", "a1c395a5234f88613d4110ca36df4808"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV1,
		0,
		"Bears Get in a Fight"
	},

	// From bkennimer in bug #3284930
	{
		{
			"beardark",
			"",
			AD_ENTRY1("DARK.LB", "81d1e6eaf88d54bd29836a133935c0d4"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// From bkennimer in bug #3284930
	// 32-bit version of the previous entry
	{
		{
			"beardark",
			"",
			AD_ENTRY1("DARK32.LB", "28abbf5498aeb29e78e5e0dec969ebe2"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// From bkennimer in bug #3284930
	{
		{
			"beardark",
			"",
			AD_ENTRY1("BookOutline", "95b2e43778ca9cfaee37bdde843e7681"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	// From aluff in bug #3461368
	{
		{
			"beardark",
			"",
			AD_ENTRY1("BookOutline", "b56746b3b2c062c8588bfb6b28e137c1"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"arthurcomp",
			"",
			AD_ENTRY1("OUTLINE", "dec4d1a05449f81b6012706932658326"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	{
		{
			"arthurcomp",
			"",
			AD_ENTRY1("BookOutline", "87bf1f9113340ce1c6c880932e815882"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	// Stellaluna 1.0
	// From pacifist
	{
		{
			"stellaluna",
			"",
			AD_ENTRY1("STELLA.LB", "763bb4a4721aebb5af316ca8e1b478ed"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Stellaluna 1.0
	// From bug Trac #6745
	{
		{
			"stellaluna",
			"",
			AD_ENTRY1("STELLA.LB", "ff8ae44b52fb19f039e82730f6a1bb51"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From aluff in bug#3306722
	{
		{
			"stellaluna",
			"",
			AD_ENTRY1("STELLA.LB", "ca8562a79f63485680e21191f5865fd7"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From aluff in bug#3306722
	{
		{
			"stellaluna",
			"",
			AD_ENTRY1("BookOutline", "7e931a455ac88557e04ca682579cd5a5"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Sheila Rae the Brave 1.0
	// From pacifist
	{
		{
			"sheila",
			"",
			AD_ENTRY1("SHEILA.LB", "c28a60f615a46384d9a8941fc5c89d63"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From aluff in bug#3309934
	{
		{
			"sheila",
			"",
			AD_ENTRY1("BookOutline", "961f0cf4de2fbaa1da8ce0011822cd38"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("SRAEB.LB", "4835612022c2ae1944bde453d3202803"),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("SRAED.LB", "3f21183534d324cf3bb8464f9217712c"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("SRAEF.LB", "96b00fc4b44c0e881c674d4bae5aa79a"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("EnglishBO", "6d3ad5724f1729a1d96d812668770c2e"),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("GermanBO", "af1dc5a8bc8da58310d17b72b657fc1f"),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// From afholman in bug#3309308
	{
		{
			"sheila",
			"",
			AD_ENTRY1("FrenchBO", "62eefcb8424a5f9ba7db5af6f0421e58"),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	// Rugrats Print Shop
	// English Windows Demo
	// From GeorgeQGreg (Rugrats Movie Soundtrack)
	{
		{
			"rugratsps",
			"Demo",
			AD_ENTRY1("outline", "808d5ee8427180ddebdd5dd4199b47cb"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GType_LIVINGBOOKSV4,
		0,
		0
	},

	{ AD_TABLE_END_MARKER, 0, 0, 0 }
};

//////////////////////////////
//Fallback detection
//////////////////////////////

static const MohawkGameDescription fallbackDescs[] = {
	{
		{
			"myst",
			"unknown",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST
		},
		GType_MYST,
		0,
		0
	},

	{
		{
			"makingofmyst",
			"unknown",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_MAKING_OF
		},
		GType_MAKINGOF,
		0,
		0
	},

	{
		{
			"myst",
			"unknown (Masterpiece Edition)",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_MYST_ME
		},
		GType_MYST,
		GF_ME,
		0
	},

	{
		{
			"riven",
			"unknown",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		0,
		0
	},

	{
		{
			"riven",
			"unknown (DVD)",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_RIVEN
		},
		GType_RIVEN,
		GF_DVD,
		0
	}
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[0].desc,  { "myst.dat", 0 } },
	{ &fallbackDescs[1].desc,  { "making.dat", 0 } },
	{ &fallbackDescs[2].desc,  { "myst.dat", "help.dat", 0 } },	// Help system doesn't exist in original
	{ &fallbackDescs[3].desc,  { "a_Data.MHK", 0 } },
	{ &fallbackDescs[4].desc,  { "a_Data.MHK", "t_Data1.MHK" , 0 } },
	{ 0, { 0 } }
};

} // End of Namespace Mohawk
