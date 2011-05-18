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
 */

#include "common/file.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "agos/agos.h"
#include "agos/sound.h"

#include "audio/audiostream.h"
#include "audio/decoders/flac.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/wave.h"

namespace AGOS {

#define SOUND_BIG_ENDIAN true

class BaseSound : Common::NonCopyable {
protected:
	Common::File *_file;
	uint32 *_offsets;
	Audio::Mixer *_mixer;
	bool _freeOffsets;
	DisposeAfterUse::Flag _disposeFile;

public:
	BaseSound(Audio::Mixer *mixer, Common::File *file, uint32 base, bool bigEndian, DisposeAfterUse::Flag disposeFileAfterUse = DisposeAfterUse::YES);
	BaseSound(Audio::Mixer *mixer, Common::File *file, uint32 *offsets, DisposeAfterUse::Flag disposeFileAfterUse = DisposeAfterUse::YES);
	virtual ~BaseSound();

	void playSound(uint sound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0) {
		playSound(sound, sound, type, handle, loop, vol);
	}
	virtual void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0) = 0;
	virtual Audio::AudioStream *makeAudioStream(uint sound) = 0;
};

BaseSound::BaseSound(Audio::Mixer *mixer, Common::File *file, uint32 base, bool bigEndian, DisposeAfterUse::Flag disposeFileAfterUse)
	: _mixer(mixer), _file(file), _disposeFile(disposeFileAfterUse) {

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

BaseSound::BaseSound(Audio::Mixer *mixer, Common::File *file, uint32 *offsets, DisposeAfterUse::Flag disposeFileAfterUse)
	: _mixer(mixer), _file(file), _disposeFile(disposeFileAfterUse) {

	_offsets = offsets;
	_freeOffsets = false;
}

BaseSound::~BaseSound() {
	if (_freeOffsets)
		free(_offsets);
	if (_disposeFile == DisposeAfterUse::YES)
		delete _file;
}

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

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

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

static void convertVolume(int &vol) {
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

static void convertPan(int &pan) {
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

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

class WavSound : public BaseSound {
public:
	WavSound(Audio::Mixer *mixer, Common::File *file, uint32 base = 0, DisposeAfterUse::Flag disposeFileAfterUse = DisposeAfterUse::YES)
		: BaseSound(mixer, file, base, false, disposeFileAfterUse) {}
	WavSound(Audio::Mixer *mixer, Common::File *file, uint32 *offsets) : BaseSound(mixer, file, offsets) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0);
};

Audio::AudioStream *WavSound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);
	return Audio::makeWAVStream(_file, DisposeAfterUse::NO);
}

void WavSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol) {
	convertVolume(vol);
	_mixer->playStream(type, handle, new LoopingAudioStream(this, sound, loopSound, loop), -1, vol);
}

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

class VocSound : public BaseSound {
	const byte _flags;
public:
	VocSound(Audio::Mixer *mixer, Common::File *file, bool isUnsigned, uint32 base = 0, bool bigEndian = false, DisposeAfterUse::Flag disposeFileAfterUse = DisposeAfterUse::YES)
		: BaseSound(mixer, file, base, bigEndian, disposeFileAfterUse), _flags(isUnsigned ? Audio::FLAG_UNSIGNED : 0) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0);
};

Audio::AudioStream *VocSound::makeAudioStream(uint sound) {
	assert(_offsets);
	_file->seek(_offsets[sound], SEEK_SET);
	return Audio::makeVOCStream(_file, _flags);
}

void VocSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol) {
	convertVolume(vol);
	_mixer->playStream(type, handle, new LoopingAudioStream(this, sound, loopSound, loop), -1, vol);
}

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

// This class is only used by speech in Simon1 Amiga CD32
class RawSound : public BaseSound {
	const byte _flags;
public:
	RawSound(Audio::Mixer *mixer, Common::File *file, bool isUnsigned)
		: BaseSound(mixer, file, 0, SOUND_BIG_ENDIAN), _flags(isUnsigned ? Audio::FLAG_UNSIGNED : 0) {}
	Audio::AudioStream *makeAudioStream(uint sound);
	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0);
};

Audio::AudioStream *RawSound::makeAudioStream(uint sound) {
	if (_offsets == NULL)
		return NULL;

	_file->seek(_offsets[sound], SEEK_SET);

	uint size = _file->readUint32BE();
	byte *buffer = (byte *)malloc(size);
	assert(buffer);
	_file->read(buffer, size);

	return Audio::makeRawStream(buffer, size, 22050, _flags);
}

