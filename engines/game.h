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
#include "common/ustr.h"
#include "common/str-array.h"
#include "common/language.h"
#include "common/platform.h"

/**
 * @defgroup engines_game Games
 * @ingroup engines
 *
 * @brief API for managing games by engines.
 *
 * @{
 */

/**
 * A simple structure used to map gameids (like "monkey", "sword1", ...) to
 * nice human readable and descriptive game titles (like "The Secret of Monkey Island").
 * This is a plain struct to make it possible to declare NULL-terminated C arrays
 * consisting of PlainGameDescriptors.
 */
struct PlainGameDescriptor {
	const char *gameId;
	const char *description;

	static PlainGameDescriptor empty();
	static PlainGameDescriptor of(const char *gameId, const char *description);
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
 * The description of a game supported by an engine
 */
struct QualifiedGameDescriptor {
	Common::String engineId;
	Common::String gameId;
	Common::String description;

	QualifiedGameDescriptor() {}
	QualifiedGameDescriptor(const char *engine, const PlainGameDescriptor &pgd);
};

typedef Common::Array<QualifiedGameDescriptor> QualifiedGameList;

/**
 * Ths is an enum to describe how done a game is. This also indicates what level of support is expected.
 */
enum GameSupportLevel {
	kStableGame = 0, // the game is fully supported
	kTestingGame,    // the game is not supposed to end up in releases yet but is ready for public testing
	kUnstableGame,   // the game is not even ready for public testing yet
	kUnupportedGame  // we don't want to support the game
};


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

/**
 * Details about a given game.
 *
 * While PlainGameDescriptor refers to a game supported by an engine, this refers to a game copy
 * that has been detected by an engine's detector.
 * It contains all the necessary data to add the game to the configuration manager and / or to launch it.
 */
struct DetectedGame {
	DetectedGame();
	DetectedGame(const Common::String &engine, const PlainGameDescriptor &pgd);
	DetectedGame(const Common::String &engine, const Common::String &id,
	               const Common::String &description,
	               Common::Language language = Common::UNK_LANG,
	               Common::Platform platform = Common::kPlatformUnknown,
	               const Common::String &extra = Common::String(),
	               bool unsupported = false);

	void setGUIOptions(const Common::String &options);
	void appendGUIOptions(const Common::String &str);
	const Common::String &getGUIOptions() const { return _guiOptions; }

	Common::String engineId;

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

	Common::String gameId;
	Common::String preferredTarget;
	Common::String description;
	Common::Language language;
	Common::Platform platform;
	Common::String path;
	Common::String shortPath;
	Common::String extra;

	/**
	 * What level of support is expected of this game
	 */
	GameSupportLevel gameSupportLevel;

	/**
	 * A list of extra keys to write to the configuration file
	 */
	Common::StringMap _extraConfigEntries;

	/**
	 * Allows adding of extra entries to be saved as part of the detection entry
	 * in the configuration file.
	 * @remarks		Any entry added using this should not be relied on being present
	 *				in the configuration file, since starting games directly from the
	 *				command line bypasses the game detection code
	 */
	void addExtraEntry(const Common::String &key, const Common::String &value) {
		_extraConfigEntries[key] = value;
	}
private:
	/**
	 * Update the description string by appending (EXTRA/PLATFORM/LANG) to it.
	 * Values that are missing are omitted, so e.g. (EXTRA/LANG) would be
	 * added if no platform has been specified but a language and an extra string.
	 */
	Common::String updateDesc(bool skipExtraField) const;

	Common::String _guiOptions;
};

/** List of games. */
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
	DetectedGames listRecognizedGames() const;

	/**
	 * List all the games that were detected
	 *
	 * That includes entries that don't have enough information to be added to the
	 * configuration manager.
	 */
	DetectedGames listDetectedGames() const;

	/**
	 * Were unknown game variants found by the engines?
	 *
	 * When unknown game variants are found, an unknown game report can be generated.
	 */
	bool foundUnknownGames() const;

	/**
	 * Generate a report that we found an unknown game variant.
	 *
	 * @see ::generateUnknownGameReport
	 */
	Common::U32String generateUnknownGameReport(bool translate, uint32 wordwrapAt = 0) const;

private:
	DetectedGames _detectedGames;
};

/**
 * Generate a report that we found an unknown game variant, together with the file
 * names, sizes and MD5 sums.
 *
 * @param translate translate the report to the currently active GUI language
 * @param fullPath include the full path where the files are located, otherwise only the name
 *                 of last component of the path is included
 * @param wordwrapAt word wrap the text part of the report after a number of characters
 */
Common::U32String generateUnknownGameReport(const DetectedGames &detectedGames, bool translate, bool fullPath, uint32 wordwrapAt = 0);
Common::U32String generateUnknownGameReport(const DetectedGame &detectedGame, bool translate, bool fullPath, uint32 wordwrapAt = 0);
/** @} */
#endif
