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

#include "base/plugins.h"
#include "common/file.h"
#include "common/gui_options.h"
#include "common/translation.h"
#include "engines/advancedDetector.h"
#include "supernova/supernova.h"

#define GAMEOPTION_IMPROVED GUIO_GAMEOPTIONS1
#define GAMEOPTION_TTS		GUIO_GAMEOPTIONS2

static const DebugChannelDef debugFlagList[] = {
	{Supernova::kDebugGeneral, "general", "Supernova general debug channel"},
	DEBUG_CHANNEL_END
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_IMPROVED,
		{
			_s("Improved mode"),
			_s("Removes some repetitive actions, adds possibility to change verbs by keyboard"),
			"improved",
			true
		}
	},

	{
		GAMEOPTION_TTS,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled",
			false
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const PlainGameDescriptor supernovaGames[] = {
	{"msn1", "Mission Supernova 1"},
	{"msn2", "Mission Supernova 2"},
	{nullptr, nullptr}
};

namespace Supernova {
static const ADGameDescription gameDescriptions[] = {
	// Mission Supernova 1
	{
		"msn1",
		nullptr,
		AD_ENTRY1s("msn_data.000", "f64f16782a86211efa919fbae41e7568", 24163),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GAMEOPTION_TTS, GAMEOPTION_IMPROVED, GUIO_NOMIDI)
	},
	{
		"msn1",
		nullptr,
		AD_ENTRY1s("msn_data.000", "f64f16782a86211efa919fbae41e7568", 24163),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GAMEOPTION_TTS, GAMEOPTION_IMPROVED, GUIO_NOMIDI)
	},
	{
		"msn1",
		nullptr,
		AD_ENTRY1s("msn_data.000", "f64f16782a86211efa919fbae41e7568", 24163),
		Common::IT_ITA,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GAMEOPTION_TTS, GAMEOPTION_IMPROVED, GUIO_NOMIDI)
	},
	// Mission Supernova 2
	{
		"msn2",
		nullptr,
		AD_ENTRY1s("ms2_data.000", "e595610cba4a6d24a763e428d05cc83f", 24805),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GAMEOPTION_TTS, GAMEOPTION_IMPROVED, GUIO_NOMIDI)
	},
	{
		"msn2",
		nullptr,
		AD_ENTRY1s("ms2_data.000", "e595610cba4a6d24a763e428d05cc83f", 24805),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO3(GAMEOPTION_TTS, GAMEOPTION_IMPROVED, GUIO_NOMIDI)
	},
	AD_TABLE_END_MARKER
};
}

class SupernovaMetaEngineDetection: public AdvancedMetaEngineDetection {
public:
	SupernovaMetaEngineDetection() : AdvancedMetaEngineDetection(Supernova::gameDescriptions, sizeof(ADGameDescription), supernovaGames, optionsList) {
	}

	const char *getEngineId() const override {
		return "supernova";
	}

	const char *getName() const override {
		return "Mission Supernova";
	}

	const char *getOriginalCopyright() const override {
		return "Mission Supernova (C) 1994 Thomas and Steffen Dingel";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};


REGISTER_PLUGIN_STATIC(SUPERNOVA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, SupernovaMetaEngineDetection);
