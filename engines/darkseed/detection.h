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

#ifndef DARKSEED_DETECTION_H
#define DARKSEED_DETECTION_H

#include "engines/advancedDetector.h"

namespace Darkseed {

enum DarkseedDebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,
};

extern const PlainGameDescriptor darkseedGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Darkseed

class DarkseedMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	DarkseedMetaEngineDetection();
	~DarkseedMetaEngineDetection() override {}

	const char *getName() const override {
		return "darkseed";
	}

	const char *getEngineName() const override {
		return "Darkseed";
	}

	const char *getOriginalCopyright() const override {
		return "(C) 1992 CYBERDREAMS, INC.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // DARKSEED_DETECTION_H
