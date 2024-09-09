/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Resource library

#include "common/config-manager.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/macresman.h"
#include "common/textconsole.h"
#include "common/translation.h"
#ifdef ENABLE_SCI32
#include "common/compression/installshield_cab.h"
#include "common/memstream.h"
#endif

#include "sci/engine/workarounds.h"
#include "sci/parser/vocabulary.h"
#include "sci/resource/resource.h"
#include "sci/resource/resource_intern.h"
#include "sci/resource/resource_patcher.h"
#include "sci/util.h"

namespace Sci {

enum {
	SCI0_RESMAP_ENTRIES_SIZE = 6,
	SCI1_RESMAP_ENTRIES_SIZE = 6,
	KQ5FMT_RESMAP_ENTRIES_SIZE = 7,
	SCI11_RESMAP_ENTRIES_SIZE = 5
};

/** resource type for SCI1 resource.map file */
struct resource_index_t {
	uint16 wOffset;
	uint16 wSize;
};

//////////////////////////////////////////////////////////////////////

SciVersion g_sciVersion = SCI_VERSION_NONE;	// FIXME: Move this inside a suitable class, e.g. SciEngine

SciVersion getSciVersionForDetection() {
	assert(!g_sci);
	return g_sciVersion;
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
	case SCI_VERSION_1_EGA_ONLY:
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
	case SCI_VERSION_2_1_EARLY:
		return "Early SCI2.1";
	case SCI_VERSION_2_1_MIDDLE:
		return "Middle SCI2.1";
	case SCI_VERSION_2_1_LATE:
		return "Late SCI2.1";
	case SCI_VERSION_3:
		return "SCI3";
	default:
		return "Unknown";
	}
}

//////////////////////////////////////////////////////////////////////

//#define SCI_VERBOSE_RESMAN

static const char *const s_errorDescriptions[] = {
	"No error",
	"I/O error",
	"Resource is empty (size 0)",
	"resource.map entry is invalid",
	"resource.map file not found",
	"No resource files found",
	"Unknown compression method",
	"Decompression failed: Sanity check failed",
	"Decompression failed: Resource too big"
};

static const char *const s_resourceTypeNames[] = {
	"view", "pic", "script", "text", "sound",
	"memory", "vocab", "font", "cursor",
	"patch", "bitmap", "palette", "cdaudio",
	"audio", "sync", "message", "map", "heap",
	"audio36", "sync36", "xlate", "robot", "vmd",
	"chunk", "animation", "etc", "duck", "clut",
	"tga", "zzz", "macibin", "macibis", "macpict",
	"rave"
};

// Resource type suffixes. Note that the
// suffix of SCI3 scripts has been changed from
// scr to csc
static const char *const s_resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	   "", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep",    "",    "",
	"trn", "rbt", "vmd", "chk",    "",
	"etc", "duk", "clu", "tga", "zzz",
	   "",    "",    "", ""
};

const char *getResourceTypeName(ResourceType restype) {
	if (restype < ARRAYSIZE(s_resourceTypeNames))
		return s_resourceTypeNames[restype];
	else
		return "invalid";
}

const char *getResourceTypeExtension(ResourceType restype) {
	if (restype < ARRAYSIZE(s_resourceTypeSuffixes))
		return s_resourceTypeSuffixes[restype];
	else
		return "";
}

static const ResourceType s_resTypeMapSci0[] = {
	kResourceTypeView, kResourceTypePic, kResourceTypeScript, kResourceTypeText,          // 0x00-0x03
	kResourceTypeSound, kResourceTypeMemory, kResourceTypeVocab, kResourceTypeFont,       // 0x04-0x07
	kResourceTypeCursor, kResourceTypePatch, kResourceTypeBitmap, kResourceTypePalette,   // 0x08-0x0B
	kResourceTypeCdAudio, kResourceTypeAudio, kResourceTypeSync, kResourceTypeMessage,    // 0x0C-0x0F
	kResourceTypeMap, kResourceTypeHeap, kResourceTypeAudio36, kResourceTypeSync36,       // 0x10-0x13
	kResourceTypeTranslation, kResourceTypeRave                                           // 0x14
};

// TODO: 12 should be "Wave", but SCI seems to just store it in Audio resources
static const ResourceType s_resTypeMapSci21[] = {
	kResourceTypeView, kResourceTypePic, kResourceTypeScript, kResourceTypeAnimation,     // 0x00-0x03
	kResourceTypeSound, kResourceTypeEtc, kResourceTypeVocab, kResourceTypeFont,          // 0x04-0x07
	kResourceTypeCursor, kResourceTypePatch, kResourceTypeBitmap, kResourceTypePalette,   // 0x08-0x0B
	kResourceTypeAudio, kResourceTypeAudio, kResourceTypeSync, kResourceTypeMessage,      // 0x0C-0x0F
	kResourceTypeMap, kResourceTypeHeap, kResourceTypeChunk, kResourceTypeAudio36,        // 0x10-0x13
	kResourceTypeSync36, kResourceTypeTranslation, kResourceTypeRobot, kResourceTypeVMD,  // 0x14-0x17
	kResourceTypeDuck, kResourceTypeClut, kResourceTypeTGA, kResourceTypeZZZ              // 0x18-0x1B
};

ResourceType ResourceManager::convertResType(byte type) {
	type &= 0x7f;

	bool forceSci0 = false;

	// LSL6 hires doesn't have the chunk resource type, to match
	// the resource types of the lowres version, thus we use the
	// older resource types here.
	// PQ4 CD and QFG4 CD are SCI2.1, but use the resource types of the
	// corresponding SCI2 floppy disk versions.
	// GK1 is the only SCI 2.0 Mac game and uses the older resource types.
	if (g_sci && (g_sci->getGameId() == GID_LSL6HIRES ||
			g_sci->getGameId() == GID_QFG4 ||
			g_sci->getGameId() == GID_PQ4 ||
			g_sci->getGameId() == GID_GK1)) {
		forceSci0 = true;
	}

	if ((_mapVersion < kResVersionSci2 && !_isSci2Mac) || forceSci0) {
		// SCI0 - SCI2
		if (type < ARRAYSIZE(s_resTypeMapSci0))
			return s_resTypeMapSci0[type];
	} else {
		if (type < ARRAYSIZE(s_resTypeMapSci21))
			return s_resTypeMapSci21[type];
	}

	return kResourceTypeInvalid;
}

//-- Resource main functions --
Resource::Resource(ResourceManager *resMan, ResourceId id) : SciSpan<const byte>(nullptr, 0, id.toString()), _resMan(resMan), _id(id) {
	_fileOffset = 0;
	_status = kResStatusNoMalloc;
	_lockers = 0;
	_source = nullptr;
	_header = nullptr;
	_headerSize = 0;
}

Resource::~Resource() {
	delete[] _data;
	delete[] _header;
	if (_source && _source->getSourceType() == kSourcePatch)
		delete _source;
}

void Resource::unalloc() {
	delete[] _data;
	_data = nullptr;
	_status = kResStatusNoMalloc;
}

void Resource::writeToStream(Common::WriteStream *stream) const {
	if (_headerSize == 0) {
		// create patch file header
		stream->writeByte(getType() | 0x80); // 0x80 is required by old Sierra SCI, otherwise it won't accept the patch file
		stream->writeByte(_headerSize);
	} else {
		// use existing patch file header
		stream->write(_header, _headerSize);
	}
	stream->write(_data, _size);
}

#ifdef ENABLE_SCI32
Common::SeekableReadStream *Resource::makeStream() const {
	return new Common::MemoryReadStream(_data, _size, DisposeAfterUse::NO);
}
#endif

uint32 Resource::getAudioCompressionType() const {
	return _source->getAudioCompressionType();
}

uint32 AudioVolumeResourceSource::getAudioCompressionType() const {
	return _audioCompressionType;
}

ResourceSource::ResourceSource(ResSourceType type, const Common::Path &name, int volNum, const Common::FSNode *resFile)
 : _sourceType(type), _name(name), _volumeNumber(volNum), _resourceFile(resFile) {
	_scanned = false;
}

ResourceSource::~ResourceSource() {
}

MacResourceForkResourceSource::MacResourceForkResourceSource(const Common::Path &name, int volNum)
 : ResourceSource(kSourceMacResourceFork, name, volNum) {
	_macResMan = new Common::MacResManager();
}

MacResourceForkResourceSource::~MacResourceForkResourceSource() {
	delete _macResMan;
}

//-- resMan helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const Common::Path &filename, int volume_nr) {
	ResourceSource *newsrc = new ExtMapResourceSource(filename, volume_nr);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addExternalMap(const Common::FSNode *mapFile, int volume_nr) {
	ResourceSource *newsrc = new ExtMapResourceSource(mapFile->getPathInArchive(), volume_nr, mapFile);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addSource(ResourceSource *newsrc) {
	assert(newsrc);

	_sources.push_back(newsrc);
	return newsrc;
}

ResourceSource *ResourceManager::addPatchDir(const Common::Path &dirname) {
	ResourceSource *newsrc = new DirectoryResourceSource(dirname);

	_sources.push_back(newsrc);
	return nullptr;
}

ResourceSource *ResourceManager::findVolume(ResourceSource *map, int volume_nr) {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *src = (*it)->findVolume(map, volume_nr);
		if (src)
			return src;
	}

	return nullptr;
}

// Resource manager constructors and operations

bool Resource::loadPatch(Common::SeekableReadStream *file) {
	// We assume that the resource type matches `type`
	//  We also assume that the current file position is right at the actual data (behind resourceid/headersize byte)

	byte *ptr = new byte[size()];
	_data = ptr;

	if (_headerSize > 0)
		_header = new byte[_headerSize];

	if (data() == nullptr || (_headerSize > 0 && _header == nullptr)) {
		error("Can't allocate %u bytes needed for loading %s", size() + _headerSize, _id.toString().c_str());
	}

	uint32 bytesRead;
	if (_headerSize > 0) {
		bytesRead = file->read(_header, _headerSize);
		if (bytesRead != _headerSize)
			error("Read %d bytes from %s but expected %d", bytesRead, _id.toString().c_str(), _headerSize);
	}

	bytesRead = file->read(ptr, size());
	if (bytesRead != size())
		error("Read %d bytes from %s but expected %u", bytesRead, _id.toString().c_str(), size());

	_status = kResStatusAllocated;
	return true;
}

bool Resource::loadFromPatchFile() {
	Common::File file;
	const Common::Path &filename = _source->getLocationName();
	if (!file.open(filename)) {
		warning("Failed to open patch file %s", filename.toString().c_str());
		unalloc();
		return false;
	}
	file.seek(0, SEEK_SET);
	return loadPatch(&file);
}

