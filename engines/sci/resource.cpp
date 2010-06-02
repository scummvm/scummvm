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

#include "common/file.h"

#include "sci/resource.h"
#include "sci/util.h"

namespace Sci {

enum {
	SCI0_RESMAP_ENTRIES_SIZE = 6,
	SCI1_RESMAP_ENTRIES_SIZE = 6,
	SCI11_RESMAP_ENTRIES_SIZE = 5
};

/** resource type for SCI1 resource.map file */
struct resource_index_t {
	uint16 wOffset;
	uint16 wSize;
};

//////////////////////////////////////////////////////////////////////

static SciVersion s_sciVersion = SCI_VERSION_NONE;	// FIXME: Move this inside a suitable class, e.g. SciEngine

SciVersion getSciVersion() {
	assert(s_sciVersion != SCI_VERSION_NONE);
	return s_sciVersion;
}

const char *getSciVersionDesc(SciVersion version) {
	switch (version) {
	case SCI_VERSION_NONE:
		return "Invalid SCI version";
	case SCI_VERSION_0_EARLY:
		return "Early SCI0";
	case SCI_VERSION_0_LATE:
		return "Late SCI0";
	case SCI_VERSION_01:
		return "SCI01";
	case SCI_VERSION_1_EGA:
		return "SCI1 EGA";
	case SCI_VERSION_1_EARLY:
		return "Early SCI1";
	case SCI_VERSION_1_MIDDLE:
		return "Middle SCI1";
	case SCI_VERSION_1_LATE:
		return "Late SCI1";
	case SCI_VERSION_1_1:
		return "SCI1.1";
	case SCI_VERSION_2:
		return "SCI2";
	case SCI_VERSION_2_1:
		return "SCI2.1";
	case SCI_VERSION_3:
		return "SCI3";
	default:
		return "Unknown";
	}
}

//////////////////////////////////////////////////////////////////////


#undef SCI_REQUIRE_RESOURCE_FILES

//#define SCI_VERBOSE_resMan 1

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
	"audio36", "sync36", "", "", "robot"
};

static const char *resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	"   ", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep", "aud", "syn",
	"trn", "   ", "rbt"
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
	_fileOffset = 0;
	_status = kResStatusNoMalloc;
	_lockers = 0;
	_source = NULL;
	_header = NULL;
	_headerSize = 0;
}

Resource::~Resource() {
	delete[] data;
	if (_source && _source->source_type == kSourcePatch)
		delete _source;
}

void Resource::unalloc() {
	delete[] data;
	data = NULL;
	_status = kResStatusNoMalloc;
}

void Resource::writeToStream(Common::WriteStream *stream) const {
	stream->writeByte(_id.type | 0x80); // 0x80 is required by old sierra sci, otherwise it wont accept the patch file
	stream->writeByte(_headerSize);
	if (_headerSize > 0)
		stream->write(_header, _headerSize);
	stream->write(data, size);
}

uint32 Resource::getAudioCompressionType() {
	return _source->audioCompressionType;
}