void RawSound::playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol) {
	// Sound looping and volume are ignored.
	_mixer->playStream(type, handle, makeAudioStream(sound));
}

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

class CompressedSound : public BaseSound {
public:
	CompressedSound(Audio::Mixer *mixer, Common::File *file, uint32 base) : BaseSound(mixer, file, base, false) {}

	Common::SeekableReadStream *loadStream(uint sound) const {
		if (_offsets == NULL)
			return NULL;

		_file->seek(_offsets[sound], SEEK_SET);

		int i = 1;
		while (_offsets[sound + i] == _offsets[sound])
			i++;

		uint32 size = _offsets[sound + i] - _offsets[sound];

		return _file->readStream(size);
	}

	void playSound(uint sound, uint loopSound, Audio::Mixer::SoundType type, Audio::SoundHandle *handle, bool loop, int vol = 0) {
		convertVolume(vol);
		_mixer->playStream(type, handle, new LoopingAudioStream(this, sound, loopSound, loop), -1, vol);
	}
};

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

#ifdef USE_MAD
class MP3Sound : public CompressedSound {
public:
	MP3Sound(Audio::Mixer *mixer, Common::File *file, uint32 base = 0) : CompressedSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound) {
		Common::SeekableReadStream *tmp = loadStream(sound);
		if (!tmp)
			return NULL;
		return Audio::makeMP3Stream(tmp, DisposeAfterUse::YES);
	}
};
#endif

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

#ifdef USE_VORBIS
class VorbisSound : public CompressedSound {
public:
	VorbisSound(Audio::Mixer *mixer, Common::File *file, uint32 base = 0) : CompressedSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound) {
		Common::SeekableReadStream *tmp = loadStream(sound);
		if (!tmp)
			return NULL;
		return Audio::makeVorbisStream(tmp, DisposeAfterUse::YES);
	}
};
#endif

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

#ifdef USE_FLAC
class FLACSound : public CompressedSound {
public:
	FLACSound(Audio::Mixer *mixer, Common::File *file, uint32 base = 0) : CompressedSound(mixer, file, base) {}
	Audio::AudioStream *makeAudioStream(uint sound) {
		Common::SeekableReadStream *tmp = loadStream(sound);
		if (!tmp)
			return NULL;
		return Audio::makeFLACStream(tmp, DisposeAfterUse::YES);
	}
};
#endif

///////////////////////////////////////////////////////////////////////////////
#pragma mark -

static CompressedSound *makeCompressedSound(Audio::Mixer *mixer, Common::File *file, const Common::String &basename) {
#ifdef USE_FLAC
	file->open(basename + ".fla");
	if (file->isOpen()) {
		return new FLACSound(mixer, file);
	}
#endif
#ifdef USE_VORBIS
	file->open(basename + ".ogg");
	if (file->isOpen()) {
		return new VorbisSound(mixer, file);
	}
#endif
#ifdef USE_MAD
	file->open(basename + ".mp3");
	if (file->isOpen()) {
		return new MP3Sound(mixer, file);
	}
#endif
	return 0;
}


///////////////////////////////////////////////////////////////////////////////
#pragma mark -

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
	Common::File *file = new Common::File();

	if (!_hasVoiceFile) {
		_voice = makeCompressedSound(_mixer, file, gss->speech_filename);
		_hasVoiceFile = (_voice != 0);
	}
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

	const bool dataIsUnsigned = true;

	if (!_hasVoiceFile) {
		sprintf(filename, "%s.voc", gss->speech_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasVoiceFile = true;
			_voice = new VocSound(_mixer, file, dataIsUnsigned);
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
				_voice = new VocSound(_mixer, file, dataIsUnsigned);
		}
	}
}

void Sound::loadSfxFile(const GameSpecificSettings *gss) {
	char filename[16];
	Common::File *file = new Common::File();

	if (!_hasEffectsFile) {
		_effects = makeCompressedSound(_mixer, file, gss->effects_filename);
		_hasEffectsFile = (_effects != 0);
	}

	const bool dataIsUnsigned = true;

	if (!_hasEffectsFile) {
		sprintf(filename, "%s.voc", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VocSound(_mixer, file, dataIsUnsigned);
		}
	}
	if (!_hasEffectsFile) {
		sprintf(filename, "%s", gss->effects_filename);
		file->open(filename);
		if (file->isOpen()) {
			_hasEffectsFile = true;
			_effects = new VocSound(_mixer, file, dataIsUnsigned);
		}
	}
}

