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

#include "tetraedge/detection.h"
#include "tetraedge/metaengine.h"
#include "tetraedge/detection_tables.h"

const DebugChannelDef TetraedgeMetaEngineDetection::debugFlagList[] = {
	{ Tetraedge::kDebugGraphics, "Graphics", "Graphics debug level" },
	{ Tetraedge::kDebugPath, "Path", "Pathfinding debug level" },
	{ Tetraedge::kDebugFilePath, "FilePath", "File path debug level" },
	{ Tetraedge::kDebugScan, "Scan", "Scan for unrecognised games" },
	{ Tetraedge::kDebugScript, "Script", "Enable debug script dump" },
	DEBUG_CHANNEL_END
};

TetraedgeMetaEngineDetection::TetraedgeMetaEngineDetection() : AdvancedMetaEngineDetection(Tetraedge::GAME_DESCRIPTIONS,
	Tetraedge::GAME_NAMES) {
	_flags = kADFlagMatchFullPaths;
}

static const Common::Language *getGameLanguages() {
	static const Common::Language languages[] = {
		Common::EN_ANY,
		Common::FR_FRA,
		Common::DE_DEU,
		Common::IT_ITA,
		Common::ES_ESP,
		Common::RU_RUS,
		Common::HE_ISR,  // This is a Fan-translation, which requires additional patch
		Common::JA_JPN,
		Common::PL_POL,
		Common::UNK_LANG
	};
	return languages;
}

DetectedGame TetraedgeMetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
	DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame);

	// The AdvancedDetector model only allows specifying a single supported
	// game language. All games support multiple languages.  Only Syberia 1
	// supports RU.
	for (const Common::Language *language = getGameLanguages(); *language != Common::UNK_LANG; language++) {
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(*language));
	}

	return game;
}


REGISTER_PLUGIN_STATIC(TETRAEDGE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TetraedgeMetaEngineDetection);
