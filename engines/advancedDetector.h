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

#ifndef ENGINES_ADVANCED_DETECTOR_H
#define ENGINES_ADVANCED_DETECTOR_H

#include "engines/metaengine.h"
#include "engines/engine.h"

#include "common/hash-str.h"

#include "common/gui_options.h" // Keep it here, so detection tables can refer to them

namespace Common {
class Error;
class FSList;
}
/**
 * @defgroup engines_advdetector Advanced Detector
 * @ingroup engines
 *
 * @brief The Advanced Detector (AD) provides a standard framework for filename and MD5-based game detection.
 *
 * @{
 */

/**
 * A record describing a file to be matched for detecting a specific game
 * variant. A list of such records is used inside every ADGameDescription to
 * enable detection.
 */
struct ADGameFileDescription {
	const char *fileName; ///< Name of the described file.
	uint16 fileType;      ///< Optional. Not used during detection, only by engines.
	const char *md5;      ///< MD5 of (the beginning of) the described file. Optional. Set to NULL to ignore.
	int64 fileSize;       ///< Size of the described file. Set to -1 to ignore.
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

/**
 * A shortcut to produce a list of ADGameFileDescription records with only two
 * records that contain just a filename with an MD5, plus a file size.
 */
#define AD_ENTRY2s(f1, x1, s1, f2, x2, s2) {{f1, 0, x1, s1}, {f2, 0, x2, s2}, AD_LISTEND}

/**
 * A shortcut to produce a list of ADGameFileDescription records with only three
 * records that contain just a filename with an MD5, plus a file size.
 */
#define AD_ENTRY3s(f1, x1, s1, f2, x2, s2, f3, x3, s3) {{f1, 0, x1, s1}, {f2, 0, x2, s2}, {f3, 0, x3, s3}, AD_LISTEND}
#define AD_ENTRY4s(f1, x1, s1, f2, x2, s2, f3, x3, s3, f4, x4, s4) {{f1, 0, x1, s1}, {f2, 0, x2, s2}, {f3, 0, x3, s3}, {f4, 0, x4, s4}, AD_LISTEND}
#define AD_ENTRY5s(f1, x1, s1, f2, x2, s2, f3, x3, s3, f4, x4, s4, f5, x5, s5) {{f1, 0, x1, s1}, {f2, 0, x2, s2}, {f3, 0, x3, s3}, {f4, 0, x4, s4}, {f5, 0, x5, s5}, AD_LISTEND}
#define AD_ENTRY6s(f1, x1, s1, f2, x2, s2, f3, x3, s3, f4, x4, s4, f5, x5, s5, f6, x6, s6) {{f1, 0, x1, s1}, {f2, 0, x2, s2}, {f3, 0, x3, s3}, {f4, 0, x4, s4}, {f5, 0, x5, s5}, {f6, 0, x6, s6}, AD_LISTEND}

/**
 * Flags used in the game description.
 *
 * Note that the lowest 16 bits are currently reserved for use by the client code.
 */
enum ADGameFlags : uint {
	ADGF_NO_FLAGS        =  0u,        ///< No flags.
	ADGF_TAILMD5         = (1u << 16), ///< Calculate the MD5 for this entry from the end of the file.
	ADGF_AUTOGENTARGET   = (1u << 17), ///< Automatically generate gameid from @ref ADGameDescription::extra.
	ADGF_UNSTABLE        = (1u << 18), ///< Flag to designate not yet officially supported games that are not fit for public testing.
	ADGF_TESTING         = (1u << 19), ///< Flag to designate not yet officially supported games that are fit for public testing.
	ADGF_PIRATED         = (1u << 20), ///< Flag to designate well-known pirated versions with cracks.
	ADGF_UNSUPPORTED     = (1u << 21), /*!< Flag to mark certain versions (like badly protected full games as demos) not to be run for various reasons.
	                                        A custom message can be provided in the @ref ADGameDescription::extra field. */
	ADGF_WARNING         = (1u << 22), /*!< Flag to mark certain versions to show confirmation warning before proceeding.
	                                        A custom message should be provided in the @ref ADGameDescription::extra field. */
	ADGF_ADDENGLISH      = (1u << 23), ///< Always add English as a language option.
	ADGF_MACRESFORK      = (1u << 24), ///< Calculate the MD5 for this entry from the resource fork.
	ADGF_USEEXTRAASTITLE = (1u << 25), ///< Use @ref ADGameDescription::extra as the main game title, not gameid.
	ADGF_DROPLANGUAGE    = (1u << 26), ///< Do not add language to gameid.
	ADGF_DROPPLATFORM    = (1u << 27), ///< Do not add platform to gameid.
	ADGF_CD              = (1u << 28), ///< Add "-cd" to gameid.
	ADGF_DVD             = (1u << 29), ///< Add "-dvd" to gameid.
	ADGF_DEMO            = (1u << 30), ///< Add "-demo" to gameid.
	ADGF_REMASTERED      = (1u << 31)  ///< Add "-remastered' to gameid.
};

/**
 * Data table describing a game variant.
 */
struct ADGameDescription {
	/**
	 * The gameid that identifies the game.
	 *
	 * Mainly used for taking the game description from the @ref PlainGameDescriptor table.
	 */
	const char *gameId;

