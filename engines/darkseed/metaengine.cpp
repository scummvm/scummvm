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

#include "darkseed/metaengine.h"
#include "darkseed/detection.h"
#include "darkseed/darkseed.h"

namespace Darkseed {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"original_menus",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Darkseed

const char *DarkseedMetaEngine::getName() const {
	return "darkseed";
}

const ADExtraGuiOptionsMap *DarkseedMetaEngine::getAdvancedExtraGuiOptions() const {
	return Darkseed::optionsList;
}

Common::Error DarkseedMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Darkseed::DarkseedEngine(syst, desc);
	return Common::kNoError;
}

bool DarkseedMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(DARKSEED)
	REGISTER_PLUGIN_DYNAMIC(DARKSEED, PLUGIN_TYPE_ENGINE, DarkseedMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(DARKSEED, PLUGIN_TYPE_ENGINE, DarkseedMetaEngine);
#endif
