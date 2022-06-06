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

#include "immortal/metaengine.h"
#include "immortal/detection.h"
#include "immortal/immortal.h"

const char *ImmortalMetaEngine::getName() const {
	return "immortal";
}

Common::Error ImmortalMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Immortal::ImmortalEngine(syst, desc);
	return Common::kNoError;
}

bool ImmortalMetaEngine::hasFeature(MetaEngineFeature f) const {
    return false;
/*	return
		(f == kSavesUseExtendedFormat) ||
		(f == kSimpleSavesNames) ||
	    (f == kSupportsListSaves) ||
	    (f == kSupportsDeleteSave) ||
	    (f == kSavesSupportMetaInfo) ||
	    (f == kSavesSupportThumbnail) ||
	    (f == kSupportsLoadingDuringStartup); */
}

#if PLUGIN_ENABLED_DYNAMIC(IMMORTAL)
REGISTER_PLUGIN_DYNAMIC(IMMORTAL, PLUGIN_TYPE_ENGINE, ImmortalMetaEngine);
#else
REGISTER_PLUGIN_STATIC(IMMORTAL, PLUGIN_TYPE_ENGINE, ImmortalMetaEngine);
#endif
