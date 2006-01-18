/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

/*
 * The code in this file is based on code with Copyright 1998 Fabrice Bellard
 * Fabrice original code is part of SoX (http://sox.sourceforge.net).
 * Max Horn adapted that code to the needs of ScummVM and rewrote it partial,
 * in the process removing any use of floating point arithmetic. Various other
 * improvments over the original code were made.
 */

#include "common/stdafx.h"
#include "sound/audiostream.h"
#include "sound/rate.h"
#include "sound/mixer.h"
#include "common/util.h"

namespace Audio {

/**
 * The precision of the fractional computations used by the rate converter.
 * Normally you should never have to modify this value.
 */
#define FRAC_BITS 16

/**
 * The size of the intermediate input cache. Bigger values may increase
 * performance, but only until some point (depends largely on cache size,
 * target processor and various other factors), at which it will decrease
 * again.
 */
#define INTERMEDIATE_BUFFER_SIZE 512


/**
 * Audio rate converter based on simple linear Interpolation.
 *
 * The use of fractional increment allows us to use no buffer. It
 * avoid the problems at the end of the buffer we had with the old
 * method which stored a possibly big buffer of size
 * lcm(in_rate,out_rate).
 *
 * Limited to sampling frequency <= 65535 Hz.
 */

template<bool stereo, bool reverseStereo>
class LinearRateConverter : public RateConverter {
protected:
	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
	const st_sample_t *inPtr;
	int inLen;

	/** fractional position of the output stream in input stream unit */
	unsigned long opos, opos_frac;

	/** fractional position increment in the output stream */
	unsigned long opos_inc, opos_inc_frac;

	/** position in the input stream (integer) */
	unsigned long ipos;

	/** last sample(s) in the input stream (left/right channel) */
	st_sample_t ilast[2];
	/** current sample(s) in the input stream (left/right channel) */
	st_sample_t icur[2];

public:
	LinearRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
LinearRateConverter<stereo, reverseStereo>::LinearRateConverter(st_rate_t inrate, st_rate_t outrate) {
	unsigned long incr;

	if (inrate == outrate) {
		error("Input and Output rates must be different to use rate effect");
	}

	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	opos_frac = 0;
	opos = 1;

	/* increment */
	incr = (inrate << FRAC_BITS) / outrate;

	opos_inc_frac = incr & ((1UL << FRAC_BITS) - 1);
	opos_inc = incr >> FRAC_BITS;

	ipos = 0;

	ilast[0] = ilast[1] = 0;
	icur[0] = icur[1] = 0;

	inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<bool stereo, bool reverseStereo>
int LinearRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *ostart, *oend;
	st_sample_t out[2];

	const int numChannels = stereo ? 2 : 1;
	int i;

	ostart = obuf;
	oend = obuf + osamp * 2;

	while (obuf < oend) {

		// read enough input samples so that ipos > opos
		while (ipos <= opos) {
			// Check if we have to refill the buffer
			if (inLen == 0) {
				inPtr = inBuf;
				inLen = input.readBuffer(inBuf, ARRAYSIZE(inBuf));
				if (inLen <= 0)
					goto the_end;
			}
			for (i = 0; i < numChannels; i++) {
				ilast[i] = icur[i];
				icur[i] = *inPtr++;
				inLen--;
			}
			ipos++;
		}

		// Loop as long as the outpos trails behind, and as long as there is
		// still space in the output buffer.
		while (ipos > opos) {

			// interpolate
			out[0] = out[1] = (st_sample_t)(ilast[0] + (((icur[0] - ilast[0]) * opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));

			if (stereo) {
				// interpolate
				out[reverseStereo ? 0 : 1] = (st_sample_t)(ilast[1] + (((icur[1] - ilast[1]) * opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));
			}

			// output left channel
			clampedAdd(*obuf++, (out[0] * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(*obuf++, (out[1] * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);

			// Increment output position
			unsigned long tmp = opos_frac + opos_inc_frac;
			opos += opos_inc + (tmp >> FRAC_BITS);
			opos_frac = tmp & ((1UL << FRAC_BITS) - 1);

			// Abort if we reached the end of the output buffer
			if (obuf >= oend)
				goto the_end;
		}
	}

the_end:
	return (ST_SUCCESS);
}


#pragma mark -


/**
 * Simple audio rate converter for the case that the inrate equals the outrate.
 */
template<bool stereo, bool reverseStereo>
class CopyRateConverter : public RateConverter {
	st_sample_t *_buffer;
	st_size_t _bufferSize;
public:
	CopyRateConverter() : _buffer(0), _bufferSize(0) {}
	~CopyRateConverter() {
		free(_buffer);
	}

	virtual int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
		assert(input.isStereo() == stereo);

		st_sample_t *ptr;
		st_size_t len;

		if (stereo)
			osamp *= 2;

		// Reallocate temp buffer, if necessary
		if (osamp > _bufferSize) {
			free(_buffer);
			_buffer = (st_sample_t *)malloc(osamp * 2);
			_bufferSize = osamp;
		}

		// Read up to 'osamp' samples into our temporary buffer
		len = input.readBuffer(_buffer, osamp);

		// Mix the data into the output buffer
		ptr = _buffer;
		while (len--) {
			st_sample_t tmp0, tmp1;
			tmp0 = tmp1 = *ptr++;
			if (stereo) {
				if (reverseStereo)
					tmp0 = *ptr++;
				else
					tmp1 = *ptr++;
				len--;
			}

			// output left channel
			clampedAdd(*obuf++, (tmp0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(*obuf++, (tmp1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);
		}
		return (ST_SUCCESS);
	}
	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};


#pragma mark -


/**
 * Create and return a RateConverter object for the specified input and output rates.
 */
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo) {
	if (inrate != outrate) {
		if (stereo) {
			if (reverseStereo)
				return new LinearRateConverter<true, true>(inrate, outrate);
			else
				return new LinearRateConverter<true, false>(inrate, outrate);
		} else
			return new LinearRateConverter<false, false>(inrate, outrate);
		//return new ResampleRateConverter(inrate, outrate, 1);
	} else {
		if (stereo) {
			if (reverseStereo)
				return new CopyRateConverter<true, true>();
			else
				return new CopyRateConverter<true, false>();
		} else
			return new CopyRateConverter<false, false>();
	}
}

} // End of namespace Audio
