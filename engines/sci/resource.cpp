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

//#define SCI_VERBOSE_resourceManager 1

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

//-- resourceManager helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const char *file_name) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = file_name;
	newsrc->resourceFile = 0;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addExternalMap(const Common::FSNode *mapFile) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = mapFile->getName();
	newsrc->resourceFile = mapFile;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addSource(ResourceSource *map, ResSourceType type, const char *filename, int number) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = type;
	newsrc->scanned = false;
	newsrc->location_name = filename;
	newsrc->resourceFile = 0;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addSource(ResourceSource *map, ResSourceType type, const Common::FSNode *resFile, int number) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = type;
	newsrc->scanned = false;
	newsrc->location_name = resFile->getName();
	newsrc->resourceFile = resFile;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addPatchDir(const char *dirname) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceDirectory;
	newsrc->scanned = false;
	newsrc->location_name = dirname;

	_sources.push_back(newsrc);
	return 0;
}

ResourceSource *ResourceManager::getVolume(ResourceSource *map, int volume_nr) {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *src = *it;
		if ((src->source_type == kSourceVolume || src->source_type == kSourceAudioVolume)
			&& src->associated_map == map && src->volume_number == volume_nr)
			return src;
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

bool ResourceManager::loadFromAudioVolumeSCI11(Resource *res, Common::File &file) {
	ResourceType type = (ResourceType)(file.readByte() & 0x7f);
	if (((res->id.type == kResourceTypeAudio || res->id.type == kResourceTypeAudio36) && (type != kResourceTypeAudio))
		|| ((res->id.type == kResourceTypeSync || res->id.type == kResourceTypeSync36) && (type != kResourceTypeSync))) {
		warning("Resource type mismatch loading %s from %s", res->id.toString().c_str(), file.getName());
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

bool ResourceManager::loadFromAudioVolumeSCI1(Resource *res, Common::File &file) {
	res->data = new byte[res->size];

	if (res->data == NULL) {
		error("Can't allocate %d bytes needed for loading %s", res->size, res->id.toString().c_str());
	}

	unsigned int really_read = file.read(res->data, res->size);
	if (really_read != res->size)
		warning("Read %d bytes from %s but expected %d", really_read, res->id.toString().c_str(), res->size);

	res->status = kResStatusAllocated;
	return true;
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

	// Either loading from volume or patch loading failed
	file = getVolumeFile(res->source->location_name.c_str());
	if (!file) {
		warning("Failed to open %s", res->source->location_name.c_str());
		res->unalloc();
		return;
	}
	file->seek(res->file_offset, SEEK_SET);

	if (res->source->source_type == kSourceAudioVolume) {
		if (_sciVersion < SCI_VERSION_1_1)
			loadFromAudioVolumeSCI1(res, *file);
		else
			loadFromAudioVolumeSCI11(res, *file);
	} else {
		int error = decompress(res, file);
		if (error) {
			warning("Error %d occured while reading %s from resource file: %s",
				    error, res->id.toString().c_str(), sci_error_types[error]);
			res->unalloc();
		}
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

int ResourceManager::addAppropriateSources() {
	ResourceSource *map;

	if (Common::File::exists("RESOURCE.MAP"))
		map = addExternalMap("RESOURCE.MAP");
#ifdef ENABLE_SCI32
	else if (Common::File::exists("RESMAP.000"))
		map = addExternalMap("RESMAP.000");
	else if (Common::File::exists("RESMAP.001"))
		map = addExternalMap("RESMAP.001");
#endif
	else
		return 0;
	

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "RESOURCE.0??");
	
#ifdef ENABLE_SCI32
	SearchMan.listMatchingMembers(files, "RESSCI.0??");
#endif

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addSource(map, kSourceVolume, name.c_str(), number);
	}
	addPatchDir(".");
	if (Common::File::exists("MESSAGE.MAP"))
		addSource(addExternalMap("MESSAGE.MAP"), kSourceVolume, "RESOURCE.MSG", 0);
	return 1;
}

int ResourceManager::addAppropriateSources(const Common::FSList &fslist) {
	ResourceSource *map = 0;

	// First, find resource.map
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map") || filename.contains("resmap.000")) {
			map = addExternalMap(file);
			break;
		}
	}

	if (!map)
		return 0;

	// Now find all the resource.0?? files
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.0")	|| filename.contains("ressci.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(map, kSourceVolume, file, number);
		}
	}

	// This function is only called by the advanced detector, and we don't really need
	// to add a patch directory or message.map here

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

