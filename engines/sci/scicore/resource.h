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
#include "common/archive.h"

namespace Common {
	class ReadStream;
}

namespace Sci {

/** The maximum allowed size for a compressed or decompressed resource */
#define SCI_MAX_RESOURCE_SIZE 0x0400000

/*** RESOURCE STATUS TYPES ***/
#define SCI_STATUS_NOMALLOC 0
#define SCI_STATUS_ALLOCATED 1
#define SCI_STATUS_ENQUEUED 2 /* In the LRU queue */
#define SCI_STATUS_LOCKED 3 /* Allocated and in use */

#define SCI_RES_FILE_NR_PATCH -1 /* Resource was read from a patch file rather than from a resource */


/*** INITIALIZATION RESULT TYPES ***/
#define SCI_ERROR_IO_ERROR 1
#define SCI_ERROR_EMPTY_OBJECT 2
#define SCI_ERROR_INVALID_RESMAP_ENTRY 3
/* Invalid resource.map entry */
#define SCI_ERROR_RESMAP_NOT_FOUND 4
#define SCI_ERROR_NO_RESOURCE_FILES_FOUND 5
/* No resource at all was found */
#define SCI_ERROR_UNKNOWN_COMPRESSION 6
#define SCI_ERROR_DECOMPRESSION_OVERFLOW 7
/* decompression failed: Buffer overflow (wrong SCI version?)  */
#define SCI_ERROR_DECOMPRESSION_INSANE 8
/* sanity checks failed during decompression */
#define SCI_ERROR_RESOURCE_TOO_BIG 9
/* Resource size exceeds SCI_MAX_RESOURCE_SIZE */
#define SCI_ERROR_UNSUPPORTED_VERSION 10
#define SCI_ERROR_INVALID_SCRIPT_VERSION 11

#define SCI_ERROR_CRITICAL SCI_ERROR_NO_RESOURCE_FILES_FOUND
/* the first critical error number */

/*** SCI VERSION NUMBERS ***/
#define SCI_VERSION_AUTODETECT 0
#define SCI_VERSION_0 1
#define SCI_VERSION_01 2
#define SCI_VERSION_01_VGA 3
#define SCI_VERSION_01_VGA_ODD 4
#define SCI_VERSION_1_EARLY 5
#define SCI_VERSION_1_LATE 6
#define SCI_VERSION_1_1 7
#define SCI_VERSION_32 8
#define SCI_VERSION_LAST SCI_VERSION_1_LATE /* The last supported SCI version */

#define SCI_VERSION_1 SCI_VERSION_1_EARLY

enum ResSourceType {
	RESSOURCE_TYPE_DIRECTORY = 0,
	RESSOURCE_TYPE_VOLUME = 2,
	RESSOURCE_TYPE_EXTERNAL_MAP = 3,
	RESSOURCE_TYPE_INTERNAL_MAP = 4,
	RESSOURCE_TYPE_MASK = 127
};

#define RESSOURCE_ADDRESSING_BASIC 0
#define RESSOURCE_ADDRESSING_EXTENDED 128
#define RESSOURCE_ADDRESSING_MASK 128

extern const char *sci_error_types[];
extern const char *sci_version_types[];
extern const int sci_max_resource_nr[]; /* Highest possible resource numbers */


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
	kResourceTypeInvalid
};

const char *getResourceTypeName(ResourceType restype);
// Suffixes for SCI1 patch files
const char *getResourceTypeSuffix(ResourceType restype);

#define sci0_last_resource kResourceTypePatch
#define sci1_last_resource kResourceTypeHeap
/* Used for autodetection */


#if 0
struct resource_index_struct {
	unsigned short resource_id;
	unsigned int resource_location;
}; /* resource type as stored in the resource.map file */

typedef struct resource_index_struct resource_index_t;
#endif

struct ResourceSource {
	ResSourceType source_type;
	bool scanned;
	Common::String location_name;	// FIXME: Replace by FSNode ?
	Common::String location_dir_name;	// FIXME: Get rid of this again, only a temporary HACK!
	int volume_number;
	ResourceSource *associated_map;
	ResourceSource *next;
};

struct resource_altsource_t {
	ResourceSource *source;
	unsigned int file_offset;
	resource_altsource_t *next;
};


/** Struct for storing resources in memory */
class Resource {

public:
// NOTE : Currently all member data has the same name and public visibility
// to let the rest of the engine compile without changes
	unsigned char *data;
	unsigned short number;
	ResourceType type;
	uint16 id; /* contains number and type */
	unsigned int size;
	unsigned int file_offset; /* Offset in file */
	unsigned char status;
	unsigned short lockers; /* Number of places where this resource was locked */
	Resource *next; /* Position marker for the LRU queue */
	Resource *prev;
	ResourceSource *source;
	resource_altsource_t *alt_sources; /* SLL of alternative resource data sources */
};


