/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "stdafx.h"
#include "common/file.h"
#include "common/util.h"
#include "simon/sound.h"
#include "sound/voc.h"

namespace Simon {

#define SOUND_BIG_ENDIAN true

class BaseSound {
protected:
	File *_file;
	uint32 *_offsets;
	SoundMixer *_mixer;
	bool _freeOffsets;

public:
	BaseSound(SoundMixer *mixer, File *file, uint32 base = 0, bool bigendian = false);
	BaseSound(SoundMixer *mixer, File *file, uint32 *offsets, bool bigendian = false);
	virtual ~BaseSound();
	virtual void playSound(uint sound, PlayingSoundHandle *handle, byte flags) = 0;
};

class WavSound : public BaseSound {
public:
	WavSound(SoundMixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	WavSound(SoundMixer *mixer, File *file, uint32 *offsets) : BaseSound(mixer, file, offsets) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};

class VocSound : public BaseSound {
public:
	VocSound(SoundMixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};
class RawSound : public BaseSound {
public:
	RawSound(SoundMixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};

BaseSound::BaseSound(SoundMixer *mixer, File *file, uint32 base, bool bigendian) {
	_mixer = mixer;
	_file = file;

	uint res = 0;
	uint32 size;

	_file->seek(base + sizeof(uint32), SEEK_SET);
	if (bigendian)
		size = _file->readUint32BE();
	else
		size = _file->readUint32LE();

	res = size / sizeof(uint32);

	_offsets = (uint32 *)malloc(size + sizeof(uint32));
	_freeOffsets = true;

	_file->seek(base, SEEK_SET);

	if (_file->read(_offsets, size) != size)
		error("Can't read offsets");

	for (uint i = 0; i < res; i++) {
#if defined(SCUMM_BIG_ENDIAN)
	if (!(bigendian))
		_offsets[i] = FROM_LE_32(_offsets[i]);
#endif
	if (bigendian)
			_offsets[i] = TO_BE_32(_offsets[i]);
		_offsets[i] += base;
	}

	// only needed for mp3
	_file->seek(0, SEEK_END);
	_offsets[res] = _file->pos();
}

BaseSound::BaseSound(SoundMixer *mixer, File *file, uint32 *offsets, bool bigendian) {
	_mixer = mixer;
	_file = file;
	_offsets = offsets;
	_freeOffsets = false;
}

BaseSound::~BaseSound() {
	if (_freeOffsets)
		free(_offsets);
	delete _file;
}

#if !defined(__GNUC__)
#pragma START_PACK_STRUCTS
#endif

struct WaveHeader {
	uint32 riff;
	uint32 unk;
	uint32 wave;
	uint32 fmt;

	uint32 size;

	uint16 format_tag;
	uint16 channels;
	uint32 samples_per_sec;
	uint32 avg_bytes;

	uint16 block_align;
	uint16 bits_per_sample;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif
	
void WavSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	WaveHeader wave_hdr;
	uint32 data[2];

	flags |= SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	if (_file->read(&wave_hdr, sizeof(wave_hdr)) != sizeof(wave_hdr) ||
			wave_hdr.riff != MKID('RIFF') || wave_hdr.wave != MKID('WAVE')
			|| wave_hdr.fmt != MKID('fmt ') || READ_LE_UINT16(&wave_hdr.format_tag) != 1
			|| READ_LE_UINT16(&wave_hdr.channels) != 1
			|| READ_LE_UINT16(&wave_hdr.bits_per_sample) != 8) {
		error("playWav(%d): can't read RIFF header", sound);
	}

	_file->seek(FROM_LE_32(wave_hdr.size) - sizeof(wave_hdr) + 20, SEEK_CUR);

	data[0] = _file->readUint32LE();
	data[1] = _file->readUint32LE();
	if (data[0] != 'atad') {
		error("playWav(%d): can't read data header", sound);
	}

	byte *buffer = (byte *)malloc(data[1]);
	_file->read(buffer, data[1]);

	_mixer->playRaw(handle, buffer, data[1], FROM_LE_32(wave_hdr.samples_per_sec), flags);
}

void VocSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int size, samples_per_sec;
	byte *buffer = loadVOCFile(_file, size, samples_per_sec);

