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

#include "common/translation.h"

#include "mediastation/metaengine.h"
#include "mediastation/detection.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

static const ADExtraGuiOptionsMap optionsList[] = {
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace MediaStation

const char *MediaStationMetaEngine::getName() const {
	return "mediastation";
}

const ADExtraGuiOptionsMap *MediaStationMetaEngine::getAdvancedExtraGuiOptions() const {
	return MediaStation::optionsList;
}

Common::Error MediaStationMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new MediaStation::MediaStationEngine(syst, desc);
	return Common::kNoError;
}

bool MediaStationMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	       (f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(MEDIASTATION)
	REGISTER_PLUGIN_DYNAMIC(MEDIASTATION, PLUGIN_TYPE_ENGINE, MediaStationMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(MEDIASTATION, PLUGIN_TYPE_ENGINE, MediaStationMetaEngine);
#endif
