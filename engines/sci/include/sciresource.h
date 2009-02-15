/***************************************************************************
 sciresource.h Copyright (C) 1999,2000,01 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) [creichen@rbg.informatik.tu-darmstadt.de]

 History:

   990327 - created (CR)

***************************************************************************/

#ifndef _SCIRESOURCE_H_
#define _SCIRESOURCE_H_

/*#define _SCI_RESOURCE_DEBUG */
/*#define _SCI_DECOMPRESS_DEBUG*/

#include "sci/include/resource.h"
#include "sci/include/versions.h"

#define SCI_MAX_RESOURCE_SIZE 0x0400000
/* The maximum allowed size for a compressed or decompressed resource */

#ifdef WIN32
#  define DIR_SEPARATOR_STR "\\"
#  define PATH_SEPARATOR_STR ";"
#else
#  define DIR_SEPARATOR_STR "/"
#  define PATH_SEPARATOR_STR ":"
#endif


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
#define RESSOURCE_TYPE_AUDIO_DIRECTORY 1
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
	sci_view=0, sci_pic, sci_script, sci_text,
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

typedef struct resource_source_struct {
	int source_type;
	int scanned;
	union {
		struct {
			char *name;
			int volume_number;
		} file;
		struct {
			char *name;
		} dir;
		struct {
			struct _resource_struct *resource;
		} internal_map;
	} location;
	struct resource_source_struct *associated_map;
	struct resource_source_struct *next;
} resource_source_t;

typedef struct _resource_altsource_struct {
	resource_source_t *source;
	unsigned int file_offset;
	struct _resource_altsource_struct *next;
} resource_altsource_t;


typedef struct _resource_struct {
	unsigned char *data;

	unsigned short number;
	unsigned short type;
	guint16 id; /* contains number and type */

	unsigned int size;

	unsigned int file_offset; /* Offset in file */
	resource_source_t *source;

	unsigned char status;
	unsigned short lockers; /* Number of places where this resource was locked */

	struct _resource_struct *next; /* Position marker for the LRU queue */
	struct _resource_struct *prev;

	resource_altsource_t *alt_sources; /* SLL of alternative resource data sources */
} resource_t; /* for storing resources in memory */


typedef struct {
	int max_memory; /* Config option: Maximum total byte number allocated */
	int sci_version; /* SCI resource version to use */

	int resources_nr;
	resource_source_t *sources;
	resource_t *resources;

	int memory_locked; /* Amount of resource bytes in locked memory */ 
	int memory_lru; /* Amount of resource bytes under LRU control */

	char *resource_path; /* Path to the resource and patch files */

	resource_t *lru_first, *lru_last; /* Pointers to the first and last LRU queue entries */
	/* LRU queue: lru_first points to the most recent entry */

	unsigned char allow_patches;
} resource_mgr_t;

/**** FUNCTION DECLARATIONS ****/

/**--- New Resource manager ---**/

resource_mgr_t *
scir_new_resource_manager(char *dir, int version, char allow_patches, int max_memory);
/* Creates a new FreeSCI resource manager
** Parameters: (char *) dir: Path to the resource and patch files (not modified or freed
**                           by the resource manager)
**             (int) version: The SCI version to look for; use SCI_VERSION_AUTODETECT
**                            in the default case.
**             (char ) allow_patches: Set to 1 if external patches (those look like
**                                   "view.101" or "script.093") should be applied
**             (int) max_memory: Maximum number of bytes to allow allocated for resources
** Returns   : (resource_mgr_t *) A newly allocated resource manager
** max_memory will not be interpreted as a hard limit, only as a restriction for resources
** which are not explicitly locked. However, a warning will be issued whenever this limit
** is exceeded.
*/

