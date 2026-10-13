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
#include "engines/game.h"

#include "hollywood/debug.h"
#include "hollywood/monsters/detection.h"

namespace Hollywood {

const PlainGameDescriptor plainGameDescriptors[] = {
	{"hollywood", "Hollywood Monsters"},
	{"runaway", "Runaway: A Road Adventure"},
	{nullptr, nullptr}
};

const char *const directoryGlobs[] = {
	"resource",
	nullptr
};

const DebugChannelDef debugFlagList[] = {
	{Hollywood::kDebugGeneral, "general", "General debug level"},
	{Hollywood::kDebugResources, "resources", "Resource archive loading"},
	{Hollywood::kDebugScene, "scene", "Scene state and presentation"},
	{Hollywood::kDebugPath, "path", "Pathfinding and actor route tracing"},
	DEBUG_CHANNEL_END
};

const ADGameDescription gameDescriptions[] = {
#include "hollywood/runaway/detection_tables.h"
#include "hollywood/monsters/detection_tables.h"

	AD_TABLE_END_MARKER
};

class HollywoodMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	HollywoodMetaEngineDetection() : AdvancedMetaEngineDetection(Hollywood::gameDescriptions, plainGameDescriptors) {
		_flags = kADFlagPreferFallbackDetection;
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist,
			ADDetectedGameExtraInfo **extra) const override {
		ADDetectedGame game = detectGameFilebased(allFiles, monstersFileBasedFallback);
		if (!game.desc)
			return ADDetectedGame();

		game.desc = detectMonstersVariant(allFiles);
		if (!game.desc)
			return ADDetectedGame();

		return game;
	}

	const char *getName() const override {
		return "hollywood";
	}

	const char *getEngineName() const override {
		return "Hollywood Monsters & Runaway";
	}

	const char *getOriginalCopyright() const override {
		return "Hollywood Monsters and Runaway (C) Pendulo Studios";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace Hollywood

REGISTER_PLUGIN_STATIC(HOLLYWOOD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hollywood::HollywoodMetaEngineDetection);
