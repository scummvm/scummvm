/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "base/plugins.h"

//#include "common/config-manager.h"
//#include "common/file.h"
//#include "common/md5.h"
//#include "common/str-array.h"
//#include "common/util.h"
#include "engines/advancedDetector.h"

static const PlainGameDescriptor waynesworldGames[] = {
	{ "waynesworld", "Wayne's World" },
	{ nullptr, nullptr }
};

#include "waynesworld/detection_tables.h"
#include "waynesworld/detection.h"

static const DebugChannelDef debugFlagList[] = {
	{WaynesWorld::kDebugScripts, "scripts", "Script debug level"},
	{WaynesWorld::kDebugMessages, "messages", "Messages debug level"},
	{WaynesWorld::kDebugCore, "core", "Core debug level"},
	DEBUG_CHANNEL_END};

static const char *const DIRECTORY_GLOBS[] = { "vnm", "m01", nullptr };

class WaynesWorldMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	WaynesWorldMetaEngineDetection() : AdvancedMetaEngineDetection(WaynesWorld::gameDescriptions, waynesworldGames) {
		_maxScanDepth = 3;
		_directoryGlobs = DIRECTORY_GLOBS;
		_flags = kADFlagMatchFullPaths;
	}

	~WaynesWorldMetaEngineDetection() override {}

	const char *getName() const override {
		return "waynesworld";
	}

	const char *getEngineName() const override {
		return "WaynesWorld";
	}

	const char *getOriginalCopyright() const override {
		return "Wayne's World (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(WAYNESWORLD_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, WaynesWorldMetaEngineDetection);
