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

#include "bolt/bolt.h"
#include "bolt/metaengine.h"
#include "bolt/detection.h"

namespace Bolt {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_EXTEND_SCREEN,
		{
			_s("Extended game resolution"),
			_s("This option allows the game to play at a slightly extended resolution. This is what the Mac and CDi versions are supposed to play like."),
			"extended_viewport",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace Bolt

const char *BoltMetaEngine::getName() const {
	return "bolt";
}

const ADExtraGuiOptionsMap *BoltMetaEngine::getAdvancedExtraGuiOptions() const {
	return Bolt::optionsList;
}

Common::Error BoltMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Bolt::BoltEngine(syst, desc);
	return Common::kNoError;
}

bool BoltMetaEngine::hasFeature(MetaEngineFeature f) const {
	return false; // No saving features whatsoever! :-)
}

#if PLUGIN_ENABLED_DYNAMIC(BOLT)
REGISTER_PLUGIN_DYNAMIC(BOLT, PLUGIN_TYPE_ENGINE, BoltMetaEngine);
#else
REGISTER_PLUGIN_STATIC(BOLT, PLUGIN_TYPE_ENGINE, BoltMetaEngine);
#endif
