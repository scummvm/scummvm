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

#include "common/system.h"

#include "engines/advancedDetector.h"

#include "pink/pink.h"

static const PlainGameDescriptor pinkGames[] = {
	{"peril", "The Pink Panther: Passport to Peril"},
	{"pokus", "The Pink Panther: Hokus Pokus Pink"},
	{nullptr, nullptr}
};

#include "pink/detection_tables.h"

static const char *directoryGlobs[] = {
	"install",
	nullptr
};

static const DebugChannelDef debugFlagList[] = {
	{Pink::kPinkDebugGeneral, "general", "General issues"},
	{Pink::kPinkDebugLoadingResources, "loading_resources", "Loading resources data"},
	{Pink::kPinkDebugLoadingObjects, "loading_objects", "Serializing objects from Orb"},
	{Pink::kPinkDebugScripts, "scripts", "Sequences"},
	{Pink::kPinkDebugActions, "actions", "Actions"},
	DEBUG_CHANNEL_END
};


class PinkMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PinkMetaEngineDetection() : AdvancedMetaEngineDetection(Pink::gameDescriptions, sizeof(ADGameDescription), pinkGames) {
		_gameIds = pinkGames;
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "pink";
	}

	const char *getEngineName() const override {
		return "Pink Panther";
	}

	const char *getOriginalCopyright() const override {
		return "Pink Panther (C) Wanderlust Interactive";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(PINK_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PinkMetaEngineDetection);
