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

//define MIX_INIT_FLUIDSYNTH MIX_INIT_MID // renamed with SDL2_mixer >= 2.0.2
#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "audio/mididrv.h"
#include "audio/midiparser.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "common/memstream.h"
#include "awe/aifc_player.h"
#include "awe/sound.h"
#include "awe/sfx_player.h"

namespace Awe {

void Sound::playMusic(const char *path, int loops) {
	warning("TODO: playMusic");
}

void Sound::playSfxMusic(int num) {
	warning("TODO: playSfxMusic");
}

void Sound::playAifcMusic(const char *path, uint32 offset) {
	warning("TODO: playAifcMusic");
}

void Sound::stopMusic() {
	warning("TODO: stopMusic");
}

void Sound::stopAifcMusic() {
	warning("TODO: stopAifcMusic");
}

void Sound::stopSfxMusic() {
	warning("TODO: stopSfxMusic");
}

void Sound::preloadSoundAiff(byte num, const byte *data) {
	warning("TODO: preloadSoundAiff");
}

void Sound::playSoundRaw(byte channel, const byte *data, size_t size,
		int freq, byte volume) {
	assert(channel < MAX_CHANNELS);

	Common::MemoryReadStream *stream =
		new Common::MemoryReadStream(data, size);
	Audio::AudioStream *sound =
		Audio::makeRawStream(stream, freq,
			Audio::FLAG_16BITS,
			DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[channel],
		sound, -1, 255, 0, DisposeAfterUse::YES);
}

void Sound::playSoundWav(byte channel, const byte *data, size_t size,
		uint16 freq, byte volume, byte loop) {
	assert(channel < MAX_CHANNELS);

	Common::MemoryReadStream *stream =
		new Common::MemoryReadStream(data, size);
	Audio::AudioStream *sound = Audio::makeWAVStream(
		stream, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[channel],
		sound, -1, 255, 0, DisposeAfterUse::YES);
}

void Sound::playSoundAiff(byte channel, byte num, byte volume) {
	warning("TODO: playSoundAiff");
}

void Sound::stopSound(byte channel) {
	assert(channel < MAX_CHANNELS);
	_mixer->stopHandle(_channels[channel]);
}

#ifdef TODO
enum {
	TAG_RIFF = 0x46464952,
	TAG_WAVE = 0x45564157,
	TAG_fmt = 0x20746D66,
	TAG_data = 0x61746164
};

static const bool kAmigaStereoChannels = false; // 0,3:left 1,2:right

static int16 toS16(int a) {
	return ((a << 8) | a) - 32768;
}

static int16 mixS16(int sample1, int sample2) {
	const int sample = sample1 + sample2;
	return sample < -32768 ? -32768 : ((sample > 32767 ? 32767 : sample));
}

struct MixerChannel {
	const byte *_data;
	Frac _pos;
	uint32 _len;
	uint32 _loopLen, _loopPos;
	int _volume;
	void (MixerChannel:: *_mixWav)(int16 *sample, int count);

	void initRaw(const byte *data, int freq, int volume, int mixingFreq) {
		_data = data + 8;
		_pos.reset(freq, mixingFreq);

		const int len = READ_BE_UINT16(data) * 2;
		_loopLen = READ_BE_UINT16(data + 2) * 2;
		_loopPos = _loopLen ? len : 0;
		_len = len;

		_volume = volume;
	}

