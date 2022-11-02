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

#include "engines/advancedDetector.h"
#include "engines/obsolete.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/installshield_cab.h"
#include "common/translation.h"

#include "agos/detection.h"
#include "agos/intern_detection.h"
#include "agos/obsolete.h" // Obsolete ID table.
#include "agos/agos.h"

/**
 * Conversion table mapping old obsolete target names to the
 * corresponding new target and platform combination.
 *
 */

static const DebugChannelDef debugFlagList[] = {
	{AGOS::kDebugOpcode, "opcode", "Opcode debug level"},
	{AGOS::kDebugVGAOpcode, "vga_opcode", "VGA Opcode debug level"},
	{AGOS::kDebugSubroutine, "subroutine", "Subroutine debug level"},
	{AGOS::kDebugVGAScript, "vga_script", "VGA Script debug level"},
	DEBUG_CHANNEL_END
};

static const PlainGameDescriptor agosGames[] = {
	{"pn", "Personal Nightmare"},
	{"elvira1", "Elvira - Mistress of the Dark"},
	{"elvira2", "Elvira II - The Jaws of Cerberus"},
	{"waxworks", "Waxworks"},
	{"simon1", "Simon the Sorcerer 1"},
	{"simon2", "Simon the Sorcerer 2"},
	{"feeble", "The Feeble Files"},
	{"dimp", "Demon in my Pocket"},
	{"jumble", "Jumble"},
	{"puzzle", "NoPatience"},
	{"swampy", "Swampy Adventures"},
	{nullptr, nullptr}
};

#include "agos/detection_tables.h"

static const char *const directoryGlobs[] = {
	"execute", // Used by Simon1 Acorn CD
	nullptr
};

using namespace AGOS;

namespace AGOS {

static const ADExtraGuiOptionsMap optionsList[] = {
	{
		GAMEOPTION_OPL3_MODE,
		{
			_s("AdLib OPL3 mode"),
			_s("When AdLib is selected, OPL3 features will be used. Depending on the game, this will prevent cut-off notes, add extra notes or instruments and/or add stereo."),
			"opl3_mode",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_DOS_TEMPOS,
		{
			_s("Use DOS version music tempos"),
			_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
			"dos_music_tempos",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_WINDOWS_TEMPOS,
		{
			_s("Use DOS version music tempos"),
			_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
			"dos_music_tempos",
			false,
			0,
			0
		}
	},
	{
		GAMEOPTION_PREFER_DIGITAL_SFX,
		{
			_s("Prefer digital sound effects"),
			_s("Prefer digital sound effects instead of synthesized ones"),
			"prefer_digitalsfx",
			true,
			0,
			0
		}
	},
	{
		GAMEOPTION_DISABLE_FADE_EFFECTS,
		{
			_s("Disable fade-out effects"),
			_s("Don't fade every screen to black when leaving a room."),
			"disable_fade_effects",
			false,
			0,
			0
		}
	},
	AD_EXTRA_GUI_OPTIONS_TERMINATOR
};

} // End of namespace AGOS

class AgosMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AgosMetaEngineDetection() : AdvancedMetaEngineDetection(AGOS::gameDescriptions, sizeof(AGOS::AGOSGameDescription), agosGames, AGOS::optionsList) {
		_guiOptions = GUIO1(GUIO_NOLAUNCHLOAD);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	const char *getName() const override {
		return "agos";
	}

	const char *getEngineName() const override {
		return "AGOS";
	}

	const char *getOriginalCopyright() const override {
		return "AGOS (C) Adventure Soft";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

REGISTER_PLUGIN_STATIC(AGOS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AgosMetaEngineDetection);
