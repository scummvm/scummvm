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

#include "common/config-manager.h"
#include "common/language.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "twine/detection.h"

static const PlainGameDescriptor twineGames[] = {
	{ "lba", "Little Big Adventure" },
	{ "lba2", "Little Big Adventure 2" },
	{ nullptr,  nullptr }
};

static const ADGameDescription twineGameDescriptions[] = {
	// Little Big Adventure - Preview Version (EN, FR)
	// LBA.EXE
	// 8 August 1994 at 19:30
	{
		"lba",
		"Preview Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 294025),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Preview Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 294025),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Demo Version (EN, FR, DE, IT, ES)
	// RELENT.EXE
	// 14 October 1994 at 10:18
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Original European Version (EN, FR, DE, IT, ES)
	// LBA.EXE
	// 14 Oct 1994 at 12:45
	{
		"lba",
		"CD Original European Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"CD Original European Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"CD Original European Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"CD Original European Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"CD Original European Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Relentless: Twinsen's Adventure - Original North America Version (EN, FR, DE, IT, ES)
	// RELENT.EXE
	// 14 Oct 1994 at 13:22
	{
		"lba",
		"Relentless: Twinsen's Adventure - CD Original North America Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Relentless: Twinsen's Adventure - CD Original North America Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Relentless: Twinsen's Adventure - CD Original North America Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Relentless: Twinsen's Adventure - CD Original North America Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Relentless: Twinsen's Adventure - CD Original North America Version",
		AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Floppy Disk Version
	// FLA_GIF.HQR
	// 11 August 1995 at 23:28
	{
		"lba",
		"Floppy Disk Version",
		AD_ENTRY1s("FLA_GIF.HQR", "3f7383f65afa212e3eec430627828b64", 1784466),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING | TwinE::TF_USE_GIF,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Original Japanese Version
	// LBAJ.EXE
	// 15 Oct 1995 at 13:28
	{
		"lba",
		"Original Japanese Version",
		AD_ENTRY1s("LBAJ.EXE", "54a1e8749448e08086a1929510ec4b6a", 278043),
		Common::JA_JPN,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - DotEmu Version (Steam)
	// LBA.DOT
	// 11 October 2011 at 17:30
	{
		"lba",
		"DotEmu Version (Steam)",
		AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Version (Steam)",
		AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Version (Steam)",
		AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Version (Steam)",
		AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Version (Steam)",
		AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - DotEmu Enhanced Version (Steam)
	// LBA.exe
	// 27 February 2018 at 08:10
	{
		"lba",
		"DotEmu Enhanced Version (Steam)",
		AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Enhanced Version (Steam)",
		AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Enhanced Version (Steam)",
		AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Enhanced Version (Steam)",
		AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"DotEmu Enhanced Version (Steam)",
		AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - GOG Version
	// LBA.GOG
	// 11 October 2011 at 17:30
	{
		"lba",
		"GOG Version",
		AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"GOG Version",
		AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"GOG Version",
		AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"GOG Version",
		AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"GOG Version",
		AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::EN_GRB,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// FAN Translations - http://lba.fishos.net/bit/index.html

	// Portuguese by xesf (alexfont)
	{
		"lba",
		"Fan Translation by xesf",
		AD_ENTRY1s("TEXT.HQR", "2a8df71946aa9ee4c777a9d6414b89ce", 282308),
		Common::PT_POR,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Polish by Zink
	{
		"lba",
		"Fan Translation by Zink",
		AD_ENTRY1s("text.hqr", "7f41b5e8efb07dd413f59377e03b1b04", 413920),
		Common::PL_POL,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Hungarian by Gregorius
	{
		"lba",
		"Fan Translation by Gregorius",
		AD_ENTRY1s("TEXT.HQR", "31d760b41a424ec2926f494d7ecac14a", 410709),
		Common::HU_HUN,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Hebrew by ChaosFish
	{
		"lba",
		"Fan Translation by ChaosFish",
		AD_ENTRY1s("TEXT.HQR", "c1adf48ea71fead82d91c5b062eeeb99", 75866),
		Common::HE_ISR,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Brazilian Portuguese by spider_ruler33
	{
		"lba",
		"Fan Translation by spider_ruler33",
		AD_ENTRY1s("TEXT.HQR", "2bf227f9e8fcdc7397372b68786c446e", 283631),
		Common::PT_BRA,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Russian by Cody
	{
		"lba",
		"Fan Translation by Cody",
		AD_ENTRY1s("TEXT.HQR", "93b1a29711f0750156280012e53fdcd2", 280306),
		Common::RU_RUS,
		Common::kPlatformDOS,
		ADGF_TESTING,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure 2

	// Little Big Adventure 2 - Original European Version (EN, FR, DE, IT, ES)
	// LBA2.EXE
	// 4 Sep 2004 at 18:44
	{
		"lba2",
		"CD Original European Version",

		AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lba2",
		"CD Original European Version",
		AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lba2",
		"CD Original European Version",
		AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lba2",
		"CD Original European Version",
		AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lba2",
		"CD Original European Version",
		AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448),
		Common::ES_ESP,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

class TwinEMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TwinEMetaEngineDetection() : AdvancedMetaEngineDetection(twineGameDescriptions, sizeof(ADGameDescription), twineGames) {
	}

	const char *getEngineId() const override {
		return "twine";
	}

	const char *getName() const override {
		return "Little Big Adventure";
	}

	const char *getOriginalCopyright() const override {
		return "Little Big Adventure (C) Adeline Software International";
	}
};

REGISTER_PLUGIN_STATIC(TWINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TwinEMetaEngineDetection);