	_mixer->playRaw(handle, buffer, size, samples_per_sec, flags | SoundMixer::FLAG_AUTOFREE);
}

void RawSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	uint size = _file->readUint32BE();
	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	_mixer->playRaw(handle, buffer, size, 22050, flags | SoundMixer::FLAG_AUTOFREE);
}

#ifdef USE_MAD
class MP3Sound : public BaseSound {
public:
	MP3Sound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};

void MP3Sound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playMP3(handle, _file, size);
}
#endif

#ifdef USE_VORBIS
class VorbisSound : public BaseSound {
public:
	VorbisSound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};

void VorbisSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playVorbis(handle, _file, size);
}
#endif

#ifdef USE_FLAC
class FlacSound : public BaseSound {
public:
	FlacSound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, PlayingSoundHandle *handle, byte flags);
};

void FlacSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playFlac(handle, _file, size);
}
#endif

Sound::Sound(const byte game, const GameSpecificSettings *gss, const Common::String &gameDataPath, SoundMixer *mixer)
	: _game(game), _gameDataPath(gameDataPath), _mixer(mixer) {
	_voice = 0;
	_effects = 0;

	_effects_paused = false;
	_ambient_paused = false;

	_filenums = 0;
	_last_voice_file = 0;
	_offsets = 0;

	_voice_file = false;
	_ambient_playing = 0;

	File *file = new File();
	const char *s;

#ifdef USE_FLAC
	if (!_voice && gss->flac_filename && gss->flac_filename[0]) {
		file->open(gss->flac_filename, gameDataPath);
		if (file->isOpen()) {
			_voice_file = true;
			_voice = new FlacSound(_mixer, file);
		}
	}
#endif
#ifdef USE_MAD
	if (!_voice && gss->mp3_filename && gss->mp3_filename[0]) {
		file->open(gss->mp3_filename, gameDataPath);
		if (file->isOpen()) {
			_voice_file = true;
			_voice = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_voice && gss->vorbis_filename && gss->vorbis_filename[0]) {
		file->open(gss->vorbis_filename, gameDataPath);
		if (file->isOpen()) {
			_voice_file = true;
			_voice = new VorbisSound(_mixer, file);
		}
	}
#endif
	if (!_voice) {
		// for simon2 mac/amiga, only read index file
		if (_game == GAME_SIMON2MAC) {
			file->open("voices.idx", gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice index file 'voices.idx'");
			} else {
				file->seek(0, SEEK_END);
				int end = file->pos();
				file->seek(0, SEEK_SET);
				_filenums = (uint16 *)malloc((end / 6 + 1) * 2);
				_offsets = (uint32 *)malloc((end / 6 + 1) * 4);

				for (int i = 1; i <= end / 6; i++) {
					_filenums[i] = file->readUint16BE();
					_offsets[i] = file->readUint32BE();
				}
				_voice_file = true;
			}
			delete file;
		} else if (_game & GF_WIN) {
			s = gss->wav_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice file %s", s);
				delete file;
			} else	{
				_voice_file = true;
				_voice = new WavSound(_mixer, file);
			}
		} else if (_game == GAME_SIMON1CD32) {
			// simon1cd32 uses separate voice files
			return;
		} else if (_game & GF_TALKIE) {
			s = gss->voc_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice file %s", s);
				delete file;
			} else {
				_voice_file = true;
				_voice = new VocSound(_mixer, file);
			}
		}
	}

	if (_game == GAME_SIMON1ACORN || _game == GAME_SIMON1TALKIE) {
		file = new File();
#ifdef USE_MAD
		if (!_effects && gss->mp3_effects_filename && gss->mp3_effects_filename[0]) {
			file->open(gss->mp3_effects_filename, gameDataPath);
			if (file->isOpen()) {
				_effects = new MP3Sound(_mixer, file);
			}
		}
#endif
#ifdef USE_VORBIS
		if (!_effects && gss->vorbis_effects_filename && gss->vorbis_effects_filename[0]) {
			file->open(gss->vorbis_effects_filename, gameDataPath);
			if (file->isOpen()) {
				_effects = new VorbisSound(_mixer, file);
			}
		}
#endif
#ifdef USE_FLAC
		if (!_effects && gss->flac_effects_filename && gss->flac_effects_filename[0]) {
			file->open(gss->flac_effects_filename, gameDataPath);
			if (file->isOpen()) {
				_effects = new FlacSound(_mixer, file);
			}
		}
#endif
		if (!_effects) {
			s = gss->voc_effects_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open effects file %s", s);
			} else {
				_effects = new VocSound(_mixer, file);
			}
		}
	}
}

