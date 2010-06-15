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
#include "sci/resource_intern.h"
#include "sci/util.h"

namespace Sci {

AudioVolumeResourceSource::AudioVolumeResourceSource(const Common::String &name, ResourceSource *map, int volNum)
	: VolumeResourceSource(name, map, volNum, kSourceAudioVolume) {

	_audioCompressionType = 0;
	_audioCompressionOffsetMapping = NULL;

	/*
	 * Check if this audio volume got compressed by our tool. If that is the
	 * case, set _audioCompressionType and read in the offset translation
	 * table for later usage.
	 */

	Common::SeekableReadStream *fileStream = getVolumeFile(0);
	if (!fileStream)
		return;

	fileStream->seek(0, SEEK_SET);
	uint32 compressionType = fileStream->readUint32BE();
	switch (compressionType) {
	case MKID_BE('MP3 '):
	case MKID_BE('OGG '):
	case MKID_BE('FLAC'):
		// Detected a compressed audio volume
		_audioCompressionType = compressionType;
		// Now read the whole offset mapping table for later usage
		int32 recordCount = fileStream->readUint32LE();
		if (!recordCount)
			error("compressed audio volume doesn't contain any entries!");
		int32 *offsetMapping = new int32[(recordCount + 1) * 2];
		_audioCompressionOffsetMapping = offsetMapping;
		for (int recordNo = 0; recordNo < recordCount; recordNo++) {
			*offsetMapping++ = fileStream->readUint32LE();
			*offsetMapping++ = fileStream->readUint32LE();
		}
		// Put ending zero
		*offsetMapping++ = 0;
		*offsetMapping++ = fileStream->size();
	}

	if (_resourceFile)
		delete fileStream;
}

bool ResourceManager::loadFromWaveFile(Resource *res, Common::SeekableReadStream *file) {
	res->data = new byte[res->size];

	uint32 really_read = file->read(res->data, res->size);
	if (really_read != res->size)
		error("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->size);

	res->_status = kResStatusAllocated;
	return true;
}

bool ResourceManager::loadFromAudioVolumeSCI11(Resource *res, Common::SeekableReadStream *file) {
	// Check for WAVE files here
	uint32 riffTag = file->readUint32BE();
	if (riffTag == MKID_BE('RIFF')) {
		res->_headerSize = 0;
		res->size = file->readUint32LE();
		file->seek(-8, SEEK_CUR);
		return loadFromWaveFile(res, file);
	}
	file->seek(-4, SEEK_CUR);

	ResourceType type = (ResourceType)(file->readByte() & 0x7f);
	if (((res->_id.type == kResourceTypeAudio || res->_id.type == kResourceTypeAudio36) && (type != kResourceTypeAudio))
		|| ((res->_id.type == kResourceTypeSync || res->_id.type == kResourceTypeSync36) && (type != kResourceTypeSync))) {
		warning("Resource type mismatch loading %s", res->_id.toString().c_str());
		res->unalloc();
		return false;
	}

	res->_headerSize = file->readByte();

	if (type == kResourceTypeAudio) {
		if (res->_headerSize != 11 && res->_headerSize != 12) {
			warning("Unsupported audio header");
			res->unalloc();
			return false;
		}

		// Load sample size
		file->seek(7, SEEK_CUR);
		res->size = file->readUint32LE();
		// Adjust offset to point at the header data again
		file->seek(-11, SEEK_CUR);
	}

	return loadPatch(res, file);
}

bool ResourceManager::loadFromAudioVolumeSCI1(Resource *res, Common::SeekableReadStream *file) {
	res->data = new byte[res->size];

	if (res->data == NULL) {
		error("Can't allocate %d bytes needed for loading %s", res->size, res->_id.toString().c_str());
	}

	unsigned int really_read = file->read(res->data, res->size);
	if (really_read != res->size)
		warning("Read %d bytes from %s but expected %d", really_read, res->_id.toString().c_str(), res->size);

	res->_status = kResStatusAllocated;
	return true;
}

void ResourceManager::addNewGMPatch(const Common::String &gameId) {
	Common::String gmPatchFile;

	if (gameId == "ecoquest")
		gmPatchFile = "ECO1GM.PAT";
	else if (gameId == "hoyle3")
		gmPatchFile = "HOY3GM.PAT";
	else if (gameId == "hoyle3")
		gmPatchFile = "HOY3GM.PAT";
	else if (gameId == "lsl1sci")
		gmPatchFile = "LL1_GM.PAT";
	else if (gameId == "lsl5")
		gmPatchFile = "LL5_GM.PAT";
	else if (gameId == "longbow")
		gmPatchFile = "ROBNGM.PAT";
	else if (gameId == "sq1sci")
		gmPatchFile = "SQ1_GM.PAT";
	else if (gameId == "sq4")
		gmPatchFile = "SQ4_GM.PAT";
	else if (gameId == "fairytales")
		gmPatchFile = "TALEGM.PAT";

	if (!gmPatchFile.empty() && Common::File::exists(gmPatchFile)) {
		ResourceSource *psrcPatch = new PatchResourceSource(gmPatchFile);
		processPatch(psrcPatch, kResourceTypePatch, 4);
	}
}

void ResourceManager::processWavePatch(ResourceId resourceId, Common::String name) {
	ResourceSource *resSrc = new WaveResourceSource(name);
	Common::File file;
	file.open(name);

	updateResource(resourceId, resSrc, name.size());

	debugC(1, kDebugLevelResMan, "Patching %s - OK", name.c_str());
}

void ResourceManager::readWaveAudioPatches() {
	// Here we do check for SCI1.1+ so we can patch wav files in as audio resources
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "*.wav");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		Common::String name = (*x)->getName();

		if (isdigit(name[0]))
			processWavePatch(ResourceId(kResourceTypeAudio, atoi(name.c_str())), name);
	}
}

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
	Resource *mapRes = findResource(ResourceId(kResourceTypeMap, map->_volumeNumber), false);

	if (!mapRes) {
		warning("Failed to open %i.MAP", map->_volumeNumber);
		return SCI_ERROR_RESMAP_NOT_FOUND;
	}

	ResourceSource *src = findVolume(map, 0);

	if (!src)
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;

	byte *ptr = mapRes->data;

	if (map->_volumeNumber == 65535) {
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
					addResource(ResourceId(kResourceTypeSync36, map->_volumeNumber, n & 0xffffff3f), src, offset, syncSize);
			}

			if (n & 0x40) {
				// This seems to define the size of raw lipsync data (at least in kq6), may also just be general appended
				//  data
				syncSize += READ_LE_UINT16(ptr);
				ptr += 2;
			}

			addResource(ResourceId(kResourceTypeAudio36, map->_volumeNumber, n & 0xffffff3f), src, offset + syncSize);
		}
	}

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

	while (1) {
		uint16 n = file.readUint16LE();
		uint32 offset = file.readUint32LE();
		uint32 size = file.readUint32LE();

		if (file.eos() || file.err()) {
			warning("Error while reading %s", map->getLocationName().c_str());
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
		if (_audioMapSCI1->_volumeNumber == language) {
			// This language is already loaded
			return;
		}

		// We already have a map loaded, so we unload it first
		readAudioMapSCI1(_audioMapSCI1, true);

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

		_audioMapSCI1 = NULL;
	}

	char filename[9];
	snprintf(filename, 9, "AUDIO%03d", language);

	Common::String fullname = Common::String(filename) + ".MAP";
	if (!Common::File::exists(fullname)) {
		warning("No audio map found for language %i", language);
		return;
	}

	_audioMapSCI1 = addSource(new ExtAudioMapResourceSource(fullname, language));

	// Search for audio volumes for this language and add them to the source list
	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, Common::String(filename) + ".0??");
	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addSource(new AudioVolumeResourceSource(name, _audioMapSCI1, number));
	}

	scanNewSources();
}

