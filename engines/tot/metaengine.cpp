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

#include "tot/metaengine.h"
#include "tot/detection.h"
#include "tot/tot.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_COPY_PROTECTION,
		{
			_s("Copy protection"),
			_s("Enable copy protection"),
			"copy_protection",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_NO_TRANSITIONS,
		{
			_s("Disable scene transitions"),
			_s("Disable original transition effects between scenes"),
			"transitions_disable",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_ORIGINAL_SAVELOAD_DIALOG,
		{
			_s("Original save/load dialog"),
			_s("Use original save and load dialogs"),
			"original_save_load_screen",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *TotMetaEngine::getName() const {
	return "tot";
}

Common::Error TotMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	*engine = new Tot::TotEngine(syst, desc);
	return Common::kNoError;
}

bool TotMetaEngine::hasFeature(MetaEngineFeature f) const {
	return checkExtendedSaves(f) ||
	(f == kSupportsLoadingDuringStartup);
}

const ADExtraGuiOptionsMap *TotMetaEngine::getAdvancedExtraGuiOptions() const {
		return optionsList;
	}

#if PLUGIN_ENABLED_DYNAMIC(TOT)
REGISTER_PLUGIN_DYNAMIC(TOT, PLUGIN_TYPE_ENGINE, TotMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TOT, PLUGIN_TYPE_ENGINE, TotMetaEngine);
#endif
