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

#include "engines/advancedDetector.h"
#include "engines/grim/detection.h"

#include "common/translation.h"

namespace Grim {

static const PlainGameDescriptor grimGames[] = {
	{"grim", "Grim Fandango"},
	{"monkey4", "Escape From Monkey Island"},
	{nullptr, nullptr}
};

#define GAMEOPTION_LOAD_DATAUSR GUIO_GAMEOPTIONS1
#define GAMEOPTION_SHOW_FPS GUIO_GAMEOPTIONS2

#define GUI_OPTIONS_GRIME GUIO2(GAMEOPTION_LOAD_DATAUSR, GAMEOPTION_SHOW_FPS)

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_LOAD_DATAUSR,
		{
			_s("Load user patch (unsupported)"),
			_s("Load an user patch. Please note that the ScummVM team doesn't provide support for using such patches."),
			"datausr_load",
			false
		}
	},
	{
		GAMEOPTION_SHOW_FPS,
		{
			_s("Show FPS"),
			_s("Show the current FPS-rate, while you play."),
			"show_fps",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const GrimGameDescription gameDescriptions[] = {
	{
		// Grim Fandango English version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "444f05f2af689c1bffd179b8b6a632bd", 57993159),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango English version (unpatched)
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "8b12ed530195c6c577436df27df62ecb", 58011176),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango French version (un/patched ???)
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "19bc0dc9554257b1f021463de54f359f", 56268691),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Portuguese version
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "89da4d4f4f90a8ea390450ed5a617f08", 57875710),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Italian version
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "9e7075f3fb0427ae8136b290538d07dd", 62185775),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Spanish version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "85d3e9504c481c5ccf2119ea6e0f4e2f", 53831340),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango German version
		{
			"grim",
			"",
			AD_ENTRY1s("VOX0001.LAB", "d304aa402098de5966816c0a11e45816", 66829347),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
/*	{
		// Grim Fandango German version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "464138caf47e580cbb237dee10674b16", 398671),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Spanish version
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "b1460cd029f13718f7f62c2403e047ec", 372709),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Spanish version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "b1460cd029f13718f7f62c2403e047ec", 372020),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Italian version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "2d99c796b7a4e5c421cae49dc29dab6c", 369242),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango French version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "3bd00ca87214862c012ac99e1758dd83", 386292),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Portuguese version (patched)
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "4dc16be476bb6036b423bc331ca8281a", 362994),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
*/	{
		// Grim Fandango English demo version
		{
			"grim",
			"Demo",
			AD_ENTRY1s("gfdemo01.lab", "755cdac083f7f751bec7506402278f1a", 29489930),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango English demo version (with intro video)
		{
			"grim",
			"Demo",
			AD_ENTRY1s("gdemo001.lab", "c04c814093be829c4811a3a0aa80833d", 46615911),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango French demo version
		{
			"grim",
			"Demo",
			AD_ENTRY2s("gfdemo01.lab", "7df813f3809f2c0234213cfa4f6da062", 29533695,
					   "voice001.lab", "7df474e03c23692ed02e4ce45f1a6b30", 13764168),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},

	{
		// Grim Fandango German demo version
		{
			"grim",
			"Demo",
			AD_ENTRY2s("gfdemo01.lab", "7df813f3809f2c0234213cfa4f6da062", 29533695,
					   "voice001.lab", "2788dc7fd226787f3a68ac9c853d2580", 16561196),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},

	{
		// Grim Fandango Italian demo version
		{
			"grim",
			"Demo",
			AD_ENTRY2s("gfdemo01.lab", "7df813f3809f2c0234213cfa4f6da062", 29533695,
					   "voice001.lab", "3b8ace62584380c66b73981e014ea40e", 14907410),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},

	{
		// Grim Fandango Spanish demo version
		{
			"grim",
			"Demo",
			AD_ENTRY2s("gfdemo01.lab", "7df813f3809f2c0234213cfa4f6da062", 29533695,
					   "voice001.lab", "a810ec11acaf9d76cd04d2f68fcdc912", 13367206),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},
	{
		// Grim Fandango Remastered
		{
			"grim",
			"Remastered",
			AD_ENTRY1s("VOX0001.LAB", "0ff872fb353707fbdb9579038d4cf31c", 382736476),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_REMASTERED,
			GUI_OPTIONS_GRIME
		},
		GType_GRIM
	},

	{
		// Escape from Monkey Island English
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "61959da91d864bf5f4588daa4a5a3019", 18515664),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island German
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "007a33881478be6b6e0228d8888536ae", 18512568),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Italian
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "d2f010c1cd1fd002eea403282a6b9a1e", 18513451),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Spanish
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "0d459954031c086a0448d2eb3fa068a1", 18514404),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island French
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "151af0a694382af873f325fcea293bb1", 18514420),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Portuguese
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "030e7637aee7886a3caad60cf102f797", 18515747),
			Common::PT_BRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Russian
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "779561a70a11dd5686974f122fc1516c", 18500052),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island German (Mac)
		{
			"monkey4",
			"",
			AD_ENTRY2s("artAll.m4b",	 "007a33881478be6b6e0228d8888536ae", 18512568,
					   "EFMI Installer", "54298c7440dafedf33d2b27c7bb24052", 9241784),
			Common::DE_DEU,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island English PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "0dc9a4df0d8553f277d8dc8e23b6249d", 34593974),
			Common::EN_ANY,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island German PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "5b5c7a3964c168eab44b82981db357d8", 34642186),
			Common::DE_DEU,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Italian PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "2de68c8fd955c1a3c50202b072bde0cb", 34642651),
			Common::IT_ITA,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island Spanish PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "ff6689dcca36c249ec834a3019aeb397", 34642656),
			Common::ES_ESP,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island French PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("artAll.m4b", "5ce964a19a8672944b9b62170e45ce28", 34593681),
			Common::FR_FRA,
			Common::kPlatformPS2,
			ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island demo (English)
		{
			"monkey4",
			"Demo",
			AD_ENTRY2s("magdemo.lab",	"9e7eaa1b9317ff47d5deeda0b2c42ce3", 19826116,
					   "i9n.lab",		 "274f8579b01e0872fe6f1ba267266149", 26951),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island demo (French)
		{
			"monkey4",
			"Demo",
			AD_ENTRY2s("magdemo.lab",	"9e7eaa1b9317ff47d5deeda0b2c42ce3", 19826116,
					   "i9n.lab",		"7f1744990472261bdcbc02036ba9f7ec", 1718385),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},

	{
		// Escape from Monkey Island demo (German)
		{
			"monkey4",
			"Demo",
			AD_ENTRY2s("magdemo.lab",	"9e7eaa1b9317ff47d5deeda0b2c42ce3", 19826116,
					   "i9n.lab",		"28f6bc270b5c31970cc110c7656ff598", 1749051),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},

	{
		// Escape from Monkey Island demo (Spanish)
		{
			"monkey4",
			"Demo",
			AD_ENTRY2s("magdemo.lab",	"9e7eaa1b9317ff47d5deeda0b2c42ce3", 19826116,
					   "i9n.lab",		"53b20d930f6e8c2e0880ed7e336eeebc", 1740761),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_DEMO | ADGF_UNSTABLE,
			GUI_OPTIONS_GRIME
		},
		GType_MONKEY4
	},

	{ AD_TABLE_END_MARKER, GType_GRIM }
};

class GrimMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	GrimMetaEngineDetection() : AdvancedMetaEngineDetection(Grim::gameDescriptions, sizeof(Grim::GrimGameDescription), grimGames, gameGuiOptions) {
		_guiOptions = GUIO_NOMIDI;
	}

	PlainGameDescriptor findGame(const char *gameid) const override {
		return Engines::findGameID(gameid, _gameIds, obsoleteGameIDsTable);
	}

	const char *getName() const override {
		return "Grim";
	}

	const char *getEngineId() const override {
		return "grim";
	}

	const char *getOriginalCopyright() const override {
		return "LucasArts GrimE Games (C) LucasArts";
	}

};

} // End of namespace Grim


REGISTER_PLUGIN_STATIC(GRIM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Grim::GrimMetaEngineDetection);
