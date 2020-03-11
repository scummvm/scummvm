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
#include "common/translation.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/nuvie/meta_engine.h"
#include "ultima/nuvie/nuvie.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/meta_engine.h"

namespace Ultima {

static const PlainGameDescriptor ULTIMA_GAMES[] = {
	{ "ultima1", "Ultima I - The First Age of Darkness" },
	{ "ultima6", "Ultima VI - The False Prophet" },
	{ "ultima6_enh", "Ultima VI - The False Prophet - Enhanced" },
	{ "ultima8", "Ultima VIII - Pagan" },

	{ "martiandreams", "Worlds of Ultima: Martian Dreams" },
	{ "martiandreams_enh", "Worlds of Ultima: Martian Dreams - Enhanced" },
	{ "thesavageempire", "Worlds of Ultima: The Savage Empire" },
	{ "thesavageempire_enh", "Worlds of Ultima: The Savage Empire - Enhanced" },
	{ 0, 0 }
};

} // End of namespace Ultima

#include "ultima/detection_tables.h"

UltimaMetaEngine::UltimaMetaEngine() : AdvancedMetaEngine(Ultima::GAME_DESCRIPTIONS,
	        sizeof(Ultima::UltimaGameDescription), Ultima::ULTIMA_GAMES) {
	static const char *const DIRECTORY_GLOBS[2] = { "usecode", 0 };
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
}

bool UltimaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Ultima::UltimaGameDescription *gd = (const Ultima::UltimaGameDescription *)desc;
	if (gd) {
		switch (gd->gameId) {
		case Ultima::GAME_ULTIMA1:
			*engine = new Ultima::Shared::UltimaEarlyEngine(syst, gd);
			break;
		case Ultima::GAME_ULTIMA6:
		case Ultima::GAME_MARTIAN_DREAMS:
		case Ultima::GAME_SAVAGE_EMPIRE:
			*engine = new Ultima::Nuvie::NuvieEngine(syst, gd);
			break;
		case Ultima::GAME_ULTIMA8:
			*engine = new Ultima::Ultima8::Ultima8Engine(syst, gd);
			break;
		default:
			error("Unsupported game specified");
		}
	}
	return gd != 0;
}

int UltimaMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

const char *UltimaMetaEngine::getSavegamePattern(const char *target) const {
	static char buffer[100];
	snprintf(buffer, 200, "%s.###", target == nullptr ? getEngineId() : target);

	return buffer;
}

const char *UltimaMetaEngine::getSavegameFile(int saveGameIdx, const char *target) const {
	static char buffer[100];
	snprintf(buffer, 200, "%s.%.3d", target == nullptr ? getEngineId() : target, saveGameIdx);

	return buffer;
}

SaveStateList UltimaMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList = AdvancedMetaEngine::listSaves(target);

	Common::String gameId = getGameId(target);
	if (gameId == "ultima6" || gameId == "ultima6_enh")
		Ultima::Nuvie::MetaEngine::listSaves(saveList);

	return saveList;
}

Common::KeymapArray UltimaMetaEngine::initKeymaps(const char *target) const {
	Common::String gameId = getGameId(target);
	if (gameId == "ultima8")
		return Ultima::Ultima8::MetaEngine::initKeymaps();

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
