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

#include "macs2/metaengine.h"
#include "macs2/detection.h"
#include "macs2/macs2.h"

namespace Macs2 {

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

} // End of namespace Macs2

const char *Macs2MetaEngine::getName() const {
	return "macs2";
}

const ADExtraGuiOptionsMap *Macs2MetaEngine::getAdvancedExtraGuiOptions() const {
	return Macs2::optionsList;
}

Common::Error Macs2MetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Macs2::Macs2Engine(syst, desc);
	return Common::kNoError;
}

bool Macs2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(MACS2)
REGISTER_PLUGIN_DYNAMIC(MACS2, PLUGIN_TYPE_ENGINE, Macs2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(MACS2, PLUGIN_TYPE_ENGINE, Macs2MetaEngine);
#endif
