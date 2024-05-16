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

#include "alcachofa/metaengine.h"
#include "alcachofa/detection.h"
#include "alcachofa/alcachofa.h"

namespace Alcachofa {

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

} // End of namespace Alcachofa

const char *AlcachofaMetaEngine::getName() const {
	return "alcachofa";
}

const ADExtraGuiOptionsMap *AlcachofaMetaEngine::getAdvancedExtraGuiOptions() const {
	return Alcachofa::optionsList;
}

Common::Error AlcachofaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Alcachofa::AlcachofaEngine(syst, desc);
	return Common::kNoError;
}

bool AlcachofaMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(ALCACHOFA)
REGISTER_PLUGIN_DYNAMIC(ALCACHOFA, PLUGIN_TYPE_ENGINE, AlcachofaMetaEngine);
#else
REGISTER_PLUGIN_STATIC(ALCACHOFA, PLUGIN_TYPE_ENGINE, AlcachofaMetaEngine);
#endif
