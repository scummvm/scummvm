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
	uint16 fileType;      // Optional. Not used during detection, only by engines.
	const char *md5;      // Optional. May be NULL.
	const int32 fileSize; // Optional. Set to -1 to ignore.
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


// FIXME/TODO: Rename this function to something more sensible.
GameDescriptor ADVANCED_DETECTOR_FIND_GAMEID(
	const char *gameid,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
GameList ADVANCED_DETECTOR_DETECT_GAMES_FUNCTION(
	const FSList &fslist,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
int ADVANCED_DETECTOR_DETECT_INIT_GAME(
	const Common::ADParams &params
	);

// FIXME/TODO: Rename this function to something more sensible.
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
