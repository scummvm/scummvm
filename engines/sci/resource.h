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

#ifndef SCI_SCICORE_RESOURCE_H
#define SCI_SCICORE_RESOURCE_H

#include "common/str.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/archive.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"			// for SoundHandle

#include "gfx/gfx_resource.h"		// for ViewType

#include "sci/decompressor.h"
#include "sci/sci.h"

namespace Common {
class ReadStream;
}

namespace Sci {

/** The maximum allowed size for a compressed or decompressed resource */
#define SCI_MAX_RESOURCE_SIZE 0x0400000

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
	MAX_OPENED_VOLUMES = 5 // Max number of simultaneously opened volumes
};

enum ResSourceType {
	kSourceDirectory = 0,
	kSourcePatch,
	kSourceVolume,
	kSourceExtMap,
	kSourceIntMap,
	kSourceAudioVolume,
	kSourceExtAudioMap
};

enum {
	SCI0_RESMAP_ENTRIES_SIZE = 6,
	SCI1_RESMAP_ENTRIES_SIZE = 6,
	SCI11_RESMAP_ENTRIES_SIZE = 5
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
	kResourceTypeInvalid
};

const char *getResourceTypeName(ResourceType restype);

#define sci0_last_resource kResourceTypePatch
#define sci1_last_resource kResourceTypeHeap
/* Used for autodetection */


/** resource type for SCI1 resource.map file */
struct resource_index_t {
	uint16 wOffset;
	uint16 wSize;
};

struct ResourceSource {
	ResSourceType source_type;
	bool scanned;
	Common::String location_name;	// FIXME: Replace by FSNode ?
	const Common::FSNode *resourceFile;
	int volume_number;
	ResourceSource *associated_map;
};

class ResourceManager;

class ResourceId {
public:
	ResourceType type;
	uint16 number;
	uint32 tuple; // Only used for audio36 and sync36

	ResourceId() : type(kResourceTypeInvalid), number(0), tuple(0) { };

	ResourceId(ResourceType type_, uint16 number_, uint32 tuple_ = 0) : type(type_), number(number_), tuple(tuple_) {
		if ((type < kResourceTypeView) || (type > kResourceTypeInvalid))
			type = kResourceTypeInvalid;
	}

	ResourceId(ResourceType type_, uint16 number_, byte noun, byte verb, byte cond, byte seq) : type(type_), number(number_) {
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
	byte *data;
	ResourceId id;
	uint32 size;
	byte *header;
	uint32 headerSize;
protected:
	uint32 file_offset; /**< Offset in file */
	ResourceStatus status;
	uint16 lockers; /**< Number of places where this resource was locked */
	ResourceSource *source;
};

typedef Common::HashMap<ResourceId, Resource *, ResourceIdHash, ResourceIdEqualTo> ResourceMap;

class ResourceManager {
public:
	enum ResVersion {
		kResVersionUnknown,
		kResVersionSci0Sci1Early,
		kResVersionSci1Middle,
		kResVersionSci1Late,
		kResVersionSci11,
		kResVersionSci32
	};

	bool isVGA() const { return (_viewType == kViewVga) || (_viewType == kViewVga11); }

	ViewType getViewType() const { return _viewType; }

	/**
	 * Returns the SCI version as detected by the resource manager
	 * @return SCI version
	 */
	SciVersion sciVersion() const { return _sciVersion; }

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
	** Returns   : (void)
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
	int _memoryLocked;	//!< Amount of resource bytes in locked memory
	int _memoryLRU;		//!< Amount of resource bytes under LRU control
	Common::List<Resource *> _LRU; //!< Last Resource Used list
	ResourceMap _resMap;
	Common::List<Common::File *> _volumeFiles; //!< list of opened volume files
	ResourceSource *_audioMapSCI1; //!< Currently loaded audio map for SCI1
	ResVersion _volVersion; //!< RESOURCE.0xx version
	ResVersion _mapVersion; //!< RESOURCE.MAP version
	SciVersion _sciVersion; //!< Detected SCI version */

	/**
	 * Initializes the resource manager
	 */
	void init();

	/**
	 * Add a path to the resource manager's list of sources.
	 * @return a pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addPatchDir(const char *path);

	ResourceSource *getVolume(ResourceSource *map, int volume_nr);

	/**
	 * Adds a source to the resource manager's list of sources.
	 * @param map		The map associated with this source
	 * @param type		The source type
	 * @param filename	The name of the source to add
	 * @return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addSource(ResourceSource *map, ResSourceType type, const char *filename,
	                          int number);

	ResourceSource *addSource(ResourceSource *map, ResSourceType type, 
								const Common::FSNode *resFile, int number);

	/**
	 * Add an external (i.e., separate file) map resource to the resource manager's list of sources.
	 * @param file_name	 The name of the volume to add
	 * @return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addExternalMap(const char *file_name);

	ResourceSource *addExternalMap(const Common::FSNode *mapFile);

	/**
	 * Add an internal (i.e., resource) map to the resource manager's list of sources.
	 * @param name		The name of the resource to add
	 * @param resNr		The map resource number
	 * @return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addInternalMap(const char *name, int resNr);

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

	Common::File *getVolumeFile(const char *filename);
	void loadResource(Resource *res);
	bool loadPatch(Resource *res, Common::File &file);
	bool loadFromPatchFile(Resource *res);
	bool loadFromAudioVolumeSCI1(Resource *res, Common::File &file);
	bool loadFromAudioVolumeSCI11(Resource *res, Common::File &file);
	void freeOldResources();
	int decompress(Resource *res, Common::File *file);
	int readResourceInfo(Resource *res, Common::File *file, uint32&szPacked, ResourceCompression &compression);
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
	void processPatch(ResourceSource *source, ResourceType restype, int resnumber);

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
	SciVersion detectSciVersion();
};

} // End of namespace Sci

#endif // SCI_SCICORE_RESOURCE_H
