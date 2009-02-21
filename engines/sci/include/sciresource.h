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

#ifndef _SCIRESOURCE_H_
#define _SCIRESOURCE_H_

/*#define _SCI_RESOURCE_DEBUG */
/*#define _SCI_DECOMPRESS_DEBUG*/

#include "common/stream.h"
#include "common/str.h"

#include "sci/tools.h"
#include "sci/include/versions.h"

namespace Sci {

#define SCI_MAX_RESOURCE_SIZE 0x0400000
/* The maximum allowed size for a compressed or decompressed resource */

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

#define RESSOURCE_TYPE_DIRECTORY 0
#define RESSOURCE_TYPE_VOLUME 2
#define RESSOURCE_TYPE_EXTERNAL_MAP 3
#define RESSOURCE_TYPE_INTERNAL_MAP 4
#define RESSOURCE_TYPE_MASK 127
#define RESSOURCE_ADDRESSING_BASIC 0
#define RESSOURCE_ADDRESSING_EXTENDED 128
#define RESSOURCE_ADDRESSING_MASK 128

extern const char* sci_error_types[];
extern const char* sci_version_types[];
extern const char* sci_resource_types[];
extern const char* sci_resource_type_suffixes[]; /* Suffixes for SCI1 patch files */
extern const int sci_max_resource_nr[]; /* Highest possible resource numbers */


enum ResourceTypes {
	sci_view = 0, sci_pic, sci_script, sci_text,
	sci_sound, sci_memory, sci_vocab, sci_font,
	sci_cursor, sci_patch, sci_bitmap, sci_palette,
	sci_cdaudio, sci_audio, sci_sync, sci_message,
	sci_map, sci_heap, sci_invalid_resource
};

#define sci0_last_resource sci_patch
#define sci1_last_resource sci_heap
/* Used for autodetection */


struct resource_index_struct {
	unsigned short resource_id;
	unsigned int resource_location;
}; /* resource type as stored in the resource.map file */

typedef struct resource_index_struct resource_index_t;

struct ResourceSource {
	int source_type;
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
struct resource_t {
	unsigned char *data;

	unsigned short number;
	unsigned short type;
	guint16 id; /* contains number and type */

	unsigned int size;

	unsigned int file_offset; /* Offset in file */
	ResourceSource *source;

	unsigned char status;
	unsigned short lockers; /* Number of places where this resource was locked */

	resource_t *next; /* Position marker for the LRU queue */
	resource_t *prev;

	resource_altsource_t *alt_sources; /* SLL of alternative resource data sources */
}; 


struct ResourceManager {
	int _maxMemory; /* Config option: Maximum total byte number allocated */
	int sci_version; /* SCI resource version to use */

	int _resourcesNr;
	ResourceSource *_sources;
	resource_t *_resources;

	int memory_locked; /* Amount of resource bytes in locked memory */
	int memory_lru; /* Amount of resource bytes under LRU control */

