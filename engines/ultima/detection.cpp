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
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/translation.h"
#ifdef ENABLE_ULTIMA1
#include "ultima/shared/early/ultima_early.h"
#endif
#ifdef ENABLE_ULTIMA6
#include "ultima/ultima6/ultima6.h"
#endif
#ifdef ENABLE_ULTIMA8
#include "ultima/ultima8/ultima8.h"
#endif

namespace Ultima {

static const PlainGameDescriptor ULTIMA_GAMES[] = {
#ifdef ENABLE_ULTIMA1
	{ "ultima1", "Ultima I - The First Age of Darkness" },
#endif
#ifdef ENABLE_ULTIMA6
	{ "ultima6", "Ultima VI - The False Prophet" },
	{ "ultima6_enh", "Ultima VI - The False Prophet - Enhanced" },
#endif
#ifdef ENABLE_ULTIMA8
	{ "ultima8", "Ultima VIII - Pagan" },
#endif
	{ 0, 0 }
};

} // End of namespace Ultima

#include "ultima/detection_tables.h"

namespace Ultima {
UltimaMetaEngine *g_metaEngine;
} // End of namespace Ultima


UltimaMetaEngine::UltimaMetaEngine() : AdvancedMetaEngine(Ultima::GAME_DESCRIPTIONS,
	        sizeof(Ultima::UltimaGameDescription), Ultima::ULTIMA_GAMES) {
	Ultima::g_metaEngine = this;

#ifdef ENABLE_ULTIMA8
	static const char *const DIRECTORY_GLOBS[2] = { "usecode", 0 };
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
#endif
}

UltimaMetaEngine::~UltimaMetaEngine() {
	Ultima::g_metaEngine = nullptr;
}

bool UltimaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsListSaves) ||
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSavesSupportPlayTime) ||
		(f == kSupportsLoadingDuringStartup) ||
		(f == kSavesUseExtendedFormat);
}

bool UltimaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Ultima::UltimaGameDescription *gd = (const Ultima::UltimaGameDescription *)desc;
	if (gd) {
		switch (gd->gameId) {
#ifdef ENABLE_ULTIMA1
		case Ultima::GAME_ULTIMA1:
			*engine = new Ultima::Shared::UltimaEarlyEngine(syst, gd);
			break;
#endif
#ifdef ENABLE_ULTIMA6
		case Ultima::GAME_ULTIMA6:
			*engine = new Ultima::Ultima6::Ultima6Engine(syst, gd);
			break;
#endif
#ifdef ENABLE_ULTIMA8
		case Ultima::GAME_ULTIMA8:
			*engine = new Ultima::Ultima8::Ultima8Engine(syst, gd);
			break;
#endif
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

#if PLUGIN_ENABLED_DYNAMIC(ULTIMA)
REGISTER_PLUGIN_DYNAMIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#endif
