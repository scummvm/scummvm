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

#include "ww/detection_tables.h"
#include "ww/detection.h"

#include "base/plugins.h"
#include "engines/advancedDetector.h"

static const PlainGameDescriptor WWGames[] = {
	{ "waynesworld", "Wayne's World" },
	{"darkhalf", "The Dark Half"},
	{ nullptr, nullptr }
};

static const DebugChannelDef debugFlagList[] = {
	{ WW::kDebugLogic, "logic", "Logic debug level" },
	{ WW::kDebugMessages, "messages", "Messages debug level" },
	{ WW::kDebugCore, "core", "Core debug level" },
	DEBUG_CHANNEL_END
};

static const char *const DIRECTORY_GLOBS[] = { "vnm", "m01", nullptr };

class WWMetaEngineDetection : public AdvancedMetaEngineDetection<WW::WWGameDescription> {
public:
	WWMetaEngineDetection() : AdvancedMetaEngineDetection(WW::gameDescriptions, WWGames) {
		_maxScanDepth = 3;
		_directoryGlobs = DIRECTORY_GLOBS;
		_flags = kADFlagMatchFullPaths;
	}

	~WWMetaEngineDetection() override {}

	const char *getName() const override {
		return "ww";
	}

	const char *getEngineName() const override {
		return "WW engine";
	}

	const char *getOriginalCopyright() const override {
		return "Wayne's World (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(WW_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, WWMetaEngineDetection);
