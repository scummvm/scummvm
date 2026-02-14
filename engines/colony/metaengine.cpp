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

#include "colony/colony.h"
#include "colony/detection.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/config-manager.h"

namespace Colony {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_WIDESCREEN,
		{
			_s("Widescreen mod"),
			_s("Enable widescreen rendering in fullscreen mode."),
			"widescreen_mod",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class ColonyMetaEngine : public AdvancedMetaEngine<ADGameDescription> {
public:
	const char *getName() const override {
		return "colony";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const override {
		*engine = new ColonyEngine(syst, gd);
		return Common::kNoError;
	}
};

} // End of namespace Colony

#if PLUGIN_ENABLED_DYNAMIC(COLONY)
REGISTER_PLUGIN_DYNAMIC(COLONY, PLUGIN_TYPE_ENGINE, Colony::ColonyMetaEngine);
#else
REGISTER_PLUGIN_STATIC(COLONY, PLUGIN_TYPE_ENGINE, Colony::ColonyMetaEngine);
#endif