//-- resMan helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const char *file_name, int volume_nr) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = file_name;
	newsrc->resourceFile = 0;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;
	newsrc->volume_number = volume_nr;

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addExternalMap(const Common::FSNode *mapFile, int volume_nr) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = mapFile->getName();
	newsrc->resourceFile = mapFile;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;
	newsrc->volume_number = volume_nr;

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
	newsrc->audioCompressionType = 0;
	newsrc->audioCompressionOffsetMapping = NULL;
	if (type == kSourceAudioVolume)
		checkIfAudioVolumeIsCompressed(newsrc);

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
	newsrc->audioCompressionType = 0;
	newsrc->audioCompressionOffsetMapping = NULL;
	if (type == kSourceAudioVolume)
		checkIfAudioVolumeIsCompressed(newsrc);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addPatchDir(const char *dirname) {
	ResourceSource *newsrc = new ResourceSource();

	newsrc->source_type = kSourceDirectory;
	newsrc->resourceFile = 0;
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

bool ResourceManager::loadPatch(Resource *res, Common::SeekableReadStream *file) {
	// We assume that the resource type matches res->type
	//  We also assume that the current file position is right at the actual data (behind resourceid/headersize byte)

	res->data = new byte[res->size];

	if (res->_headerSize > 0)
		res->_header = new byte[res->_headerSize];

	if ((res->data == NULL) || ((res->_headerSize > 0) && (res->_header == NULL))) {
		error("Can't allocate %d bytes needed for loading %s", res->size + res->_headerSize, res->_id.toString().c_str());
	}

	unsigned int really_read;
	if (res->_headerSize > 0) {
		really_read = file->read(res->_header, res->_headerSize);
		if (really_read != res->_headerSize)
			error("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->_headerSize);
	}

	really_read = file->read(res->data, res->size);
	if (really_read != res->size)
		error("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->size);

	res->_status = kResStatusAllocated;
	return true;
}

bool ResourceManager::loadFromPatchFile(Resource *res) {
	Common::File file;
	const char *filename = res->_source->location_name.c_str();
	if (file.open(filename) == false) {
		warning("Failed to open patch file %s", filename);
		res->unalloc();
		return false;
	}
	// Skip resourceid and header size byte
	file.seek(2, SEEK_SET);
	return loadPatch(res, &file);
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
		++it;
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

static uint32 resTypeToMacTag(ResourceType type);

void ResourceManager::loadResource(Resource *res) {
	if (res->_source->source_type == kSourcePatch && loadFromPatchFile(res))
		return;

	if (res->_source->source_type == kSourceMacResourceFork) {
		Common::SeekableReadStream *stream = res->_source->macResMan.getResource(resTypeToMacTag(res->_id.type), res->_id.number);

		if (!stream)
			error("Could not get Mac resource fork resource: %d %d", res->_id.type, res->_id.number);

		int error = decompress(res, stream);
		if (error) {
			warning("Error %d occured while reading %s from Mac resource file: %s",
				    error, res->_id.toString().c_str(), sci_error_types[error]);
			res->unalloc();
		}
		return;
	}

	Common::SeekableReadStream *fileStream;

	// Either loading from volume or patch loading failed
	if (res->_source->resourceFile)
		fileStream = res->_source->resourceFile->createReadStream();
	else
		fileStream = getVolumeFile(res->_source->location_name.c_str());

	if (!fileStream) {
		warning("Failed to open %s", res->_source->location_name.c_str());
		res->unalloc();
		return;
	}

	switch(res->_source->source_type) {
	case kSourceWave:
		fileStream->seek(res->_fileOffset, SEEK_SET);
		loadFromWaveFile(res, fileStream);
		return;

	case kSourceAudioVolume:
		if (res->_source->audioCompressionType) {
			// this file is compressed, so lookup our offset in the offset-translation table and get the new offset
			//  also calculate the compressed size by using the next offset
			int32 *mappingTable = res->_source->audioCompressionOffsetMapping;
			int32 compressedOffset = 0;

			do {
				if (*mappingTable == res->_fileOffset) {
					mappingTable++;
					compressedOffset = *mappingTable;
					// Go to next compressed offset and use that to calculate size of compressed sample
					switch (res->_id.type) {
					case kResourceTypeSync:
					case kResourceTypeSync36:
						// we should already have a (valid) size
						break;
					default:
						mappingTable += 2;
						res->size = *mappingTable - compressedOffset;
					}
					break;
				}
				mappingTable += 2;
			} while (*mappingTable);

			if (!compressedOffset)
				error("could not translate offset to compressed offset in audio volume");
			fileStream->seek(compressedOffset, SEEK_SET);

			switch (res->_id.type) {
			case kResourceTypeAudio:
			case kResourceTypeAudio36:
				// Directly read the stream, compressed audio wont have resource type id and header size for SCI1.1
				loadFromAudioVolumeSCI1(res, fileStream);
				return;
			default:
				break;
			}
		} else {
			// original file, directly seek to given offset and get SCI1/SCI1.1 audio resource
			fileStream->seek(res->_fileOffset, SEEK_SET);
		}
		if (getSciVersion() < SCI_VERSION_1_1)
			loadFromAudioVolumeSCI1(res, fileStream);
		else
			loadFromAudioVolumeSCI11(res, fileStream);
		return;

	default:
		fileStream->seek(res->_fileOffset, SEEK_SET);
		int error = decompress(res, fileStream);
		if (error) {
			warning("Error %d occured while reading %s from resource file: %s",
				    error, res->_id.toString().c_str(), sci_error_types[error]);
			res->unalloc();
		}
	}
}

Resource *ResourceManager::testResource(ResourceId id) {
	return _resMap.getVal(id, NULL);
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
	Common::ArchiveMemberList files;

	if (Common::File::exists("resource.map")) {
		// SCI0-SCI2 file naming scheme
		ResourceSource *map = addExternalMap("resource.map");

		SearchMan.listMatchingMembers(files, "resource.0??");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			const Common::String name = (*x)->getName();
			const char *dot = strrchr(name.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(map, kSourceVolume, name.c_str(), number);
		}
#ifdef ENABLE_SCI32
		// GK1CD hires content
		if (Common::File::exists("alt.map") && Common::File::exists("resource.alt"))
			addSource(addExternalMap("alt.map", 10), kSourceVolume, "resource.alt", 10);
#endif
	} else if (Common::File::exists("Data1")) {
		// Mac SCI1.1+ file naming scheme
		SearchMan.listMatchingMembers(files, "Data?*");

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			Common::String filename = (*x)->getName();
			addSource(0, kSourceMacResourceFork, filename.c_str(), atoi(filename.c_str() + 4));
		}
#ifdef ENABLE_SCI32
		// Mac SCI32 games have extra folders for patches
		addPatchDir("Robot Folder");
		addPatchDir("Sound Folder");
		addPatchDir("Voices Folder");
		//addPatchDir("VMD Folder");

		// There can also be a "Patches" resource fork with patches
		if (Common::File::exists("Patches"))
			addSource(0, kSourceMacResourceFork, "Patches", 100);
	} else {
		// SCI2.1-SCI3 file naming scheme
		Common::ArchiveMemberList mapFiles;
		SearchMan.listMatchingMembers(mapFiles, "resmap.0??");
		SearchMan.listMatchingMembers(files, "ressci.0??");

		// We need to have the same number of maps as resource archives
		if (mapFiles.empty() || files.empty() || mapFiles.size() != files.size())
			return 0;

		for (Common::ArchiveMemberList::const_iterator mapIterator = mapFiles.begin(); mapIterator != mapFiles.end(); ++mapIterator) {
			Common::String mapName = (*mapIterator)->getName();
			int mapNumber = atoi(strrchr(mapName.c_str(), '.') + 1);

			for (Common::ArchiveMemberList::const_iterator fileIterator = files.begin(); fileIterator != files.end(); ++fileIterator) {
				Common::String resName = (*fileIterator)->getName();
				int resNumber = atoi(strrchr(resName.c_str(), '.') + 1);

				if (mapNumber == resNumber) {
					addSource(addExternalMap(mapName.c_str(), mapNumber), kSourceVolume, resName.c_str(), mapNumber);
					break;
				}
			}
		}

		// SCI2.1 resource patches
		if (Common::File::exists("resmap.pat") && Common::File::exists("ressci.pat")) {
			// We add this resource with a map which surely won't exist
			addSource(addExternalMap("resmap.pat", 100), kSourceVolume, "ressci.pat", 100);
		}
	}
#else
	} else
		return 0;
#endif

	addPatchDir(".");
	if (Common::File::exists("message.map"))
		addSource(addExternalMap("message.map"), kSourceVolume, "resource.msg", 0);

	return 1;
}

int ResourceManager::addAppropriateSources(const Common::FSList &fslist) {
	ResourceSource *map = 0;
#ifdef ENABLE_SCI32
	ResourceSource *sci21PatchMap = 0;
	const Common::FSNode *sci21PatchRes = 0;
#endif

	// First, find resource.map
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getName();
		filename.toLowercase();

		if (filename.contains("resource.map"))
			map = addExternalMap(file);

		if (filename.contains("resmap.0")) {
			const char *dot = strrchr(file->getName().c_str(), '.');
			int number = atoi(dot + 1);
			map = addExternalMap(file, number);
		}

#ifdef ENABLE_SCI32
		// SCI2.1 resource patches
		if (filename.contains("resmap.pat"))
			sci21PatchMap = addExternalMap(file, 100);

		if (filename.contains("ressci.pat"))
			sci21PatchRes = file;
#endif
	}

	if (!map)
		return 0;

#ifdef ENABLE_SCI32
	if (sci21PatchMap && sci21PatchRes)
		addSource(sci21PatchMap, kSourceVolume, sci21PatchRes, 100);
#endif

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

		++itr;
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
				readWaveAudioPatches();
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
			case kSourceMacResourceFork:
				readMacResourceFork(source);
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

	debugC(1, kDebugLevelResMan, "resMan: Detected resource map version %d: %s", _mapVersion, versionDescription(_mapVersion));
	debugC(1, kDebugLevelResMan, "resMan: Detected volume version %d: %s", _volVersion, versionDescription(_volVersion));

	if ((_mapVersion == kResVersionUnknown) && (_volVersion == kResVersionUnknown)) {
		warning("Volume and map version not detected, assuming that this is not a sci game");
		_viewType = kViewUnknown;
		return;
	}

	scanNewSources();
	addInternalSources();
	scanNewSources();

	detectSciVersion();

	debugC(1, kDebugLevelResMan, "resMan: Detected %s", getSciVersionDesc(getSciVersion()));

	switch (_viewType) {
	case kViewEga:
		debugC(1, kDebugLevelResMan, "resMan: Detected EGA graphic resources");
		break;
	case kViewVga:
		debugC(1, kDebugLevelResMan, "resMan: Detected VGA graphic resources");
		break;
	case kViewVga11:
		debugC(1, kDebugLevelResMan, "resMan: Detected SCI1.1 VGA graphic resources");
		break;
	case kViewAmiga:
		debugC(1, kDebugLevelResMan, "resMan: Detected Amiga graphic resources");
		break;
	default:
		warning("resMan: Couldn't determine view type");
	}
}

ResourceManager::~ResourceManager() {
	// freeing resources
	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		++itr;
	}
	freeResourceSources();

	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	while (it != _volumeFiles.end()) {
		delete *it;
		++it;
	}
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->_status != kResStatusEnqueued) {
		warning("resMan: trying to remove resource that isn't enqueued");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->_status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->_status != kResStatusAllocated) {
		warning("resMan: trying to enqueue resource with state %d", res->_status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if SCI_VERBOSE_resMan
	debug("Adding %s.%03d (%d bytes) to lru control: %d bytes total",
	      getResourceTypeName(res->type), res->number, res->size,
	      mgr->_memoryLRU);
#endif
	res->_status = kResStatusEnqueued;
}

void ResourceManager::printLRU() {
	int mem = 0;
	int entries = 0;
	Common::List<Resource *>::iterator it = _LRU.begin();
	Resource *res;

	while (it != _LRU.end()) {
		res = *it;
		debug("\t%s: %d bytes", res->_id.toString().c_str(), res->size);
		mem += res->size;
		++entries;
		++it;
	}

	debug("Total: %d entries, %d bytes (mgr says %d)", entries, mem, _memoryLRU);
}

void ResourceManager::freeOldResources() {
	while (MAX_MEMORY < _memoryLRU) {
		assert(!_LRU.empty());
		Resource *goner = *_LRU.reverse_begin();
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_resMan
		printf("resMan-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
#endif
	}
}

Common::List<ResourceId> *ResourceManager::listResources(ResourceType type, int mapNumber) {
	Common::List<ResourceId> *resources = new Common::List<ResourceId>;

	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		if ((itr->_value->_id.type == type) && ((mapNumber == -1) || (itr->_value->_id.number == mapNumber)))
			resources->push_back(itr->_value->_id);
		++itr;
	}

	return resources;
}

Resource *ResourceManager::findResource(ResourceId id, bool lock) {
	Resource *retval = testResource(id);

	if (!retval)
		return NULL;

	if (retval->_status == kResStatusNoMalloc)
		loadResource(retval);
	else if (retval->_status == kResStatusEnqueued)
		removeFromLRU(retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	freeOldResources();

	if (lock) {
		if (retval->_status == kResStatusAllocated) {
			retval->_status = kResStatusLocked;
			retval->_lockers = 0;
			_memoryLocked += retval->size;
		}
		retval->_lockers++;
	} else if (retval->_status != kResStatusLocked) { // Don't lock it
		if (retval->_status == kResStatusAllocated)
			addToLRU(retval);
	}

	if (retval->data)
		return retval;
	else {
		warning("resMan: Failed to read %s", retval->_id.toString().c_str());
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res) {
	assert(res);

	if (res->_status != kResStatusLocked) {
		warning("[resMan] Attempt to unlock unlocked resource %s", res->_id.toString().c_str());
		return;
	}

	if (!--res->_lockers) { // No more lockers?
		res->_status = kResStatusAllocated;
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
	case kResVersionSci11Mac:
		return "Mac SCI1.1+";
	case kResVersionSci32:
		return "SCI32";
	}

	return "Version not valid";
}

ResourceManager::ResVersion ResourceManager::detectMapVersion() {
	Common::SeekableReadStream *fileStream = 0;
	byte buff[6];
	ResourceSource *rsrc= 0;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceExtMap) {
			if (rsrc->resourceFile) {
				fileStream = rsrc->resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->location_name);
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->source_type == kSourceMacResourceFork)
			return kResVersionSci11Mac;
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
	ResourceSource *rsrc;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceVolume) {
			if (rsrc->resourceFile) {
				fileStream = rsrc->resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->location_name);
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->source_type == kSourceMacResourceFork)
			return kResVersionSci11Mac;
	}

	if (!fileStream) {
		error("Failed to open volume file - if you got resource.p01/resource.p02/etc. files, merge them together into resource.000");
		// resource.p01/resource.p02/etc. may be there when directly copying the files from the original floppies
		// the sierra installer would merge those together (perhaps we could do this as well?)
		// possible TODO
		// example for such game: Laura Bow 2
		return kResVersionUnknown;
	}

	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format:   {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	// Try to parse volume with SCI0 scheme to see if it make sense
	// Checking 1MB of data should be enough to determine the version
	uint16 resId, wCompression;
	uint32 dwPacked, dwUnpacked;
	ResVersion curVersion = kResVersionSci0Sci1Early;
	bool failed = false;
	bool sci11Align = false;

	// Check for SCI0, SCI1, SCI1.1 and SCI32 v2 (Gabriel Knight 1 CD) formats
	while (!fileStream->eos() && fileStream->pos() < 0x100000) {
		if (curVersion > kResVersionSci0Sci1Early)
			fileStream->readByte();
		resId = fileStream->readUint16LE();
		dwPacked = (curVersion < kResVersionSci32) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		dwUnpacked = (curVersion < kResVersionSci32) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		wCompression = fileStream->readUint16LE();
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
			} else if (curVersion == kResVersionSci11 && !sci11Align) {
				// Later versions (e.g. QFG1VGA) have resources word-aligned
				sci11Align = true;
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
			fileStream->seek(sci11Align && ((9 + dwPacked) % 2) ? dwPacked + 1 : dwPacked, SEEK_CUR);
		else if (curVersion == kResVersionSci32)
			fileStream->seek(dwPacked, SEEK_CUR);
	}

	delete fileStream;

	if (!failed)
		return curVersion;

	// Failed to detect volume version
	return kResVersionUnknown;
}

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source, ResourceType restype, int resnumber) {
	Common::SeekableReadStream *fileStream = 0;
	Resource *newrsc;
	ResourceId resId = ResourceId(restype, resnumber);
	byte patchtype, patch_data_offset;
	int fsize;

	if (resnumber == -1)
		return;

	if (source->resourceFile) {
		fileStream = source->resourceFile->createReadStream();
	} else {
		Common::File *file = new Common::File();
		if (!file->open(source->location_name)) {
			warning("ResourceManager::processPatch(): failed to open %s", source->location_name.c_str());
			return;
		}
		fileStream = file;
	}
	fsize = fileStream->size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", source->location_name.c_str());
		return;
	}

	patchtype = fileStream->readByte() & 0x7F;
	patch_data_offset = fileStream->readByte();

	delete fileStream;

	if (patchtype != restype) {
		debug("Patching %s failed - resource type mismatch", source->location_name.c_str());
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
			case 4:
				patch_data_offset = 8;
				break;
			default:
				warning("Resource patch unsupported special case %X", patch_data_offset & 0x7F);
				return;
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
	newrsc->_id = resId;
	newrsc->_status = kResStatusNoMalloc;
	newrsc->_source = source;
	newrsc->size = fsize - patch_data_offset - 2;
	newrsc->_headerSize = patch_data_offset;
	newrsc->_fileOffset = 0;
	debugC(1, kDebugLevelResMan, "Patching %s - OK", source->location_name.c_str());
}


