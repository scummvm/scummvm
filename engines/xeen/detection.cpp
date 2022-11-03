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

#include "xeen/detection.h"
#include "xeen/xeen.h"

static const PlainGameDescriptor XeenGames[] = {
	{ "cloudsofxeen", "Might and Magic IV: Clouds of Xeen" },
	{ "darksideofxeen", "Might and Magic V: Darkside of Xeen" },
	{ "worldofxeen", "Might and Magic: World of Xeen" },
	{ "swordsofxeen", "Might and Magic: Swords of Xeen" },
	{0, 0}
};

static const DebugChannelDef debugFlagList[] = {
	{Xeen::kDebugPath, "Path", "Pathfinding debug level"},
	{Xeen::kDebugScripts, "scripts", "Game scripts"},
	{Xeen::kDebugGraphics, "graphics", "Graphics handling"},
	{Xeen::kDebugSound, "sound", "Sound processing"},
	DEBUG_CHANNEL_END
};

#include "xeen/detection_tables.h"

class XeenMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	XeenMetaEngineDetection() : AdvancedMetaEngineDetection(Xeen::gameDescriptions, sizeof(Xeen::XeenGameDescription),
			XeenGames) {
		_maxScanDepth = 3;
	}

	const char *getName() const override {
		return "xeen";
	}

	const char *getEngineName() const override {
		return "Xeen";
	}

	const char *getOriginalCopyright() const override {
		return "Xeen (C) 1992-1993 New World Computing, Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(XEEN_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, XeenMetaEngineDetection);
