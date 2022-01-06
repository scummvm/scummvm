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

#include "common/config-manager.h"
#include "common/translation.h"

#include "ultima/detection.h"
#include "ultima/detection_tables.h"

namespace Ultima {

static const PlainGameDescriptor ULTIMA_GAMES[] = {
#ifndef RELEASE_BUILD
	{ "ultima1", "Ultima I - The First Age of Darkness" },
#endif
	{ "ultima4", "Ultima IV - Quest of the Avatar" },
	{ "ultima4_enh", "Ultima IV - Quest of the Avatar - Enhanced" },
	{ "ultima6", "Ultima VI - The False Prophet" },
	{ "ultima6_enh", "Ultima VI - The False Prophet - Enhanced" },
	{ "ultima8", "Ultima VIII - Pagan" },
	{ "remorse", "Crusader: No Remorse" },
	{ "regret", "Crusader: No Regret" },

	{ "martiandreams", "Worlds of Ultima: Martian Dreams" },
	{ "martiandreams_enh", "Worlds of Ultima: Martian Dreams - Enhanced" },
	{ "thesavageempire", "Worlds of Ultima: The Savage Empire" },
	{ "thesavageempire_enh", "Worlds of Ultima: The Savage Empire - Enhanced" },
	{ 0, 0 }
};

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_ORIGINAL_SAVELOAD,
		{
			_s("Use original save/load screens"),
			_s("Use the original save/load screens instead of the ScummVM ones"),
			"originalsaveload",
			false
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

static const ExtraGuiOption COMMON_OPTIONS[] = {
	{
		_s("Enable frame skipping"),
		_s("Allow the game to skip animation frames when running too slow."),
		"frameSkip",
		false
	},
	{
		_s("Enable frame limiting"),
		_s("Limits the speed of the game to prevent running too fast."),
		"frameLimit",
		true
	},
	{
		_s("Enable cheats"),
		_s("Allow cheats by commands and a menu when player is clicked."),
		"cheat",
		false
	},
	{
		_s("Enable high resolution"),
		_s("Enable a higher resolution for the game"),
		"usehighres",
		false
	},
	{ nullptr, nullptr, nullptr, false }
};

static const ExtraGuiOption U8_OPTIONS[] = {
	{
		_s("Play foot step sounds"),
		_s("Plays sound when the player moves."),
		"footsteps",
		true
	},
	{
		_s("Enable jump to mouse position"),
		_s("Jumping while not moving targets the mouse cursor rather than direction only."),
		"targetedjump",
		true
	},
	{
		_s("Use original save/load screens"),
		_s("Use the original save/load screens instead of the ScummVM ones"),
		"originalsaveload",
		false
	},
	{
		_s("Enable font replacement"),
		_s("Replaces game fonts with rendered fonts"),
		"font_override",
		false
	},
	{
		_s("Enable font anti-aliasing"),
		_s("When font anti-aliasing is enabled, the text is smoother."),
		"font_antialiasing",
		false
	},
	{ nullptr, nullptr, nullptr, false }
};

static const ExtraGuiOption CAMERA_WITH_SILENCER = {
	// I18N: Silencer is the player-character in Crusader games, known as the Avatar in Ultima series.
	_s("Camera moves with Silencer"),
	_s("Camera tracks the player movement rather than snapping to defined positions."),
	"camera_on_player",
	true
};

static const ExtraGuiOption REMORSE_OPTIONS[] = {
	CAMERA_WITH_SILENCER,
	{ nullptr, nullptr, nullptr, false }
};

static const ExtraGuiOption REGRET_OPTIONS[] = {
	CAMERA_WITH_SILENCER,
	{
		_s("Always enable Christmas easter-egg"),
		_s("Enable the Christmas music at any time of year."),
		"always_christmas",
		true
	},
	{ nullptr, nullptr, nullptr, false }
};

} // End of namespace Ultima

UltimaMetaEngineDetection::UltimaMetaEngineDetection() : AdvancedMetaEngineDetection(Ultima::GAME_DESCRIPTIONS,
	        sizeof(Ultima::UltimaGameDescription), Ultima::ULTIMA_GAMES, Ultima::optionsList) {
	static const char *const DIRECTORY_GLOBS[2] = { "usecode", 0 };
	_maxScanDepth = 2;
	_directoryGlobs = DIRECTORY_GLOBS;
}

Common::String UltimaMetaEngineDetection::getGameId(const Common::String& target) {
	// Store a copy of the active domain
	Common::String currDomain = ConfMan.getActiveDomainName();

	// Switch to the given target domain and get it's game Id
	ConfMan.setActiveDomain(target);
	Common::String gameId = ConfMan.get("gameid");

	// Switch back to the original domain and return the game Id
	ConfMan.setActiveDomain(currDomain);
	return gameId;
}

const ExtraGuiOptions UltimaMetaEngineDetection::getExtraGuiOptions(const Common::String &target) const {
	// TODO: Use ADExtraGuiOptionsMap for all games
	const Common::String gameId = getGameId(target);
	if (gameId == "ultima8" || gameId == "remorse" || gameId == "regret") {
		ExtraGuiOptions options;

		for (const ExtraGuiOption *o = Ultima::COMMON_OPTIONS; o->configOption; ++o) {
			options.push_back(*o);
		}

		// Game specific options
		const ExtraGuiOption *game_options;
		if (gameId == "ultima8")
			game_options = Ultima::U8_OPTIONS;
		else if (gameId == "remorse")
			game_options = Ultima::REMORSE_OPTIONS;
		else
			game_options = Ultima::REGRET_OPTIONS;

		for (const ExtraGuiOption *o = game_options; o->configOption; ++o) {
			options.push_back(*o);
		}

		return options;
	}

	return MetaEngineDetection::getExtraGuiOptions(gameId);
}

REGISTER_PLUGIN_STATIC(ULTIMA_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, UltimaMetaEngineDetection);
