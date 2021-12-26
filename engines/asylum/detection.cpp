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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"
#include "base/plugins.h"

#include "asylum/detection_tables.h"
#include "asylum/console.h"

static const DebugChannelDef debugFlagList[] = {
	{Asylum::kDebugLevelMain,      "Main",      "Generic debug level"},
	{Asylum::kDebugLevelResources, "Resources", "Resources debugging"},
	{Asylum::kDebugLevelSprites,   "Sprites",   "Sprites debugging"},
	{Asylum::kDebugLevelInput,     "Input",     "Input events debugging"},
	{Asylum::kDebugLevelMenu,      "Menu",      "Menu debugging"},
	{Asylum::kDebugLevelScripts,   "Scripts",   "Scripts debugging"},
	{Asylum::kDebugLevelSound,     "Sound",     "Sound debugging"},
	{Asylum::kDebugLevelSavegame,  "Savegame",  "Saving & restoring game debugging"},
	{Asylum::kDebugLevelScene,     "Scene",     "Scene process and draw debugging"},
	{Asylum::kDebugLevelObjects,   "Objects",   "Debug Object objects"},
	{Asylum::kDebugLevelActor,     "Actor",     "Debug Actor"},
	{Asylum::kDebugLevelEncounter, "Encounter", "Debug Encounter"},
	{Asylum::kDebugLevelVideo,     "Video",     "Debug Video"},
	{Asylum::kDebugLevelCommands,  "Commands",  "Script commands debugging"},
	DEBUG_CHANNEL_END
};

class AsylumMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AsylumMetaEngineDetection() : AdvancedMetaEngineDetection(Asylum::gameDescriptions, sizeof(ADGameDescription), asylumGames) {
		_md5Bytes = 0;
		_maxScanDepth = 4;
		_directoryGlobs = Asylum::directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override {
		return detectGameFilebased(allFiles, Asylum::fileBasedFallback);
	}

	const char *getEngineId() const override {
		return "asylum";
	}

	const char *getName() const override {
		return "Sanitarium";
	}

	const char *getOriginalCopyright() const override {
		return "Sanitarium (c) ASC Games";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(ASYLUM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AsylumMetaEngineDetection);
