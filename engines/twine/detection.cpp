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

#include "common/language.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "base/plugins.h"
#include "twine/detection.h"
#include "twine/shared.h"

static const PlainGameDescriptor twineGames[] = {
	{ "lba", "Little Big Adventure" },
	{ "lbashow", "Little Big Adventure Freeware Slide Show" },
	{ "lba2", "Little Big Adventure 2" },
	{ nullptr,  nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{TwinE::kDebugScriptsMove, "scriptsmove", "Move script debugging"},
	{TwinE::kDebugScriptsLife, "scriptslife", "Life script debugging"},
	{TwinE::kDebugResources, "resources", "Resources debugging"},
	{TwinE::kDebugTimers, "timers", "Timer debugging"},
	{TwinE::kDebugImGui, "imgui", "UI for debugging"},
	{TwinE::kDebugInput, "input", "Input debugging"},
	{TwinE::kDebugMovies, "movies", "Movies debugging"},
	{TwinE::kDebugPalette, "palette", "Palette debugging"},
	{TwinE::kDebugCollision, "collision", "Collision debugging"},
	{TwinE::kDebugAnimation, "animation", "Animation debugging"},
	DEBUG_CHANNEL_END
};

#define TWINE_DETECTION_ENTRY(gameid, extra, filesDescriptions, platform, flags) \
	{ \
		gameid,            \
		extra,             \
		filesDescriptions, \
		Common::EN_ANY,    \
		platform,          \
		flags,             \
		GUIO1(GUIO_NONE)   \
	}, \
	{ \
		gameid,            \
		extra,             \
		filesDescriptions, \
		Common::FR_FRA,    \
		platform,          \
		flags,             \
		GUIO1(GUIO_NONE)   \
	}, \
	{ \
		gameid,            \
		extra,             \
		filesDescriptions, \
		Common::DE_DEU,    \
		platform,          \
		flags,             \
		GUIO1(GUIO_NONE)   \
	}, \
	{ \
		gameid,            \
		extra,             \
		filesDescriptions, \
		Common::IT_ITA,    \
		platform,          \
		flags,             \
		GUIO1(GUIO_NONE)   \
	}, \
	{ \
		gameid,            \
		extra,             \
		filesDescriptions, \
		Common::ES_ESP,    \
		platform,          \
		flags,             \
		GUIO1(GUIO_NONE)   \
	}

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
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Preview Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 294025),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Preview Version (EN, FR)
	// LBA.EXE
	// 15 August 1994 at 18:28
	{
		"lba",
		"Preview Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 298697),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"Preview Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 298697),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Demo Version (EN, FR, DE, IT, ES)
	// RELENT.EXE
	// 14 October 1994 at 10:18
	TWINE_DETECTION_ENTRY("lba", "Demo Version", AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 245961), Common::kPlatformDOS, ADGF_DEMO),

	// Little Big Adventure - Original European Version (EN, FR, DE, IT, ES)
	// LBA.EXE
	// 14 Oct 1994 at 12:45
	TWINE_DETECTION_ENTRY("lba", "CD Original European Version", AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513), Common::kPlatformDOS, ADGF_CD),

	// Relentless: Twinsen's Adventure - Original North America Version (EN, FR, DE, IT, ES)
	// RELENT.EXE
	// 14 Oct 1994 at 13:22
	TWINE_DETECTION_ENTRY("lba", "Relentless: Twinsen's Adventure - CD Original North America Version", AD_ENTRY1s("RELENT.EXE", "c1a887e38283d43f271249ad9f2a73ef", 258513), Common::kPlatformDOS, TwinE::TF_VERSION_USA | ADGF_CD),

	// Little Big Adventure - Demo Version (FR)
	// LBA.EXE
	// 21 October 1994 at 15:25
	{
		"lba",
		"Demo Version",
		AD_ENTRY1s("LBA.EXE", "c1a887e38283d43f271249ad9f2a73ef", 273281),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_DEMO,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Floppy Disk Version
	// FLA_GIF.HQR
	// 11 August 1995 at 23:28
	{
		"lba",
#ifdef USE_GIF
		"Floppy Disk Version",
		AD_ENTRY1s("FLA_GIF.HQR", "3f7383f65afa212e3eec430627828b64", 1784466),
		Common::EN_ANY,
		Common::kPlatformDOS,
		TwinE::TF_USE_GIF,
#else
		_s("This version requires Giflib which was not compiled into ScummVM"),
		AD_ENTRY1s("FLA_GIF.HQR", "3f7383f65afa212e3eec430627828b64", 1784466),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSUPPORTED,
#endif
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
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Virgin Asia cd release - english only
	{
		"lba",
		"Virgin Asia CD release",
		AD_ENTRY1s("text.hqr", "5b8329ebd078adc92979d04987692e9b", 442921),
		Common::EN_GRB,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - DotEmu Version (Steam)
	// LBA.DOT
	// 11 October 2011 at 17:30
	TWINE_DETECTION_ENTRY("lba", "DotEmu Version (Steam)", AD_ENTRY1s("LBA.DOT", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496), Common::kPlatformDOS, ADGF_NO_FLAGS),

	// Little Big Adventure (CD Spanish)
	TWINE_DETECTION_ENTRY("lba", "", AD_ENTRY1s("text.hqr", "ae7343552f8fbd17a1fc6cea2197a912", 248654), Common::kPlatformDOS, ADGF_NO_FLAGS),

	// Little Big Adventure - DotEmu Enhanced Version (Steam)
	// LBA.exe
	// 27 February 2018 at 08:10
	TWINE_DETECTION_ENTRY("lba", "DotEmu Enhanced Version (Steam)", AD_ENTRY1s("LBA.exe", "1f176b4329fbc7efc8f9f30f97013c5f", 1165728), Common::kPlatformWindows, TwinE::TF_DOTEMU_ENHANCED),

	// Little Big Adventure - DotEmu Enhanced Version (Android)
	// liblba.so
	// 8 Sep 2014 at 15:56
	TWINE_DETECTION_ENTRY("lba", "DotEmu", AD_ENTRY1s("text.hqr", "a374c93450dd2bb874b7167a63974e8d", 377224), Common::kPlatformAndroid, TwinE::TF_DOTEMU_ENHANCED),

	// Twinsen's Little Big Adventure Classic - 2015 Edition (Steam)
	TWINE_DETECTION_ENTRY("lba", "DotEmu (Steam)", AD_ENTRY1s("LBA.EXE", "615a9a0c3dae2c3b5fca0dee4d84dc72", 931328), Common::kPlatformWindows, TwinE::TF_DOTEMU_ENHANCED),

	// Little Big Adventure - GOG Version
	// LBA.GOG
	// 11 October 2011 at 17:30
	TWINE_DETECTION_ENTRY("lba", "GOG.com Version", AD_ENTRY1s("LBA.GOG", "6dc00342c80bc41b4ff5a43c560c7abc", 380666496), Common::kPlatformDOS, ADGF_NO_FLAGS),

	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::EN_GRB,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::FR_FRA,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},
	{
		"lba",
		"",
		AD_ENTRY1s("text.hqr", "31d880f658cc6cc6d6cf70df732aec4f", 248829),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure - Steam Version
	// TLBA1C.exe
	// 31 May 2022 12:58
	TWINE_DETECTION_ENTRY("lba", "Classic Version (Steam)", AD_ENTRY1s("TLBA1C.exe", "38b038eee2d93a5bc0e0405886161252", 4417024), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - Steam Version
	// TLBA1C.exe
	// 10 Jun 2022 14:51
	TWINE_DETECTION_ENTRY("lba", "Classic Version (Steam)", AD_ENTRY1s("TLBA1C.exe", "e4bab4647eabb998f627ac7628d94790", 4418048), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - Steam Version (3.2.2)
	// TLBA1C.exe
	// 1 Jul 2022
	TWINE_DETECTION_ENTRY("lba", "Classic Version (Steam)", AD_ENTRY1s("TLBA1C.exe", "58a7fba8a556196bb14d4f492017fb2b", 4416000), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - Steam Version (3.2.3)
	// TLBA1C.exe
	// 8 Sep 2022
	TWINE_DETECTION_ENTRY("lba", "Classic Version (Steam)", AD_ENTRY1s("TLBA1C.exe", "01f38555eca4a5dd076a4599359de4eb", 4445696), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - GOG Version 3.2.0 (56122)
	// TLBA1C.exe
	// 1st June 2022 02:18
	TWINE_DETECTION_ENTRY("lba", "GOG.com Classic Version", AD_ENTRY1s("TLBA1C.exe", "e377d036e997acbf543bc3023ce72be6", 4404224), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - GOG Version 3.2.1 (56604)
	// TLBA1C.exe
	// 17 Jun 2022 00:30
	TWINE_DETECTION_ENTRY("lba", "GOG.com Classic Version", AD_ENTRY1s("TLBA1C.exe", "e86192e4a491805dc011dda5ca83c608", 4404736), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - GOG Version 3.2.2
	// TLBA1C.exe
	// 1 Jul 2022
	TWINE_DETECTION_ENTRY("lba", "GOG.com Classic Version", AD_ENTRY1s("TLBA1C.exe", "76b227e87038c17a3376b1c681f15474", 4402688), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// Little Big Adventure - GOG Version 3.2.3.1 (58613)
	// TLBA1C.exe
	TWINE_DETECTION_ENTRY("lba", "GOG.com Classic Version", AD_ENTRY1s("TLBA1C.exe", "252216e83f83dc770fafc7fd787a6da5", 4432896), Common::kPlatformWindows, TwinE::TF_LBA1_CLASSIC),

	// FAN Translations - http://lba.fishos.net/bit/index.html

	// Portuguese by xesf (alexfont)
	{
		"lba",
		"Fan Translation by xesf",
		AD_ENTRY1s("TEXT.HQR", "2a8df71946aa9ee4c777a9d6414b89ce", 282308),
		Common::PT_PRT,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Polish by Zink
	{
		"lba",
		"Fan Translation by Zink",
		AD_ENTRY1s("text.hqr", "7f41b5e8efb07dd413f59377e03b1b04", 413920),
		Common::PL_POL,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Hungarian by Gregorius
	{
		"lba",
		"Fan Translation by Gregorius",
		AD_ENTRY1s("TEXT.HQR", "31d760b41a424ec2926f494d7ecac14a", 410709),
		Common::HU_HUN,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Hebrew by ChaosFish
	{
		"lba",
		"Fan Translation by ChaosFish",
		AD_ENTRY1s("TEXT.HQR", "c1adf48ea71fead82d91c5b062eeeb99", 75866),
		Common::HE_ISR,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Brazilian Portuguese by spider_ruler33
	{
		"lba",
		"Fan Translation by spider_ruler33",
		AD_ENTRY1s("TEXT.HQR", "2bf227f9e8fcdc7397372b68786c446e", 283631),
		Common::PT_BRA,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Russian by Cody
	{
		"lba",
		"Fan Translation by Cody",
		AD_ENTRY1s("TEXT.HQR", "93b1a29711f0750156280012e53fdcd2", 280306),
		Common::RU_RUS,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// https://forum.magicball.net/showthread.php?p=386653#post386653
	// LBA:LID Demo v1.0
	{
		"lba",
		"LBA: Lupin Island Destiny",
		AD_ENTRY1s("TEXT.HQR", "859339686e87f5c9f71253c25610a9fd", 240238),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO | TwinE::TF_MOD,
		GUIO1(GUIO_NONE)
	},

	// https://forum.magicball.net/showthread.php?t=16549
	// LBA:Prequel Demo v1.0
	{
		"lba",
		"LBA: Prequel",
		AD_ENTRY1s("TEXT.HQR", "79352a6f59ff2d8984573bfa421ef346", 259722),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE | ADGF_DEMO | TwinE::TF_MOD,
		GUIO1(GUIO_NONE)
	},

	// LBA Freeware Slide Show
	// 4 Apr 1994
	{
		"lbashow",
		"LBA Freeware Slide Show",
		AD_ENTRY1s("LBA_SHOW.EXE", "c1a887e38283d43f271249ad9f2a73ef", 85928),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO1(GUIO_NONE)
	},

	// Little Big Adventure 2

	// Little Big Adventure 2 - Original European Version (EN, FR, DE, IT, ES)
	// LBA2.EXE
	// 4 Sep 2004 at 18:44
	TWINE_DETECTION_ENTRY("lba2", "CD Original European Version", AD_ENTRY1s("LBA2.EXE", "ba915d65b3c7a743a87804f73f29675b", 616448), Common::kPlatformDOS, ADGF_UNSTABLE),
	TWINE_DETECTION_ENTRY("lba2", "CD Original European Version", AD_ENTRY1s("text.hqr", "dafcec7d9f1b2ab4a12d478786eb0f61", 443010), Common::kPlatformDOS, ADGF_UNSTABLE),

	AD_TABLE_END_MARKER
};

class TwinEMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	TwinEMetaEngineDetection() : AdvancedMetaEngineDetection(twineGameDescriptions, twineGames) {
		_guiOptions = GUIO11(GAMEOPTION_WALL_COLLISION, GAMEOPTION_DISABLE_SAVE_MENU,  GAMEOPTION_DEBUG, GAMEOPTION_SOUND, GAMEOPTION_VOICES, GAMEOPTION_TEXT, GAMEOPTION_MOVIES, GAMEOPTION_MOUSE, GAMEOPTION_USA_VERSION, GAMEOPTION_HIGH_RESOLUTION, GAMEOPTION_TEXT_TO_SPEECH);
	}

	const char *getName() const override {
		return "twine";
	}

	const char *getEngineName() const override {
		return "Little Big Adventure";
	}

	const char *getOriginalCopyright() const override {
		return "Little Big Adventure (C) Adeline Software International";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(TWINE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TwinEMetaEngineDetection);
