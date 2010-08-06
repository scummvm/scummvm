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

#ifdef DETECT_BRODERBUND_TITLES
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

	{
		{
			"maggiess",
			"",
			AD_ENTRY1("MAGGIESS.MHK", "08f75fc8c0390e68fdada5ddb35d0355"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			Common::GUIO_NONE
		},
		GType_MAGGIESS,
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
		1
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
			"Demo v1.0",
			AD_ENTRY1("TORTOISE.512", "75d9a2f8339e423604a0c6e8177600a6"),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			Common::GUIO_NONE
		},
		GType_LIVINGBOOKSV1,
		GF_DEMO,
		0
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
		0
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
		0
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
		0,
		0
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
		GF_DEMO,
		0
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
		0
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
		GF_DEMO,
		0
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
		GF_DEMO,
		0
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
		0
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
		GF_DEMO,
		0
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
		0
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
		0
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
		0
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
		0
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
		0
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
		0
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
		0
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
		0
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
		0
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
		0
	},
#endif

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
