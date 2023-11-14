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

#ifndef ULTIMA_DETECTION
#define ULTIMA_DETECTION

#include "engines/advancedDetector.h"

namespace Ultima {

enum GameId {
	GAME_AKALABETH,
	GAME_ULTIMA1,
	GAME_ULTIMA2,
	GAME_ULTIMA3,
	GAME_ULTIMA4,
	GAME_ULTIMA5,
	GAME_ULTIMA6,
	GAME_SAVAGE_EMPIRE,
	GAME_MARTIAN_DREAMS,
	GAME_ULTIMA_UNDERWORLD1,
	GAME_ULTIMA_UNDERWORLD2,
	GAME_ULTIMA7
};

enum UltimaGameFeatures {
	GF_VGA_ENHANCED = 1 << 0
};

struct UltimaGameDescription {
	ADGameDescription desc;
	GameId gameId;
	uint32 features;
};

} // End of namespace Ultima

class UltimaMetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	UltimaMetaEngineDetection();
	~UltimaMetaEngineDetection() override {}

	const char *getName() const override {
		return "ultima";
	}

	const char *getEngineName() const override {
		return "Ultima";
	}

	const char *getOriginalCopyright() const override {
		return "Ultima Games (C) 1980-1995 Origin Systems Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif
