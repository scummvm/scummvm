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

static const ExtraGuiOption opl3Mode = {
	_s("AdLib OPL3 mode"),
	_s("When AdLib is selected, OPL3 features will be used. Depending on the game, this will prevent cut-off notes, add extra notes or instruments and/or add stereo."),
	"opl3_mode",
	false,
	0,
	0
};

static const ExtraGuiOption useDosTempos = {
	_s("Use DOS version music tempos"),
	_s("Selecting this option will play the music using the tempos used by the DOS version of the game. Otherwise, the faster tempos of the Windows version will be used."),
	"dos_music_tempos",
	false,
	0,
	0
};

static const ExtraGuiOption preferDigitalSfx = {
	_s("Prefer digital sound effects"),
	_s("Prefer digital sound effects instead of synthesized ones"),
	"prefer_digitalsfx",
	true,
	0,
	0
};

class AgosMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	AgosMetaEngineDetection() : AdvancedMetaEngineDetection(AGOS::gameDescriptions, sizeof(AGOS::AGOSGameDescription), agosGames) {
		_guiOptions = GUIO1(GUIO_NOLAUNCHLOAD);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	const char *getEngineId() const override {
		return "agos";
	}

	const char *getName() const override {
		return "AGOS";
	}

	const char *getOriginalCopyright() const override {
		return "AGOS (C) Adventure Soft";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override {
		const Common::String gameid = ConfMan.get("gameid", target);
		const Common::String platform = ConfMan.get("platform", target);
		const Common::String extra = ConfMan.get("extra", target);

		ExtraGuiOptions options;
		if (target.empty() || ((gameid == "elvira1" || gameid == "elvira2" || gameid == "waxworks" || gameid == "simon1") && platform == "pc")) {
			// DOS versions of Elvira 1 and 2, Waxworks and Simon 1 can
			// optionally make use of AdLib OPL3 features.
			options.push_back(opl3Mode);
		}
		if (target.empty() || (gameid == "simon1" && ((platform == "pc" && extra != "Floppy Demo") || platform == "windows" ||
				(platform == "acorn" && extra.contains("CD"))))) {
			// Simon 1 DOS (except the floppy demo), Windows and Acorn CD can
			// choose between the DOS or Windows music tempos.
			ExtraGuiOption dosTemposOption = useDosTempos;
			// DOS tempos are default for the DOS versions; other versions use
			// the Windows tempos by default.
			dosTemposOption.defaultState = platform == "pc";
			options.push_back(dosTemposOption);
		}
		if (target.empty() || ((gameid == "elvira2" || gameid == "waxworks") && platform == "pc")) {
			// DOS versions of Elvira 2 and Waxworks can use either Adlib or
			// digital SFX.
			options.push_back(preferDigitalSfx);
		}
		return options;
	}
};

REGISTER_PLUGIN_STATIC(AGOS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AgosMetaEngineDetection);
