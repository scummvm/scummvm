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

#include "common/archive.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/memstream.h"
#include "sci/resource/resource.h"
#include "sci/resource/resource_intern.h"
#include "sci/util.h"

namespace Sci {

AudioVolumeResourceSource::AudioVolumeResourceSource(ResourceManager *resMan, const Common::Path &name, ResourceSource *map, int volNum)
	: VolumeResourceSource(name, map, volNum, kSourceAudioVolume) {

	_audioCompressionType = 0;

	/*
	 * Check if this audio volume got compressed by our tool. If that is the
	 * case, set _audioCompressionType and read in the offset translation
	 * table for later usage.
	 */

	Common::SeekableReadStream *fileStream = getVolumeFile(resMan, nullptr);
	if (!fileStream)
		return;

	fileStream->seek(0, SEEK_SET);
	const uint32 compressionType = fileStream->readUint32BE();
	switch (compressionType) {
	case MKTAG('M','P','3',' '):
	case MKTAG('O','G','G',' '):
	case MKTAG('F','L','A','C'): {
		_audioCompressionType = compressionType;
		const uint32 numEntries = fileStream->readUint32LE();
		if (!numEntries) {
			error("Compressed audio volume %s has no relocation table entries", name.toString().c_str());
		}

		CompressedTableEntry *lastEntry = nullptr;
		for (uint i = 0; i < numEntries; ++i) {
			CompressedTableEntry nextEntry;
			const uint32 sourceOffset = fileStream->readUint32LE();
			nextEntry.offset = fileStream->readUint32LE();
			if (lastEntry != nullptr) {
				lastEntry->size = nextEntry.offset - lastEntry->offset;
			}

			_compressedOffsets.setVal(sourceOffset, nextEntry);
			lastEntry = &_compressedOffsets.getVal(sourceOffset);
		}

		lastEntry->size = fileStream->size() - lastEntry->offset;
		}
		break;
	default:
		break;
	}

	resMan->disposeVolumeFileStream(fileStream, this);
}

bool Resource::loadFromWaveFile(Common::SeekableReadStream *file) {
	byte *ptr = new byte[_size];
	_data = ptr;

	uint32 bytesRead = file->read(ptr, _size);
	if (bytesRead != _size)
		error("Read %d bytes from %s but expected %u", bytesRead, _id.toString().c_str(), _size);

	_status = kResStatusAllocated;
	return true;
}

bool Resource::loadFromAudioVolumeSCI11(Common::SeekableReadStream *file) {
	// Check for WAVE files here
	uint32 riffTag = file->readUint32BE();
	if (riffTag == MKTAG('R','I','F','F')) {
		_size = file->readUint32LE() + 8;
		file->seek(-8, SEEK_CUR);
		return loadFromWaveFile(file);
	}
	file->seek(-4, SEEK_CUR);

	// Rave-resources (King's Quest 6) don't have any header at all
	if (getType() != kResourceTypeRave) {
		ResourceType type = _resMan->convertResType(file->readByte());

		if (((getType() == kResourceTypeAudio || getType() == kResourceTypeAudio36) && (type != kResourceTypeAudio))
			|| ((getType() == kResourceTypeSync || getType() == kResourceTypeSync36) && (type != kResourceTypeSync))) {
			warning("Resource type mismatch loading %s", _id.toString().c_str());
			unalloc();
			return false;
		}

		const uint8 headerSize = file->readByte();

		if (type == kResourceTypeAudio) {
			if (headerSize != 7 && headerSize != 11 && headerSize != 12) {
				warning("Unsupported audio header size %d in %s", headerSize, _id.toString().c_str());
				unalloc();
				return false;
			}

			if (headerSize != 7) { // Size is defined already from the map
				// Load sample size
				file->seek(7, SEEK_CUR);
				_size = file->readUint32LE() + headerSize + kResourceHeaderSize;
				if (file->err() || file->eos()) {
					warning("Error while reading size of %s", _id.toString().c_str());
					unalloc();
					return false;
				}
				// Adjust offset to point at the beginning of the audio file
				// again
				file->seek(-11, SEEK_CUR);
			}

			// SOL audio files are designed to require the resource header
			file->seek(-2, SEEK_CUR);
		}
	}
	return loadPatch(file);
}

bool Resource::loadFromAudioVolumeSCI1(Common::SeekableReadStream *file) {
	byte *ptr = new byte[size()];
	_data = ptr;

	if (!ptr) {
		error("Can't allocate %u bytes needed for loading %s", _size, _id.toString().c_str());
	}

	uint32 bytesRead = file->read(ptr, size());
	if (bytesRead != size())
		warning("Read %d bytes from %s but expected %u", bytesRead, _id.toString().c_str(), _size);

	_status = kResStatusAllocated;
	return true;
}

void ResourceManager::addNewGMPatch(SciGameId gameId) {
	const char *gmPatchFile = nullptr;

	switch (gameId) {
	case GID_ECOQUEST:
		gmPatchFile = "ECO1GM.PAT";
		break;
	case GID_HOYLE3:
		gmPatchFile = "HOY3GM.PAT";
		break;
	case GID_LSL1:
		gmPatchFile = "LL1_GM.PAT";
		break;
	case GID_LSL5:
		gmPatchFile = "LL5_GM.PAT";
		break;
	case GID_LONGBOW:
		gmPatchFile = "ROBNGM.PAT";
		break;
	case GID_SQ1:
		gmPatchFile = "SQ1_GM.PAT";
		break;
	case GID_SQ4:
		gmPatchFile = "SQ4_GM.PAT";
		break;
	case GID_FAIRYTALES:
		gmPatchFile = "TALEGM.PAT";
		break;
	default:
		return;
	}

	if (Common::File::exists(gmPatchFile)) {
		ResourceSource *psrcPatch = new PatchResourceSource(gmPatchFile);
		processPatch(psrcPatch, kResourceTypePatch, 4);
	}
}

void ResourceManager::addNewD110Patch(SciGameId gameId) {
	const char *patchFile = nullptr;

	switch (gameId) {
	case GID_CAMELOT:
		patchFile = "CAMELOT.000";
		break;
	case GID_HOYLE1:
		patchFile = "HOYLE.000";
		break;
	case GID_QFG1:
		patchFile = "HQ1.000";
		break;
	case GID_ICEMAN:
		patchFile = "ICEMAN.000"; // Also ICE.000, but let's go with this one
		break;
	case GID_KQ4:
		patchFile = "KQ4.000";
		break;
	case GID_LSL2:
		patchFile = "LSL2.000";
		break;
	case GID_LSL3:
		patchFile = "LSL3.000";
		break;
	case GID_PQ2:
		patchFile = "PQ2.000";
		break;
	case GID_SQ3:
		patchFile = "SQ3.000";
		break;
	default:
		// There's also a CB.000, but unfortunately that file contains an MT-32 patch
		return;
	}

	if (Common::File::exists(patchFile)) {
		ResourceSource *psrcPatch = new PatchResourceSource(patchFile);
		processPatch(psrcPatch, kResourceTypePatch, 0);
	}
}

void ResourceManager::processWavePatch(ResourceId resourceId, const Common::Path &name) {
	ResourceSource *resSrc = new WaveResourceSource(name);
	Common::File file;
	file.open(name);

	updateResource(resourceId, resSrc, 0, file.size(), name);
	_sources.push_back(resSrc);

	debugC(1, kDebugLevelResMan, "Patching %s - OK", name.toString().c_str());
}

void ResourceManager::readWaveAudioPatches() {
	// Here we do check for SCI1.1+ so we can patch wav files in as audio resources
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.wav");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		Common::String name = (*x)->getFileName();

		if (Common::isDigit(name[0]))
			processWavePatch(ResourceId(kResourceTypeAudio, atoi(name.c_str())), (*x)->getPathInArchive());
	}
}