Common::SeekableReadStream *ResourceManager::getVolumeFile(ResourceSource *source) {
	Common::List<Common::File *>::iterator it = _volumeFiles.begin();
	Common::File *file;

#ifdef ENABLE_SCI32
	ChunkResourceSource *chunkSource = dynamic_cast<ChunkResourceSource *>(source);
	if (chunkSource != nullptr) {
		Resource *res = findResource(ResourceId(kResourceTypeChunk, chunkSource->getNumber()), false);
		return res ? res->makeStream() : nullptr;
	}
#endif

	if (source->_resourceFile)
		return source->_resourceFile->createReadStream();

	// TODO: check if it still works
	const Common::String filename = source->getLocationName().toString('/');

	// check if file is already opened
	while (it != _volumeFiles.end()) {
		file = *it;
		if (scumm_stricmp(file->getName(), filename.c_str()) == 0) {
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
	if (file->open(source->getLocationName())) {
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
	return nullptr;
}

void ResourceManager::disposeVolumeFileStream(Common::SeekableReadStream *fileStream, Sci::ResourceSource *source) {
#ifdef ENABLE_SCI32
	ChunkResourceSource *chunkSource = dynamic_cast<ChunkResourceSource *>(source);
	if (chunkSource != nullptr) {
		delete fileStream;
		return;
	}
#endif

	if (source->_resourceFile) {
		delete fileStream;
		return;
	}

	// Other volume file streams are cached in _volumeFiles and should only be
	// deleted from _volumeFiles
}

void ResourceManager::loadResource(Resource *res) {
	res->_source->loadResource(this, res);
	if (_patcher) {
		_patcher->applyPatch(*res);
	};
}


void PatchResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	bool result = res->loadFromPatchFile();
	if (!result) {
		// TODO: We used to fallback to the "default" code here if loadFromPatchFile
		// failed, but I am not sure whether that is really appropriate.
		// In fact it looks like a bug to me, so I commented this out for now.
		//ResourceSource::loadResource(res);
	}
}

static Common::Array<uint32> resTypeToMacTags(ResourceType type);

void MacResourceForkResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	ResourceType type = res->getType();
	Common::SeekableReadStream *stream = nullptr;

	if (type == kResourceTypeAudio36 || type == kResourceTypeSync36) {
		// Handle audio36/sync36, convert back to audio/sync
		stream = _macResMan->getResource(res->_id.toPatchNameBase36());
	} else {
		// Plain resource handling
		Common::Array<uint32> tagArray = resTypeToMacTags(type);

		for (uint32 i = 0; i < tagArray.size() && !stream; i++)
			stream = _macResMan->getResource(tagArray[i], res->getNumber());
	}

	if (stream)
		decompressResource(stream, res);
}

bool MacResourceForkResourceSource::isCompressableResource(ResourceType type) const {
	// Any types that were not originally an SCI format are not compressed, it seems.
	// (Audio/36 being Mac snd resources here)
	return type != kResourceTypeMacPict && type != kResourceTypeAudio &&
			type != kResourceTypeMacIconBarPictN && type != kResourceTypeMacIconBarPictS &&
			type != kResourceTypeAudio36 && type != kResourceTypeSync &&
			type != kResourceTypeSync36 && type != kResourceTypeCursor;
}

#define OUTPUT_LITERAL() \
	assert(ptr + literalLength <= bufferEnd); \
	while (literalLength--) \
		*ptr++ = stream->readByte();

#define OUTPUT_COPY() \
	assert(ptr + copyLength <= bufferEnd); \
	while (copyLength--) { \
		byte value = ptr[-offset]; \
		*ptr++ = value; \
	}

void MacResourceForkResourceSource::decompressResource(Common::SeekableReadStream *stream, Resource *resource) const {
	// KQ6 Mac is the only game not compressed. It's not worth writing a
	// heuristic just for that game. Also, skip over any resource that cannot
	// be compressed.
	bool canBeCompressed = !(g_sci && g_sci->getGameId() == GID_KQ6) && isCompressableResource(resource->_id.getType());
	uint32 uncompressedSize = 0;

#ifdef ENABLE_SCI32
	// GK2 Mac is crazy. In its Patches resource fork, picture 2315 is not
	// compressed and it is hardcoded in the executable to say that it's
	// not compressed. Why didn't they just add four zeroes to the end of
	// the resource? (Checked with PPC disasm)
	if (g_sci && g_sci->getGameId() == GID_GK2 && resource->_id.getType() == kResourceTypePic && resource->_id.getNumber() == 2315)
		canBeCompressed = false;
#endif

	// Get the uncompressed size from the end of the resource
	if (canBeCompressed && stream->size() > 4) {
		stream->seek(-4, SEEK_END);
		uncompressedSize = stream->readUint32BE();
		stream->seek(0);
	}

	if (uncompressedSize == 0) {
		// Not compressed
		resource->_size = stream->size();

		// Cut out the 'non-compressed marker' (four zeroes) at the end
		if (canBeCompressed)
			resource->_size -= 4;

		byte *ptr = new byte[resource->size()];
		resource->_data = ptr;
		stream->read(ptr, resource->size());
	} else {
		// Decompress
		resource->_size = uncompressedSize;
		byte *ptr = new byte[uncompressedSize];
		resource->_data = ptr;

		const byte *const bufferEnd = resource->data() + uncompressedSize;

		while (stream->pos() < stream->size()) {
			byte code = stream->readByte();

			int literalLength = 0, offset = 0, copyLength = 0;
			byte extraByte1 = 0, extraByte2 = 0;

			if (code == 0xFF) {
				// End of stream marker
				break;
			}

			switch (code & 0xC0) {
			case 0x80:
				// Copy chunk expanded
				extraByte1 = stream->readByte();
				extraByte2 = stream->readByte();

				literalLength = extraByte2 & 3;

				OUTPUT_LITERAL()

				offset = ((code & 0x3f) | ((extraByte1 & 0xe0) << 1) | ((extraByte2 & 0xfc) << 7)) + 1;
				copyLength = (extraByte1 & 0x1f) + 3;

				OUTPUT_COPY()
				break;
			case 0xC0:
				// Literal chunk
				if (code >= 0xD0) {
					// These codes cannot be used
					if (code == 0xD0 || code > 0xD3)
						error("Bad Mac compression code %02x", code);

					literalLength = code & 3;
				} else
					literalLength = (code & 0xf) * 4 + 4;

				OUTPUT_LITERAL()
				break;
			default:
				// Copy chunk
				extraByte1 = stream->readByte();

				literalLength = (extraByte1 >> 3) & 0x3;

				OUTPUT_LITERAL()

				offset = (code + ((extraByte1 & 0xE0) << 2)) + 1;
				copyLength = (extraByte1 & 0x7) + 3;

				OUTPUT_COPY()
				break;
			}
		}
	}

	resource->_status = kResStatusAllocated;
	delete stream;
}

#undef OUTPUT_LITERAL
#undef OUTPUT_COPY

Common::SeekableReadStream *ResourceSource::getVolumeFile(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = resMan->getVolumeFile(this);

	if (!fileStream) {
		warning("Failed to open %s", getLocationName().toString().c_str());
		resMan->_hasBadResources = true;
		if (res)
			res->unalloc();
	}

	return fileStream;
}

void ResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = getVolumeFile(resMan, res);
	if (!fileStream)
		return;

	fileStream->seek(0, SEEK_SET);
	ResourceType type = resMan->convertResType(fileStream->readByte());
	ResVersion volVersion = resMan->getVolVersion();

	// FIXME: if resource.msg has different version from SCI, this has to be modified.
	if (
		(
			(type == kResourceTypeMessage && res->getType() == kResourceTypeMessage) ||
			(type == kResourceTypeText && res->getType() == kResourceTypeText)
		) &&
		g_sci && g_sci->getLanguage() == Common::KO_KOR)
		volVersion = kResVersionSci11;
	fileStream->seek(res->_fileOffset, SEEK_SET);

	int error = res->decompress(volVersion, fileStream);
	if (error) {
		warning("Error %d occurred while reading %s from resource file %s: %s",
				error, res->_id.toString().c_str(), res->getResourceLocation().toString().c_str(),
				s_errorDescriptions[error]);
		res->unalloc();
	}

	resMan->disposeVolumeFileStream(fileStream, this);
}

Resource *ResourceManager::testResource(const ResourceId &id) const {
	return _resMap.getValOrDefault(id, NULL);
}

void ResourceManager::addAppropriateSources() {
#ifdef ENABLE_SCI32
	_multiDiscAudio = false;
#endif
	if (Common::File::exists("resource.map")) {
		// SCI0-SCI2 file naming scheme
		ResourceSource *map = addExternalMap("resource.map");

		Common::ArchiveMemberList files;
		SearchMan.listMatchingMembers(files, "resource.0##");

		Common::sort(files.begin(), files.end(), Common::ArchiveMemberListComparator());
		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			const Common::String name = (*x)->getFileName();
			const char *dot = strrchr(name.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(new VolumeResourceSource((*x)->getPathInArchive(), map, number));
		}
#ifdef ENABLE_SCI32
		// GK1CD hires content
		if (Common::File::exists("alt.map") && Common::File::exists("resource.alt"))
			addSource(new VolumeResourceSource("resource.alt", addExternalMap("alt.map", 10), 10));
#endif
	} else if (Common::MacResManager::exists("Data1")) {
		// Mac SCI1.1+ file naming scheme
		Common::Array<Common::Path> files;
		Common::MacResManager::listFiles(files, "Data#");
		Common::MacResManager::listFiles(files, "Data##");

		for (Common::Array<Common::Path>::const_iterator x = files.begin(); x != files.end(); ++x) {
			Common::String baseName(x->baseName());
			addSource(new MacResourceForkResourceSource(*x, atoi(baseName.c_str() + 4)));
		}

#ifdef ENABLE_SCI32
		// There can also be a "Patches" resource fork with patches
		if (Common::MacResManager::exists("Patches"))
			addSource(new MacResourceForkResourceSource("Patches", 100));
	} else {
		// SCI2.1-SCI3 file naming scheme
		Common::ArchiveMemberList mapFiles, files;
		SearchMan.listMatchingMembers(mapFiles, "resmap.0##");
		SearchMan.listMatchingMembers(files, "ressci.0##");

		if (mapFiles.empty() || files.empty())
			return;

		if (Common::File::exists("ressci.001") && !Common::File::exists("resource.aud")) {
			_multiDiscAudio = true;
		}

		Common::sort(mapFiles.begin(), mapFiles.end(), Common::ArchiveMemberListComparator());
		for (Common::ArchiveMemberList::const_iterator mapIterator = mapFiles.begin(); mapIterator != mapFiles.end(); ++mapIterator) {
			Common::String mapName = (*mapIterator)->getFileName();
			int mapNumber = atoi(strrchr(mapName.c_str(), '.') + 1);
			bool foundVolume = false;

			for (Common::ArchiveMemberList::const_iterator fileIterator = files.begin(); fileIterator != files.end(); ++fileIterator) {
				Common::String resName = (*fileIterator)->getFileName();
				int resNumber = atoi(strrchr(resName.c_str(), '.') + 1);

				if (mapNumber == resNumber) {
					foundVolume = true;
					addSource(new VolumeResourceSource((*fileIterator)->getPathInArchive(), addExternalMap((*mapIterator)->getPathInArchive(), mapNumber), mapNumber));
					break;
				}
			}

			if (!foundVolume &&
				g_sci &&
				// GK2 on Steam comes with an extra bogus resource map file;
				// ignore it instead of treating it as a bad resource
				(g_sci->getGameId() != GID_GK2 || mapFiles.size() != 2 || mapNumber != 1)) {

				warning("Could not find corresponding volume for %s", mapName.c_str());
				_hasBadResources = true;
			}
		}

		// SCI2.1 resource patches
		if (Common::File::exists("resmap.pat") && Common::File::exists("ressci.pat")) {
			// We add this resource with a map which surely won't exist
			addSource(new VolumeResourceSource("ressci.pat", addExternalMap("resmap.pat", kResPatVolumeNumber), kResPatVolumeNumber));
		}
	}
#else
	} else
		return;
#endif

	addPatchDir(".");

	if (Common::File::exists("message.map"))
		addSource(new VolumeResourceSource("resource.msg", addExternalMap("message.map"), 0));

	if (Common::File::exists("altres.map"))
		addSource(new VolumeResourceSource("altres.000", addExternalMap("altres.map"), 0));

#ifdef ENABLE_SCI32
	// Some LSL7 Polish CDs have all of the patch files in InstallShield cabinet files
	//  (data1.cab/hdr) while the rest of the game is in normal SCI files. Trac #10066
	if (g_sci &&
		g_sci->getGameId() == GID_LSL7 && g_sci->getLanguage() == Common::PL_POL) {
		Common::Archive *archive = Common::makeInstallShieldArchive("data");
		if (archive != nullptr) {
			SearchMan.add("data1.cab", archive);
		}
	}
#endif
}

void ResourceManager::addAppropriateSourcesForDetection(const Common::FSList &fslist) {
	ResourceSource *map = nullptr;
	Common::Array<ResourceSource *> sci21Maps;

#ifdef ENABLE_SCI32
	ResourceSource *sci21PatchMap = nullptr;
	const Common::FSNode *sci21PatchRes = nullptr;
	_multiDiscAudio = false;
#endif

	// First, find resource.map
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getFileName();
		filename.toLowercase();

		if (filename.contains("resource.map"))
			map = addExternalMap(file);

		if (filename.contains("resmap.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			uint number = atoi(dot + 1);

			// We need to store each of these maps for use later on
			if (number >= sci21Maps.size())
				sci21Maps.resize(number + 1);

			sci21Maps[number] = addExternalMap(file, number);
		}

#ifdef ENABLE_SCI32
		// SCI2.1 resource patches
		if (filename.contains("resmap.pat"))
			sci21PatchMap = addExternalMap(file, kResPatVolumeNumber);

		if (filename.contains("ressci.pat"))
			sci21PatchRes = file;
#endif
	}

	if (!map && sci21Maps.empty())
		return;

#ifdef ENABLE_SCI32
	if (sci21PatchMap && sci21PatchRes)
		addSource(new VolumeResourceSource(sci21PatchRes->getPathInArchive(), sci21PatchMap, kResPatVolumeNumber, sci21PatchRes));
#endif

	// Now find all the resource.0?? files
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		if (file->isDirectory())
			continue;

		Common::String filename = file->getFileName();
		filename.toLowercase();

		if (filename.contains("resource.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			int number = atoi(dot + 1);

			addSource(new VolumeResourceSource(file->getPathInArchive(), map, number, file));
		} else if (filename.contains("ressci.0")) {
			const char *dot = strrchr(filename.c_str(), '.');
			int number = atoi(dot + 1);

			// Match this volume to its own map
			addSource(new VolumeResourceSource(file->getPathInArchive(), sci21Maps[number], number, file));
		}
	}

	// This function is only called by the advanced detector, and we don't really need
	// to add a patch directory or message.map here
}

