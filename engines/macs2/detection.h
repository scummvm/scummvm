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

#ifndef MACS2_DETECTION_H
#define MACS2_DETECTION_H

#include "engines/advancedDetector.h"

namespace Macs2 {

enum Macs2DebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath     = 1 << 1,
	kDebugScan     = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript   = 1 << 4,
};

extern const PlainGameDescriptor macs2Games[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Macs2

class Macs2MetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	Macs2MetaEngineDetection();
	~Macs2MetaEngineDetection() override {}

	const char *getName() const override {
		return "macs2";
	}

	const char *getEngineName() const override {
		return "Macs2";
	}

	const char *getOriginalCopyright() const override {
		return "Macs2 (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // MACS2_DETECTION_H
