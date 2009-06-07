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

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/tools.h"
#include "sci/resource.h"
#include "sci/vocabulary.h"
#include "sci/decompressor.h"

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

const int sci_max_resource_nr[] = {65536, 1000, 2048, 2048, 2048, 65536, 65536, 65536};

enum SolFlags {
	kSolFlagCompressed = 1 << 0,
	kSolFlagUnknown    = 1 << 1,
	kSolFlag16Bit      = 1 << 2,
	kSolFlagIsSigned   = 1 << 3
};

static const char *sci_error_types[] = {
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

// These are the 20 resource types supported by SCI1.1
static const char *resourceTypeNames[] = {
	"view", "pic", "script", "text", "sound",
	"memory", "vocab", "font", "cursor",
	"patch", "bitmap", "palette", "cdaudio",
	"audio", "sync", "message", "map", "heap",
	"audio36", "sync36"
};

static const char *resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	"   ", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep", "aud", "syn"
};

const char *getResourceTypeName(ResourceType restype) {
	if (restype != kResourceTypeInvalid)
		return resourceTypeNames[restype];
	else
		return "invalid";
}

//-- Resource main functions --
Resource::Resource() {
	data = NULL;
	size = 0;
	file_offset = 0;
	status = kResStatusNoMalloc;
	lockers = 0;
	source = NULL;
	header = NULL;
	headerSize = 0;
}

Resource::~Resource() {
	delete[] data;
	if (source && source->source_type == kSourcePatch)
		delete source;
}

void Resource::unalloc() {
	delete[] data;
	data = NULL;
	status = kResStatusNoMalloc;
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

ResourceSource *ResourceManager::addSource(ResourceSource *map, ResSourceType type, const char *filename, int number) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = type;
	newsrc->scanned = false;
	newsrc->location_name = filename;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	return newsrc;
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
		if ((seeker->source_type == kSourceVolume || seeker->source_type == kSourceAudioVolume)
			&& seeker->associated_map == map && seeker->volume_number == volume_nr)
			return seeker;
		seeker = seeker->next;
	}

	return NULL;
}

// Resource manager constructors and operations

bool ResourceManager::loadPatch(Resource *res, Common::File &file) {
	// We assume that the resource type matches res->type
	file.seek(res->file_offset + 2, SEEK_SET);

	res->data = new byte[res->size];

	if (res->headerSize > 0)
		res->header = new byte[res->headerSize];	

	if ((res->data == NULL) || ((res->headerSize > 0) && (res->header == NULL))) {
		error("Can't allocate %d bytes needed for loading %s", res->size + res->headerSize, res->id.toString().c_str());
	}

	unsigned int really_read;
	if (res->headerSize > 0) {
		really_read = file.read(res->header, res->headerSize);
		if (really_read != res->headerSize)
			error("Read %d bytes from %s but expected %d", really_read, res->id.toString().c_str(), res->headerSize);
	}

	really_read = file.read(res->data, res->size);
	if (really_read != res->size)
		error("Read %d bytes from %s but expected %d", really_read, res->id.toString().c_str(), res->size);

	res->status = kResStatusAllocated;
	return true;
}

bool ResourceManager::loadFromPatchFile(Resource *res) {
	Common::File file;
	const char *filename = res->source->location_name.c_str();
	if (file.open(filename) == false) {
		warning("Failed to open patch file %s", filename);
		res->unalloc();
		return false;
	}

	return loadPatch(res, file);
}

bool ResourceManager::loadFromAudioVolume(Resource *res) {
	Common::File file;
	const char *filename = res->source->location_name.c_str();
	if (file.open(filename) == false) {
		warning("Failed to open audio volume %s", filename);
		res->unalloc();
		return false;
	}

	file.seek(res->file_offset, SEEK_SET);

	ResourceType type = (ResourceType)(file.readByte() & 0x7f);
	if (((res->id.type == kResourceTypeAudio || res->id.type == kResourceTypeAudio36) && (type != kResourceTypeAudio))
		|| ((res->id.type == kResourceTypeSync || res->id.type == kResourceTypeSync36) && (type != kResourceTypeSync))) {
		warning("Resource type mismatch loading %s from %s", res->id.toString().c_str(), filename);
		res->unalloc();
		return false;
	}

	res->headerSize = file.readByte();

	if (type == kResourceTypeAudio) {
		if (res->headerSize != 11 && res->headerSize != 12) {
			warning("Unsupported audio header");
			res->unalloc();
			return false;
		}

		// Load sample size
		file.seek(7, SEEK_CUR);
		res->size = file.readUint32LE();
	}

	return loadPatch(res, file);
}

Common::File *ResourceManager::getVolumeFile(const char *filename) {
	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	Common::File *file;

	// check if file is already opened
	while (it != _volumeFiles.end()) {
		file = *it;
		if (scumm_stricmp(file->getName(), filename) == 0) {
			// move file to top
			if (it != _volumeFiles.begin()) {
				_volumeFiles.erase(it);
				_volumeFiles.push_front(file);
			}
			return file;
		}
		it ++;
	}
	// adding a new file
	file = new Common::File;
	if (file->open(filename)) {
		if (_volumeFiles.size() == MAX_OPENED_VOLUMES) {
			it = --_volumeFiles.end();
			delete *it;
			_volumeFiles.erase(it);
		}
		_volumeFiles.push_front(file);
		return file;
	}
	// failed
	delete file;
	return NULL;
}

void ResourceManager::loadResource(Resource *res) {
	Common::File *file;

	if (res->source->source_type == kSourcePatch && loadFromPatchFile(res))
		return;
	if (res->source->source_type == kSourceAudioVolume) {
		loadFromAudioVolume(res);
		return;
	}
	// Either loading from volume or patch loading failed
	file = getVolumeFile(res->source->location_name.c_str());
	if (!file) {
		warning("Failed to open %s", res->source->location_name.c_str());
		res->unalloc();
		return;
	}
	file->seek(res->file_offset, SEEK_SET);
	int error = decompress(res, file);
	if (error) {
		warning("Error %d occured while reading %s from resource file: %s",
		        error, res->id.toString().c_str(), sci_error_types[error]);
		res->unalloc();
	}

}

