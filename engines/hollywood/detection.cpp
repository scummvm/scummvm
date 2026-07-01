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

#include "hollywood/hollywood.h"

namespace Hollywood {

const PlainGameDescriptor plainGameDescriptors[] = {
	{"hollywood", "Hollywood Monsters"},
	{nullptr, nullptr}
};

const DebugChannelDef debugFlagList[] = {
	{Hollywood::kDebugGeneral, "general", "General debug level"},
	{Hollywood::kDebugResources, "resources", "Resource archive loading"},
	{Hollywood::kDebugScene, "scene", "Scene state and presentation"},
	{Hollywood::kDebugPath, "path", "Pathfinding and actor route tracing"},
	DEBUG_CHANNEL_END
};

const ADGameDescription gameDescriptions[] = {
	// Spanish Windows full game, installed MONSTERS directory.
	{
		"hollywood",
		nullptr,
		AD_ENTRY3s("MONSTERS.EXE", "5b097e04e6ca7225d9dea6cd99909f6d", 1431552,
			"RESOURCE.000", "bf580b6b3eedf45e427bb83ab29ccd21", 4320424,
			"RESOURCE.003", "352126965c51832ce970faece65cc2f8", 9444153),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

class HollywoodMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	HollywoodMetaEngineDetection() : AdvancedMetaEngineDetection(Hollywood::gameDescriptions, plainGameDescriptors) {
	}

	const char *getName() const override {
		return "hollywood";
	}

	const char *getEngineName() const override {
		return "Hollywood Monsters";
	}

	const char *getOriginalCopyright() const override {
		return "Hollywood Monsters (C) 1997 Pendulo Studios";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace Hollywood

REGISTER_PLUGIN_STATIC(HOLLYWOOD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Hollywood::HollywoodMetaEngineDetection);