int ResourceManager::getAudioLanguage() const {
	return (_audioMapSCI1 ? _audioMapSCI1->_volumeNumber : 0);
}

SoundResource::SoundResource(uint32 resNumber, ResourceManager *resMan, SciVersion soundVersion) : _resMan(resMan), _soundVersion(soundVersion) {
	Resource *resource = _resMan->findResource(ResourceId(kResourceTypeSound, resNumber), true);
	int trackNr, channelNr;
	if (!resource)
		return;

	_innerResource = resource;

	byte *data, *data2;
	byte *dataEnd;
	Channel *channel, *sampleChannel;

	_channelsUsed = 0;

	switch (_soundVersion) {
	case SCI_VERSION_0_EARLY:
	case SCI_VERSION_0_LATE:
		// SCI0 only has a header of 0x11/0x21 byte length and the actual midi track follows afterwards
		_trackCount = 1;
		_tracks = new Track[_trackCount];
		_tracks->digitalChannelNr = -1;
		_tracks->type = 0; // Not used for SCI0
		_tracks->channelCount = 1;
		// Digital sample data included? -> Add an additional channel
		if (resource->data[0] == 2)
			_tracks->channelCount++;
		_tracks->channels = new Channel[_tracks->channelCount];
		memset(_tracks->channels, 0, sizeof(Channel) * _tracks->channelCount);
		channel = &_tracks->channels[0];
		if (_soundVersion == SCI_VERSION_0_EARLY) {
			channel->data = resource->data + 0x11;
			channel->size = resource->size - 0x11;
		} else {
			channel->data = resource->data + 0x21;
			channel->size = resource->size - 0x21;
		}
		if (_tracks->channelCount == 2) {
			// Digital sample data included
			_tracks->digitalChannelNr = 1;
			sampleChannel = &_tracks->channels[1];
			// we need to find 0xFC (channel terminator) within the data
			data = channel->data;
			dataEnd = channel->data + channel->size;
			while ((data < dataEnd) && (*data != 0xfc))
				data++;
			// Skip any following 0xFCs as well
			while ((data < dataEnd) && (*data == 0xfc))
				data++;
			// Now adjust channels accordingly
			sampleChannel->data = data;
			sampleChannel->size = channel->size - (data - channel->data);
			channel->size = data - channel->data;
			// Read sample header information
			//Offset 14 in the header contains the frequency as a short integer. Offset 32 contains the sample length, also as a short integer.
			_tracks->digitalSampleRate = READ_LE_UINT16(sampleChannel->data + 14);
			_tracks->digitalSampleSize = READ_LE_UINT16(sampleChannel->data + 32);
			_tracks->digitalSampleStart = 0;
			_tracks->digitalSampleEnd = 0;
			sampleChannel->data += 44; // Skip over header
			sampleChannel->size -= 44;
		}
		break;

	case SCI_VERSION_1_EARLY:
	case SCI_VERSION_1_LATE:
		data = resource->data;
		// Count # of tracks
		_trackCount = 0;
		while ((*data++) != 0xFF) {
			_trackCount++;
			while (*data != 0xFF)
				data += 6;
			data++;
		}
		_tracks = new Track[_trackCount];
		data = resource->data;
		for (trackNr = 0; trackNr < _trackCount; trackNr++) {
			// Track info starts with track type:BYTE
			// Then the channel information gets appended Unknown:WORD, ChannelOffset:WORD, ChannelSize:WORD
			// 0xFF:BYTE as terminator to end that track and begin with another track type
			// Track type 0xFF is the marker signifying the end of the tracks

			_tracks[trackNr].type = *data++;
			// Counting # of channels used
			data2 = data;
			_tracks[trackNr].channelCount = 0;
			while (*data2 != 0xFF) {
				data2 += 6;
				_tracks[trackNr].channelCount++;
			}
			_tracks[trackNr].channels = new Channel[_tracks[trackNr].channelCount];
			_tracks[trackNr].digitalChannelNr = -1; // No digital sound associated
			_tracks[trackNr].digitalSampleRate = 0;
			_tracks[trackNr].digitalSampleSize = 0;
			_tracks[trackNr].digitalSampleStart = 0;
			_tracks[trackNr].digitalSampleEnd = 0;
			if (_tracks[trackNr].type != 0xF0) { // Digital track marker - not supported currently
				for (channelNr = 0; channelNr < _tracks[trackNr].channelCount; channelNr++) {
					channel = &_tracks[trackNr].channels[channelNr];
					channel->prio = READ_LE_UINT16(data);
					channel->data = resource->data + READ_LE_UINT16(data + 2) + 2;
					channel->size = READ_LE_UINT16(data + 4) - 2; // Not counting channel header
					channel->number = *(channel->data - 2);
					setChannelUsed(channel->number);
					channel->poly = *(channel->data - 1);
					channel->time = channel->prev = 0;
					if (channel->number == 0xFE) { // Digital channel
						_tracks[trackNr].digitalChannelNr = channelNr;
						_tracks[trackNr].digitalSampleRate = READ_LE_UINT16(channel->data);
						_tracks[trackNr].digitalSampleSize = READ_LE_UINT16(channel->data + 2);
						_tracks[trackNr].digitalSampleStart = READ_LE_UINT16(channel->data + 4);
						_tracks[trackNr].digitalSampleEnd = READ_LE_UINT16(channel->data + 6);
						channel->data += 8; // Skip over header
						channel->size -= 8;
					}
					data += 6;
				}
			} else {
				// Skip over digital track
				data += 6;
			}
			data++; // Skipping 0xFF that closes channels list
		}
		break;

	default:
		error("SoundResource: SCI version %d is unsupported", _soundVersion);
	}
}