	/**
	 * Field that is used to distinguish between different variants of a game.
	 *
	 * The content of this field is inserted in the generated description for the config file game entry.
	 * If the @ref kADFlagUseExtraAsHint ADFlag is set, the contents of this field are stored
	 * in the config file, and are used to additionally distinguish between game variants.
	 * Also, if the ADGF_USEEXTRAASTITLE game flag is set, the contents of this field
	 * will be used as a description instead the one extracted from the @ref PlainGameDescriptor table.
	 */
	const char *extra;

	/**
	 * A list of individual file entries used for detection.
	 *
	 * 13 files (last is zero-terminated) is the maximum number of files currently used in ScummVM.
	 * A hardcoded number is used here due to a C++ limitation for defining const arrays.
	 */
	ADGameFileDescription filesDescriptions[14];

	/**
	 * Language of the game variant.
	 */
	Common::Language language;

	/**
	 * Platform of the game variant.
	 */
	Common::Platform platform;

	/**
	 * A bitmask of extra flags.
	 *
	 * The top 16 bits are reserved for generic flags defined in @ref ADGameFlags.
	 * This leaves 16 bits to be used by the client code.
	 */
	uint32 flags;

	/**
	 * Game features that are user-controllable.
	 *
	 * This list reflects which features of the GUI should be turned on or off in order to minimize user confusion.
	 * For instance, there is no point in changing game language in single-language games
	 * or have MIDI controls in a game that only supports digital music.
	 */
	const char *guiOptions;
};

/**
 * struct which saved extra information for detected games
 */
struct ADDetectedGameExtraInfo {
	Common::String gameName;			/*!< Extra info which saved game name */
	Common::String targetID;			/*!< targetID which will be used on preferred target id */
};

/**
 * A game installation matching an AD game description.
 */
struct ADDetectedGame {
	bool hasUnknownFiles;           /*!< Whether the game has unknown files. */
	FilePropertiesMap matchedFiles; /*!< List of the files that were used to match the game. */
	const ADGameDescription *desc;  /*!< Human-readable game title. */

	ADDetectedGame() : desc(nullptr), hasUnknownFiles(false) {}
	/**
	 * Construct an ADDetectedGame object.
	 */
	explicit ADDetectedGame(const ADGameDescription *d) : desc(d), hasUnknownFiles(false) {}
};

/** A list of games detected by the AD. */
typedef Common::Array<ADDetectedGame> ADDetectedGames;

/**
 * End marker for a table of @ref ADGameDescription structures. Use this to
 * terminate a list to be passed to the Advanced Detector API.
 */
#define AD_TABLE_END_MARKER	\
	{ NULL, NULL, { { NULL, 0, NULL, 0 } }, Common::UNK_LANG, Common::kPlatformUnknown, ADGF_NO_FLAGS, GUIO0() }

struct ADFileBasedFallback {
	/**
	 * Pointer to an @ref ADGameDescription or its subclass that will be
	 * returned if there is a detection match.
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
	 * Store the value of the @ref ADGameDescription::extra field
	 * in the config file, and use it as a hint on subsequent runs.
	 *
	 * This can be used when there is no way to autodetect the game
	 * (when more than one game are located in same directory), and the user picks
	 * up a variant manually.
	 * In addition, this is useful if two variants of a game sharing the same
	 * gameid are contained in a single directory.
	 */
	kADFlagUseExtraAsHint = (1 << 0),

