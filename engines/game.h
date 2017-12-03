/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ENGINES_GAME_H
#define ENGINES_GAME_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"

/**
 * A simple structure used to map gameids (like "monkey", "sword1", ...) to
 * nice human readable and descriptive game titles (like "The Secret of Monkey Island").
 * This is a plain struct to make it possible to declare NULL-terminated C arrays
 * consisting of PlainGameDescriptors.
 */
struct PlainGameDescriptor {
	const char *gameId;
	const char *description;

	PlainGameDescriptor() : gameId(nullptr), description(nullptr) {}
	PlainGameDescriptor(const char *id, const char *desc) : gameId(id), description(desc) {}
};

/**
 * Given a list of PlainGameDescriptors, returns the first PlainGameDescriptor
 * matching the given gameid. If not match is found return 0.
 * The end of the list must be marked by an entry with gameid 0.
 */
const PlainGameDescriptor *findPlainGameDescriptor(const char *gameid, const PlainGameDescriptor *list);

class PlainGameList : public Common::Array<PlainGameDescriptor> {
public:
	PlainGameList() {}
	PlainGameList(const PlainGameList &list) : Common::Array<PlainGameDescriptor>(list) {}
	PlainGameList(const PlainGameDescriptor *g) {
		while (g->gameId) {
			push_back(*g);
			g++;
		}
	}
};

/**
 * Ths is an enum to describe how done a game is. This also indicates what level of support is expected.
 */
enum GameSupportLevel {
	kStableGame = 0, // the game is fully supported
	kTestingGame, // the game is not supposed to end up in releases yet but is ready for public testing
	kUnstableGame // the game is not even ready for public testing yet
};

/**
 * A hashmap describing details about a given game. In a sense this is a refined
 * version of PlainGameDescriptor, as it also contains a gameid and a description string.
 * But in addition, platform and language settings, as well as arbitrary other settings,
 * can be contained in a GameDescriptor.
 * This is an essential part of the glue between the game engines and the launcher code.
 */
class GameDescriptor {
public:
	GameDescriptor();
	explicit GameDescriptor(const PlainGameDescriptor &pgd, const Common::String &guioptions = Common::String());
	GameDescriptor(const Common::String &id,
	              const Common::String &description,
	              Common::Language language = Common::UNK_LANG,
				  Common::Platform platform = Common::kPlatformUnknown,
				  const Common::String &guioptions = Common::String(),
				  GameSupportLevel gsl = kStableGame);

	/**
	 * Update the description string by appending (EXTRA/PLATFORM/LANG) to it.
	 * Values that are missing are omitted, so e.g. (EXTRA/LANG) would be
	 * added if no platform has been specified but a language and an extra string.
	 */
	void updateDesc(const char *extraDesc = 0);

	void setGUIOptions(const Common::String &options);
	void appendGUIOptions(const Common::String &str);
	Common::String getGUIOptions() const { return _guiOptions; }

	Common::String gameId;
	Common::String preferredTarget;
	Common::String description;
	Common::Language language;
	Common::Platform platform;
	Common::String path;
	Common::String extra;

	/**
	 * What level of support is expected of this game
	 */
	GameSupportLevel gameSupportLevel;

private:
	Common::String _guiOptions;
};

/** List of games. */
typedef Common::Array<GameDescriptor> GameList;

/**
 * A record describing the properties of a file. Used on the existing
 * files while detecting a game.
 */
struct FileProperties {
	int32 size;
	Common::String md5;

	FileProperties() : size(-1) {}
};

/**
 * A map of all relevant existing files while detecting.
 */
typedef Common::HashMap<Common::String, FileProperties, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FilePropertiesMap;

struct DetectedGame {
	/**
	 * The name of the engine supporting the detected game
	 */
	const char *engineName;

	/**
	 * The identifier of the detected game
	 *
	 * For engines using the singleId feature, this is the true engine-specific gameId, not the singleId.
	 */
	const char *gameId;

	/**
	 * A game was detected, but some files were not recognized
	 *
	 * This can happen when the md5 or size of the detected files did not match the engine's detection tables.
	 * When this is true, the list of matched files below contains detail about the unknown files.
	 *
	 * @see matchedFiles
	 */
	bool hasUnknownFiles;

	/**
	 * An optional list of the files that were used to match the game with the engine's detection tables
	 */
	FilePropertiesMap matchedFiles;

	/**
	 * This detection entry contains enough data to add the game to the configuration manager and launch it
	 *
	 * @see matchedGame
	 */
	bool canBeAdded;

	/**
	 * Details about the detected game
	 */
	GameDescriptor matchedGame;

	DetectedGame() : engineName(nullptr), gameId(nullptr), hasUnknownFiles(false), canBeAdded(true) {}
};

typedef Common::Array<DetectedGame> DetectedGames;

/**
 * Contains a list of games found by the engines' detectors.
 *
 * Each detected game can either:
 * - be fully recognized (e.g. an exact match was found in the detection tables of an engine)
 * - be an unknown variant (e.g. a game using files with the same name was found in the detection tables)
 * - be recognized with unknown files (e.g. the game was exactly not found in the detection tables,
 *              but the detector was able to gather enough data to allow launching the game)
 *
 * Practically, this means a detected game can be in both the recognized game list and in the unknown game
 * report handled by this class.
 */
class DetectionResults {
public:
	explicit DetectionResults(const DetectedGames &detectedGames);

	/**
	 * List all the games that were recognized by the engines
	 *
	 * Recognized games can be added to the configuration manager and then launched.
	 */
	DetectedGames listRecognizedGames();

	/**
	 * Were unknown game variants found by the engines?
	 *
	 * When unknown game variants are found, an unknown game report can be generated.
	 */
	bool foundUnknownGames() const;

	/**
	 * Generate a report that we found an unknown game variant, together with the file
	 * names, sizes and MD5 sums.
	 *
	 * @param translate translate the report to the currently active GUI language
	 * @param wordwrapAt word wrap the text part of the report after a number of characters
	 */
	Common::String generateUnknownGameReport(bool translate, uint32 wordwrapAt = 0) const;

private:
	DetectedGames _detectedGames;
};

#endif