	void initWav(const byte *data, int freq, int volume, int mixingFreq, int len, bool bits16, bool stereo, bool loop) {
		_data = data;
		_pos.reset(freq, mixingFreq);

		_len = len;
		_loopLen = loop ? len : 0;
		_loopPos = 0;
		_volume = volume;
		_mixWav = bits16 ? (stereo ? &MixerChannel::mixWav<16, true> : &MixerChannel::mixWav<16, false>) : (stereo ? &MixerChannel::mixWav<8, true> : &MixerChannel::mixWav<8, false>);
	}
	void mixRaw(int16 &sample) {
		if (_data) {
			uint32 pos = _pos.getInt();
			_pos.offset += _pos.inc;
			if (_loopLen != 0) {
				if (pos >= _loopPos + _loopLen) {
					pos = _loopPos;
					_pos.offset = (_loopPos << Frac::BITS) + _pos.inc;
				}
			} else {
				if (pos >= _len) {
					_data = 0;
					return;
				}
			}
			sample = mixS16(sample, toS16(_data[pos] ^ 0x80) * _volume / 64);
		}
	}

	template<int bits, bool stereo>
	void mixWav(int16 *samples, int count) {
		for (int i = 0; i < count; i += 2) {
			uint32 pos = _pos.getInt();
			_pos.offset += _pos.inc;
			if (pos >= _len) {
				if (_loopLen != 0) {
					pos = 0;
					_pos.offset = _pos.inc;
				} else {
					_data = 0;
					break;
				}
			}
			if (stereo) {
				pos *= 2;
			}
			int valueL;
			if (bits == 8) { // U8
				valueL = toS16(_data[pos]) * _volume / 64;
			} else { // S16
				valueL = ((int16)READ_LE_UINT16(&_data[pos * sizeof(int16)])) * _volume / 64;
			}
			*samples = mixS16(*samples, valueL);
			++samples;

			int valueR;
			if (!stereo) {
				valueR = valueL;
			} else {
				if (bits == 8) { // U8
					valueR = toS16(_data[pos + 1]) * _volume / 64;
				} else { // S16
					valueR = ((int16)READ_LE_UINT16(&_data[(pos + 1) * sizeof(int16)])) * _volume / 64;
				}
			}
			*samples = mixS16(*samples, valueR);
			++samples;
		}
	}
};

static const byte *loadWav(const byte *data, int &freq, int &len, bool &bits16, bool &stereo) {
	uint32 riffMagic = READ_LE_UINT32(data);
	if (riffMagic != TAG_RIFF) return 0;
	uint32 riffLength = READ_LE_UINT32(data + 4);
	uint32 waveMagic = READ_LE_UINT32(data + 8);
	if (waveMagic != TAG_WAVE) return 0;
	uint32 offset = 12;
	uint32 chunkMagic, chunkLength = 0;
	// find fmt chunk
	do {
		offset += chunkLength + (chunkLength & 1);
		if (offset >= riffLength) return 0;
		chunkMagic = READ_LE_UINT32(data + offset);
		chunkLength = READ_LE_UINT32(data + offset + 4);
		offset += 8;
	} while (chunkMagic != TAG_fmt);

	if (chunkLength < 14) return 0;
	if (offset + chunkLength >= riffLength) return 0;

	// read format
	int formatTag = READ_LE_UINT16(data + offset);
	int channels = READ_LE_UINT16(data + offset + 2);
	int samplesPerSec = READ_LE_UINT32(data + offset + 4);
	int bitsPerSample = 0;
	if (chunkLength >= 16) {
		bitsPerSample = READ_LE_UINT16(data + offset + 14);
	} else if (formatTag == 1 && channels != 0) {
		int blockAlign = READ_LE_UINT16(data + offset + 12);
		bitsPerSample = (blockAlign * 8) / channels;
	}
	// check supported format
	if ((formatTag != 1) || // PCM
		(channels != 1 && channels != 2) || // mono or stereo
		(bitsPerSample != 8 && bitsPerSample != 16)) { // 8bit or 16bit
		warning("Unsupported wave file");
		return 0;
	}

	// find data chunk
	do {
		offset += chunkLength + (chunkLength & 1);
		if (offset >= riffLength) return 0;
		chunkMagic = READ_LE_UINT32(data + offset);
		chunkLength = READ_LE_UINT32(data + offset + 4);
		offset += 8;
	} while (chunkMagic != TAG_data);

	uint32 lengthSamples = chunkLength;
	if (offset + lengthSamples - 4 > riffLength) {
		lengthSamples = riffLength + 4 - offset;
	}
	if (channels == 2) lengthSamples >>= 1;
	if (bitsPerSample == 16) lengthSamples >>= 1;

	freq = samplesPerSec;
	len = lengthSamples;
	bits16 = (bitsPerSample == 16);
	stereo = (channels == 2);

	return data + offset;
}

struct Mixer_impl {

