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

#include "made/detection.h"

#include "common/config-manager.h"

static const PlainGameDescriptor madeGames[] = {
	{"manhole", "The Manhole"},
	{"rtz", "Return to Zork"},
	{"lgop2", "Leather Goddesses of Phobos 2"},
	{"rodney", "Rodney's Funscreen"},
	{nullptr, nullptr}
};

#include "made/detection_tables.h"

namespace Made {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_INTRO_MUSIC_DIGITAL,
		{
			_s("Play a digital soundtrack during the opening movie"),
			_s("If selected, the game will use a digital soundtrack during the introduction. Otherwise, it will play MIDI music."),
			"intro_music_digital",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Made

class MadeMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MadeMetaEngineDetection() : AdvancedMetaEngineDetection(Made::gameDescriptions, sizeof(Made::MadeGameDescription), madeGames, Made::optionsList) {
	}

	const char *getName() const override {
		return "made";
	}

	const char *getEngineName() const override {
		return "MADE";
	}

	const char *getOriginalCopyright() const override {
		return "MADE Engine (C) Activision";
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;
};

ADDetectedGame MadeMetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	// Set the default values for the fallback descriptor's ADGameDescription part.
	Made::g_fallbackDesc.desc.language = Common::UNK_LANG;
	Made::g_fallbackDesc.desc.platform = Common::kPlatformDOS;
	Made::g_fallbackDesc.desc.flags = ADGF_NO_FLAGS;

	// Set default values for the fallback descriptor's MadeGameDescription part.
	Made::g_fallbackDesc.gameID = 0;
	Made::g_fallbackDesc.features = 0;
	Made::g_fallbackDesc.version = 3;

	//return (const ADGameDescription *)&Made::g_fallbackDesc;
	return ADDetectedGame();
}

REGISTER_PLUGIN_STATIC(MADE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MadeMetaEngineDetection);
