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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

namespace Mohawk {

static const MohawkGameDescription gameDescriptions[] = {
	// Myst
	// English Windows 3.11
	// From clone2727
	{
		{
			"myst",
			"",
			AD_ENTRY1("MYST.DAT", "ae3258c9c90128d274aa6a790b3ad181"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_DEMO,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "4beb3366ed3f3b9bfb6e81a14a43bdcc"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "e0937cca1ab125e48e30dc3cd5046ddf"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "f7e7d7ca69934f1351b5acd4fe4d44c2"),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "032c88e3b7e8db4ca475e7b7db9a66bb"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "d631d42567a941c67c78f2e491f4ea58"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			"MakingOfMyst",
			"",
			AD_ENTRY1("MAKING.DAT", "f6387e8f0f7b8a3e42c95294315d6a0e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			"MakingOfMyst",
			"",
			AD_ENTRY1("MAKING.DAT", "03ff62607e64419ab2b6ebf7b7bcdf63"),
			Common::JA_JPN,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "c4cae9f143b5947262e6cb2397e1617e"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_MYST,
		GF_ME,
		0,
	},

	// Myst Masterpiece Edition
	// English Windows
	// From clone2727
	{
		{
			"myst",
			"Masterpiece Edition",
			AD_ENTRY1("MYST.DAT", "c4cae9f143b5947262e6cb2397e1617e"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "f88e0ace66dbca78eebdaaa1d3314ceb"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			AD_ENTRY1("MYST.DAT", "aea81633b2d2ae498f09072fb87263b6"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_MYST,
		GF_ME,
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_RIVEN,
		0,
		0,
	},

	// Riven: The Sequel to Myst
	// Version 1.? (5CD)
	// From jvprat
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "249e8c995d191b03ee94c892c0eac775"),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_RIVEN,
		GF_DVD,
		0,
	},

	// Riven: The Sequel to Myst
	// Version ? (DVD, From "Myst: La Trilogie")
	// From gamin
	{
		{
			"riven",
			"",
			AD_ENTRY1("a_Data.MHK", "aff2a384aaa9a0e0ec51010f708c5c04"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_RIVEN,
		GF_DVD,
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
			ADGF_DEMO,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_DEMO,
			Common::GUIO_NONE
		},
		GType_CSTIME,
		GF_DEMO,
		0
	},


	{
		{
			"zoombini",
			"",
			AD_ENTRY1("ZOOMBINI.MHK", "98b758fec55104c096cfd129048be9a6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ZOOMBINI,
		GF_HASMIDI,
		0
	},

	{
		{
			"zoombini",
			"",
			AD_ENTRY1("ZOOMBINI.MHK", "0672f65c40dd065840c896e41c13f980"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ZOOMBINI,
		GF_HASMIDI,
		0
	},

	{
		{
			"zoombini",
			"",
			AD_ENTRY1("ZOOMBINI.MHK", "6ae0bdf791266b1fe3d4fabbf44c3faa"),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ZOOMBINI,
		GF_HASMIDI,
		0
	},

	{
		{
			"zoombini",
			"",
			AD_ENTRY1("ZOOMBINI.MHK", "8231e58525143ccf6e8b747df34b139f"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_ZOOMBINI,
		GF_HASMIDI,
		0
	},

	{
		{
			"csworld",
			"v3.0",
			AD_ENTRY1("C2K.MHK", "605fe88380848031bbd0ff84ade6fe40"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_CSWORLD,
		0,
		0
	},

	{
		{
			"csworld",
			"v3.5",
			AD_ENTRY1("C2K.MHK", "d4857aeb0f5e2e0c4ac556aa74f38c23"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_CSWORLD,
		0,
		0
	},

	{
		{
			"csamtrak",
			"",
			AD_ENTRY1("AMTRAK.MHK", "2f95301f0bb950d555bb7b0e3b1b7eb1"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_CSAMTRAK,
		0,
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
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV5,
		0,
		0
	},

	{
		{
			"maggiesfa",
			"",
			AD_ENTRY1("Outline", "b7dc6e65fa9e80784a5bb8b557aa37c4"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"jamesmath",
			"",
			AD_ENTRY1("BRODER.MHK", "007299da8b2c6e8ec1cde9598c243024"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_JAMESMATH,
		GF_HASMIDI,
		0
	},

	// This is in the NEWDATA folder, so I assume it's a newer version ;)
	{
		{
			"jamesmath",
			"",
			AD_ENTRY1("BRODER.MHK", "53c000938a50dca92860fd9b546dd276"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_JAMESMATH,
		GF_HASMIDI,
		0
	},

	{
		{
			"treehouse",
			"",
			AD_ENTRY1("MAINROOM.MHK", "12f51894d7f838af639ea9bf1bc8f45b"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_TREEHOUSE,
		GF_HASMIDI,
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV2,
		0,
		0
	},

	{
		{
			"1stdegree",
			"",
			AD_ENTRY1("AL236_1.MHK", "3ba145492a7b8b4dee0ef4222c5639c3"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_1STDEGREE,
		GF_HASMIDI,
		0
	},

	// In The 1st Degree
	// French Windows
	// From Strangerke
	{
		{
			"1stdegree",
			"",
			AD_ENTRY1("AL236_1.MHK", "0e0c70b1b702b6ddca61a1192ada1282"),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_1STDEGREE,
		GF_HASMIDI,
		0
	},

	{
		{
			"csusa",
			"",
			AD_ENTRY1("USAC2K.MHK", "b8c9d3a2586f62bce3a48b50d7a700e9"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_CSUSA,
		0,
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		0,
		"TORTOISE.EXE"
	},

	{
		{
			"tortoise",
			"Demo v1.0",
			AD_ENTRY1("TORTOISE.512", "75d9a2f8339e423604a0c6e8177600a6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV3,
		0,
		0
	},

	{
		{
			"grandma",
			"Demo v1.0",
			AD_ENTRY1("PAGES.512", "95d9f4b035bf5d15c57a9189f231b0f8"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		"GRANDMA.EXE"
	},

	{
		{
			"grandma",
			"Demo",
			AD_ENTRY1("Bookoutline", "553c93891b9631d1e1d269599e1efa6c"),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_DEMO,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		0,
		"RUFF.EXE"
	},

	{
		{
			"newkid",
			"Demo v1.0",
			AD_ENTRY1("NEWKID.512", "2b9d94763a50d514c04a3af488934f73"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		0,
		"Little Monster at School"
	},

	{
		{
			"catinthehat",
			"",
			AD_ENTRY1("Outline", "0b5ab6dd7c08cf23066efa709fa48bbc"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		0,
		"Bears Get in a Fight"
	},

	{
		{
			"arthurcomp",
			"",
			AD_ENTRY1("OUTLINE", "dec4d1a05449f81b6012706932658326"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV2,
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
			Common::GUIO_NONE
		},
		GType_MYST,
		0,
		0
	},

	{
		{
			"MakingOfMyst",
			"unknown",
			AD_ENTRY1(0, 0),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
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
			Common::GUIO_NONE
		},
		GType_RIVEN,
		GF_DVD,
		0
	}
};

static const ADFileBasedFallback fileBased[] = {
	{ &fallbackDescs[0],  { "MYST.DAT", 0 } },
	{ &fallbackDescs[1],  { "MAKING.DAT", 0 } },
	{ &fallbackDescs[2],  { "MYST.DAT", "Help.dat", 0 } },	// Help system doesn't exist in original
	{ &fallbackDescs[3],  { "a_Data.MHK", 0 } },
	{ &fallbackDescs[4],  { "a_Data.MHK", "t_Data1.MHK" , 0 } },
	{ 0, { 0 } }
};

} // End of Namespace Mohawk