Resource *ResourceManager::testResource(ResourceId id) {
	if (_resMap.contains(id))
		return _resMap.getVal(id);
	return NULL;
}

int sci0_get_compression_method(Common::ReadStream &stream) {
	uint16 compressionMethod;

	stream.readUint16LE();
	stream.readUint16LE();
	stream.readUint16LE();
	compressionMethod = stream.readUint16LE();
	if (stream.err())
		return SCI_ERROR_IO_ERROR;

	return compressionMethod;
}

int ResourceManager::guessSciVersion() {
	Common::File file;
	char filename[MAXPATHLEN];
	int compression;
	Resource *res;
	int i;

	for (i = 0; i < 1000; i++) {
		res = testResource(ResourceId(kResourceTypeView, i));

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
			return SCI_VERSION_01_VGA;
		}
	}

	// Try the same thing with pics
	for (i = 0; i < 1000; i++) {
		res = testResource(ResourceId(kResourceTypePic, i));

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
			return SCI_VERSION_01_VGA;
		}
	}

	return SCI_VERSION_AUTODETECT;
}

int ResourceManager::addAppropriateSources() {
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

		addSource(map, kSourceVolume, name.c_str(), number);
	}
	addPatchDir(".");
	// TODO: add RESOURCE.AUD and RESOURCE.SFX for SCI1.1 games
	if (Common::File::exists("MESSAGE.MAP"))
		addSource(addExternalMap("MESSAGE.MAP"), kSourceVolume, "RESOURCE.MSG", 0);
	return 1;
}

int ResourceManager::addInternalSources() {
	Common::List<ResourceId> *resources = listResources(kResourceTypeMap);
	Common::List<ResourceId>::iterator itr = resources->begin();

	while (itr != resources->end()) {
		ResourceSource *src = addSource(NULL, kSourceIntMap, "MAP", itr->number);

		if ((itr->number == 65535) && Common::File::exists("RESOURCE.SFX"))
			addSource(src, kSourceAudioVolume, "RESOURCE.SFX", 0);
		else if (Common::File::exists("RESOURCE.AUD"))
			addSource(src, kSourceAudioVolume, "RESOURCE.AUD", 0);

		itr++;
	}

	return 1;
}

int ResourceManager::scanNewSources(ResourceSource *source) {
	if (!source)
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;

	int resource_error = 0;
	if (source->next)
		scanNewSources(source->next);

	if (!source->scanned) {
		source->scanned = true;
		switch (source->source_type) {
		case kSourceDirectory:
			readResourcePatches(source);
			break;
		case kSourceExtMap:
			if (_mapVersion < SCI_VERSION_1)
				resource_error = readResourceMapSCI0(source);
			else
				resource_error = readResourceMapSCI1(source);

			if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
				// FIXME: Try reading w/o resource.map
				resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
			}

			if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
				// Initialize empty resource manager
				_resMap.clear();
				resource_error = 0;
			}
			break;
		case kSourceIntMap:
			resource_error = readMap(source);
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
	_maxMemory = maxMemory;
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_sources = NULL;
	_sciVersion = version;

	addAppropriateSources();

	if (version != SCI_VERSION_AUTODETECT) {
		_mapVersion = version;
		_volVersion = version;
	} else {
		_mapVersion = detectMapVersion();
		_volVersion = detectVolVersion();
	}
	debug("Using resource map version %d %s", _mapVersion, sci_version_types[_mapVersion]);
	debug("Using volume version %d %s", _volVersion, sci_version_types[_volVersion]);

	scanNewSources(_sources);
	addInternalSources();
	scanNewSources(_sources);

	if (version == SCI_VERSION_AUTODETECT)
		switch (_mapVersion) {
		case SCI_VERSION_0:
			if (testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB))) {
				version = guessSciVersion() ? SCI_VERSION_01_VGA : SCI_VERSION_0;
			} else if (testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB))) {
				version = guessSciVersion();
				if (version != SCI_VERSION_01_VGA) {
					version = testResource(ResourceId(kResourceTypeVocab, 912)) ? SCI_VERSION_0 : SCI_VERSION_01;
				}
			} else {
				version = guessSciVersion() ? SCI_VERSION_01_VGA : SCI_VERSION_0;
			}
			break;
		case SCI_VERSION_01_VGA_ODD:
			version = _mapVersion;
			break;
		case SCI_VERSION_1: {
			Resource *res = testResource(ResourceId(kResourceTypeScript, 0));

			_sciVersion = version = SCI_VERSION_1_EARLY;
			loadResource(res);

			if (res->status == kResStatusNoMalloc)
				version = SCI_VERSION_1_LATE;
			break;
		}
		case SCI_VERSION_1_1:
			// No need to handle SCI 1.1 here - it was done in resource_map.cpp
			version = SCI_VERSION_1_1;
			break;
		default:
			version = SCI_VERSION_AUTODETECT;
		}

	_sciVersion = version;
	// temporary version printout - should be reworked later
	switch (_sciVersion) {
	case SCI_VERSION_0:
		debug("Resmgr: Detected SCI0");
		break;
	case SCI_VERSION_01:
		debug("Resmgr: Detected SCI01");
		break;
	case SCI_VERSION_01_VGA:
		debug("Resmgr: Detected SCI01VGA - KQ5 or similar");
		break;
	case SCI_VERSION_01_VGA_ODD:
		debug("Resmgr: Detected SCI01VGA - Jones/CD or similar");
		break;
	case SCI_VERSION_1_EARLY:
		debug("Resmgr: Detected SCI1 Early");
		break;
	case SCI_VERSION_1_LATE:
		debug("Resmgr: Detected SCI1 Late");
		break;
	case SCI_VERSION_1_1:
		debug("Resmgr: Detected SCI1.1");
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_32:
		debug("Resmgr: Couldn't determine SCI version");
		break;
#endif
	default:
		debug("Resmgr: Couldn't determine SCI version");
		break;
	}
}