	/**
	 * If set, filenames will be matched against the entire path, relative to
	 * the root detection directory.
	 *
	 * For example: "foo/bar.000" for a file at "<root>/foo/bar.000").
	 * Otherwise, filenames only match the base name (e.g. "bar.000" for the same file).
	 *
	 * @note @c _maxScanDepth must still be configured to allow
	 * the detector to find files inside subdirectories. @c _directoryGlobs are
	 * extracted from the entries.
	 */
	 kADFlagMatchFullPaths = (1 << 1),

	/**
	 * If set, the engine's fallback detection results are used instead of the
	 * partial matches found in the detection table.
	 *
	 * An engine sets this if its fallback detection produces good results that
	 * should always be used. If fallback detection fails, then partial matches
	 * are still used.
	 */
	 kADFlagPreferFallbackDetection = (1 << 2),

	 /**
	  * Indicates engine's ability to play a variant of a game with unknown files.
	  * This will leave the detection entries with partial matches in the list
	  * of detected games.
	  */
	kADFlagCanPlayUnknownVariants = (1 << 3),
};


/**
 * Map entry for mapping GUIO_GAMEOPTIONS* to their @ref ExtraGuiOption
 * description.
 */
struct ADExtraGuiOptionsMap {
	const char *guioFlag;  /*!< GUIO_GAMEOPTION* string. */
	ExtraGuiOption option; /*!< The associated option. */
};

#define AD_EXTRA_GUI_OPTIONS_TERMINATOR { 0, { 0, 0, 0, 0, 0, 0 } }

/**
 * A @ref MetaEngineDetection implementation based on the Advanced Detector code.
 */
class AdvancedMetaEngineDetection : public MetaEngineDetection {
protected:
	/**
	 * Pointer to an array of objects which are either ADGameDescription
	 * or superset structures (i.e. start with an ADGameDescription member).
	 *
	 * The list is terminated by an entry with a gameid equal to 0
	 * (see @ref AD_TABLE_END_MARKER).
	 */
	const byte *_gameDescriptors;

	/**
	 * The size of a single entry of the above descriptions array.
	 *
	 * Always must be >= sizeof(ADGameDescription).
	 */
	const uint _descItemSize;

	/**
	 * A list of all gameids (and their corresponding descriptions) supported
	 * by this engine.
	 */
	const PlainGameDescriptor *_gameIds;

	/**
	 * The number of bytes to compute the MD5 checksum for.
	 *
	 * The Advanced Detector is primarily based on computing and matching
	 * MD5 checksums of files. Since doing that for large files can be slow,
	 * it can be restricted to a subset of all files.
	 * Typically this will be set to something between 5 and 50 kilobytes,
	 * but arbitrary non-zero values are possible. The default is 5000.
	 */
	uint _md5Bytes;

	/**
	 * A bitmask of flags which can be used to configure the behavior
	 * of the Advanced Detector.
	 *
	 * See @ref ADFlags for a list of flags that can be ORed together and passed here.
	 */
	uint32 _flags;

	/**
	 * A list of game GUI options which will be added to each
	 * entry in addition to per-game options.
	 *
	 * Refer to @ref GameGUIOption enum for the list.
	 */
	Common::String _guiOptions;

	/**
	 * Maximum depth of directories to look up.
	 *
	 * If set to 0, the depth is 1 level.
	 */
	uint32 _maxScanDepth;

	/**
	 * Case-insensitive list of directory globs that can be used for
	 * going deeper into the directory structure.
	 *
	 * See @ref String::matchString() method for format description.
	 *
	 * @note The last item must be 0.
	 */
	const char * const *_directoryGlobs;

	/**
	 * If ADGF_AUTOGENTARGET is used, then this specifies the max length
	 * of the autogenerated name.
	 * The default is 15.
	 */
	int _maxAutogenLength;

