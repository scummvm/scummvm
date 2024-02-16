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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "dgds/music.h"
#include "dgds/parser.h"
#include "dgds/resource.h"
#include "dgds/sound.h"

namespace Dgds {

Sound::Sound(Audio::Mixer *mixer, ResourceManager *resource, Decompressor *decompressor) :
	_mixer(mixer), _resource(resource), _decompressor(decompressor) {
	_midiPlayer = new DgdsMidiPlayer();
}

Sound::~Sound() {
	delete _midiPlayer;
	delete _soundData;
	delete[] _musicData;
}

void Sound::loadMusic(Common::SeekableReadStream &file, Decompressor *decompressor) {
	_musicData = decompressor->decompress(&file, file.size() - file.pos(), _musicSize);
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
	_soundData = new Common::MemoryReadStream(dest, sfxStream->size(), DisposeAfterUse::YES);
	delete sfxStream;

	stopSfx(channel);

	if (_soundData) {
		Channel *ch = &_channels[channel];
		Audio::AudioStream *input = Audio::makeAIFFStream(_soundData, DisposeAfterUse::YES);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
		_soundData = 0;
	}
}

void Sound::stopSfx(byte channel) {
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
	byte numParts = loadSndTrack(DIGITAL_PCM, trackPtr, trackSiz, data, size);
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
		byte volume = 255;
		Audio::AudioStream *input = Audio::makeRawStream(trackPtr[part], trackSiz[part],
														 rate, Audio::FLAG_UNSIGNED, DisposeAfterUse::NO);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &ch->handle, input, -1, volume);
	}
	return true;
}

extern void readStrings(Common::SeekableReadStream *stream);

void Sound::playMacMusic(const Common::String &filename) {
	if (filename.hasSuffixIgnoreCase(".sng")) {
		Common::String macFileName = filename.substr(0, filename.find(".")) + ".sx";
		playMacMusic(macFileName);
		return;
	}

	if (!filename.hasSuffixIgnoreCase(".sx"))
		error("Unhandled music file type: %s", filename.c_str());

	Common::SeekableReadStream *musicStream = _resource->getResource(filename);
	if (!musicStream)
		error("Music file %s not found", filename.c_str());

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

			debug("        %u [%u]:", type, count);
			for (uint16 k = 0; k < count; k++) {
				uint16 idx = stream->readUint16LE();
				debug("        %2u: %u", k, idx);
			}
		} else if (chunk.isSection(ID_TAG) || chunk.isSection(ID_FNM)) {
			readStrings(stream);
		} else if (chunk.isSection(ID_DAT)) {
			/*uint16 idx = */ stream->readUint16LE();
			/*uint16 type = */ stream->readUint16LE();
			_musicData = _decompressor->decompress(stream, stream->size() - stream->pos(), _musicSize);
		}
	}

	delete musicStream;

	// stopMusic();

	// TODO: This isn't correct
	if (_musicData) {
		uint32 tracks = availableSndTracks(_musicData, _musicSize);
		if (tracks & TRACK_MT32)
			_midiPlayer->play(_musicData, _musicSize);
		else if (tracks & DIGITAL_PCM)
			playPCM(_musicData, _musicSize);
	}
}

void Sound::playMusic(const Common::String &filename) {
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
			_musicSize = stream->size();
			_musicData = new byte[_musicSize];
			stream->read(_musicData, _musicSize);
		} else if (chunk.isSection(ID_INF)) {
			uint32 count = stream->size() / 2;
			//debug("        [%u]", count);
			for (uint32 k = 0; k < count; k++) {
				/*uint16 idx = */stream->readUint16LE();
				//debug("        %2u: %u", k, idx);
			}
		}
	}

	delete musicStream;

	// stopMusic();

	if (_musicData) {
		uint32 tracks = availableSndTracks(_musicData, _musicSize);
		if (tracks & TRACK_MT32)
			_midiPlayer->play(_musicData, _musicSize);
		else if (tracks & DIGITAL_PCM)
			playPCM(_musicData, _musicSize);
	}
}

static inline
void readHeader(const byte* &pos, uint32 &sci_header) {
	sci_header = 0;
	if (READ_LE_UINT16(pos) == 0x0084) sci_header = 2;

	pos += sci_header;
	if (pos[0] == 0xF0) {
		debug("SysEx transfer = %d bytes", pos[1]);
		pos += 2;
		pos += 6;
	}
}

static inline
void readPartHeader(const byte* &pos, uint16 &off, uint16 &siz) {
	pos += 2;
	off = READ_LE_UINT16(pos);
	pos += 2;
	siz = READ_LE_UINT16(pos);
	pos += 2;
}

static inline
void skipPartHeader(const byte* &pos) {
	pos += 6;
}

uint32 availableSndTracks(const byte *data, uint32 size) {
	const byte *pos = data;

	uint32 sci_header;
	readHeader(pos, sci_header);

	uint32 tracks = 0;
	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		//debug("(%d)", drv);

		while (pos[0] != 0xFF) {
			uint16 off, siz;
			readPartHeader(pos, off, siz);
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
				break;
			case 12:
				//debug("- MT-32");
				tracks |= TRACK_MT32;
				break;
			case 18:
				//debug("- PC Speaker");
				break;
			case 19:
				//debug("- Tandy 1000");
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

byte loadSndTrack(uint32 track, const byte** trackPtr, uint16* trackSiz, const byte *data, uint32 size) {
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
	readHeader(pos, sci_header);

	while (pos[0] != 0xFF) {
		byte drv = *pos++;

		byte part;
		const byte *ptr;

		part = 0;
		for (ptr = pos; *ptr != 0xFF; skipPartHeader(ptr))
			part++;

		if (matchDrv == drv) {
			part = 0;
			while (pos[0] != 0xFF) {
				uint16 off, siz;
				readPartHeader(pos, off, siz);
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

} // End of namespace Dgds