resource_source_t *
scir_add_patch_dir(resource_mgr_t *mgr, int type, char *path);
/* Add a path to the resource manager's list of sources.
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (int) dirtype: The type of patch directory to add, 
**             either RESSOURCE_TYPE_DIRECTORY or RESSOURCE_TYPE_AUDIO_DIRECTORY
**             (char *) path: The path to add
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

resource_source_t *
scir_get_volume(resource_mgr_t *mgr, resource_source_t *map, int volume_nr);

resource_source_t *
scir_add_volume(resource_mgr_t *mgr, resource_source_t *map, char *filename,
	        int number, int extended_addressing);
/* Add a volume to the resource manager's list of sources.
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (resource_source_t *) map: The map associated with this volume
**             (char *) filename: The name of the volume to add
**             (int) extended_addressing: 1 if this volume uses extended addressing,
**                                        0 otherwise.
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

resource_source_t *
scir_add_external_map(resource_mgr_t *mgr, char *file_name);
/* Add an external (i.e. separate file) map resource to the resource manager's list of sources.
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (char *) file_name: The name of the volume to add
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

resource_source_t *
scir_add_internal_map(resource_mgr_t *mgr, resource_t *map);
/* Add an internal (i.e. a resource) map resource to the resource manager's list of sources.
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (char *) file_name: The name of the volume to add
** Returns: A pointer to the added source structure, or NULL if an error occurred.
*/

int
scir_scan_new_sources(resource_mgr_t *mgr, int *detected_version);
/* Scans newly registered resource sources for resources, earliest addition first. 
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (int *) detected_version: Pointer to the detected version number,
**					 used during startup. May be NULL.
** Returns: One of SCI_ERROR_*. 
*/

resource_t *
scir_find_resource(resource_mgr_t *mgr, int type, int number, int lock);
/* Looks up a resource's data
** Parameters: (resource_mgr_t *) mgr: The resource manager to look up in
**             (int) type: The resource type to look for
**             (int) number: The resource number to search
**             (int) lock: non-zero iff the resource should be locked
** Returns   : (resource_t *): The resource, or NULL if it doesn't exist
** Locked resources are guaranteed not to have their contents freed until
** they are unlocked explicitly (by scir_unlock_resource).
*/

void
scir_unlock_resource(resource_mgr_t *mgr, resource_t *res, int restype, int resnum);
/* Unlocks a previously locked resource
** Parameters: (resource_mgr_t *) mgr: The manager the resource should be freed from
**             (resource_t *) res: The resource to free
**             (int) type: Type of the resource to check (for error checking)
**             (int) number: Number of the resource to check (ditto)
** Returns   : (void)
*/

resource_t *
scir_test_resource(resource_mgr_t *mgr, int type, int number);
/* Tests whether a resource exists
** Parameters: (resource_mgr_t *) mgr: The resource manager to search in
**             (int) type: Type of the resource to check
**             (int) number: Number of the resource to check
** Returns   : (resource_t *) non-NULL if the resource exists, NULL otherwise
** This function may often be much faster than finding the resource
** and should be preferred for simple tests.
** The resource object returned is, indeed, the resource in question, but
** it should be used with care, as it may be unallocated.
** Use scir_find_resource() if you want to use the data contained in the resource.
*/

void
scir_free_resource_manager(resource_mgr_t *mgr);
/* Frees a resource manager and all memory handled by it
** Parameters: (resource_mgr_t *) mgr: The Manager to free
** Returns   : (void)
*/

/**--- Resource map decoding functions ---*/

int
sci0_read_resource_map(resource_mgr_t *mgr, resource_source_t *map, resource_t **resources, int *resource_nr_p, int *sci_version);
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

