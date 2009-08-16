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



#include "common/file.h"
#include "common/util.h"

#include "agos/agos.h"
#include "agos/sound.h"

#include "sound/adpcm.h"
#include "sound/audiostream.h"
#include "sound/flac.h"
#include "sound/mixer.h"
#include "sound/mp3.h"
#include "sound/voc.h"
#include "sound/vorbis.h"
#include "sound/wave.h"

using Common::File;

namespace AGOS {

#define SOUND_BIG_ENDIAN true

class BaseSound {
protected:
	File *_file;
	uint32 *_offsets;
	Audio::Mixer *_mixer;
	bool _freeOffsets;

public:
	BaseSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigEndian = false);
	BaseSound(Audio::Mixer *mixer, File *file, uint32 *offsets, bool bigEndian = false);
	virtual ~BaseSound();
	void close();

	void playSound(uint sound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0) {
		playSound(sound, sound, type, handle, flags, vol);
	}
	virtual void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0) = 0;
	virtual Audio::AudioStream *makeAudioStream(uint sound) { return NULL; }
};

class LoopingAudioStream : public Audio::AudioStream {
private:
	BaseSound *_parent;
	Audio::AudioStream *_stream;
	bool _loop;
	uint _sound;
	uint _loopSound;
public:
	LoopingAudioStream(BaseSound *parent, uint sound, uint loopSound, bool loop);
	~LoopingAudioStream();
	int readBuffer(int16 *buffer, const int numSamples);
	bool isStereo() const { return _stream ? _stream->isStereo() : 0; }
	bool endOfData() const;
	int getRate() const { return _stream ? _stream->getRate() : 22050; }
};

LoopingAudioStream::LoopingAudioStream(BaseSound *parent, uint sound, uint loopSound, bool loop) {
	_parent = parent;
	_sound = sound;
	_loop = loop;
	_loopSound = loopSound;

	_stream = _parent->makeAudioStream(sound);
}

LoopingAudioStream::~LoopingAudioStream() {
	delete _stream;
}

int LoopingAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	if (!_loop) {
		return _stream->readBuffer(buffer, numSamples);
	}

	int16 *buf = buffer;
	int samplesLeft = numSamples;

	while (samplesLeft > 0) {
		int len = _stream->readBuffer(buf, samplesLeft);
		if (len < samplesLeft) {
			delete _stream;
			_stream = _parent->makeAudioStream(_loopSound);
		}
		samplesLeft -= len;
		buf += len;
	}

	return numSamples;
}

bool LoopingAudioStream::endOfData() const {
	if (!_stream)
		return true;
	if (_loop)
		return false;
	return _stream->endOfData();
}

class WavSound : public BaseSound {
public:
	WavSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigEndian = false) : BaseSound(mixer, file, base, bigEndian) {}
	WavSound(Audio::Mixer *mixer, File *file, uint32 *offsets) : BaseSound(mixer, file, offsets) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

class VocSound : public BaseSound {
	byte _flags;
public:
	VocSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigEndian = false) : BaseSound(mixer, file, base, bigEndian), _flags(0) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