void ResourceManager::addScriptChunkSources() {
#ifdef ENABLE_SCI32
	if (_mapVersion >= kResVersionSci2) {
		// If we have no scripts, but chunk 0 is present, open up the chunk
		// to try to get to any scripts in there. The Lighthouse SCI2.1 demo
		// does exactly this.

		Common::List<ResourceId> resources = listResources(kResourceTypeScript);

		if (resources.empty() && testResource(ResourceId(kResourceTypeChunk, 0)))
			addResourcesFromChunk(0);
	}
#endif
}

extern int showScummVMDialog(const Common::U32String &message, const Common::U32String &altButton = Common::U32String(), bool alignCenter = true);

void ResourceManager::scanNewSources() {
	_hasBadResources = false;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *source = *it;

		if (!source->_scanned) {
			source->_scanned = true;
			source->scanSource(this);
		}
	}

	// The warning dialog is shown here instead of someplace more obvious like
	// SciEngine::run because resource sources can be dynamically added
	// (e.g. KQ5 via kDoAudio, MGDX via kSetLanguage), and users really should
	// be warned of bad resources in this situation (KQ Collection 1997 has a
	// bad copy of KQ5 on CD 1; the working copy is on CD 2)
	if (!_detectionMode && _hasBadResources) {
		showScummVMDialog(_("Missing or corrupt game resources have been detected. "
							"Some game features may not work properly. Please check "
							"the console for more information, and verify that your "
							"game files are valid."));
	}
}

void DirectoryResourceSource::scanSource(ResourceManager *resMan) {
	resMan->readResourcePatches();

	// We can't use getSciVersion() at this point, thus using _volVersion
	if (resMan->_volVersion >= kResVersionSci11)	// SCI1.1+
		resMan->readResourcePatchesBase36();

	resMan->readWaveAudioPatches();
#ifdef ENABLE_SCI32
	resMan->readAIFFAudioPatches();
#endif
}

void ExtMapResourceSource::scanSource(ResourceManager *resMan) {
	if (resMan->_mapVersion < kResVersionSci1Late && !resMan->isKoreanMessageMap(this)) {
		if (resMan->readResourceMapSCI0(this) != SCI_ERROR_NONE) {
			resMan->_hasBadResources = true;
		}
	} else {
		if (resMan->readResourceMapSCI1(this) != SCI_ERROR_NONE) {
			resMan->_hasBadResources = true;
		}
	}
}

void ExtAudioMapResourceSource::scanSource(ResourceManager *resMan) {
	if (resMan->readAudioMapSCI1(this) != SCI_ERROR_NONE) {
		resMan->_hasBadResources = true;
	}
}

void IntMapResourceSource::scanSource(ResourceManager *resMan) {
	if (resMan->readAudioMapSCI11(this) != SCI_ERROR_NONE) {
		resMan->_hasBadResources = true;
	}
}

#ifdef ENABLE_SCI32

// Chunk resources are resources that hold other resources. They are normally called
// when using the kLoadChunk SCI2.1 kernel function. However, for example, the Lighthouse
// SCI2.1 demo has a chunk but no scripts outside of the chunk.

// A chunk resource is pretty straightforward in terms of layout
// It begins with 11-byte entries in the header:
// =========
// b resType
// w nEntry
// dw offset
// dw length

ChunkResourceSource::ChunkResourceSource(const Common::Path &name, uint16 number)
	: ResourceSource(kSourceChunk, name) {

	_number = number;
}

void ChunkResourceSource::scanSource(ResourceManager *resMan) {
	Resource *chunk = resMan->findResource(ResourceId(kResourceTypeChunk, _number), false);

	if (chunk == nullptr)
		error("Trying to load non-existent chunk");

	const byte *ptr = chunk->data();
	uint32 firstOffset = 0;

	for (;;) {
		ResourceType type = resMan->convertResType(*ptr);
		uint16 number = READ_LE_UINT16(ptr + 1);
		ResourceId id(type, number);

		ResourceEntry entry;
		entry.offset = READ_LE_UINT32(ptr + 3);
		entry.length = READ_LE_UINT32(ptr + 7);

		_resMap[id] = entry;
		ptr += 11;

		debugC(kDebugLevelResMan, 2, "Found %s in chunk %d", id.toString().c_str(), _number);

		resMan->updateResource(id, this, entry.length, chunk->_source->getLocationName());

		// There's no end marker to the data table, but the first resource
		// begins directly after the entry table. So, when we hit the first
		// resource, we're at the end of the entry table.

		if (!firstOffset)
			firstOffset = entry.offset;

		if ((size_t)(ptr - chunk->data()) >= firstOffset)
			break;
	}
}

void ChunkResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Resource *chunk = resMan->findResource(ResourceId(kResourceTypeChunk, _number), false);
	if (chunk == nullptr)
		error("Trying to load non-existent chunk");

	ResourceEntry entry;
	if (!_resMap.tryGetVal(res->_id, entry))
		error("Trying to load non-existent resource %s from chunk %d", res->_id.toString().c_str(), _number);

	if (entry.offset + entry.length > chunk->size()) {
		error("Resource %s is too large to exist within chunk %d (%u + %u > %u)", res->_id.toString().c_str(), _number, entry.offset, entry.length, chunk->size());
	}
	byte *ptr = new byte[entry.length];
	res->_data = ptr;
	res->_size = entry.length;
	res->_header = 0;
	res->_headerSize = 0;
	res->_status = kResStatusAllocated;

	// Copy the resource data over
	memcpy(ptr, chunk->data() + entry.offset, entry.length);
}

void ResourceManager::addResourcesFromChunk(uint16 id) {
	addSource(new ChunkResourceSource(Common::Path(Common::String::format("Chunk %d", id)), id));
	scanNewSources();
}

void ResourceManager::findDisc(const int16 discNo) {
	// Since all resources are expected to be copied from the original discs
	// into a single game directory, this call just records the number of the CD
	// that the game has requested
	_currentDiscNo = discNo;
}

#endif

void ResourceManager::freeResourceSources() {
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it)
		delete *it;

	_sources.clear();
}

ResourceManager::ResourceManager(const bool detectionMode) :
	_detectionMode(detectionMode) {}

void ResourceManager::init() {
	_maxMemoryLRU = 256 * 1024; // 256KiB
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_audioMapSCI1 = nullptr;
#ifdef ENABLE_SCI32
	_currentDiscNo = 1;
#endif
	if (g_sci) {
		_patcher = new ResourcePatcher(g_sci->getGameId(), g_sci->isCD(), g_sci->getPlatform(), g_sci->getLanguage());
		addSource(_patcher);
	} else {
		_patcher = nullptr;
	};

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

	if ((_volVersion == kResVersionSci3) && (_mapVersion < kResVersionSci2)) {
		warning("Detected volume version is too high for detected map version. Setting volume version to map version");
		_volVersion = _mapVersion;
	}

	debugC(1, kDebugLevelResMan, "resMan: Detected resource map version %d: %s", _mapVersion, versionDescription(_mapVersion));
	debugC(1, kDebugLevelResMan, "resMan: Detected volume version %d: %s", _volVersion, versionDescription(_volVersion));

	if ((_mapVersion == kResVersionUnknown) && (_volVersion == kResVersionUnknown)) {
		warning("Volume and map version not detected, assuming that this is not a SCI game");
		_viewType = kViewUnknown;
		return;
	}

#ifdef ENABLE_SCI32
	if (_volVersion == kResVersionSci11Mac)
		_isSci2Mac = detectSci2Mac();
	else
#endif
		_isSci2Mac = false;

	scanNewSources();

	if (!addAudioSources()) {
		// FIXME: This error message is not always correct.
		// OTOH, it is nice to be able to detect missing files/sources
		// So we should definitely fix addAudioSources so this error
		// only pops up when necessary. Disabling for now.
		//error("Somehow I can't seem to find the sound files I need (RESOURCE.AUD/RESOURCE.SFX), aborting");
	}

	addScriptChunkSources();
	scanNewSources();

	detectSciVersion();

	debugC(1, kDebugLevelResMan, "resMan: Detected %s", getSciVersionDesc(getSciVersion()));

	// Resources in SCI32 games are significantly larger than SCI16
	// games and can cause immediate exhaustion of the LRU resource
	// cache, leading to constant decompression of picture resources
	// and making the renderer very slow.
	if (getSciVersion() >= SCI_VERSION_2) {
		_maxMemoryLRU = 4096 * 1024; // 4MiB
	}

	switch (_viewType) {
	case kViewEga:
		debugC(1, kDebugLevelResMan, "resMan: Detected EGA graphic resources");
		break;
	case kViewAmiga:
		debugC(1, kDebugLevelResMan, "resMan: Detected Amiga ECS graphic resources");
		break;
	case kViewAmiga64:
		debugC(1, kDebugLevelResMan, "resMan: Detected Amiga AGA graphic resources");
		break;
	case kViewVga:
		debugC(1, kDebugLevelResMan, "resMan: Detected VGA graphic resources");
		break;
	case kViewVga11:
		debugC(1, kDebugLevelResMan, "resMan: Detected SCI1.1 VGA graphic resources");
		break;
	default:
		// Throw a warning, but do not error out here, because this is called from the
		// fallback detector, and the user could be pointing to a folder with a non-SCI
		// game, but with SCI-like file names (e.g. Pinball Creep)
		warning("resMan: Couldn't determine view type");
		break;
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
	_memoryLRU -= res->size();
	res->_status = kResStatusAllocated;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->_status != kResStatusAllocated) {
		warning("resMan: trying to enqueue resource with state %d", res->_status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size();
#ifdef SCI_VERBOSE_RESMAN
	debug("Adding %s (%d bytes) to lru control: %d bytes total",
	      res->_id.toString().c_str(), res->size,
	      _memoryLRU);
#endif
	res->_status = kResStatusEnqueued;
}

void ResourceManager::freeOldResources() {
	while (_maxMemoryLRU < _memoryLRU) {
		assert(!_LRU.empty());
		Resource *goner = _LRU.back();
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMAN
		debug("resMan-debug: LRU: Freeing %s (%d bytes)", goner->_id.toString().c_str(), goner->size);
#endif
	}
}

Common::List<ResourceId> ResourceManager::listResources(ResourceType type, int mapNumber) {
	Common::List<ResourceId> resources;

	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		if ((itr->_value->getType() == type) && ((mapNumber == -1) || (itr->_value->getNumber() == mapNumber)))
			resources.push_back(itr->_value->_id);
		++itr;
	}

	return resources;
}

bool ResourceManager::hasResourceType(ResourceType type) {
	ResourceMap::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		if (itr->_value->getType() == type) {
			return true;
		}
		++itr;
	}
	return false;
}

Resource *ResourceManager::findResource(ResourceId id, bool lock) {
	// remap known incorrect audio36 and sync36 resource ids
	if (id.getType() == kResourceTypeAudio36) {
		id = remapAudio36ResourceId(id);
	} else if (id.getType() == kResourceTypeSync36) {
		id = remapSync36ResourceId(id);
	}
	Resource *retval = testResource(id);

	if (!retval)
		return nullptr;

	if (retval->_status == kResStatusNoMalloc)
		loadResource(retval);
	else if (retval->_status == kResStatusEnqueued)
		// The resource is removed from its current position
		// in the LRU list because it has been requested
		// again. Below, it will either be locked, or it
		// will be added back to the LRU list at the 'most
		// recent' position.
		removeFromLRU(retval);

	// Unless an error occurred, the resource is now either
	// locked or allocated, but never queued or freed.

	freeOldResources();

	if (lock) {
		if (retval->_status == kResStatusAllocated) {
			retval->_status = kResStatusLocked;
			retval->_lockers = 0;
			_memoryLocked += retval->_size;
		}
		retval->_lockers++;
	} else if (retval->_status != kResStatusLocked) { // Don't lock it
		if (retval->_status == kResStatusAllocated)
			addToLRU(retval);
	}

	if (retval->data())
		return retval;
	else {
		warning("resMan: Failed to read %s", retval->_id.toString().c_str());
		return nullptr;
	}
}

