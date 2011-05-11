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
#ifndef ENGINES_ADVANCED_DETECTOR_H
#define ENGINES_ADVANCED_DETECTOR_H

#include "engines/metaengine.h"

namespace Common {
class Error;
class FSList;
}


struct ADGameFileDescription {
	const char *fileName;
	uint16 fileType; // Optional. Not used during detection, only by engines.
	const char *md5; // Optional. May be NULL.
	int32 fileSize;  // Optional. Set to -1 to ignore.
};

#define AD_LISTEND {NULL, 0, NULL, 0}

#define AD_ENTRY1(f, x) {{ f, 0, x, -1}, AD_LISTEND}
#define AD_ENTRY1s(f, x, s) {{ f, 0, x, s}, AD_LISTEND}

enum ADGameFlags {
	ADGF_NO_FLAGS = 0,
	ADGF_PIRATED = (1 << 23), // flag to designate well known pirated versions with cracks
	ADGF_ADDENGLISH = (1 << 24), // always add English as language option
	ADGF_MACRESFORK = (1 << 25), // the md5 for this entry will be calculated from the resource fork
	ADGF_USEEXTRAASTITLE = (1 << 26), // Extra field value will be used as main game title, not gameid
	ADGF_DROPLANGUAGE = (1 << 28), // don't add language to gameid
	ADGF_CD = (1 << 29),    	// add "-cd" to gameid
	ADGF_DEMO = (1 << 30)   	// add "-demo" to gameid
};

struct ADGameDescription {
	const char *gameid;
	const char *extra;
	ADGameFileDescription filesDescriptions[14];
	Common::Language language;
	Common::Platform platform;

	/**
	 * A bitmask of extra flags. The top 16 bits are reserved for generic flags
	 * defined in the ADGameFlags. This leaves 16 bits to be used by client
	 * code.
	 */
	uint32 flags;

	uint32 guioptions;
};

/**
 * End marker for a table of ADGameDescription structs. Use this to
 * terminate a list to be passed to the AdvancedDetector API.
 */
#define AD_TABLE_END_MARKER	\
	{ NULL, NULL, { { NULL, 0, NULL, 0 } }, Common::UNK_LANG, Common::kPlatformUnknown, ADGF_NO_FLAGS, Common::GUIO_NONE }


struct ADObsoleteGameID {
	const char *from;
	const char *to;
	Common::Platform platform;
};

struct ADFileBasedFallback {
	/**
	 * Pointer to an ADGameDescription or subclass thereof which will get
	 * returned if there's a detection match.
	 */
	const void *desc;

	/**
	 * A zero-terminated list of filenames used for matching. All files in
	 * the list must be present to get a detection match.
	 */
	const char *filenames[10];
};


enum ADFlags {
	/**
	 * Generate/augment preferred target with information on the language (if
	 * not equal to english) and platform (if not equal to PC).
	 */
	kADFlagDontAugmentPreferredTarget = (1 << 0),
	/**
	 * Warn user about new variant if his version was detected with fallback
	 */
	kADFlagPrintWarningOnFileBasedFallback = (1 << 1),
	/**
	 * Store value of extra field in config file, and use it as a hint
	 * on subsequent runs. Could be used when there is no way to autodetect
	 * game (when more than one game sits in same directory), and user picks
	 * up a variant manually.
	 */
	kADFlagUseExtraAsHint = (1 << 2)
};

/**
 * A structure containing all parameters for the AdvancedDetector.
 * Typically, an engine will have a single instance of this which is
 * used by its AdvancedMetaEngine subclass as a parameter to the
 * primary AdvancedMetaEngine constructor.
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
	 * Structure for autoupgrading obsolete targets (optional).
	 *
	 * @todo Properly explain this.
	 */
	const ADObsoleteGameID *obsoleteList;

	/**
	 * Name of single gameid (optional).
	 *
	 * @todo Properly explain this -- what does it do?
	 */
	const char *singleid;

	/**
	 * List of files for file-based fallback detection (optional).
	 * This is used if the regular MD5 based detection failed to
	 * detect anything.
	 * As usual this list is terminated by an all-zero entry.
	 *
	 * @todo Properly explain this
	 */
	const ADFileBasedFallback *fileBasedFallback;

	/**
	 * A bitmask of flags which can be used to configure the behavior
	 * of the AdvancedDetector. Refer to ADFlags for a list of flags
	 * that can be ORed together and passed here.
	 */
	uint32 flags;

	/**
	 * A bitmask of game GUI options which will be added to each
	 * entry in addition to per-game options. Refer to GameGUIOption
	 * enum for the list.
	 */
	uint32 guioptions;

	/**
	 * Maximum depth of directories to look up
	 * If set to 0, the depth is 1 level
	 */
	uint32 depth;

	/**
	 * Case-insensitive list of directory globs which could be used for
	 * going deeper int directory structure.
	 * @see String::matchString() method for format description.
	 *
	 * @note Last item must be 0
	 */
	const char * const *directoryGlobs;
};


namespace AdvancedDetector {

/**
 * Scan through the game descriptors specified in params and search for
 * 'gameid' in there. If a match is found, returns a GameDescriptor
 * with gameid and description set.
 */
GameDescriptor findGameID(
	const char *gameid,
	const PlainGameDescriptor *list,
	const ADObsoleteGameID *obsoleteList = 0
	);

} // End of namespace AdvancedDetector

/**
 * A MetaEngine implementation based around the advanced detector code.
 */
class AdvancedMetaEngine : public MetaEngine {
	const ADParams &params;
public:
	AdvancedMetaEngine(const ADParams &dp) : params(dp) {}

	virtual GameList getSupportedGames() const;
	virtual GameDescriptor findGame(const char *gameid) const;
	virtual GameList detectGames(const Common::FSList &fslist) const;
	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;

	// To be provided by subclasses
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const = 0;

	/**
	 * An (optional) generic fallback detect function which is invoked
	 * if both the regular MD5 based detection as well as the file
	 * based fallback failed to detect anything.
	 */
	virtual const ADGameDescription *fallbackDetect(const Common::FSList &fslist) const {
		return 0;
	}
};

#endif