ResourceManager::~ResourceManager() {
	// freeing resources
	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		itr ++;
	}
	freeResourceSources(_sources);
	_resMap.empty();

	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	while (it != _volumeFiles.end()) {
		delete *it;
		it ++;
	}
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->status != kResStatusEnqueued) {
		sciprintf("Resmgr: Oops: trying to remove resource that isn't enqueued\n");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->status != kResStatusAllocated) {
		warning("Resmgr: Oops: trying to enqueue resource with state %d", res->status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if (SCI_VERBOSE_RESMGR > 1)
	debug("Adding %s.%03d (%d bytes) to lru control: %d bytes total",
	      getResourceTypeName(res->type), res->number, res->size,
	      mgr->_memoryLRU);

#endif
	res->status = kResStatusEnqueued;
}

void ResourceManager::printLRU() {
	int mem = 0;
	int entries = 0;
	Common::List<Resource *>::iterator it = _LRU.begin();
	Resource *res;

	while (it != _LRU.end()) {
		res = *it;
		debug("\t%s: %d bytes", res->id.toString().c_str(), res->size);
		mem += res->size;
		entries ++;
		it ++;
	}

	debug("Total: %d entries, %d bytes (mgr says %d)", entries, mem, _memoryLRU);
}

void ResourceManager::freeOldResources(int last_invulnerable) {
	while (_maxMemory < _memoryLRU && (!last_invulnerable || !_LRU.empty())) {
		Resource *goner = *_LRU.reverse_begin();
		if (!goner) {
			debug("Internal error: mgr->lru_last is NULL!");
			debug("LRU-mem= %d", _memoryLRU);
			debug("lru_first = %p", (void *)*_LRU.begin());
			printLRU();
		}
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMGR
		sciprintf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
#endif
	}
}

Common::List<ResourceId> *ResourceManager::listResources(ResourceType type, int mapNumber) {
	Common::List<ResourceId> *resources = new Common::List<ResourceId>;

	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		if ((itr->_value->id.type == type) && ((mapNumber == -1) || (itr->_value->id.number == mapNumber)))
			resources->push_back(itr->_value->id);
		itr++;
	}

	return resources;
}