class RawSound : public BaseSound {
public:
	RawSound(Audio::Mixer *mixer, File *file, uint32 base = 0, bool bigEndian = false) : BaseSound(mixer, file, base, bigEndian) {}
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

BaseSound::BaseSound(Audio::Mixer *mixer, File *file, uint32 base, bool bigEndian) {
	_mixer = mixer;
	_file = file;

	uint res = 0;
	uint32 size;

	_file->seek(base + sizeof(uint32), SEEK_SET);
	if (bigEndian)
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

	for (uint i = 0; i < res; i++) {
		if (bigEndian)
			_offsets[i] = base + _file->readUint32BE();
		else
			_offsets[i] = base + _file->readUint32LE();
	}

	// only needed for mp3
	_offsets[res] = _file->size();
}

BaseSound::BaseSound(Audio::Mixer *mixer, File *file, uint32 *offsets, bool bigEndian) {
	_mixer = mixer;
	_file = file;
	_offsets = offsets;
	_freeOffsets = false;
}

void BaseSound::close() {
	if (_freeOffsets) {
		free(_offsets);
	}
}

BaseSound::~BaseSound() {
	if (_freeOffsets)
		free(_offsets);
	delete _file;
}

void convertVolume(int &vol) {
	// DirectSound was orginally used, which specifies volume
	// and panning differently than ScummVM does, using a logarithmic scale
	// rather than a linear one.
	//
	// Volume is a value between -10,000 and 0.
	//
	// In both cases, the -10,000 represents -100 dB. When panning, only
	// one speaker's volume is affected - just like in ScummVM - with
	// negative values affecting the left speaker, and positive values
	// affecting the right speaker. Thus -10,000 means the left speaker is
	// silent.

	int v = CLIP(vol, -10000, 0);
	if (v) {
		vol = (int)((double)Audio::Mixer::kMaxChannelVolume * pow(10.0, (double)v / 2000.0) + 0.5);
	} else {
		vol = Audio::Mixer::kMaxChannelVolume;
	}
}

void convertPan(int &pan) {
	// DirectSound was orginally used, which specifies volume
	// and panning differently than ScummVM does, using a logarithmic scale
	// rather than a linear one.
	//
	// Panning is a value between -10,000 and 10,000.
	//
	// In both cases, the -10,000 represents -100 dB. When panning, only
	// one speaker's volume is affected - just like in ScummVM - with
	// negative values affecting the left speaker, and positive values
	// affecting the right speaker. Thus -10,000 means the left speaker is
	// silent.

	int p = CLIP(pan, -10000, 10000);
	if (p < 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / 2000.0) + 127.5);
	} else if (p > 0) {
		pan = (int)(255.0 * pow(10.0, (double)p / -2000.0) - 127.5);
	} else {
		pan = 0;
	}
}

Audio::AudioStream *WavSound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);
	return Audio::makeWAVStream(_file, false);
}

void WavSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	convertVolume(vol);
	_mixer->playInputStream(type, handle, new LoopingAudioStream(this, sound, loopSound, (flags & Audio::Mixer::FLAG_LOOP) != 0), -1, vol);
}

Audio::AudioStream *VocSound::makeAudioStream(uint sound) {
	_file->seek(_offsets[sound], SEEK_SET);
	return Audio::makeVOCStream(*_file, _flags);
}

void VocSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	convertVolume(vol);
	_flags = flags;
	_mixer->playInputStream(type, handle, new LoopingAudioStream(this, sound, loopSound, (flags & Audio::Mixer::FLAG_LOOP) != 0), -1, vol);
}

void RawSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	if (_offsets == NULL)
		return;

	_file->seek(_offsets[sound], SEEK_SET);

	uint size = _file->readUint32BE();
	byte *buffer = (byte *)malloc(size);
	assert(buffer);
	_file->read(buffer, size);
	_mixer->playRaw(type, handle, buffer, size, 22050, flags | Audio::Mixer::FLAG_AUTOFREE);
}

#ifdef USE_MAD
class MP3Sound : public BaseSound {
public:
	MP3Sound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

Audio::AudioStream *MP3Sound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
		i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	Common::MemoryReadStream *tmp = _file->readStream(size);
	assert(tmp);
	return Audio::makeMP3Stream(tmp, true);
}

void MP3Sound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	convertVolume(vol);
	_mixer->playInputStream(type, handle, new LoopingAudioStream(this, sound, loopSound, (flags & Audio::Mixer::FLAG_LOOP) != 0), -1, vol);
}
#endif

#ifdef USE_VORBIS
class VorbisSound : public BaseSound {
public:
	VorbisSound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

Audio::AudioStream *VorbisSound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
		i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	Common::MemoryReadStream *tmp = _file->readStream(size);
	assert(tmp);
	return Audio::makeVorbisStream(tmp, true);
}

void VorbisSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	convertVolume(vol);
	_mixer->playInputStream(type, handle, new LoopingAudioStream(this, sound, loopSound, (flags & Audio::Mixer::FLAG_LOOP) != 0), -1, vol);
}
#endif

#ifdef USE_FLAC
class FlacSound : public BaseSound {
public:
	FlacSound(Audio::Mixer *mixer, File *file, uint32 base = 0) : BaseSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol = 0);
};

Audio::AudioStream *FlacSound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);

	int i = 1;
	while (_offsets[sound + i] == _offsets[sound])
		i++;

	uint32 size = _offsets[sound + i] - _offsets[sound];

	Common::MemoryReadStream *tmp = _file->readStream(size);
	assert(tmp);
	return Audio::makeFlacStream(tmp, true);
}

void FlacSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, byte flags, int vol) {
	convertVolume(vol);
	_mixer->playInputStream(type, handle, new LoopingAudioStream(this, sound, loopSound, (flags & Audio::Mixer::FLAG_LOOP) != 0), -1, vol);
}
#endif

Sound::Sound(AGOSEngine *vm, const GameSpecificSettings *gss, Audio::Mixer *mixer)
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

	_soundQueuePtr = 0;
	_soundQueueNum = 0;
	_soundQueueSize = 0;
	_soundQueueFreq = 0;

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
		sprintf(filename, "%s.fla", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new FlacSound(_mixer, file);
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
			if (_vm->getGameType() == GType_PP)
				_voice = new WavSound(_mixer, file);
			else
				_voice = new VocSound(_mixer, file);
		}
	}
}

void Sound::loadSfxFile(const GameSpecificSettings *gss) {
	char filename[16];
	File *file = new File();

#ifdef USE_FLAC
	if (!_hasEffectsFile) {
		sprintf(filename, "%s.fla", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new FlacSound(_mixer, file);
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

	_mixer->stopHandle(_effectsHandle);

	File *file = new File();
	file->open(filename);

	if (file->isOpen() == false) {
		error("readSfxFile: Can't load sfx file %s", filename);
	}

	delete _effects;
	if (_vm->getGameId() == GID_SIMON1CD32) {
		_effects = new VocSound(_mixer, file, 0, SOUND_BIG_ENDIAN);
	} else
		_effects = new WavSound(_mixer, file);
}

void Sound::loadSfxTable(File *gameFile, uint32 base) {
	stopAll();

	if (_effects)
		_effects->close();

	if (_vm->getPlatform() == Common::kPlatformWindows)
		_effects = new WavSound(_mixer, gameFile, base);
	else
		_effects = new VocSound(_mixer, gameFile, base);
}

void Sound::readVoiceFile(const char *filename) {
	_mixer->stopHandle(_voiceHandle);

	File *file = new File();
	file->open(filename);

	if (file->isOpen() == false)
		error("readVoiceFile: Can't load voice file %s", filename);

	delete _voice;
	_voice = new RawSound(_mixer, file, 0, SOUND_BIG_ENDIAN);
}

void Sound::playVoice(uint sound) {
	if (_filenums) {
		if (_lastVoiceFile != _filenums[sound]) {
			_mixer->stopHandle(_voiceHandle);

			char filename[16];
			_lastVoiceFile = _filenums[sound];
			sprintf(filename, "voices%d.dat", _filenums[sound]);
			File *file = new File();
			file->open(filename);
			if (file->isOpen() == false)
				error("playVoice: Can't load voice file %s", filename);

			delete _voice;
			_voice = new WavSound(_mixer, file, _offsets);
		}
	}

	if (!_voice)
		return;

	_mixer->stopHandle(_voiceHandle);
	if (_vm->getGameType() == GType_PP) {
		if (sound < 11)
			_voice->playSound(sound, sound + 1, Audio::Mixer::kMusicSoundType, &_voiceHandle, Audio::Mixer::FLAG_LOOP, -1500);
		else
			_voice->playSound(sound, sound, Audio::Mixer::kMusicSoundType, &_voiceHandle, Audio::Mixer::FLAG_LOOP);
	} else if (_vm->getGameType() == GType_FF || _vm->getGameId() == GID_SIMON1CD32) {
		_voice->playSound(sound, Audio::Mixer::kSpeechSoundType, &_voiceHandle, 0);
	} else {
		_voice->playSound(sound, Audio::Mixer::kSpeechSoundType, &_voiceHandle, Audio::Mixer::FLAG_UNSIGNED);
	}
}

void Sound::playEffects(uint sound) {
	if (!_effects)
		return;

	if (_effectsPaused)
		return;

	if (_vm->getGameType() == GType_SIMON1)
		_mixer->stopHandle(_effectsHandle);
	_effects->playSound(sound, Audio::Mixer::kSFXSoundType, &_effectsHandle, (_vm->getGameId() == GID_SIMON1CD32) ? 0 : Audio::Mixer::FLAG_UNSIGNED);
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
	_effects->playSound(sound, Audio::Mixer::kSFXSoundType, &_ambientHandle, Audio::Mixer::FLAG_LOOP | Audio::Mixer::FLAG_UNSIGNED);
}

bool Sound::hasVoice() const {
	return _hasVoiceFile;
}

bool Sound::isSfxActive() const {
	return _mixer->isSoundHandleActive(_effectsHandle);
}

bool Sound::isVoiceActive() const {
	return _mixer->isSoundHandleActive(_voiceHandle);
}

void Sound::stopAllSfx() {
	_mixer->stopHandle(_ambientHandle);
	_mixer->stopHandle(_effectsHandle);
	_mixer->stopHandle(_sfx5Handle);
	_ambientPlaying = 0;
}

void Sound::stopSfx() {
	_mixer->stopHandle(_effectsHandle);
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

// Personal Nightmare specific
void Sound::handleSoundQueue() {
	if (isSfxActive())
		return;

	_vm->_sampleEnd = 1;

	if (_soundQueuePtr) {
		playRawData(_soundQueuePtr, _soundQueueNum, _soundQueueSize, _soundQueueFreq);

		_vm->_sampleWait = 1;
		_vm->_sampleEnd = 0;
		_soundQueuePtr = 0;
		_soundQueueNum = 0;
		_soundQueueSize = 0;
		_soundQueueFreq = 0;
	}
}

void Sound::queueSound(byte *ptr, uint16 sound, uint32 size, uint16 freq) {
	if (_effectsPaused)
		return;

	// Only a single sound can be queued
	_soundQueuePtr = ptr;
	_soundQueueNum = sound;
	_soundQueueSize = size;
	_soundQueueFreq = freq;
}

// Elvira 1/2 and Waxworks specific
void Sound::playRawData(byte *soundData, uint sound, uint size, uint freq) {
	if (_effectsPaused)
		return;

	byte *buffer = (byte *)malloc(size);
	memcpy(buffer, soundData, size);

	if (_vm->getPlatform() == Common::kPlatformPC)
		_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_effectsHandle, buffer, size, freq, Audio::Mixer::FLAG_UNSIGNED | Audio::Mixer::FLAG_AUTOFREE);
	else
		_mixer->playRaw(Audio::Mixer::kSFXSoundType, &_effectsHandle, buffer, size, freq, Audio::Mixer::FLAG_AUTOFREE);
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

	// TODO: Use makeWAVStream() in future, when makeADPCMStream() allows sound looping
	int size = READ_LE_UINT32(soundData + 4);
	Common::MemoryReadStream stream(soundData, size);
	if (!Audio::loadWAVFromStream(stream, size, rate, flags, &compType, &blockAlign))
		error("playSoundData: Not a valid WAV data");

	convertVolume(vol);
	convertPan(pan);

	if (loop == true)
		flags |= Audio::Mixer::FLAG_LOOP;

	if (compType == 2) {
		Audio::AudioStream *sndStream = Audio::makeADPCMStream(&stream, false, size, Audio::kADPCMMS, rate, (flags & Audio::Mixer::FLAG_STEREO) ? 2 : 1, blockAlign);
		buffer = (byte *)malloc(size * 4);
		size = sndStream->readBuffer((int16*)buffer, size * 2);
		size *= 2; // 16bits.
		delete sndStream;
	} else {
		buffer = (byte *)malloc(size);
		memcpy(buffer, soundData + stream.pos(), size);
	}

	_mixer->playRaw(Audio::Mixer::kSFXSoundType, handle, buffer, size, rate, flags | Audio::Mixer::FLAG_AUTOFREE, -1, vol, pan);
}

void Sound::stopSfx5() {
	_mixer->stopHandle(_sfx5Handle);
}

void Sound::switchVoiceFile(const GameSpecificSettings *gss, uint disc) {
	if (_lastVoiceFile == disc)
		return;

	_mixer->stopHandle(_voiceHandle);
	delete _voice;

	_hasVoiceFile = false;
	_lastVoiceFile = disc;

	char filename[16];
	File *file = new File();

#ifdef USE_FLAC
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.fla", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new FlacSound(_mixer, file);
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
	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d.wav", gss->speech_filename, disc);
		file->open(filename);
		if (file->isOpen() == false) {
			error("switchVoiceFile: Can't load voice file %s", filename);
		}
		_hasVoiceFile = true;
		_voice = new WavSound(_mixer, file);
	}
}

} // End of namespace AGOS
