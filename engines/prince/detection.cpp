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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "prince/detection.h"

namespace Prince {

int PrinceEngine::getGameType() const {
	return _gameDescription->gameType;
}

const char *PrinceEngine::getGameId() const {
	return _gameDescription->desc.gameid;
}

uint32 PrinceEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::Language PrinceEngine::getLanguage() const {
	return _gameDescription->desc.language;
}

bool PrinceMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	using namespace Prince;
	const PrinceGameDescription *gd = (const PrinceGameDescription *)desc;
	if (gd) {
		*engine = new PrinceEngine(syst, gd);
	}
	return gd != 0;
}

bool PrinceMetaEngine::hasFeature(MetaEngineFeature f) const {
	return
		(f == kSupportsDeleteSave) ||
		(f == kSavesSupportMetaInfo) ||
		(f == kSavesSupportThumbnail) ||
		(f == kSavesSupportCreationDate) ||
		(f == kSupportsListSaves) ||
		(f == kSupportsLoadingDuringStartup);
}

bool Prince::PrinceEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsRTL);
}

} // End of namespace Prince

#if PLUGIN_ENABLED_DYNAMIC(PRINCE)
REGISTER_PLUGIN_DYNAMIC(PRINCE, PLUGIN_TYPE_ENGINE, Prince::PrinceMetaEngine);
#else
REGISTER_PLUGIN_STATIC(PRINCE, PLUGIN_TYPE_ENGINE, Prince::PrinceMetaEngine);
#endif
