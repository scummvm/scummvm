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
#include "common/fs.h"
#include "common/language.h"

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


static bool hasSimon2LangFile(const Common::FSNode &gameDir, const char *filename) {
	Common::FSNode rootFile = gameDir.getChild(filename);
	if (rootFile.exists())
		return true;

	Common::FSNode dataDir = gameDir.getChild("data");
	if (dataDir.exists() && dataDir.isDirectory()) {
		Common::FSNode dataFile = dataDir.getChild(filename);
		if (dataFile.exists())
			return true;
	}

	return false;
}

static Common::String stripLanguageGUIOptions(const Common::String &guiOptions) {
	Common::String filtered;
	const char *p = guiOptions.c_str();

	while (*p) {
		while (*p == ' ')
			++p;
		if (!*p)
			break;

		const char *start = p;
		while (*p && *p != ' ')
			++p;

		Common::String token(start, p);
		if (!token.hasPrefix("lang_")) {
			if (!filtered.empty())
				filtered += " ";
			filtered += token;
		}
	}

	return filtered;
}

static void applySimon2LanguageGUIOptions(DetectedGame &game, const Common::FSNode &gameDir) {
	if (game.gameId != "simon2" || !gameDir.exists() || !gameDir.isDirectory())
		return;

	const bool hasEnglish = hasSimon2LangFile(gameDir, "simon2.english");
	const bool hasGerman = hasSimon2LangFile(gameDir, "simon2.german");
	const bool hasItalian = hasSimon2LangFile(gameDir, "simon2.italian");
	const bool hasFrench = hasSimon2LangFile(gameDir, "simon2.french");

	if (!hasEnglish && !hasGerman && !hasItalian && !hasFrench)
		return;

	game.setGUIOptions(stripLanguageGUIOptions(game.getGUIOptions()));

	if (hasEnglish)
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::EN_ANY));
	if (hasGerman)
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::DE_DEU));
	if (hasItalian)
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::IT_ITA));
	if (hasFrench)
		game.appendGUIOptions(Common::getGameGUIOptionsDescriptionLanguage(Common::FR_FRA));
}

using namespace AGOS;

class AgosMetaEngineDetection : public AdvancedMetaEngineDetection<AGOS::AGOSGameDescription> {
public:
	AgosMetaEngineDetection() : AdvancedMetaEngineDetection(AGOS::gameDescriptions, agosGames) {
		_guiOptions = GUIO1(GUIO_NOLAUNCHLOAD);
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	PlainGameDescriptor findGame(const char *gameId) const override {
		return Engines::findGameID(gameId, _gameIds, obsoleteGameIDsTable);
	}

	Common::Error identifyGame(DetectedGame &game, const void **descriptor) override {
		Engines::upgradeTargetIfNecessary(obsoleteGameIDsTable);
		Common::Error err = AdvancedMetaEngineDetection::identifyGame(game, descriptor);

		if (err.getCode() == Common::kNoError && game.gameId == "simon2" && ConfMan.hasKey("path")) {
			Common::FSNode gameDir(ConfMan.getPath("path"));
			applySimon2LanguageGUIOptions(game, gameDir);
		}

		return err;
	}

	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override {
		DetectedGames games = AdvancedMetaEngineDetection::detectGames(fslist, skipADFlags, skipIncomplete);

		if (fslist.empty())
			return games;

		Common::FSNode gameDir = fslist.begin()->getParent();

		for (DetectedGame &game : games) {
			if (game.gameId == "simon2")
				applySimon2LanguageGUIOptions(game, gameDir);
		}

		return games;
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