	/**
	 * If kADFlagMatchFullPaths is set, then use this value for specifying
	 * depth for addSubDirectoryMatching() call
	 * The default is 5;
	 */
	 int _fullPathGlobsDepth;

public:
	/**
	 * Initialize game detection using AdvancedMetaEngineDetection.
	 */
	AdvancedMetaEngineDetection(const void *descs, uint descItemSize, const PlainGameDescriptor *gameIds);

	/**
	 * Return a list of targets supported by the engine.
	 *
	 * Distinguishes engines with a single ID.
	 */
	PlainGameList getSupportedGames() const override;

	/** Query the engine for a @ref PlainGameDescriptor for the specified gameid, if any. */
	PlainGameDescriptor findGame(const char *gameId) const override;

	/**
	 * Run the engine's game detector on the given list of files, and return a
	 * (possibly empty) list of games supported by the engine that were
	 * found among the given files.
	 */
	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) override;

	/**
	 * A generic createInstance.
	 *
	 * For instantiating engine objects, this method is called first,
	 * and then the subclass implemented createInstance is called from within.
	 */
	Common::Error createInstance(OSystem *syst, Engine **engine);

	static Common::StringArray getPathsFromEntry(const ADGameDescription *g);

	uint getMD5Bytes() const override final { return _md5Bytes; }

protected:
	/**
	 * A hashmap of files and their MD5 checksums.
	 */
	typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	/**
	 * An (optional) generic fallback detection function that is invoked
	 * if the regular MD5-based detection failed to detect anything.
	 */
	virtual ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra = nullptr) const {
		return ADDetectedGame();
	}

private:
	void initSubSystems(const ADGameDescription *gameDesc) const;
	void preprocessDescriptions();
	bool isEntryGrayListed(const ADGameDescription *g) const;
	void detectClashes() const;

private:
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _grayListMap;
	Common::HashMap<Common::String, bool, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _globsMap;
	bool _hashMapsInited;

protected:
	/**
	 * Detect games in the specified directory.
	 *
	 * Parameters @p language and @p platform are used to pass the values
	 * specified by the user. This is used to restrict search scope.
	 *
	 * @param parent		  Parent node of this file node.
	 * @param allFiles		  List of all present files, as computed by the @ref composeFileHashMap.
	 * @param language		  Restrict results to the specified language.
	 * @param platform		  Restrict results to the specified platform.
	 * @param extra			  Restrict results to the specified @c extra string (only if @ref kADFlagUseExtraAsHint is set).
	 * @param skipADFlags	  Specify bitmask of ADGF flags to be ignored (for mass add).
	 * @param skipIncomplete  Ignore results with incomplete file/md5/size matches (for mass add).
	 *
	 * @return A list of @ref ADGameDescription pointers corresponding to the matched games.
	 */
	virtual ADDetectedGames detectGame(const Common::FSNode &parent, const FileMap &allFiles, Common::Language language, Common::Platform platform, const Common::String &extra, uint32 skipADFlags = 0, bool skipIncomplete = false);

	/**
	 * Iterate over all @ref ADFileBasedFallback records inside @p fileBasedFallback
	 * and return the record (or rather, the ADGameDescription
	 * contained in it) for which all files described by it are present, and
	 * among those the one with the maximal number of matching files.
	 *
	 * In case of a tie, the entry coming first in the list is chosen.
	 *
	 * @param allFiles           Map describing all present files
	 * @param fileBasedFallback  List of ADFileBasedFallback records, zero-terminated.
	 */
	ADDetectedGame detectGameFilebased(const FileMap &allFiles, const ADFileBasedFallback *fileBasedFallback) const;

	/**
	 * Compose a hashmap of all files in @p fslist.
	 *
	 * Removes trailing dots and ignores case in the process.
	 */
	void composeFileHashMap(FileMap &allFiles, const Common::FSList &fslist, int depth, const Common::String &parentName = Common::String()) const;

	/** Get the properties (size and MD5) of this file. */
	bool getFileProperties(const FileMap &allFiles, MD5Properties md5prop, const Common::String &fname, FileProperties &fileProps) const;

	/** Convert an AD game description into the shared game description format. */
	virtual DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo = nullptr) const;

	/** Check for pirated games in the given detected games */
	bool cleanupPirated(ADDetectedGames &matched) const;

	friend class FileMapArchive;
};

