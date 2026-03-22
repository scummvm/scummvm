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

#ifndef AUDIO_RATE_H
#define AUDIO_RATE_H

#include "common/frac.h"

namespace Audio {
/**
 * @defgroup audio_rate Sample rate
 * @ingroup audio
 *
 * @brief API for managing sound sample rate.
 * @{
 */

class AudioStream;

typedef uint16 st_volume_t;
typedef uint32 st_size_t;
typedef uint32 st_rate_t;

// This assumes that 'a' and 'b' are 24-bit samples at most
template <typename T>
static inline void clampedAdd(T& a, int b) {
	constexpr unsigned long long highestBit = 1ULL << (sizeof(T) * 8 - 1);
	constexpr int maxVal = (int)(highestBit - 1);
	constexpr int minVal = ~maxVal;

	int val;
#ifdef OUTPUT_UNSIGNED_AUDIO
	constexpr int signMask = (int)highestBit;
	val = ((int)a ^ signMask) + b;
#else
	val = (int)a + b;
#endif

	if (val > maxVal)
		val = maxVal;
	else if (val < minVal)
		val = minVal;

#ifdef OUTPUT_UNSIGNED_AUDIO
	a = (T)(val ^ signMask);
#else
	a = (T)val;
#endif
}

/**
 * Helper class that handles resampling an AudioStream between an input and output
 * sample rate. Its regular use case is upsampling from the native stream rate
 * to the one used by the sound mixer. However, the input/output rates can be
 * manually adjusted to change playback speed and produce sound effects.
*/
class RateConverter {
public:
	RateConverter() {}
	virtual ~RateConverter() {}

	/**
	 * Convert the provided AudioStream to the target sample rate.
	 *
	 * @param input				The AudioStream to read data from.
	 * @param outBuffer			The buffer that the resampled audio will be written to. Must have size of at least @p numSamples.
	 * @param outBytesPerSample	The size of each output sample in bytes (e.g. sizeof(int16) or sizeof(int32)).
	 * @param numSamples		The desired number of samples to be written into the buffer.
	 * @param vol_l				Volume for left channel.
	 * @param vol_r				Volume for right channel.
	 *
	 * @return Number of sample pairs written into the buffer.
	 */
	virtual int convert(AudioStream &input, byte *outBuffer, uint outBytesPerSample, st_size_t numSamples, st_volume_t vol_l, st_volume_t vol_r) = 0;

	virtual void setInputRate(st_rate_t inputRate) = 0;
	virtual void setOutputRate(st_rate_t outputRate) = 0;

	virtual st_rate_t getInputRate() const = 0;
	virtual st_rate_t getOutputRate() const = 0;

	/**
	 * Does the internal buffer still have some leftover data?
	 *
	 * @return True if we need to drain, false otherwise
	 */
	virtual bool needsDraining() const = 0;
};

RateConverter *makeRateConverter(st_rate_t inRate, st_rate_t outRate, bool inStereo, bool outStereo, bool reverseStereo);

/** @} */
} // End of namespace Audio

#endif
