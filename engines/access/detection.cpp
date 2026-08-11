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
#include "access/detection.h"
#include "access/access.h"

static const PlainGameDescriptor AccessGames[] = {
	{"amazon", "Amazon: Guardians of Eden"},
	{"martian", "Martian Memorandum"},
	{"noctropolis", "Noctropolis"},
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{Access::kDebugPath, "path", "Pathfinding debug level"},
	{Access::kDebugScripts, "scripts", "Game scripts"},
	{Access::kDebugGraphics, "graphics", "Graphics handling"},
	{Access::kDebugSound, "sound", "Sound and Music handling"},
	DEBUG_CHANNEL_END
};

#include "access/detection_tables.h"

class AccessMetaEngineDetection : public AdvancedMetaEngineDetection<Access::AccessGameDescription> {
public:
	AccessMetaEngineDetection() : AdvancedMetaEngineDetection(Access::gameDescriptions, AccessGames) {
		static const char *const DIRECTORY_GLOBS[3] = { "dark", "demo", 0 };
		_directoryGlobs = DIRECTORY_GLOBS;
		_flags = kADFlagMatchFullPaths;
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "access";
	}

	const char *getEngineName() const override {
		return "Access";
	}

	const char *getOriginalCopyright() const override {
		return "Access Engine (C) 1989-1994 Access Software";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override {
		static const Common::Language NOCTROPOLIS_RERELEASE_LANGS[] = {
			Common::EN_ANY,
			Common::FR_FRA,
			Common::DE_DEU,
			Common::ES_ESP,
		};
		DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame);

		const Access::AccessGameDescription *desc = reinterpret_cast<const Access::AccessGameDescription *>(adGame.desc);
		assert(desc);

		if (desc && (desc->features & Access::FEATURE_NOCT_MULTI_LANG)) {
			// The AdvancedDetector model only allows specifying a single supported
			// game language. The Noctropolis re-release supports multiple languages.
			for (const auto lang: NOCTROPOLIS_RERELEASE_LANGS) {
				game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(lang));
			}
		}

		return game;
	}

};


REGISTER_PLUGIN_STATIC(ACCESS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AccessMetaEngineDetection);
