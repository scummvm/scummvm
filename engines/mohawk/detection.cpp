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

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/savefile.h"

#include "mohawk/myst.h"
#include "mohawk/riven.h"
#include "mohawk/livingbooks.h"

// Define this to enable detection of other Broderbund titles which use Mohawk (besides Myst/Riven)
#define DETECT_BRODERBUND_TITLES

namespace Mohawk {

struct MohawkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	uint16 version;
};

const char* MohawkEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 MohawkEngine::getFeatures() const {
	return _gameDescription->features;
}

Common::Platform MohawkEngine::getPlatform() const {
	return _gameDescription->desc.platform;
}

uint16 MohawkEngine::getVersion() const {
	return _gameDescription->version;
}

uint8 MohawkEngine::getGameType() {
	return _gameDescription->gameType;
}

Common::String MohawkEngine_LivingBooks::getBookInfoFileName() {
	return _gameDescription->desc.filesDescriptions[0].fileName;
}

Common::Language MohawkEngine::getLanguage() {
	return _gameDescription->desc.language;
}

bool MohawkEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL);
}

bool MohawkEngine_Myst::hasFeature(EngineFeature f) const {
	return 
		MohawkEngine::hasFeature(f)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

bool MohawkEngine_Riven::hasFeature(EngineFeature f) const {
	return 
		MohawkEngine::hasFeature(f)
		|| (f == kSupportsLoadingDuringRuntime)
		|| (f == kSupportsSavingDuringRuntime);
}

} // End of Namespace Mohawk

static const PlainGameDescriptor mohawkGames[] = {
	{"mohawk", "Mohawk Game"},
	{"myst", "Myst"},
	{"MakingOfMyst", "The Making of Myst"},
	{"riven", "Riven: The Sequel to Myst"},
#ifdef DETECT_BRODERBUND_TITLES
	{"zoombini", "Logical Journey of the Zoombinis Deluxe"},
	{"csworld", "Where in the World is Carmen Sandiego?"},
	{"csamtrak", "Where in America is Carmen Sandiego? (The Great Amtrak Train Adventure)"},
	{"maggiess", "Maggie's Farmyard Adventure"},
	{"jamesmath", "James Discovers/Explores Math"},
	{"treehouse", "The Treehouse"},
	{"greeneggs", "Green Eggs and Ham"},
	{"1stdegree", "In the 1st Degree"},
	{"csusa", "Where in the USA is Carmen Sandiego?"},
	{"tortoise", "Aesop's Fables: The Tortoise and the Hare"},
	{"arthur", "Arthur's Teacher Troubles"},
	{"grandma", "Just Grandma and Me"},
	{"ruff", "Ruff's Bone"},
	{"newkid", "The New Kid on the Block"},
	{"arthurrace", "Arthur's Reading Race"},
#endif
	{0, 0}
};


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
		GF_ME|GF_10TH,
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
		GF_ME|GF_10TH,
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
		GF_DVD|GF_10TH,
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
		GType_NEWLIVINGBOOKS,
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
		GType_NEWLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_OLDLIVINGBOOKS,
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
		GType_NEWLIVINGBOOKS,
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
		GType_NEWLIVINGBOOKS,
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

	// The Masterpiece Edition of Myst has 24bit color, and thus is not supported by ScummVM
	// Currently, its graphics are just bypassed.
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

} // End of namespace Mohawk

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)Mohawk::gameDescriptions,
	// Size of that superset structure
	sizeof(Mohawk::MohawkGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	mohawkGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"mohawk",
	// List of files for file-based fallback detection (optional)
	Mohawk::fileBased,
	// Flags
	0,
	// Additional GUI options (for every game)
	Common::GUIO_NONE
};

class MohawkMetaEngine : public AdvancedMetaEngine {
public:
	MohawkMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Mohawk Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "Myst and Riven (C) Cyan Worlds\nMohawk OS (C) Ubisoft";
	}

	virtual bool hasFeature(MetaEngineFeature f) const;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
	virtual SaveStateList listSaves(const char *target) const;
	virtual int getMaximumSaveSlot() const { return 999; }
	virtual void removeSaveState(const char *target, int slot) const;
};

bool MohawkMetaEngine::hasFeature(MetaEngineFeature f) const {
	return 
		(f == kSupportsListSaves)
		|| (f == kSupportsLoadingDuringStartup)
		|| (f == kSupportsDeleteSave);
}

SaveStateList MohawkMetaEngine::listSaves(const char *target) const {
	Common::StringList filenames;
	SaveStateList saveList;

	// Loading games is only supported in Myst/Riven currently.
	if (strstr(target, "myst")) {
		filenames = g_system->getSavefileManager()->listSavefiles("*.mys");
	
		for (uint32 i = 0; i < filenames.size(); i++)
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));
	} else if (strstr(target, "riven")) {
		filenames = g_system->getSavefileManager()->listSavefiles("*.rvn");
	
		for (uint32 i = 0; i < filenames.size(); i++)
			saveList.push_back(SaveStateDescriptor(i, filenames[i]));
	} 
			
	return saveList;
}

void MohawkMetaEngine::removeSaveState(const char *target, int slot) const {
	// Removing saved games is only supported in Myst/Riven currently.
	if (strstr(target, "myst")) {
		Common::StringList filenames = g_system->getSavefileManager()->listSavefiles("*.mys");
		g_system->getSavefileManager()->removeSavefile(filenames[slot].c_str());
	} else if (strstr(target, "riven")) {
		Common::StringList filenames = g_system->getSavefileManager()->listSavefiles("*.rvn");
		g_system->getSavefileManager()->removeSavefile(filenames[slot].c_str());
	}
}

bool MohawkMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Mohawk::MohawkGameDescription *gd = (const Mohawk::MohawkGameDescription *)desc;
	
	if (gd) {
		switch (gd->gameType) {
		case Mohawk::GType_MYST:
		case Mohawk::GType_MAKINGOF:
			*engine = new Mohawk::MohawkEngine_Myst(syst, gd);
			break;
		case Mohawk::GType_RIVEN:
			*engine = new Mohawk::MohawkEngine_Riven(syst, gd);
			break;
		case Mohawk::GType_OLDLIVINGBOOKS:
		case Mohawk::GType_NEWLIVINGBOOKS:
			*engine = new Mohawk::MohawkEngine_LivingBooks(syst, gd);
			break;
		case Mohawk::GType_ZOOMBINI:
		case Mohawk::GType_CSWORLD:
		case Mohawk::GType_CSAMTRAK:
		case Mohawk::GType_MAGGIESS:
		case Mohawk::GType_JAMESMATH:
		case Mohawk::GType_TREEHOUSE:
		case Mohawk::GType_1STDEGREE:
		case Mohawk::GType_CSUSA:
			error ("Unsupported Mohawk Engine");
			break;
		default:
			error ("Unknown Mohawk Engine");
		}
	}

	return (gd != 0);
}

#if PLUGIN_ENABLED_DYNAMIC(MOHAWK)
	REGISTER_PLUGIN_DYNAMIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MOHAWK, PLUGIN_TYPE_ENGINE, MohawkMetaEngine);
#endif
