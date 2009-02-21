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

// Resource library

#include "common/archive.h"
#include "common/file.h"
#include "common/util.h"

#include "sci/tools.h"
#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"
#include "sci/include/vocabulary.h"

namespace Sci {

#undef SCI_REQUIRE_RESOURCE_FILES
//#define SCI_VERBOSE_RESMGR 1

const char* sci_version_types[] = {
	"SCI version undetermined (Autodetect failed / not run)",
	"SCI version 0.xxx",
	"SCI version 0.xxx w/ 1.000 compression",
	"SCI version 1.000 w/ 0.xxx resource.map",
	"SCI version 1.000 w/ special resource.map",
	"SCI version 1.000 (early)",
	"SCI version 1.000 (late)",
	"SCI version 1.001",
	"SCI WIN/32"
};

const int sci_max_resource_nr[] = {65536, 1000, 2048, 2048, 2048, 8192, 8192, 65536};

const char* sci_error_types[] = {
	"No error",
	"I/O error",
	"Resource is empty (size 0)",
	"resource.map entry is invalid",
	"resource.map file not found",
	"No resource files found",
	"Unknown compression method",
	"Decompression failed: Decompression buffer overflow",
	"Decompression failed: Sanity check failed",
	"Decompression failed: Resource too big",
	"SCI version is unsupported"
};

const char* sci_resource_types[] = {"view", "pic", "script", "text", "sound",
                                    "memory", "vocab", "font", "cursor",
                                    "patch", "bitmap", "palette", "cdaudio",
                                    "audio", "sync", "message", "map", "heap"
                                   };
// These are the 18 resource types supported by SCI1

const char *sci_resource_type_suffixes[] = {"v56", "p56", "scr", "tex", "snd",
        "   ", "voc", "fon", "cur", "pat",
        "bit", "pal", "cda", "aud", "syn",
        "msg", "map", "hep"
                                           };


int resourcecmp(const void *first, const void *second);

typedef int decomp_funct(resource_t *result, Common::ReadStream &stream, int sci_version);
typedef void patch_sprintf_funct(char *string, resource_t *res);

static decomp_funct *decompressors[] = {
	NULL,
	&decompress0,
	&decompress01,
	&decompress01,
	&decompress01,
	&decompress1,
	&decompress1,
	&decompress11,
	NULL
};

static patch_sprintf_funct *patch_sprintfers[] = {
	NULL,
	&sci0_sprintf_patch_file_name,
	&sci0_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name,
	&sci1_sprintf_patch_file_name
};


int resourcecmp(const void *first, const void *second) {
	if (((resource_t *)first)->type ==
	        ((resource_t *)second)->type)
		return (((resource_t *)first)->number <
		        ((resource_t *)second)->number) ? -1 :
		       !(((resource_t *)first)->number ==
		         ((resource_t *)second)->number);
	else
		return (((resource_t *)first)->type <
		        ((resource_t *)second)->type) ? -1 : 1;
}


//-- Resmgr helper functions --

void _scir_add_altsource(resource_t *res, ResourceSource *source, unsigned int file_offset) {
	resource_altsource_t *rsrc = (resource_altsource_t *)sci_malloc(sizeof(resource_altsource_t));

	rsrc->next = res->alt_sources;
	rsrc->source = source;
	rsrc->file_offset = file_offset;
	res->alt_sources = rsrc;
}

resource_t *_scir_find_resource_unsorted(resource_t *res, int res_nr, int type, int number) {
	int i;
	for (i = 0; i < res_nr; i++)
		if (res[i].number == number && res[i].type == type)
			return res + i;
	return NULL;
}

// Resource source list management

ResourceSource *scir_add_external_map(ResourceManager *mgr, const char *file_name) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = mgr->_sources;
	mgr->_sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_EXTERNAL_MAP;
	newsrc->location_name = file_name;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;

	return newsrc;
}

ResourceSource *scir_add_volume(ResourceManager *mgr, ResourceSource *map, const char *filename, int number, int extended_addressing) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = mgr->_sources;
	mgr->_sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_VOLUME;
	newsrc->scanned = false;
	newsrc->location_name = filename;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	return 0;
}

