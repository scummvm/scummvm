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
#include "common/compression/installshield_cab.h"

#include "agos/detection.h"
#include "agos/intern_detection.h"
#include "agos/detection_fallback.h"
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
	{"elvira1", "Elvira: Mistress of the Dark"},
	{"elvira2", "Elvira II: The Jaws of Cerberus"},
	{"waxworks", "Waxworks"},
	{"simon1", "Simon the Sorcerer"},
	{"simon2", "Simon the Sorcerer II: The Lion, the Wizard and the Wardrobe"},
	{"feeble", "The Feeble Files"},
	{"dimp", "Simon the Sorcerer's Puzzle Pack: Demon in my Pocket"},
	{"jumble", "Simon the Sorcerer's Puzzle Pack: Jumble"},
	{"puzzle", "Simon the Sorcerer's Puzzle Pack: NoPatience"},
	{"swampy", "Simon the Sorcerer's Puzzle Pack: Swampy Adventures"},
	{nullptr, nullptr}
};

#include "agos/detection_tables.h"

static const char *const directoryGlobs[] = {
	"execute", // Used by Simon1 Acorn CD
	nullptr
};

using namespace AGOS;

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

	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override {
		ADDetectedGame detectedGame = detectGameFilebased(allFiles, AGOS::fileBased);
		if (!detectedGame.desc) {
			return ADDetectedGame();
		}

		return detectedGame;
	}

};

REGISTER_PLUGIN_STATIC(AGOS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, AgosMetaEngineDetection);