void ResourceManager::unlockResource(Resource *res) {
	assert(res);

	if (res->_status != kResStatusLocked) {
		debugC(kDebugLevelResMan, 2, "[resMan] Attempt to unlock unlocked resource %s", res->_id.toString().c_str());
		return;
	}

	if (!--res->_lockers) { // No more lockers?
		res->_status = kResStatusAllocated;
		_memoryLocked -= res->size();
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
	case kResVersionKQ5FMT:
		return "KQ5 FM Towns";
	case kResVersionSci1Late:
		return "Late SCI1";
	case kResVersionSci11:
		return "SCI1.1";
	case kResVersionSci11Mac:
		return "Mac SCI1.1+";
	case kResVersionSci2:
		return "SCI2/2.1";
	case kResVersionSci3:
		return "SCI3";
	default:
		break;
	}

	return "Version not valid";
}

ResVersion ResourceManager::detectMapVersion() {
	Common::SeekableReadStream *fileStream = nullptr;
	byte buff[6];
	ResourceSource *rsrc= nullptr;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->getSourceType() == kSourceExtMap) {
			if (rsrc->_resourceFile) {
				fileStream = rsrc->_resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->getLocationName());
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->getSourceType() == kSourceMacResourceFork) {
			return kResVersionSci11Mac;
		}
	}

	if (!fileStream) {
		warning("Failed to open resource map file");
		return kResVersionUnknown;
	}

	// detection
	// SCI0 and SCI01 maps have last 6 bytes set to FF
	fileStream->seek(-4, SEEK_END);
	uint32 uEnd = fileStream->readUint32LE();
	if (uEnd == 0xFFFFFFFF) {
		// check if the last 7 bytes are all ff, indicating a KQ5 FM-Towns map
		fileStream->seek(-7, SEEK_END);
		fileStream->read(buff, 3);
		if (buff[0] == 0xff && buff[1] == 0xff && buff[2] == 0xff) {
			delete fileStream;
			return kResVersionKQ5FMT;
		}

		// check if 0 or 01 - try to read resources in SCI0 format and see if exists
		fileStream->seek(0, SEEK_SET);
		while (fileStream->read(buff, 6) == 6 && !(buff[0] == 0xFF && buff[1] == 0xFF && buff[2] == 0xFF)) {
			if (findVolume(rsrc, (buff[5] & 0xFC) >> 2) == nullptr) {
				delete fileStream;
				return kResVersionSci1Middle;
			}
		}
		delete fileStream;
		return kResVersionSci0Sci1Early;
	}

	// SCI1 and SCI1.1 maps consist of a fixed 3-byte header, a directory list (3-bytes each) that has one entry
	// of id FFh and points to EOF. The actual entries have 6-bytes on SCI1 and 5-bytes on SCI1.1
	uint16 lastDirectoryOffset = 0;
	ResVersion mapDetected = kResVersionUnknown;
	fileStream->seek(0, SEEK_SET);

	while (!fileStream->eos()) {
		byte directoryType = fileStream->readByte();
		uint16 directoryOffset = fileStream->readUint16LE();

		// Only SCI32 has directory type < 0x80
		if (directoryType < 0x80 && (mapDetected == kResVersionUnknown || mapDetected == kResVersionSci2))
			mapDetected = kResVersionSci2;
		else if (directoryType < 0x80 || ((directoryType & 0x7f) > 0x20 && directoryType != 0xFF))
			break;

		// Offset is above file size? -> definitely not SCI1/SCI1.1
		if (directoryOffset > fileStream->size())
			break;

		if (lastDirectoryOffset && mapDetected == kResVersionUnknown) {
			uint16 directorySize = directoryOffset - lastDirectoryOffset;
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

ResVersion ResourceManager::detectVolVersion() {
	Common::SeekableReadStream *fileStream = nullptr;
	ResourceSource *rsrc;

	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		rsrc = *it;

		if (rsrc->getSourceType() == kSourceVolume) {
			if (rsrc->_resourceFile) {
				fileStream = rsrc->_resourceFile->createReadStream();
			} else {
				Common::File *file = new Common::File();
				file->open(rsrc->getLocationName());
				if (file->isOpen())
					fileStream = file;
			}
			break;
		} else if (rsrc->getSourceType() == kSourceMacResourceFork)
			return kResVersionSci11Mac;
	}

	if (!fileStream) {
		warning("Failed to open volume file - if you got resource.p01/resource.p02/etc. files, merge them together into resource.000");
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
	ResVersion curVersion = kResVersionSci0Sci1Early;
	bool failed = false;
	bool sci11Align = false;

	// Check for SCI0, SCI1, SCI1.1, SCI32 v2 (Gabriel Knight 1 CD) and SCI32 v3 (LSL7) formats
	while (!fileStream->eos() && fileStream->pos() < 0x100000) {
		if (curVersion > kResVersionSci0Sci1Early)
			fileStream->readByte();
		fileStream->skip(2);	// resId
		uint32 dwPacked = (curVersion < kResVersionSci2) ? fileStream->readUint16LE() : fileStream->readUint32LE();
		uint32 dwUnpacked = (curVersion < kResVersionSci2) ? fileStream->readUint16LE() : fileStream->readUint32LE();

		// The compression field is present, but bogus when
		// loading SCI3 volumes, the format is otherwise
		// identical to SCI2. We therefore get the compression
		// indicator here, but disregard it in the following
		// code.
		uint16 wCompression = fileStream->readUint16LE();

		if (fileStream->eos()) {
			delete fileStream;
			return curVersion;
		}

		int chk;

		if (curVersion == kResVersionSci0Sci1Early)
			chk = 4;
		else if (curVersion < kResVersionSci2)
			chk = 20;
		else
			chk = 32; // We don't need this, but include it for completeness

		int offs = curVersion < kResVersionSci11 ? 4 : 0;
		if ((curVersion < kResVersionSci2 && wCompression > chk)
				|| (curVersion == kResVersionSci2 && wCompression != 0 && wCompression != 32)
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
				curVersion = kResVersionSci2;
			} else if (curVersion == kResVersionSci2) {
				curVersion = kResVersionSci3;
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
		else if (curVersion >= kResVersionSci2)
			fileStream->seek(dwPacked, SEEK_CUR);
	}

	delete fileStream;

	if (!failed)
		return curVersion;

	// Failed to detect volume version
	return kResVersionUnknown;
}

#ifdef ENABLE_SCI32
bool ResourceManager::detectSci2Mac() {
	// SCI2 Mac games use the same volume format as SCI11 and so an extra initial check is required
	//  to differentiate between versions so that resource parsing can apply the correct resource
	//  type mapping before full SCI version detection occurs. A simple way to differentiate is to
	//  search for the SCI2 Object class' script resource in Mac volume files.
	Common::MacResManager macResManager;
	for (Common::List<ResourceSource *>::iterator it = _sources.begin(); it != _sources.end(); ++it) {
		ResourceSource *rsrc = *it;
		if (rsrc->getSourceType() == kSourceMacResourceFork) {
			if (macResManager.open(rsrc->getLocationName())) {
				const uint32 scriptTypeID = MKTAG('S', 'C', 'R', ' ');
				const uint32 objectScriptID = 64999;
				Common::SeekableReadStream *resource = macResManager.getResource(scriptTypeID, objectScriptID);
				bool objectScriptExists = false;
				if (resource != nullptr) {
					objectScriptExists = true;
					delete resource;
				}
				macResManager.close();
				if (objectScriptExists) {
					return true;
				}
			}
		}
	}
	return false;
}
#endif

bool ResourceManager::isBlacklistedPatch(const ResourceId &resId) const {
	if (!g_sci)
		return false;

	switch (g_sci->getGameId()) {
	case GID_SHIVERS:
	case GID_SQ6:
		// The SFX resource map patch in the Shivers interactive demo has
		// broken offsets for some sounds; ignore it so that the correct map
		// from RESSCI.000 will be used instead.
		// This also occurs in an early SQ6 demo; the original ignored the patch.
		return g_sci->isDemo() &&
			resId.getType() == kResourceTypeMap &&
			resId.getNumber() == 65535;
	case GID_PHANTASMAGORIA:
		// The GOG release of Phantasmagoria 1 merges all resources into a
		// single-disc bundle, but they also include the 65535.MAP from the
		// original game's CD 1, which does not contain the entries for sound
		// effects from later CDs. So, just ignore this map patch since the
		// correct maps will be found in the RESSCI.000 file. This also helps
		// eliminate user error when copying files from the original CDs, since
		// each CD had a different 65535.MAP patch file.
		return resId.getType() == kResourceTypeMap && resId.getNumber() == 65535;
	case GID_MOTHERGOOSE256:
		// The multilingual CD of Mothergoose 256 has a patch file SOUND.001
		//  which only contains a General MIDI track of the main music for
		//  Windows. Ignore this patch file for DOS so that the resource in
		//  RESOURCE.001 with all the normal tracks gets used. Bug #11243
		return g_sci->isCD() &&
			g_sci->getPlatform() == Common::kPlatformDOS &&
			resId.getType() == kResourceTypeSound &&
			resId.getNumber() == 1;
	case GID_SQ1:
		// In the SCI remake of SQ1, the bearded musicians at the bar in Kerona
		//  were removed for legal reasons. Sierra still included them, but the
		//  real graphics were suppressed with patch files that shrunk the
		//  guitarists to 13 pixels and replaced the drummer with an alien and
		//  the Sierra logo. We allow users to enable the original graphics,
		//  which we do by simply ignoring the patch files. In later versions
		//  the musicians were completely removed from the game scripts.
		return resId.getType() == kResourceTypeView &&
			(resId.getNumber() == 433 || resId.getNumber() == 533) &&
			ConfMan.getBool("enable_bearded_musicians") &&
			testResource(resId); // ensure that the original view exists, just to be safe
	default:
		return false;
	}
}

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source, ResourceType resourceType, uint16 resourceNr, uint32 tuple) {
	Common::SeekableReadStream *fileStream = nullptr;
	Resource *newrsc = nullptr;
	ResourceId resId = ResourceId(resourceType, resourceNr, tuple);
	ResourceType checkForType = resourceType;

	if (isBlacklistedPatch(resId)) {
		debug("Skipping blacklisted patch file %s", source->getLocationName().toString().c_str());
		delete source;
		return;
	}

	// base36 encoded patches (i.e. audio36 and sync36) have the same type as their non-base36 encoded counterparts
	if (checkForType == kResourceTypeAudio36)
		checkForType = kResourceTypeAudio;
	else if (checkForType == kResourceTypeSync36)
		checkForType = kResourceTypeSync;

	if (source->_resourceFile) {
		fileStream = source->_resourceFile->createReadStream();
	} else {
		Common::File *file = new Common::File();
		if (!file->open(source->getLocationName())) {
			warning("ResourceManager::processPatch(): failed to open %s", source->getLocationName().toString().c_str());
			delete source;
			delete file;
			return;
		}
		fileStream = file;
	}

	int fsize = fileStream->size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", source->getLocationName().toString().c_str());
		delete source;
		delete fileStream;
		return;
	}

	byte patchType;
	if (fileStream->readUint32BE() == MKTAG('R','I','F','F')) {
		fileStream->seek(-4, SEEK_CUR);
		patchType = kResourceTypeAudio;
	} else {
		fileStream->seek(-4, SEEK_CUR);
		patchType = convertResType(fileStream->readByte());
	}

	enum {
		kExtraHeaderSize    = 2, ///< extra header used in gfx resources
		kViewHeaderSize     = 22 ///< extra header used in view resources
	};

	int32 patchDataOffset = kResourceHeaderSize;
	if (_volVersion < kResVersionSci11) {
		patchDataOffset += fileStream->readByte();
	} else {
		switch (patchType) {
		case kResourceTypeView:
			fileStream->seek(3, SEEK_SET);
			patchDataOffset += fileStream->readByte() + kViewHeaderSize + kExtraHeaderSize;
			break;
		case kResourceTypePic:
			if (_volVersion < kResVersionSci2) {
				fileStream->seek(3, SEEK_SET);
				patchDataOffset += fileStream->readByte() + kViewHeaderSize + kExtraHeaderSize;
			} else {
				patchDataOffset += kExtraHeaderSize;
			}
			break;
		case kResourceTypePalette:
			fileStream->seek(3, SEEK_SET);
			patchDataOffset += fileStream->readByte() + kExtraHeaderSize;
			break;
		case kResourceTypeAudio:
		case kResourceTypeAudio36:
#ifdef ENABLE_SCI32
		case kResourceTypeWave:
		case kResourceTypeVMD:
		case kResourceTypeDuck:
		case kResourceTypeClut:
		case kResourceTypeTGA:
		case kResourceTypeZZZ:
		case kResourceTypeEtc:
#endif
			patchDataOffset = 0;
			break;
		default:
			fileStream->seek(1, SEEK_SET);
			patchDataOffset += fileStream->readByte();
			break;
		}
	}

	delete fileStream;

	if (patchType != checkForType) {
		debug("Patching %s failed - resource type mismatch", source->getLocationName().toString().c_str());
		delete source;
		return;
	}

	if (patchDataOffset >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
		      source->getLocationName().toString().c_str(), patchDataOffset, fsize);
		delete source;
		return;
	}

	// Overwrite everything, because we're patching
	newrsc = updateResource(resId, source, 0, fsize - patchDataOffset, source->getLocationName());
	newrsc->_headerSize = patchDataOffset;

	debugC(1, kDebugLevelResMan, "Patching %s - OK", source->getLocationName().toString().c_str());
}