ResourceSource *scir_add_patch_dir(ResourceManager *mgr, const char *dirname) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = mgr->_sources;
	mgr->_sources = newsrc;

	newsrc->source_type = RESSOURCE_TYPE_DIRECTORY;
	newsrc->scanned = false;
	newsrc->location_name = dirname;

	return 0;
}

ResourceSource *scir_get_volume(ResourceManager *mgr, ResourceSource *map, int volume_nr) {
	ResourceSource *seeker = mgr->_sources;

	while (seeker) {
		if (seeker->source_type == RESSOURCE_TYPE_VOLUME && seeker->associated_map == map &&
		        seeker->volume_number == volume_nr)
			return seeker;
		seeker = seeker->next;
	}

	return NULL;
}

// Resource manager constructors and operations

static void _scir_load_from_patch_file(Common::File &file, resource_t *res, char *filename) {
	unsigned int really_read;

	res->data = (unsigned char *)sci_malloc(res->size);
	really_read = file.read(res->data, res->size);

	if (really_read < res->size) {
		error("Read %d bytes from %s but expected %d!", really_read, filename, res->size);
	}

	res->status = SCI_STATUS_ALLOCATED;
}

static void _scir_load_resource(ResourceManager *mgr, resource_t *res, bool protect) {
	char filename[MAXPATHLEN];
	Common::File file;
	resource_t backup;

	memcpy(&backup, res, sizeof(resource_t));

	// First try lower-case name
	if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY) {
		if (!patch_sprintfers[mgr->sci_version]) {
			error("Resource manager's SCI version (%d) has no patch file name printers", mgr->sci_version);
		}

		// Get patch file name
		patch_sprintfers[mgr->sci_version](filename, res);
		
		// FIXME: Instead of using SearchMan, maybe we should only search
		// a single dir specified by this RESSOURCE_TYPE_DIRECTORY ResourceSource?
	} else
		strcpy(filename, res->source->location_name.c_str());

	if (!file.open(filename)) {
		warning("Failed to open %s", filename);
		res->data = NULL;
		res->status = SCI_STATUS_NOMALLOC;
		res->size = 0;
		return;
	}

	file.seek(res->file_offset, SEEK_SET);

	if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY)
		_scir_load_from_patch_file(file, res, filename);
	else if (!decompressors[mgr->sci_version]) {
		// Check whether we support this at all
		error("Resource manager's SCI version (%d) is invalid", mgr->sci_version);
	} else {
		int error = // Decompress from regular resource file
		    decompressors[mgr->sci_version](res, file, mgr->sci_version);

		if (error) {
			sciprintf("Error %d occured while reading %s.%03d from resource file: %s\n",
			          error, sci_resource_types[res->type], res->number, sci_error_types[error]);

			if (protect)
				memcpy(res, &backup, sizeof(resource_t));

			res->data = NULL;
			res->status = SCI_STATUS_NOMALLOC;
			res->size = 0;
		}
	}

}

resource_t *scir_test_resource(ResourceManager *mgr, int type, int number) {
	resource_t binseeker;
	binseeker.type = type;
	binseeker.number = number;
	return (resource_t *)bsearch(&binseeker, mgr->_resources, mgr->_resourcesNr, sizeof(resource_t), resourcecmp);
}

int sci0_get_compression_method(Common::ReadStream &stream);

int sci_test_view_type(ResourceManager *mgr) {
	Common::File file;
	char filename[MAXPATHLEN];
	int compression;
	resource_t *res;
	int i;

	mgr->sci_version = SCI_VERSION_AUTODETECT;

	for (i = 0; i < 1000; i++) {
		res = scir_test_resource(mgr, sci_view, i);

		if (!res)
			continue;

		if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3)
			return (mgr->sci_version = SCI_VERSION_01_VGA);
	}

	// Try the same thing with pics
	for (i = 0; i < 1000; i++) {
		res = scir_test_resource(mgr, sci_pic, i);

		if (!res)
			continue;

		if (res->source->source_type == RESSOURCE_TYPE_DIRECTORY)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3)
			return (mgr->sci_version = SCI_VERSION_01_VGA);
	}

	return mgr->sci_version;
}

