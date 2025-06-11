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
#include "common/debug-channels.h"
#include "common/scummsys.h"
#include "common/str-array.h"
#include "engines/advancedDetector.h"
#include "zvision/detection.h"
#include "zvision/detection_tables.h"

namespace ZVision {

static const DebugChannelDef debugFlagList[] = {
	{ ZVision::kDebugScript, "Script", "Show debug messages for script file parsing" },
	{ ZVision::kDebugLoop, "Loop", "Show debug messages for main game logic cycle" },
	{ ZVision::kDebugPuzzle, "Puzzle", "Show debug messages for puzzle processing" },
	{ ZVision::kDebugAction, "Action", "Show debug messages for action processing" },
	{ ZVision::kDebugControl, "Control", "Show debug messages for control processing" },
	{ ZVision::kDebugEffect, "Effect", "Show debug messages for effect processing" },
	{ ZVision::kDebugGraphics, "Graphics", "Show debug messages for graphics compositing & rendering" },
	{ ZVision::kDebugVideo, "Video", "Show debug messages for video decoding & playback" },
	{ ZVision::kDebugSound, "Sound", "Show debug messages for sound & music" },
	{ ZVision::kDebugSubtitle, "Subtitle", "Show debug messages for subtitles" },
	{ ZVision::kDebugFile, "File", "Show debug messages for file search & load operations" },
	{ ZVision::kDebugMouse, "Mouse", "Print coordinates of mouse clicks"},
	{ ZVision::kDebugAssign, "Assign", "Print new slotkey values when changed by an assignment action"},
	{ ZVision::kDebugEvent, "Event", "Show debug messages for event processing"},
	DEBUG_CHANNEL_END
};

class ZVisionMetaEngineDetection : public AdvancedMetaEngineDetection<ZVision::ZVisionGameDescription> {
public:
	ZVisionMetaEngineDetection() : AdvancedMetaEngineDetection(ZVision::gameDescriptions, ZVision::zVisionGames) {
		_maxScanDepth = 2;
		_directoryGlobs = ZVision::directoryGlobs;
	}

	const char *getName() const override {
		return "zvision";
	}

	const char *getEngineName() const override {
		return "Z-Vision";
	}

	const char *getOriginalCopyright() const override {
		return "Z-Vision (C) 1996 Activision";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

} // End of namespace ZVision

REGISTER_PLUGIN_STATIC(ZVISION_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ZVision::ZVisionMetaEngineDetection);