	static const int kMixFreq = 44100;
	static const SDL_AudioFormat kMixFormat = AUDIO_S16SYS;
	static const int kMixSoundChannels = 2;
	static const int kMixBufSize = 4096;
	static const int kMixChannels = 4;

	Mix_Chunk *_sounds[kMixChannels];
	Mix_Music *_music;
	MixerChannel _channels[kMixChannels];
	SfxPlayer *_sfx;
	std::map<int, Mix_Chunk *> _preloads; // AIFF preloads (3DO)

	void init(MixerType mixerType) {
		memset(_sounds, 0, sizeof(_sounds));
		_music = 0;
		memset(_channels, 0, sizeof(_channels));
		for (int i = 0; i < kMixChannels; ++i) {
			_channels[i]._mixWav = &MixerChannel::mixWav<8, false>;
		}
		_sfx = 0;

		Mix_Init(MIX_INIT_OGG | MIX_INIT_FLUIDSYNTH);
		if (Mix_OpenAudio(kMixFreq, kMixFormat, kMixSoundChannels, kMixBufSize) < 0) {
			warning("Mix_OpenAudio failed: %s", Mix_GetError());
		}
		switch (mixerType) {
		case kMixerTypeRaw:
			Mix_HookMusic(mixAudio, this);
			break;
		case kMixerTypeWav:
			Mix_SetPostMix(mixAudioWav, this);
			break;
		case kMixerTypeAiff:
			Mix_AllocateChannels(kMixChannels);
			break;
		default:
			break;
		}
	}
	void quit() {
		stopAll();
		Mix_CloseAudio();
		Mix_Quit();
	}

	void update() {
		for (int i = 0; i < kMixChannels; ++i) {
			if (_sounds[i] && !Mix_Playing(i)) {
				freeSound(i);
			}
		}
	}

	void playSoundRaw(byte channel, const byte *data, int freq, byte volume) {
		SDL_LockAudio();
		_channels[channel].initRaw(data, freq, volume, kMixFreq);
		SDL_UnlockAudio();
	}
	void playSoundWav(byte channel, const byte *data, int freq, byte volume, bool loop) {
		int wavFreq, len;
		bool bits16, stereo;
		const byte *wavData = loadWav(data, wavFreq, len, bits16, stereo);
		if (!wavData) return;

		if (wavFreq == 22050 || wavFreq == 44100 || wavFreq == 48000) {
			freq = (int)(freq * (wavFreq / 9943.0f));
		}

		SDL_LockAudio();
		_channels[channel].initWav(wavData, freq, volume, kMixFreq, len, bits16, stereo, loop);
		SDL_UnlockAudio();
	}
	void playSound(byte channel, int volume, Mix_Chunk *chunk, int loops = 0) {
		stopSound(channel);
		if (chunk) {
			Mix_PlayChannel(channel, chunk, loops);
		}
		setChannelVolume(channel, volume);
		_sounds[channel] = chunk;
	}
	void stopSound(byte channel) {
		SDL_LockAudio();
		_channels[channel]._data = 0;
		SDL_UnlockAudio();
		Mix_HaltChannel(channel);
		freeSound(channel);
	}
	void freeSound(int channel) {
		Mix_FreeChunk(_sounds[channel]);
		_sounds[channel] = 0;
	}
	void setChannelVolume(byte channel, byte volume) {
		SDL_LockAudio();
		_channels[channel]._volume = volume;
		SDL_UnlockAudio();
		Mix_Volume(channel, volume * MIX_MAX_VOLUME / 63);
	}

