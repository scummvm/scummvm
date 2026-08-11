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

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/frac.h"
#include "common/mutex.h"
#include "common/system.h"

#ifndef DGDS_SOUND_DRIVERS_MACMIXER_H
#define DGDS_SOUND_DRIVERS_MACMIXER_H

namespace Dgds {

// Unsigned version of frac_t
typedef uint32 ufrac_t;
static inline ufrac_t uintToUfrac(uint16 value) { return value << FRAC_BITS; }
static inline uint16 ufracToUint(ufrac_t value) { return value >> FRAC_BITS; }

template <typename T>
class Mixer_Mac : public Audio::AudioStream {
public:
	enum {
		kChannels = 4,
		kInterruptFreq = 60
	};

	enum Mode {
		kModeAuthentic,
		kModeHq,
		kModeHqStereo
	};

	Mixer_Mac(Mode mode);
	void startMixer();
	void stopMixer();
	void setMixerVolume(byte volume) { _mixVolume = volume; }
	void resetChannel(uint channel);
	void resetChannels();
	// NOTE: Last sample accessed is data[endOffset + 1] in kModeHq(Stereo)
	void setChannelData(uint channel, const byte *data, uint16 startOffset, uint16 endOffset, uint16 loopLength = 0);
	void setChannelStep(uint channel, ufrac_t step);
	void setChannelVolume(uint channel, byte volume);
	void setChannelPan(uint channel, byte pan);

	// AudioStream
	bool isStereo() const override { return _mode == kModeHqStereo; }
	int getRate() const override { return (_mode == kModeAuthentic ? 11127 : g_system->getMixer()->getOutputRate()); }
	int readBuffer(int16 *data, const int numSamples) override;
	bool endOfData() const override { return false; }

	Common::Mutex _mutex;

private:
	template <Mode mode>
	void generateSamples(int16 *buf, int len);

	struct Channel {
		ufrac_t pos;
		ufrac_t step;
		const byte *data;
		uint16 endOffset;
		uint16 loopLength;
		byte volume;
		int8 pan;
	};

