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
#include "engines/advancedDetector.h"

#include "common/text-to-speech.h"
#include "common/translation.h"

static const PlainGameDescriptor griffonGames[] = {
	{"griffon", "The Griffon Legend"},
	{nullptr, nullptr}
};

#ifdef USE_TTS

#define GAMEOPTION_TTS_NARRATOR 	GUIO_GAMEOPTIONS1

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_TTS_NARRATOR,
		{
			_s("Enable Text to Speech"),
			_s("Use TTS to read the descriptions (if TTS is available)"),
			"tts_enabled",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

#endif

namespace Griffon {

static const ADGameDescription gameDescriptions[] = {
	{
		"griffon",
		nullptr,
		AD_ENTRY1s("objectdb.dat", "ec5371da28f01ccf88980b32d9de2232", 27754),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
#ifdef USE_TTS
		GUIO2(GUIO_NOMIDI, GAMEOPTION_TTS_NARRATOR)
#else
		GUIO1(GUIO_NOMIDI)
#endif
	},

	AD_TABLE_END_MARKER
};

}

class GriffonMetaEngineDetection: public AdvancedMetaEngineDetection {
public:
	GriffonMetaEngineDetection() : AdvancedMetaEngineDetection(Griffon::gameDescriptions, sizeof(ADGameDescription), griffonGames
#ifdef USE_TTS
			, optionsList
#endif
			) {
	}

	const char *getName() const override {
		return "griffon";
	}

	const char *getEngineName() const override {
		return "Griffon Engine";
	}

	const char *getOriginalCopyright() const override {
		return "The Griffon Legend (c) 2005 Syn9 (Daniel Kennedy)";
	}
};

REGISTER_PLUGIN_STATIC(GRIFFON_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, GriffonMetaEngineDetection);
