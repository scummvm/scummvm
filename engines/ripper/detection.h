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

#ifndef RIPPER_DETECTION_H
#define RIPPER_DETECTION_H

#include "engines/advancedDetector.h"

#define GAMEOPTION_SKIP_INTRO GUIO_GAMEOPTIONS1
#define GAMEOPTION_SUBTITLES GUIO_GAMEOPTIONS2
#define GAMEOPTION_SUBTITLE_AUTOSCROLL GUIO_GAMEOPTIONS3
#define GAMEOPTION_EASTER_EGG GUIO_GAMEOPTIONS4

namespace Ripper {

enum RipperDebugChannels {
	kDebugGeneral = 1,
	kDebugResources,
	kDebugScripts,
	kDebugScene,
	kDebugVideo,
	kDebugInput,
	kDebugCursor,
	kDebugAudio,
	kDebugDialogue,
	kDebugWac,
	kDebugSaveLoad,
	kDebugMilestones,
	kDebugPuzzles,
	kDebugCyber,
	kDebugCombat
};

extern const PlainGameDescriptor ripperGames[];
extern const ADGameDescription gameDescriptions[];

} // End of namespace Ripper

class RipperMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	RipperMetaEngineDetection();

	const char *getName() const override {
		return "ripper";
	}

	const char *getEngineName() const override {
		return "Ripper";
	}

	const char *getOriginalCopyright() const override {
		return "Ripper (C) Take-Two Interactive";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // RIPPER_DETECTION_H
