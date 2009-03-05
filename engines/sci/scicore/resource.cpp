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

#include "common/util.h"
#include "common/debug.h"

#include "sci/tools.h"
#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"
#include "sci/scicore/vocabulary.h"

namespace Sci {

#undef SCI_REQUIRE_RESOURCE_FILES
//#define SCI_VERBOSE_RESMGR 1

const char *sci_version_types[] = {
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

const char *sci_error_types[] = {
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

// These are the 18 resource types supported by SCI1
const char *resourceTypeNames[] = {
	"view", "pic", "script", "text", "sound",
	"memory", "vocab", "font", "cursor",
	"patch", "bitmap", "palette", "cdaudio",
	"audio", "sync", "message", "map", "heap"
};

const char *resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	"   ", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep"
};

const char *getResourceTypeName(ResourceType restype) {
	return resourceTypeNames[restype];
}

const char *getResourceTypeSuffix(ResourceType restype) {
	return resourceTypeSuffixes[restype];
}

typedef int decomp_funct(Resource *result, Common::ReadStream &stream, int sci_version);
typedef void patch_sprintf_funct(char *string, Resource *res);

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

//-- Resource main functions --
Resource::Resource() {
	data = NULL;
	number = 0;
	type = kResourceTypeInvalid;
	id = 0;
	size = 0;
	file_offset = 0;
	status = SCI_STATUS_NOMALLOC;
	lockers = 0;
	next = prev = NULL;
	source = NULL;
}

Resource::~Resource() {
	delete[] data;
}

void Resource::unalloc() {
	delete[] data;
	data = NULL;
	status = SCI_STATUS_NOMALLOC;
}

//-- Resmgr helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const char *file_name) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = file_name;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;

	return newsrc;
}

ResourceSource *ResourceManager::addVolume(ResourceSource *map, const char *filename, int number, int extended_addressing) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceVolume;
	newsrc->scanned = false;
	newsrc->location_name = filename;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	return 0;
}

ResourceSource *ResourceManager::addPatchDir(const char *dirname) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceDirectory;
	newsrc->scanned = false;
	newsrc->location_name = dirname;

	return 0;
}

ResourceSource *ResourceManager::getVolume(ResourceSource *map, int volume_nr) {
	ResourceSource *seeker = _sources;

	while (seeker) {
		if (seeker->source_type == kSourceVolume && seeker->associated_map == map &&
		        seeker->volume_number == volume_nr)
			return seeker;
		seeker = seeker->next;
	}

	return NULL;
}

// Resource manager constructors and operations

bool ResourceManager::loadFromPatchFile(Resource *res) {
	Common::File file;
	char filename[MAXPATHLEN];
	if (!patch_sprintfers[_sciVersion]) {
		error("Resource manager's SCI version (%d) has no patch file name printers", _sciVersion);
	}
	// TODO: use only dir specified by res->source->location_dir_name
	patch_sprintfers[_sciVersion](filename, res);
	if (file.open(filename) == false) {
		warning("Failed to open patch file %s", filename);
		res->unalloc();
		return false;
	}
	res->data = new byte[res->size];

	if (res->data == NULL) {
		error("Can't allocate %d bytes needed for loading %s!", res->size, filename);
	}

	file.seek(res->file_offset, SEEK_SET);
	unsigned int really_read = file.read(res->data, res->size);
	if (really_read != res->size) {
		error("Read %d bytes from %s but expected %d!", really_read, filename, res->size);
	}
	res->status = SCI_STATUS_ALLOCATED;
	return true;
}

void ResourceManager::loadResource(Resource *res, bool protect) {
	// TODO: check if protect is needed at all
	char filename[MAXPATHLEN];
	Common::File file;
	Resource backup;

	memcpy(&backup, res, sizeof(Resource));

	if (res->source->source_type == kSourceDirectory && loadFromPatchFile(res))
		return;
	// Either loading from volume or patch loading failed
	strcpy(filename, res->source->location_name.c_str());

	if (!file.open(filename)) {
		warning("Failed to open %s", filename);
		res->unalloc();
		return;
	}
	file.seek(res->file_offset, SEEK_SET);

	// Check whether we support this at all
	if (decompressors[_sciVersion] == NULL)
		error("Resource manager's SCI version (%d) is invalid", _sciVersion);
	// Decompress from regular resource file
	int error = decompressors[_sciVersion](res, file, _sciVersion);

	if (error) {
		warning("Error %d occured while reading %s.%03d from resource file: %s\n",
			error, getResourceTypeName(res->type), res->number, sci_error_types[error]);

		if (protect)
			memcpy(res, &backup, sizeof(Resource));
		res->unalloc();
	}
}

Resource *ResourceManager::testResource(ResourceType type, int number) {
	Resource binseeker;
	binseeker.type = type;
	binseeker.number = number;
	if (_resMap.contains(RESOURCE_HASH(type, number)))
		return _resMap.getVal(RESOURCE_HASH(type, number));
	return NULL;
}

int sci0_get_compression_method(Common::ReadStream &stream);