void ResourceManager::scanNewSources() {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *source = *it;

		if (!source->scanned) {
			source->scanned = true;
			switch (source->source_type) {
			case kSourceDirectory:
				readResourcePatches(source);
				break;
			case kSourceExtMap:
				if (_mapVersion < kResVersionSci1Late)
					readResourceMapSCI0(source);
				else
					readResourceMapSCI1(source);
				break;
			case kSourceExtAudioMap:
				readAudioMapSCI1(source);
				break;
			case kSourceIntMap:
				readAudioMapSCI11(source);
				break;
			default:
				break;
			}
		}
	}
}

void ResourceManager::freeResourceSources() {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it)
		delete *it;

	_sources.clear();
}

ResourceManager::ResourceManager() {
	addAppropriateSources();
	init();
}

ResourceManager::ResourceManager(const Common::FSList &fslist) {
	addAppropriateSources(fslist);
	init();
}

void ResourceManager::init() {
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_audioMapSCI1 = NULL;

	// FIXME: put this in an Init() function, so that we can error out if detection fails completely

	_mapVersion = detectMapVersion();
	_volVersion = detectVolVersion();
	if ((_volVersion == kResVersionUnknown) && (_mapVersion != kResVersionUnknown)) {
		warning("Volume version not detected, but map version has been detected. Setting volume version to map version");
		_volVersion = _mapVersion;
	}

	if ((_mapVersion == kResVersionUnknown) && (_volVersion != kResVersionUnknown)) {
		warning("Map version not detected, but volume version has been detected. Setting map version to volume version");
		_mapVersion = _volVersion;
	}

	debug("resourceManager: Detected resource map version %d: %s", _mapVersion, versionDescription(_mapVersion));
	debug("resourceManager: Detected volume version %d: %s", _volVersion, versionDescription(_volVersion));

	scanNewSources();
	addInternalSources();
	scanNewSources();

	_sciVersion = detectSciVersion();

	if (_sciVersion != SCI_VERSION_AUTODETECT)
		debug("resourceManager: Detected %s", ((SciEngine *)g_engine)->getSciVersionDesc(_sciVersion).c_str());
	else
		warning("resourceManager: Couldn't determine SCI version");

	switch (_viewType) {
	case kViewEga:
		debug("resourceManager: Detected EGA graphic resources");
		break;
	case kViewVga:
		debug("resourceManager: Detected VGA graphic resources");
		break;
	case kViewVga11:
		debug("resourceManager: Detected SCI1.1 VGA graphic resources");
		break;
	case kViewAmiga:
		debug("resourceManager: Detected Amiga graphic resources");
		break;
	default:
		warning("resourceManager: Couldn't determine view type");
	}
}

