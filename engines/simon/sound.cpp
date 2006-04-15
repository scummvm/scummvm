/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "common/stdafx.h"

#include "common/file.h"
#include "common/util.h"

#include "simon/simon.h"
#include "simon/sound.h"

#include "sound/flac.h"
#include "sound/mp3.h"
#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

using Common::File;

namespace Simon {

#define SOUND_BIG_ENDIAN true

class BaseSound {
protected:
	File *_file;
	uint32 *_offsets;
	Audio::Mixer *_mixer;
	bool _freeOffsets;

public:
	BaseSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigendian = false);
	BaseSound(Audio::Mixer *mixer, File *file, uint32 *offsets, bool bigendian = false);
	virtual ~BaseSound();
	virtual void playSound(uint sound, Audio::SoundHandle *handle, byte flags) = 0;
};

class WavSound : public BaseSound {
public:
	WavSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	WavSound(Audio::Mixer *mixer, File *file, uint32 *offsets) : BaseSound(mixer, file, offsets) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};

class VocSound : public BaseSound {
public:
	VocSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};
class RawSound : public BaseSound {
public:
	RawSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigendian = false) : BaseSound(mixer, file, base, bigendian) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};

BaseSound::BaseSound(Audio::Mixer *mixer, File *file, uint32 base, bool bigendian) {
	_mixer = mixer;
	_file = file;

	uint res = 0;
	uint32 size;

	_file->seek(base + sizeof(uint32), SEEK_SET);
	if (bigendian)
		size = _file->readUint32BE();
	else
		size = _file->readUint32LE();

	// The Feeble Files uses set amount of voice offsets
	if (size == 0)
		size = 40000;

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

BaseSound::BaseSound(Audio::Mixer *mixer, File *file, uint32 *offsets, bool bigendian) {
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

void WavSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	byte wavFlags;
	int size, rate;
	if (!loadWAVFromStream(*_file, size, rate, wavFlags)) {
		error("playSound: Not a valid WAV file");
	}

	flags |= Audio::Mixer::FLAG_AUTOFREE | wavFlags;

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);
	_mixer->playRaw(handle, buffer, size, rate, flags);
}

void VocSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int size, rate;
	byte *buffer = loadVOCFromStream(*_file, size, rate);

	_mixer->playRaw(handle, buffer, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE);
}

void RawSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	uint size = _file->readUint32BE();
	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);

	_mixer->playRaw(handle, buffer, size, 22050, flags | Audio::Mixer::FLAG_AUTOFREE);
}

#ifdef USE_MAD
class MP3Sound : public BaseSound {
public:
	MP3Sound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};

void MP3Sound::playSound(uint sound, Audio::SoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, makeMP3Stream(_file, size));
}
#endif

#ifdef USE_VORBIS
class VorbisSound : public BaseSound {
public:
	VorbisSound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};

void VorbisSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, makeVorbisStream(_file, size));
}
#endif

#ifdef USE_FLAC
class FlacSound : public BaseSound {
public:
	FlacSound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {};
	void playSound(uint sound, Audio::SoundHandle *handle, byte flags);
};

void FlacSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags)
{
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
			i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, makeFlacStream(_file, size));
}
#endif

Sound::Sound(SimonEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer) {
	_voice = 0;
	_effects = 0;

	_effectsPaused = false;
	_ambientPaused = false;

	_filenums = 0;
	_lastVoiceFile = 0;
	_offsets = 0;

	_hasEffectsFile = false;
	_hasVoiceFile = false;
	_ambientPlaying = 0;

	if (_vm->getFeatures() & GF_TALKIE) {
		loadVoiceFile(gss);

		if (_vm->getGameType() == GType_SIMON1)
			loadSfxFile(gss);
	}

}