#ifdef ENABLE_SCI32
void ResourceManager::readAIFFAudioPatches() {
	// LSL6 hires Mac is the only game that has AIFF audio patch files,
	//  which it plays with special overloads of kDoAudio. Restrict this
	//  scan to just this game since the filenames are so generic.
	if (!(g_sci->getGameId() == GID_LSL6HIRES && _isSci2Mac)) {
		return;
	}

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "####");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		Common::String name = (*x)->getFileName();

		processWavePatch(ResourceId(kResourceTypeAudio, atoi(name.c_str())), (*x)->getPathInArchive());
	}
}
#endif

void ResourceManager::removeAudioResource(ResourceId resId) {
	// Remove resource, unless it was loaded from a patch
	if (_resMap.contains(resId)) {
		Resource *res = _resMap.getVal(resId);

		if (res->_source->getSourceType() == kSourceAudioVolume) {
			if (res->_status == kResStatusLocked) {
				warning("Failed to remove resource %s (still in use)", resId.toString().c_str());
			} else {
				if (res->_status == kResStatusEnqueued)
					removeFromLRU(res);

				_resMap.erase(resId);
				delete res;
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

// QFG3 Demo 0.MAP structure:
// =========
// 10-byte entries:
// w nEntry
// dw offset
// dw size

// LB2 Floppy/Mother Goose SCI1.1 0.MAP structure:
// =========
// 8-byte entries:
// w nEntry
// w 0xffff
// dw offset

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

int ResourceManager::readAudioMapSCI11(IntMapResourceSource *map) {
#ifndef ENABLE_SCI32
	// SCI32 support is not built in. Check if this is a SCI32 game
	// and if it is abort here.
	if (_volVersion >= kResVersionSci2)
		return SCI_ERROR_RESMAP_NOT_FOUND;
#endif

	uint32 offset = 0;
	const ResourceId mapResId(kResourceTypeMap, map->_mapNumber);
	Resource *mapRes = nullptr;

	if (!_resMap.tryGetVal(mapResId,mapRes)) {
		warning("Failed to open %s", mapResId.toString().c_str());
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	// Here, we allocate audio maps ourselves instead of using findResource to
	// do this for us. This is in order to prevent the map resources from
	// getting into the LRU cache. These resources must be read and then
	// deallocated in games with multi-disc audio in order to read the audio
	// maps from every CD, and LRU eviction freaks out if an unallocated
	// resource ends up in the LRU list. It is also not necessary for these
	// resources to be cached in the LRU at all, since they are only used upon
	// game startup to populate _resMap.
	assert(mapRes->_status == kResStatusNoMalloc);
	loadResource(mapRes);

	if (!mapRes->data()) {
		warning("Failed to read data for %s", mapResId.toString().c_str());
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	ResourceSource *src = findVolume(map, map->_volumeNumber);

	if (!src) {
		warning("Failed to find volume for %s", mapResId.toString().c_str());
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
	}

	Common::SeekableReadStream *fileStream = getVolumeFile(src);

	if (!fileStream) {
		warning("Failed to open file stream for %s", src->getLocationName().toString().c_str());
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
	}

	disposeVolumeFileStream(fileStream, src);

	SciSpan<const byte>::const_iterator ptr = mapRes->cbegin();

	uint32 entrySize = 0;
	if (_volVersion >= kResVersionSci2) {
		// The heuristic size detection is incompatible with at least Torin RU,
		// which is fine because it is not needed for SCI32
		entrySize = 11;
	} else {
		// Heuristic to detect entry size
		for (int i = mapRes->size() - 1; i >= 0; --i) {
			if (ptr[i] == 0xff)
				entrySize++;
			else
				break;
		}
	}

	if (map->_mapNumber == 65535) {
		while (ptr != mapRes->cend()) {
			uint16 n = ptr.getUint16LE();
			ptr += 2;

			if (n == 0xffff)
				break;

			if (entrySize == 6) {
				offset = ptr.getUint32LE();
				ptr += 4;
			} else {
				offset += ptr.getUint24LE();
				ptr += 3;
			}

			addResource(ResourceId(kResourceTypeAudio, n), src, offset, 0, map->getLocationName());
		}
	} else if (map->_mapNumber == 0 && entrySize == 10 && ptr[3] == 0) {
		// QFG3 demo format
		// ptr[3] would be 'seq' in the normal format and cannot possibly be 0
		while (ptr != mapRes->cend()) {
			uint16 n = ptr.getUint16BE();
			ptr += 2;

			if (n == 0xffff)
				break;

			offset = ptr.getUint32LE();
			ptr += 4;
			uint32 size = ptr.getUint32LE();
			ptr += 4;

			addResource(ResourceId(kResourceTypeAudio, n), src, offset, size, map->getLocationName());
		}
	} else if (map->_mapNumber == 0 && entrySize == 8 && (ptr + 2).getUint16LE() == 0xffff) {
		// LB2 Floppy/Mother Goose SCI1.1 format
		Common::SeekableReadStream *stream = getVolumeFile(src);

		while (ptr != mapRes->cend()) {
			uint16 n = ptr.getUint16LE();
			ptr += 4;

			if (n == 0xffff)
				break;

			const ResourceId audioResId(kResourceTypeAudio, n);

			offset = ptr.getUint32LE();
			ptr += 4;

			uint32 size;
			if (src->getAudioCompressionType() == 0) {
				// The size is not stored in the map and the entries have no order.
				// We need to dig into the audio resource in the volume to get the size.
				stream->seek(offset + 1);
				byte headerSize = stream->readByte();
				if (headerSize != 11 && headerSize != 12) {
					error("Unexpected header size in %s: should be 11 or 12, got %d", audioResId.toString().c_str(), headerSize);
				}

				stream->skip(7);
				size = stream->readUint32LE() + headerSize + 2;
			} else {
				size = 0;
			}
			addResource(audioResId, src, offset, size, map->getLocationName());
		}

		disposeVolumeFileStream(stream, src);
	} else {
		// EQ1CD & SQ4CD are "early" games; KQ6CD and all SCI32 are "late" games
		const bool isEarly = (entrySize != 11);

		if (!isEarly) {
			offset = ptr.getUint32LE();
			ptr += 4;
		}

		enum {
			kRaveFlag = 0x40,
			kSyncFlag = 0x80,
			kEndOfMapFlag = 0xFF
		};

		while (ptr != mapRes->cend()) {
			uint32 n = ptr.getUint32BE();
			uint32 syncSize = 0;
			ptr += 4;

			// Checking the entire tuple breaks Torin RU and is not how SSCI
			// works
			if ((n & kEndOfMapFlag) == kEndOfMapFlag) {
				const uint32 bytesLeft = mapRes->cend() - ptr;
				if (bytesLeft >= entrySize && entrySize > 0) {
					warning("End of %s reached, but %u entries remain", mapResId.toString().c_str(), bytesLeft / entrySize);
				}
				break;
			}

			if (isEarly) {
				offset = ptr.getUint32LE();
				ptr += 4;
			} else {
				offset += ptr.getUint24LE();
				ptr += 3;
			}

			if (isEarly || (n & kSyncFlag)) {
				syncSize = ptr.getUint16LE();
				ptr += 2;

				// FIXME: The sync36 resource seems to be two bytes too big in KQ6CD
				// (bytes taken from the RAVE resource right after it)
				if (syncSize > 0) {
					addResource(ResourceId(kResourceTypeSync36, map->_mapNumber, n & 0xffffff3f), src, offset, syncSize, map->getLocationName());
				}
			}

			// Checking for this 0x40 flag breaks at least Laura Bow 2 CD 1.1
			// map 448
			if (g_sci->getGameId() == GID_KQ6 && (n & kRaveFlag)) {
				// This seems to define the size of raw lipsync data (at least
				// in KQ6 CD Windows).
				uint32 kq6HiresSyncSize = ptr.getUint16LE();
				ptr += 2;

				if (kq6HiresSyncSize > 0) {
					// Rave resources do not have separate entries in the audio
					// map (their data was just appended to sync resources), so
					// we have to use the sync resource offset first and then
					// adjust the offset & size later, otherwise offset
					// validation will fail for compressed volumes (since the
					// relocation table in a compressed volume only contains
					// offsets that existed in the original audio map)
					Resource *res = addResource(ResourceId(kResourceTypeRave, map->_mapNumber, n & 0xffffff3f), src, offset, syncSize + kq6HiresSyncSize, map->getLocationName());
					res->_fileOffset += syncSize;
					res->_size -= syncSize;
					syncSize += kq6HiresSyncSize;
				}
			}

			const ResourceId id(kResourceTypeAudio36, map->_mapNumber, n & 0xffffff3f);

			// Map 405 on CD 1 of the US release of PQ:SWAT 1.000 is broken
			// and points to garbage in the RESOURCE.AUD. The affected audio36
			// assets seem to be able to load successfully from one of the later
			// CDs, so just ignore the map on this disc
			if (g_sci->getGameId() == GID_PQSWAT &&
				g_sci->getLanguage() == Common::EN_ANY &&
				map->_volumeNumber == 1 &&
				map->_mapNumber == 405) {
				continue;
			}

			// GK2 has invalid audio36 map entries on CD 1 of the German
			//  version and CDs 5-6 of all versions. All are safe to ignore
			//  because their content doesn't apply to the disc's chapter.
			if (g_sci->getGameId() == GID_GK2) {
				// Map 2020 on CD 1 only exists in localized versions and
				//  contains inventory messages from later chapters.
				if (map->_volumeNumber == 1 &&
					map->_mapNumber == 2020) {
					continue;
				}

				// Maps 210 and 220 on CD 5 point to garbage data.
				//  These maps are for Huber Farm messages but this area
				//  was removed from chapter 5. If these map entries are
				//  used then it breaks the audio during other chapters.
				if (map->_volumeNumber == 5 &&
					(map->_mapNumber == 210 || map->_mapNumber == 220)) {
					continue;
				}

				// Maps 22 and 160 on CD 6 appear in various broken forms
				//  in English and apparently every localized version.
				//  These messages are for Grace's notebook and castle
				//  secret passage rooms which aren't in chapter 6.
				if (map->_volumeNumber == 6 &&
					(map->_mapNumber == 22 || map->_mapNumber == 160)) {
					continue;
				}
			}

			// Lighthouse German has invalid audio36 map entries for
			//  content that was cut from the game. These resources
			//  existed in the English version even though they were
			//  inaccessible.
			if (g_sci->getGameId() == GID_LIGHTHOUSE &&
				map->_mapNumber == 800) {
				continue;
			}

			// LSL7 French has an invalid audio36 map entry for a narrator
			//  message that was cut from the game. This resource existed
			//  in the English version even though it was inaccessible.
			if (g_sci->getGameId() == GID_LSL7 &&
				map->_mapNumber == 999) {
				continue;
			}

			// Map 800 and 4176 contain content that was cut from the game. The
			// French version of the game includes map files from the US
			// release, but the audio resources are French so the maps don't
			// match. Since the content was never used, just ignore these maps
			// everywhere
			if (g_sci->getGameId() == GID_PHANTASMAGORIA2 &&
				(map->_mapNumber == 800 || map->_mapNumber == 4176)) {
				continue;
			}

			addResource(id, src, offset + syncSize, 0, map->getLocationName());
		}
	}

	mapRes->unalloc();

	return 0;
}

// AUDIOnnn.MAP contains 10-byte entries:
// Early format:
// w 5 bits resource type and 11 bits resource number
// dw 7 bits volume number and 25 bits offset
// dw size
// Later format:
// w nEntry
// dw offset+volume (as in resource.map)
// dw size
// ending with 10 0xFFs
int ResourceManager::readAudioMapSCI1(ResourceSource *map, bool unload) {
	Common::File file;

	if (!file.open(map->getLocationName()))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	bool oldFormat = (file.readUint16LE() >> 11) == kResourceTypeAudio;
	file.seek(0);

	for (;;) {
		uint16 n = file.readUint16LE();
		uint32 offset = file.readUint32LE();
		uint32 size = file.readUint32LE();

		if (file.eos() || file.err()) {
			warning("Error while reading %s", map->getLocationName().toString().c_str());
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}

		if (n == 0xffff)
			break;

		byte volume_nr;

		if (oldFormat) {
			n &= 0x07ff; // Mask out resource type
			volume_nr = offset >> 25; // most significant 7 bits
			offset &= 0x01ffffff; // least significant 25 bits
		} else {
			volume_nr = offset >> 28; // most significant 4 bits
			offset &= 0x0fffffff; // least significant 28 bits
		}

		ResourceSource *src = findVolume(map, volume_nr);

		if (src) {
			const ResourceId resId(kResourceTypeAudio, n);

			if (unload)
				removeAudioResource(resId);
			else
				addResource(resId, src, offset, size, map->getLocationName());
		} else {
			warning("Failed to find audio volume %i", volume_nr);
			return SCI_ERROR_NO_RESOURCE_FILES_FOUND;
		}
	}

	return 0;
}

bool ResourceManager::setAudioLanguage(int language) {
	if (_audioMapSCI1) {
		if (_audioMapSCI1->_volumeNumber == language) {
			// This language is already loaded
			return true;
		}

		// We already have a map loaded, so we unload it and its sources
		unloadAudioLanguage();
	}

	Common::Path filename(Common::String::format("AUDIO%03d", language));
	Common::Path fullname = filename.append(".MAP");
	if (!Common::File::exists(fullname)) {
		warning("No audio map found for language %i", language);
		return false;
	}

	_audioMapSCI1 = addSource(new ExtAudioMapResourceSource(fullname, language));

	// Search for audio volumes for this language and add them to the source list
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, filename.append(".0##"));
	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getFileName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addSource(new AudioVolumeResourceSource(this, (*x)->getPathInArchive(), _audioMapSCI1, number));
	}

	scanNewSources();
	return true;
}

void ResourceManager::unloadAudioLanguage() {
	if (_audioMapSCI1 == nullptr) {
		return;
	}

	// Unload the map
	if (readAudioMapSCI1(_audioMapSCI1, true) != SCI_ERROR_NONE) {
		_hasBadResources = true;
	}

	// Remove all volumes that use this map from the source list
	Common::List<ResourceSource *>::iterator it = _sources.begin();
	while (it != _sources.end()) {
		ResourceSource *src = *it;
		if (src->findVolume(_audioMapSCI1, src->_volumeNumber)) {
			it = _sources.erase(it);
			delete src;
		} else {
			++it;
		}
	}

	// Remove the map itself from the source list
	_sources.remove(_audioMapSCI1);
	delete _audioMapSCI1;

	_audioMapSCI1 = nullptr;
}

int ResourceManager::getAudioLanguage() const {
	return (_audioMapSCI1 ? _audioMapSCI1->_volumeNumber : 0);
}

bool ResourceManager::isGMTrackIncluded() {
	// This check only makes sense for SCI1 and newer games
	if (getSciVersion() < SCI_VERSION_1_EARLY)
		return false;

	// SCI2 and newer games always have GM tracks
	if (getSciVersion() >= SCI_VERSION_2)
		return true;

	// For the leftover games, we can safely use SCI_VERSION_1_EARLY for the soundVersion
	const SciVersion soundVersion = SCI_VERSION_1_EARLY;

	// Read the first song and check if it has a GM track
	bool result = false;
	Common::List<ResourceId> resources = listResources(kResourceTypeSound, -1);
	Common::sort(resources.begin(), resources.end());
	Common::List<ResourceId>::iterator itr = resources.begin();
	int firstSongId = itr->getNumber();

	SoundResource song1(firstSongId, this, soundVersion);
	if (!song1.exists()) {
		warning("ResourceManager::isGMTrackIncluded: track 1 not found");
		return false;
	}

	SoundResource::Track *gmTrack = song1.getTrackByType(0x07);
	if (gmTrack)
		result = true;

	return result;
}

SoundResource::SoundResource(uint32 resourceNr, ResourceManager *resMan, SciVersion soundVersion) :
	_resMan(resMan), _soundVersion(soundVersion), _trackCount(0), _tracks(nullptr), _soundPriority(0xFF) {
	_resource = _resMan->findResource(ResourceId(kResourceTypeSound, resourceNr), true);
	if (!_resource)
		return;

	Channel *channel;

	if (_soundVersion <= SCI_VERSION_0_LATE) {
		// SCI0 only has a header of 0x11/0x21 byte length and the actual midi track follows afterwards
		_trackCount = 1;
		_tracks = new Track[_trackCount];
		_tracks->digitalChannelNr = -1;
		_tracks->type = 0; // Not used for SCI0
		_tracks->channelCount = 1;
		// Digital sample data included? -> Add an additional channel
		if (_resource->getUint8At(0) == 2)
			_tracks->channelCount++;
		// header information that can be passed to the SCI0 sound driver
		_tracks->header = _resource->subspan(0, _soundVersion == SCI_VERSION_0_EARLY ? 0x11 : 0x21);
		_tracks->channels = new Channel[_tracks->channelCount];
		channel = &_tracks->channels[0];
		channel->flags |= 2; // don't remap (SCI0 doesn't have remapping)
		if (_soundVersion == SCI_VERSION_0_EARLY) {
			channel->data = _resource->subspan(0x11);
		} else {
			channel->data = _resource->subspan(0x21);
		}
		if (_tracks->channelCount == 2) {
			// we need to find 0xFC (channel terminator) within the data
			SciSpan<const byte>::const_iterator it = channel->data.cbegin();
			while (it != channel->data.cend() && *it != 0xfc)
				it++;
			// Skip any following 0xFCs as well
			while (it != channel->data.cend() && *it == 0xfc)
				it++;
			// Verify that there is data after the channel terminator
			if (it != channel->data.cend()) {
				// Digital sample data included
				_tracks->digitalChannelNr = 1;
				// Now adjust channels accordingly
				Channel *sampleChannel = &_tracks->channels[1];
				sampleChannel->data = channel->data.subspan(it - channel->data.cbegin());
				channel->data = channel->data.subspan(0, it - channel->data.cbegin());
				// Read sample header information
				//Offset 14 in the header contains the frequency as a short integer. Offset 32 contains the sample length, also as a short integer.
				_tracks->digitalSampleRate = sampleChannel->data.getUint16LEAt(14);
				_tracks->digitalSampleSize = sampleChannel->data.getUint16LEAt(32);
				_tracks->digitalSampleStart = 0;
				_tracks->digitalSampleEnd = 0;
				sampleChannel->data += 44; // Skip over header
			} else {
				// Early versions of SQ3 have the digital sample flag set in
				// sound 35 even though there is no digital sample. Bug #13206
				warning("No digital sample data in sound resource %d", resourceNr);
				_tracks->channelCount--; // ignore the digital sample flag
			}
		}
	} else if (_soundVersion >= SCI_VERSION_1_EARLY && _soundVersion <= SCI_VERSION_2_1_MIDDLE) {
		SciSpan<const byte> data = *_resource;
		// Count # of tracks
		_trackCount = 0;
		while ((*data++) != 0xFF) {
			_trackCount++;
			while (*data != 0xFF)
				data += 6;
			++data;
		}
		_tracks = new Track[_trackCount];
		data = *_resource;

		for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
			// Track info starts with track type:BYTE
			// Then the channel information gets appended Unknown:WORD, ChannelOffset:WORD, ChannelSize:WORD
			// 0xFF:BYTE as terminator to end that track and begin with another track type
			// Track type 0xFF is the marker signifying the end of the tracks

			_tracks[trackNr].type = *data++;
			// Counting # of channels used
			SciSpan<const byte> data2 = data;
			byte channelCount = 0;
			while (*data2 != 0xFF) {
				data2 += 6;
				channelCount++;
			}
			_tracks[trackNr].channels = new Channel[channelCount];
			_tracks[trackNr].channelCount = 0;
			_tracks[trackNr].digitalChannelNr = -1; // No digital sound associated
			_tracks[trackNr].digitalSampleRate = 0;
			_tracks[trackNr].digitalSampleSize = 0;
			_tracks[trackNr].digitalSampleStart = 0;
			_tracks[trackNr].digitalSampleEnd = 0;
			if (_tracks[trackNr].type != 0xF0) { // Digital track marker - not supported currently
				int channelNr = 0;
				while (channelCount--) {
					channel = &_tracks[trackNr].channels[channelNr];
					const uint16 dataOffset = data.getUint16LEAt(2);

					if (dataOffset >= _resource->size()) {
						warning("Invalid offset inside sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
						data += 6;
						continue;
					}

					uint16 size = data.getUint16LEAt(4);

					if ((uint32)dataOffset + size > _resource->size()) {
						warning("Invalid size inside sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
						size = _resource->size() - dataOffset;
					}

					if (size == 0) {
						warning("Empty channel in sound resource %d: track %d, channel %d", resourceNr, trackNr, channelNr);
						data += 6;
						continue;
					}

					channel->data = _resource->subspan(dataOffset, size);

					channel->curPos = 0;
					channel->number = channel->data[0];

					channel->poly = channel->data[1] & 0x0F;
					channel->prio = channel->data[1] >> 4;
					channel->time = channel->prev = 0;
					channel->data += 2; // skip over header
					if (channel->number == 0xFE) { // Digital channel
						_tracks[trackNr].digitalChannelNr = channelNr;
						_tracks[trackNr].digitalSampleRate = channel->data.getUint16LEAt(0);
						_tracks[trackNr].digitalSampleSize = channel->data.getUint16LEAt(2);
						_tracks[trackNr].digitalSampleStart = channel->data.getUint16LEAt(4);
						_tracks[trackNr].digitalSampleEnd = channel->data.getUint16LEAt(6);
						channel->data += 8; // Skip over header
						channel->flags = 0;
					} else {
						channel->flags = channel->number >> 4;
						channel->number = channel->number & 0x0F;
						// Flag 1:	Channel start offset is 0 instead of 10 (currently: everything 0)
						//			Also: Don't map the channel to the device at all, but still play it.
						//			It doesn't stop other sounds playing sounds on that channel, it even
						//			allows other sounds to map to this channel (in that case the dontmap
						//			channel has limited access, it can't send control change, program
						//			change and pitch wheel messages.
						//			This is basically a marker for the channel as a "real" channel
						//			(used mostly for rhythm channels on devices that have one). These
						//			channels will also consequently start the parsing at offset 0 instead
						//			of 10: Normal channels would read the parameters of the first couple of
						//			events into the channel structs, but the "real" channels have to
						//			send these to the device right away, since they don't use the stored
						//			data.
						//			Very early games like KQ5 (but including the DOS CD version) and SQ2
						//			have support for this flag, only. It isn't even a flag there, since
						//			all these games do is check for a channel number below 0x10.
						// 
						// Flag 2:	Don't remap the channel. It is placed in the map, but only in the
						//			exact matching slot of the channel number. All the other games except
						//			the very early ones use this flag to mark the rhythm channels. I
						//			haven't seen any usage of flag 1 in any of these games. They all use
						//			flag 2 instead, but still have full support of flag 1 in the code.
						//			Using this flag is really preferable, since there can't be conflicts
						//			with different sounds playing on the channel.
						// 
						// Flag 4:	Start up muted. The channel won't be mapped (and thus, not have any
						//			output), until the mute gets removed.
					}
					_tracks[trackNr].channelCount++;
					channelNr++;
					data += 6;
				}
			} else {
				// The first byte of the 0xF0 track's channel list is priority
				_soundPriority = *data;

				// Skip over digital track
				data += 6;
			}
			++data; // Skipping 0xFF that closes channels list
		}
	} else {
		error("SoundResource: SCI version %s is unsupported", getSciVersionDesc(_soundVersion));
	}
}

SoundResource::~SoundResource() {
	if (_tracks != nullptr) {
		for (int trackNr = 0; trackNr < _trackCount; trackNr++)
			delete[] _tracks[trackNr].channels;
		delete[] _tracks;
	}

	if (_resource != nullptr) {
		_resMan->unlockResource(_resource);
	}
}

#if 0
SoundResource::Track* SoundResource::getTrackByNumber(uint16 number) {
	if (_soundVersion <= SCI_VERSION_0_LATE)
		return &_tracks[0];

	if (/*number >= 0 &&*/number < _trackCount)
		return &_tracks[number];
	return NULL;
}
#endif

SoundResource::Track *SoundResource::getTrackByType(byte type) {
	if (_soundVersion <= SCI_VERSION_0_LATE)
		return &_tracks[0];

	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		if (_tracks[trackNr].type == type)
			return &_tracks[trackNr];
	}
	return nullptr;
}

SoundResource::Track *SoundResource::getDigitalTrack() {
	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		if (_tracks[trackNr].digitalChannelNr != -1)
			return &_tracks[trackNr];
	}
	return nullptr;
}

// Gets the filter mask for SCI0 sound resources
int SoundResource::getChannelFilterMask(int hardwareMask, bool wantsRhythm) {
	SciSpan<const byte> data = *_resource;
	int channelMask = 0;

	if (_soundVersion > SCI_VERSION_0_LATE)
		return 0;

	++data; // Skip over digital sample flag

	for (int channelNr = 0; channelNr < 16; channelNr++) {
		channelMask = channelMask >> 1;

		byte flags;

		if (_soundVersion == SCI_VERSION_0_EARLY) {
			// Each channel is specified by a single byte
			// Upper 4 bits of the byte is a voices count
			// Lower 4 bits -> bit 0 set: use for AdLib
			//				   bit 1 set: use for PCjr
			//				   bit 2 set: use for PC speaker
			//				   bit 3 set and bit 0 clear: control channel (15)
			//				   bit 3 set and bit 0 set: rhythm channel (9)
			// Note: control channel is dynamically assigned inside the drivers,
			// but seems to be fixed at 15 in the song data.
			flags = *data++;

			// Get device bits
			flags &= 0x7;
		} else {
			// Each channel is specified by 2 bytes
			// 1st byte is voices count
			// 2nd byte is play mask, which specifies if the channel is supposed to be played
			// by the corresponding hardware

			// Skip voice count
			++data;

			flags = *data++;
		}

		bool play;
		switch (channelNr) {
		case 15:
			// Always play control channel
			play = true;
			break;
		case 9:
			// Play rhythm channel when requested
			play = wantsRhythm || (flags & hardwareMask);
			break;
		default:
			// Otherwise check for flag
			play = flags & hardwareMask;
		}

		if (play) {
			// This Channel is supposed to be played by the hardware
			channelMask |= 0x8000;
		}
	}

	return channelMask;
}

#if 0
byte SoundResource::getInitialVoiceCount(byte channel) {
	if (_soundVersion > SCI_VERSION_0_LATE)
		return 0; // TODO

	// Skip over digital sample flag
	SciSpan<const byte> data = _resource->subspan(1);

	if (_soundVersion == SCI_VERSION_0_EARLY)
		return data[channel] >> 4;
	else
		return data[channel * 2];
}
#endif

void WaveResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = getVolumeFile(resMan, res);
	if (!fileStream)
		return;

	fileStream->seek(res->_fileOffset, SEEK_SET);
	res->loadFromWaveFile(fileStream);
	resMan->disposeVolumeFileStream(fileStream, this);
}

void AudioVolumeResourceSource::loadResource(ResourceManager *resMan, Resource *res) {
	Common::SeekableReadStream *fileStream = getVolumeFile(resMan, res);
	if (!fileStream)
		return;

	fileStream->seek(res->_fileOffset, SEEK_SET);

	// For compressed audio, using loadFromAudioVolumeSCI1 is a hack to bypass
	// the resource type checking in loadFromAudioVolumeSCI11 (since
	// loadFromAudioVolumeSCI1 does nothing more than read raw data)
	if (_audioCompressionType != 0 &&
		(res->getType() == kResourceTypeAudio ||
		 res->getType() == kResourceTypeAudio36)) {
		res->loadFromAudioVolumeSCI1(fileStream);
	} else if (getSciVersion() < SCI_VERSION_1_1)
		res->loadFromAudioVolumeSCI1(fileStream);
	else
		res->loadFromAudioVolumeSCI11(fileStream);

	resMan->disposeVolumeFileStream(fileStream, this);
}

bool ResourceManager::addAudioSources() {
#ifdef ENABLE_SCI32
	// Multi-disc audio is added during addAppropriateSources for those titles
	// that require it
	if (_multiDiscAudio) {
		return true;
	}
#endif

	Common::List<ResourceId> resources = listResources(kResourceTypeMap);
	Common::List<ResourceId>::iterator itr;

	for (itr = resources.begin(); itr != resources.end(); ++itr) {
		const Resource *mapResource = _resMap.getVal(*itr);
		ResourceSource *src = addSource(new IntMapResourceSource(mapResource->getResourceLocation(), 0, itr->getNumber()));

		if (itr->getNumber() == 65535 && Common::File::exists("RESOURCE.SFX"))
			addSource(new AudioVolumeResourceSource(this, "RESOURCE.SFX", src, 0));
		else if (Common::File::exists("RESOURCE.AUD"))
			addSource(new AudioVolumeResourceSource(this, "RESOURCE.AUD", src, 0));
		else
			return false;
	}

	return true;
}

void ResourceManager::changeAudioDirectory(const Common::Path &path) {
	const Common::Path resAudPath = path.join("RESOURCE.AUD");

	if (!SearchMan.hasFile(resAudPath)) {
		error("Could not find %s", resAudPath.toString().c_str());
	}

	// When a IntMapResourceSource is scanned, it will not update existing
	// resources. There is also no guarantee that there are exactly the same
	// number of audio36/sync36/map resources in each audio directory.
	// Therefore, all of these resources must be deleted before scanning.
	for (ResourceMap::iterator it = _resMap.begin(); it != _resMap.end(); ++it) {
		const ResourceType type = it->_key.getType();

		if (type == kResourceTypeMap || type == kResourceTypeAudio36 || type == kResourceTypeSync36) {
			if (type == kResourceTypeMap && it->_key.getNumber() == 65535) {
				continue;
			}

			Resource *resource = it->_value;
			if (resource) {
				// If one of these resources ends up being locked here, it
				// probably means Audio32 is using it and we need to stop
				// playback of audio before switching directories
				assert(!resource->isLocked());

				if (resource->_status == kResStatusEnqueued) {
					removeFromLRU(resource);
				}

				// A PatchResourceSource is not added to _sources and is
				// automatically deleted when the corresponding Resource is
				// deleted
				delete resource;
			}

			_resMap.erase(it);
		}
	}

	for (SourcesList::iterator it = _sources.begin(); it != _sources.end(); ) {
		IntMapResourceSource *mapSource = dynamic_cast<IntMapResourceSource *>(*it);
		if (mapSource && mapSource->_mapNumber != 65535) {
			delete *it;
			it = _sources.erase(it);
			continue;
		}

		AudioVolumeResourceSource *volSource = dynamic_cast<AudioVolumeResourceSource *>(*it);
		if (volSource && volSource->getLocationName().baseName() == "RESOURCE.AUD") {
			delete volSource;
			it = _sources.erase(it);
			continue;
		}

		++it;
	}

	// # is used as the first pattern character to avoid matching non-audio maps
	// like RESOURCE.MAP
	Common::ArchiveMemberList mapFiles;
	SearchMan.listMatchingMembers(mapFiles, path.join("#*.MAP"));

	for (Common::ArchiveMemberList::const_iterator it = mapFiles.begin(); it != mapFiles.end(); ++it) {
		const Common::ArchiveMemberPtr &file = *it;
		assert(file);

		const Common::String fileName = file->getFileName();
		const int mapNo = atoi(fileName.c_str());

		// Sound effects are the same across all audio directories, so ignore
		// any new SFX map
		if (mapNo == 65535) {
			continue;
		}

		ResourceSource *newSource = new PatchResourceSource(path.appendComponent(fileName));
		processPatch(newSource, kResourceTypeMap, mapNo);
		Resource *mapResource = _resMap.getVal(ResourceId(kResourceTypeMap, mapNo));
		assert(mapResource);

		ResourceSource *audioMap = addSource(new IntMapResourceSource(mapResource->getResourceLocation(), 0, mapNo));
		addSource(new AudioVolumeResourceSource(this, resAudPath, audioMap, 0));
	}

	scanNewSources();
}

void ResourceManager::changeMacAudioDirectory(const Common::Path &path_) {
	// delete all Audio36 resources so that they can be replaced with
	//  different patch files from the new directory.
	for (ResourceMap::iterator it = _resMap.begin(); it != _resMap.end(); ++it) {
		const ResourceType type = it->_key.getType();

		if (type == kResourceTypeAudio36) {
			Resource *resource = it->_value;
			if (resource) {
				// If one of these resources ends up being locked here, it
				// probably means Audio32 is using it and we need to stop
				// playback of audio before switching directories
				assert(!resource->isLocked());

				if (resource->_status == kResStatusEnqueued) {
					removeFromLRU(resource);
				}

				delete resource;
			}

			_resMap.erase(it);
		}
	}

	Common::Path path(path_);
	if (path.empty()) {
		path = "english";
	}
	path = Common::Path("voices").join(path);

	// add all Audio36 wave patch files from language directory
	Common::ArchiveMemberList audio36Files;
	SearchMan.listMatchingMembers(audio36Files, path.join("A???????.???"));
	for (Common::ArchiveMemberList::const_iterator it = audio36Files.begin(); it != audio36Files.end(); ++it) {
		const Common::ArchiveMemberPtr &file = *it;
		assert(file);

		ResourceId resource36 = convertPatchNameBase36(kResourceTypeAudio36, file->getFileName());
		processWavePatch(resource36, file->getPathInArchive());
	}
}

} // End of namespace Sci