// This method is only used by Simon1 Amiga CD32 & Windows
void Sound::readSfxFile(const Common::String &filename) {
	if (_hasEffectsFile)
		return;

	_mixer->stopHandle(_effectsHandle);

	Common::File *file = new Common::File();
	file->open(filename);

	if (file->isOpen() == false) {
		error("readSfxFile: Can't load sfx file %s", filename.c_str());
	}

	const bool dataIsUnsigned = (_vm->getGameId() != GID_SIMON1CD32);

	delete _effects;
	if (_vm->getGameId() == GID_SIMON1CD32) {
		_effects = new VocSound(_mixer, file, dataIsUnsigned, 0, SOUND_BIG_ENDIAN);
	} else
		_effects = new WavSound(_mixer, file);
}

// This method is only used by Simon2
void Sound::loadSfxTable(Common::File *gameFile, uint32 base) {
	stopAll();

	delete _effects;
	const bool dataIsUnsigned = true;
	if (_vm->getPlatform() == Common::kPlatformWindows)
		_effects = new WavSound(_mixer, gameFile, base, DisposeAfterUse::NO);
	else
		_effects = new VocSound(_mixer, gameFile, dataIsUnsigned, base, false, DisposeAfterUse::NO);
}

// This method is only used by Simon1 Amiga CD32
void Sound::readVoiceFile(const Common::String &filename) {
	_mixer->stopHandle(_voiceHandle);

	Common::File *file = new Common::File();
	file->open(filename);

	if (file->isOpen() == false)
		error("readVoiceFile: Can't load voice file %s", filename.c_str());

	const bool dataIsUnsigned = false;

	delete _voice;
	_voice = new RawSound(_mixer, file, dataIsUnsigned);
}

void Sound::playVoice(uint sound) {
	if (_filenums) {
		if (_lastVoiceFile != _filenums[sound]) {
			_mixer->stopHandle(_voiceHandle);

			char filename[16];
			_lastVoiceFile = _filenums[sound];
			sprintf(filename, "voices%d.dat", _filenums[sound]);
			Common::File *file = new Common::File();
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
			_voice->playSound(sound, sound + 1, Audio::Mixer::kMusicSoundType, &_voiceHandle, true, -1500);
		else
			_voice->playSound(sound, sound, Audio::Mixer::kMusicSoundType, &_voiceHandle, true);
	} else if (_vm->getGameType() == GType_FF || _vm->getGameId() == GID_SIMON1CD32) {
		_voice->playSound(sound, Audio::Mixer::kSpeechSoundType, &_voiceHandle, false);
	} else {
		_voice->playSound(sound, Audio::Mixer::kSpeechSoundType, &_voiceHandle, false);
	}
}

void Sound::playEffects(uint sound) {
	if (!_effects)
		return;

	if (_effectsPaused)
		return;

	if (_vm->getGameType() == GType_SIMON1)
		_mixer->stopHandle(_effectsHandle);
	_effects->playSound(sound, Audio::Mixer::kSFXSoundType, &_effectsHandle, false);
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
	_effects->playSound(sound, Audio::Mixer::kSFXSoundType, &_ambientHandle, true);
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

	byte flags = 0;
	if (_vm->getPlatform() == Common::kPlatformPC)
		flags = Audio::FLAG_UNSIGNED;

	Audio::AudioStream *stream = Audio::makeRawStream(buffer, size, freq, flags);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_effectsHandle, stream);
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
	int size = READ_LE_UINT32(soundData + 4) + 8;
	Common::SeekableReadStream *stream = new Common::MemoryReadStream(soundData, size);
	Audio::RewindableAudioStream *sndStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);

	convertVolume(vol);
	convertPan(pan);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, handle, Audio::makeLoopingAudioStream(sndStream, loop ? 0 : 1), -1, vol, pan);
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
	Common::File *file = new Common::File();

	if (!_hasVoiceFile) {
		sprintf(filename, "%s%d", gss->speech_filename, disc);
		_voice = makeCompressedSound(_mixer, file, filename);
		_hasVoiceFile = (_voice != 0);
	}
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
