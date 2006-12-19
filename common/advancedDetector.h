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
#include "base/plugins.h"	// For DetectedGameList


namespace Common {

struct ADGameFileDescription {
	const char *fileName;
	uint16 fileType;
	const char *md5;
};

struct ADGameDescription {
	const char *name;
	const char *extra;
	const ADGameFileDescription *filesDescriptions;
	Language language;
	Platform platform;
};

struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

typedef Array<int> ADList;
typedef Array<const ADGameDescription*> ADGameDescList;

// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
// Maybe rename it to something like asGameList or pgdArrayToGameList,
// and move it to base/game.h. Or add a constructor to GameList ... ?
GameList real_ADVANCED_DETECTOR_GAMEID_LIST(const PlainGameDescriptor *list);

// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
GameDescriptor real_ADVANCED_DETECTOR_FIND_GAMEID(
	const char *gameid,
	const PlainGameDescriptor *list,
	const Common::ADObsoleteGameID *obsoleteList
	);


// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
// Also, we could get rid of the descSize parameter, if we simply terminated the
// list of game descriptions by an all-zero entry (like the SCUMM engine does in
// similar cases).
DetectedGameList real_ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
	const FSList &fslist,
	const byte *descs,
	const int descItemSize,
	const int md5Bytes,
	const PlainGameDescriptor *list
	);


// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
// Also, we could get rid of the descSize parameter, if we simply terminated the
// list of game descriptions by an all-zero entry (like the SCUMM engine does in
// similar cases).
int real_ADVANCED_DETECTOR_DETECT_INIT_GAME(
	const byte *descs,
	const int descItemSize,
	const int md5Bytes,
	const PlainGameDescriptor *list
	);

// FIXME/TODO: Rename this function to something more sensible.
// Possibly move it inside class AdvancedDetector ?
PluginError real_ADVANCED_DETECTOR_ENGINE_CREATE(
	DetectedGameList (*detectFunc)(const FSList &fslist),
	const Common::ADObsoleteGameID *obsoleteList
	);


#define ADVANCED_DETECTOR_GAMEID_LIST(engine,list) \
	GameList Engine_##engine##_gameIDList() { \
		return Common::real_ADVANCED_DETECTOR_GAMEID_LIST(list); \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_FIND_GAMEID(engine,list,obsoleteList) \
	GameDescriptor Engine_##engine##_findGameID(const char *gameid) { \
		return Common::real_ADVANCED_DETECTOR_FIND_GAMEID(gameid,list,obsoleteList); \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DETECT_GAMES(engine,detectFunc) \
	DetectedGameList Engine_##engine##_detectGames(const FSList &fslist) { \
		return detectFunc(fslist);						\
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_ENGINE_CREATE(engine,createFunction,detectFunc,obsoleteList) \
	PluginError Engine_##engine##_create(OSystem *syst, Engine **engine) { \
		assert(syst); \
		assert(engine); \
		PluginError err = real_ADVANCED_DETECTOR_ENGINE_CREATE(detectFunc, obsoleteList); \
		if (err == kNoError) \
			*engine = new createFunction(syst); \
		return err; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DEFINE_PLUGIN(engine,createFunction,detectFunc,list,obsoleteList) \
	ADVANCED_DETECTOR_GAMEID_LIST(engine, list); \
	ADVANCED_DETECTOR_FIND_GAMEID(engine, list, obsoleteList); \
	ADVANCED_DETECTOR_DETECT_GAMES(engine, detectFunc); \
	ADVANCED_DETECTOR_ENGINE_CREATE(engine, createFunction, detectFunc, obsoleteList)


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
	AdvancedDetector();
	~AdvancedDetector() {};


	void registerGameDescriptions(ADGameDescList gameDescriptions) {
		_gameDescriptions = gameDescriptions;
	}

	/**
	 * Specify number of bytes which are used to calculate MD5.
	 * Default value is 0 which means whole file.
	 */
	void setFileMD5Bytes(int bytes) { _fileMD5Bytes = bytes; }

	/**
	 * Detect games in specified directory.
	 * Parameters language and platform are used to pass on values
	 * specified by the user. I.e. this is used to restrict search scope.
	 *
	 * @param fslist	FSList to scan or NULL for scanning all specified
	 *  default directories.
	 * @param language	restrict results to specified language only
	 * @param platform	restrict results to specified platform only
	 * @return	list of indexes to GameDescriptions of matched games
	 */
	ADList detectGame(const FSList *fslist, Language language, Platform platform);

private:
	ADGameDescList _gameDescriptions;

	int _fileMD5Bytes;

	String getDescription(int num) const;
};

}	// End of namespace Common

#endif