	resource_t *lru_first, *lru_last; /* Pointers to the first and last LRU queue entries */
	/* LRU queue: lru_first points to the most recent entry */

public:
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
};

/**** FUNCTION DECLARATIONS ****/

/**--- New Resource manager ---**/

ResourceSource *scir_add_patch_dir(ResourceManager *mgr, const char *path);
/* Add a path to the resource manager's list of sources.
** Parameters: (ResourceManager *) mgr: The resource manager to look up in
**             (const char *) path: The path to add
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

ResourceSource *scir_get_volume(ResourceManager *mgr, ResourceSource *map, int volume_nr);

ResourceSource *scir_add_volume(ResourceManager *mgr, ResourceSource *map, const char *filename,
                int number, int extended_addressing);
/* Add a volume to the resource manager's list of sources.
** Parameters: (ResourceManager *) mgr: The resource manager to look up in
**             (ResourceSource *) map: The map associated with this volume
**             (char *) filename: The name of the volume to add
**             (int) extended_addressing: 1 if this volume uses extended addressing,
**                                        0 otherwise.
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

ResourceSource *scir_add_external_map(ResourceManager *mgr, const char *file_name);
/* Add an external (i.e. separate file) map resource to the resource manager's list of sources.
** Parameters: (ResourceManager *) mgr: The resource manager to look up in
**             (const char *) file_name: The name of the volume to add
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

int scir_scan_new_sources(ResourceManager *mgr, int *detected_version);
/* Scans newly registered resource sources for resources, earliest addition first.
** Parameters: (ResourceManager *) mgr: The resource manager to look up in
**             (int *) detected_version: Pointer to the detected version number,
**					 used during startup. May be NULL.
** Returns: One of SCI_ERROR_*.
*/

resource_t *scir_find_resource(ResourceManager *mgr, int type, int number, int lock);
/* Looks up a resource's data
** Parameters: (ResourceManager *) mgr: The resource manager to look up in
**             (int) type: The resource type to look for
**             (int) number: The resource number to search
**             (int) lock: non-zero iff the resource should be locked
** Returns   : (resource_t *): The resource, or NULL if it doesn't exist
** Locked resources are guaranteed not to have their contents freed until
** they are unlocked explicitly (by scir_unlock_resource).
*/

void scir_unlock_resource(ResourceManager *mgr, resource_t *res, int restype, int resnum);
/* Unlocks a previously locked resource
** Parameters: (ResourceManager *) mgr: The manager the resource should be freed from
**             (resource_t *) res: The resource to free
**             (int) type: Type of the resource to check (for error checking)
**             (int) number: Number of the resource to check (ditto)
** Returns   : (void)
*/

resource_t *scir_test_resource(ResourceManager *mgr, int type, int number);
/* Tests whether a resource exists
** Parameters: (ResourceManager *) mgr: The resource manager to search in
**             (int) type: Type of the resource to check
**             (int) number: Number of the resource to check
** Returns   : (resource_t *) non-NULL if the resource exists, NULL otherwise
** This function may often be much faster than finding the resource
** and should be preferred for simple tests.
** The resource object returned is, indeed, the resource in question, but
** it should be used with care, as it may be unallocated.
** Use scir_find_resource() if you want to use the data contained in the resource.
*/

/**--- Resource map decoding functions ---*/

int sci0_read_resource_map(ResourceManager *mgr, ResourceSource *map, resource_t **resources, int *resource_nr_p, int *sci_version);
/* Reads the SCI0 resource.map file from a local directory
** Parameters: (char *) path: (unused)
**             (resource_t **) resources: Pointer to a pointer
**                                        that will be set to the
**                                        location of the resources
**                                        (in one large chunk)
**             (int *) resource_nr_p: Pointer to an int the number of resources
**                                    read is stored in
**             (int) sci_version: SCI resource version
** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
*/

int sci1_read_resource_map(ResourceManager *mgr, ResourceSource *map, ResourceSource *vol,
                       resource_t **resource_p, int *resource_nr_p, int *sci_version);
/* Reads the SCI1 resource.map file from a local directory
** Parameters: (char *) path: (unused)
**             (resource_t **) resources: Pointer to a pointer
**                                        that will be set to the
**                                        location of the resources
**                                        (in one large chunk)
**             (int *) resource_nr_p: Pointer to an int the number of resources
**                                    read is stored in
**             (int) sci_version: SCI resource version
** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
*/

/**--- Patch management functions ---*/

void sci0_sprintf_patch_file_name(char *string, resource_t *res);
/* Prints the name of a matching patch to a string buffer
** Parameters: (char *) string: The buffer to print to
**             (resource_t *) res: Resource containing the number and type of the
**                                 resource whose name is to be print
** Returns   : (void)
*/

void sci1_sprintf_patch_file_name(char *string, resource_t *res);
/* Prints the name of a matching patch to a string buffer
** Parameters: (char *) string: The buffer to print to
**             (resource_t *) res: Resource containing the number and type of the
**                                 resource whose name is to be print
** Returns   : (void)
*/

int sci0_read_resource_patches(ResourceSource *source, resource_t **resources, int *resource_nr_p);
/* Reads SCI0 patch files from a local directory
** Parameters: (char *) path: (unused)
**             (resource_t **) resources: Pointer to a pointer
**                                        that will be set to the
**                                        location of the resources
**                                        (in one large chunk)
**             (int *) resource_nr_p: Pointer to an int the number of resources
**                                    read is stored in
** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
*/

int sci1_read_resource_patches(ResourceSource *source, resource_t **resources, int *resource_nr_p);
/* Reads SCI1 patch files from a local directory
** Parameters: (char *) path: (unused)
**             (resource_t **) resources: Pointer to a pointer
**                                        that will be set to the
**                                        location of the resources
**                                        (in one large chunk)
**             (int *) resource_nr_p: Pointer to an int the number of resources
**                                    read is stored in
** Returns   : (int) 0 on success, an SCI_ERROR_* code otherwise
*/


/**--- Decompression functions ---**/


int decompress0(resource_t *result, Common::ReadStream &stream, int sci_version);
/* Decrypts resource data and stores the result for SCI0-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              stream: Stream of the resource file
**              sci_version : Actual SCI resource version
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/

int decompress01(resource_t *result, Common::ReadStream &stream, int sci_version);
/* Decrypts resource data and stores the result for SCI01-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              stream: Stream of the resource file
**              sci_version : Actual SCI resource version
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/

int decompress1(resource_t *result, Common::ReadStream &stream, int sci_version);
/* Decrypts resource data and stores the result for SCI1.1-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              sci_version : Actual SCI resource version
**              stream: Stream of the resource file
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/


int decompress11(resource_t *result, Common::ReadStream &stream, int sci_version);
/* Decrypts resource data and stores the result for SCI1.1-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              sci_version : Actual SCI resource version
**              stream: Stream of the resource file
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/


int decrypt2(guint8* dest, guint8* src, int length, int complength);
/* Huffman token decryptor - defined in decompress0.c and used in decompress01.c
*/

int decrypt4(guint8* dest, guint8* src, int length, int complength);
/* DCL inflate- implemented in decompress1.c
*/

byte *view_reorder(byte *inbuffer, int dsize);
/* SCI1 style view compression */

byte *pic_reorder(byte *inbuffer, int dsize);
/* SCI1 style pic compression */

/*--- Internal helper functions ---*/

void _scir_free_resources(resource_t *resources, int _resourcesNr);
/* Frees a block of resources and associated data
** Parameters: (resource_t *) resources: The resources to free
**             (int) _resourcesNr: Number of resources in the block
** Returns   : (void)
*/

resource_t *_scir_find_resource_unsorted(resource_t *res, int res_nr, int type, int number);
/* Finds a resource matching type.number in an unsorted resource_t block
** To be used during initial resource loading, when the resource list
** may not have been sorted yet.
** Parameters: (resource_t *) res: Pointer to the block to search in
**             (int) res_nr: Number of resource_t structs allocated and defined
**                           in the block pointed to by res
**             (int) type: Type of the resource to look for
**             (int) number: Number of the resource to look for
** Returns   : (resource_t) The matching resource entry, or NULL if not found
*/

void _scir_add_altsource(resource_t *res, ResourceSource *source, unsigned int file_offset);
/* Adds an alternative source to a resource
** Parameters: (resource_t *) res: The resource to add to
**             (ResourceSource *) source: The source of the resource
**             (unsigned int) file_offset: Offset in the file the resource
**                            is stored at
** Returns   : (void)
*/

} // End of namespace Sci

#endif