Sound::~Sound() {
	delete _voice;
	delete _effects;
	
	free(_filenums);
	free(_offsets);
}

void Sound::readSfxFile(const char *filename, const Common::String &gameDataPath) {
	stopAll();

	File *file = new File();
	file->open(filename, gameDataPath);

	if (file->isOpen() == false) {
		char *filename2;
		filename2 = (char *)malloc(strlen(filename) + 2);
		strcpy(filename2, filename);
		strcat(filename2, ".");
		file->open(filename2, gameDataPath);
		free(filename2);
		if (file->isOpen() == false) {
			if (atoi(filename + 6) != 1 && atoi(filename + 6) != 30)
				warning("readSfxFile: Can't load sfx file %s", filename);
			return;
		}
	}

	delete _effects;
	if (_game == GAME_SIMON1CD32) {
		_effects = new VocSound(_mixer, file, 0, SOUND_BIG_ENDIAN);
	} else
		_effects = new WavSound(_mixer, file);
}

void Sound::loadSfxTable(File *gameFile, uint32 base) {
	stopAll();

	if (_game & GF_WIN)
		_effects = new WavSound(_mixer, gameFile, base);
	else
		_effects = new VocSound(_mixer, gameFile, base);
}

void Sound::readVoiceFile(const char *filename, const Common::String &gameDataPath) {
	stopAll();

	File *file = new File();
	file->open(filename, gameDataPath);

	if (file->isOpen() == false) {
		char *filename2;
		filename2 = (char *)malloc(strlen(filename) + 2);
		strcpy(filename2, filename);
		strcat(filename2, ".");
		file->open(filename2, gameDataPath);
		free(filename2);
		if (file->isOpen() == false) {
			warning("readVoiceFile: Can't load voice file %s", filename);
			return;
		}
	}

	delete _voice;
	_voice = new RawSound(_mixer, file, 0, SOUND_BIG_ENDIAN);
}

void Sound::playVoice(uint sound) {
	if (_game == GAME_SIMON2MAC && _filenums) {
		if (_last_voice_file != _filenums[sound]) {
			stopAll();

			char filename[16];
			_last_voice_file = _filenums[sound];
			sprintf(filename, "voices%d.dat", _filenums[sound]);
			File *file = new File();
			file->open(filename, _gameDataPath);
			if (file->isOpen() == false) {
				warning("playVoice: Can't load voice file %s", filename);
				return;
			} 
			delete _voice;
			_voice = new WavSound(_mixer, file, _offsets);
		}
	}

	if (!_voice)
		return;

	_mixer->stopHandle(_voice_handle);
	_voice->playSound(sound, &_voice_handle, (_game == GAME_SIMON1CD32) ? 0 : SoundMixer::FLAG_UNSIGNED);
}

void Sound::playEffects(uint sound) {
	if (!_effects)
		return;
	
	if (_effects_paused)
		return;

	_effects->playSound(sound, &_effects_handle, (_game == GAME_SIMON1CD32) ? 0 : SoundMixer::FLAG_UNSIGNED);
}

void Sound::playAmbient(uint sound) {
	if (!_effects)
		return;

	if (sound == _ambient_playing)
		return;

	_ambient_playing = sound;

	if (_ambient_paused)
		return;

	_mixer->stopHandle(_ambient_handle);
	_effects->playSound(sound, &_ambient_handle, SoundMixer::FLAG_LOOP|SoundMixer::FLAG_UNSIGNED);
}

bool Sound::hasVoice() {
	return _voice_file;
}

void Sound::stopVoice() {
	_mixer->stopHandle(_voice_handle);
}

void Sound::stopAll() {
	_mixer->stopAll();
	_ambient_playing = 0;
}

void Sound::effectsPause(bool b) {
	_effects_paused = b;
}

void Sound::ambientPause(bool b) {
	_ambient_paused = b;

	if (_ambient_paused && _ambient_playing) {
		_mixer->stopHandle(_ambient_handle);
	} else if (_ambient_playing) {
		uint tmp = _ambient_playing;
		_ambient_playing = 0;
		playAmbient(tmp);
	}
}

} // End of namespace Simon