void ResourceManager::readResourcePatches(ResourceSource *source) {
	// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
	// this function tries to read patch file with any supported naming scheme,
	// regardless of s_sciVersion value

	// Note that audio36 and sync36 use a different naming scheme, because they cannot be described
	// with a single resource number, but are a result of a <number, noun, verb, cond, seq> tuple.
	// Please don't be confused with the normal audio patches (*.aud) and normal sync patches (*.syn).
	// audio36 patches can be seen for example in the AUD folder of GK1CD, and are like this file:
	// @0CS0M00.0X1. GK1CD is the first game where these have been observed. The actual audio36 and
	// sync36 resources exist in SCI1.1 as well, but the first game where external patch files for
	// them have been found is GK1CD

	Common::String mask, name;
	Common::ArchiveMemberList files;
	int number = -1;
	const char *szResType;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeView; i <= kResourceTypeRobot; ++i) {
		// TODO: add support for audio36 and sync36 files
		// Such patches were introduced in SCI2, and didn't exist in SCI0-SCI1.1
		if (i == kResourceTypeAudio36 || i == kResourceTypeSync36)
			continue;

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
		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			bool bAdd = false;
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
				psrcPatch->resourceFile = 0;
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

		if (file.eos() || file.err()) {
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
			res->_source = getVolume(map, offset >> bShift);
			if (!res->_source) {
				warning("Could not get volume for resource %d, VolumeID %d", id, offset >> bShift);
				if (_mapVersion != _volVersion) {
					warning("Retrying with the detected volume version instead");
					warning("Map version was: %d, retrying with: %d", _mapVersion, _volVersion);
					_mapVersion = _volVersion;
					bMask = (_mapVersion == kResVersionSci1Middle) ? 0xF0 : 0xFC;
					bShift = (_mapVersion == kResVersionSci1Middle) ? 28 : 26;
					res->_source = getVolume(map, offset >> bShift);
				}
			}
			res->_fileOffset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->_id = resId;
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
			if (file.eos() || file.err()) {
				warning("Error while reading %s", map->location_name.c_str());
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = ResourceId((ResourceType)type, number);
			// adding new resource only if it does not exist
			if (_resMap.contains(resId) == false) {
				res = new Resource;
				_resMap.setVal(resId, res);
				res->_id = resId;
				
				// NOTE: We add the map's volume number here to the specified volume number
				// for SCI2.1 and SCI3 maps that are not resmap.000. The resmap.* files' numbers
				// need to be used in concurrence with the volume specified in the map to get
				// the actual resource file.
				res->_source = getVolume(map, volume_nr + map->volume_number);
				res->_fileOffset = off;
			}
		}
	}
	return 0;
}

