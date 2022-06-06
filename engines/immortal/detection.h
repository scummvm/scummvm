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

#ifndef IMMORTAL_DETECTION_H
#define IMMORTAL_DETECTION_H

#include "engines/advancedDetector.h"

namespace Immortal {

enum ImmortalDebugChannels {
	kDebugTest = 1 << 0
};

extern const PlainGameDescriptor immortalGames[];
extern const ADGameDescription gameDescriptions[];

} // namespace Immortal

class ImmortalMetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	ImmortalMetaEngineDetection();
	~ImmortalMetaEngineDetection() override {}

	const char *getName() const override {
		return "immortal";
	}

	const char *getEngineName() const override {
		return "The Immortal";
	}

	const char *getOriginalCopyright() const override {
		return "(c)1990 Will Harvey & Electronic Arts";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif
