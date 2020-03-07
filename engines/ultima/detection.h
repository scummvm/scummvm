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
#include "backends/keymapper/keymapper.h"

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
	GAME_SAVAGE_EMPIRE,
	GAME_MARTIAN_DREAMS,
	GAME_ULTIMA_UNDERWORLD1,
	GAME_ULTIMA_UNDERWORLD2,
	GAME_ULTIMA7,
	GAME_ULTIMA8
	// There is no game after Ultima 8. Nope.. none at all.
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

class UltimaMetaEngine : public AdvancedMetaEngine {
private:
	/**
	 * Gets the game Id given a target string
	 */
	static Common::String getGameId(const char *target);
public:
	UltimaMetaEngine();
	~UltimaMetaEngine() override {}

	const char *getEngineId() const override {
		return "ultima";
	}

	const char *getName() const override {
		return "Ultima";
	}

	const char *getOriginalCopyright() const override {
		return "Ultima Games (C) 1980-1995 Origin Systems Inc.";
	}

	const char *getSavegamePattern(const char *target = nullptr) const override;
	const char *getSavegameFile(int saveGameIdx, const char *target = nullptr) const override;

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
	int getMaximumSaveSlot() const override;

	/**
	 * Return a list of all save states associated with the given target.
	 */
	SaveStateList listSaves(const char *target) const override;

	/**
	 * Initialize keymaps
	 */
	Common::KeymapArray initKeymaps(const char *target) const override;
};

#endif