struct {
	uint32 tag;
	ResourceType type;
} static const macResTagMap[] = {
	{ MKID_BE('V56 '), kResourceTypeView },
	{ MKID_BE('P56 '), kResourceTypePic },
	{ MKID_BE('SCR '), kResourceTypeScript },
	{ MKID_BE('TEX '), kResourceTypeText },
	{ MKID_BE('SND '), kResourceTypeSound },
	{ MKID_BE('VOC '), kResourceTypeVocab },
	{ MKID_BE('FON '), kResourceTypeFont },
	{ MKID_BE('CURS'), kResourceTypeCursor },
	{ MKID_BE('crsr'), kResourceTypeCursor },
	{ MKID_BE('Pat '), kResourceTypePatch },
	{ MKID_BE('PAL '), kResourceTypePalette },
	{ MKID_BE('snd '), kResourceTypeAudio },
	{ MKID_BE('MSG '), kResourceTypeMessage },
	{ MKID_BE('HEP '), kResourceTypeHeap },
	{ MKID_BE('IBIN'), kResourceTypeMacIconBarPictN },
	{ MKID_BE('IBIS'), kResourceTypeMacIconBarPictS },
	{ MKID_BE('PICT'), kResourceTypeMacPict },
	{ MKID_BE('SYN '), kResourceTypeSync }
};