	void playMusic(const char *path, int loops = 0) {
		stopMusic();
		_music = Mix_LoadMUS(path);
		if (_music) {
			Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
			Mix_PlayMusic(_music, loops);
		} else {
			warning("Failed to load music '%s', %s", path, Mix_GetError());
		}
	}
	void stopMusic() {
		Mix_HaltMusic();
		Mix_FreeMusic(_music);
		_music = 0;
	}

	static void mixAifcPlayer(void *data, byte *s16buf, int len) {
		((AifcPlayer *)data)->readSamples((int16 *)s16buf, len / 2);
	}
	void playAifcMusic(AifcPlayer *aifc) {
		Mix_HookMusic(mixAifcPlayer, aifc);
	}
	void stopAifcMusic() {
		Mix_HookMusic(0, 0);
	}

	void playSfxMusic(SfxPlayer *sfx) {
		SDL_LockAudio();
		_sfx = sfx;
		_sfx->play(kMixFreq);
		SDL_UnlockAudio();
	}
	void stopSfxMusic() {
		SDL_LockAudio();
		if (_sfx) {
			_sfx->stop();
			_sfx = 0;
		}
		SDL_UnlockAudio();
	}

	void mixChannels(int16 *samples, int count) {
		if (kAmigaStereoChannels) {
			for (int i = 0; i < count; i += 2) {
				_channels[0].mixRaw(*samples);
				_channels[3].mixRaw(*samples);
				++samples;
				_channels[1].mixRaw(*samples);
				_channels[2].mixRaw(*samples);
				++samples;
			}
		} else {
			for (int i = 0; i < count; i += 2) {
				for (int j = 0; j < kMixChannels; ++j) {
					_channels[j].mixRaw(samples[i]);
				}
				samples[i + 1] = samples[i];
			}
		}
	}

	static void mixAudio(void *data, byte *s16buf, int len) {
		memset(s16buf, 0, len);
		Mixer_impl *mixer = (Mixer_impl *)data;
		mixer->mixChannels((int16 *)s16buf, len / sizeof(int16));
		if (mixer->_sfx) {
			mixer->_sfx->readSamples((int16 *)s16buf, len / sizeof(int16));
		}
	}

	void mixChannelsWav(int16 *samples, int count) {
		for (int i = 0; i < kMixChannels; ++i) {
			if (_channels[i]._data) {
				(_channels[i].*_channels[i]._mixWav)(samples, count);
			}
		}
	}

	static void mixAudioWav(void *data, byte *s16buf, int len) {
		Mixer_impl *mixer = (Mixer_impl *)data;
		mixer->mixChannelsWav((int16 *)s16buf, len / sizeof(int16));
	}

	void stopAll() {
		for (int i = 0; i < kMixChannels; ++i) {
			stopSound(i);
		}
		stopMusic();
		stopSfxMusic();
		for (std::map<int, Mix_Chunk *>::iterator it = _preloads.begin(); it != _preloads.end(); ++it) {
			debugC(kDebugSound, "Flush preload %d", it->first);
			Mix_FreeChunk(it->second);
		}
		_preloads.clear();
	}

	void preloadSoundAiff(int num, const byte *data) {
		if (_preloads.find(num) != _preloads.end()) {
			warning("AIFF sound %d is already preloaded", num);
		} else {
			const uint32 size = READ_BE_UINT32(data + 4) + 8;
			SDL_RWops *rw = SDL_RWFromConstMem(data, size);
			Mix_Chunk *chunk = Mix_LoadWAV_RW(rw, 1);
			_preloads[num] = chunk;
		}
	}

