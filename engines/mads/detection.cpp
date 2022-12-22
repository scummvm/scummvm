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
 *
 */

#include "base/plugins.h"
#include "common/str-array.h"
#include "engines/advancedDetector.h"
#include "common/system.h"

#include "mads/detection.h"
#include "mads/mads.h"

static const PlainGameDescriptor MADSGames[] = {
	{"dragonsphere", "Dragonsphere"},
	{"nebular", "Rex Nebular and the Cosmic Gender Bender"},
	{"phantom", "Return of the Phantom"},
	{"forest", "Once Upon a Forest"},
	{nullptr, nullptr}
};

static const DebugChannelDef debugFlagList[] = {
	{MADS::kDebugPath, "Path", "Pathfinding debug level"},
	{MADS::kDebugScripts, "scripts", "Game scripts"},
	{MADS::kDebugGraphics, "graphics", "Graphics handling"},
	DEBUG_CHANNEL_END
};

#include "mads/detection_tables.h"

class MADSMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	MADSMetaEngineDetection() : AdvancedMetaEngineDetection(MADS::gameDescriptions, sizeof(MADS::MADSGameDescription), MADSGames) {
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "mads";
	}

	const char *getEngineName() const override {
		return "MADS";
	}

	const char *getOriginalCopyright() const override {
		return "MADS (C) Microprose";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(MADS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, MADSMetaEngineDetection);