ResourceId convertPatchNameBase36(ResourceType type, const Common::String &filename) {
	// The base36 encoded resource contains the following:
	// uint16 resourceId, byte noun, byte verb, byte cond, byte seq

	// Skip patch type character
	uint16 resourceNr = strtol(Common::String(filename.c_str() + 1, 3).c_str(), nullptr, 36); // 3 characters
	uint16 noun = strtol(Common::String(filename.c_str() + 4, 2).c_str(), nullptr, 36);       // 2 characters
	uint16 verb = strtol(Common::String(filename.c_str() + 6, 2).c_str(), nullptr, 36);       // 2 characters
	// Skip '.'
	uint16 cond = strtol(Common::String(filename.c_str() + 9, 2).c_str(), nullptr, 36);       // 2 characters
	uint16 seq = strtol(Common::String(filename.c_str() + 11, 1).c_str(), nullptr, 36);       // 1 character

	return ResourceId(type, resourceNr, noun, verb, cond, seq);
}

void ResourceManager::readResourcePatchesBase36() {
	// The base36 encoded audio36 and sync36 resources use a different naming scheme, because they
	// cannot be described with a single resource number, but are a result of a
	// <number, noun, verb, cond, seq> tuple. Please don't be confused with the normal audio patches
	// (*.aud) and normal sync patches (*.syn). audio36 patches can be seen for example in the AUD
	// folder of GK1CD, and are like this file: @0CS0M00.0X1. GK1CD is the first game where these
	// have been observed. The actual audio36 and sync36 resources exist in SCI1.1 as well, but the
	// first game where external patch files for them have been found is GK1CD. The names of these
	// files are base36 encoded, and we handle their decoding here. audio36 files start with a '@',
	// whereas sync36 start with a '#'. Mac versions begin with 'A' (probably meaning AIFF). Torin
	// has several that begin with 'B'.

	Common::ArchiveMemberList files;

	for (int i = kResourceTypeAudio36; i <= kResourceTypeSync36; ++i) {
		files.clear();

		// audio36 resources start with a @, A, or B
		// sync36 resources start with a #, S, or T
		if (i == kResourceTypeAudio36) {
			SearchMan.listMatchingMembers(files, "@???????.???");
			SearchMan.listMatchingMembers(files, "A???????.???");
			SearchMan.listMatchingMembers(files, "B???????.???");
		} else {
			SearchMan.listMatchingMembers(files, "\\#???????.???");
#ifdef ENABLE_SCI32
			SearchMan.listMatchingMembers(files, "S???????.???");
			SearchMan.listMatchingMembers(files, "T???????.???");
#endif
		}

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			Common::Path path = (*x)->getPathInArchive();
			Common::String name(path.baseName());
			name.toUppercase();

			// The S/T prefixes often conflict with non-patch files and generate
			// spurious warnings about invalid patches
			if (name.hasSuffix(".DLL") || name.hasSuffix(".EXE") || name.hasSuffix(".TXT") || name.hasSuffix(".OLD") || name.hasSuffix(".WIN") || name.hasSuffix(".DOS") ||
				name.hasSuffix(".HLP") || name.hasSuffix(".DRV")) {
				continue;
			}

			ResourceId resource36 = convertPatchNameBase36((ResourceType)i, name);

			/*
			if (i == kResourceTypeAudio36)
				debug("audio36 patch: %s => %s. tuple:%d, %s\n", name.c_str(), inputName.c_str(), resource36.tuple, resource36.toString().c_str());
			else
				debug("sync36 patch: %s => %s. tuple:%d, %s\n", name.c_str(), inputName.c_str(), resource36.tuple, resource36.toString().c_str());
			*/

			// Make sure that the audio patch is a valid resource
			if (i == kResourceTypeAudio36) {
				Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(path);
				uint32 tag = stream->readUint32BE();

				if (tag == MKTAG('R','I','F','F') || tag == MKTAG('F','O','R','M')) {
					delete stream;
					processWavePatch(resource36, path);
					continue;
				}

				// Check for SOL as well
				tag = (tag << 16) | stream->readUint16BE();

				if (tag != MKTAG('S','O','L',0)) {
					delete stream;
					continue;
				}

				delete stream;
			}

			ResourceSource *psrcPatch = new PatchResourceSource(path);
			processPatch(psrcPatch, (ResourceType)i, resource36.getNumber(), resource36.getTuple());
		}
	}
}

void ResourceManager::readResourcePatches() {
	// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
	// this function tries to read patch file with any supported naming scheme,
	// regardless of s_sciVersion value

	Common::Path mask, name;
	Common::ArchiveMemberList files;
	uint16 resourceNr = 0;
	const char *szResType;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeView; i < kResourceTypeInvalid; ++i) {
		// Ignore the types that can't be patched (and Robot/VMD is handled externally for now)
		if (!s_resourceTypeSuffixes[i] || (i >= kResourceTypeRobot && i != kResourceTypeChunk))
			continue;

		files.clear();
		szResType = getResourceTypeName((ResourceType)i);
		// SCI0 naming - type.nnn
		mask = szResType;
		mask.appendInPlace(".###");
		SearchMan.listMatchingMembers(files, mask);
		// SCI1 and later naming - nnn.typ
		mask = "*.";
		mask.appendInPlace(s_resourceTypeSuffixes[i]);
		SearchMan.listMatchingMembers(files, mask);

		if (i == kResourceTypeView) {
			SearchMan.listMatchingMembers(files, "*.v16");	// EGA SCI1 view patches
			SearchMan.listMatchingMembers(files, "*.v32");	// Amiga SCI1 view patches
			SearchMan.listMatchingMembers(files, "*.v64");	// Amiga AGA SCI1 (i.e. Longbow) view patches
		} else if (i == kResourceTypePic) {
			SearchMan.listMatchingMembers(files, "*.p16");	// EGA SCI1 picture patches
			SearchMan.listMatchingMembers(files, "*.p32");	// Amiga SCI1 picture patches
			SearchMan.listMatchingMembers(files, "*.p64");	// Amiga AGA SCI1 (i.e. Longbow) picture patches
		} else if (i == kResourceTypeScript) {
			// SCI3 (we can't use getSciVersion() at this point)
			SearchMan.listMatchingMembers(files, "*.csc");
		}

		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
			bool bAdd = false;
			name = (*x)->getPathInArchive();
			Common::String baseName(name.baseName());

			// SCI1 scheme
			if (Common::isDigit(baseName[0])) {
				char *end = nullptr;
				resourceNr = strtol(baseName.c_str(), &end, 10);
				bAdd = (*end == '.'); // Ensure the next character is the period
			} else {
				// SCI0 scheme
				int resname_len = strlen(szResType);
				if (scumm_strnicmp(baseName.c_str(), szResType, resname_len) == 0
					&& !Common::isAlpha(baseName[resname_len + 1])) {
					resourceNr = atoi(baseName.c_str() + resname_len + 1);
					bAdd = true;
				}
			}

			if (bAdd) {
				psrcPatch = new PatchResourceSource(name);
				processPatch(psrcPatch, (ResourceType)i, resourceNr);
			}
		}
	}
}

