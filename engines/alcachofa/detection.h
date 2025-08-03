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

#ifndef ALCACHOFA_DETECTION_H
#define ALCACHOFA_DETECTION_H

#include "engines/advancedDetector.h"

namespace Alcachofa {

enum AlcachofaDebugChannels {
	kDebugGraphics = 1,
	kDebugScript,
	kDebugGameplay,
	kDebugSounds,
	kDebugSemaphores
};

extern const PlainGameDescriptor alcachofaGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_HIGH_QUALITY GUIO_GAMEOPTIONS1 // I should comment what this does, but I don't know
#define GAMEOPTION_32BITS GUIO_GAMEOPTIONS2

} // End of namespace Alcachofa

class AlcachofaMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	AlcachofaMetaEngineDetection();
	~AlcachofaMetaEngineDetection() override {}

	const char *getName() const override {
		return "alcachofa";
	}

	const char *getEngineName() const override {
		return "Alcachofa";
	}

	const char *getOriginalCopyright() const override {
		return "Alcachofa Soft (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // ALCACHOFA_DETECTION_H
