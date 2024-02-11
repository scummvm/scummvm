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

#ifndef TWP_DETECTION_H
#define TWP_DETECTION_H

#include "engines/advancedDetector.h"

namespace Twp {

enum TwpDebugChannels {
	kDebugText	     	= 1 << 0,
	kDebugGGPack     	= 1 << 1,
	kDebugRes     		= 1 << 2,
	kDebugDialog 		= 1 << 3,
	kDebugGenScript 	= 1 << 4,
	kDebugObjScript 	= 1 << 5,
	kDebugSysScript 	= 1 << 6,
	kDebugRoomScript	= 1 << 7,
	kDebugActScript		= 1 << 8,
	kDebugSndScript		= 1 << 9,
	kDebugGame			= 1 << 10,
};

extern const PlainGameDescriptor twpGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Twp

class TwpMetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	TwpMetaEngineDetection();
	~TwpMetaEngineDetection() override = default;

	const char *getName() const override {
		return "twp";
	}

	const char *getEngineName() const override {
		return "Thimbleweed Park";
	}

	const char *getOriginalCopyright() const override {
		return "Thimbleweed Park (C) Terrible Toybox";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

private:
	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;
};

#endif // TWP_DETECTION_H