Resource *ResourceManager::findResource(ResourceId id, bool lock) {
	Resource *retval;

	if (id.number >= sci_max_resource_nr[_sciVersion]) {
		ResourceId moddedId = ResourceId(id.type, id.number % sci_max_resource_nr[_sciVersion], id.tuple);
		sciprintf("[resmgr] Requested invalid resource %s, mapped to %s\n",
		          id.toString().c_str(), moddedId.toString().c_str());
		id = moddedId;
	}

	retval = testResource(id);

	if (!retval)
		return NULL;

	if (retval->status == kResStatusNoMalloc)
		loadResource(retval);
	else if (retval->status == kResStatusEnqueued)
		removeFromLRU(retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	if (lock) {
		if (retval->status == kResStatusAllocated) {
			retval->status = kResStatusLocked;
			retval->lockers = 0;
			_memoryLocked += retval->size;
		}
		retval->lockers++;
	} else if (retval->status != kResStatusLocked) { // Don't lock it
		if (retval->status == kResStatusAllocated)
			addToLRU(retval);
	}

	freeOldResources(retval->status == kResStatusAllocated);

	if (retval->data)
		return retval;
	else {
		sciprintf("Resmgr: Failed to read %s\n", retval->id.toString().c_str());
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res) {
	assert(res);

	if (res->status != kResStatusLocked) {
		warning("[Resmgr] Attempt to unlock unlocked resource %s", res->id.toString().c_str());
		return;
	}

	if (!--res->lockers) { // No more lockers?
		res->status = kResStatusAllocated;
		_memoryLocked -= res->size;
		addToLRU(res);
	}

	freeOldResources(0);
}

int ResourceManager::detectMapVersion() {
	Common::File file;
	byte buff[6];
	ResourceSource *rsrc = _sources;
	// looking for extarnal map among sources
	while (rsrc) {
		if (rsrc->source_type == kSourceExtMap) {
			file.open(rsrc->location_name);
			break;
		}
		rsrc = rsrc->next;
	}
	if (file.isOpen() == false) {
		warning("Failed to open resource map file");
		return SCI_VERSION_AUTODETECT;
	}
	// detection
	// SCI0 and SCI01 maps have last 6 bytes set to FF
	file.seek(-4, SEEK_END);
	uint32 uEnd = file.readUint32LE();
	if (uEnd == 0xFFFFFFFF) {
		// check if 0 or 01 - try to read resources in SCI0 format and see if exists
		file.seek(0, SEEK_SET);
		while (file.read(buff, 6) == 6 && !(buff[0] == 0xFF && buff[1] == 0xFF && buff[2] == 0xFF)) {
			if (getVolume(rsrc, (buff[5] & 0xFC) >> 2) == NULL)
				return SCI_VERSION_01_VGA_ODD;
		}
		return SCI_VERSION_0;
	}
	// SCI1E/L and some SCI1.1 maps have last directory entry set to 0xFF
	// and offset set to filesize
	// SCI1 have 6-bytes entries, while SCI1.1 have 5-byte entries
	file.seek(1, SEEK_SET);
	uint16 off1, off = file.readUint16LE();
	uint16 nEntries  = off / 3;
	file.seek(1, SEEK_CUR);
	file.seek(off - 3, SEEK_SET);
	if (file.readByte() == 0xFF && file.readUint16LE() == file.size()) {
		file.seek(3, SEEK_SET);
		for (int i = 0; i < nEntries; i++) {
			file.seek(1, SEEK_CUR);
			off1 = file.readUint16LE();
			if ((off1 - off) % 5 && (off1 - off) % 6 == 0)
				return SCI_VERSION_1;
			if ((off1 - off) % 5 == 0 && (off1 - off) % 6)
				return SCI_VERSION_1_1;
			off = off1;
		}
		return SCI_VERSION_1;
	}

#ifdef ENABLE_SCI32
	// late SCI1.1 and SCI32 maps have last directory entry set to 0xFF
	// offset set to filesize and 4 more bytes
	file.seek(off - 7, SEEK_SET);
	if (file.readByte() == 0xFF && file.readUint16LE() == file.size())
		return SCI_VERSION_32; // TODO : check if there is a difference between these maps
#endif

	return SCI_VERSION_AUTODETECT;
}

int ResourceManager::detectVolVersion() {
	Common::File file;
	ResourceSource *rsrc = _sources;
	// looking for a volume among sources
	while (rsrc) {
		if (rsrc->source_type == kSourceVolume) {
			file.open(rsrc->location_name);
			break;
		}
		rsrc = rsrc->next;
	}
	if (file.isOpen() == false) {
		warning("Failed to open volume file");
		return SCI_VERSION_AUTODETECT;
	}
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	// Try to parse volume with SCI0 scheme to see if it make sense
	// Checking 1MB of data should be enough to determine the version
	uint16 resId, wCompression;
	uint32 dwPacked, dwUnpacked;
	int curVersion = SCI_VERSION_0;
	bool failed = false;

	// Check for SCI0, SCI1, SCI1.1 and SCI32 v2 (Gabriel Knight 1 CD) formats
	while (!file.eos() && file.pos() < 0x100000) {
		if (curVersion > SCI_VERSION_0)
			file.readByte();
		resId = file.readUint16LE();
		dwPacked = (curVersion < SCI_VERSION_32) ? file.readUint16LE() : file.readUint32LE();
		dwUnpacked = (curVersion < SCI_VERSION_32) ? file.readUint16LE() : file.readUint32LE();
		wCompression = (curVersion < SCI_VERSION_32) ? file.readUint16LE() : file.readUint32LE();
		if (file.eos())
			return curVersion;

		int chk = (curVersion == SCI_VERSION_0) ? 4 : 20;
		int offs = curVersion < SCI_VERSION_1_1 ? 4 : 0;
		if ((curVersion < SCI_VERSION_32 && wCompression > chk)
				|| (curVersion == SCI_VERSION_32 && wCompression != 0 && wCompression != 32)
				|| (wCompression == 0 && dwPacked != dwUnpacked + offs)
		        || (dwUnpacked < dwPacked - offs)) {

			// Retry with a newer SCI version
			if (curVersion == SCI_VERSION_0) {
				curVersion = SCI_VERSION_1;
			} else if (curVersion == SCI_VERSION_1) {
				curVersion = SCI_VERSION_1_1;
			} else if (curVersion == SCI_VERSION_1_1) {
				curVersion = SCI_VERSION_32;
			} else {
				// All version checks failed, exit loop
				failed = true;
				break;
			}

			file.seek(0, SEEK_SET);
			continue;
		}

		if (curVersion < SCI_VERSION_1_1)
			file.seek(dwPacked - 4, SEEK_CUR);
		else if (curVersion == SCI_VERSION_1_1)
			file.seek((9 + dwPacked) % 2 ? dwPacked + 1 : dwPacked, SEEK_CUR);
		else if (curVersion == SCI_VERSION_32)
			file.seek(dwPacked, SEEK_CUR);//(9 + wPacked) % 2 ? wPacked + 1 : wPacked, SEEK_CUR);
	}

	if (!failed)
		return curVersion;

	// Failed to detect volume version
	return SCI_VERSION_AUTODETECT;
}

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source, ResourceType restype, int resnumber) {
	Common::File file;
	Resource *newrsc;
	ResourceId resId = ResourceId(restype, resnumber);
	byte patchtype, patch_data_offset;
	int fsize;

	if (resnumber == -1)
		return;
	if (!file.open(source->location_name)) {
		perror("""__FILE__"": (""__LINE__""): failed to open");
		return;
	}
	fsize = file.size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", source->location_name.c_str());
		return;
	}

	patchtype = file.readByte() & 0x7F;
	patch_data_offset = file.readByte();

	if (patchtype != restype) {
		debug("Patching %s failed - resource type mismatch", source->location_name.c_str());
		return;
	}
	if (patch_data_offset + 2 >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
		      source->location_name.c_str(), patch_data_offset + 2, fsize);
		return;
	}
	// Prepare destination, if neccessary
	if (_resMap.contains(resId) == false) {
		// FIXME: code duplication
		switch (restype) {
		case kResourceTypeSync:
			newrsc = new ResourceSync;
			break;
		default:
			newrsc = new Resource;
			break;
		}
		_resMap.setVal(resId, newrsc);
	} else
		newrsc = _resMap.getVal(resId);
	// Overwrite everything, because we're patching
	newrsc->id = resId;
	newrsc->status = kResStatusNoMalloc;
	newrsc->source = source;
	newrsc->size = fsize - patch_data_offset - 2;
	newrsc->headerSize = patch_data_offset;
	newrsc->file_offset = 0;
	debug("Patching %s - OK", source->location_name.c_str());
}


void ResourceManager::readResourcePatches(ResourceSource *source) {
// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
// this function tries to read patch file with any supported naming scheme,
// regardless of _sciVersion value

	Common::String mask, name;
	Common::ArchiveMemberList files;
	int number;
	const char *szResType;
	ResourceSource *psrcPatch;
	bool bAdd;

	for (int i = kResourceTypeView; i < kResourceTypeAudio36; i ++) {
		files.clear();
		szResType = getResourceTypeName((ResourceType)i);
		// SCI0 naming - type.nnn
		mask = szResType;
		mask += ".???";
		SearchMan.listMatchingMembers(files, mask);
		// SCI1 and later naming - nnn.typ
		mask = "*.";
		mask += resourceTypeSuffixes[i];
		SearchMan.listMatchingMembers(files, mask);
		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); x++) {
			bAdd = false;
			name = (*x)->getName();
			// SCI1 scheme
			if (isdigit(name[0])) {
				number = atoi(name.c_str());
				bAdd = true;
			} else {
				// SCI0 scheme
				int resname_len = strlen(szResType);
				if (scumm_strnicmp(name.c_str(), szResType, resname_len) == 0
					&& !isalpha(name[resname_len + 1])) {
					number = atoi(name.c_str() + resname_len + 1);
					bAdd = true;
				}
			}

			if (bAdd) {
				psrcPatch = new ResourceSource;
				psrcPatch->source_type = kSourcePatch;
				psrcPatch->location_name = name;
				processPatch(psrcPatch, (ResourceType)i, number);
			}
		}
	}
}

int ResourceManager::readResourceMapSCI0(ResourceSource *map) {
	Common::File file;
	Resource *res;
	ResourceType type;
	uint16 number, id;
	uint32 offset;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	file.seek(0, SEEK_SET);

	byte bMask = 0xFC;
	// FIXME: The code above seems to give correct results for Jones
	//byte bMask = _mapVersion == SCI_VERSION_01_VGA_ODD ? 0xF0 : 0xFC;
	byte bShift = _mapVersion == SCI_VERSION_01_VGA_ODD ? 28 : 26;

	do {
		id = file.readUint16LE();
		offset = file.readUint32LE();

		if (file.ioFailed()) {
			warning("Error while reading %s: ", map->location_name.c_str());
			perror("");
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}
		if (offset == 0xFFFFFFFF)
			break;

		type = (ResourceType)(id >> 11);
		number = id & 0x7FF;
		ResourceId resId = ResourceId(type, number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			res = new Resource;
			res->file_offset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->id = resId;
			res->source = getVolume(map, offset >> bShift);
			_resMap.setVal(resId, res);
		}
	} while (!file.eos());
	return 0;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map) {
	Common::File file;
	Resource *res;
	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	resource_index_t resMap[32];
	memset(resMap, 0, sizeof(resource_index_t) * 32);
	byte type = 0, prevtype = 0;
	byte nEntrySize = _mapVersion == SCI_VERSION_1_1 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
	ResourceId resId;

	// Read resource type and offsets to resource offsets block from .MAP file
	// The last entry has type=0xFF (0x1F) and offset equals to map file length
	do {
		type = file.readByte() & 0x1F;
		resMap[type].wOffset = file.readUint16LE();
		resMap[prevtype].wSize = (resMap[type].wOffset
		                          - resMap[prevtype].wOffset) / nEntrySize;
		prevtype = type;
	} while (type != 0x1F); // the last entry is FF

	// reading each type's offsets
	uint32 off = 0;
	for (type = 0; type < 32; type++) {
		if (resMap[type].wOffset == 0) // this resource does not exist in map
			continue;
		file.seek(resMap[type].wOffset);
		for (int i = 0; i < resMap[type].wSize; i++) {
			uint16 number = file.readUint16LE();
			int volume_nr = 0;
			if (_mapVersion == SCI_VERSION_1_1) {
				// offset stored in 3 bytes
				off = file.readUint16LE();
				off |= file.readByte() << 16;
				off <<= 1;
			} else {
				// offset/volume stored in 4 bytes
				off = file.readUint32LE();
				if (_mapVersion < SCI_VERSION_1_1) {
					volume_nr = off >> 28; // most significant 4 bits
					off &= 0x0FFFFFFF;     // least significant 28 bits
				} else {
					// in SCI32 it's a plain offset
				}
			}
			if (file.ioFailed()) {
				warning("Error while reading %s: ", map->location_name.c_str());
				perror("");
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = ResourceId((ResourceType)type, number);
			// adding new resource only if it does not exist
			if (_resMap.contains(resId) == false) {
				switch (type) {
				case kResourceTypeSync:
					res = new ResourceSync;
					break;
				default:
					res = new Resource;
					break;
				}

				_resMap.setVal(resId, res);
				res->id = resId;
				res->source = getVolume(map, volume_nr);
				res->file_offset = off;
			}
		}
	}
	return 0;
}

void ResourceManager::addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size) {
	// Adding new resource only if it does not exist
	if (_resMap.contains(resId) == false) {
		Resource *res;
		if ((resId.type == kResourceTypeSync) || (resId.type == kResourceTypeSync36))
			res = new ResourceSync;
		else
			res = new Resource;
		_resMap.setVal(resId, res);
		res->id = resId;
		res->source = src;
		res->file_offset = offset;
		res->size = size;
	}
}

// Early SCI1.1 65535.MAP structure (uses RESOURCE.AUD):
// =========
// 6-byte entries:
// w nEntry
// dw offset

// Late SCI1.1 65535.MAP structure (uses RESOURCE.SFX):
// =========
// 5-byte entries:
// w nEntry
// tb offset (cumulative)

// Early SCI1.1 MAP structure:
// ===============
// 10-byte entries:
// b noun
// b verb
// b cond
// b seq
// dw offset
// w syncSize + syncAscSize

// Late SCI1.1 MAP structure:
// ===============
// Header:
// dw baseOffset
// Followed by 7 or 11-byte entries:
// b noun
// b verb
// b cond
// b seq
// tb cOffset (cumulative offset)
// w syncSize (iff seq has bit 7 set)
// w syncAscSize (iff seq has bit 6 set)

int ResourceManager::readMap(ResourceSource *map) {
	bool isEarly = true;
	uint32 offset = 0;
	Resource *mapRes = findResource(ResourceId(kResourceTypeMap, map->volume_number), false);

	if (!mapRes) {
		warning("Failed to open %i.MAP", map->volume_number);
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	ResourceSource *src = getVolume(map, 0);

	if (!src)
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;

	byte *ptr = mapRes->data;

	if (map->volume_number == 65535) {
		// Heuristic to detect late SCI1.1 map format
		if ((mapRes->size >= 6) && (ptr[mapRes->size - 6] != 0xff))
			isEarly = false;

		while (ptr < mapRes->data + mapRes->size) {
			uint16 n = READ_LE_UINT16(ptr);
			ptr += 2;

			if (n == 0xffff)
				break;

			if (isEarly) {
				offset = READ_LE_UINT32(ptr);
				ptr += 4;
			} else {
				offset += READ_LE_UINT24(ptr);
				ptr += 3;
			}

			addResource(ResourceId(kResourceTypeAudio, n), src, offset);
		}
	} else {
		// Heuristic to detect late SCI1.1 map format
		if ((mapRes->size >= 11) && (ptr[mapRes->size - 11] == 0xff))
			isEarly = false;

		if (!isEarly) {
			offset = READ_LE_UINT32(ptr);
			ptr += 4;
		}

		while (ptr < mapRes->data + mapRes->size) {
			uint32 n = READ_BE_UINT32(ptr);
			int syncSize = 0;
			ptr += 4;

			if (n == 0xffffffff)
				break;

			if (isEarly) {
				offset = READ_LE_UINT32(ptr);
				ptr += 4;
			} else {
				offset += READ_LE_UINT24(ptr);
				ptr += 3;
			}

			if (isEarly || (n & 0x80)) {
				syncSize = READ_LE_UINT16(ptr);
				ptr += 2;

				if (syncSize > 0)
					addResource(ResourceId(kResourceTypeSync36, map->volume_number, n & 0xffffff3f), src, offset, syncSize);
			}

			if (n & 0x40) {
				syncSize += READ_LE_UINT16(ptr);
				ptr += 2;
			}

			addResource(ResourceId(kResourceTypeAudio36, map->volume_number, n & 0xffffff3f), src, offset + syncSize);
		}
	}

	return 0;
}

int ResourceManager::readResourceInfo(Resource *res, Common::File *file,
                                      uint32&szPacked, ResourceCompression &compression) {
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	uint16 w, number, szUnpacked;
	uint32 wCompression;
	ResourceType type;

	switch (_volVersion) {
	case SCI_VERSION_0:
		w = file->readUint16LE();
		type = (ResourceType)(w >> 11);
		number = w & 0x7FF;
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case SCI_VERSION_1:
		type = (ResourceType)(file->readByte() & 0x7F);
		number = file->readUint16LE();
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case SCI_VERSION_1_1:
		type = (ResourceType)(file->readByte() & 0x7F);
		number = file->readUint16LE();
		szPacked = file->readUint16LE();
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
#ifdef ENABLE_SCI32
	case SCI_VERSION_32:
		type = (ResourceType)(file->readByte() &0x7F);
		number = file->readUint16LE();
		szPacked = file->readUint32LE();
		szUnpacked = file->readUint32LE();
		wCompression = file->readUint16LE();
		break;
#endif
	default:
		return SCI_ERROR_INVALID_RESMAP_ENTRY;
	}
	// check if there were errors while reading
	if (file->ioFailed())
		return SCI_ERROR_IO_ERROR;
	res->id = ResourceId(type, number);
	res->size = szUnpacked;
	// checking compression method
	switch (wCompression) {
	case 0:
		compression = kCompNone;
		break;
	case 1:
		compression = (_sciVersion == SCI_VERSION_0) ? kCompLZW : kCompHuffman;
		break;
	case 2:
		compression = (_sciVersion == SCI_VERSION_0) ? kCompHuffman : kCompLZW1;
		break;
	case 3:
		compression = kCompLZW1View;
		break;
	case 4:
		compression = kCompLZW1Pic;
		break;
	case 18:
	case 19:
	case 20:
		compression = kCompDCL;
		break;
#ifdef ENABLE_SCI32
	case 32:
		compression = kCompSTACpack;
		break;
#endif
	default:
		compression = kCompUnknown;
	}

	return compression == kCompUnknown ? SCI_ERROR_UNKNOWN_COMPRESSION : 0;
}

int ResourceManager::decompress(Resource *res, Common::File *file) {
	int error;
	uint32 szPacked = 0;
	ResourceCompression compression = kCompUnknown;

	// fill resource info
	error = readResourceInfo(res, file, szPacked, compression);
	if (error)
		return error;
	// getting a decompressor
	Decompressor *dec = NULL;
	switch (compression) {
	case kCompNone:
		dec = new Decompressor;
		break;
	case kCompHuffman:
		dec = new DecompressorHuffman;
		break;
	case kCompLZW:
	case kCompLZW1:
	case kCompLZW1View:
	case kCompLZW1Pic:
		dec = new DecompressorLZW(compression);
		break;
	case kCompDCL:
		dec = new DecompressorDCL;
		break;
#ifdef ENABLE_SCI32
	case kCompSTACpack:
		dec = new DecompressorLZS;
		break;
#endif
	default:
		warning("Resource %s: Compression method %d not supported", res->id.toString().c_str(), compression);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	res->data = new byte[res->size];
	res->status = kResStatusAllocated;
	error = res->data ? dec->unpack(file, res->data, szPacked, res->size) : SCI_ERROR_RESOURCE_TOO_BIG;
	if (error)
		res->unalloc();

	delete dec;
	return error;
}

void ResourceSync::startSync(EngineState *s, reg_t obj) {
	_syncTime = _syncCue = -1;
	PUT_SEL32V(obj, syncCue, 0);
	_ptr = (uint16 *)data;
	//syncStarted = true;	// not used
}

void ResourceSync::nextSync(EngineState *s, reg_t obj) {
	if (_ptr) {
		_syncTime = (int16)READ_LE_UINT16(_ptr);
		if (_syncTime == -1) {
			stopSync();
		} else {
			_syncCue = (int16)READ_LE_UINT16(_ptr + 1);
			_ptr += 2;
		}
		PUT_SEL32V(obj, syncTime, _syncTime);
		PUT_SEL32V(obj, syncCue, _syncCue);
	}
}
//--------------------------------
void ResourceSync::stopSync() {
	_ptr = 0;
	_syncCue = -1;
	//syncStarted = false;	// not used
}


AudioResource::AudioResource(ResourceManager *resMgr, int sciVersion) {
	_resMgr = resMgr;
	_sciVersion = sciVersion;
	_audioRate = 11025;
	_lang = 0;
	_audioMapSCI1 = 0;
	_audioMapSCI11 = 0;
}

AudioResource::~AudioResource() {
	if (_sciVersion < SCI_VERSION_1_1) {
		if (_audioMapSCI1) {
			delete[] _audioMapSCI1;
			_audioMapSCI1 = 0;
		}
	} else {
		if (_audioMapSCI11)
			_resMgr->unlockResource(_audioMapSCI11);
	}
}

// Used in SCI1 games
void AudioResource::setAudioLang(int16 lang) {
	if (lang != -1) {
		_lang = lang;

		char filename[40];
		sprintf(filename, "AUDIO%03d.MAP", _lang);

		Common::File* audioMapFile = new Common::File();
		if (audioMapFile->open(filename)) {
			// The audio map is freed in the destructor
			_audioMapSCI1 = new byte[audioMapFile->size()];
			audioMapFile->read(_audioMapSCI1, audioMapFile->size());
			audioMapFile->close();
			delete audioMapFile;
		} else {
			_audioMapSCI1 = 0;
		}
	}
}

int AudioResource::getAudioPosition() {
	if (g_system->getMixer()->isSoundHandleActive(_audioHandle)) {
		return g_system->getMixer()->getSoundElapsedTime(_audioHandle) * 6 / 100; // return elapsed time in ticks
	} else {
		return -1; // Sound finished
	}
}

bool AudioResource::findAudEntrySCI1(uint16 audioNumber, byte &volume, uint32 &offset, uint32 &size) {
	// AUDIO00X.MAP contains 10-byte entries:
	// w nEntry
	// dw offset+volume (as in resource.map)
	// dw size
	// ending with 10 0xFFs
	uint16 n;
	uint32 off;

	if (_audioMapSCI1 == 0)
		return false;

	byte *ptr = _audioMapSCI1;
	while ((n = READ_LE_UINT16(ptr)) != 0xFFFF) {
		if (n == audioNumber) {
			off = READ_LE_UINT32(ptr + 2);
			size = READ_LE_UINT32(ptr + 6);
			volume = off >> 28;
			offset = off & 0x0FFFFFFF;
			return true;
		}
		ptr += 10;
	}

	return false;
}

// FIXME: Move this to sound/adpcm.cpp?
// Note that the 16-bit version is also used in coktelvideo.cpp
static const uint16 tableDPCM16[128] = {
	0x0000, 0x0008, 0x0010, 0x0020, 0x0030, 0x0040, 0x0050, 0x0060, 0x0070, 0x0080,
	0x0090, 0x00A0, 0x00B0, 0x00C0, 0x00D0, 0x00E0, 0x00F0, 0x0100, 0x0110, 0x0120,
	0x0130, 0x0140, 0x0150, 0x0160, 0x0170, 0x0180, 0x0190, 0x01A0, 0x01B0, 0x01C0,
	0x01D0, 0x01E0, 0x01F0, 0x0200, 0x0208, 0x0210, 0x0218, 0x0220, 0x0228, 0x0230,
	0x0238, 0x0240, 0x0248, 0x0250, 0x0258, 0x0260, 0x0268, 0x0270, 0x0278, 0x0280,
	0x0288, 0x0290, 0x0298, 0x02A0, 0x02A8, 0x02B0, 0x02B8, 0x02C0, 0x02C8, 0x02D0,
	0x02D8, 0x02E0, 0x02E8, 0x02F0, 0x02F8, 0x0300, 0x0308, 0x0310, 0x0318, 0x0320,
	0x0328, 0x0330, 0x0338, 0x0340, 0x0348, 0x0350, 0x0358, 0x0360, 0x0368, 0x0370,
	0x0378, 0x0380, 0x0388, 0x0390, 0x0398, 0x03A0, 0x03A8, 0x03B0, 0x03B8, 0x03C0,
	0x03C8, 0x03D0, 0x03D8, 0x03E0, 0x03E8, 0x03F0, 0x03F8, 0x0400, 0x0440, 0x0480,
	0x04C0, 0x0500, 0x0540, 0x0580, 0x05C0, 0x0600, 0x0640, 0x0680, 0x06C0, 0x0700,
	0x0740, 0x0780, 0x07C0, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00,
	0x0F00, 0x1000, 0x1400, 0x1800, 0x1C00, 0x2000, 0x3000, 0x4000
};

static const byte tableDPCM8[8] = {0, 1, 2, 3, 6, 10, 15, 21};

static void deDPCM16(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int16 *out = (int16 *) soundBuf;

	int32 s = 0;
	for (uint32 i = 0; i < n; i++) {
		byte b = audioStream.readByte();
		if (b & 0x80)
			s -= tableDPCM16[b & 0x7f];
		else
			s += tableDPCM16[b];

		s = CLIP<int32>(s, -32768, 32767);
		*out++ = TO_BE_16(s);
	}
}

static void deDPCM8Nibble(byte *soundBuf, int32 &s, byte b) {
	if (b & 8)
		s -= tableDPCM8[7 - (b & 7)];
	else
		s += tableDPCM8[b & 7];
	s = CLIP<int32>(s, 0, 255);
	*soundBuf = s;
}

static void deDPCM8(byte *soundBuf, Common::SeekableReadStream &audioStream, uint32 n) {
	int32 s = 0x80;

	for (uint i = 0; i < n; i++) {
		byte b = audioStream.readByte();

		deDPCM8Nibble(soundBuf++, s, b >> 4);
		deDPCM8Nibble(soundBuf++, s, b & 0xf);
	}
}

// Sierra SOL audio file reader
// Check here for more info: http://wiki.multimedia.cx/index.php?title=Sierra_Audio
static bool readSOLHeader(Common::SeekableReadStream *audioStream, int headerSize, uint32 &size, uint16 &audioRate, byte &audioFlags) {
	if (headerSize != 11 && headerSize != 12) {
		warning("SOL audio header of size %i not supported", headerSize);
		return false;
	}

	audioStream->readUint32LE();			// skip "SOL" + 0 (4 bytes)
	audioRate = audioStream->readUint16LE();
	audioFlags = audioStream->readByte();

	size = audioStream->readUint32LE();
	return true;
}

static byte* readSOLAudio(Common::SeekableReadStream *audioStream, uint32 &size, byte audioFlags, byte &flags) {
	byte *buffer;

	// Convert the SOL stream flags to our own format
	flags = 0;
	if (audioFlags & kSolFlag16Bit)
		flags |= Audio::Mixer::FLAG_16BITS;
	if (!(audioFlags & kSolFlagIsSigned))
		flags |= Audio::Mixer::FLAG_UNSIGNED;

	if (audioFlags & kSolFlagCompressed) {
		buffer = (byte *)malloc(size * 2);

		if (audioFlags & kSolFlag16Bit)
			deDPCM16(buffer, *audioStream, size);
		else
			deDPCM8(buffer, *audioStream, size);

		size *= 2;
	} else {
		// We assume that the sound data is raw PCM
		buffer = (byte *)malloc(size);
		audioStream->read(buffer, size);
	}

	return buffer;
}

Audio::AudioStream* AudioResource::getAudioStream(uint32 audioNumber, uint32 volume, int *sampleLen) {
	Audio::AudioStream *audioStream = 0;
	uint32 offset;
	uint32 size;
	bool found = false;
	byte *data = 0;
	char filename[40];
	byte flags = 0;
	Sci::Resource* audioRes = NULL;

	// Try to load from resource manager
	if (volume == 65535)
		audioRes = _resMgr->findResource(ResourceId(kResourceTypeAudio, audioNumber), false);
	else
		audioRes = _resMgr->findResource(ResourceId(kResourceTypeAudio36, volume, audioNumber), false);

	if (audioRes) {
		if (_sciVersion < SCI_VERSION_1_1) {
			size = audioRes->size;
			data = audioRes->data;
		} else {
			byte audioFlags;

			Common::MemoryReadStream *headerStream = 
				new Common::MemoryReadStream(audioRes->header, audioRes->headerSize, false);

			if (readSOLHeader(headerStream, audioRes->headerSize, size, _audioRate, audioFlags)) {
				Common::MemoryReadStream *dataStream = 
					new Common::MemoryReadStream(audioRes->data, audioRes->size, false);
				data = readSOLAudio(dataStream, size, audioFlags, flags);
				delete dataStream;
			}
			delete headerStream;
		}

		if (data) {
			audioStream = Audio::makeLinearInputStream(data, size, _audioRate, 
											flags | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
		}
	} else {
		// Load it from the audio file
		if (_sciVersion < SCI_VERSION_1_1) {
			byte sci1Volume;
			found = findAudEntrySCI1(audioNumber, sci1Volume, offset, size);
			sprintf(filename, "AUDIO%03d.%03d", _lang, sci1Volume);
			flags |= Audio::Mixer::FLAG_UNSIGNED;
		}

		if (found) {
	#if 0
			// TODO: This tries to load directly from the KQ5CD audio file with MP3/OGG/FLAC
			// compression. Once we got a tool to compress this file AND update the map file
			// at the same time, we can use this code to play compressed audio.
			if (_sciVersion < SCI_VERSION_1_1) {
				uint32 start = offset * 1000 / _audioRate;
				uint32 duration = size * 1000 / _audioRate;
		
				// Try to load compressed
				audioStream = Audio::AudioStream::openStreamFile(filename, start, duration); 
			}
	#endif

			if (!audioStream) { 
				// Compressed file load failed, try to load original raw data
				Common::File* audioFile = new Common::File();
				if (audioFile->open(filename)) {
					audioFile->seek(offset);

					if (_sciVersion < SCI_VERSION_1_1) {
						data = (byte *)malloc(size);
						audioFile->read(data, size);
					} else {
						byte type = audioFile->readByte() & 0x7f;
						byte audioFlags;

						if (type != kResourceTypeAudio) {
							warning("Resource type mismatch");
							delete audioFile;
							return NULL;
						}

						byte headerSize = audioFile->readByte();

						if (readSOLHeader(audioFile, headerSize, size, _audioRate, audioFlags))
							data = readSOLAudio(audioFile, size, audioFlags, flags);

						if (!data) {
							delete audioFile;
							return NULL;
						}
					}

					audioFile->close();

					if (data) {
						audioStream = Audio::makeLinearInputStream(data, size, _audioRate,
														flags | Audio::Mixer::FLAG_AUTOFREE, 0, 0);
					}
				}

				delete audioFile;
			}
		} else {
			warning("Failed to find audio entry (%i, %i, %i, %i, %i)", volume, (audioNumber >> 24) & 0xff,
					(audioNumber >> 16) & 0xff, (audioNumber >> 8) & 0xff, audioNumber & 0xff);
		}
	}

	if (audioStream) {
		*sampleLen = (flags & Audio::Mixer::FLAG_16BITS ? size >> 1 : size) * 60 / _audioRate;
		return audioStream;
	}

	return NULL;
}

} // End of namespace Sci
