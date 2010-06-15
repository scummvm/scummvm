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

#ifndef SCI_RESOURCE_H
#define SCI_RESOURCE_H

#include "common/fs.h"
#include "common/str.h"

#include "sci/graphics/helpers.h"		// for ViewType
#include "sci/decompressor.h"
#include "sci/sci.h"

namespace Common {
class File;
}

namespace Sci {

enum {
	/** The maximum allowed size for a compressed or decompressed resource */
	SCI_MAX_RESOURCE_SIZE = 0x0400000
};

/** Resource status types */
enum ResourceStatus {
	kResStatusNoMalloc = 0,
	kResStatusAllocated,
	kResStatusEnqueued, /**< In the LRU queue */
	kResStatusLocked /**< Allocated and in use */
};

/** Initialization result types */
enum {
	SCI_ERROR_IO_ERROR = 1,
	SCI_ERROR_INVALID_RESMAP_ENTRY = 2,	/**< Invalid resource.map entry */
	SCI_ERROR_RESMAP_NOT_FOUND = 3,
	SCI_ERROR_NO_RESOURCE_FILES_FOUND = 4,	/**< No resource at all was found */
	SCI_ERROR_UNKNOWN_COMPRESSION = 5,
	SCI_ERROR_DECOMPRESSION_ERROR = 6,	/**< sanity checks failed during decompression */
	SCI_ERROR_RESOURCE_TOO_BIG = 8	/**< Resource size exceeds SCI_MAX_RESOURCE_SIZE */

	/* the first critical error number */
};

enum {
	MAX_OPENED_VOLUMES = 5 ///< Max number of simultaneously opened volumes
};

enum ResourceType {
	kResourceTypeView = 0,
	kResourceTypePic,
	kResourceTypeScript,
	kResourceTypeText,
	kResourceTypeSound,
	kResourceTypeMemory,
	kResourceTypeVocab,
	kResourceTypeFont,
	kResourceTypeCursor,
	kResourceTypePatch,
	kResourceTypeBitmap,
	kResourceTypePalette,
	kResourceTypeCdAudio,
	kResourceTypeAudio,
	kResourceTypeSync,
	kResourceTypeMessage,
	kResourceTypeMap,
	kResourceTypeHeap,
	kResourceTypeAudio36,
	kResourceTypeSync36,
	kResourceTypeUnknown1, // Translation, currently unsupported
	kResourceTypeUnknown2,
	kResourceTypeRobot,
	kResourceTypeInvalid,

	// Mac-only resources, these resource types are self-defined
	// Numbers subject to change
	kResourceTypeMacIconBarPictN = -1, // IBIN resources (icon bar, not selected)
	kResourceTypeMacIconBarPictS = -2, // IBIS resources (icon bar, selected)
	kResourceTypeMacPict = -3          // PICT resources (inventory)
};

const char *getResourceTypeName(ResourceType restype);


class ResourceManager;
class ResourceSource;

class ResourceId {
public:
	ResourceType type;
	uint16 number;
	uint32 tuple; // Only used for audio36 and sync36

	ResourceId() : type(kResourceTypeInvalid), number(0), tuple(0) { }

	ResourceId(ResourceType type_, uint16 number_, uint32 tuple_ = 0)
			: type(type_), number(number_), tuple(tuple_) {
		if (type < kResourceTypeMacPict || type > kResourceTypeInvalid)
			type = kResourceTypeInvalid;
	}

	ResourceId(ResourceType type_, uint16 number_, byte noun, byte verb, byte cond, byte seq)
			: type(type_), number(number_) {
		tuple = (noun << 24) | (verb << 16) | (cond << 8) | seq;

		if ((type < kResourceTypeView) || (type > kResourceTypeInvalid))
			type = kResourceTypeInvalid;
	}

