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

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = file_name;
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
			return SCI_VERSION_01;
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
			return SCI_VERSION_01;
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

ResourceManager::ResourceManager(int maxMemory) {
	_maxMemory = maxMemory;
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_audioMapSCI1 = NULL;

	addAppropriateSources();

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

	debug("Resmgr: Detected resource map version %d: %s", _mapVersion, versionDescription(_mapVersion));
	debug("Resmgr: Detected volume version %d: %s", _volVersion, versionDescription(_volVersion));

	scanNewSources();
	addInternalSources();
	scanNewSources();

	switch (_mapVersion) {
	case kResVersionSci0Sci1Early:
		if (testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB))) {
			_sciVersion = guessSciVersion() ? SCI_VERSION_01 : SCI_VERSION_0;
		} else if (testResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB))) {
			_sciVersion = guessSciVersion();
			if (_sciVersion != SCI_VERSION_01) {
				_sciVersion = testResource(ResourceId(kResourceTypeVocab, 912)) ? SCI_VERSION_0 : SCI_VERSION_01;
			}
		} else {
			_sciVersion = guessSciVersion() ? SCI_VERSION_01 : SCI_VERSION_0;
		}
		break;
	case kResVersionSci1Middle:
		_sciVersion = SCI_VERSION_01;
		break;
	case kResVersionSci1Late:
		_sciVersion = SCI_VERSION_1;
		break;
	case kResVersionSci11:
		_sciVersion = SCI_VERSION_1_1;
		break;
	case kResVersionSci32:
		_sciVersion = SCI_VERSION_32;
		break;
	default:
		_sciVersion = SCI_VERSION_AUTODETECT;
	}

	_isVGA = false;

	// Determine if the game is using EGA graphics or not
	if (_sciVersion == SCI_VERSION_0) {
		_isVGA = false;		// There is no SCI0 VGA game
	} else if (_sciVersion >= SCI_VERSION_1_1) {
		_isVGA = true;		// There is no SCI11 EGA game
	} else {
		// SCI01 or SCI1: EGA games have the second byte of their views set
		// to 0, VGA ones to non-zero
		int i = 0;

		while (true) {
			Resource *res = findResource(ResourceId(kResourceTypeView, i), 0);
			if (res) {
				_isVGA = (res->data[1] != 0);
				break;
			}
			i++;
		}
	}

	// Workaround for QFG1 VGA (has SCI 1.1 view data with SCI 1 compression)
	if (_sciVersion == SCI_VERSION_1 && !strcmp(((SciEngine*)g_engine)->getGameID(), "qfg1")) {
		debug("Resmgr: Detected QFG1 VGA");
		_isVGA = true;
	}

	// temporary version printout - should be reworked later
	switch (_sciVersion) {
	case SCI_VERSION_0:
		debug("Resmgr: Detected SCI0");
		break;
	case SCI_VERSION_01:
		debug("Resmgr: Detected SCI01");
		break;
	case SCI_VERSION_1:
		debug("Resmgr: Detected SCI1");
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

	if (_isVGA)
		debug("Resmgr: Detected VGA graphic resources");
	else
		debug("Resmgr: Detected non-VGA/EGA graphic resources");
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
		warning("Resmgr: trying to remove resource that isn't enqueued");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->status != kResStatusAllocated) {
		warning("Resmgr: trying to enqueue resource with state %d", res->status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if SCI_VERBOSE_RESMGR
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
	while (_maxMemory < _memoryLRU) {
		assert(!_LRU.empty());
		Resource *goner = *_LRU.reverse_begin();
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMGR
		printf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
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
		warning("Resmgr: Failed to read %s", retval->id.toString().c_str());
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
	Common::File file;
	byte buff[6];
	ResourceSource *rsrc= 0;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceExtMap) {
			file.open(rsrc->location_name);
			break;
		}
	}
	if (file.isOpen() == false) {
		error("Failed to open resource map file");
		return kResVersionUnknown;
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
	file.seek(0, SEEK_SET);
	while (!file.eos()) {
		directoryType = file.readByte();
		directoryOffset = file.readUint16LE();
		if ((directoryType < 0x80) || ((directoryType > 0xA0) && (directoryType != 0xFF)))
			break;
		// Offset is above file size? -> definitely not SCI1/SCI1.1
		if (directoryOffset > file.size())
			break;
		if (lastDirectoryOffset) {
			directorySize = directoryOffset - lastDirectoryOffset;
			if ((directorySize % 5) && (directorySize % 6 == 0))
				mapDetected = kResVersionSci1Late;
			if ((directorySize % 5 == 0) && (directorySize % 6))
				mapDetected = kResVersionSci11;
		}
		if (directoryType==0xFF) {
			// FFh entry needs to point to EOF
			if (directoryOffset != file.size())
				break;
			if (mapDetected) 
				return mapDetected;
			return kResVersionSci1Late;
		}
		lastDirectoryOffset = directoryOffset;
	}

#ifdef ENABLE_SCI32
	// late SCI1.1 and SCI32 maps have last directory entry set to 0xFF
	// offset set to filesize and 4 more bytes

	// TODO/FIXME: This code was not updated in r42300, which changed the behavior of this
	// function a lot. To make it compile again "off" was changed to the newly introduced
	// "lastDirectoryOffset". This is probably not the correct fix, since before r43000
	// the loop above could not prematurely terminate and thus this would always check the
	// last directory entry instead of the last checked directory entry.
	file.seek(lastDirectoryOffset - 7, SEEK_SET);
	if (file.readByte() == 0xFF && file.readUint16LE() == file.size())
		return kResVersionSci32; // TODO : check if there is a difference between these maps
#endif

	return kResVersionUnknown;
}

ResourceManager::ResVersion ResourceManager::detectVolVersion() {
	Common::File file;
	ResourceSource *rsrc;
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->source_type == kSourceVolume) {
			file.open(rsrc->location_name);
			break;
		}
	}
	if (file.isOpen() == false) {
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
	while (!file.eos() && file.pos() < 0x100000) {
		if (curVersion > kResVersionSci0Sci1Early)
			file.readByte();
		resId = file.readUint16LE();
		dwPacked = (curVersion < kResVersionSci32) ? file.readUint16LE() : file.readUint32LE();
		dwUnpacked = (curVersion < kResVersionSci32) ? file.readUint16LE() : file.readUint32LE();
		wCompression = (curVersion < kResVersionSci32) ? file.readUint16LE() : file.readUint32LE();
		if (file.eos())
			return curVersion;

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

			file.seek(0, SEEK_SET);
			continue;
		}

		if (curVersion < kResVersionSci11)
			file.seek(dwPacked - 4, SEEK_CUR);
		else if (curVersion == kResVersionSci11)
			file.seek((9 + dwPacked) % 2 ? dwPacked + 1 : dwPacked, SEEK_CUR);
		else if (curVersion == kResVersionSci32)
			file.seek(dwPacked, SEEK_CUR);//(9 + wPacked) % 2 ? wPacked + 1 : wPacked, SEEK_CUR);
	}

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
				warning("Resource patch unsupported special case %X\n", patch_data_offset);
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
			res->file_offset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->id = resId;
			res->source = getVolume(map, offset >> bShift);
			if (!res->source) {
				warning("Could not get volume for resource %d, VolumeID %d\n", id, offset >> bShift);
			}
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

} // End of namespace Sci
