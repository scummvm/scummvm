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
	uint16 fileType; // Optional. Not used during detection, only by engines.
	const char *md5; // Optional. May be NULL.
	int32 fileSize;  // Optional. Set to -1 to ignore.
};

struct ADGameDescription {
	const char *gameid;
	const char *extra;
	ADGameFileDescription filesDescriptions[14];
	Language language;
	Platform platform;
};

struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

enum ADFlags {
	kADFlagComplexID = (1 << 0) // Generate complex suggested IDs
};

struct ADParams {
	// Pointer to ADGameDescription or its superset structure
	const byte *descs;
	// Size of that superset structure
	int descItemSize;
	// Number of bytes to compute MD5 sum for
	int md5Bytes;
	// List of all engine targets
	const PlainGameDescriptor *list;
	// Structure for autoupgrading obsolete targets (optional)
	const Common::ADObsoleteGameID *obsoleteList;
	// Name of single gameid (optional)
	const char *singleid;
	// Flags
	const uint32 flags;
};

typedef Array<int> ADList;
typedef Array<const ADGameDescription*> ADGameDescList;

#define AD_ENTRY1(f, x) {{ f, 0, x, -1}, {NULL, 0, NULL, 0}}
#define AD_ENTRY1s(f, x, s) {{ f, 0, x, s}, {NULL, 0, NULL, 0}}


namespace AdvancedDetector {

/**
 * Scan through the game descriptors specified in params and search for
 * 'gameid' in there. If a match is found, returns a  GameDescriptor
 * with gameid and description set.
 */
GameDescriptor findGameID(
	const char *gameid,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
GameList detectAllGames(
	const FSList &fslist,
	const Common::ADParams &params
	);


// FIXME/TODO: Rename this function to something more sensible.
int detectBestMatchingGame(
	const Common::ADParams &params
	);

// FIXME/TODO: Rename this function to something more sensible.
void upgradeTargetIfNecessary(const Common::ADParams &params);

// FIXME/TODO: Rename this function to something more sensible.
PluginError detectGameForEngineCreation(
	GameList (*detectFunc)(const FSList &fslist),
	const Common::ADParams &params
	);


// FIXME: It would probably be good to merge detectBestMatchingGame
// and detectGameForEngineCreation into a single function. Right now, the
// detection code called priort to creating an engine instance
// (i.e. detectGameForEngineCreation) differs from the detection code the 
// engines call internally (i.e. detectBestMatchingGame). This could lead
// to hard to debug and odd errors.


} // End of namespace AdvancedDetector


#define ADVANCED_DETECTOR_DEFINE_PLUGIN_WITH_FUNC(engine,factoryFunc,detectFunc,params) \
	GameList Engine_##engine##_gameIDList() { \
		return GameList(params.list); \
	} \
	GameDescriptor Engine_##engine##_findGameID(const char *gameid) { \
		return Common::AdvancedDetector::findGameID(gameid, params); \
	} \
	GameList Engine_##engine##_detectGames(const FSList &fslist) { \
		return detectFunc(fslist);						\
	} \
	PluginError Engine_##engine##_create(OSystem *syst, Engine **engine) { \
		assert(syst); \
		assert(engine); \
		Common::AdvancedDetector::upgradeTargetIfNecessary(params); \
		PluginError err = Common::AdvancedDetector::detectGameForEngineCreation(detectFunc, params); \
		if (err == kNoError) \
			*engine = factoryFunc(syst); \
		return err; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DEFINE_PLUGIN(engine,className,detectFunc,params) \
	static className *engine##_createInstance(OSystem *syst) { \
		return new className(syst); \
	} \
	ADVANCED_DETECTOR_DEFINE_PLUGIN_WITH_FUNC(engine,engine##_createInstance,detectFunc,params); \
	void dummyFuncToAllowTrailingSemicolon()


}	// End of namespace Common

#endif
