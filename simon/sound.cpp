/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "simon/sound.h"
#include "common/file.h"
#include "common/engine.h"

class BaseSound {
protected:
	File *_file;
	uint32 *_offsets;
	SoundMixer *_mixer;

public:
	BaseSound(SoundMixer *mixer, File *file, uint32 base = 0);
	BaseSound(SoundMixer *mixer, File *file, uint32 *offsets);
	virtual ~BaseSound();
	virtual int playSound(uint sound, PlayingSoundHandle *handle, byte flags = 0) = 0;
};

class WavSound : public BaseSound {
public:
	WavSound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	WavSound(SoundMixer *mixer, File *file, uint32 *offsets) : BaseSound(mixer, file, offsets) {};
	int playSound(uint sound, PlayingSoundHandle *handle, byte flags = 0);
};

class VocSound : public BaseSound {
public:
	VocSound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	int playSound(uint sound, PlayingSoundHandle *handle, byte flags = 0);
};

BaseSound::BaseSound(SoundMixer *mixer, File *file, uint32 base) {
	_mixer = mixer;
	_file = file;

	uint res = 0;
	uint32 size;

	_file->seek(base + sizeof(uint32), SEEK_SET);
	size = _file->readUint32LE();

	res = size / sizeof(uint32);

	_offsets = (uint32 *)malloc(size + sizeof(uint32));

	_file->seek(base, SEEK_SET);

	if (_file->read(_offsets, size) != size)
		error("Can't read offsets");

	for (uint i = 0; i < res; i++) {
#if defined(SCUMM_BIG_ENDIAN)
		_offsets[i] = FROM_LE_32(_offsets[i]);
#endif
		_offsets[i] += base;
	}

	// only needed for mp3
	_file->seek(0, SEEK_END);
	_offsets[res] = _file->pos();
}

BaseSound::BaseSound(SoundMixer *mixer, File *file, uint32 *offsets) {
	_mixer = mixer;
	_file = file;
	_offsets = offsets;
}

BaseSound::~BaseSound() {
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

struct VocHeader {
	uint8 desc[20];
	uint16 datablock_offset;
	uint16 version;
	uint16 id;
} GCC_PACK;

struct VocBlockHeader {
	uint8 blocktype;
	uint8 size[3];
	uint8 sr;
	uint8 pack;
} GCC_PACK;

#if !defined(__GNUC__)
#pragma END_PACK_STRUCTS
#endif
	
int WavSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return 0;

	WaveHeader wave_hdr;
	uint32 data[2];

	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;

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
	if (//fread(data, sizeof(data), 1, sound_file) != 1 ||
			 data[0] != 'atad') {
		error("playWav(%d): can't read data header", sound);
	}

	byte *buffer = (byte *)malloc(data[1]);
	_file->read(buffer, data[1]);

	return _mixer->playRaw(handle, buffer, data[1], FROM_LE_32(wave_hdr.samples_per_sec), flags);
}

int VocSound::playSound(uint sound, PlayingSoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return 0;

	VocHeader voc_hdr;
	VocBlockHeader voc_block_hdr;
	uint32 size;

	flags |= SoundMixer::FLAG_UNSIGNED|SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	if (_file->read(&voc_hdr, sizeof(voc_hdr)) != sizeof(voc_hdr) ||
			strncmp((char *)voc_hdr.desc, "Creative Voice File\x1A", 10) != 0) {
		error("playVoc(%d): can't read voc header", sound);
	}

	_file->read(&voc_block_hdr, sizeof(voc_block_hdr));

	size = voc_block_hdr.size[0] + (voc_block_hdr.size[1] << 8) + (voc_block_hdr.size[2] << 16) - 2;
	uint32 samples_per_sec;

	/* workaround for voc weakness */
	if (voc_block_hdr.sr == 0xa6) {
		samples_per_sec = 11025;
	} else if (voc_block_hdr.sr == 0xd2) {
		samples_per_sec = 22050;
	} else {
		samples_per_sec = 1000000L / (256L - (long)voc_block_hdr.sr);
		warning("inexact sample rate used: %i", samples_per_sec);
	}

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	return _mixer->playRaw(handle, buffer, size, samples_per_sec, flags);
}