int ResourceManager::readResourceMapSCI0(ResourceSource *map) {
	Common::SeekableReadStream *fileStream = nullptr;
	ResourceType type = kResourceTypeInvalid;	// to silence a false positive in MSVC

	if (map->_resourceFile) {
		fileStream = map->_resourceFile->createReadStream();
		if (!fileStream)
			return SCI_ERROR_RESMAP_NOT_FOUND;
	} else {
		Common::File *file = new Common::File();
		if (!file->open(map->getLocationName())) {
			delete file;
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}
		fileStream = file;
	}

	fileStream->seek(0, SEEK_SET);

	byte bMask = (_mapVersion >= kResVersionSci1Middle) ? 0xF0 : 0xFC;
	byte bShift = (_mapVersion >= kResVersionSci1Middle) ? 28 : 26;

	do {
		// King's Quest 5 FM-Towns uses a 7 byte version of the SCI1 Middle map,
		// splitting the type from the id.
		if (_mapVersion == kResVersionKQ5FMT)
			type = convertResType(fileStream->readByte());

		uint16 id = fileStream->readUint16LE();
		uint32 offset = fileStream->readUint32LE();

		if (fileStream->eos() || fileStream->err()) {
			delete fileStream;
			warning("Error while reading %s", map->getLocationName().toString().c_str());
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}

		if (offset == 0xFFFFFFFF)
			break;

		uint16 number;
		if (_mapVersion == kResVersionKQ5FMT) {
			number = id;
		} else {
			type = convertResType(id >> 11);
			number = id & 0x7FF;
		}

		ResourceId resId = ResourceId(type, number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			ResourceSource *source = findVolume(map, offset >> bShift);
			if (!source) {
				warning("Could not get volume for resource %d, VolumeID %d", id, offset >> bShift);
				if (_mapVersion != _volVersion) {
					warning("Retrying with the detected volume version instead");
					warning("Map version was: %d, retrying with: %d", _mapVersion, _volVersion);
					_mapVersion = _volVersion;
					bMask = (_mapVersion == kResVersionSci1Middle) ? 0xF0 : 0xFC;
					bShift = (_mapVersion == kResVersionSci1Middle) ? 28 : 26;
					source = findVolume(map, offset >> bShift);
					if (!source) {
						delete fileStream;
						warning("Still couldn't find the volume");
						return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
					}
				} else {
					delete fileStream;
					return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
				}
			}

			addResource(resId, source, offset & ((((byte)~bMask) << 24) | 0xFFFFFF), 0, map->getLocationName());
		}
	} while (!fileStream->eos());

	delete fileStream;
	return 0;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map) {
	Common::SeekableReadStream *fileStream = nullptr;

	if (map->_resourceFile) {
		fileStream = map->_resourceFile->createReadStream();
		if (!fileStream)
			return SCI_ERROR_RESMAP_NOT_FOUND;
	} else {
		Common::File *file = new Common::File();
		if (!file->open(map->getLocationName())) {
			delete file;
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}
		fileStream = file;
	}

	resource_index_t resMap[32];
	memset(resMap, 0, sizeof(resource_index_t) * 32);
	byte type = 0, prevtype = 0;
	byte nEntrySize = _mapVersion == kResVersionSci11 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
	if (isKoreanMessageMap(map))
		nEntrySize = SCI1_RESMAP_ENTRIES_SIZE;
	ResourceId resId;

	// Read resource type and offsets to resource offsets block from .MAP file
	// The last entry has type=0xFF (0x1F) and offset equals to map file length
	do {
		type = fileStream->readByte() & 0x1F;
		resMap[type].wOffset = fileStream->readUint16LE();
		if (fileStream->eos()) {
			delete fileStream;
			warning("Premature end of file %s", map->getLocationName().toString().c_str());
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}

		resMap[prevtype].wSize = (resMap[type].wOffset
		                          - resMap[prevtype].wOffset) / nEntrySize;
		prevtype = type;
	} while (type != 0x1F); // the last entry is FF

	// reading each type's offsets
	uint32 fileOffset = 0;
	for (type = 0; type < 32; type++) {
		if (resMap[type].wOffset == 0) // this resource does not exist in map
			continue;
		fileStream->seek(resMap[type].wOffset);
		for (int i = 0; i < resMap[type].wSize; i++) {
			uint16 number = fileStream->readUint16LE();
			int volume_nr = 0;
			if (_mapVersion == kResVersionSci11 && !isKoreanMessageMap(map)) {
				// offset stored in 3 bytes
				fileOffset = fileStream->readUint16LE();
				fileOffset |= fileStream->readByte() << 16;
				fileOffset <<= 1;
			} else {
				// offset/volume stored in 4 bytes
				fileOffset = fileStream->readUint32LE();
				if (_mapVersion < kResVersionSci11 && !isKoreanMessageMap(map)) {
					volume_nr = fileOffset >> 28; // most significant 4 bits
					fileOffset &= 0x0FFFFFFF;     // least significant 28 bits
				} else {
					// in SCI32 it's a plain offset
				}
			}
			if (fileStream->eos() || fileStream->err()) {
				delete fileStream;
				warning("Error while reading %s", map->getLocationName().toString().c_str());
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = ResourceId(convertResType(type), number);
			// NOTE: We add the map's volume number here to the specified volume number
			// for SCI2.1 and SCI3 maps that are not resmap.000. The resmap.* files' numbers
			// need to be used in concurrence with the volume specified in the map to get
			// the actual resource file.
			int mapVolumeNr = volume_nr + map->_volumeNumber;
			ResourceSource *source = findVolume(map, mapVolumeNr);

			if (!source) {
				delete fileStream;
				warning("Could not get volume for resource %d, VolumeID %d", number, mapVolumeNr);
				return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
			}

			Resource *resource =  nullptr;
			if (!_resMap.tryGetVal(resId,resource)) {
				addResource(resId, source, fileOffset, 0, map->getLocationName());
			} else {
				// If the resource is already present in a volume, change it to
				// the new content (but only in a volume, so as not to overwrite
				// external patches - refer to bug #5796).
				// This is needed at least for the German version of Pharkas.
				// That version contains several duplicate resources INSIDE the
				// resource data files like fonts, views, scripts, etc. Thus,
				// if we use the first entries in the resource file, half of the
				// game will be English and umlauts will also be missing :P
				if (resource->_source->getSourceType() == kSourceVolume) {
					updateResource(resId, source, fileOffset, 0, map->getLocationName());
				}
			}

#ifdef ENABLE_SCI32
			// Different CDs may have different audio maps on each disc. The
			// ResourceManager does not know how to deal with this; it expects
			// each resource ID to be unique across an entire game. To work
			// around this problem, all audio maps from this disc must be
			// processed immediately, since they will be replaced by the audio
			// map from the next disc on the next call to readResourceMapSCI1
			if (_multiDiscAudio && resId.getType() == kResourceTypeMap) {
				IntMapResourceSource *audioMap = new IntMapResourceSource(source->getLocationName(), mapVolumeNr, resId.getNumber());
				addSource(audioMap);

				Common::Path volumeName;
				if (mapVolumeNr == kResPatVolumeNumber) {
					if (resId.getNumber() == 65535) {
						volumeName = "RESSCI.PAT";
					} else {
						volumeName = "RESAUD.001";
					}
				} else if (resId.getNumber() == 65535) {
					volumeName = Common::Path(Common::String::format("RESSFX.%03d", mapVolumeNr));

					if (g_sci && g_sci->getGameId() == GID_RAMA && !Common::File::exists(volumeName)) {
						if (Common::File::exists("RESOURCE.SFX")) {
							volumeName = "RESOURCE.SFX";
						} else if (Common::File::exists("RESSFX.001")) {
							volumeName = "RESSFX.001";
						}
					}
				} else {
					volumeName = Common::Path(Common::String::format("RESAUD.%03d", mapVolumeNr));
				}

				ResourceSource *audioVolume = addSource(new AudioVolumeResourceSource(this, volumeName, audioMap, mapVolumeNr));
				if (!audioMap->_scanned) {
					audioVolume->_scanned = true;
					audioMap->_scanned = true;
					audioMap->scanSource(this);
				}
			}
#endif
		}
	}

	delete fileStream;
	return 0;
}

struct MacResTag {
	uint32 tag;
	ResourceType type;
};

static const MacResTag macResTagMap[] = {
	{ MKTAG('V','5','6',' '), kResourceTypeView },
	{ MKTAG('P','5','6',' '), kResourceTypePic },
	{ MKTAG('S','C','R',' '), kResourceTypeScript },
	{ MKTAG('T','E','X',' '), kResourceTypeText },
	{ MKTAG('S','N','D',' '), kResourceTypeSound },
	{ MKTAG('V','O','C',' '), kResourceTypeVocab },
	{ MKTAG('F','O','N',' '), kResourceTypeFont },
	{ MKTAG('C','U','R','S'), kResourceTypeCursor },
	{ MKTAG('c','r','s','r'), kResourceTypeCursor },
	{ MKTAG('P','a','t',' '), kResourceTypePatch },
	{ MKTAG('P','A','L',' '), kResourceTypePalette },
	{ MKTAG('s','n','d',' '), kResourceTypeAudio },
	{ MKTAG('M','S','G',' '), kResourceTypeMessage },
	{ MKTAG('H','E','P',' '), kResourceTypeHeap },
	{ MKTAG('I','B','I','N'), kResourceTypeMacIconBarPictN },
	{ MKTAG('I','B','I','S'), kResourceTypeMacIconBarPictS },
	{ MKTAG('P','I','C','T'), kResourceTypeMacPict },
	{ MKTAG('S','Y','N',' '), kResourceTypeSync },
	{ MKTAG('S','Y','N','C'), kResourceTypeSync }
};

static Common::Array<uint32> resTypeToMacTags(ResourceType type) {
	Common::Array<uint32> tags;

	for (uint32 i = 0; i < ARRAYSIZE(macResTagMap); i++)
		if (macResTagMap[i].type == type)
			tags.push_back(macResTagMap[i].tag);

	return tags;
}

void MacResourceForkResourceSource::scanSource(ResourceManager *resMan) {
	if (!_macResMan->open(getLocationName()))
		error("%s is not a valid Mac resource fork", getLocationName().toString().c_str());

	Common::MacResTagArray tagArray = _macResMan->getResTagArray();

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

		Common::MacResIDArray idArray = _macResMan->getResIDArray(tagArray[i]);

		for (uint32 j = 0; j < idArray.size(); j++) {
			ResourceId resId;

			// Check to see if we've got a base36 encoded resource name
			if (type == kResourceTypeAudio) {
				Common::String resourceName = _macResMan->getResName(tagArray[i], idArray[j]);

				// If we have a file name on an audio resource, we've got an audio36
				// resource. Parse the file name to get the id.
				if (!resourceName.empty() && resourceName[0] == '@')
					resId = convertPatchNameBase36(kResourceTypeAudio36, resourceName);
				else
					resId = ResourceId(type, idArray[j]);
			} else if (type == kResourceTypeSync) {
				Common::String resourceName = _macResMan->getResName(tagArray[i], idArray[j]);

				// Same as with audio36 above
				if (!resourceName.empty() &&
					(resourceName[0] == '#' ||
					 resourceName[0] == 'S' || // Most SCI32 games
					 resourceName[0] == 'T'))  // Torin syncs start with T or S
					resId = convertPatchNameBase36(kResourceTypeSync36, resourceName);
				else
					resId = ResourceId(type, idArray[j]);
			} else {
				// Otherwise, we're just going with the id that was given
				resId = ResourceId(type, idArray[j]);
			}

			// Overwrite Resource instance. Resource forks may contain patches.
			// The size will be filled in later by decompressResource()
			resMan->updateResource(resId, this, 0, getLocationName());
		}
	}
}

bool ResourceManager::validateResource(const ResourceId &resourceId, const Common::Path &sourceMapLocation, const Common::Path &sourceName, const uint32 offset, const uint32 size, const uint32 sourceSize) const {
	if (size != 0) {
		if (offset + size > sourceSize) {
			warning("Resource %s from %s points beyond end of %s (%u + %u > %u)", resourceId.toString().c_str(), sourceMapLocation.toString().c_str(), sourceName.toString().c_str(), offset, size, sourceSize);
			return false;
		}
	} else {
		if (offset >= sourceSize) {
			warning("Resource %s from %s points beyond end of %s (%u >= %u)", resourceId.toString().c_str(), sourceMapLocation.toString().c_str(), sourceName.toString().c_str(), offset, sourceSize);
			return false;
		}
	}

	return true;
}

Resource *ResourceManager::addResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size, const Common::Path &sourceMapLocation) {
	// Adding new resource only if it does not exist
	// Hoyle 4 contains each audio resource twice. The first file is in an unknown
	// format and only static sounds are heard when it's played. The second file
	// is a typical SOL audio file. We therefore skip the first audio file and add
	// second one for this game.
	if (_resMap.contains(resId) == false || (resId.getType() == kResourceTypeAudio && g_sci && g_sci->getGameId() == GID_HOYLE4)) {
		return updateResource(resId, src, offset, size, sourceMapLocation);
	} else {
		return _resMap.getVal(resId);
	}
}

Resource *ResourceManager::updateResource(ResourceId resId, ResourceSource *src, uint32 size, const Common::Path &sourceMapLocation) {
	uint32 offset = 0;
	if (_resMap.contains(resId)) {
		const Resource *res = _resMap.getVal(resId);
		offset = res->_fileOffset;
	}
	return updateResource(resId, src, offset, size, sourceMapLocation);
}

Resource *ResourceManager::updateResource(ResourceId resId, ResourceSource *src, uint32 offset, uint32 size, const Common::Path &sourceMapLocation) {
	// Update a patched resource, whether it exists or not
	Resource *res = _resMap.getValOrDefault(resId, nullptr);

	// When pulling from resource the "main" file may not even
	// exist as both forks may be combined into MacBin
	Common::SeekableReadStream *volumeFile = nullptr;
	if (src->getSourceType() != kSourceMacResourceFork) {
		volumeFile = getVolumeFile(src);
		if (volumeFile == nullptr) {
			error("Could not open %s for reading", src->getLocationName().toString().c_str());
		}
	}

	AudioVolumeResourceSource *avSrc = dynamic_cast<AudioVolumeResourceSource *>(src);
	if (avSrc != nullptr && !avSrc->relocateMapOffset(offset, size)) {
		warning("Compressed volume %s does not contain a valid entry for %s (map offset %u)", src->getLocationName().toString().c_str(), resId.toString().c_str(), offset);
		_hasBadResources = true;
		if (volumeFile != nullptr)
			disposeVolumeFileStream(volumeFile, src);
		return res;
	}

	// Resources from MacResourceForkResourceSource do not have a source size
	// since the source "volume file" is the empty data fork, and they don't
	// have an offset either since the MacResManager handles this, so trying to
	// validate these resources using the normal validation would always fail
	if (src->getSourceType() == kSourceMacResourceFork ||
		validateResource(resId, sourceMapLocation, src->getLocationName(), offset, size, volumeFile->size())) {
		if (res == nullptr) {
			res = new Resource(this, resId);
			_resMap.setVal(resId, res);
		}

		res->_status = kResStatusNoMalloc;
		res->_source = src;
		res->_headerSize = 0;
		res->_fileOffset = offset;
		res->_size = size;
	} else {
		_hasBadResources = true;
	}

	if (volumeFile != nullptr)
		disposeVolumeFileStream(volumeFile, src);
	return res;
}

int Resource::readResourceInfo(ResVersion volVersion, Common::SeekableReadStream *file,
									  uint32 &szPacked, ResourceCompression &compression) {
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked dwUnpacked wCompression} = 13 bytes
	uint16 w, number;
	uint32 wCompression, szUnpacked;
	ResourceType type;

	if (file->size() == 0)
		return SCI_ERROR_EMPTY_RESOURCE;

	switch (volVersion) {
	case kResVersionSci0Sci1Early:
	case kResVersionSci1Middle:
		w = file->readUint16LE();
		type = _resMan->convertResType(w >> 11);
		number = w & 0x7FF;
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci1Late:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint16LE() - 4;
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
	case kResVersionSci11:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint16LE();
		szUnpacked = file->readUint16LE();
		wCompression = file->readUint16LE();
		break;
#ifdef ENABLE_SCI32
	case kResVersionSci2:
	case kResVersionSci3:
		type = _resMan->convertResType(file->readByte());
		number = file->readUint16LE();
		szPacked = file->readUint32LE();
		szUnpacked = file->readUint32LE();

		// The same comment applies here as in
		// detectVolVersion regarding SCI3. We ignore the
		// compression field for SCI3 games, but must presume
		// it exists in the file.
		wCompression = file->readUint16LE();

		if (volVersion == kResVersionSci3)
			wCompression = szPacked != szUnpacked ? 32 : 0;

		break;
#endif
	default:
		return SCI_ERROR_RESMAP_INVALID_ENTRY;
	}

	// check if there were errors while reading
	if ((file->eos() || file->err()))
		return SCI_ERROR_IO_ERROR;

	_id = ResourceId(type, number);
	_size = szUnpacked;

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

	return (compression == kCompUnknown) ? SCI_ERROR_UNKNOWN_COMPRESSION : SCI_ERROR_NONE;
}