	void playSoundAiff(int channel, int num, int volume) {
		if (_preloads.find(num) == _preloads.end()) {
			warning("AIFF sound %d is not preloaded", num);
		} else {
			Mix_Chunk *chunk = _preloads[num];
			Mix_PlayChannel(channel, chunk, 0);
			Mix_Volume(channel, volume * MIX_MAX_VOLUME / 63);
		}
	}
};

Mixer::Mixer(SfxPlayer *sfx)
	: _aifc(0), _sfx(sfx) {
}

void Mixer::init(MixerType mixerType) {
	_impl = new Mixer_impl();
	_impl->init(mixerType);
}

void Mixer::quit() {
	stopAll();
	if (_impl) {
		_impl->quit();
		delete _impl;
	}
	delete _aifc;
}

void Mixer::update() {
	if (_impl) {
		_impl->update();
	}
}

void Mixer::playSoundRaw(byte channel, const byte *data, uint16 freq, byte volume) {
	debugC(kDebugSound, "Mixer::playChannel(%d, %d, %d)", channel, freq, volume);
	if (_impl) {
		return _impl->playSoundRaw(channel, data, freq, volume);
	}
}

void Mixer::playSoundWav(byte channel, const byte *data, uint16 freq, byte volume, byte loop) {
	debugC(kDebugSound, "Mixer::playSoundWav(%d, %d, %d)", channel, volume, loop);
	if (_impl) {
		return _impl->playSoundWav(channel, data, freq, volume, loop);
	}
}

void Mixer::stopSound(byte channel) {
	debugC(kDebugSound, "Mixer::stopChannel(%d)", channel);
	if (_impl) {
		return _impl->stopSound(channel);
	}
}

void Mixer::setChannelVolume(byte channel, byte volume) {
	debugC(kDebugSound, "Mixer::setChannelVolume(%d, %d)", channel, volume);
	if (_impl) {
		return _impl->setChannelVolume(channel, volume);
	}
}

void Mixer::playMusic(const char *path, byte loop) {
	debugC(kDebugSound, "Mixer::playMusic(%s, %d)", path, loop);
	if (_impl) {
		return _impl->playMusic(path, (loop != 0) ? -1 : 0);
	}
}

void Mixer::stopMusic() {
	debugC(kDebugSound, "Mixer::stopMusic()");
	if (_impl) {
		return _impl->stopMusic();
	}
}

void Mixer::playAifcMusic(const char *path, uint32 offset) {
	debugC(kDebugSound, "Mixer::playAifcMusic(%s)", path);
	if (!_aifc) {
		_aifc = new AifcPlayer();
	}
	if (_impl) {
		_impl->stopAifcMusic();
		if (_aifc->play(Mixer_impl::kMixFreq, path, offset)) {
			_impl->playAifcMusic(_aifc);
		}
	}
}

void Mixer::stopAifcMusic() {
	debugC(kDebugSound, "Mixer::stopAifcMusic()");
	if (_impl && _aifc) {
		_aifc->stop();
		_impl->stopAifcMusic();
	}
}

void Mixer::playSfxMusic(int num) {
	debugC(kDebugSound, "Mixer::playSfxMusic(%d)", num);
	if (_impl && _sfx) {
		return _impl->playSfxMusic(_sfx);
	}
}

void Mixer::stopSfxMusic() {
	debugC(kDebugSound, "Mixer::stopSfxMusic()");
	if (_impl && _sfx) {
		return _impl->stopSfxMusic();
	}
}

void Mixer::stopAll() {
	debugC(kDebugSound, "Mixer::stopAll()");
	if (_impl) {
		return _impl->stopAll();
	}
}

void Mixer::preloadSoundAiff(byte num, const byte *data) {
	debugC(kDebugSound, "Mixer::preloadSoundAiff(num:%d, data:%p)", num, data);
	if (_impl) {
		return _impl->preloadSoundAiff(num, data);
	}
}

void Mixer::playSoundAiff(byte channel, byte num, byte volume) {
	debugC(kDebugSound, "Mixer::playSoundAiff()");
	if (_impl) {
		return _impl->playSoundAiff(channel, num, volume);
	}
}
#endif

} // namespace Awe
