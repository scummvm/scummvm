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

#ifndef HPL1_DETECTION_H
#define HPL1_DETECTION_H

#include "engines/advancedDetector.h"

namespace Hpl1 {

enum Hpl1DebugChannels {
	kDebugGraphics = 1 << 0,
	kDebugPath = 1 << 1,
	kDebugScan = 1 << 2,
	kDebugFilePath = 1 << 3,
	kDebugScript = 1 << 4
};

extern const PlainGameDescriptor GAME_NAMES[];

extern const ADGameDescription GAME_DESCRIPTIONS[];

} // namespace Hpl1

class Hpl1MetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	Hpl1MetaEngineDetection();
	~Hpl1MetaEngineDetection() override {}

	const char *getName() const override {
		return "hpl1";
	}

	const char *getEngineName() const override {
		return "HPL1";
	}

	const char *getOriginalCopyright() const override {
		return "HPL1 (C) Frictional Games AB";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	PlainGameList getSupportedGames() const override;

	PlainGameDescriptor findGame(const char *gameid);
};

#endif