	Common::String toString() {
		char buf[32];

		snprintf(buf, 32, "%s.%i", getResourceTypeName(type), number);
		Common::String retStr = buf;

		if (tuple != 0) {
			snprintf(buf, 32, "(%i, %i, %i, %i)", tuple >> 24, (tuple >> 16) & 0xff, (tuple >> 8) & 0xff, tuple & 0xff);
			retStr += buf;
		}

		return retStr;
	}
};

struct ResourceIdHash : public Common::UnaryFunction<ResourceId, uint> {
	uint operator()(ResourceId val) const { return ((uint)((val.type << 16) | val.number)) ^ val.tuple; }
};

struct ResourceIdEqualTo : public Common::BinaryFunction<ResourceId, ResourceId, bool> {
	bool operator()(const ResourceId &x, const ResourceId &y) const { return (x.type == y.type) && (x.number == y.number) && (x.tuple == y.tuple); }
};

struct ResourceIdLess : public Common::BinaryFunction<ResourceId, ResourceId, bool> {
	bool operator()(const ResourceId &x, const ResourceId &y) const {
		return (x.type < y.type) || ((x.type == y.type) && (x.number < y.number))
			    || ((x.type == y.type) && (x.number == y.number) && (x.tuple < y.tuple));
	}
};

/** Class for storing resources in memory */
class Resource {
	friend class ResourceManager;
public:
	Resource();
	~Resource();
	void unalloc();

// NOTE : Currently all member data has the same name and public visibility
// to let the rest of the engine compile without changes
public:
	ResourceId _id;
	byte *data;
	uint32 size;
	byte *_header;
	uint32 _headerSize;

	/**
	 * Write the resource to the specified stream.
	 * This method is used only by the "dump" debugger command.
	 */
	void writeToStream(Common::WriteStream *stream) const;
	uint32 getAudioCompressionType();

protected:
	int32 _fileOffset; /**< Offset in file */
	ResourceStatus _status;
	uint16 _lockers; /**< Number of places where this resource was locked */
	ResourceSource *_source;
};

typedef Common::HashMap<ResourceId, Resource *, ResourceIdHash, ResourceIdEqualTo> ResourceMap;

class ResourceManager {
	// FIXME: These 'friend' declarations are meant to be a temporary hack to
	// ease transition to the ResourceSource class system.
	friend class ResourceSource;
	friend class DirectoryResourceSource;
	friend class ExtMapResourceSource;
	friend class IntMapResourceSource;
	friend class ExtAudioMapResourceSource;
	friend class MacResourceForkResourceSource;

public:
	enum ResVersion {
		kResVersionUnknown,
		kResVersionSci0Sci1Early,
		kResVersionSci1Middle,
		kResVersionSci1Late,
		kResVersionSci11,
		kResVersionSci11Mac,
		kResVersionSci32
	};

	/**
	 * Creates a new SCI resource manager.
	 */
	ResourceManager();
	ResourceManager(const Common::FSList &fslist);
	~ResourceManager();

	/**
	 * Looks up a resource's data.
	 * @param id: The resource type to look for
	 * @param lock: non-zero iff the resource should be locked
	 * @return (Resource *): The resource, or NULL if it doesn't exist
	 * @note Locked resources are guaranteed not to have their contents freed until
	 *       they are unlocked explicitly (by unlockResource).
	 */
	Resource *findResource(ResourceId id, bool lock);

	/* Unlocks a previously locked resource
	**             (Resource *) res: The resource to free
	** Returns   : ()
	*/
	void unlockResource(Resource *res);

	/* Tests whether a resource exists
	**             (ResourceId) id: Id of the resource to check
	** Returns   : (Resource *) non-NULL if the resource exists, NULL otherwise
	** This function may often be much faster than finding the resource
	** and should be preferred for simple tests.
	** The resource object returned is, indeed, the resource in question, but
	** it should be used with care, as it may be unallocated.
	** Use scir_find_resource() if you want to use the data contained in the resource.
	*/
	Resource *testResource(ResourceId id);

	/**
	 * Returns a list of all resources of the specified type.
	 * @param type: The resource type to look for
	 * @param mapNumber: For audio36 and sync36, limit search to this map
	 * @return: The resource list
	 */
	Common::List<ResourceId> *listResources(ResourceType type, int mapNumber = -1);

	void setAudioLanguage(int language);
	int getAudioLanguage() const;
	bool isVGA() const { return (_viewType == kViewVga) || (_viewType == kViewVga11); }
	bool isAmiga32color() const { return _viewType == kViewAmiga; }
	bool isSci11Mac() const { return _volVersion == kResVersionSci11Mac; }
	ViewType getViewType() const { return _viewType; }
	const char *getMapVersionDesc() const { return versionDescription(_mapVersion); }
	const char *getVolVersionDesc() const { return versionDescription(_volVersion); }

