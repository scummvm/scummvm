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

#ifndef M4_DETECTION_H
#define M4_DETECTION_H

#include "engines/advancedDetector.h"

namespace M4 {

enum M4DebugChannels {
	kDebugScript = 1 << 0,
	kDebugConversations = 1 << 1,
	kDebugGraphics = 1 << 2,
	kDebugSound = 1 << 3,
	kDebugCore = 1 << 4,
	kDebugWSSequ = 1 << 5,
	kDebugWSMach = 1 << 6
};

enum M4GameType {
	GType_Riddle = 1,
	GType_Burger = 2
};

enum Features {
	kFeaturesNone = 0,
	kFeaturesCD = 1 << 0,
	kFeaturesDemo = 1 << 1
};

struct M4GameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace M4

class M4MetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	M4MetaEngineDetection();
	~M4MetaEngineDetection() override {}

	const char *getName() const override {
		return "m4";
	}

	const char *getEngineName() const override {
		return "M4";
	}

	const char *getOriginalCopyright() const override {
		return "M4 (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // M4_DETECTION_H
