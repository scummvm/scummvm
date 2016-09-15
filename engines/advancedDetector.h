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

#ifndef ENGINES_ADVANCED_DETECTOR_H
#define ENGINES_ADVANCED_DETECTOR_H

#include "engines/metaengine.h"
#include "engines/engine.h"

#include "common/hash-str.h"

#include "common/gui_options.h" // FIXME: Temporary hack?

namespace Common {
class Error;
class FSList;
}

/**
 * A record describing a file to be matched for detecting a specific game
 * variant. A list of such records is used inside every ADGameDescription to
 * enable detection.
 */
struct ADGameFileDescription {
	const char *fileName; ///< Name of described file.
	uint16 fileType;      ///< Optional. Not used during detection, only by engines.
	const char *md5;      ///< MD5 of (the beginning of) the described file. Optional. Set to NULL to ignore.
	int32 fileSize;       ///< Size of the described file. Set to -1 to ignore.
};

/**
 * A shortcut to produce an empty ADGameFileDescription record. Used to mark
 * the end of a list of these.
 */
#define AD_LISTEND {NULL, 0, NULL, 0}

/**
 * A shortcut to produce a list of ADGameFileDescription records with only one
 * record that contains just a filename with an MD5, and no file size.
 */
#define AD_ENTRY1(f, x) {{ f, 0, x, -1}, AD_LISTEND}

/**
 * A shortcut to produce a list of ADGameFileDescription records with only one
 * record that contains just a filename with an MD5, plus a file size.
 */
#define AD_ENTRY1s(f, x, s) {{ f, 0, x, s}, AD_LISTEND}

enum ADGameFlags {
	ADGF_NO_FLAGS        =  0,
	ADGF_AUTOGENTARGET   = (1 << 20), ///< automatically generate gameid from extra
	ADGF_UNSTABLE        = (1 << 21), ///< flag to designate not yet officially-supported games that are not fit for public testing
	ADGF_TESTING         = (1 << 22), ///< flag to designate not yet officially-supported games that are fit for public testing
	ADGF_PIRATED         = (1 << 23), ///< flag to designate well known pirated versions with cracks
	ADGF_ADDENGLISH      = (1 << 24), ///< always add English as language option
	ADGF_MACRESFORK      = (1 << 25), ///< the md5 for this entry will be calculated from the resource fork
	ADGF_USEEXTRAASTITLE = (1 << 26), ///< Extra field value will be used as main game title, not gameid
	ADGF_DROPLANGUAGE    = (1 << 27), ///< don't add language to gameid
	ADGF_DROPPLATFORM    = (1 << 28), ///< don't add platform to gameid
	ADGF_CD              = (1 << 29), ///< add "-cd" to gameid
	ADGF_DEMO            = (1 << 30)  ///< add "-demo" to gameid
};

struct ADGameDescription {
	const char *gameId;
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

	const char *guiOptions;
};

/**
 * A game installation matching an AD game description
 */
struct ADDetectedGame {
	bool hasUnknownFiles;
	FilePropertiesMap matchedFiles;
	const ADGameDescription *desc;

	ADDetectedGame() : desc(nullptr), hasUnknownFiles(false) {}
	explicit ADDetectedGame(const ADGameDescription *d) : desc(d), hasUnknownFiles(false) {}
};

/** A list of games detected by the AD */
typedef Common::Array<ADDetectedGame> ADDetectedGames;

/**
 * End marker for a table of ADGameDescription structs. Use this to
 * terminate a list to be passed to the AdvancedDetector API.
 */
#define AD_TABLE_END_MARKER	\
	{ NULL, NULL, { { NULL, 0, NULL, 0 } }, Common::UNK_LANG, Common::kPlatformUnknown, ADGF_NO_FLAGS, GUIO0() }

struct ADFileBasedFallback {
	/**
	 * Pointer to an ADGameDescription or subclass thereof which will get
	 * returned if there's a detection match.
	 */
	const ADGameDescription *desc;

	/**
	 * A zero-terminated list of filenames used for matching. All files in
	 * the list must be present to get a detection match.
	 */
	const char *filenames[10];
};


enum ADFlags {
	/**
	 * Store value of extra field in config file, and use it as a hint
	 * on subsequent runs. Could be used when there is no way to autodetect
	 * game (when more than one game sits in same directory), and user picks
	 * up a variant manually.
	 * In addition, this is useful if two variants of a game sharing the same
	 * gameid are contained in a single directory.
	 */
	kADFlagUseExtraAsHint = (1 << 0)
};


/**
 * Map entry for mapping GUIO_GAMEOPTIONS* to their ExtraGuiOption
 * description.
 */
struct ADExtraGuiOptionsMap {
	/**
	 * GUIO_GAMEOPTION* string.
	 */
	const char *guioFlag;

	/**
	 * The associated option.
	 */
	ExtraGuiOption option;
};

#define AD_EXTRA_GUI_OPTIONS_TERMINATOR { 0, { 0, 0, 0, 0 } }

/**
 * A MetaEngine implementation based around the advanced detector code.
 */
