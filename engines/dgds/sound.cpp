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

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/stream.h"

#include "audio/audiostream.h"
#include "audio/decoders/aiff.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"

#include "dgds/decompress.h"
#include "dgds/includes.h"
#include "dgds/parser.h"
#include "dgds/resource.h"
#include "dgds/sound.h"
#include "dgds/sound/music.h"
#include "dgds/sound/resource/sci_resource.h"

namespace Dgds {

static const uint16 SIGNAL_OFFSET = 0xffff;

// Offsets from sound/music num to playing ID.
// This is to make a fake SCI-style "resource ID"
static const int SND_RESOURCE_OFFSET = 4096;
static const int MUSIC_RESOURCE_OFFSET = 8192;

static const uint16 FLAG_LOOP = 1;

static void _readHeader(const byte* &pos, uint32 &sci_header) {
	sci_header = 0;
	if (READ_LE_UINT16(pos) == 0x0084)
		sci_header = 2;

	pos += sci_header;
	if (pos[0] == 0xF0) {
		debug("SysEx transfer = %d bytes", pos[1]);
		pos += 2;
		pos += 6;
	}
}

static void _readPartHeader(const byte* &pos, uint16 &off, uint16 &siz) {
	pos += 2;
	off = READ_LE_UINT16(pos);
	pos += 2;
	siz = READ_LE_UINT16(pos);
	pos += 2;
}

static void _skipPartHeader(const byte* &pos) {
	pos += 6;
}

static uint32 _availableSndTracks(const byte *data, uint32 size) {
	const byte *pos = data;

	uint32 sci_header;
	_readHeader(pos, sci_header);

	uint32 tracks = 0;
	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		//debug("(%d)", drv);

		while (pos[0] != 0xFF) {
			uint16 off, siz;
			_readPartHeader(pos, off, siz);
			off += sci_header;

			//debug("%06d:%d ", off, siz);

			//debug("Header bytes");
			//debug("[%06X]  ", data[off]);
			//debug("[%02X]  ", data[off+0]);
			//debug("[%02X]  ", data[off+1]);

			bool digital_pcm = false;
			if (READ_LE_UINT16(&data[off]) == 0x00FE) {
				digital_pcm = true;
			}

			switch (drv) {
			case 0:	if (digital_pcm) {
					//debug("- Soundblaster");
					tracks |= DIGITAL_PCM;
				} else {
					//debug("- Adlib");
					tracks |= TRACK_ADLIB;
				}
				break;
			case 7:
				//debug("- General MIDI");
				tracks |= TRACK_GM;
				break;
			case 9:
				//debug("- CMS");
				tracks |= TRACK_CMS;
				break;
			case 12:
				//debug("- MT-32");
				tracks |= TRACK_MT32;
				break;
			case 18:
				//debug("- PC Speaker");
				tracks |= TRACK_PCSPK;
				break;
			case 19:
				//debug("- Tandy 1000");
				tracks |= TRACK_TANDY;
				break;
			default:
				//debug("- Unknown %d", drv);
				warning("Unknown music type %d", drv);
				break;
			}
		}

		pos++;
	}
	pos++;
	return tracks;
}


static byte _loadSndTrack(uint32 track, const byte** trackPtr, uint16* trackSiz, const byte *data, uint32 size) {
	byte matchDrv;
	switch (track) {
	case DIGITAL_PCM:
	case TRACK_ADLIB: matchDrv = 0;    break;
	case TRACK_GM:	  matchDrv = 7;    break;
	case TRACK_MT32:  matchDrv = 12;   break;
	default:			   return 0;
	}

	const byte *pos = data;

	uint32 sci_header;
	_readHeader(pos, sci_header);

	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		byte part;
		const byte *ptr;

		part = 0;
		for (ptr = pos; *ptr != 0xFF; _skipPartHeader(ptr))
			part++;

		if (matchDrv == drv) {
			part = 0;
			while (pos[0] != 0xFF) {
				uint16 off, siz;
				_readPartHeader(pos, off, siz);
				off += sci_header;

				trackPtr[part] = data + off;
				trackSiz[part] = siz;
				part++;
			}
			debug("- (%d) Play parts = %d", drv, part);
			return part;
		} else {
			pos = ptr;
		}
		pos++;
	}
	pos++;
	return 0;
}


Sound::Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor) :
	_mixer(mixer), _resource(resource), _decompressor(decompressor), _music(nullptr) {
	ARRAYCLEAR(_channels);
	_music = new SciMusic(true);
	_music->init();
}

Sound::~Sound() {
	unloadMusic();

	for (auto &data: _sfxData)
		delete [] data._data;
}

