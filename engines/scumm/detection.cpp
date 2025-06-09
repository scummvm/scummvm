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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "base/plugins.h"

#include "engines/metaengine.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/list.h"
#include "common/md5.h"
#include "common/system.h"
#include "common/translation.h"

#include "audio/mididrv.h"

#include "scumm/detection.h"
#include "scumm/detection_tables.h"
#include "scumm/file.h"
#include "scumm/file_nes.h"
#include "scumm/scumm-md5.h"

#pragma mark -
#pragma mark --- Detection code ---
#pragma mark -


// Various methods to help in core detection.
#include "scumm/detection_internal.h"


#pragma mark -
#pragma mark --- Plugin code ---
#pragma mark -

static const DebugChannelDef debugFlagList[] = {
		{Scumm::DEBUG_SCRIPTS, "SCRIPTS", "Track script execution"},
		{Scumm::DEBUG_OPCODES, "OPCODES", "Track opcode execution"},
		{Scumm::DEBUG_IMUSE, "IMUSE", "Track iMUSE events"},
		{Scumm::DEBUG_RESOURCE, "RESOURCE", "Track resource loading/management"},
		{Scumm::DEBUG_VARS, "VARS", "Track variable changes",},
		{Scumm::DEBUG_ACTORS, "ACTORS", "Actor-related debug"},
		{Scumm::DEBUG_SOUND, "SOUND", "Sound related debug"},
		{Scumm::DEBUG_INSANE, "INSANE", "Track INSANE"},
		{Scumm::DEBUG_SMUSH, "SMUSH", "Track SMUSH"},
		{Scumm::DEBUG_MOONBASE_AI, "MOONBASEAI", "Track Moonbase AI"},
		{Scumm::DEBUG_NETWORK, "NETWORK", "Track Networking"},
		DEBUG_CHANNEL_END
};

using namespace Scumm;

class ScummMetaEngineDetection : public MetaEngineDetection {
public:
	const char *getName() const override {
		return "scumm";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}

	const char *getEngineName() const override;
	const char *getOriginalCopyright() const override;

	PlainGameList getSupportedGames() const override;
	PlainGameDescriptor findGame(const char *gameid) const override;
	Common::Error identifyGame(DetectedGame &game, const void **descriptor) override;
	DetectedGames detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) override;

	uint getMD5Bytes() const override {
		 return 1024 * 1024;
	}
	int getGameVariantCount() const override {
		int entries = 0;
		for (const GameSettings *g = gameVariantsTable; g->gameid; ++g)
			++entries;
		return entries;
	}

	void dumpDetectionEntries() const override final;

	GameFilenamePattern matchGameFilenamePattern(const MD5Table *entry) const;
};

PlainGameList ScummMetaEngineDetection::getSupportedGames() const {
	return PlainGameList(gameDescriptions);
}

GameFilenamePattern ScummMetaEngineDetection::matchGameFilenamePattern(const MD5Table *entry) const {
	GameFilenamePattern bestMatch = GameFilenamePattern();

	for (const GameFilenamePattern *gfp = gameFilenamesTable; gfp->gameid; ++gfp) {
		if (!scumm_stricmp(gfp->gameid, entry->gameid)) {
			if (gfp->platform == UNK || entry->platform == UNK || gfp->platform == entry->platform) {
				bestMatch = *gfp;

				if (gfp->language == UNK_LANG || entry->language == UNK_LANG || gfp->language == entry->language) {
					if (!gfp->variant || !entry->variant || !scumm_stricmp(gfp->variant, entry->variant))
						return *gfp;
				}
			}
		}
	}

	return bestMatch;
}

#if 0
struct EntryPos {
	const char *gameid;
	int id;

	EntryPos(const char *strId, int intId) : gameid(strId), id(intId) {}
};