static uint32 resTypeToMacTag(ResourceType type) {
	for (uint32 i = 0; i < ARRAYSIZE(macResTagMap); i++)
		if (macResTagMap[i].type == type)
			return macResTagMap[i].tag;

	return 0;
}

int ResourceManager::readMacResourceFork(ResourceSource *source) {
	if (!source->macResMan.open(source->location_name.c_str()))
		error("%s is not a valid Mac resource fork", source->location_name.c_str());

	Common::MacResTagArray tagArray = source->macResMan.getResTagArray();

	for (uint32 i = 0; i < tagArray.size(); i++) {
		ResourceType type = kResourceTypeInvalid;

		// Map the Mac tags to our ResourceType
		for (uint32 j = 0; j < ARRAYSIZE(macResTagMap); j++)
			if (tagArray[i] == macResTagMap[j].tag) {
				type = macResTagMap[j].type;
				break;
			}

		if (type == kResourceTypeInvalid)
			continue;

		Common::MacResIDArray idArray = source->macResMan.getResIDArray(tagArray[i]);

		for (uint32 j = 0; j < idArray.size(); j++) {
			// Get the size of the file
			Common::SeekableReadStream *stream = source->macResMan.getResource(tagArray[i], idArray[j]);

			// Some IBIS resources have a size of 0, so we skip them
			if (!stream)
				continue;

			uint32 fileSize = stream->size();
			delete stream;

			ResourceId resId = ResourceId(type, idArray[j]);

			Resource *newrsc = NULL;

			// Prepare destination, if neccessary. Resource forks may contain patches.
			if (!_resMap.contains(resId)) {
				newrsc = new Resource;
				_resMap.setVal(resId, newrsc);
			} else
				newrsc = _resMap.getVal(resId);

			// Overwrite everything
			newrsc->_id = resId;
			newrsc->_status = kResStatusNoMalloc;
			newrsc->_source = source;
			newrsc->size = fileSize;
			newrsc->_headerSize = 0;
		}
	}

	return 0;
}