void Sound::playAmigaSfx(const Common::String &filename, byte channel, byte volume) {
	if (!filename.hasSuffixIgnoreCase(".ins"))
		error("Unhandled SFX file type: %s", filename.c_str());

	Common::SeekableReadStream *sfxStream = _resource->getResource(filename);
	if (!sfxStream) {
		warning("SFX file %s not found", filename.c_str());
		return;
	}

	byte *dest = new byte[sfxStream->size()];
	sfxStream->read(dest, sfxStream->size());
	Common::MemoryReadStream *soundData = new Common::MemoryReadStream(dest, sfxStream->size(), DisposeAfterUse::YES);
	delete sfxStream;

	stopSfxForChannel(channel);

	Channel *ch = &_channels[channel];
	Audio::AudioStream *input = Audio::makeAIFFStream(soundData, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
}

void Sound::stopAllSfx() {
	_music->stopSFX();
	for (uint i = 0; i < ARRAYSIZE(_channels); i++)
		stopSfxForChannel(i);
}

void Sound::stopSfxForChannel(byte channel) {
	if (_mixer->isSoundHandleActive(_channels[channel].handle)) {
		_mixer->stopHandle(_channels[channel].handle);
		_channels[channel].stream = 0;
	}
}

bool Sound::playPCM(const byte *data, uint32 size) {
	_mixer->stopAll();

	if (!data)
		return false;

	const byte *trackPtr[0xFF];
	uint16 trackSiz[0xFF];
	byte numParts = _loadSndTrack(DIGITAL_PCM, trackPtr, trackSiz, data, size);
	if (numParts == 0)
		return false;

	for (byte part = 0; part < numParts; part++) {
		const byte *ptr = trackPtr[part];

		bool digital_pcm = false;
		if (READ_LE_UINT16(ptr) == 0x00FE) {
			digital_pcm = true;
		}
		ptr += 2;

		if (!digital_pcm)
			continue;

		uint16 rate, length, first, last;
		rate = READ_LE_UINT16(ptr);
		length = READ_LE_UINT16(ptr + 2);
		first = READ_LE_UINT16(ptr + 4);
		last = READ_LE_UINT16(ptr + 6);
		ptr += 8;

		ptr += first;
		debug(" - Digital PCM: %u Hz, [%u]=%u:%u",
			  rate, length, first, last);
		trackPtr[part] = ptr;
		trackSiz[part] = length;

		Channel *ch = &_channels[part];
		byte volume = 127;
		Audio::AudioStream *input = Audio::makeRawStream(trackPtr[part], trackSiz[part],
														 rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume, 0, DisposeAfterUse::YES);
	}
	return true;
}

static void _readStrings(Common::SeekableReadStream *stream) {
	uint16 count = stream->readUint16LE();
	debug("        %u strs:", count);

	for (uint16 k = 0; k < count; k++) {
		uint16 idx = stream->readUint16LE();
		Common::String str = stream->readString();

		debug("        %2u: %2u, \"%s\"", k, idx, str.c_str());
	}
}

bool Sound::loadMacMusic(const Common::String &filename) {
	if (filename.hasSuffixIgnoreCase(".sng")) {
		Common::String macFileName = filename.substr(0, filename.find(".")) + ".sx";
		return loadMacMusic(macFileName);
	}

	if (!filename.hasSuffixIgnoreCase(".sx"))
		error("Unhandled music file type: %s", filename.c_str());

	if (filename == _currentMusic)
		return false;

	Common::SeekableReadStream *musicStream = _resource->getResource(filename);
	if (!musicStream) {
		warning("Music file %s not found", filename.c_str());
		return false;
	}

	DgdsChunkReader chunk(musicStream);

	while (chunk.readNextHeader(EX_SX, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);
		Common::SeekableReadStream *stream = chunk.getContent();

		if (chunk.isSection(ID_INF)) {
			uint16 type = stream->readUint16LE();
			uint16 count = stream->readUint16LE();

			debug("  SX INF %u [%u entries]:  (%s)", type, count, filename.c_str());
			for (uint16 k = 0; k < count; k++) {
				uint16 idx = stream->readUint16LE();
				debug("        %2u: %u", k, idx);
				_musicIdMap[idx] = k;
			}
		} else if (chunk.isSection(ID_TAG) || chunk.isSection(ID_FNM)) {
			_readStrings(stream);
		} else if (chunk.isSection(ID_DAT)) {
			// TODO: Should we record the indexes?
			/*uint16 idx = */ stream->readUint16LE();
			/*uint16 type = */ stream->readUint16LE();
			SoundData soundData;
			soundData._data = _decompressor->decompress(stream, stream->size() - stream->pos(), soundData._size);
			_musicData.push_back(soundData);
		}
	}

	delete musicStream;
	_currentMusic = filename;
	return true;
}

bool Sound::loadMusic(const Common::String &filename) {
	if (filename == _currentMusic)
		return false;
	unloadMusic();
	loadPCSound(filename, _musicData);
	_currentMusic = filename;
	debug("Sound: Loaded music %s with %d entries", filename.c_str(), _musicData.size());
	return true;
}

void Sound::loadSFX(const Common::String &filename) {
	if (_sfxData.size())
		error("Sound: SFX data should only be loaded once");
	loadPCSound(filename, _sfxData);
}

void Sound::loadPCSound(const Common::String &filename, Common::Array<SoundData> &dataArray) {
	if (!filename.hasSuffixIgnoreCase(".sng"))
		error("Unhandled music file type: %s", filename.c_str());

	Common::SeekableReadStream *musicStream = _resource->getResource(filename);
	if (!musicStream)
		error("Music file %s not found", filename.c_str());

	DgdsChunkReader chunk(musicStream);

	while (chunk.readNextHeader(EX_SNG, filename)) {
		if (chunk.isContainer()) {
			continue;
		}

		chunk.readContent(_decompressor);

		Common::SeekableReadStream *stream = chunk.getContent();
		if (chunk.isSection(ID_SNG)) {
			SoundData soundData;
			soundData._size = stream->size();
			byte *data = new byte[soundData._size];
			stream->read(data, soundData._size);
			soundData._data = data;
			dataArray.push_back(soundData);
		} else if (chunk.isSection(ID_INF)) {
			uint32 count = stream->size() / 2;
			if (count > dataArray.size())
				error("Sound: %s has more flags in INF than SNG entries.", filename.c_str());
			debug("  SNG INF [%u entries]", count);
			for (uint32 k = 0; k < count; k++) {
				uint16 flags = stream->readUint16LE();
				debug("        %2u: 0x%04x", k, flags);
				dataArray[k]._flags = flags;
			}
		} else {
			warning("loadPCSound: skip unused chunk %s in %s", chunk.getIdStr(), filename.c_str());
		}
	}

	delete musicStream;
}

int Sound::mapSfxNum(int num) const {
	// Fixed offset in Dragon and HoC?
	if (DgdsEngine::getInstance()->getGameId() == GID_DRAGON || DgdsEngine::getInstance()->getGameId() == GID_HOC)
		return num - 24;
	return num;
}

void Sound::playSFX(int num) {
	int mappedNum = mapSfxNum(num);
	debug("Sound: Play SFX %d (-> %d), have %d entries", num, mappedNum, _sfxData.size());
	playPCSound(mappedNum, _sfxData, Audio::Mixer::kSFXSoundType);
}

void Sound::stopSfxByNum(int num) {
	int mappedNum = mapSfxNum(num);
	debug("Sound: Stop SFX %d (-> %d)", num, mappedNum);

	MusicEntry *musicSlot = _music->getSlot(mappedNum + SND_RESOURCE_OFFSET);
	if (!musicSlot) {
		debug("stopSfxByNum: Slot for sfx num %d not found.", mappedNum);
		return;
	}

	musicSlot->dataInc = 0;
	musicSlot->signal = SIGNAL_OFFSET;
	_music->soundStop(musicSlot);
}

void Sound::playMusic(int num) {
	debug("Sound: Play music %d (%s), have %d entries", num, _currentMusic.c_str(), _musicData.size());
	playPCSound(num, _musicData, Audio::Mixer::kMusicSoundType);
}

void Sound::processInitSound(uint32 obj, const SoundData &data, Audio::Mixer::SoundType soundType) {
	// Check if a track with the same sound object is already playing
	MusicEntry *oldSound = _music->getSlot(obj);
	if (oldSound) {
		processDisposeSound(obj);
	}

	MusicEntry *newSound = new MusicEntry();
	newSound->resourceId = obj;
	newSound->soundObj = obj;
	newSound->loop = 0; // set in processPlaySound
	newSound->overridePriority = false;
	newSound->priority = 255;
	newSound->volume = MUSIC_VOLUME_DEFAULT;
	newSound->reverb = -1;	// initialize to SCI invalid, it'll be set correctly in soundInitSnd() below

	debug("processInitSound: %08x number %d, loop %d, prio %d, vol %d", obj,
			obj, newSound->loop, newSound->priority, newSound->volume);

	initSoundResource(newSound, data, soundType);

	_music->pushBackSlot(newSound);
}

void Sound::initSoundResource(MusicEntry *newSound, const SoundData &data, Audio::Mixer::SoundType soundType) {
	if (newSound->resourceId) {
		// Skip the header.
		const byte *dataPtr = data._data;
		uint32 hdrSize = 0;
		_readHeader(dataPtr, hdrSize);
		newSound->soundRes = new SoundResource(newSound->resourceId, dataPtr, data._size - hdrSize);
		if (!newSound->soundRes->exists()) {
			delete newSound->soundRes;
			newSound->soundRes = nullptr;
		}
	} else {
		newSound->soundRes = nullptr;
	}

	if (!newSound->isSample && newSound->soundRes)
		_music->soundInitSnd(newSound);

	newSound->soundType = soundType;
}

void Sound::processDisposeSound(uint32 obj) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("processDisposeSound: Slot not found (%08x)", obj);
		return;
	}

	processStopSound(obj, false);

	_music->soundKill(musicSlot);
}