int sci_test_view_type(ResourceManager *mgr) {
	Common::File file;
	char filename[MAXPATHLEN];
	int compression;
	Resource *res;
	int i;

	mgr->_sciVersion = SCI_VERSION_AUTODETECT;

	for (i = 0; i < 1000; i++) {
		res = mgr->testResource(kResourceTypeView, i);

		if (!res)
			continue;

		if (res->source->source_type == kSourceDirectory)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3) {
			mgr->_sciVersion = SCI_VERSION_01_VGA;
			return mgr->_sciVersion;
		}
	}

	// Try the same thing with pics
	for (i = 0; i < 1000; i++) {
		res = mgr->testResource(kResourceTypePic, i);

		if (!res)
			continue;

		if (res->source->source_type == kSourceDirectory)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3) {
			mgr->_sciVersion = SCI_VERSION_01_VGA;
			return mgr->_sciVersion;
		}
	}

	return mgr->_sciVersion;
}

int ResourceManager::addAppropriateSources() {
	//char path_separator;
	ResourceSource *map;

	if (!Common::File::exists("RESOURCE.MAP"))
		return 0;
	map = addExternalMap("RESOURCE.MAP");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "RESOURCE.0??");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addVolume(map, name.c_str(), number, 0);
	}
	addPatchDir("");	// FIXME: used to pass the 'current' instead of ""
	return 1;
}

int ResourceManager::scanNewSources(int *detected_version, ResourceSource *source) {
	if (!source)
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;

	int preset_version = _sciVersion;
	int resource_error = 0;
	int dummy = _sciVersion;

	if (detected_version == NULL)
		detected_version = &dummy;

	*detected_version = _sciVersion;
	if (source->next)
		scanNewSources(detected_version, source->next);

	if (!source->scanned) {
		source->scanned = true;
		switch (source->source_type) {
		case kSourceDirectory:
			readResourcePatches(source);
			break;
		case kSourceExtMap:
			if (preset_version <= SCI_VERSION_01_VGA_ODD /* || preset_version == SCI_VERSION_AUTODETECT -- subsumed by the above line */) {
				resource_error = readResourceMapSCI0(source, detected_version);
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
					mgr->_resources = 0; // FIXME: Was = (Resource*)sci_malloc(1);
					resource_error = 0;
				}
#endif
			}

			if ((preset_version == SCI_VERSION_1_EARLY) || (preset_version == SCI_VERSION_1_LATE) || (preset_version == SCI_VERSION_1_1) ||
			        ((*detected_version == SCI_VERSION_AUTODETECT) && (preset_version == SCI_VERSION_AUTODETECT))) {
				resource_error = readResourceMapSCI1(source, getVolume(source, 0), detected_version);
				if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
					// FIXME: Try reading w/o resource.map
					resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}

				if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
					// Initialize empty resource manager
					_resMap.clear();
					resource_error = 0;
				}
			}

			_sciVersion = *detected_version;
			break;
		default:
			break;
		}
	}
	return resource_error;
}

void ResourceManager::freeResourceSources(ResourceSource *rss) {
	if (rss) {
		freeResourceSources(rss->next);
		delete rss;
	}
}

ResourceManager::ResourceManager(int version, int maxMemory) {
	int resmap_version = version;

	_maxMemory = maxMemory;

	_memoryLocked = 0;
	_memoryLRU = 0;

	_resMap.clear();
	_sources = NULL;
	_sciVersion = version;

	lru_first = NULL;
	lru_last = NULL;

	addAppropriateSources();
	scanNewSources(&resmap_version, _sources);

	if (version == SCI_VERSION_AUTODETECT)
		switch (resmap_version) {
		case SCI_VERSION_0:
			if (testResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB)) {
				version = sci_test_view_type(this);
				if (version == SCI_VERSION_01_VGA) {
					debug("Resmgr: Detected KQ5 or similar\n");
				} else {
					debug("Resmgr: Detected SCI0\n");
					version = SCI_VERSION_0;
				}
			} else if (testResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB)) {
				version = sci_test_view_type(this);
				if (version == SCI_VERSION_01_VGA) {
					debug("Resmgr: Detected KQ5 or similar\n");
				} else {
					if (testResource(kResourceTypeVocab, 912)) {
						debug("Resmgr: Running KQ1 or similar, using SCI0 resource encoding\n");
						version = SCI_VERSION_0;
					} else {
						version = SCI_VERSION_01;
						debug("Resmgr: Detected SCI01\n");
					}
				}
			} else {
				version = sci_test_view_type(this);
				if (version == SCI_VERSION_01_VGA) {
					debug("Resmgr: Detected KQ5 or similar\n");
				} else {
					debug("Resmgr: Warning: Could not find vocabulary; assuming SCI0 w/o parser\n");
					version = SCI_VERSION_0;
				}
			}
			break;
		case SCI_VERSION_01_VGA_ODD:
			version = resmap_version;
			debug("Resmgr: Detected Jones/CD or similar\n");
			break;
		case SCI_VERSION_1: {
			Resource *res = testResource(kResourceTypeScript, 0);

			_sciVersion = version = SCI_VERSION_1_EARLY;
			loadResource(res, true);

			if (res->status == SCI_STATUS_NOMALLOC) {
				_sciVersion = version = SCI_VERSION_1_LATE;
				debug("Resmgr: Detected SCI1 Late"); 
			} else
				debug("Resmgr: Detected SCI1 Early"); 
			break;
		}
		case SCI_VERSION_1_1:
			// No need to handle SCI 1.1 here - it was done in resource_map.cpp
			version = SCI_VERSION_1_1;
			debug("Resmgr: Detected SCI1.1"); 
			break;
		default:
			debug("Resmgr: Warning: While autodetecting: Couldn't determine SCI version");
		}

	_sciVersion = version;
}