void ResourceManager::addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size) {
	// Adding new resource only if it does not exist
	if (_resMap.contains(resId) == false) {
		Resource *res = new Resource;
		_resMap.setVal(resId, res);
		res->_id = resId;
		res->_source = src;
		res->_fileOffset = offset;
		res->size = size;
	}
}

int ResourceManager::readResourceInfo(Resource *res, Common::SeekableReadStream *file,
                                      uint32&szPacked, ResourceCompression &compression) {
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	uint16 w, number;
	uint32 wCompression, szUnpacked;
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
	case kResVersionSci11Mac:
		// Doesn't store this data in the resource. Fortunately,
		// we already have this data.
		type = res->_id.type;
		number = res->_id.number;
		szPacked = file->size();
		szUnpacked = file->size();
		wCompression = 0;
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
	if ((file->eos() || file->err()))
		return SCI_ERROR_IO_ERROR;

	res->_id = ResourceId(type, number);
	res->size = szUnpacked;

	// checking compression method
	switch (wCompression) {
	case 0:
		compression = kCompNone;
		break;
	case 1:
		compression = (getSciVersion() <= SCI_VERSION_01) ? kCompLZW : kCompHuffman;
		break;
	case 2:
		compression = (getSciVersion() <= SCI_VERSION_01) ? kCompHuffman : kCompLZW1;
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

int ResourceManager::decompress(Resource *res, Common::SeekableReadStream *file) {
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
		warning("Resource %s: Compression method %d not supported", res->_id.toString().c_str(), compression);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	res->data = new byte[res->size];
	res->_status = kResStatusAllocated;
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

		if (res->_source->source_type != kSourceVolume)
			continue;

		file = getVolumeFile(res->_source->location_name.c_str());
		if (!file)
			continue;
		file->seek(res->_fileOffset, SEEK_SET);

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
			switch (res->data[1]) {
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

				// To improve the heuristic, we skip very small views
				if (height < 10)
					continue;

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

	warning("resMan: Couldn't find any views");
	return kViewUnknown;
}

void ResourceManager::detectSciVersion() {
	// We use the view compression to set a preliminary s_sciVersion for the sake of getResourceInfo
	// Pretend we have a SCI0 game
	s_sciVersion = SCI_VERSION_0_EARLY;
	bool oldDecompressors = true;

	ResourceCompression viewCompression = getViewCompression();
	if (viewCompression != kCompLZW) {
		// If it's a different compression type from kCompLZW, the game is probably
		// SCI_VERSION_1_EGA or later. If the views are uncompressed, it is
		// likely not an early disk game.
		s_sciVersion = SCI_VERSION_1_EGA;
		oldDecompressors = false;
	}

	// Set view type
	if (viewCompression == kCompDCL
		|| _volVersion == kResVersionSci11 // pq4demo
		|| _volVersion == kResVersionSci11Mac
#ifdef ENABLE_SCI32
		|| viewCompression == kCompSTACpack
		|| _volVersion == kResVersionSci32 // kq7
#endif
		) {
		// SCI1.1 VGA views
		_viewType = kViewVga11;
	} else {
		// Otherwise we detect it from a view
		_viewType = detectViewType();
	}
	
	if (_volVersion == kResVersionSci11Mac) {
		// SCI32 doesn't have the resource.cfg file, so we can figure out
		// which of the games are SCI1.1.
		// TODO: Decide between SCI2 and SCI2.1
		if (Common::File::exists("resource.cfg"))
			s_sciVersion = SCI_VERSION_1_1;
		else if (Common::File::exists("Patches"))
			s_sciVersion = SCI_VERSION_2_1;
		else
			s_sciVersion = SCI_VERSION_2;
		return;
	}

	// Handle SCI32 versions here
	if (_volVersion == kResVersionSci32) {
		// SCI2.1/3 and SCI1 Late resource maps are the same, except that
		// SCI1 Late resource maps have the resource types or'd with
		// 0x80. We differentiate between SCI2 and SCI2.1/3 based on that.
		// TODO: Differentiate between SCI2.1 and SCI3
		if (_mapVersion == kResVersionSci1Late) {
			s_sciVersion = SCI_VERSION_2;
			return;
		} else {
			s_sciVersion = SCI_VERSION_2_1;
			return;
		}
	}

	// Check for transitive SCI1/SCI1.1 games, like PQ1 here
	// If the game has any heap file (here we check for heap file 0), then
	// it definitely uses a SCI1.1 kernel
	if (testResource(ResourceId(kResourceTypeHeap, 0))) {
		s_sciVersion = SCI_VERSION_1_1;
		return;
	}

	switch (_mapVersion) {
	case kResVersionSci0Sci1Early:
		if (_viewType == kViewVga) {
			// VGA
			s_sciVersion = SCI_VERSION_1_EARLY;
			return;
		}

		// EGA
		if (hasOldScriptHeader()) {
			s_sciVersion = SCI_VERSION_0_EARLY;
			return;
		}

		if (oldDecompressors) {
			// It's either SCI_VERSION_0_LATE or SCI_VERSION_01

			// We first check for SCI1 vocab.999
			if (testResource(ResourceId(kResourceTypeVocab, 999))) {
				if (hasSci0Voc999()) {
					s_sciVersion = SCI_VERSION_0_LATE;
					return;
				} else {
					s_sciVersion = SCI_VERSION_01;
					return;
				}
			}

			// If vocab.999 is missing, we try vocab.900
			if (testResource(ResourceId(kResourceTypeVocab, 900))) {
				if (hasSci1Voc900()) {
					s_sciVersion = SCI_VERSION_01;
					return;
				} else {
					s_sciVersion = SCI_VERSION_0_LATE;
					return;
				}
			}

			warning("Failed to accurately determine SCI version");
			// No parser, we assume SCI_VERSION_01.
			s_sciVersion = SCI_VERSION_01;
			return;
		}

		// New decompressors. It's either SCI_VERSION_1_EGA or SCI_VERSION_1_EARLY.
		if (hasSci1Voc900()) {
			s_sciVersion = SCI_VERSION_1_EGA;
			return;
		}

		// SCI_VERSION_1_EARLY EGA versions seem to be lacking a valid vocab.900.
		// If this turns out to be unreliable, we could do some pic resource checks instead.
		s_sciVersion = SCI_VERSION_1_EARLY;
		return;
	case kResVersionSci1Middle:
		s_sciVersion = SCI_VERSION_1_MIDDLE;
		return;
	case kResVersionSci1Late:
		if (_volVersion == kResVersionSci11) {
			s_sciVersion = SCI_VERSION_1_1;
			return;
		}
		s_sciVersion = SCI_VERSION_1_LATE;
		return;
	case kResVersionSci11:
		s_sciVersion = SCI_VERSION_1_1;
		return;
	default:
		s_sciVersion = SCI_VERSION_NONE;
		error("detectSciVersion(): Unable to detect the game's SCI version");
	}
}

bool ResourceManager::detectHires() {
	// SCI 1.1 and prior is never hires
	if (getSciVersion() <= SCI_VERSION_1_1)
		return false;

#ifdef ENABLE_SCI32
	for (int i = 0; i < 32768; i++) {
		Resource *res = findResource(ResourceId(kResourceTypePic, i), 0);

		if (res) {
			if (READ_LE_UINT16(res->data) == 0x0e) {
				// SCI32 picture
				uint16 width = READ_LE_UINT16(res->data + 10);
				uint16 height = READ_LE_UINT16(res->data + 12);
				if ((width == 320) && ((height == 190) || (height == 200)))
					return false;
				if ((width >= 600) || (height >= 400))
					return true;
			}
		}
	}

	warning("resMan: Couldn't detect hires");
	return false;
#else
	error("no sci32 support");
#endif
}

bool ResourceManager::detectFontExtended() {

	Resource *res = findResource(ResourceId(kResourceTypeFont, 0), 0);
	if (res) {
		if (res->size >= 4) {
			uint16 numChars = READ_LE_UINT16(res->data + 2);
			if (numChars > 0x80)
				return true;
		}
	}
	return false;
}

// Functions below are based on PD code by Brian Provinciano (SCI Studio)
bool ResourceManager::hasOldScriptHeader() {
	Resource *res = findResource(ResourceId(kResourceTypeScript, 0), 0);

	if (!res) {
		warning("resMan: Failed to find script.000");
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

// Same function as Script::findBlock(). Slight code
// duplication here, but this has been done to keep the resource
// manager independent from the rest of the engine
static byte *findSci0ExportsBlock(byte *buffer) {
	byte *buf = buffer;
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	do {
		int seekerType = READ_LE_UINT16(buf);

		if (seekerType == 0)
			break;
		if (seekerType == 7)	// exports
			return buf;

		int seekerSize = READ_LE_UINT16(buf + 2);
		assert(seekerSize > 0);
		buf += seekerSize;
	} while (1);

	return NULL;
}

reg_t ResourceManager::findGameObject(bool addSci11ScriptOffset) {
	Resource *script = findResource(ResourceId(kResourceTypeScript, 0), false);

	if (!script)
		return NULL_REG;

	byte *offsetPtr = 0;

	if (getSciVersion() < SCI_VERSION_1_1) {
		byte *buf = (getSciVersion() == SCI_VERSION_0_EARLY) ? script->data + 2 : script->data;

		// Check if the first block is the exports block (in most cases, it is)
		bool exportsIsFirst = (READ_LE_UINT16(buf + 4) == 7);
		if (exportsIsFirst) {
			offsetPtr = buf + 4 + 2;
		} else {
			offsetPtr = findSci0ExportsBlock(script->data);
			if (!offsetPtr)
				error("Unable to find exports block from script 0");
			offsetPtr += 4 + 2;
		}
	} else {
		offsetPtr = script->data + 4 + 2 + 2;
	}
	
	int16 offset = !isSci11Mac() ? READ_LE_UINT16(offsetPtr) : READ_BE_UINT16(offsetPtr);

	// In SCI1.1 and newer, the heap is appended at the end of the script,
	// so adjust the offset accordingly
	if (getSciVersion() >= SCI_VERSION_1_1 && addSci11ScriptOffset) {
		offset += script->size;

		// Ensure that the start of the heap is word-aligned - same as in Script::init()
		if (script->size & 2)
			offset++;
	}

	return make_reg(1, offset);
}

Common::String ResourceManager::findSierraGameId() {
	// In SCI0-SCI1, the heap is embedded in the script. In SCI1.1+, it's separated
	Resource *heap = 0;
	int nameSelector = 3;

	if (getSciVersion() < SCI_VERSION_1_1) {
		heap = findResource(ResourceId(kResourceTypeScript, 0), false);
	} else {
		heap = findResource(ResourceId(kResourceTypeHeap, 0), false);
		nameSelector += 5;
	}

	if (!heap)
		return "";

	int16 gameObjectOffset = findGameObject(false).offset;

	if (!gameObjectOffset)
		return "";

	// Seek to the name selector of the first export
	byte *seeker = heap->data + READ_UINT16(heap->data + gameObjectOffset + nameSelector * 2);
	Common::String sierraId;
	sierraId += (const char *)seeker;

	return sierraId;
}

} // End of namespace Sci