#ifdef USE_MAD
class MP3Sound : public BaseSound {
public:
	MP3Sound(SoundMixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	int playSound(uint sound, PlayingSoundHandle *handle, byte flags = 0);
};

int MP3Sound::playSound(uint sound, PlayingSoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return 0;

	flags |= SoundMixer::FLAG_AUTOFREE;

	_file->seek(_offsets[sound], SEEK_SET);

	uint32 size = _offsets[sound+1] - _offsets[sound];

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	return _mixer->playMP3(handle, buffer, size, flags);
}
#endif


/******************************************************************************/


SimonSound::SimonSound(const byte game, const GameSpecificSettings *gss, const char *gameDataPath, SoundMixer *mixer) {
	_game = game;
	_gameDataPath = gameDataPath;
	_mixer = mixer;
	
	_voice_index = 0;
	_ambient_index = 0;

	_voice = 0;
	_effects = 0;

	_effects_paused = false;
	_ambient_paused = false;

	_filenums = 0;
	_offsets = 0;

	_voice_handle = 0;
	_effects_handle = 0;
	_ambient_handle = 0;

	_voice_file = false;
	_ambient_playing = 0;

	File *file = new File();
	const char *s;

#ifdef USE_MAD
	file->open(gss->mp3_filename, gameDataPath);
	if (file->isOpen() == false) {
#endif
		// for simon2 mac/amiga, only read index file
		if (_game == GAME_SIMON2MAC) {
			file->open("voices.idx", gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice index file 'voices.idx'");
				delete file;
			} else {
				file->seek(0, SEEK_END);
				int end = file->pos();
				file->seek(0, SEEK_SET);
				_filenums = (uint16 *)malloc(end / 3 + 1);
				_offsets = (uint32 *)malloc((end / 6) * 4 + 1);

				for (int i = 1; i <= end / 6; i++) {
					_filenums[i] = file->readUint16BE();
					_offsets[i] = file->readUint32BE();
				}
				_voice_file = true;
			}
		} else if (_game & GF_WIN) {
			s = gss->wav_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice file %s", s);
			} else	{
				_voice_file = true;
				_voice = new WavSound(_mixer, file);
			}
		} else if (_game & GF_TALKIE) {
			s = gss->voc_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open voice file %s", s);
			} else {
				_voice_file = true;
				_voice = new VocSound(_mixer, file);
			}
		}
#ifdef USE_MAD
	} else {
		_voice_file = true;
		_voice = new MP3Sound(_mixer, file);
	}
#endif

	if (_game == GAME_SIMON1TALKIE) {
		file = new File();
#ifdef USE_MAD
		file->open(gss->mp3_effects_filename, gameDataPath);
		if (file->isOpen() == false) {
#endif
			s = gss->voc_effects_filename;
			file->open(s, gameDataPath);
			if (file->isOpen() == false) {
				warning("Can't open effects file %s", s);
			} else {
				_effects = new VocSound(_mixer, file);
			}
#ifdef USE_MAD
		} else {
			_effects = new MP3Sound(_mixer, file);
		}
#endif
	}
}

SimonSound::~SimonSound() {
	delete _voice;
	delete _effects;
	
	free(_filenums);
	free(_offsets);
}

void SimonSound::readSfxFile(const char *filename, const char *gameDataPath) {
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

	_effects = new WavSound(_mixer, file);
}

void SimonSound::loadSfxTable(File *gameFile, uint32 base) {
	stopAll();

	if (_game & GF_WIN)
		_effects = new WavSound(_mixer, gameFile, base);
	else
		_effects = new VocSound(_mixer, gameFile, base);
}

void SimonSound::playVoice(uint sound) {
	if (_game == GAME_SIMON2MAC && _filenums) {
		char filename[16];
		sprintf(filename, "voices%d.dat", _filenums[sound]);
		File *file = new File();
		file->open(filename, _gameDataPath);
		if (file->isOpen() == false) {
			warning("Can't open voice file %s", filename);
		} else {
			delete _voice;
			_voice = new WavSound(_mixer, file, _offsets);
		}
	}

	if (!_voice)
		return;
	
	_voice_index = _voice->playSound(sound, &_voice_handle);
}

void SimonSound::playEffects(uint sound) {
	if (!_effects)
		return;
	
	if (_effects_paused)
		return;

	_effects->playSound(sound, &_effects_handle);
}

void SimonSound::playAmbient(uint sound) {
	if (!_effects)
		return;

	if (sound == _ambient_playing)
		return;

	_ambient_playing = sound;

	if (_ambient_paused)
		return;

	if (_ambient_handle)
		_mixer->stop(_ambient_index);

	_ambient_index = _effects->playSound(sound, &_ambient_handle, SoundMixer::FLAG_LOOP);
}

bool SimonSound::hasVoice() {
	return _voice_file;
}

void SimonSound::stopVoice() {
	_mixer->stop(_voice_index);
}

void SimonSound::stopAll() {
	_mixer->stopAll();
	_ambient_playing = 0;
}

void SimonSound::effectsPause(bool b) {
	_effects_paused = b;
}

void SimonSound::ambientPause(bool b) {
	_ambient_paused = b;

	if (_ambient_paused && _ambient_playing) {
		_mixer->stop(_ambient_index);
	} else if (_ambient_playing) {
		uint tmp = _ambient_playing;
		_ambient_playing = 0;
		playAmbient(tmp);
	}
}
