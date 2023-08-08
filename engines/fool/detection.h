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

#ifndef FOOL_DETECTION_H
#define FOOL_DETECTION_H

#include "engines/advancedDetector.h"

namespace Fool {

enum FoolDebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath     = 1 << 1,
	kDebugScan     = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript   = 1 << 4,
};

extern const PlainGameDescriptor foolGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Fool

class FoolMetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	FoolMetaEngineDetection();
	~FoolMetaEngineDetection() override {}

	const char *getName() const override {
		return "fool";
	}

	const char *getEngineName() const override {
		return "Fool";
	}

	const char *getOriginalCopyright() const override {
		return "Fool (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // FOOL_DETECTION_H
