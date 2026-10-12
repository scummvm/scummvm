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

#ifndef IGOR_DETECTION_H
#define IGOR_DETECTION_H

#include "engines/advancedDetector.h"

namespace Igor {

enum {
	kFlagDemo = 1 << 0,
	kFlagFloppy = 1 << 1,
	kFlagTalkie = 1 << 2
};

enum {
	kIdEngDemo100,
	kIdEngDemo110,
	kIdEngFloppy,
	kIdSpaFloppy,
	kIdEngCD,
	kIdSpaCD
};

struct IgorGameDescription {
	ADGameDescription desc;
	int gameVersion;
	int gameFlags;
	const char *ovlFileName;
	const char *sfxFileName;

	uint32 sizeBuffer() const {
		return desc.sizeBuffer() + ADDynamicDescription::strSizeBuffer(ovlFileName) +
			ADDynamicDescription::strSizeBuffer(sfxFileName);
	}

	void *toBuffer(void *buffer) {
		buffer = desc.toBuffer(buffer);
		buffer = ADDynamicDescription::strToBuffer(buffer, ovlFileName);
		return ADDynamicDescription::strToBuffer(buffer, sfxFileName);
	}
};

enum IgorDebugChannels {
	kDebugGraphics = 1,
	kDebugPath,
	kDebugScan,
	kDebugFilePath,
	kDebugScript,

	kDebugEngine   = 400 << 1,
	kDebugResource = 400 << 2,
	kDebugScreen   = 400 << 3,
	kDebugWalk     = 400 << 4,
	kDebugGame     = 400 << 5,
};

extern const PlainGameDescriptor igorGames[];

extern const IgorGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Igor

class IgorMetaEngineDetection : public AdvancedMetaEngineDetection<Igor::IgorGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	IgorMetaEngineDetection();
	~IgorMetaEngineDetection() override {}

	const char *getName() const override {
		return "igor";
	}

	const char *getEngineName() const override {
		return "Igor";
	}

	const char *getOriginalCopyright() const override {
		return "Igor (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // IGOR_DETECTION_H
