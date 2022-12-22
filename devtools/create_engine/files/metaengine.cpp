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

#include "xyzzy/metaengine.h"
#include "xyzzy/detection.h"
#include "xyzzy/xyzzy.h"

namespace Xyzzy {

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

} // End of namespace Xyzzy

const char *XyzzyMetaEngine::getName() const {
	return "xyzzy";
}

const ADExtraGuiOptionsMap *XyzzyMetaEngine::getAdvancedExtraGuiOptions() const {
	return Xyzzy::optionsList;
}

Common::Error XyzzyMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Xyzzy::XyzzyEngine(syst, desc);
	return Common::kNoError;
}

bool XyzzyMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(XYZZY)
REGISTER_PLUGIN_DYNAMIC(XYZZY, PLUGIN_TYPE_ENGINE, XyzzyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(XYZZY, PLUGIN_TYPE_ENGINE, XyzzyMetaEngine);
#endif
