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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_DETECTION_H
#define RIPPER_DETECTION_H

#include "engines/advancedDetector.h"

namespace Ripper {

enum RipperDebugChannels {
	kDebugGeneral = 1 << 0,
	kDebugResources = 1 << 1,
	kDebugScripts = 1 << 2,
	kDebugScene = 1 << 3,
	kDebugVideo = 1 << 4,
	kDebugInput = 1 << 5,
	kDebugCursor = 1 << 6
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