	/**
	 * Adds the appropriate GM patch from the Sierra MIDI utility as 4.pat, without
	 * requiring the user to rename the file to 4.pat. Thus, the original Sierra
	 * archive can be extracted in the extras directory, and the GM patches can be
	 * applied per game, if applicable
	 */
	void addNewGMPatch(const Common::String &gameId);

	bool detectHires();
	// Detects, if standard font of current game includes extended characters (>0x80)
	bool detectFontExtended();

	/**
	 * Finds the internal Sierra ID of the current game from script 0
	 */
	Common::String findSierraGameId();

	/**
	 * Finds the location of the game object from script 0
	 * @param addSci11ScriptOffset: Adjust the return value for SCI1.1 and newer
	 *        games. Needs to be false when the heap is accessed directly inside
	 *        findSierraGameId().
	 */
	reg_t findGameObject(bool addSci11ScriptOffset = true);

protected:
	// Maximum number of bytes to allow being allocated for resources
	// Note: maxMemory will not be interpreted as a hard limit, only as a restriction
	// for resources which are not explicitly locked. However, a warning will be
	// issued whenever this limit is exceeded.
	enum {
		MAX_MEMORY = 256 * 1024	// 256KB
	};

	ViewType _viewType; // Used to determine if the game has EGA or VGA graphics
	Common::List<ResourceSource *> _sources;
	int _memoryLocked;	///< Amount of resource bytes in locked memory
	int _memoryLRU;		///< Amount of resource bytes under LRU control
	Common::List<Resource *> _LRU; ///< Last Resource Used list
	ResourceMap _resMap;
	Common::List<Common::File *> _volumeFiles; ///< list of opened volume files
	ResourceSource *_audioMapSCI1; ///< Currently loaded audio map for SCI1
	ResVersion _volVersion; ///< resource.0xx version
	ResVersion _mapVersion; ///< resource.map version

	/**
	 * Initializes the resource manager
	 */
	void init();

	/**
	 * Add a path to the resource manager's list of sources.
	 * @return a pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addPatchDir(const Common::String &path);

	ResourceSource *findVolume(ResourceSource *map, int volume_nr);

	/**
	 * Adds a source to the resource manager's list of sources.
	 * @param source	The new source to add
	 * @return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addSource(ResourceSource *source, int number);

	ResourceSource *addSource(ResourceSource *source, const Common::FSNode *resFile, int number);

	/**
	 * Add an external (i.e., separate file) map resource to the resource manager's list of sources.
	 * @param file_name	 The name of the volume to add
	 * @param volume_nr  The volume number the map starts at, 0 for <SCI2.1
	 * @return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addExternalMap(const Common::String &filename, int volume_nr = 0);

	ResourceSource *addExternalMap(const Common::FSNode *mapFile, int volume_nr = 0);

	/**
	 * Add an internal (i.e., resource) map to the resource manager's list of sources.
	 * @param name		The name of the resource to add
	 * @param resNr		The map resource number
	 * @return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addInternalMap(const Common::String &name, int resNr);

	/**
	 * Checks, if an audio volume got compressed by our tool. If that's the case, it will set audioCompressionType
	 *  and read in the offset translation table for later usage.
	 */
	void checkIfAudioVolumeIsCompressed(ResourceSource *source);

	/**
	 * Scans newly registered resource sources for resources, earliest addition first.
	 * @param detected_version: Pointer to the detected version number,
	 *					 used during startup. May be NULL.
	 * @return One of SCI_ERROR_*.
	 */
	void scanNewSources();
	int addAppropriateSources();
	int addAppropriateSources(const Common::FSList &fslist);
	int addInternalSources();
	void freeResourceSources();

	/**
	 * Returns a string describing a ResVersion
	 * @param version: The resource version
	 * @return: The description of version
	 */
	const char *versionDescription(ResVersion version) const;

