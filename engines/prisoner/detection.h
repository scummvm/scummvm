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

#ifndef PRISONER_DETECTION_H
#define PRISONER_DETECTION_H

#include "engines/advancedDetector.h"

namespace Prisoner {

enum PrisonerDebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,
};

extern const PlainGameDescriptor prisonerGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

// "music type" setting: digital (the default) or MIDI music
#define GAMEOPTION_MIDI_MUSIC GUIO_GAMEOPTIONS2

} // End of namespace Prisoner

class PrisonerMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	PrisonerMetaEngineDetection();
	~PrisonerMetaEngineDetection() override {}

	const char *getName() const override {
		return "prisoner";
	}

	const char *getEngineName() const override {
		return "Prisoner Engine";
	}

	const char *getOriginalCopyright() const override {
		return "Prisoner Engine (C) Infogrames";
	}
};

#endif // PRISONER_DETECTION_H