SoundResource::~SoundResource() {
	for (int trackNr = 0; trackNr < _trackCount; trackNr++)
		delete[] _tracks[trackNr].channels;
	delete[] _tracks;

	_resMan->unlockResource(_innerResource);
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
	return NULL;
}

SoundResource::Track *SoundResource::getDigitalTrack() {
	for (int trackNr = 0; trackNr < _trackCount; trackNr++) {
		if (_tracks[trackNr].digitalChannelNr != -1)
			return &_tracks[trackNr];
	}
	return NULL;
}

// Gets the filter mask for SCI0 sound resources
int SoundResource::getChannelFilterMask(int hardwareMask, bool wantsRhythm) {
	byte *data = _innerResource->data;
	int channelMask = 0;

	if (_soundVersion > SCI_VERSION_0_LATE)
		return 0;

	data++; // Skip over digital sample flag

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
			data++;

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
			play = wantsRhythm;
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

byte SoundResource::getInitialVoiceCount(byte channel) {
	byte *data = _innerResource->data;

	if (_soundVersion > SCI_VERSION_0_LATE)
		return 0; // TODO

	data++; // Skip over digital sample flag

	if (_soundVersion == SCI_VERSION_0_EARLY)
		return data[channel] >> 4;
	else
		return data[channel * 2];
}

} // End of namespace Sci