void Sound::loadVoiceFile(const GameSpecificSettings *gss) {
	// Game versions which use separate voice files
	if (_vm->getGameType() == GType_FF || _vm->getGameId() == GID_SIMON1CD32)
		return;

	File *file = new File();

#ifdef USE_FLAC
	if (!_hasVoiceFile && gss->flac_filename && gss->flac_filename[0]) {
		file->open(gss->flac_filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new FlacSound(_mixer, file);
		}
	}
#endif
#ifdef USE_MAD
	if (!_hasVoiceFile && gss->mp3_filename && gss->mp3_filename[0]) {
		file->open(gss->mp3_filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_hasVoiceFile && gss->vorbis_filename && gss->vorbis_filename[0]) {
		file->open(gss->vorbis_filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VorbisSound(_mixer, file);
		}
	}
#endif
	if (!_hasVoiceFile && _vm->getGameType() == GType_SIMON2) {
		// for simon2 mac/amiga, only read index file
		file->open("voices.idx");
		if (file->isOpen() == true) {
			file->seek(0, SEEK_END);
			int end = file->pos();
			file->seek(0, SEEK_SET);
			_filenums = (uint16 *)malloc((end / 6 + 1) * 2);
			_offsets = (uint32 *)malloc((end / 6 + 1) * 4);

			for (int i = 1; i <= end / 6; i++) {
				_filenums[i] = file->readUint16BE();
				_offsets[i] = file->readUint32BE();
			}
			_hasVoiceFile = true;
		}
	}
	if (!_hasVoiceFile && gss->wav_filename && gss->wav_filename[0]) {
		file->open(gss->wav_filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new WavSound(_mixer, file);
		}
	}
	if (!_hasVoiceFile && gss->voc_filename && gss->voc_filename[0]) {
		file->open(gss->voc_filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VocSound(_mixer, file);
		}
	}
}

void Sound::loadSfxFile(const GameSpecificSettings *gss) {
	File *file = new File();

#ifdef USE_MAD
	if (!_hasEffectsFile && gss->mp3_effects_filename && gss->mp3_effects_filename[0]) {
		file->open(gss->mp3_effects_filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_hasEffectsFile && gss->vorbis_effects_filename && gss->vorbis_effects_filename[0]) {
		file->open(gss->vorbis_effects_filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VorbisSound(_mixer, file);
		}
	}
#endif
#ifdef USE_FLAC
	if (!_hasEffectsFile && gss->flac_effects_filename && gss->flac_effects_filename[0]) {
		file->open(gss->flac_effects_filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new FlacSound(_mixer, file);
		}
	}
#endif
	if (!_hasEffectsFile && gss->voc_effects_filename && gss->voc_effects_filename[0]) {
		file->open(gss->voc_effects_filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VocSound(_mixer, file);
		}
	}
}

Sound::~Sound() {
	delete _voice;
	delete _effects;

	free(_filenums);
	free(_offsets);
}

void Sound::readSfxFile(const char *filename) {
	if (_hasEffectsFile)
		return;

	stopAll();

	File *file = new File();
	file->open(filename);

	if (file->isOpen() == false) {
		char *filename2;
		filename2 = (char *)malloc(strlen(filename) + 2);
		strcpy(filename2, filename);
		strcat(filename2, ".");
		file->open(filename2);
		free(filename2);
		if (file->isOpen() == false) {
			if (atoi(filename + 6) != 1 && atoi(filename + 6) != 30)
				warning("readSfxFile: Can't load sfx file %s", filename);
			return;
		}
	}

	delete _effects;
	if (_vm->getGameId() == GID_SIMON1CD32) {
		_effects = new VocSound(_mixer, file, 0, SOUND_BIG_ENDIAN);
	} else
		_effects = new WavSound(_mixer, file);
}

void Sound::loadSfxTable(File *gameFile, uint32 base) {
	stopAll();

	if (_vm->getPlatform() == Common::kPlatformWindows)
		_effects = new WavSound(_mixer, gameFile, base);
	else
		_effects = new VocSound(_mixer, gameFile, base);
}

void Sound::readVoiceFile(const char *filename) {
	stopAll();

	File *file = new File();
	file->open(filename);

	if (file->isOpen() == false) {
		char *filename2;
		filename2 = (char *)malloc(strlen(filename) + 2);
		strcpy(filename2, filename);
		strcat(filename2, ".");
		file->open(filename2);
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
	if (_filenums) {
		if (_lastVoiceFile != _filenums[sound]) {
			stopAll();

			char filename[16];
			_lastVoiceFile = _filenums[sound];
			sprintf(filename, "voices%d.dat", _filenums[sound]);
			File *file = new File();
			file->open(filename);
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

	_mixer->stopHandle(_voiceHandle);
	if (_vm->getGameType() == GType_FF || _vm->getGameId() == GID_SIMON1CD32) {
		_voice->playSound(sound, &_voiceHandle, 0);
	} else {
		_voice->playSound(sound, &_voiceHandle, Audio::Mixer::FLAG_UNSIGNED);
	}
}

void Sound::playEffects(uint sound) {
	if (!_effects)
		return;

	if (_effectsPaused)
		return;

	_effects->playSound(sound, &_effectsHandle, (_vm->getGameId() == GID_SIMON1CD32) ? 0 : Audio::Mixer::FLAG_UNSIGNED);
}

void Sound::playAmbient(uint sound) {
	if (!_effects)
		return;

	if (sound == _ambientPlaying)
		return;

	_ambientPlaying = sound;

	if (_ambientPaused)
		return;

	_mixer->stopHandle(_ambientHandle);
	_effects->playSound(sound, &_ambientHandle, Audio::Mixer::FLAG_LOOP|Audio::Mixer::FLAG_UNSIGNED);
}

bool Sound::hasVoice() const {
	return _hasVoiceFile;
}

bool Sound::isVoiceActive() const {
	return _mixer->isSoundHandleActive(_voiceHandle) ;
}

void Sound::stopVoice() {
	_mixer->stopHandle(_voiceHandle);
}

void Sound::stopAll() {
	_mixer->stopAll();
	_ambientPlaying = 0;
}

void Sound::effectsPause(bool b) {
	_effectsPaused = b;
}

void Sound::ambientPause(bool b) {
	_ambientPaused = b;

	if (_ambientPaused && _ambientPlaying) {
		_mixer->stopHandle(_ambientHandle);
	} else if (_ambientPlaying) {
		uint tmp = _ambientPlaying;
		_ambientPlaying = 0;
		playAmbient(tmp);
	}
}

// Feeble Files specific
void Sound::playSoundData(byte *soundData, uint sound, uint pan, uint vol, bool ambient) {
	byte flags;
	int rate;

	if (ambient == true) {
		if (sound == _ambientPlaying)
			return;

		_ambientPlaying = sound;

		if (_ambientPaused)
			return;
	} else {
		if (_effectsPaused)
			return;
	}

	int size = READ_LE_UINT32(soundData + 4);
	Common::MemoryReadStream stream(soundData, size);
	if (!loadWAVFromStream(stream, size, rate, flags)) {
		error("playSoundData: Not a valid WAV data");
	}

	byte *buffer = (byte *)malloc(size);
	memcpy(buffer, soundData + stream.pos(), size);

	if (ambient == true) {
		_mixer->stopHandle(_ambientHandle);
		_mixer->playRaw(&_ambientHandle, buffer, size, rate, Audio::Mixer::FLAG_LOOP|flags);
	} else {
		_mixer->playRaw(&_effectsHandle, buffer, size, rate, flags);
	}
}

void Sound::playVoiceData(byte *soundData, uint sound) {
	byte flags;
	int rate;

	int size = READ_LE_UINT32(soundData + 4);
	Common::MemoryReadStream stream(soundData, size);
	if (!loadWAVFromStream(stream, size, rate, flags)) {
		error("playSoundData: Not a valid WAV data");
	}

	byte *buffer = (byte *)malloc(size);
	memcpy(buffer, soundData + stream.pos(), size);

	_mixer->stopHandle(_voiceHandle);
	_mixer->playRaw(&_voiceHandle, buffer, size, rate, flags);
}

void Sound::switchVoiceFile(uint disc) {
	if (_lastVoiceFile != disc) {
		stopAll();

		char filename[16];
		_lastVoiceFile = disc;
		sprintf(filename, "voices%d.wav",disc);
		File *file = new File();
		file->open(filename);
		if (file->isOpen() == false) {
			warning("playVoice: Can't load voice file %s", filename);
			return;
		}
		delete _voice;
		_voice = new WavSound(_mixer, file);
	}
}

} // End of namespace Simon
