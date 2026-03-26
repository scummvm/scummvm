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

#include "base/plugins.h"

#include "common/translation.h"
#include "harvester/detection.h"
#include "harvester/metaengine.h"
#include "harvester/harvester.h"

static const ADExtraGuiOptionsMap gameGuiOptions[] = {
	{
		GAMEOPTION_GORE,
		{
			_s("Enable gore"),
			_s("When disabled, the startup movie player follows the original GORE=NO censorship path."),
			"gore",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_SHOW_CD_CHANGE_PROMPTS,
		{
			_s("Show CD change prompts"),
			_s("Display the original CD swap image and wait for a left click when the script requests disc 2 or 3."),
			"show_cd_change_prompts",
			false,
			0,
			0
		}
	},

	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *HarvesterMetaEngine::getName() const {
	return "harvester";
}

const ADExtraGuiOptionsMap *HarvesterMetaEngine::getAdvancedExtraGuiOptions() const {
	return gameGuiOptions;
}

Common::Error HarvesterMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Harvester::HarvesterEngine(syst, desc);
	return Common::kNoError;
}

int HarvesterMetaEngine::getMaximumSaveSlot() const {
	return 24;
}

int HarvesterMetaEngine::getAutosaveSlot() const {
	return -1;
}

bool HarvesterMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
		(f == kSupportsLoadingDuringStartup);
}

#if PLUGIN_ENABLED_DYNAMIC(HARVESTER)
REGISTER_PLUGIN_DYNAMIC(HARVESTER, PLUGIN_TYPE_ENGINE, HarvesterMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HARVESTER, PLUGIN_TYPE_ENGINE, HarvesterMetaEngine);
#endif