ResourceManager::~ResourceManager() {
	// freeing resources
	Common::HashMap<uint32, Resource *>::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		itr ++;
	}
	freeResourceSources(_sources);
	_resMap.empty();
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->status != SCI_STATUS_ENQUEUED) {
		sciprintf("Resmgr: Oops: trying to remove resource that isn't enqueued\n");
		return;
	}

	if (res->next)
		res->next->prev = res->prev;
	if (res->prev)
		res->prev->next = res->next;
	if (lru_first == res)
		lru_first = res->next;
	if (lru_last == res)
		lru_last = res->prev;

	_memoryLRU -= res->size;

	res->status = SCI_STATUS_ALLOCATED;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->status != SCI_STATUS_ALLOCATED) {
		sciprintf("Resmgr: Oops: trying to enqueue resource with state %d\n", res->status);
		return;
	}

	res->prev = NULL;
	res->next = lru_first;
	lru_first = res;
	if (!lru_last)
		lru_last = res;
	if (res->next)
		res->next->prev = res;

	_memoryLRU += res->size;
#if (SCI_VERBOSE_RESMGR > 1)
	fprintf(stderr, "Adding %s.%03d (%d bytes) to lru control: %d bytes total\n",
	        getResourceTypeName(res->type), res->number, res->size,
	        mgr->_memoryLRU);

#endif

	res->status = SCI_STATUS_ENQUEUED;
}

void ResourceManager::printLRU() {
	int mem = 0;
	int entries = 0;
	Resource *res = lru_first;

	while (res) {
		fprintf(stderr, "\t%s.%03d: %d bytes\n",
		        getResourceTypeName(res->type), res->number,
		        res->size);
		mem += res->size;
		++entries;
		res = res->next;
	}

	fprintf(stderr, "Total: %d entries, %d bytes (mgr says %d)\n",
	        entries, mem, _memoryLRU);
}

void ResourceManager::freeOldResources(int last_invulnerable) {
	while (_maxMemory < _memoryLRU && (!last_invulnerable || lru_first != lru_last)) {
		Resource *goner = lru_last;
		if (!goner) {
			fprintf(stderr, "Internal error: mgr->lru_last is NULL!\n");
			fprintf(stderr, "LRU-mem= %d\n", _memoryLRU);
			fprintf(stderr, "lru_first = %p\n", (void *)lru_first);
			printLRU();
		}

		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMGR
		sciprintf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
#endif
	}
}

Resource *ResourceManager::findResource(ResourceType type, int number, int lock) {
	Resource *retval;

	if (number >= sci_max_resource_nr[_sciVersion]) {
		int modded_number = number % sci_max_resource_nr[_sciVersion];
		sciprintf("[resmgr] Requested invalid resource %s.%d, mapped to %s.%d\n",
		          getResourceTypeName(type), number, getResourceTypeName(type), modded_number);
		number = modded_number;
	}

	retval = testResource(type, number);

	if (!retval)
		return NULL;

	if (!retval->status)
		loadResource(retval, false);

	else if (retval->status == SCI_STATUS_ENQUEUED)
		removeFromLRU(retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	if (lock) {
		if (retval->status == SCI_STATUS_ALLOCATED) {
			retval->status = SCI_STATUS_LOCKED;
			retval->lockers = 0;
			_memoryLocked += retval->size;
		}

		++retval->lockers;

	} else if (retval->status != SCI_STATUS_LOCKED) { // Don't lock it
		if (retval->status == SCI_STATUS_ALLOCATED)
			addToLRU(retval);
	}

	freeOldResources(retval->status == SCI_STATUS_ALLOCATED);

	if (retval->data)
		return retval;
	else {
		sciprintf("Resmgr: Failed to read %s.%03d\n", getResourceTypeName(retval->type), retval->number);
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res, int resnum, ResourceType restype) {
	if (!res) {
		if (restype == kResourceTypeInvalid)
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %03d.%03d!\n", restype, resnum);
		else
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %s.%03d!\n", getResourceTypeName(restype), resnum);
		return;
	}

	if (res->status != SCI_STATUS_LOCKED) {
		sciprintf("Resmgr: Warning: Attempt to unlock unlocked resource %s.%03d\n",
		          getResourceTypeName(res->type), res->number);
		return;
	}

	if (!--res->lockers) { // No more lockers?
		res->status = SCI_STATUS_ALLOCATED;
		_memoryLocked -= res->size;
		addToLRU(res);
	}

	freeOldResources(0);
}

} // End of namespace Sci