	Common::SeekableReadStream *getVolumeFile(ResourceSource *source);
	void loadResource(Resource *res);
	bool loadPatch(Resource *res, Common::SeekableReadStream *file);
	bool loadFromPatchFile(Resource *res);
	bool loadFromWaveFile(Resource *res, Common::SeekableReadStream *file);
	bool loadFromAudioVolumeSCI1(Resource *res, Common::SeekableReadStream *file);
	bool loadFromAudioVolumeSCI11(Resource *res, Common::SeekableReadStream *file);
	void freeOldResources();
	int decompress(Resource *res, Common::SeekableReadStream *file);
	int readResourceInfo(Resource *res, Common::SeekableReadStream *file, uint32&szPacked, ResourceCompression &compression);
	void addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size = 0);
	void removeAudioResource(ResourceId resId);

	/**--- Resource map decoding functions ---*/
	ResVersion detectMapVersion();
	ResVersion detectVolVersion();

	/**
	 * Reads the SCI0 resource.map file from a local directory.
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI0(ResourceSource *map);

	/**
	 * Reads the SCI1 resource.map file from a local directory.
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readResourceMapSCI1(ResourceSource *map);
	
	/**
	 * Reads the SCI1.1+ resource file from a Mac resource fork.
	 * @param source The source
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readMacResourceFork(ResourceSource *source);

	/**
	 * Reads SCI1.1 audio map resources
	 * @param map The map
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readAudioMapSCI11(ResourceSource *map);

	/**
	 * Reads SCI1 audio map files
	 * @param map The map
	 * @param unload Unload the map instead of loading it
	 * @return 0 on success, an SCI_ERROR_* code otherwise
	 */
	int readAudioMapSCI1(ResourceSource *map, bool unload = false);

	/**--- Patch management functions ---*/

	/**
	 * Reads patch files from a local directory.
	 */
	void readResourcePatches(ResourceSource *source);
	void readResourcePatchesBase36(ResourceSource *source);
	void processPatch(ResourceSource *source, ResourceType resourceType, uint16 resourceNr, uint32 tuple = 0);

	/**
	 * Process wave files as patches for Audio resources
	 */
	void readWaveAudioPatches();
	void processWavePatch(ResourceId resourceId, Common::String name);

 	/**
	 * Applies to all versions before 0.000.395 (i.e. KQ4 old, XMAS 1988 and LSL2).
	 * Old SCI versions used two word header for script blocks (first word equal
	 * to 0x82, meaning of the second one unknown). New SCI versions used one
	 * word header.
	 * Also, old SCI versions assign 120 degrees to left & right, and 60 to up
	 * and down. Later versions use an even 90 degree distribution.
	 */
	bool hasOldScriptHeader();

	void printLRU();
	void addToLRU(Resource *res);
	void removeFromLRU(Resource *res);

	ResourceCompression getViewCompression();
	ViewType detectViewType();
	bool hasSci0Voc999();
	bool hasSci1Voc900();
	void detectSciVersion();
};

class SoundResource {
public:
	struct Channel {
		byte number;
		byte poly;
		uint16 prio;
		uint16 size;
		byte *data;
		long time;
		byte prev;
	};

	struct Track {
		byte type;
		byte channelCount;
		Channel *channels;
		int16 digitalChannelNr;
		uint16 digitalSampleRate;
		uint16 digitalSampleSize;
		uint16 digitalSampleStart;
		uint16 digitalSampleEnd;
	};
public:
	SoundResource(uint32 resNumber, ResourceManager *resMan, SciVersion soundVersion);
	~SoundResource();
#if 0
	Track *getTrackByNumber(uint16 number);
#endif
	Track *getTrackByType(byte type);
	Track *getDigitalTrack();
	int getChannelFilterMask(int hardwareMask, bool wantsRhythm);
	byte getInitialVoiceCount(byte channel);
	bool isChannelUsed(byte channel) const { return _channelsUsed & (1 << channel); }

private:
	SciVersion _soundVersion;
	int _trackCount;
	Track *_tracks;
	Resource *_innerResource;
	ResourceManager *_resMan;
	uint16 _channelsUsed;

	void setChannelUsed(byte channel) { _channelsUsed |= (1 << channel); }
};

} // End of namespace Sci

#endif // SCI_RESOURCE_H
