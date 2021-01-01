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

#include "ultima/detection.h"
#include "base/plugins.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/meta_engine.h"
#include "ultima/nuvie/meta_engine.h"
#include "ultima/nuvie/nuvie.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/meta_engine.h"

#include "ultima/metaengine.h"

const char *UltimaMetaEngine::getName() const {
	return "ultima";
}

Common::Error UltimaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Ultima::UltimaGameDescription *gd = (const Ultima::UltimaGameDescription *)desc;
	switch (gd->gameId) {
#ifndef RELEASE_BUILD
	case Ultima::GAME_ULTIMA1:
		*engine = new Ultima::Shared::UltimaEarlyEngine(syst, gd);
		break;
#endif
	case Ultima::GAME_ULTIMA4:
		*engine = new Ultima::Ultima4::Ultima4Engine(syst, gd);
		break;
	case Ultima::GAME_ULTIMA6:
	case Ultima::GAME_MARTIAN_DREAMS:
	case Ultima::GAME_SAVAGE_EMPIRE:
		*engine = new Ultima::Nuvie::NuvieEngine(syst, gd);
		break;
	case Ultima::GAME_ULTIMA8:
	case Ultima::GAME_CRUSADER_REG:
	case Ultima::GAME_CRUSADER_REM:
		*engine = new Ultima::Ultima8::Ultima8Engine(syst, gd);
		break;

	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

int UltimaMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

SaveStateList UltimaMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList = AdvancedMetaEngine::listSaves(target);

	Common::String gameId = getGameId(target);
	if (gameId == "ultima6" || gameId == "ultima6_enh")
		Ultima::Nuvie::MetaEngine::listSaves(saveList);

	return saveList;
}

Common::KeymapArray UltimaMetaEngine::initKeymaps(const char *target) const {
	const Common::String gameId = getGameId(target);
	if (gameId == "ultima4" || gameId == "ultima4_enh")
		return Ultima::Ultima4::MetaEngine::initKeymaps();
	if (gameId == "ultima8" || gameId == "remorse" || gameId == "regret")
		return Ultima::Ultima8::MetaEngine::initKeymaps(gameId);

	return Common::KeymapArray();
}

Common::String UltimaMetaEngine::getGameId(const char *target) {
	// Store a copy of the active domain
	Common::String currDomain = ConfMan.getActiveDomainName();

	// Switch to the given target domain and get it's game Id
	ConfMan.setActiveDomain(target);
	Common::String gameId = ConfMan.get("gameid");

	// Switch back to the original domain and return the game Id
	ConfMan.setActiveDomain(currDomain);
	return gameId;
}

#if PLUGIN_ENABLED_DYNAMIC(ULTIMA)
	REGISTER_PLUGIN_DYNAMIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#endif