int scir_add_appropriate_sources(ResourceManager *mgr) {
	//char path_separator;
	ResourceSource *map;

	if (!Common::File::exists("RESOURCE.MAP"))
		return 0;
	map = scir_add_external_map(mgr, "RESOURCE.MAP");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "RESOURCE.0??");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		scir_add_volume(mgr, map, name.c_str(), number, 0);
	}
	scir_add_patch_dir(mgr, "");	// FIXME: used to pass the 'current' instead of ""

	return 1;
}

static int _scir_scan_new_sources(ResourceManager *mgr, int *detected_version, ResourceSource *source) {
	int preset_version = mgr->sci_version;
	int resource_error = 0;
	int dummy = mgr->sci_version;
	//resource_t **concat_ptr = &(mgr->_resources[mgr->_resourcesNr - 1].next);

	if (detected_version == NULL)
		detected_version = &dummy;

	*detected_version = mgr->sci_version;
	if (source->next)
		_scir_scan_new_sources(mgr, detected_version, source->next);

	if (!source->scanned) {
		source->scanned = true;
		switch (source->source_type) {
		case RESSOURCE_TYPE_DIRECTORY:
			if (mgr->sci_version <= SCI_VERSION_01)
				sci0_read_resource_patches(source, &mgr->_resources, &mgr->_resourcesNr);
			else
				sci1_read_resource_patches(source, &mgr->_resources, &mgr->_resourcesNr);
			break;
		case RESSOURCE_TYPE_EXTERNAL_MAP:
			if (preset_version <= SCI_VERSION_01_VGA_ODD /* || preset_version == SCI_VERSION_AUTODETECT -- subsumed by the above line */) {
				resource_error = sci0_read_resource_map(mgr, source, &mgr->_resources, &mgr->_resourcesNr, detected_version);
#if 0
				if (resource_error >= SCI_ERROR_CRITICAL) {
					sciprintf("Resmgr: Error while loading resource map: %s\n", sci_error_types[resource_error]);
					if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND)
						sciprintf("Running SCI games without a resource map is not supported ATM\n");
					sci_free(mgr);
					return NULL;
				}
				if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
					// FIXME: Try reading w/o resource.map
					resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}

				if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
					// Initialize empty resource manager
					mgr->_resourcesNr = 0;
					mgr->_resources = 0; // FIXME: Was = (resource_t*)sci_malloc(1);
					resource_error = 0;
				}
#endif
			}

			if ((preset_version == SCI_VERSION_1_EARLY) || (preset_version == SCI_VERSION_1_LATE) || (preset_version == SCI_VERSION_1_1) ||
			        ((*detected_version == SCI_VERSION_AUTODETECT) && (preset_version == SCI_VERSION_AUTODETECT))) {
				resource_error = sci1_read_resource_map(mgr, source, scir_get_volume(mgr, source, 0),
														&mgr->_resources, &mgr->_resourcesNr, detected_version);
				if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
					// FIXME: Try reading w/o resource.map
					resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}

				if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
					// Initialize empty resource manager
					mgr->_resourcesNr = 0;
					mgr->_resources = 0; // FIXME: Was = (resource_t*)sci_malloc(1);
					resource_error = 0;
				}
			}

			mgr->sci_version = *detected_version;
			break;
		}
		qsort(mgr->_resources, mgr->_resourcesNr, sizeof(resource_t), resourcecmp); // Sort resources
	}
	return resource_error;
}

int scir_scan_new_sources(ResourceManager *mgr, int *detected_version) {
	_scir_scan_new_sources(mgr, detected_version, mgr->_sources);

	return 0;
}

static void _scir_free_resource_sources(ResourceSource *rss) {
	if (rss) {
		_scir_free_resource_sources(rss->next);
		delete rss;
	}
}

