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

#include "kyra/sound/sound_digital_mr.h"
#include "kyra/resource/resource.h"
#include "kyra/engine/kyra_mr.h"

#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/flac.h"

#include "common/util.h"

namespace Kyra {

class KyraAudioStream : public Audio::SeekableAudioStream {
public:
	KyraAudioStream(Audio::SeekableAudioStream *impl) : _impl(impl), _rate(impl->getRate()), _fadeSamples(0), _fadeCount(0), _fading(0), _endOfData(false) {}
	~KyraAudioStream() override { delete _impl; _impl = 0; }

	int readBuffer(int16 *buffer, const int numSamples) override;
	bool isStereo() const override { return _impl->isStereo(); }
	bool endOfData() const override { return _impl->endOfData() | _endOfData; }
	int getRate() const override { return _rate; }

	void setRate(int newRate) { _rate = newRate; }
	void beginFadeOut(uint32 millis);

	bool seek(const Audio::Timestamp &where) override { return _impl->seek(where); }
	Audio::Timestamp getLength() const override { return _impl->getLength(); }
private:
	Audio::SeekableAudioStream *_impl;

	int _rate;

	int32 _fadeSamples;
	int32 _fadeCount;
	int _fading;

	bool _endOfData;
};

void KyraAudioStream::beginFadeOut(uint32 millis) {
	_fadeSamples = (millis * getRate()) / 1000;
	if (_fading == 0)
		_fadeCount = _fadeSamples;
	_fading = -1;
}

int KyraAudioStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesRead = _impl->readBuffer(buffer, numSamples);

	if (_fading) {
		int samplesProcessed = 0;
		for (; samplesProcessed < samplesRead; ++samplesProcessed) {
			// To help avoid overflows for long fade times, we divide both
			// _fadeSamples and _fadeCount when calculating the new sample.

			int32 div = _fadeSamples / 256;
			if (_fading) {
				*buffer = (*buffer * (_fadeCount / 256)) / div;
				++buffer;

				_fadeCount += _fading;

				if (_fadeCount < 0) {
					_fadeCount = 0;
					_endOfData = true;
				} else if (_fadeCount > _fadeSamples) {
					_fadeCount = _fadeSamples;
					_fading = 0;
				}
			}
		}

		if (_endOfData) {
			memset(buffer, 0, (samplesRead - samplesProcessed) * sizeof(int16));
			samplesRead = samplesProcessed;
		}
	}

	return samplesRead;
}

SoundDigital_MR::SoundDigital_MR(KyraEngine_MR *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	for (uint i = 0; i < ARRAYSIZE(_sounds); ++i)
		_sounds[i].stream = 0;
}

SoundDigital_MR::~SoundDigital_MR() {
	for (int i = 0; i < ARRAYSIZE(_sounds); ++i)
		stopSound(i);
}

int SoundDigital_MR::playSound(const char *filename, uint8 priority, Audio::Mixer::SoundType type, int volume, bool loop, int channel) {
	Sound *use = 0;
	if (channel != -1 && channel < ARRAYSIZE(_sounds)) {
		stopSound(channel);
		use = &_sounds[channel];
	} else {
		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (!isPlaying(channel)) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (strcmp(_sounds[channel].filename, filename) == 0) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		for (channel = 0; !use && channel < ARRAYSIZE(_sounds); ++channel) {
			if (_sounds[channel].priority <= priority) {
				stopSound(channel);
				use = &_sounds[channel];
				break;
			}
		}

		if (!use) {
			warning("no free sound channel");
			return -1;
		}
	}

	Common::SeekableReadStream *stream = 0;
	int usedCodec = -1;
	for (int i = 0; _supportedCodecs[i].fileext; ++i) {
		Common::String file = filename;
		file += _supportedCodecs[i].fileext;

		if (!_vm->resource()->exists(file.c_str()))
			continue;

		stream = _vm->resource()->createReadStream(file);
		usedCodec = i;
	}

	if (!stream) {
		warning("Couldn't find soundfile '%s'", filename);
		return -1;
	}

	Common::strlcpy(use->filename, filename, sizeof(use->filename));
	use->priority = priority;
	debugC(5, kDebugLevelSound, "playSound: \"%s\"", use->filename);
	Audio::SeekableAudioStream *audioStream = _supportedCodecs[usedCodec].streamFunc(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Couldn't create audio stream for file '%s'", filename);
		return -1;
	}
	use->stream = new KyraAudioStream(audioStream);
	assert(use->stream);
	if (use->stream->endOfData()) {
		delete use->stream;
		use->stream = 0;

		return -1;
	}

	if (volume > 255)
		volume = 255;
	volume = (volume * Audio::Mixer::kMaxChannelVolume) / 255;

	if (type == Audio::Mixer::kSpeechSoundType && _vm->heliumMode())
		use->stream->setRate(32765);

	_mixer->playStream(type, &use->handle, makeLoopingAudioStream(use->stream, loop ? 0 : 1), -1, volume);
	return use - _sounds;
}

bool SoundDigital_MR::isPlaying(int channel) {
	if (channel == -1)
		return false;

	assert(channel >= 0 && channel < ARRAYSIZE(_sounds));

	if (!_sounds[channel].stream)
		return false;

	return _mixer->isSoundHandleActive(_sounds[channel].handle);
}

void SoundDigital_MR::stopSound(int channel) {
	if (channel == -1)
		return;

	assert(channel >= 0 && channel < ARRAYSIZE(_sounds));
	_mixer->stopHandle(_sounds[channel].handle);
	_sounds[channel].stream = 0;
}

void SoundDigital_MR::stopAllSounds() {
	for (int i = 0; i < ARRAYSIZE(_sounds); ++i) {
		if (isPlaying(i))
			stopSound(i);
	}
}

void SoundDigital_MR::beginFadeOut(int channel, int ticks) {
	if (isPlaying(channel))
		_sounds[channel].stream->beginFadeOut(ticks * _vm->tickLength());
}

// static res

Audio::SeekableAudioStream *makeAUDStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse);

const SoundDigital_MR::AudioCodecs SoundDigital_MR::_supportedCodecs[] = {
#ifdef USE_FLAC
	{ ".FLA", Audio::makeFLACStream },
#endif // USE_FLAC
#ifdef USE_VORBIS
	{ ".OGG", Audio::makeVorbisStream },
#endif // USE_VORBIS
#ifdef USE_MAD
	{ ".MP3", Audio::makeMP3Stream },
#endif // USE_MAD
	{ ".AUD", makeAUDStream },
	{ 0, 0 }
};

} // End of namespace Kyra
