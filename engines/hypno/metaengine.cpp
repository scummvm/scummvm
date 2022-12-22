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

#include "engines/advancedDetector.h"

#include "common/translation.h"

#include "hypno/hypno.h"
#include "hypno/detection.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_CHEATS,
		{
			_s("Enable original cheats"),
			_s("Allow cheats using the C key."),
			"cheats",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_INFINITE_HEALTH,
		{
			_s("Enable infinite health cheat"),
			_s("Player health will never decrease (except for game over scenes)."),
			"infiniteHealth",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_INFINITE_AMMO,
		{
			_s("Enable infinite ammo cheat"),
			_s("Player ammo will never decrease."),
			"infiniteAmmo",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_UNLOCK_ALL_LEVELS,
		{
			_s("Unlock all levels"),
			_s("All levels will be available to play."),
			"unlockAllLevels",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_RESTORED_CONTENT,
		{
			_s("Enable restored content"),
			_s("Add additional content that is not enabled the original implementation."),
			"restored",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

class HypnoMetaEngine : public AdvancedMetaEngine {
public:
	const char *getName() const override {
		return "hypno";
	}

	const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const override {
		return optionsList;
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const override;
};

Common::Error HypnoMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	if (Common::String(desc->gameId) == "wetlands") {
		*engine = (Engine *)new Hypno::WetEngine(syst, desc);
	} else if (Common::String(desc->gameId) == "sinistersix") {
		*engine = (Engine *)new Hypno::SpiderEngine(syst, desc);
	} else if (Common::String(desc->gameId) == "soldierboyz") {
		*engine = (Engine *)new Hypno::BoyzEngine(syst, desc);
	} else
		return Common::kUnsupportedGameidError;

	return Common::kNoError;
}

namespace Hypno {

bool HypnoEngine::isDemo() const {
	return (bool)(_gameDescription->flags & ADGF_DEMO);
}

} // End of namespace Hypno

#if PLUGIN_ENABLED_DYNAMIC(HYPNO)
REGISTER_PLUGIN_DYNAMIC(HYPNO, PLUGIN_TYPE_ENGINE, HypnoMetaEngine);
#else
REGISTER_PLUGIN_STATIC(HYPNO, PLUGIN_TYPE_ENGINE, HypnoMetaEngine);
#endif

