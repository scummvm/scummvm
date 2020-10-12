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

#include "base/plugins.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"

#include "hdb/detection.h"

static const PlainGameDescriptor hdbGames[] = {
	{"hdb", "Hyperspace Delivery Boy!"},
	{0, 0}
};

#define GAMEOPTION_CHEATMODE GUIO_GAMEOPTIONS1

namespace HDB {

static const ADGameDescription gameDescriptions[] = {
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.mpc", "ff8e51d0872736bc6afe87cfcb846b70", 50339161),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.mpc", "39d3c9dbc9614f370ad9430307c043f9", 45645305),
		Common::EN_ANY,
		Common::kPlatformLinux,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	// PocketPC Arm
	{
		"hdb",
		"",
		AD_ENTRY1s("hyperspace.msd", "a62468904beb3efe16d4d64f3955a32e", 6825555),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},

	// Demos
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "d8743b3b8be56486bcfb1398b2f2aad4", 13816461),
		Common::EN_ANY,
		Common::kPlatformLinux,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "f3bc878e179f00b8666a9846f3d9f9f5", 5236568),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	// provided by sev
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.mpc", "7bc533e8f1866931c884f1bc09353744", 13906865),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	{
		"hdb",
		"Demo",
		AD_ENTRY1s("hyperdemo.msd", "312525298ca9f5ac904883d1ce19dc0f", 3088651),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		ADGF_DEMO,
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	{
		"hdb",
		"Handango Demo",
		AD_ENTRY1s("hyperdemo.msd", "2d4457b284a940b7058b36e5706b9951", 3094241),
		Common::EN_ANY,
		Common::kPlatformPocketPC,
		(ADGF_DEMO | GF_HANDANGO),
		GUIO2(GUIO_NOMIDI,GAMEOPTION_CHEATMODE)
	},
	AD_TABLE_END_MARKER
};

} // End of namespace HDB

static const ADExtraGuiOptionsMap optionsList[] = {
		{
				GAMEOPTION_CHEATMODE,
				{
						_s("Enable cheat mode"),
						_s("Debug info and level selection becomes available"),
						"hypercheat",
						false
				}
		},

		AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class HDBMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	HDBMetaEngineDetection() : AdvancedMetaEngineDetection(HDB::gameDescriptions, sizeof(ADGameDescription), hdbGames, optionsList) {
	}

	const char *getEngineId() const override {
		return "hdb";
	}

	const char *getName() const override {
		return "Hyperspace Delivery Boy!";
	}

	const char *getOriginalCopyright() const override {
		return "Hyperspace Delivery Boy! (C) 2001 Monkeystone Games";
	}
};

REGISTER_PLUGIN_STATIC(HDB_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, HDBMetaEngineDetection);
