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

#include "ultima/detection.h"
#include "base/plugins.h"
#include "common/system.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/str-array.h"
#include "common/memstream.h"
#include "common/translation.h"
#include "ultima/shared/early/ultima_early.h"
#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/metaengine.h"
#include "ultima/nuvie/metaengine.h"
#include "ultima/nuvie/nuvie.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/metaengine.h"

#include "ultima/metaengine.h"

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAME_SKIPPING,
		{
			_s("Enable frame skipping"),
			_s("Allow the game to skip animation frames when running too slow."),
			"frameSkip",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FRAME_LIMITING,
		{
			_s("Enable frame limiting"),
			_s("Limits the speed of the game to prevent running too fast."),
			"frameLimit",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_CHEATS,
		{
			_s("Enable cheats"),
			_s("Allow cheats by commands and a menu when player is clicked."),
			"cheat",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_HIGH_RESOLUTION,
		{
			_s("Enable high resolution"),
			_s("Enable a higher resolution for the game"),
			"usehighres",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FOOTSTEP_SOUNDS,
		{
			_s("Play foot step sounds"),
			_s("Plays sound when the player moves."),
			"footsteps",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_JUMP_TO_MOUSE,
		{
			_s("Enable jump to mouse position"),
			_s("Jumping while not moving targets the mouse cursor rather than direction only."),
			"targetedjump",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_REPLACEMENT,
		{
			_s("Enable font replacement"),
			_s("Replaces game fonts with rendered fonts"),
			"font_override",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_FONT_ANTIALIASING,
		{
			_s("Enable font anti-aliasing"),
			_s("When font anti-aliasing is enabled, the text is smoother."),
			"font_antialiasing",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_CAMERA_WITH_SILENCER,
		{
			// I18N: Silencer is the player-character in Crusader games, known as the Avatar in Ultima series.
			_s("Camera moves with Silencer"),
			_s("Camera tracks the player movement rather than snapping to defined positions."),
			"camera_on_player",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_ALWAYS_CHRISTMAS,
		{
			_s("Always enable Christmas easter-egg"),
			_s("Enable the Christmas music at any time of year."),
			"always_christmas",
			true,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

const char *UltimaMetaEngine::getName() const {
	return "ultima";
}

const ADExtraGuiOptionsMap *UltimaMetaEngine::getAdvancedExtraGuiOptions() const {
	return optionsList;
}

Common::Error UltimaMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const Ultima::UltimaGameDescription *gd = (const Ultima::UltimaGameDescription *)desc;
	switch (gd->gameId) {
#ifndef RELEASE_BUILD
	case Ultima::GAME_ULTIMA1:
		*engine = new Ultima::Shared::UltimaEarlyEngine(syst, gd);
		break;
#endif
	case Ultima::GAME_ULTIMA4:
		*engine = new Ultima::Ultima4::Ultima4Engine(syst, gd);
		break;
	case Ultima::GAME_ULTIMA6:
	case Ultima::GAME_MARTIAN_DREAMS:
	case Ultima::GAME_SAVAGE_EMPIRE:
		*engine = new Ultima::Nuvie::NuvieEngine(syst, gd);
		break;
	case Ultima::GAME_ULTIMA8:
	case Ultima::GAME_CRUSADER_REG:
	case Ultima::GAME_CRUSADER_REM:
		*engine = new Ultima::Ultima8::Ultima8Engine(syst, gd);
		break;

	default:
		return Common::kUnsupportedGameidError;
	}
	return Common::kNoError;
}

int UltimaMetaEngine::getMaximumSaveSlot() const {
	return MAX_SAVES;
}

SaveStateList UltimaMetaEngine::listSaves(const char *target) const {
	SaveStateList saveList = AdvancedMetaEngine::listSaves(target);

	Common::String gameId = getGameId(target);
	if (gameId == "ultima6" || gameId == "ultima6_enh")
		Ultima::Nuvie::MetaEngine::listSaves(saveList);

	return saveList;
}

SaveStateDescriptor UltimaMetaEngine::querySaveMetaInfos(const char *target, int slot) const {
	SaveStateDescriptor desc = AdvancedMetaEngine::querySaveMetaInfos(target, slot);
	if (!desc.isValid() && slot > 0) {
		Common::String gameId = getGameId(target);
		if (gameId == "ultima8") {
			Common::String filename = getSavegameFile(slot, target);
			desc = SaveStateDescriptor(this, slot, Common::U32String());
			if (!Ultima::Ultima8::MetaEngine::querySaveMetaInfos(filename, desc))
				return SaveStateDescriptor();
		}
	}

	return desc;
}

Common::KeymapArray UltimaMetaEngine::initKeymaps(const char *target) const {
	const Common::String gameId = getGameId(target);
	if (gameId == "ultima4" || gameId == "ultima4_enh")
		return Ultima::Ultima4::MetaEngine::initKeymaps();
	if (gameId == "ultima8" || gameId == "remorse" || gameId == "regret")
		return Ultima::Ultima8::MetaEngine::initKeymaps(gameId);

	return Common::KeymapArray();
}

Common::String UltimaMetaEngine::getGameId(const Common::String& target) {
	// Store a copy of the active domain
	Common::String currDomain = ConfMan.getActiveDomainName();

	// Switch to the given target domain and get it's game Id
	ConfMan.setActiveDomain(target);
	Common::String gameId = ConfMan.get("gameid");

	// Switch back to the original domain and return the game Id
	ConfMan.setActiveDomain(currDomain);
	return gameId;
}

#if PLUGIN_ENABLED_DYNAMIC(ULTIMA)
	REGISTER_PLUGIN_DYNAMIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(ULTIMA, PLUGIN_TYPE_ENGINE, UltimaMetaEngine);
#endif
