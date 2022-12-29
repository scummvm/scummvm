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
	GAME_ULTIMA7,
	GAME_ULTIMA8,
	GAME_CRUSADER_REG,
	GAME_CRUSADER_REM
	// There is no ultima game after Ultima 8. Nope.. none at all.
};

enum UltimaGameFeatures {
	GF_VGA_ENHANCED = 1 << 0
};

// Custom game detector flags used by Ultima engines
enum UltimaGameFlags {
	// Usecode variants (used by Crusader games)
	ADGF_USECODE_MASK = (0xF | ADGF_DEMO),	// Mask of flags data used for usecode variants
	ADGF_USECODE_DEFAULT = 0,	// Default usecode tables (latest/gog editions of game)
	ADGF_USECODE_DEMO = ADGF_DEMO, // Demo versions of each game (re-use demo flag)
	ADGF_USECODE_ORIG = 1, 		// Original (eg, 1.01) CD version
	ADGF_USECODE_ES = 2,		// Spanish version of game
	ADGF_USECODE_DE = 3, 		// German version of game
	ADGF_USECODE_FR = 4,		// French version of game
	ADGF_USECODE_JA = 5			// Japanese version of game
};

struct UltimaGameDescription {
	ADGameDescription desc;
	GameId gameId;
	uint32 features;
};

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1
#define GAMEOPTION_FRAME_SKIPPING    GUIO_GAMEOPTIONS2
#define GAMEOPTION_FRAME_LIMITING    GUIO_GAMEOPTIONS3
#define GAMEOPTION_CHEATS            GUIO_GAMEOPTIONS4
#define GAMEOPTION_HIGH_RESOLUTION   GUIO_GAMEOPTIONS5
#define GAMEOPTION_FOOTSTEP_SOUNDS   GUIO_GAMEOPTIONS6
#define GAMEOPTION_JUMP_TO_MOUSE     GUIO_GAMEOPTIONS7
#define GAMEOPTION_FONT_REPLACEMENT  GUIO_GAMEOPTIONS8
#define GAMEOPTION_FONT_ANTIALIASING GUIO_GAMEOPTIONS9
#define GAMEOPTION_CAMERA_WITH_SILENCER GUIO_GAMEOPTIONS10
#define GAMEOPTION_ALWAYS_CHRISTMAS     GUIO_GAMEOPTIONS11

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
