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

#include "tony/detection.h"
#include "tony/tony.h"

static const PlainGameDescriptor tonyGames[] = {
	{"tony", "Tony Tough and the Night of Roasted Moths"},
	{0, 0}
};

static const DebugChannelDef debugFlagList[] = {
	{Tony::kTonyDebugAnimations, "animations", "Animations debugging"},
	{Tony::kTonyDebugActions, "actions", "Actions debugging"},
	{Tony::kTonyDebugSound, "sound", "Sound debugging"},
	{Tony::kTonyDebugMusic, "music", "Music debugging"},
	DEBUG_CHANNEL_END
};

#include "tony/detection_tables.h"

static const char *const directoryGlobs[] = {
	"roasted",
	"voices",
	0
};

class TonyMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	TonyMetaEngineDetection() : AdvancedMetaEngineDetection(Tony::gameDescriptions, sizeof(Tony::TonyGameDescription), tonyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "tony";
	}

	const char *getEngineName() const override {
		return "Tony Tough and the Night of Roasted Moths";
	}

	const char *getOriginalCopyright() const override {
		return "Tony Tough and the Night of Roasted Moths (C) Protonic Interactive";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(TONY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TonyMetaEngineDetection);