static int compareTreeNodes(const void *a, const void *b) {
	return scumm_stricmp(((const EntryPos *)a)->gameid, ((const EntryPos *)b)->gameid);
}
#endif

 void ScummMetaEngineDetection::dumpDetectionEntries() const {
#if 0
	// Sort all entries by gameid, as they are sorted by md5
	Common::SortedArray<EntryPos *> gameIDs(compareTreeNodes);
	for (int i = 0; md5table[i].gameid != 0; ++i)
		gameIDs.insert(new EntryPos(md5table[i].gameid , i));

	for (auto &iter : gameIDs) {
		const MD5Table *entry = &md5table[iter->id];
		PlainGameDescriptor pd = findGame(entry->gameid);
		const char *title = pd.description;

		printf("game (\n");
		printf("\tname \"%s\"\n", escapeString(entry->gameid).c_str());
		printf("\ttitle \"%s\"\n", escapeString(title).c_str());
		printf("\textra \"%s\"\n", escapeString(entry->extra).c_str());
		printf("\tlanguage \"%s\"\n", escapeString(getLanguageLocale(entry->language)).c_str());
		printf("\tplatform \"%s\"\n", escapeString(getPlatformCode(entry->platform)).c_str());
		printf("\tsourcefile \"%s\"\n", escapeString(getName()).c_str());
		printf("\tengine \"%s\"\n", escapeString("scumm").c_str());

		// Match the appropriate file name for the current game variant.
		GameFilenamePattern gameEntry = matchGameFilenamePattern(entry);
		Common::String fileName;

		if (gameEntry.gameid) {
			fileName = generateFilenameForDetection(gameEntry.pattern, gameEntry.genMethod, gameEntry.platform);
		} else {
			warning("dumpDetectionEntries(): no game entry found for '%s'", entry->gameid);
			fileName = entry->gameid;
		}

		printf("\trom ( name \"%s\" size %lld md5-%d %s )\n",
			escapeString(fileName.c_str()).c_str(),
			static_cast<long long int>(entry->filesize),
			getMD5Bytes(),
			entry->md5);

		printf(")\n\n"); // Closing for 'game ('
	}

	for (auto &iter : gameIDs)
		delete iter;
#endif
}

PlainGameDescriptor ScummMetaEngineDetection::findGame(const char *gameid) const {
	return Engines::findGameID(gameid, gameDescriptions, obsoleteGameIDsTable);
}

Common::Error ScummMetaEngineDetection::identifyGame(DetectedGame &game, const void **descriptor) {
	*descriptor = nullptr;
	game = DetectedGame(getName(), findGame(ConfMan.get("gameid").c_str()));
	return game.gameId.empty() ? Common::kUnknownError : Common::kNoError;
}

static Common::String generatePreferredTarget(const DetectorResult &x) {
	Common::String res(x.game.gameid);

	if (x.game.preferredTag) {
		res = res + "-" + x.game.preferredTag;
	}

	if (x.game.features & GF_DEMO) {
		res = res + "-demo";
	}

	// Append the platform, if a non-standard one has been specified.
	if (x.game.platform != Common::kPlatformDOS && x.game.platform != Common::kPlatformUnknown) {
		// HACK: For CoMI, it's pointless to encode the fact that it's for Windows.
		if (x.game.id != GID_CMI)
			res = res + "-" + Common::getPlatformAbbrev(x.game.platform);
	}

	// Append the language, if a non-standard one has been specified.
	if (x.language != Common::EN_ANY && x.language != Common::UNK_LANG) {
		res = res + "-" + Common::getLanguageCode(x.language);
	}

	return res;
}

DetectedGames ScummMetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 /*skipADFlags*/, bool /*skipIncomplete*/) {
	DetectedGames detectedGames;
	Common::List<DetectorResult> results;
	::detectGames(fslist, results, nullptr);

	for (Common::List<DetectorResult>::iterator
	          x = results.begin(); x != results.end(); ++x) {
		const PlainGameDescriptor *g = findPlainGameDescriptor(x->game.gameid, gameDescriptions);
		assert(g);

		DetectedGame game = DetectedGame(getName(), x->game.gameid, g->description, x->language, x->game.platform, x->extra);

		// Compute and set the preferred target name for this game.
		// Based on generateComplexID() in advancedDetector.cpp.
		game.preferredTarget = generatePreferredTarget(*x);

		game.setGUIOptions(customizeGuiOptions(*x));
		game.appendGUIOptions(getGameGUIOptionsDescriptionLanguage(x->language));
		game.appendGUIOptions(getGameGUIOptionsDescriptionPlatform(x->game.platform));


		detectedGames.push_back(game);
	}

	return detectedGames;
}

const char *ScummMetaEngineDetection::getEngineName() const {
	return "SCUMM ["

#if defined(ENABLE_SCUMM_7_8) && defined(ENABLE_HE)
		"all games"
#else

		"v0-v6 games"

#if defined(ENABLE_SCUMM_7_8)
		", v7 & v8 games"
#endif
#if defined(ENABLE_HE)
		", HE71+ games"
#endif

#endif
		"]";
}

const char *ScummMetaEngineDetection::getOriginalCopyright() const {
	return "LucasArts SCUMM Games (C) LucasArts\n"
	       "Humongous SCUMM Games (C) Humongous";
}

REGISTER_PLUGIN_STATIC(SCUMM_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ScummMetaEngineDetection);
