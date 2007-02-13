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

#define AD_ENTRY1(f, x) {{ f, 0, x, -1}, {NULL, 0, NULL, 0}}
#define AD_ENTRY1s(f, x, s) {{ f, 0, x, s}, {NULL, 0, NULL, 0}}

enum ADGameFlags {
	ADGF_NO_FLAGS = 0,
	ADGF_DEMO = (1 << 30)
};

struct ADGameDescription {
	const char *gameid;
	const char *extra;
	ADGameFileDescription filesDescriptions[14];
	Language language;
	Platform platform;
	
	/**
	 * A bitmask of extra flags. The top 8 bits are reserved for generic flags
	 * defined in the ADGameFlags. This leaves 24 flags to be used by client
	 * code.
	 */
	uint32 flags;
};

/**
 * End marker for a table of ADGameDescription structs. Use this to
 * terminate a list to be passed to the AdvancedDetector API.
 */
#define AD_TABLE_END_MARKER	\
	{ NULL, NULL, { { NULL, 0, NULL, 0 } }, Common::UNK_LANG, Common::kPlatformUnknown, Common::ADGF_NO_FLAGS }


struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

enum ADFlags {
	/**
	 * Generate/augment preferred target with information on the language (if
	 * not equal to english) and platform (if not equal to PC).
	 */
	kADFlagAugmentPreferredTarget = (1 << 0),
	kADFlagFilebasedFallback = (1 << 1)  // Use file based fallback detection
};

/**
 * A structure containing all parameters for the AdvancedDetector.
 * Typically, an engine will have a single instance of this which is
 * then passed to the various AdvancedDetector functions.
 */
struct ADParams {
	/**
	 * Pointer to an array of objects which are either ADGameDescription
	 * or superset structures (i.e. start with an ADGameDescription member.
	 * The list is terminated by an entry with a gameid equal to 0
	 * (see AD_TABLE_END_MARKER).
	 */
	const byte *descs;

	/**
	 * The size of a single entry of the above descs array. Always
	 * must be >= sizeof(ADGameDescription).
	 */
	uint descItemSize;

	/**
	 * The number of bytes to compute MD5 sum for. The AdvancedDetector
	 * is primarily based on computing and matching MD5 checksums of files.
	 * Since doing that for large files can be slow, it can be restricted
	 * to a subset of all files.
	 * Typically this will be set to something between 5 and 50 kilobyte,
	 * but arbitrary non-zero values are possible.
	 */
	uint md5Bytes;

	/**
	 * A list of all gameids (and their corresponding descriptions) supported
	 * by this engine.
	 */
	const PlainGameDescriptor *list;

	/**
	 * Structure for autoupgrading obsolete targets (optional)
	 *
	 * @todo Properly explain this.
	 */
	const Common::ADObsoleteGameID *obsoleteList;

	/**
	 * Name of single gameid (optional).
	 *
	 * @todo Properly explain this -- what does it do?
	 */
	const char *singleid;

	/**
	 * List of files for file-based fallback detection (optional)
	 
	 * @todo Properly explain this
	 */
	const char **fileBasedFallback;

	/** Flags */
	uint32 flags;
};


namespace AdvancedDetector {

/**
 * Returns list of targets supported by the engine.
 * Distinguishes engines with single ID
 */
GameList gameIDList(const Common::ADParams &params);

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
	const Common::ADParams &params
	);


// FIXME: It would probably be good to merge detectBestMatchingGame
// and detectGameForEngineCreation into a single function. Right now, the
// detection code called priort to creating an engine instance
// (i.e. detectGameForEngineCreation) differs from the detection code the 
// engines call internally (i.e. detectBestMatchingGame). This could lead
// to hard to debug and odd errors.


} // End of namespace AdvancedDetector


#define ADVANCED_DETECTOR_DEFINE_PLUGIN_WITH_FUNC(engine,factoryFunc,params) \
	GameList Engine_##engine##_gameIDList() { \
		return Common::AdvancedDetector::gameIDList(params); \
	} \
	GameDescriptor Engine_##engine##_findGameID(const char *gameid) { \
		return Common::AdvancedDetector::findGameID(gameid, params); \
	} \
	GameList Engine_##engine##_detectGames(const FSList &fslist) { \
		return Common::AdvancedDetector::detectAllGames(fslist, params); \
	} \
	PluginError Engine_##engine##_create(OSystem *syst, Engine **engine) { \
		assert(syst); \
		assert(engine); \
		Common::AdvancedDetector::upgradeTargetIfNecessary(params); \
		PluginError err = Common::AdvancedDetector::detectGameForEngineCreation(params); \
		if (err == kNoError) \
			*engine = factoryFunc(syst); \
		return err; \
	} \
	void dummyFuncToAllowTrailingSemicolon()

#define ADVANCED_DETECTOR_DEFINE_PLUGIN(engine,className,params) \
	static Engine *engine##_createInstance(OSystem *syst) { \
		return new className(syst); \
	} \
	ADVANCED_DETECTOR_DEFINE_PLUGIN_WITH_FUNC(engine,engine##_createInstance,params); \
	void dummyFuncToAllowTrailingSemicolon()


}	// End of namespace Common

#endif