class ResourceManager {
public:
	int _sciVersion; /* SCI resource version to use */

	/**
	 * Creates a new FreeSCI resource manager.
	 * @param version		The SCI version to look for; use SCI_VERSION_AUTODETECT
	 *						in the default case.
	 * @param maxMemory		Maximum number of bytes to allow allocated for resources
	 *
	 * @note maxMemory will not be interpreted as a hard limit, only as a restriction
	 *    for resources which are not explicitly locked. However, a warning will be
	 *    issued whenever this limit is exceeded.
	 */
	ResourceManager(int version, int maxMemory);
	~ResourceManager();

	/* Add a path to the resource manager's list of sources.
	** Returns: A pointer to the added source structure, or NULL if an error occurred.
	*/
	ResourceSource *addPatchDir(const char *path);

	ResourceSource *getVolume(ResourceSource *map, int volume_nr);

	//! Add a volume to the resource manager's list of sources.
	/** @param map		The map associated with this volume
	 *	@param filename	The name of the volume to add
	 *	@param extended_addressing	1 if this volume uses extended addressing,
	 *                                        0 otherwise.
	 *	@return A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addVolume(ResourceSource *map, const char *filename,
		int number, int extended_addressing);

	//! Add an external (i.e. separate file) map resource to the resource manager's list of sources.
	/**	@param file_name	 The name of the volume to add
	 *	@return		A pointer to the added source structure, or NULL if an error occurred.
	 */
	ResourceSource *addExternalMap(const char *file_name);

	//! Scans newly registered resource sources for resources, earliest addition first.
	/** @param detected_version: Pointer to the detected version number,
	 *					 used during startup. May be NULL.
	 * @return One of SCI_ERROR_*.
	 */
	int scanNewSources(int *detected_version, ResourceSource *source);

	//! Looks up a resource's data
	/**	@param type: The resource type to look for
	 *	@param number: The resource number to search
	 *	@param lock: non-zero iff the resource should be locked
	 *	@return (Resource *): The resource, or NULL if it doesn't exist
	 *	@note Locked resources are guaranteed not to have their contents freed until
	 *		they are unlocked explicitly (by unlockResource).
	 */
	Resource *findResource(ResourceType type, int number, int lock);

	/* Unlocks a previously locked resource
	**             (Resource *) res: The resource to free
	**             (int) number: Number of the resource to check (ditto)
	**             (ResourceType) type: Type of the resource to check (for error checking)
	** Returns   : (void)
	*/
	void unlockResource(Resource *res, int restype, ResourceType resnum);

	/* Tests whether a resource exists
	**             (ResourceType) type: Type of the resource to check
	**             (int) number: Number of the resource to check
	** Returns   : (Resource *) non-NULL if the resource exists, NULL otherwise
	** This function may often be much faster than finding the resource
	** and should be preferred for simple tests.
	** The resource object returned is, indeed, the resource in question, but
	** it should be used with care, as it may be unallocated.
	** Use scir_find_resource() if you want to use the data contained in the resource.
	*/
	Resource *testResource(ResourceType type, int number);

protected:
	int _maxMemory; /* Config option: Maximum total byte number allocated */
	int _resourcesNr;
	ResourceSource *_sources;
	Resource *_resources;
	int _memoryLocked;	// Amount of resource bytes in locked memory
	int _memoryLRU;		// Amount of resource bytes under LRU control
	Resource *lru_first, *lru_last;	// Pointers to the first and last LRU queue entries
										// LRU queue: lru_first points to the most recent entry

	/* Frees a block of resources and associated data
	** Parameters: (Resource *) resources: The resources to free
	**             (int) _resourcesNr: Number of resources in the block
	** Returns   : (void)
	*/
	void freeResources(Resource *resources, int _resourcesNr);

	/* Finds a resource matching type.number in an unsorted Resource block
	** To be used during initial resource loading, when the resource list
	** may not have been sorted yet.
	** Parameters: (Resource *) res: Pointer to the block to search in
	**             (int) res_nr: Number of Resource structs allocated and defined
	**                           in the block pointed to by res
	**             (ResourceType) type: Type of the resource to look for
	**             (int) number: Number of the resource to look for
	** Returns   : (Resource) The matching resource entry, or NULL if not found
	*/
	Resource *findResourceUnsorted(Resource *res, int res_nr, ResourceType type, int number);

	/* Adds an alternative source to a resource
	** Parameters: (Resource *) res: The resource to add to
	**             (ResourceSource *) source: The source of the resource
	**             (unsigned int) file_offset: Offset in the file the resource
	**                            is stored at
	** Returns   : (void)
	*/
	void addAltSource(Resource *res, ResourceSource *source, unsigned int file_offset);

	int addAppropriateSources();
	void freeResourceSources(ResourceSource *rss);
	void freeAltSources(resource_altsource_t *dynressrc);