int Resource::decompress(ResVersion volVersion, Common::SeekableReadStream *file) {
	int errorNum;
	uint32 szPacked = 0;
	ResourceCompression compression = kCompUnknown;

	// fill resource info
	errorNum = readResourceInfo(volVersion, file, szPacked, compression);
	if (errorNum)
		return errorNum;

	// getting a decompressor
	Decompressor *dec = nullptr;
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
		error("Resource %s: Compression method %d not supported", _id.toString().c_str(), compression);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	byte *ptr = new byte[_size];
	_data = ptr;
	_status = kResStatusAllocated;
	errorNum = ptr ? dec->unpack(file, ptr, szPacked, _size) : SCI_ERROR_RESOURCE_TOO_BIG;
	if (errorNum) {
		unalloc();
	} else {
		// At least Lighthouse puts sound effects in RESSCI.00n/RESSCI.PAT
		// instead of using a RESOURCE.SFX
		if (getType() == kResourceTypeAudio) {
			const uint8 headerSize = ptr[1];
			if (headerSize < 11) {
				error("Unexpected audio header size for %s: should be >= 11, but got %d", _id.toString().c_str(), headerSize);
			}
			const uint32 audioSize = READ_LE_UINT32(ptr + 9);
			const uint32 calculatedTotalSize = audioSize + headerSize + kResourceHeaderSize;
			if (calculatedTotalSize != _size) {
				warning("Unexpected audio file size: the size of %s in %s is %d, but the volume says it should be %d", _id.toString().c_str(), _source->getLocationName().toString().c_str(), calculatedTotalSize, _size);
			}
			_size = MIN(_size - kResourceHeaderSize, headerSize + audioSize);
		}
	}

	delete dec;
	return errorNum;
}

ResourceCompression ResourceManager::getViewCompression() {
	int viewsTested = 0;

	// Test 10 views to see if any are compressed
	for (int i = 0; i < 1000; i++) {
		Common::SeekableReadStream *fileStream = nullptr;
		Resource *res = testResource(ResourceId(kResourceTypeView, i));

		if (!res)
			continue;

		if (res->_source->getSourceType() != kSourceVolume)
			continue;

		fileStream = getVolumeFile(res->_source);

		if (!fileStream)
			continue;
		fileStream->seek(res->_fileOffset, SEEK_SET);

		uint32 szPacked;
		ResourceCompression compression;

		if (res->readResourceInfo(_volVersion, fileStream, szPacked, compression)) {
			disposeVolumeFileStream(fileStream, res->_source);
			continue;
		}

		disposeVolumeFileStream(fileStream, res->_source);

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
			// Skip views coming from patch files
			if (res->_source->getSourceType() == kSourcePatch)
				continue;

			switch (res->getUint8At(1)) {
			case 128:
				// If the 2nd byte is 128, it's a VGA game.
				// However, Longbow Amiga (AGA, 64 colors), also sets this byte
				// to 128, but it's a mixed VGA/Amiga format. Detect this from
				// the platform here.
				if (g_sci && g_sci->getPlatform() == Common::kPlatformAmiga)
					return kViewAmiga64;

				return kViewVga;
			case 0: {
				// EGA or Amiga, try to read as Amiga view

				if (res->size() < 10)
					return kViewUnknown;

				// Read offset of first loop
				uint16 offset = res->getUint16LEAt(8);

				if (offset + 6U >= res->size())
					return kViewUnknown;

				// Read offset of first cel
				offset = res->getUint16LEAt(offset + 4);

				if (offset + 4U >= res->size())
					return kViewUnknown;

				// Check palette offset, amiga views have no palette
				if (res->getUint16LEAt(6) != 0)
					return kViewEga;

				uint16 width = res->getUint16LEAt(offset);
				offset += 2;
				uint16 height = res->getUint16LEAt(offset);
				offset += 6;

				// To improve the heuristic, we skip very small views
				if (height < 10)
					continue;

				// Check that the RLE data stays within bounds
				int y;
				for (y = 0; y < height; y++) {
					int x = 0;

					while ((x < width) && (offset < res->size())) {
						byte op = res->getUint8At(offset++);
						x += (op & 0x07) ? op & 0x07 : op >> 3;
					}

					// Make sure we got exactly the right number of pixels for this row
					if (x != width)
						return kViewEga;
				}

				return kViewAmiga;
			}

			default:
				break;
			}
		}
	}

	// this may happen if there are serious system issues (or trying to add a broken game)
	warning("resMan: Couldn't find any views");
	return kViewUnknown;
}

// to detect selector "wordFail" in LE vocab resource
static const byte detectSci21EarlySignature[] = {
	10, // size of signature
	0x08, 0x00, 'w', 'o', 'r', 'd', 'F', 'a', 'i', 'l'
};

// to detect selector "wordFail" in BE vocab resource (SCI2.1 Early)
static const byte detectSci21EarlyBESignature[] = {
	10, // size of signature
	0x00, 0x08, 'w', 'o', 'r', 'd', 'F', 'a', 'i', 'l'
};

// to detect new kString calling to detect SCI2.1 Late
static const byte detectSci21NewStringSignature[] = {
	8, // size of signature
	0x78, // push1
	0x78, // push1
	0x39, 0x09, // pushi 09
	0x59, 0x01, // rest 01
	0x43, 0x5c, // callk String
};

bool ResourceManager::checkResourceDataForSignature(Resource *resource, const byte *signature) {
	byte signatureSize = *signature;

	signature++; // skip over size byte
	if (signatureSize < 4)
		error("resource signature is too small, internal error");
	if (signatureSize > resource->size())
		return false;

	const uint32 signatureDWord = READ_UINT32(signature);
	signature += 4; signatureSize -= 4;

	const uint32 searchLimit = resource->size() - signatureSize + 1;
	uint32 DWordOffset = 0;
	while (DWordOffset < searchLimit) {
		if (signatureDWord == resource->getUint32At(DWordOffset)) {
			// magic DWORD found, check if the rest matches as well
			uint32 offset = DWordOffset + 4;
			uint32 signaturePos  = 0;
			while (signaturePos < signatureSize) {
				if (resource->getUint8At(offset) != signature[signaturePos])
					break;
				offset++;
				signaturePos++;
			}
			if (signaturePos >= signatureSize)
				return true; // signature found
		}
		DWordOffset++;
	}
	return false;
}

bool ResourceManager::checkResourceForSignatures(ResourceType resourceType, uint16 resourceNr, const byte *signature1, const byte *signature2) {
	Resource *resource = findResource(ResourceId(resourceType, resourceNr), false);

	if (resource) {
		// resource found and loaded, check for signatures
		if (signature1) {
			if (checkResourceDataForSignature(resource, signature1))
				return true;
		}
		if (signature2) {
			if (checkResourceDataForSignature(resource, signature2))
				return true;
		}
	}
	return false;
}

void ResourceManager::detectSciVersion() {
	// We use the view compression to set a preliminary s_sciVersion for the sake of getResourceInfo
	// Pretend we have a SCI0 game
	g_sciVersion = SCI_VERSION_0_EARLY;
	bool oldDecompressors = true;

	ResourceCompression viewCompression;
#ifdef ENABLE_SCI32
	viewCompression = getViewCompression();
#else
	if (_volVersion >= kResVersionSci2) {
		// SCI32 support isn't built in, thus view detection will fail
		viewCompression = kCompUnknown;
	} else {
		viewCompression = getViewCompression();
	}
#endif

	if (viewCompression != kCompLZW) {
		// If it's a different compression type from kCompLZW, the game is probably
		// SCI_VERSION_1_EGA_ONLY or later. If the views are uncompressed, it is
		// likely not an early disk game.
		g_sciVersion = SCI_VERSION_1_EGA_ONLY;
		oldDecompressors = false;
	}

	// Set view type
	if (viewCompression == kCompDCL
		|| _volVersion == kResVersionSci11 // pq4demo
		|| _volVersion == kResVersionSci11Mac
#ifdef ENABLE_SCI32
		|| viewCompression == kCompSTACpack
		|| _volVersion == kResVersionSci2 // kq7
#endif
		) {
		// SCI1.1 VGA views
		_viewType = kViewVga11;
	} else {
#ifdef ENABLE_SCI32
		// Otherwise we detect it from a view
		_viewType = detectViewType();
#else
		if (_volVersion == kResVersionSci2 && viewCompression == kCompUnknown) {
			// A SCI32 game, but SCI32 support is disabled. Force the view type
			// to kViewVga11, as we can't read from the game's resource files
			_viewType = kViewVga11;
		} else {
			_viewType = detectViewType();
		}
#endif
	}

	if (_volVersion == kResVersionSci11Mac) {
		Resource *res = testResource(ResourceId(kResourceTypeScript, 64920));
		// Distinguish between SCI1.1 and SCI32 games here. SCI32 games will
		// always include script 64920 (the Array class). Note that there are
		// no Mac SCI2 games. Yes, that means that GK1 Mac is SCI2.1 and not SCI2.
		// There are also no SCI3 Mac games, the final Mac games are Late SCI2.1
		// versions of SCI3 PC games. That is, the Mac scripts are compiled as
		// separate script and hunk resources instead of the SCI3 script format.
		if (res) {
			g_sciVersion = SCI_VERSION_2_1_EARLY; // we check for SCI2.1 specifics a bit later
		} else {
			g_sciVersion = SCI_VERSION_1_1;
			return;
		}
	}

	// Handle SCI32 versions here
	if (g_sciVersion != SCI_VERSION_2_1_EARLY) {
		if (_volVersion >= kResVersionSci2) {
			Common::List<ResourceId> heaps = listResources(kResourceTypeHeap);
			bool hasHeapResources = !heaps.empty();

			// SCI2.1/3 and SCI1 Late resource maps are the same, except that
			// SCI1 Late resource maps have the resource types or'd with
			// 0x80. We differentiate between SCI2 and SCI2.1/3 based on that.
			if (_mapVersion == kResVersionSci1Late) {
				g_sciVersion = SCI_VERSION_2;
				return;
			} else if (hasHeapResources) {
				g_sciVersion = SCI_VERSION_2_1_EARLY; // exact SCI2.1 version is checked a bit later
			} else {
				g_sciVersion = SCI_VERSION_3;
				return;
			}
		}
	}

	if (g_sciVersion == SCI_VERSION_2_1_EARLY) {
		// we only know that it's SCI2.1, not which exact version it is

		// check, if selector "wordFail" inside vocab 997 exists, if it does it's SCI2.1 Early
		if ((checkResourceForSignatures(kResourceTypeVocab, 997, detectSci21EarlySignature, detectSci21EarlyBESignature))) {
			// found -> it is SCI2.1 early
			return;
		}

		g_sciVersion = SCI_VERSION_2_1_MIDDLE;
		if (checkResourceForSignatures(kResourceTypeScript, 64918, detectSci21NewStringSignature, nullptr)) {
			// new kString call detected, it's SCI2.1 late
			// TODO: this call seems to be different on Mac
			g_sciVersion = SCI_VERSION_2_1_LATE;
			return;
		}
		return;
	}

	// Check for transitive SCI1/SCI1.1 games, like PQ1 here
	// If the game has any heap file (here we check for heap file 0), then
	// it definitely uses a SCI1.1 kernel
	if (testResource(ResourceId(kResourceTypeHeap, 0))) {
		g_sciVersion = SCI_VERSION_1_1;
		return;
	}

	switch (_mapVersion) {
	case kResVersionSci0Sci1Early:
		if (_viewType == kViewVga) {
			// VGA
			g_sciVersion = SCI_VERSION_1_EARLY;
			return;
		}

		// EGA
		if (hasOldScriptHeader()) {
			g_sciVersion = SCI_VERSION_0_EARLY;
			return;
		}

		if (hasSci0Voc999()) {
			g_sciVersion = SCI_VERSION_0_LATE;
			return;
		}

		if (oldDecompressors) {
			// It's either SCI_VERSION_0_LATE or SCI_VERSION_01

			// We first check for SCI1 vocab.999
			if (testResource(ResourceId(kResourceTypeVocab, 999))) {
				g_sciVersion = SCI_VERSION_01;
				return;
			}

			// If vocab.999 is missing, we try vocab.900
			if (testResource(ResourceId(kResourceTypeVocab, 900))) {
				if (hasSci1Voc900()) {
					g_sciVersion = SCI_VERSION_01;
					return;
				} else {
					g_sciVersion = SCI_VERSION_0_LATE;
					return;
				}
			}

			error("Failed to accurately determine SCI version");
		}

		// New decompressors. It's either SCI_VERSION_1_EGA_ONLY or SCI_VERSION_1_EARLY.
		if (hasSci1Voc900()) {
			g_sciVersion = SCI_VERSION_1_EGA_ONLY;
			return;
		}

		// SCI_VERSION_1_EARLY EGA versions lack the parser vocab
		g_sciVersion = SCI_VERSION_1_EARLY;
		return;
	case kResVersionSci1Middle:
	case kResVersionKQ5FMT:
		g_sciVersion = SCI_VERSION_1_MIDDLE;
		// Amiga SCI1 middle games are actually SCI1 late
		if (_viewType == kViewAmiga || _viewType == kViewAmiga64)
			g_sciVersion = SCI_VERSION_1_LATE;
		// Same goes for Mac SCI1 middle games
		if (g_sci && g_sci->getPlatform() == Common::kPlatformMacintosh)
			g_sciVersion = SCI_VERSION_1_LATE;
		return;
	case kResVersionSci1Late:
		if (_volVersion == kResVersionSci11) {
			g_sciVersion = SCI_VERSION_1_1;
			return;
		}
		g_sciVersion = SCI_VERSION_1_LATE;
		return;
	case kResVersionSci11:
		g_sciVersion = SCI_VERSION_1_1;
		return;
	default:
		g_sciVersion = SCI_VERSION_NONE;
		error("detectSciVersion(): Unable to detect the game's SCI version");
	}
}