ResourceManager::ResourceManager(int version, int maxMemory) {
	int resource_error = 0;
	ResourceManager *mgr = this;
	int resmap_version = version;

	_maxMemory = maxMemory;

	mgr->memory_locked = 0;
	mgr->memory_lru = 0;

	_resources = NULL;
	_resourcesNr = 0;
	_sources = NULL;
	mgr->sci_version = version;

	mgr->lru_first = NULL;
	mgr->lru_last = NULL;

	scir_add_appropriate_sources(mgr);
	scir_scan_new_sources(mgr, &resmap_version);

	if (!_resources || !_resourcesNr) {
		if (_resources) {
			free(_resources);
			_resources = NULL;
		}
		sciprintf("Resmgr: Could not retrieve a resource list!\n");
		_scir_free_resource_sources(mgr->_sources);
		error("FIXME: Move this code to an init() method so that we can perform error handling");
//		return NULL;
	}

	qsort(_resources, _resourcesNr, sizeof(resource_t), resourcecmp); // Sort resources

	if (version == SCI_VERSION_AUTODETECT)
		switch (resmap_version) {
		case SCI_VERSION_0:
			if (scir_test_resource(mgr, sci_vocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB)) {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					sciprintf("Resmgr: Detected SCI0\n");
					version = SCI_VERSION_0;
				}
			} else if (scir_test_resource(mgr, sci_vocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB)) {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					if (scir_test_resource(mgr, sci_vocab, 912)) {
						sciprintf("Resmgr: Running KQ1 or similar, using SCI0 resource encoding\n");
						version = SCI_VERSION_0;
					} else {
						version = SCI_VERSION_01;
						sciprintf("Resmgr: Detected SCI01\n");
					}
				}
			} else {
				version = sci_test_view_type(mgr);
				if (version == SCI_VERSION_01_VGA) {
					sciprintf("Resmgr: Detected KQ5 or similar\n");
				} else {
					sciprintf("Resmgr: Warning: Could not find vocabulary; assuming SCI0 w/o parser\n");
					version = SCI_VERSION_0;
				}
			}
			break;
		case SCI_VERSION_01_VGA_ODD:
			version = resmap_version;
			sciprintf("Resmgr: Detected Jones/CD or similar\n");
			break;
		case SCI_VERSION_1: {
			resource_t *res = scir_test_resource(mgr, sci_script, 0);

			mgr->sci_version = version = SCI_VERSION_1_EARLY;
			_scir_load_resource(mgr, res, true);

			if (res->status == SCI_STATUS_NOMALLOC)
				mgr->sci_version = version = SCI_VERSION_1_LATE;
			break;
		}
		case SCI_VERSION_1_1:
			// No need to handle SCI 1.1 here - it was done in resource_map.cpp
			version = SCI_VERSION_1_1;
			break;
		default:
			sciprintf("Resmgr: Warning: While autodetecting: Couldn't determine SCI version");
		}

	if (!resource_error) {
		qsort(_resources, _resourcesNr, sizeof(resource_t), resourcecmp); // Sort resources
	}

	mgr->sci_version = version;
}

static void _scir_free_altsources(resource_altsource_t *dynressrc) {
	if (dynressrc) {
		_scir_free_altsources(dynressrc->next);
		free(dynressrc);
	}
}

void _scir_free_resources(resource_t *resources, int _resourcesNr) {
	int i;

	for (i = 0; i < _resourcesNr; i++) {
		resource_t *res = resources + i;

		// FIXME: alt_sources->next may point to an invalid memory location
		_scir_free_altsources(res->alt_sources);

		if (res->status != SCI_STATUS_NOMALLOC)
			free(res->data);
	}

	free(resources);
}

ResourceManager::~ResourceManager() {
	_scir_free_resources(_resources, _resourcesNr);
	_scir_free_resource_sources(_sources);
	_resources = NULL;
}

static void _scir_unalloc(resource_t *res) {
	free(res->data);
	res->data = NULL;
	res->status = SCI_STATUS_NOMALLOC;
}

static void _scir_remove_from_lru(ResourceManager *mgr, resource_t *res) {
	if (res->status != SCI_STATUS_ENQUEUED) {
		sciprintf("Resmgr: Oops: trying to remove resource that isn't enqueued\n");
		return;
	}

	if (res->next)
		res->next->prev = res->prev;
	if (res->prev)
		res->prev->next = res->next;
	if (mgr->lru_first == res)
		mgr->lru_first = res->next;
	if (mgr->lru_last == res)
		mgr->lru_last = res->prev;

	mgr->memory_lru -= res->size;

	res->status = SCI_STATUS_ALLOCATED;
}