void Sound::processStopSound(uint32 obj, bool sampleFinishedPlaying) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);
	if (!musicSlot) {
		warning("processStopSound: Slot not found (%08x)", obj);
		return;
	}

	musicSlot->dataInc = 0;
	musicSlot->signal = SIGNAL_OFFSET;
	_music->soundStop(musicSlot);
}

void Sound::processPlaySound(uint32 obj, bool playBed, bool restoring, const SoundData &data) {
	// Mostly copied from SCI soundcmd.
	MusicEntry *musicSlot = _music->getSlot(obj);

	if (!musicSlot) {
		error("kDoSound(play): Slot not found (%08x)", obj);
	}

	int32 resourceId;
	if (!restoring)
		resourceId = obj;
	else
		// Handle cases where a game was saved while track A was playing, but track B was initialized, waiting to be played later.
		// In such cases, musicSlot->resourceId contains the actual track that was playing (A), while getSoundResourceId(obj)
		// contains the track that's waiting to be played later (B) - bug #10907.
		resourceId = musicSlot->resourceId;

	if (musicSlot->resourceId != resourceId) { // another sound loaded into struct
		processDisposeSound(obj);
		processInitSound(obj, data, Audio::Mixer::kSFXSoundType);
		// Find slot again :)
		musicSlot = _music->getSlot(obj);
	}

	assert(musicSlot);

	musicSlot->loop = (data._flags & FLAG_LOOP) ? 1 : 0;

	// Get song priority from either obj or soundRes
	byte resourcePriority = 0xFF;
	if (musicSlot->soundRes)
		resourcePriority = musicSlot->soundRes->getSoundPriority();
	if (!musicSlot->overridePriority && resourcePriority != 0xFF) {
		musicSlot->priority = resourcePriority;
	} else {
		musicSlot->priority = 255;
	}

	// Reset hold when starting a new song. kDoSoundSetHold is always called after
	// kDoSoundPlay to set it properly, if needed. Fixes bug #5851.
	musicSlot->hold = -1;
	musicSlot->playBed = playBed;
	musicSlot->volume = MUSIC_VOLUME_DEFAULT;

	debug("processPlaySound: %08x number %d, sz %d, loop %d, prio %d, vol %d, bed %d", obj,
			resourceId, data._size, musicSlot->loop, musicSlot->priority, musicSlot->volume, playBed ? 1 : 0);

	_music->soundPlay(musicSlot, restoring);

	// Reset any left-over signals
	musicSlot->signal = 0;
	musicSlot->fadeStep = 0;
}

void Sound::playPCSound(int num, const Common::Array<SoundData> &dataArray, Audio::Mixer::SoundType soundType) {
	if (_musicIdMap.size()) {
		num = _musicIdMap[num];
	}

	if (num >= 0 && num < (int)dataArray.size()) {
		const SoundData &data = dataArray[num];
		uint32 tracks = _availableSndTracks(data._data, data._size);
		if (tracks & DIGITAL_PCM) {
			playPCM(data._data, data._size);
		} else {
			int idOffset = soundType == Audio::Mixer::kSFXSoundType ? SND_RESOURCE_OFFSET : MUSIC_RESOURCE_OFFSET;
			processInitSound(num + idOffset, data, soundType);
			processPlaySound(num + idOffset, false, false, data);
		}
	} else {
		warning("Sound: Requested to play %d but only have %d tracks", num, dataArray.size());
	}
}

void Sound::stopMusic() {
	debug("Sound: Stop music.");
	_music->stopMusic();
}

void Sound::unloadMusic() {
	stopMusic();
	for (auto &data: _musicData)
		delete [] data._data;
	_musicData.clear();

	// Don't unload sfxData.
}


} // End of namespace Dgds