bool ResourceManager::detectFontExtended() {

	Resource *res = findResource(ResourceId(kResourceTypeFont, 0), 0);
	if (res) {
		if (res->size() >= 4) {
			uint16 numChars = READ_LE_UINT16(res->data() + 2);
			if (numChars > 0x80)
				return true;
		}
	}
	return false;
}

// detects, if SCI1.1 game uses palette merging or copying - this is supposed to only get used on SCI1.1 games
bool ResourceManager::detectPaletteMergingSci11() {
	// Load palette 999 (default palette)
	Resource *res = findResource(ResourceId(kResourceTypePalette, 999), false);

	if (res && res->size() > 30) {
		// Old palette format used in palette resource? -> it's merging
		if ((res->getUint8At(0) == 0 && res->getUint8At(1) == 1) ||
			(res->getUint8At(0) == 0 && res->getUint8At(1) == 0 && res->getUint16LEAt(29) == 0)) {
			return true;
		}

		// Hardcoded: Laura Bow 2 floppy uses new palette resource, but still palette merging + 16 bit color matching
		if (g_sci && g_sci->getGameId() == GID_LAURABOW2 && !g_sci->isCD() && !g_sci->isDemo()) {
			return true;
		}
	}

	return false;
}

// is called on SCI0EARLY games to make sure that sound resources are in fact also SCI0EARLY
bool ResourceManager::detectEarlySound() {
	Resource *res = findResource(ResourceId(kResourceTypeSound, 1), false);
	if (res &&
		res->size() >= 0x22 &&
		res->getUint16LEAt(0x1f) == 0 && // channel 15 voice count + play mask is 0 in SCI0LATE
		res->getUint8At(0x21) == 0) { // last byte right before actual data is 0 as well

		return false;
	}

	return true;
}

// Functions below are based on PD code by Brian Provinciano (SCI Studio)
bool ResourceManager::hasOldScriptHeader() {
	Resource *res = findResource(ResourceId(kResourceTypeScript, 0), false);

	if (!res) {
		// Script 0 missing -> corrupted / non-SCI resource files.
		// Don't error out here, because this might have been called
		// from the fallback detector
		return false;
	}

	uint offset = 2;
	const int objTypes = 17;

	while (offset < res->size()) {
		uint16 objType = res->getUint16LEAt(offset);

		if (!objType) {
			offset += 2;
			// We should be at the end of the resource now
			return offset == res->size();
		}

		if (objType >= objTypes) {
			// Invalid objType
			return false;
		}

		int skip = res->getUint16LEAt(offset + 2);

		if (skip < 2) {
			// Invalid size
			return false;
		}

		offset += skip;
	}

	return false;
}

bool ResourceManager::hasSci0Voc999() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 999), false);

	if (!res) {
		// No vocab present, possibly a demo version
		return false;
	}

	if (res->size() < 2)
		return false;

	uint16 count = res->getUint16LEAt(0);

	// Make sure there's enough room for the pointers
	if (res->size() < (uint)count * 2)
		return false;

	// Iterate over all pointers
	for (uint i = 0; i < count; i++) {
		// Offset to string
		uint16 offset = res->getUint16LEAt(2 + count * 2);

		// Look for end of string
		do {
			if (offset >= res->size()) {
				// Out of bounds
				return false;
			}
		} while (res->getUint8At(offset++));
	}

	return true;
}

bool ResourceManager::hasSci1Voc900() {
	Resource *res = findResource(ResourceId(kResourceTypeVocab, 900), 0);

	if (!res )
		return false;

	if (res->size() < 0x1fe)
		return false;

	uint16 offset = 0x1fe;

	while (offset < res->size()) {
		offset++;
		do {
			if (offset >= res->size()) {
				// Out of bounds;
				return false;
			}
		} while (res->getUint8At(offset++));
		offset += 3;
	}

	return offset == res->size();
}

// Same function as Script::findBlockSCI0(). Slight code
// duplication here, but this has been done to keep the resource
// manager independent from the rest of the engine
static SciSpan<const byte>::const_iterator findSci0ExportsBlock(const SciSpan<const byte> &buffer) {
	SciSpan<const byte>::const_iterator buf = buffer.cbegin();
	bool oldScriptHeader = (getSciVersion() == SCI_VERSION_0_EARLY);

	if (oldScriptHeader)
		buf += 2;

	for (;;) {
		int seekerType = buf.getUint16LE();

		if (seekerType == 0)
			break;
		if (seekerType == 7)	// exports
			return buf;

		int seekerSize = (buf + 2).getUint16LE();
		assert(seekerSize > 0);
		buf += seekerSize;
	}

	return buffer.cend();
}

// This code duplicates Script::relocateOffsetSci3, but we can't use
// that here since we can't instantiate scripts at this point.
static int relocateOffsetSci3(const SciSpan<const byte> &buf, uint32 offset) {
	int relocStart = buf.getUint32LEAt(8);
	int relocCount = buf.getUint16LEAt(18);
	SciSpan<const byte>::const_iterator seeker = buf.cbegin() + relocStart;

	for (int i = 0; i < relocCount; ++i) {
		const uint32 candidateOffset = seeker.getUint32LE();
		if (candidateOffset == offset) {
			return buf.getUint16LEAt(offset) + (seeker + 4).getUint32LE();
		}
		seeker += 10;
	}

	return -1;
}

reg_t ResourceManager::findGameObject(const bool addSci11ScriptOffset) {
	Resource *script = findResource(ResourceId(kResourceTypeScript, 0), false);

	if (!script)
		return NULL_REG;

	SciSpan<const byte>::const_iterator offsetPtr;

	if (getSciVersion() <= SCI_VERSION_1_LATE) {
		offsetPtr = findSci0ExportsBlock(*script);
		if (offsetPtr == script->cend())
			error("Unable to find exports block from script 0");
		offsetPtr += 4 + 2;

		uint16 offset = !isSci11Mac() ? offsetPtr.getUint16LE() : offsetPtr.getUint16BE();
		return make_reg(1, offset);
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		offsetPtr = script->cbegin() + 4 + 2 + 2;

		// In SCI1.1 - SCI2.1, the heap is appended at the end of the script,
		// so adjust the offset accordingly if requested
		int16 offset = !isSci11Mac() ? offsetPtr.getUint16LE() : offsetPtr.getUint16BE();
		if (addSci11ScriptOffset) {
			offset += script->size();

			// Ensure that the start of the heap is word-aligned - same as in Script::init()
			if (script->size() & 2)
				offset++;
		}

		return make_reg(1, offset);
	} else {
		return make_reg(1, relocateOffsetSci3(*script, 22));
	}
}

Common::String ResourceManager::findSierraGameId() {
	// In SCI0-SCI1, the heap is embedded in the script. In SCI1.1 - SCI2.1,
	// it's in a separate heap resource
	Resource *heap = nullptr;
	int nameSelector = -1;

	if (getSciVersion() < SCI_VERSION_1_1) {
		heap = findResource(ResourceId(kResourceTypeScript, 0), false);
		nameSelector = 3;
	} else if (getSciVersion() >= SCI_VERSION_1_1 && getSciVersion() <= SCI_VERSION_2_1_LATE) {
		heap = findResource(ResourceId(kResourceTypeHeap, 0), false);
		nameSelector = 8;
	} else if (getSciVersion() == SCI_VERSION_3) {
		heap = findResource(ResourceId(kResourceTypeScript, 0), false);

		Resource *vocab = findResource(ResourceId(kResourceTypeVocab, VOCAB_RESOURCE_SELECTORS), false);
		if (!vocab)
			return "";

		const uint16 numSelectors = vocab->getUint16LEAt(0);
		for (uint16 i = 0; i < numSelectors; ++i) {
			uint16 selectorOffset = vocab->getUint16LEAt((i + 1) * sizeof(uint16));
			uint16 selectorSize = vocab->getUint16LEAt(selectorOffset);

			Common::String selectorName = Common::String((const char *)vocab->getUnsafeDataAt(selectorOffset + 2, selectorSize), selectorSize);
			if (selectorName == "name") {
				nameSelector = i;
				break;
			}
		}
	}

	if (!heap || nameSelector == -1)
		return "";

	int16 gameObjectOffset = findGameObject(false).getOffset();

	if (!gameObjectOffset)
		return "";

	int32 offset;
	if (getSciVersion() == SCI_VERSION_3) {
		offset = relocateOffsetSci3(*heap, gameObjectOffset + /* base selector offset */ 0x110 + nameSelector * sizeof(uint16));
	} else {
		// Seek to the name selector of the first export
		SciSpan<const byte>::const_iterator offsetPtr = heap->cbegin() + gameObjectOffset + nameSelector * sizeof(uint16);
		offset = !isSci11Mac() ? offsetPtr.getUint16LE() : offsetPtr.getUint16BE();
	}

	return heap->getStringAt(offset);
}

// Mac executables are currently used for icon bars and native fonts.
// Eventually they should be used for native menus and possibly even splash screens.
// For example, LSL6 can't function without native menus. (bug #11356)
// Executables that we currently don't use are commented out.
Common::Path ResourceManager::getMacExecutableName() const {
	switch (g_sci->getGameId()) {
	case GID_CASTLEBRAIN: return "Castle of Dr. Brain"; // fonts, splash screen
	case GID_FREDDYPHARKAS: return "Freddy Pharkas"; // fonts, icon bar, menu, splash screen
	//case GID_HOYLE4: return "Hoyle"; // menu, splash screen
	//case GID_KQ5: return "King's Quest V"; // fonts (not supported yet), splash screen
	case GID_KQ6: return "King's Quest VI"; // fonts, icon bar, menu, splash screen
	case GID_LSL1: return "Leisure Suit Larry 1"; // fonts, splash screen
	case GID_LSL5: return "Leisure Suit Larry 5"; // fonts, splash screen
	//case GID_LSL6: return "Leisure Suit Larry 6"; // menu, splash screen
	//case GID_QFG1VGA: return "Quest for Glory"; // menu, splash screen
	case GID_SQ1: return "Space Quest 1"; // fonts, splash screen
	//case GID_SQ3: return "SQ3"; // menu, splash screen
	//case GID_SQ4: return "Space Quest IV"; // splash screen
	default: return "";
	}
}

bool ResourceManager::isKoreanMessageMap(ResourceSource *source) {
	return source->getLocationName() == "message.map" && g_sci && g_sci->getLanguage() == Common::KO_KOR;
}

const Common::Path &Resource::getResourceLocation() const {
	return _source->getLocationName();
}

} // End of namespace Sci
