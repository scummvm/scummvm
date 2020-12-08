/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/advancedDetector.h"

#include "common/translation.h"

namespace Stark {

static const PlainGameDescriptor starkGames[] = {
	{ "tlj", "The Longest Journey" },
	{ nullptr, nullptr }
};

static const ADGameDescription gameDescriptions[] = {
	// The Longest Journey
	// English DVD
	{
		"tlj", "DVD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// GOG edition
	{
		"tlj", "GOG",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// English Old Demo
	{
		"tlj", "Old Demo",
		AD_ENTRY2s("x.xarc",		"97abc1bb9239dee4c208e533f3c97e1c", 98,
				   "chapters.ini",	"5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_NONE
	},

	// The Longest Journey
	// English v1.61 Demo
	{
		"tlj", "v1.61 Demo",
		AD_ENTRY2s("x.xarc",		"61093bcd499b386ed5c0345c52f48909", 98,
				   "chapters.ini",	"5b5a1f1dd2297d9ce0d3d12216d5d2c5", 485),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_NONE
	},

	// The Longest Journey
	// French Demo
	{
		"tlj", "Demo",
		AD_ENTRY2s("x.xarc",		"97abc1bb9239dee4c208e533f3c97e1c", 98,
				   "chapters.ini",	"e54f6370dca06496069790840409cf95", 506),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian Demo
	{
		"tlj", "Demo",
		AD_ENTRY2s("x.xarc",		"97abc1bb9239dee4c208e533f3c97e1c", 98,
				   "chapters.ini",	"f358f604abd1aa1476ed05d6d271ac70", 473),
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian 4 CD version - supplied by L0ngcat
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"f358f604abd1aa1476ed05d6d271ac70", 473),
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Norwegian DLC-edition (DVD?)
	{
		"tlj", "DVD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"f358f604abd1aa1476ed05d6d271ac70", 473),
		Common::NB_NOR,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// German DVD version supplied by Vorph on the forums
	{
		"tlj", "DVD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"e4611d143a87b263d8d7a54edc7e7cd7", 515),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// German 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"e4611d143a87b263d8d7a54edc7e7cd7", 515),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Italian DVD version
	{
		"tlj", "DVD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"9a81ea4e6f5b84511dd4e56d04a64e2e", 498),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Italian 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"9a81ea4e6f5b84511dd4e56d04a64e2e", 498),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Dutch 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"c8dadd9a3b41640734d6213e89cd5635", 508),
		Common::NL_NLD,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Spanish 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"3640df6d536b186bff228337284d9631", 525),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// French 2CD
	{
		"tlj", "2 CD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"e54f6370dca06496069790840409cf95", 506),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// French 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"e54f6370dca06496069790840409cf95", 506),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish Demo
	{
		"tlj", "Demo",
		AD_ENTRY2s("x.xarc",		"97abc1bb9239dee4c208e533f3c97e1c", 98,
				   "chapters.ini",	"f6a2007300209492b7b90b4c0467832d", 462),
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"f6a2007300209492b7b90b4c0467832d", 462),
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Swedish DVD Nordic Special Edition - supplied by L0ngcat
	{
		"tlj", "DVD",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"f6a2007300209492b7b90b4c0467832d", 462),
		Common::SE_SWE,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Polish 4CD
	{
		"tlj", "4 CD",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"6abc5c38e6e31face4b675355b117620", 499),
		Common::PL_POL,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Russian 2CD by 1C
	{
		"tlj", "2 CD/Fargus",
		AD_ENTRY2s("x.xarc",		"de8327850d7bba90b690b141eaa23f61", 3032,
				   "chapters.ini",	"740b97b94e97ed11f064f5fa125ebee1", 486),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	// The Longest Journey
	// Russian 2CD by 7Wolf
	{
		"tlj", "2 CD/7Wolf",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"8e08025c89575d2573c2edf0daa1cb34", 406),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO_NONE
	},

	// The Longest Journey
	// Hungarian fan-made
	{
		"tlj", "Fanmade",
		AD_ENTRY2s("x.xarc",		"a0559457126caadab0cadac02d35f26f", 3032,
				   "chapters.ini",	"790b51a88b5493bff5168a77738e0e84", 451),
		Common::HU_HUN,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO_NONE
	},

	AD_TABLE_END_MARKER
};

#define GAMEOPTION_ASSETS_MOD        GUIO_GAMEOPTIONS1
#define GAMEOPTION_LINEAR_FILTERING  GUIO_GAMEOPTIONS2
#define GAMEOPTION_FONT_ANTIALIASING GUIO_GAMEOPTIONS3

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ASSETS_MOD,
		{
			_s("Load modded assets"),
			_s("Enable loading of external replacement assets."),
			"enable_assets_mod",
			true
		}
	},
	{
		GAMEOPTION_LINEAR_FILTERING,
		{
			_s("Enable linear filtering of the backgrounds images"),
			_s("When linear filtering is enabled the background graphics are smoother in full screen mode, at the cost of some details."),
			"use_linear_filtering",
			true
		}
	},
	{
		GAMEOPTION_FONT_ANTIALIASING,
		{
			_s("Enable font anti-aliasing"),
			_s("When font anti-aliasing is enabled, the text is smoother."),
			"enable_font_antialiasing",
			true
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class StarkMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	StarkMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(ADGameDescription), starkGames, optionsList) {
		_guiOptions = GUIO4(GUIO_NOMIDI, GAMEOPTION_ASSETS_MOD, GAMEOPTION_LINEAR_FILTERING, GAMEOPTION_FONT_ANTIALIASING);
	}

	const char *getName() const override {
		return "Stark";
	}

	const char *getEngineId() const override {
		return "stark";
	}

	const char *getOriginalCopyright() const override {
		return "(C) Funcom";
	}
};

} // End of namespace Stark

REGISTER_PLUGIN_STATIC(STARK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Stark::StarkMetaEngineDetection);
