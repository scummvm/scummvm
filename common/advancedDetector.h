/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef COMMON_ADVANCED_DETECTOR_H
#define COMMON_ADVANCED_DETECTOR_H

#include "common/fs.h"

#include "base/game.h"	// For PlainGameDescriptor and GameList
#include "base/plugins.h"	// For PluginError


namespace Common {

struct ADGameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
	const int32 fileSize;
};

struct ADGameDescription {
	const char *gameid;
	const char *extra;
	const ADGameFileDescription filesDescriptions[14];
	Language language;
	Platform platform;
};

struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

struct ADParams {
	// Pointer to ADGameDescription or its superset structure
	const byte *descs;
	// Size of that superset structure
	const int descItemSize;
	// Number of bytes to compute MD5 sum for
	const int md5Bytes;
	// List of all engine targets
	const PlainGameDescriptor *list;
	// Structure for autoupgrading obsolete targets
	const Common::ADObsoleteGameID *obsoleteList;
};

typedef Array<int> ADList;
typedef Array<const ADGameDescription*> ADGameDescList;

#define AD_ENTRY1(f, x) {{ f, 0, x, -1}, {NULL, 0, NULL, 0}}


// TODO/FIXME: Fingolfin asks: Why is AdvancedDetector a class, considering that
// it is only used as follow:
//  1) Create an instance of it on the stack
//  2) invoke registerGameDescriptions and setFileMD5Bytes 
//  3) invoke detectGame *once*
// Obviously, 2) could also be handled by passing more params to detectGame.
// So it seem we could replace this class by a simple advancedDetectGame(...)
// function, w/o a class or instantiating object... ? Or is there a deeper
// reason I miss?
class AdvancedDetector {
public:

	void registerGameDescriptions(ADGameDescList gameDescriptions) {
		_gameDescriptions = gameDescriptions;
	}

	/**
	 * Detect games in specified directory.
	 * Parameters language and platform are used to pass on values
	 * specified by the user. I.e. this is used to restrict search scope.
	 *
	 * @param fslist	FSList to scan or NULL for scanning all specified
	 *  default directories.
	 * @param md5Bytes	number of bytes which are used to calculate MD5
	 * @param language	restrict results to specified language only
	 * @param platform	restrict results to specified platform only
	 * @return	list of indexes to GameDescriptions of matched games
	 */
	ADList detectGame(const FSList *fslist, const Common::ADParams &params, Language language, Platform platform);

private:
	ADGameDescList _gameDescriptions;
};


// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
GameDescriptor ADVANCED_DETECTOR_FIND_GAMEID(
	const char *gameid,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
GameList ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
	const FSList &fslist,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
int ADVANCED_DETECTOR_DETECT_INIT_GAME(
	const Common::ADParams &params
	);

// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
PluginError ADVANCED_DETECTOR_ENGINE_CREATE(
	GameList (*detectFunc)(const FSList &fslist),
	const Common::ADParams &params
	);


#define ADVANCED_DETECTOR_DEFINE_PLUGIN(engine,createFunction,detectFunc,params) \
	GameList Engine_##engine##_gameIDList() { \
		return GameList(params.list); \
	} \
	GameDescriptor Engine_##engine##_findGameID(const char *gameid) { \
		return Common::ADVANCED_DETECTOR_FIND_GAMEID(gameid, params); \
	} \
	GameList Engine_##engine##_detectGames(const FSList &fslist) { \
		return detectFunc(fslist);						\
	} \
	PluginError Engine_##engine##_create(OSystem *syst, Engine **engine) { \
		assert(syst); \
		assert(engine); \
		PluginError err = ADVANCED_DETECTOR_ENGINE_CREATE(detectFunc, params); \
		if (err == kNoError) \
			*engine = new createFunction(syst); \
		return err; \
	} \
	void dummyFuncToAllowTrailingSemicolon()


}	// End of namespace Common

#endif
