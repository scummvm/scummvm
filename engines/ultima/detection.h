/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_DETECTION
#define ULTIMA_DETECTION

#include "engines/advancedDetector.h"

#define MAX_SAVES 99

namespace Ultima {

enum GameId {
	GAME_AKALABETH,
	GAME_ULTIMA1,
	GAME_ULTIMA2,
	GAME_ULTIMA3,
	GAME_ULTIMA4,
	GAME_ULTIMA5,
	GAME_ULTIMA6,
	GAME_ULTIMA6_ENHANCED,
	GAME_SAVAGE_EMPIRE,
	GAME_MARTIAN_DREAMS,
	GAME_ULTIMA_UNDERWORLD1,
	GAME_ULTIMA_UNDERWORLD2,
	GAME_ULTIMA7,
	GAME_ULTIMA8
	// There is no game after Ultima 8. Nope.. none at all.
};

struct UltimaGameDescription {
	ADGameDescription desc;
	GameId gameId;
};

} // End of namespace Ultima8

class UltimaMetaEngine : public AdvancedMetaEngine {
public:
	UltimaMetaEngine();
	virtual ~UltimaMetaEngine();

	virtual const char *getEngineId() const override {
		return "ultima";
	}

	virtual const char *getName() const override {
		return "Ultima";
	}

	virtual const char *getOriginalCopyright() const override {
		return "Ultima games (C) Origin Systems Inc.";
	}

	virtual const char *getSavegamePattern(const char *target = nullptr) const override;
	virtual const char *getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;

	virtual bool hasFeature(MetaEngineFeature f) const override;
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	virtual int getMaximumSaveSlot() const override;
};

namespace Ultima {
extern UltimaMetaEngine *g_metaEngine;
}; // End of namespace Ultima

#endif
