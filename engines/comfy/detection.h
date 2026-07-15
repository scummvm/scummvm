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

#ifndef COMFY_DETECTION_H
#define COMFY_DETECTION_H

#include "engines/advancedDetector.h"

namespace Comfy {

enum ComfyAction {
	kActionNone,
	kActionKeyboardContact0,
	kActionKeyboardContactLast = kActionKeyboardContact0 + 23
};

enum ComfyDebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,
};

struct ComfyGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;
	byte version;
	byte scriptFeatures;
	uint16 dirtyRectMergeThreshold;
};

#define COMFY_SCRIPT_OPCODE_31 (1 << 0)
#define COMFY_SCRIPT_OPCODE_32 (1 << 1)
#define COMFY_SCRIPT_OPCODE_33 (1 << 2)

extern const PlainGameDescriptor comfyGames[];

extern const ComfyGameDescription gameDescriptions[];

#define GAMEOPTION_FORCE_LANGUAGE_SETUP GUIO_GAMEOPTIONS1
#define GAMEOPTION_SKIP_INTRO           GUIO_GAMEOPTIONS2
#define GAMEOPTION_USE_IMGUI_KEYBOARD   GUIO_GAMEOPTIONS3

} // End of namespace Comfy

class ComfyMetaEngineDetection : public AdvancedMetaEngineDetection<Comfy::ComfyGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	ComfyMetaEngineDetection();
	~ComfyMetaEngineDetection() override {}

	const char *getName() const override {
		return "comfy";
	}

	const char *getEngineName() const override {
		return "Comfy";
	}

	const char *getOriginalCopyright() const override {
		return "Comfy Interactive Movies, Ltd. (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // COMFY_DETECTION_H