class AdvancedMetaEngine : public MetaEngine {
protected:
	/**
	 * Pointer to an array of objects which are either ADGameDescription
	 * or superset structures (i.e. start with an ADGameDescription member.
	 * The list is terminated by an entry with a gameid equal to 0
	 * (see AD_TABLE_END_MARKER).
	 */
	const byte *_gameDescriptors;

	/**
	 * The size of a single entry of the above descs array. Always
	 * must be >= sizeof(ADGameDescription).
	 */
	const uint _descItemSize;

	/**
	 * A list of all gameids (and their corresponding descriptions) supported
	 * by this engine.
	 */
	const PlainGameDescriptor *_gameIds;

	/**
	 * A map containing all the extra game GUI options the engine supports.
	 */
	const ADExtraGuiOptionsMap * const _extraGuiOptions;

	/**
	 * The number of bytes to compute MD5 sum for. The AdvancedDetector
	 * is primarily based on computing and matching MD5 checksums of files.
	 * Since doing that for large files can be slow, it can be restricted
	 * to a subset of all files.
	 * Typically this will be set to something between 5 and 50 kilobytes,
	 * but arbitrary non-zero values are possible. The default is 5000.
	 */
	uint _md5Bytes;

	/**
	 * A bitmask of flags which can be used to configure the behavior
	 * of the AdvancedDetector. Refer to ADFlags for a list of flags
	 * that can be ORed together and passed here.
	 */
	uint32 _flags;

	/**
	 * A list of game GUI options which will be added to each
	 * entry in addition to per-game options. Refer to GameGUIOption
	 * enum for the list.
	 */
	Common::String _guiOptions;

	/**
	 * Maximum depth of directories to look up.
	 * If set to 0, the depth is 1 level
	 */
	uint32 _maxScanDepth;

	/**
	 * Case-insensitive list of directory globs which could be used for
	 * going deeper into the directory structure.
	 * @see String::matchString() method for format description.
	 *
	 * @note Last item must be 0
	 */
	const char * const *_directoryGlobs;

	/**
	 * If true, filenames will be matched against the entire path, relative to
	 * the root detection directory (e.g. "foo/bar.000" for a file at
	 * "<root>/foo/bar.000"). Otherwise, filenames only match the basename
	 * (e.g. "bar.000" for the same file).
	 *
	 * @note _maxScanDepth and _directoryGlobs must still be configured to allow
	 * the detector to find files inside subdirectories.
	 */
	bool _matchFullPaths;

public:
	AdvancedMetaEngine(const void *descs, uint descItemSize, const PlainGameDescriptor *gameIds, const ADExtraGuiOptionsMap *extraGuiOptions = 0);

	/**
	 * Returns list of targets supported by the engine.
	 * Distinguishes engines with single ID
	 */
	PlainGameList getSupportedGames() const override;

	PlainGameDescriptor findGame(const char *gameId) const override;

	DetectedGames detectGames(const Common::FSList &fslist) const override;

	virtual Common::Error createInstance(OSystem *syst, Engine **engine) const;

	virtual const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const;

protected:
	// To be implemented by subclasses
	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const = 0;

	typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	/**
	 * An (optional) generic fallback detect function which is invoked
	 * if the regular MD5 based detection failed to detect anything.
	 */
	virtual ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist) const {
		return ADDetectedGame();
	}

private:
	void initSubSystems(const ADGameDescription *gameDesc) const;

protected:
	/**
	 * Detect games in specified directory.
	 * Parameters language and platform are used to pass on values
	 * specified by the user. This is used to restrict search scope.
	 *
	 * @param allFiles	list of all present files, as computed by composeFileHashMap
	 * @param language	restrict results to specified language
	 * @param platform	restrict results to specified platform
	 * @param extra		restrict results to specified extra string (only if kADFlagUseExtraAsHint is set)
	 * @return	list of ADGameDescription pointers corresponding to matched games
	 */
	virtual ADDetectedGames detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra) const;

	/**
	 * Iterates over all ADFileBasedFallback records inside fileBasedFallback.
	 * This then returns the record (or rather, the ADGameDescription
	 * contained in it) for which all files described by it are present, and
	 * among those the one with the maximal number of matching files.
	 * In case of a tie, the entry coming first in the list is chosen.
	 *
	 * @param allFiles	a map describing all present files
	 * @param fslist	a list of nodes for all present files
	 * @param fileBasedFallback	a list of ADFileBasedFallback records, zero-terminated
	 * @param filesProps	if not 0, return a map of properties for all detected files here
	 */
	ADDetectedGame detectGameFilebased(const FileMap &allFiles, const Common::FSList &fslist, const ADFileBasedFallback *fileBasedFallback) const;

	/**
	 * Compose a hashmap of all files in fslist.
	 * Includes nifty stuff like removing trailing dots and ignoring case.
	 */
	void composeFileHashMap(FileMap &allFiles, const Common::FSList &fslist, int depth, const Common::String &parentName = Common::String()) const;

	/** Get the properties (size and MD5) of this file. */
	bool getFileProperties(const Common::FSNode &parent, const FileMap &allFiles, const ADGameDescription &game, const Common::String fname, FileProperties &fileProps) const;

	/** Convert an AD game description into the shared game description format */
	DetectedGame toDetectedGame(const ADDetectedGame &adGame) const;
};

#endif