int
sci1_read_resource_map(resource_mgr_t *mgr, resource_source_t *map, resource_source_t *vol,
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

void
sci0_sprintf_patch_file_name(char *string, resource_t *res);
/* Prints the name of a matching patch to a string buffer
** Parameters: (char *) string: The buffer to print to
**             (resource_t *) res: Resource containing the number and type of the
**                                 resource whose name is to be print
** Returns   : (void)
*/

void
sci1_sprintf_patch_file_name(char *string, resource_t *res);
/* Prints the name of a matching patch to a string buffer
** Parameters: (char *) string: The buffer to print to
**             (resource_t *) res: Resource containing the number and type of the
**                                 resource whose name is to be print
** Returns   : (void)
*/

int
sci0_read_resource_patches(resource_source_t *source, resource_t **resources, int *resource_nr_p);
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

int
sci1_read_resource_patches(resource_source_t *source, resource_t **resources, int *resource_nr_p);
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


int decompress0(resource_t *result, int resh, int sci_version);
/* Decrypts resource data and stores the result for SCI0-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              resh  : File handle of the resource file
**              sci_version : Actual SCI resource version
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/

int decompress01(resource_t *result, int resh, int sci_version);
/* Decrypts resource data and stores the result for SCI01-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              resh  : File handle of the resource file
**              sci_version : Actual SCI resource version
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/

int decompress1(resource_t *result, int resh, int sci_version);
/* Decrypts resource data and stores the result for SCI1.1-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              sci_version : Actual SCI resource version
**              resh  : File handle of the resource file
** Returns    : (int) 0 on success, one of SCI_ERROR_* if a problem was
**               encountered.
*/


int decompress11(resource_t *result, int resh, int sci_version);
/* Decrypts resource data and stores the result for SCI1.1-style compression.
** Parameters : result: The resource_t the decompressed data is stored in.
**              sci_version : Actual SCI resource version
**              resh  : File handle of the resource file
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

void
_scir_free_resources(resource_t *resources, int resources_nr);
/* Frees a block of resources and associated data
** Parameters: (resource_t *) resources: The resources to free
**             (int) resources_nr: Number of resources in the block
** Returns   : (void)
*/

resource_t *
_scir_find_resource_unsorted(resource_t *res, int res_nr, int type, int number);
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

void
_scir_add_altsource(resource_t *res, resource_source_t *source, unsigned int file_offset);
/* Adds an alternative source to a resource
** Parameters: (resource_t *) res: The resource to add to
**             (resource_source_t *) source: The source of the resource
**             (unsigned int) file_offset: Offset in the file the resource
**                            is stored at
** Retruns   : (void)
*/


/**** Internal #defines ****/

#define SCI_RESOURCE_FILE_PATCH -1 /* Identifies resources read from patches */

/* Resource type encoding */
#define SCI0_B1_RESTYPE_MASK  0xf8
#define SCI0_B1_RESTYPE_SHIFT 3
#define SCI0_B3_RESFILE_MASK  0xfc
#define SCI0_B3_RESFILE_SHIFT 2
#define SCI01V_B3_RESFILE_MASK  0xf0
#define SCI01V_B3_RESFILE_SHIFT 4

#define SCI0_RESID_GET_TYPE(bytes) \
    (((bytes)[1] & SCI0_B1_RESTYPE_MASK) >> SCI0_B1_RESTYPE_SHIFT)
#define SCI0_RESID_GET_NUMBER(bytes) \
    ((((bytes)[1] & ~SCI0_B1_RESTYPE_MASK) << 8) | ((bytes)[0]))

#define SCI0_RESFILE_GET_FILE(bytes) \
    (((bytes)[3] & SCI0_B3_RESFILE_MASK) >> SCI0_B3_RESFILE_SHIFT)
#define SCI0_RESFILE_GET_OFFSET(bytes) \
    ((((bytes)[3] & ~SCI0_B3_RESFILE_MASK) << 24) \
      | (((bytes)[2]) << 16) \
      | (((bytes)[1]) << 8) \
      | (((bytes)[0]) << 0))

#define SCI01V_RESFILE_GET_FILE(bytes) \
    (((bytes)[3] & SCI01V_B3_RESFILE_MASK) >> SCI01V_B3_RESFILE_SHIFT)
#define SCI01V_RESFILE_GET_OFFSET(bytes) \
    ((((bytes)[3] & ~SCI01V_B3_RESFILE_MASK) << 24) \
      | (((bytes)[2]) << 16) \
      | (((bytes)[1]) << 8) \
      | (((bytes)[0]) << 0))

#define SCI1_B5_RESFILE_MASK 0xf0
#define SCI1_B5_RESFILE_SHIFT 4

#define SCI1_RESFILE_GET_FILE(bytes) \
  (((bytes)[5] & SCI1_B5_RESFILE_MASK) >> SCI1_B5_RESFILE_SHIFT)

#define SCI1_RESFILE_GET_OFFSET(bytes) \
    ((((bytes)[5] & ~SCI1_B5_RESFILE_MASK) << 24) \
      | (((bytes)[4]) << 16) \
      | (((bytes)[3]) << 8) \
      | (((bytes)[2]) << 0))

#define SCI1_RESFILE_GET_NUMBER(bytes) \
      ((((bytes)[1]) << 8) \
      | (((bytes)[0]) << 0))

#define SCI11_RESFILE_GET_OFFSET(bytes) \
    ((((bytes)[4]) << 17) \
      | (((bytes)[3]) << 9) \
      | (((bytes)[2]) << 1))

#endif



