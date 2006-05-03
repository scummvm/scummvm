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

#include "sound/adpcm.h"
#include "sound/audiostream.h"
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
	_offsets[res] = _file->size();
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
	if (!Audio::loadWAVFromStream(*_file, size, rate, wavFlags)) {
		error("playSound: Not a valid WAV file");
	}

	flags |= wavFlags;

	byte *buffer = (byte *)malloc(size);
	_file->read(buffer, size);
	_mixer->playRaw(handle, buffer, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE);
}

void VocSound::playSound(uint sound, Audio::SoundHandle *handle, byte flags) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	int size, rate;
	byte *buffer = Audio::loadVOCFromStream(*_file, size, rate);
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

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, Audio::makeMP3Stream(_file, size));
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

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, Audio::makeVorbisStream(_file, size));
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

	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, handle, Audio::makeFlacStream(_file, size));
}
#endif

Sound::Sound(SimonEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer)
	: _vm(vm), _mixer(mixer) {
	_voice = 0;
	_effects = 0;

	_effectsPaused = false;
	_ambientPaused = false;
	_sfx5Paused = false;

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

Sound::~Sound() {
	delete _voice;
	delete _effects;

	free(_filenums);
	free(_offsets);
}

void Sound::loadVoiceFile(const GameSpecificSettings *gss) {
	// Game versions which use separate voice files
	if (_vm->getGameType() == GType_FF || _vm->getGameId() == GID_SIMON1CD32)
		return;

	char filename[16];
	File *file = new File();

#ifdef USE_FLAC
	if (!_hasVoiceFile) {
		sprintf(filename, "%s.flac", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new FlacSound(_mixer, file);
		}
	}
#endif
#ifdef USE_MAD
	if (!_hasVoiceFile) {
		sprintf(filename, "%s.mp3", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_hasVoiceFile) {
		sprintf(filename, "%s.ogg", gss->speech_filename);
		file->open(filename);
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
			int end = file->size();
			_filenums = (uint16 *)malloc((end / 6 + 1) * 2);
			_offsets = (uint32 *)malloc((end / 6 + 1) * 4);

			for (int i = 1; i <= end / 6; i++) {
				_filenums[i] = file->readUint16BE();
				_offsets[i] = file->readUint32BE();
			}
			_hasVoiceFile = true;
		}
	}
	if (!_hasVoiceFile) {
		sprintf(filename, "%s.wav", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new WavSound(_mixer, file);
		}
	}
	if (!_hasVoiceFile) {
		sprintf(filename, "%s.voc", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VocSound(_mixer, file);
		}
	}
	if (!_hasVoiceFile) {
		sprintf(filename, "%s", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VocSound(_mixer, file);
		}
	}
}

void Sound::loadSfxFile(const GameSpecificSettings *gss) {
	char filename[16];
	File *file = new File();

#ifdef USE_MAD
	if (!_hasEffectsFile) {
		sprintf(filename, "%s.mp3", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_hasEffectsFile) {
		sprintf(filename, "%s.ogg", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VorbisSound(_mixer, file);
		}
	}
#endif
#ifdef USE_FLAC
	if (!_hasEffectsFile) {
		sprintf(filename, "%s.flac", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new FlacSound(_mixer, file);
		}
	}
#endif
	if (!_hasEffectsFile) {
		sprintf(filename, "%s.voc", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VocSound(_mixer, file);
		}
	}
	if (!_hasEffectsFile) {
		sprintf(filename, "%s", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VocSound(_mixer, file);
		}
	}
}

void Sound::readSfxFile(const char *filename) {
	if (_hasEffectsFile)
		return;

	stopAll();

	File *file = new File();
	file->open(filename);

	if (file->isOpen() == false) {
		if (atoi(filename + 6) != 1 && atoi(filename + 6) != 30)
			warning("readSfxFile: Can't load sfx file %s", filename);
		return;
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
		warning("readVoiceFile: Can't load voice file %s", filename);
		return;
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
	return _mixer->isSoundHandleActive(_voiceHandle);
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
	_sfx5Paused = b;
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
void Sound::playAmbientData(byte *soundData, uint sound, uint pan, uint vol) {
	if (sound == _ambientPlaying)
		return;

	_ambientPlaying = sound;

	if (_ambientPaused)
		return;

	_mixer->stopHandle(_ambientHandle);
	playSoundData(&_ambientHandle, soundData, sound, pan, vol, true);
}

void Sound::playSfxData(byte *soundData, uint sound, uint pan, uint vol) {
	if (_effectsPaused)
		return;

	playSoundData(&_effectsHandle, soundData, sound, pan, vol, false);
}

void Sound::playSfx5Data(byte *soundData, uint sound, uint pan, uint vol) {
	if (_sfx5Paused)
		return;

	_mixer->stopHandle(_sfx5Handle);
	playSoundData(&_sfx5Handle, soundData, sound, pan, vol, true);
}

void Sound::playVoiceData(byte *soundData, uint sound) {
	_mixer->stopHandle(_voiceHandle);
	playSoundData(&_voiceHandle, soundData, sound);
}

void Sound::playSoundData(Audio::SoundHandle *handle, byte *soundData, uint sound, int pan, int vol, bool loop) {
	byte *buffer, flags;
	uint16 compType;
	int blockAlign, rate;

	int size = READ_LE_UINT32(soundData + 4);
	Common::MemoryReadStream stream(soundData, size);
	if (!Audio::loadWAVFromStream(stream, size, rate, flags, &compType, &blockAlign)) {
		error("playSoundData: Not a valid WAV data");
	}

	// The Feeble Files originally used DirectSound, which specifies volume
	// and panning differently than ScummVM does, using a logarithmic scale
	// rather than a linear one.
	//
	// Volume is a value between -10,000 and 0.
	// Panning is a value between -10,000 and 10,000.
	//
	// In both cases, the -10,000 represents -100 dB. When panning, only
	// one speaker's volume is affected - just like in ScummVM - with
	// negative values affecting the left speaker, and positive values
	// affecting the right speaker. Thus -10,000 means the left speaker is
	// silent.

	int v, p;

	vol = CLIP(vol, -10000, 0);
	pan = CLIP(pan, -10000, 10000);

	if (vol) {
		v = (int)((double)Audio::Mixer::kMaxChannelVolume * pow(10.0, (double)vol / 2000.0) + 0.5);
	} else {
		v = Audio::Mixer::kMaxChannelVolume;
	}

	if (pan < 0) {
		p = (int)(255.0 * pow(10.0, (double)pan / 2000.0) + 127.5);
	} else if (pan > 0) {
		p = (int)(255.0 * pow(10.0, (double)pan / -2000.0) - 127.5);
	} else {
		p = 0;
	}

	if (loop == true)
		flags |= Audio::Mixer::FLAG_LOOP;
	
	if (compType == 2) {
		Audio::AudioStream *sndStream = Audio::makeADPCMStream(&stream, size, Audio::kADPCMMS, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign);
		buffer = (byte *)malloc(size * 4);
		size = sndStream->readBuffer((int16*)buffer, size * 2);
		size *= 2; // 16bits.
		delete sndStream;
	} else {
		buffer = (byte *)malloc(size);
		memcpy(buffer, soundData + stream.pos(), size);
	}

	_mixer->playRaw(handle, buffer, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE, -1, v, p);
}

void Sound::stopSfx5() {
	_mixer->stopHandle(_sfx5Handle);
}

void Sound::switchVoiceFile(const GameSpecificSettings *gss, uint disc) {
	if (_lastVoiceFile == disc)
		return;

	stopAll();
	delete _voice;

	_hasVoiceFile = false;
	_lastVoiceFile = disc;

	char filename[16];
	File *file = new File();

#ifdef USE_FLAC
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.flac", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new FlacSound(_mixer, file);
		}
	}
#endif
#ifdef USE_MAD
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.mp3", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new MP3Sound(_mixer, file);
		}
	}
#endif
#ifdef USE_VORBIS
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.ogg", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VorbisSound(_mixer, file);
		}
	}
#endif
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.wav", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen() == false) {
			warning("switchVoiceFile: Can't load voice file %s", filename);
			return;
		}
		_hasVoiceFile = true;
		_voice = new WavSound(_mixer, file);
	}
}

} // End of namespace Simon
