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

typedef int16 st_sample_t;
typedef uint16 st_volume_t;
typedef uint32 st_size_t;
typedef uint32 st_rate_t;

/* Minimum and maximum values a sample can hold. */
enum {
	ST_SAMPLE_MAX = 0x7fffL,
	ST_SAMPLE_MIN = (-ST_SAMPLE_MAX - 1L)
};

static inline void clampedAdd(int16& a, int b) {
	int val;
#ifdef OUTPUT_UNSIGNED_AUDIO
	val = (a ^ 0x8000) + b;
#else
	val = a + b;
#endif

	if (val > ST_SAMPLE_MAX)
		val = ST_SAMPLE_MAX;
	else if (val < ST_SAMPLE_MIN)
		val = ST_SAMPLE_MIN;

#ifdef OUTPUT_UNSIGNED_AUDIO
	a = ((int16)val) ^ 0x8000;
#else
	a = val;
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
	 * @param input			The AudioStream to read data from.
	 * @param outBuffer		The buffer that the resampled audio will be written to. Must have size of at least @p numSamples.
	 * @param numSamples	The desired number of samples to be written into the buffer.
	 * @param vol_l			Volume for left channel.
	 * @param vol_r			Volume for right channel.
	 * 
	 * @return Number of sample pairs written into the buffer.
	 */
	virtual int convert(AudioStream &input, st_sample_t *outBuffer, st_size_t numSamples, st_volume_t vol_l, st_volume_t vol_r) = 0;

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
