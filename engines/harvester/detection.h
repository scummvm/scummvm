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

#ifndef HARVESTER_DETECTION_H
#define HARVESTER_DETECTION_H

#include "engines/advancedDetector.h"

#define GAMEOPTION_GORE GUIO_GAMEOPTIONS1
#define GAMEOPTION_SHOW_CD_CHANGE_PROMPTS GUIO_GAMEOPTIONS2

namespace Harvester {

enum HarvesterDebugChannels {
	kDebugGeneral   = 1 << 0,
	kDebugCombat    = 1 << 1,
	kDebugResources = 1 << 2,
	kDebugScene     = 1 << 3,
	kDebugDialogue  = 1 << 4,
	kDebugInventory = 1 << 5,
	kDebugCursor    = 1 << 6,
	kDebugRoom      = 1 << 7,
	kDebugPlayer      = 1 << 8,
	kDebugPathfinding = 1 << 9
};

extern const PlainGameDescriptor harvesterGames[];

extern const ADGameDescription gameDescriptions[];

} // End of namespace Harvester

class HarvesterMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	HarvesterMetaEngineDetection();
	~HarvesterMetaEngineDetection() override {}

	const char *getName() const override {
		return "harvester";
	}

	const char *getEngineName() const override {
		return "Harvester";
	}

	const char *getOriginalCopyright() const override {
		return "Harvester (C) DigiFX Interactive";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // HARVESTER_DETECTION_H