ResourceManager::~ResourceManager() {
	// freeing resources
	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		itr ++;
	}
	freeResourceSources();
	_resMap.empty();

	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	while (it != _volumeFiles.end()) {
		delete *it;
		it ++;
	}
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->status != kResStatusEnqueued) {
		warning("resourceManager: trying to remove resource that isn't enqueued");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->status != kResStatusAllocated) {
		warning("resourceManager: trying to enqueue resource with state %d", res->status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if SCI_VERBOSE_resourceManager
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

void ResourceManager::freeOldResources() {
	while (MAX_MEMORY < _memoryLRU) {
		assert(!_LRU.empty());
		Resource *goner = *_LRU.reverse_begin();
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_resourceManager
		printf("resourceManager-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
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
	Resource *retval = testResource(id);

	if (!retval)
		return NULL;

	if (retval->status == kResStatusNoMalloc)
		loadResource(retval);
	else if (retval->status == kResStatusEnqueued)
		removeFromLRU(retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	freeOldResources();

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

	if (retval->data)
		return retval;
	else {
		warning("resourceManager: Failed to read %s", retval->id.toString().c_str());
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res) {
	assert(res);

	if (res->status != kResStatusLocked) {
		warning("[resourceManager] Attempt to unlock unlocked resource %s", res->id.toString().c_str());
		return;
	}

	if (!--res->lockers) { // No more lockers?
		res->status = kResStatusAllocated;
		_memoryLocked -= res->size;
		addToLRU(res);
	}

	freeOldResources();
}

const char *ResourceManager::versionDescription(ResVersion version) const {
	switch (version) {
	case kResVersionUnknown:
		return "Unknown";
	case kResVersionSci0Sci1Early:
		return "SCI0 / Early SCI1";
	case kResVersionSci1Middle:
		return "Middle SCI1";
	case kResVersionSci1Late:
		return "Late SCI1";
	case kResVersionSci11:
		return "SCI1.1";
	case kResVersionSci32:
		return "SCI32";
	}

	return "Version not valid";
}

ResourceManager::ResVersion ResourceManager::detectMapVersion() {
	Common::SeekableReadStream *fileStream = 0;
	Common::File *file = 0;
	byte buff[6];
	ResourceSource *rsrc= 0;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceExtMap) {
			if (rsrc->resourceFile) {
				fileStream = rsrc->resourceFile->createReadStream();
			} else {
				file = new Common::File();
				file->open(rsrc->location_name);
				if (file->isOpen())
					fileStream = file;
			}
			break;
		}
	}

	if (!fileStream)
		error("Failed to open resource map file");

	// detection
	// SCI0 and SCI01 maps have last 6 bytes set to FF
	fileStream->seek(-4, SEEK_END);
	uint32 uEnd = fileStream->readUint32LE();
	if (uEnd == 0xFFFFFFFF) {
		// check if 0 or 01 - try to read resources in SCI0 format and see if exists
		fileStream->seek(0, SEEK_SET);
		while (fileStream->read(buff, 6) == 6 && !(buff[0] == 0xFF && buff[1] == 0xFF && buff[2] == 0xFF)) {
			if (getVolume(rsrc, (buff[5] & 0xFC) >> 2) == NULL)
				return kResVersionSci1Middle;
		}
		return kResVersionSci0Sci1Early;
	}

	// SCI1 and SCI1.1 maps consist of a fixed 3-byte header, a directory list (3-bytes each) that has one entry
	// of id FFh and points to EOF. The actual entries have 6-bytes on SCI1 and 5-bytes on SCI1.1
	byte directoryType = 0;
	uint16 directoryOffset = 0;
	uint16 lastDirectoryOffset = 0;
	uint16 directorySize = 0;
	ResVersion mapDetected = kResVersionUnknown;
	fileStream->seek(0, SEEK_SET);

	while (!fileStream->eos()) {
		directoryType = fileStream->readByte();
		directoryOffset = fileStream->readUint16LE();

		// Only SCI32 has directory type < 0x80
		if (directoryType < 0x80 && (mapDetected == kResVersionUnknown || mapDetected == kResVersionSci32))
			mapDetected = kResVersionSci32;
		else if (directoryType < 0x80 || ((directoryType & 0x7f) > 0x20 && directoryType != 0xFF))
			break;
		
		// Offset is above file size? -> definitely not SCI1/SCI1.1
		if (directoryOffset > fileStream->size())
			break;

		if (lastDirectoryOffset && mapDetected == kResVersionUnknown) {
			directorySize = directoryOffset - lastDirectoryOffset;
			if ((directorySize % 5) && (directorySize % 6 == 0))
				mapDetected = kResVersionSci1Late;
			if ((directorySize % 5 == 0) && (directorySize % 6))
				mapDetected = kResVersionSci11;
		}
		
		if (directoryType == 0xFF) {
			// FFh entry needs to point to EOF
			if (directoryOffset != fileStream->size())
				break;

			delete fileStream;

			if (mapDetected)
				return mapDetected;
			return kResVersionSci1Late;
		}

		lastDirectoryOffset = directoryOffset;
	}

	delete fileStream;

	return kResVersionUnknown;
}

ResourceManager::ResVersion ResourceManager::detectVolVersion() {
	Common::SeekableReadStream *fileStream = 0;
	Common::File *file = 0;
	ResourceSource *rsrc;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceVolume) {
			if (rsrc->resourceFile) {
				fileStream = rsrc->resourceFile->createReadStream();
			} else {
				file = new Common::File();
				file->open(rsrc->location_name);
				if (file->isOpen())
					fileStream = file;
			}
			break;
		}
	}
	if (!fileStream) {
		error("Failed to open volume file");
		return kResVersionUnknown;
	}

	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	// Try to parse volume with SCI0 scheme to see if it make sense
	// Checking 1MB of data should be enough to determine the version
	uint16 resId, wCompression;
	uint32 dwPacked, dwUnpacked;
	ResVersion curVersion = kResVersionSci0Sci1Early;
	bool failed = false;

	// Check for SCI0, SCI1, SCI1.1 and SCI32 v2 (Gabriel Knight 1 CD) formats
	while (!fileStream->eos() && fileStream->pos() < 0x100000) {
		if (curVersion > kResVersionSci0Sci1Early)
			fileStream->readByte();
		resId = fileStream->readUint16LE();
		dwPacked = (curVersion < kResVersionSci32) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		dwUnpacked = (curVersion < kResVersionSci32) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		wCompression = (curVersion < kResVersionSci32) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		if (fileStream->eos()) {
			delete fileStream;
			return curVersion;
		}

		int chk = (curVersion == kResVersionSci0Sci1Early) ? 4 : 20;
		int offs = curVersion < kResVersionSci11 ? 4 : 0;
		if ((curVersion < kResVersionSci32 && wCompression > chk)
				|| (curVersion == kResVersionSci32 && wCompression != 0 && wCompression != 32)
				|| (wCompression == 0 && dwPacked != dwUnpacked + offs)
		        || (dwUnpacked < dwPacked - offs)) {

			// Retry with a newer SCI version
			if (curVersion == kResVersionSci0Sci1Early) {
				curVersion = kResVersionSci1Late;
			} else if (curVersion == kResVersionSci1Late) {
				curVersion = kResVersionSci11;
			} else if (curVersion == kResVersionSci11) {
				curVersion = kResVersionSci32;
			} else {
				// All version checks failed, exit loop
				failed = true;
				break;
			}

			fileStream->seek(0);
			continue;
		}

		if (curVersion < kResVersionSci11)
			fileStream->seek(dwPacked - 4, SEEK_CUR);
		else if (curVersion == kResVersionSci11)
			fileStream->seek((9 + dwPacked) % 2 ? dwPacked + 1 : dwPacked, SEEK_CUR);
		else if (curVersion == kResVersionSci32)
			fileStream->seek(dwPacked - 2, SEEK_CUR);
	}

	delete fileStream;

	if (!failed)
		return curVersion;

	// Failed to detect volume version
	return kResVersionUnknown;
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
		warning("ResourceManager::processPatch(): failed to open %s", source->location_name.c_str());
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

	// Fixes SQ5/German, patch file special case logic taken from SCI View disassembly
	if (patch_data_offset & 0x80) {
		switch (patch_data_offset & 0x7F) {
			case 0:
				patch_data_offset = 24;
				break;
			case 1:
				patch_data_offset = 2;
				break;
			default:
				warning("Resource patch unsupported special case %X", patch_data_offset);
		}
	}

	if (patch_data_offset + 2 >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
		      source->location_name.c_str(), patch_data_offset + 2, fsize);
		return;
	}
	// Prepare destination, if neccessary
	if (_resMap.contains(resId) == false) {
		newrsc = new Resource;
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

	byte bMask = (_mapVersion == kResVersionSci1Middle) ? 0xF0 : 0xFC;
	byte bShift = (_mapVersion == kResVersionSci1Middle) ? 28 : 26;

	do {
		id = file.readUint16LE();
		offset = file.readUint32LE();

		if (file.ioFailed()) {
			warning("Error while reading %s", map->location_name.c_str());
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
			res->source = getVolume(map, offset >> bShift);
			if (!res->source) {
				warning("Could not get volume for resource %d, VolumeID %d", id, offset >> bShift);
				if (_mapVersion != _volVersion) {
					warning("Retrying with the detected volume version instead");
					warning("Map version was: %d, retrying with: %d", _mapVersion, _volVersion);
					_mapVersion = _volVersion;
					bMask = (_mapVersion == kResVersionSci1Middle) ? 0xF0 : 0xFC;
					bShift = (_mapVersion == kResVersionSci1Middle) ? 28 : 26;
					res->source = getVolume(map, offset >> bShift);
				}
			}
			res->file_offset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->id = resId;
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
	byte nEntrySize = _mapVersion == kResVersionSci11 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
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
			if (_mapVersion == kResVersionSci11) {
				// offset stored in 3 bytes
				off = file.readUint16LE();
				off |= file.readByte() << 16;
				off <<= 1;
			} else {
				// offset/volume stored in 4 bytes
				off = file.readUint32LE();
				if (_mapVersion < kResVersionSci11) {
					volume_nr = off >> 28; // most significant 4 bits
					off &= 0x0FFFFFFF;     // least significant 28 bits
				} else {
					// in SCI32 it's a plain offset
				}
			}
			if (file.ioFailed()) {
				warning("Error while reading %s", map->location_name.c_str());
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = ResourceId((ResourceType)type, number);
			// adding new resource only if it does not exist
			if (_resMap.contains(resId) == false) {
				res = new Resource;
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
		Resource *res = new Resource;
		_resMap.setVal(resId, res);
		res->id = resId;
		res->source = src;
		res->file_offset = offset;
		res->size = size;
	}
}

void ResourceManager::removeAudioResource(ResourceId resId) {
	// Remove resource, unless it was loaded from a patch
	if (_resMap.contains(resId)) {
		Resource *res = _resMap.getVal(resId);

		if (res->source->source_type == kSourceAudioVolume) {
			if (res->lockers == 0) {
				_resMap.erase(resId);
				delete res;
			} else {
				warning("Failed to remove resource %s (still in use)", resId.toString().c_str());
			}
		}
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

int ResourceManager::readAudioMapSCI11(ResourceSource *map) {
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

// AUDIOnnn.MAP contains 10-byte entries:
// w nEntry
// dw offset+volume (as in resource.map)
// dw size
// ending with 10 0xFFs

int ResourceManager::readAudioMapSCI1(ResourceSource *map, bool unload) {
	Common::File file;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	while (1) {
		uint16 n = file.readUint16LE();
		uint32 offset = file.readUint32LE();
		uint32 size = file.readUint32LE();

		if (file.ioFailed()) {
			warning("Error while reading %s", map->location_name.c_str());
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}

		if (n == 0xffff)
			break;

		byte volume_nr = offset >> 28; // most significant 4 bits
		offset &= 0x0fffffff; // least significant 28 bits

		ResourceSource *src = getVolume(map, volume_nr);

		if (src) {
			if (unload)
				removeAudioResource(ResourceId(kResourceTypeAudio, n));
			else
				addResource(ResourceId(kResourceTypeAudio, n), src, offset, size);
		} else {
			warning("Failed to find audio volume %i", volume_nr);
		}
	}

	return 0;
}

void ResourceManager::setAudioLanguage(int language) {
	if (_audioMapSCI1) {
		if (_audioMapSCI1->volume_number == language) {
			// This language is already loaded
			return;
		}

		// We already have a map loaded, so we unload it first
		readAudioMapSCI1(_audioMapSCI1, true);

		// Remove all volumes that use this map from the source list
		Common::List<ResourceSource *>::iterator it = _sources.begin();
		while (it != _sources.end()) {
			ResourceSource *src = *it;
			if (src->associated_map == _audioMapSCI1) {
				it = _sources.erase(it);
				delete src;
			} else {
				++it;
			}
		}

		// Remove the map itself from the source list
		_sources.remove(_audioMapSCI1);
		delete _audioMapSCI1;

		_audioMapSCI1 = NULL;
	}

	char filename[9];
	snprintf(filename, 9, "AUDIO%03d", language);

	Common::String fullname = Common::String(filename) + ".MAP";
	if (!Common::File::exists(fullname)) {
		warning("No audio map found for language %i", language);
		return;
	}

	_audioMapSCI1 = addSource(NULL, kSourceExtAudioMap, fullname.c_str(), language);

	// Search for audio volumes for this language and add them to the source list
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, Common::String(filename) + ".0??");
	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addSource(_audioMapSCI1, kSourceAudioVolume, name.c_str(), number);
	}

	scanNewSources();
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
	case kResVersionSci0Sci1Early:
	case kResVersionSci1Middle:
		w = file->readUint16LE();
		type = (ResourceType)(w >> 11);
		number = w & 0x7FF;
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci1Late:
		type = (ResourceType)(file->readByte() & 0x7F);
		number = file->readUint16LE();
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci11:
		type = (ResourceType)(file->readByte() & 0x7F);
		number = file->readUint16LE();
		szPacked = file->readUint16LE();
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
#ifdef ENABLE_SCI32
	case kResVersionSci32:
		type = (ResourceType)(file->readByte() & 0x7F);
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
		compression = (_sciVersion <= SCI_VERSION_01) ? kCompLZW : kCompHuffman;
		break;
	case 2:
		compression = (_sciVersion <= SCI_VERSION_01) ? kCompHuffman : kCompLZW1;
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

ResourceCompression ResourceManager::getViewCompression() {
	int viewsTested = 0;

	// Test 10 views to see if any are compressed
	for (int i = 0; i < 1000; i++) {
		Common::File *file;
		Resource *res = testResource(ResourceId(kResourceTypeView, i));

		if (!res)
			continue;

		if (res->source->source_type != kSourceVolume)
			continue;

		file = getVolumeFile(res->source->location_name.c_str());
		if (!file)
			continue;
		file->seek(res->file_offset, SEEK_SET);

		uint32 szPacked;
		ResourceCompression compression;

		if (readResourceInfo(res, file, szPacked, compression))
			continue;

		if (compression != kCompNone)
			return compression;

		if (++viewsTested == 10)
			break;
	}

	return kCompNone;
}

ViewType ResourceManager::detectViewType() {
	for (int i = 0; i < 1000; i++) {
		Resource *res = findResource(ResourceId(kResourceTypeView, i), 0);

		if (res) {
			switch(res->data[1]) {
			case 128:
				// If the 2nd byte is 128, it's a VGA game
				return kViewVga;
			case 0:
				// EGA or Amiga, try to read as Amiga view

				if (res->size < 10)
					return kViewUnknown;

				// Read offset of first loop
				uint16 offset = READ_LE_UINT16(res->data + 8);

				if (offset + 6U >= res->size)
					return kViewUnknown;

				// Read offset of first cel
				offset = READ_LE_UINT16(res->data + offset + 4);

				if (offset + 4U >= res->size)
					return kViewUnknown;

				// Check palette offset, amiga views have no palette
				if (READ_LE_UINT16(res->data + 6) != 0)
					return kViewEga;

				uint16 width = READ_LE_UINT16(res->data + offset);
				offset += 2;
				uint16 height = READ_LE_UINT16(res->data + offset);
				offset += 6;

				// Check that the RLE data stays within bounds
				int y;
				for (y = 0; y < height; y++) {
					int x = 0;

					while ((x < width) && (offset < res->size)) {
						byte op = res->data[offset++];
						x += (op & 0x07) ? op & 0x07 : op >> 3;
					}

					// Make sure we got exactly the right number of pixels for this row
					if (x != width)
						return kViewEga;
				}

				return kViewAmiga;
			}
		}
	}

	warning("resourceManager: Couldn't find any views");
	return kViewUnknown;
}

SciVersion ResourceManager::detectSciVersion() {
	// We use the view compression to set a preliminary _sciVersion for the sake of getResourceInfo
	// Pretend we have a SCI0 game
	_sciVersion = SCI_VERSION_0_EARLY;
	bool oldDecompressors = true;

	ResourceCompression viewCompression = getViewCompression();
	if (viewCompression != kCompLZW) {
		// If it's a different compression type from kCompLZW, the game is probably
		// SCI_VERSION_1_EGA or later. If the views are uncompressed, it is
		// likely not an early disk game.
		_sciVersion = SCI_VERSION_1_EGA;
		oldDecompressors = false;
	}

	// Set view type
	if (viewCompression == kCompDCL 
#ifdef ENABLE_SCI32
		|| viewCompression == kCompSTACpack
#endif
		) {
		// SCI1.1 VGA views
		_viewType = kViewVga11;
	} else {
		// Otherwise we detect it from a view
		_viewType = detectViewType();
	}
	
	// Handle SCI32 versions here
	if (_volVersion == kResVersionSci32) {
		// SCI2.1/3 and SCI1 Late resource maps are the same, except that
		// SCI1 Late resource maps have the resource types or'd with
		// 0x80. We differentiate between SCI2 and SCI2.1/3 based on that.
		// TODO: Differentiate between SCI2.1 and SCI3
		if (_mapVersion == kResVersionSci1Late)
			return SCI_VERSION_2;
		else
			return SCI_VERSION_2_1;
	}

	switch (_mapVersion) {
	case kResVersionSci0Sci1Early:
		if (_viewType == kViewVga) {
			// VGA
			return SCI_VERSION_1_EARLY;
		}

		// EGA
		if (hasOldScriptHeader())
			return SCI_VERSION_0_EARLY;

		if (oldDecompressors) {
			// It's either SCI_VERSION_0_LATE or SCI_VERSION_01

			// We first check for SCI1 vocab.999
			if (testResource(ResourceId(kResourceTypeVocab, 999))) {
				if (hasSci0Voc999()) {
					return SCI_VERSION_0_LATE;
				} else {
					return SCI_VERSION_01;
				}
			}

			// If vocab.999 is missing, we try vocab.900
			if (testResource(ResourceId(kResourceTypeVocab, 900))) {
				if (hasSci1Voc900()) {
					return SCI_VERSION_01;
				} else {
					return SCI_VERSION_0_LATE;
				}
			}

			warning("Failed to accurately determine SCI version");
			// No parser, we assume SCI_VERSION_01.
			return SCI_VERSION_01;
		}

		// New decompressors. It's either SCI_VERSION_1_EGA or SCI_VERSION_1_EARLY.
		if (hasSci1Voc900())
			return SCI_VERSION_1_EGA;

		// SCI_VERSION_1_EARLY EGA versions seem to be lacking a valid vocab.900.
		// If this turns out to be unreliable, we could do some pic resource checks instead.
		return SCI_VERSION_1_EARLY;
	case kResVersionSci1Middle:
		return SCI_VERSION_1_MIDDLE;
	case kResVersionSci1Late:
		if (_viewType == kViewVga11) {
			// SCI1.1 resources, assume SCI1.1
			return SCI_VERSION_1_1;
		} 
		return SCI_VERSION_1_LATE;
	case kResVersionSci11:
		return SCI_VERSION_1_1;
	default:
		return SCI_VERSION_AUTODETECT;
	}
}

// Functions below are based on PD code by Brian Provinciano (SCI Studio)
bool ResourceManager::hasOldScriptHeader() {
	Resource *res = findResource(ResourceId(kResourceTypeScript, 0), 0);

	if (!res) {
		warning("resourceManager: Failed to find script.000");
		return false;
	}

	uint offset = 2;
	const int objTypes = 17;

	while (offset < res->size) {
		uint16 objType = READ_LE_UINT16(res->data + offset);

		if (!objType) {
			offset += 2;
			// We should be at the end of the resource now
			return offset == res->size;
		}

		if (objType >= objTypes) {
			// Invalid objType
			return false;
		}

		int skip = READ_LE_UINT16(res->data + offset + 2);

		if (skip < 2) {
			// Invalid size
			return false;
		}

		offset += skip;
	}

	return false;
}

bool ResourceManager::hasSci0Voc999() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 999), 0);

	if (!res) {
		// No vocab present, possibly a demo version
		return false;
	}

	if (res->size < 2)
		return false;

	uint16 count = READ_LE_UINT16(res->data);

	// Make sure there's enough room for the pointers
	if (res->size < (uint)count * 2)
		return false;

	// Iterate over all pointers
	for (uint i = 0; i < count; i++) {
		// Offset to string
		uint16 offset = READ_LE_UINT16(res->data + 2 + count * 2);

		// Look for end of string
		do {
			if (offset >= res->size) {
				// Out of bounds
				return false;
			}
		} while (res->data[offset++]);
	}

	return true;
}

bool ResourceManager::hasSci1Voc900() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 900), 0);

	if (!res )
		return false;

	if (res->size < 0x1fe)
		return false;

	uint16 offset = 0x1fe;

	while (offset < res->size) {
		offset++;
		do {
			if (offset >= res->size) {
				// Out of bounds;
				return false;
			}
		} while (res->data[offset++]);
		offset += 3;
	}

	return offset == res->size;
}

} // End of namespace Sci