static void _scir_add_to_lru(ResourceManager *mgr, resource_t *res) {
	if (res->status != SCI_STATUS_ALLOCATED) {
		sciprintf("Resmgr: Oops: trying to enqueue resource with state %d\n", res->status);
		return;
	}

	res->prev = NULL;
	res->next = mgr->lru_first;
	mgr->lru_first = res;
	if (!mgr->lru_last)
		mgr->lru_last = res;
	if (res->next)
		res->next->prev = res;

	mgr->memory_lru += res->size;
#if (SCI_VERBOSE_RESMGR > 1)
	fprintf(stderr, "Adding %s.%03d (%d bytes) to lru control: %d bytes total\n",
	        sci_resource_types[res->type], res->number, res->size,
	        mgr->memory_lru);

#endif

	res->status = SCI_STATUS_ENQUEUED;
}

static void _scir_print_lru_list(ResourceManager *mgr) {
	int mem = 0;
	int entries = 0;
	resource_t *res = mgr->lru_first;

	while (res) {
		fprintf(stderr, "\t%s.%03d: %d bytes\n",
		        sci_resource_types[res->type], res->number,
		        res->size);
		mem += res->size;
		++entries;
		res = res->next;
	}

	fprintf(stderr, "Total: %d entries, %d bytes (mgr says %d)\n",
	        entries, mem, mgr->memory_lru);
}

static void _scir_free_old_resources(ResourceManager *mgr, int last_invulnerable) {
	while (mgr->_maxMemory < mgr->memory_lru && (!last_invulnerable || mgr->lru_first != mgr->lru_last)) {
		resource_t *goner = mgr->lru_last;
		if (!goner) {
			fprintf(stderr, "Internal error: mgr->lru_last is NULL!\n");
			fprintf(stderr, "LRU-mem= %d\n", mgr->memory_lru);
			fprintf(stderr, "lru_first = %p\n", (void *)mgr->lru_first);
			_scir_print_lru_list(mgr);
		}

		_scir_remove_from_lru(mgr, goner);
		_scir_unalloc(goner);
#ifdef SCI_VERBOSE_RESMGR
		sciprintf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n", sci_resource_types[goner->type], goner->number, goner->size);
#endif
	}
}

resource_t * scir_find_resource(ResourceManager *mgr, int type, int number, int lock) {
	resource_t *retval;

	if (number >= sci_max_resource_nr[mgr->sci_version]) {
		int modded_number = number % sci_max_resource_nr[mgr->sci_version];
		sciprintf("[resmgr] Requested invalid resource %s.%d, mapped to %s.%d\n",
		          sci_resource_types[type], number, sci_resource_types[type], modded_number);
		number = modded_number;
	}

	retval = scir_test_resource(mgr, type, number);

	if (!retval)
		return NULL;

	if (!retval->status)
		_scir_load_resource(mgr, retval, false);

	else if (retval->status == SCI_STATUS_ENQUEUED)
		_scir_remove_from_lru(mgr, retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	if (lock) {
		if (retval->status == SCI_STATUS_ALLOCATED) {
			retval->status = SCI_STATUS_LOCKED;
			retval->lockers = 0;
			mgr->memory_locked += retval->size;
		}

		++retval->lockers;

	} else if (retval->status != SCI_STATUS_LOCKED) { // Don't lock it
		if (retval->status == SCI_STATUS_ALLOCATED)
			_scir_add_to_lru(mgr, retval);
	}

	_scir_free_old_resources(mgr, retval->status == SCI_STATUS_ALLOCATED);

	if (retval->data)
		return retval;
	else {
		sciprintf("Resmgr: Failed to read %s.%03d\n", sci_resource_types[retval->type], retval->number);
		return NULL;
	}
}

void scir_unlock_resource(ResourceManager *mgr, resource_t *res, int resnum, int restype) {
	if (!res) {
		if (restype >= ARRAYSIZE(sci_resource_types))
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %03d.%03d!\n", restype, resnum);
		else
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %s.%03d!\n", sci_resource_types[restype], resnum);
		return;
	}

	if (res->status != SCI_STATUS_LOCKED) {
		sciprintf("Resmgr: Warning: Attempt to unlock unlocked resource %s.%03d\n",
		          sci_resource_types[res->type], res->number);
		return;
	}

	if (!--res->lockers) { // No more lockers?
		res->status = SCI_STATUS_ALLOCATED;
		mgr->memory_locked -= res->size;
		_scir_add_to_lru(mgr, res);
	}

	_scir_free_old_resources(mgr, 0);
}

} // End of namespace Sci