	void loadResource(Resource *res, bool protect);
	void loadFromPatchFile(Common::File &file, Resource *res, char *filename);
	void freeOldResources(int last_invulnerable);
	void unalloc(Resource *res);

	/**--- Resource map decoding functions ---*/

	/* Reads the SCI0 resource.map file from a local directory
	** Parameters: (char *) path: (unused)
	**             (int) sci_version: SCI resource version
	** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
	*/
	int readResourceMapSCI0(ResourceSource *map, int *sci_version);

	/* Reads the SCI1 resource.map file from a local directory
	** Parameters: (char *) path: (unused)
	**             (int) sci_version: SCI resource version
	** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
	*/
	int readResourceMapSCI1(ResourceSource *map, ResourceSource *vol, int *sci_version);

	int isSCI10or11(int *types);
	int detectOddSCI01(Common::File &file);
	int resReadEntry(ResourceSource *map, byte *buf, Resource *res, int sci_version);
	ResourceType resTypeSCI1(int ofs, int *types, ResourceType lastrt);
	int parseHeaderSCI1(Common::ReadStream &stream, int *types, ResourceType *lastrt);

	/**--- Patch management functions ---*/

	/* Reads SCI0 patch files from a local directory
	** Parameters: (char *) path: (unused)
	**             (Resource **) resources: Pointer to a pointer
	**                                        that will be set to the
	**                                        location of the resources
	**                                        (in one large chunk)
	**             (int *) resource_nr_p: Pointer to an int the number of resources
	**                                    read is stored in
	** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
	*/
	int readResourcePatchesSCI0(ResourceSource *source);

	/* Reads SCI1 patch files from a local directory
	** Parameters: (char *) path: (unused)
	**             (Resource **) resources: Pointer to a pointer
	**                                        that will be set to the
	**                                        location of the resources
	**                                        (in one large chunk)
	**             (int *) resource_nr_p: Pointer to an int the number of resources
	**                                    read is stored in
	** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
	*/
	int readResourcePatchesSCI1(ResourceSource *source);

	void process_patch(ResourceSource *source, Common::ArchiveMember &member,
		ResourceType restype, int resnumber);

	void printLRU();
	void addToLRU(Resource *res);
	void removeFromLRU(Resource *res);
};


	/* Prints the name of a matching patch to a string buffer
	** Parameters: (char *) string: The buffer to print to
	**             (Resource *) res: Resource containing the number and type of the
	**                                 resource whose name is to be print
	** Returns   : (void)
	*/
	void sci0_sprintf_patch_file_name(char *string, Resource *res);

	/* Prints the name of a matching patch to a string buffer
	** Parameters: (char *) string: The buffer to print to
	**             (Resource *) res: Resource containing the number and type of the
	**                                 resource whose name is to be print
	** Returns   : (void)
	*/
	void sci1_sprintf_patch_file_name(char *string, Resource *res);

	/**--- Decompression functions ---**/
	int decompress0(Resource *result, Common::ReadStream &stream, int sci_version);
	/* Decrypts resource data and stores the result for SCI0-style compression.
	** Parameters : result: The Resource the decompressed data is stored in.
	**              stream: Stream of the resource file
	**              sci_version : Actual SCI resource version
	** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
	**               encountered.
	*/

	int decompress01(Resource *result, Common::ReadStream &stream, int sci_version);
	/* Decrypts resource data and stores the result for SCI01-style compression.
	** Parameters : result: The Resource the decompressed data is stored in.
	**              stream: Stream of the resource file
	**              sci_version : Actual SCI resource version
	** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
	**               encountered.
	*/

	int decompress1(Resource *result, Common::ReadStream &stream, int sci_version);
	/* Decrypts resource data and stores the result for SCI1.1-style compression.
	** Parameters : result: The Resource the decompressed data is stored in.
	**              sci_version : Actual SCI resource version
	**              stream: Stream of the resource file
	** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
	**               encountered.
	*/

	int decompress11(Resource *result, Common::ReadStream &stream, int sci_version);
	/* Decrypts resource data and stores the result for SCI1.1-style compression.
	** Parameters : result: The Resource the decompressed data is stored in.
	**              sci_version : Actual SCI resource version
	**              stream: Stream of the resource file
	** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
	**               encountered.
	*/

	int unpackHuffman(uint8* dest, uint8* src, int length, int complength);
	/* Huffman token decryptor - defined in decompress0.c and used in decompress01.c
	*/

	int unpackDCL(uint8* dest, uint8* src, int length, int complength);
	/* DCL inflate- implemented in decompress1.c
	*/

	byte *view_reorder(byte *inbuffer, int dsize);
	/* SCI1 style view compression */

	byte *pic_reorder(byte *inbuffer, int dsize);
	/* SCI1 style pic compression */

} // End of namespace Sci

#endif // SCI_SCICORE_RESOURCE_H