	ufrac_t _nextTick;
	ufrac_t _samplesPerTick;
	bool _isPlaying;
	const Mode _mode;
	Channel _mixChannels[kChannels];
	byte _mixVolume;
};

template <typename T>
Mixer_Mac<T>::Mixer_Mac(Mode mode) :
	_nextTick(0),
	_samplesPerTick(0),
	_mode(mode),
	_isPlaying(false),
	_mixChannels(),
	_mixVolume(8) {}

template <typename T>
void Mixer_Mac<T>::startMixer() {
	_nextTick = _samplesPerTick = uintToUfrac(getRate() / kInterruptFreq) + uintToUfrac(getRate() % kInterruptFreq) / kInterruptFreq;

	resetChannels();
	_isPlaying = true;
}

template <typename T>
void Mixer_Mac<T>::stopMixer() {
	resetChannels();
	_isPlaying = false;
}

template <typename T>
void Mixer_Mac<T>::setChannelData(uint channel, const byte *data, uint16 startOffset, uint16 endOffset, uint16 loopLength) {
	assert(channel < kChannels);

	Channel &ch = _mixChannels[channel];

	ch.data = data;
	ch.pos = uintToUfrac(startOffset);
	ch.endOffset = endOffset;
	ch.loopLength = loopLength;
}

template <typename T>
void Mixer_Mac<T>::setChannelStep(uint channel, ufrac_t step) {
	assert(channel < kChannels);

	if (_mode == kModeAuthentic) {
		_mixChannels[channel].step = step;
	} else {
		// We could take 11127Hz here, but it appears the original steps were
		// computed for 11000Hz
		// FIXME: One or two more bits of step precision might be nice here
		_mixChannels[channel].step = (ufrac_t)(step * 11000ULL / getRate());
	}
}

template <typename T>
void Mixer_Mac<T>::setChannelVolume(uint channel, byte volume) {
	assert(channel < kChannels);
	_mixChannels[channel].volume = volume;
}

template <typename T>
void Mixer_Mac<T>::setChannelPan(uint channel, byte pan) {
	assert(channel < kChannels);
	_mixChannels[channel].pan = pan;
}

template <typename T>
template <typename Mixer_Mac<T>::Mode mode>
void Mixer_Mac<T>::generateSamples(int16 *data, int len) {
	for (int i = 0; i < len; ++i) {
		int32 mixL = 0;
		int32 mixR = 0;

		for (int ci = 0; ci < kChannels; ++ci) {
			Channel &ch = _mixChannels[ci];

			if (!ch.data)
				continue;

			const uint16 curOffset = ufracToUint(ch.pos);

			if (mode == kModeHq || mode == kModeHqStereo) {
				int32 sample = (ch.data[curOffset] - 0x80) << 8;
				// Since _extraSamples > 0, we can safely access this sample
				const int32 sample2 = (ch.data[curOffset + 1] - 0x80) << 8;
				sample += fracToInt((sample2 - sample) * (ch.pos & FRAC_LO_MASK));
				sample *= ch.volume;

				if (mode == kModeHqStereo) {
					mixL += sample * (127 - ch.pan) / (63 * 64);
					mixR += sample * ch.pan / (63 * 64);
				} else {
					mixL += sample / 63;
				}
			} else {
				mixL += static_cast<T *>(this)->applyChannelVolume(ch.volume, ch.data[curOffset]) << 8;
			}

			ch.pos += ch.step;

			if (ufracToUint(ch.pos) > ch.endOffset) {
				if (ch.loopLength > 0) {
					do {
						ch.pos -= uintToUfrac(ch.loopLength);
					} while (ufracToUint(ch.pos) > ch.endOffset);
				} else {
					static_cast<T *>(this)->onChannelFinished(ci);
					ch.data = nullptr;
				}
			}
		}

		*data++ = (int16)CLIP<int32>(mixL, -32768, 32767) * _mixVolume / 8;
		if (mode == kModeHqStereo)
			*data++ = (int16)CLIP<int32>(mixR, -32768, 32767) * _mixVolume / 8;
	}
}

template <typename T>
int Mixer_Mac<T>::readBuffer(int16 *data, const int numSamples) {
	// Would probably be better inside generateSamples, but let's follow Audio::Paula
	Common::StackLock lock(_mutex);

	if (!_isPlaying) {
		memset(data, 0, numSamples * 2);
		return numSamples;
	}

	const int stereoFactor = isStereo() ? 2 : 1;
	int len = numSamples / stereoFactor;

	do {
		int step = len;
		if (step > ufracToUint(_nextTick))
			step = ufracToUint(_nextTick);

		switch (_mode) {
		case kModeAuthentic:
			generateSamples<kModeAuthentic>(data, step);
			break;
		case kModeHq:
			generateSamples<kModeHq>(data, step);
			break;
		case kModeHqStereo:
			generateSamples<kModeHqStereo>(data, step);
		}

		_nextTick -= uintToUfrac(step);
		if (ufracToUint(_nextTick) == 0) {
			static_cast<T *>(this)->interrupt();
			_nextTick += _samplesPerTick;
		}

		data += step * stereoFactor;
		len -= step;
	} while (len);

	return numSamples;
}

template <typename T>
void Mixer_Mac<T>::resetChannel(uint channel) {
	assert(channel < kChannels);

	Channel &ch = _mixChannels[channel];

	ch.pos = 0;
	ch.step = 0;
	ch.data = nullptr;
	ch.endOffset = 0;
	ch.loopLength = 0;
	ch.volume = 0;
	ch.pan = 64;
}

template <typename T>
void Mixer_Mac<T>::resetChannels() {
	for (uint ci = 0; ci < kChannels; ++ci)
		resetChannel(ci);
}

} // End of namespace Dgds

#endif // DGDS_SOUND_DRIVERS_MACMIXER_H
