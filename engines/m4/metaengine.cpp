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

#include "m4/metaengine.h"
#include "m4/detection.h"
#include "m4/m4.h"

namespace M4 {

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

} // End of namespace M4

const char *M4MetaEngine::getName() const {
	return "m4";
}

const ADExtraGuiOptionsMap *M4MetaEngine::getAdvancedExtraGuiOptions() const {
	return M4::optionsList;
}

Common::Error M4MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new M4::M4Engine(syst, desc);
	return Common::kNoError;
}

bool M4MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(M4)
REGISTER_PLUGIN_DYNAMIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#else
REGISTER_PLUGIN_STATIC(M4, PLUGIN_TYPE_ENGINE, M4MetaEngine);
#endif