/**
 * A MetaEngine implementation of AdvancedMetaEngine.
 */
class AdvancedMetaEngine : public MetaEngine {
public:
	/**
	 * Base createInstance for AdvancedMetaEngine.
	 *
	 * The AdvancedMetaEngine provides a default createInstance that is called first,
	 * so this should be invoked first.
	 * By the time this is called, it is assumed that there is only one
	 * plugin engine loaded in memory.
	 */
	Common::Error createInstance(OSystem *syst, Engine **engine) override;

	/**
	 * A createInstance implementation for subclasses. To be called after the base
	 * createInstance function above is called.
	 */
	virtual Common::Error createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const = 0;

	/**
	 * Return the name of the engine plugin based on the engineID.
	 *
	 * The engineID must match the one from MetaEngine.
	 *
	 * @see MetaEngine::getName().
	 */
	const char *getName() const override = 0;

public:
	/**
	 * A hashmap of files and their MD5 checksums.
	 */
	typedef Common::HashMap<Common::String, Common::FSNode, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileMap;

	/**
	 * An (optional) generic fallback detection function that is invoked
	 * if the regular MD5-based detection failed to detect anything.

	 * @note This is only meant to be used if fallback detection heavily depends on engine resources.
	 *
	 * To use this, implement the intended fallbackDetectExtern inside the relevant MetaEngine class.
	 * Then, override the method "fallbackDetect" inside your MetaEngine class.
	 * Finally, provide a "hook" to fetch the relevant MetaEngine class and then use the original detection
	 * method.
	 *
	 * An example of how this is implemented can be found in the Wintermute Engine.
	 */
	virtual ADDetectedGame fallbackDetectExtern(uint md5Bytes, const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra = nullptr) const {
		return ADDetectedGame();
	}

	/**
	 * Get the properties (size and MD5) of this file.
	 *
	 * Based on @ref MetaEngine::getFileProperties.
	 */
	bool getFilePropertiesExtern(uint md5Bytes, const FileMap &allFiles, MD5Properties md5prop, const Common::String &fname, FileProperties &fileProps) const;

protected:
	/**
	 * Return a list of extra GUI options for the specified target.
	 *
	 * If no target is specified, all of the available custom GUI options are
	 * returned for the plugin (used to set default values).
	 *
	 * Currently, this only supports options with checkboxes.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @param target    Name of a config manager target.
	 *
	 * @return A list of extra GUI options for an engine plugin and target.
	 */
	const ExtraGuiOptions getExtraGuiOptions(const Common::String &target) const override final;

	/**
	 * Returns a map containing all the extra game GUI options the engine supports.
	 */
	virtual const ADExtraGuiOptionsMap *getAdvancedExtraGuiOptions() const { return nullptr; }

	/**
	 * Returns the set of features that need to be enabled for the
	 * extended save format to work
	 */
	bool checkExtendedSaves(MetaEngineFeature f) const;
};

/**
 * Singleton Cache Storage for Computed MD5s
 */
class MD5CacheManager : public Common::Singleton<MD5CacheManager> {
public:
	void setMD5(Common::String fname, Common::String md5) {
		md5HashMap.setVal(fname, md5);
	}

	Common::String getMD5(Common::String fname) {
		return md5HashMap.getVal(fname);
	}

	void setSize(Common::String fname, int64 size) {
		sizeHashMap.setVal(fname, size);
	}

	int64 getSize(Common::String fname) {
		return sizeHashMap.getVal(fname);
	}

	bool contains(Common::String fname) {
		return (md5HashMap.contains(fname) && sizeHashMap.contains(fname));
	}

	MD5CacheManager() {
		clear();
	}

	void clear() {
		md5HashMap.clear(true);
		sizeHashMap.clear(true);
	}

private:
	friend class Common::Singleton<MD5CacheManager>;

	typedef Common::HashMap<Common::String, Common::String, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> FileHashMap;
	typedef Common::HashMap<Common::String, int64, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SizeHashMap;
	FileHashMap md5HashMap;
	SizeHashMap sizeHashMap;
};

/** Convenience shortcut for accessing the MD5CacheManager. */
#define MD5Man MD5CacheManager::instance()
/** @} */
#endif
