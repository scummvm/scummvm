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
	kDebugScripts = 1 << 0,
	kDebugMessages = 1 << 1,
	kDebugCore = 1 << 2
};

enum M4GameType {
	GType_Riddle = 1,
	GType_Burger = 2
};

enum M4GameStyle {
	GStyle_Game = 0,
	GStyle_Demo = 1,
	GStyle_NonInteractiveDemo = 2
};

enum Features {
	kFeaturesNone = 0,
	kFeaturesCD = 1 << 0,
	kFeaturesDemo = 1 << 1,
	kFeaturesNonInteractiveDemo = 1 << 2
};

struct M4GameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;

	int gameType;
	uint32 features;
};

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace M4

class M4MetaEngineDetection : public AdvancedMetaEngineDetection<M4::M4GameDescription> {
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
		return "M4 (C) 1995-1996 Sanctuary Woods Multimedia Corporation";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // M4_DETECTION_H
