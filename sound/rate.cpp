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
#include "sound/filter.h"
#include "common/frac.h"
#include "common/util.h"
#include "common/config-manager.h"

namespace Audio {


/**
 * The size of the intermediate input cache. Bigger values may increase
 * performance, but only until some point (depends largely on cache size,
 * target processor and various other factors), at which it will decrease
 * again.
 */
#define INTERMEDIATE_BUFFER_SIZE 512

static st_rate_t gcd(st_rate_t a, st_rate_t b);

/**
 * Audio rate converter based on simple resampling. Used when no
 * interpolation is required.
 *
 * Limited to sampling frequency <= 65535 Hz.
 */
template<bool stereo, bool reverseStereo>
class SimpleRateConverter : public RateConverter {
protected:
	st_sample_t inBuf[INTERMEDIATE_BUFFER_SIZE];
	const st_sample_t *inPtr;
	int inLen;

	/** position of how far output is ahead of input */
	/** Holds what would have been opos-ipos */
	long opos;

	/** fractional position increment in the output stream */
	long opos_inc;

public:
	SimpleRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
		return ST_SUCCESS;
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
SimpleRateConverter<stereo, reverseStereo>::SimpleRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if ((inrate % outrate) != 0) {
		error("Input rate must be a multiple of output rate to use rate effect");
	}

	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	opos = 1;

	/* increment */
	opos_inc = inrate / outrate;

	inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<bool stereo, bool reverseStereo>
int SimpleRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *ostart, *oend;

	ostart = obuf;
	oend = obuf + osamp * 2;

	while (obuf < oend) {

		// read enough input samples so that opos >= 0
		do {
			// Check if we have to refill the buffer
			if (inLen == 0) {
				inPtr = inBuf;
				inLen = input.readBuffer(inBuf, ARRAYSIZE(inBuf));
				if (inLen <= 0)
					return ST_EOF;
			}
			inLen -= (stereo ? 2 : 1);
			opos--;
			if (opos >= 0) {
				inPtr += (stereo ? 2 : 1);
			}
		} while (opos >= 0);

		st_sample_t out0, out1;
		out0 = *inPtr++;
		out1 = (stereo ? *inPtr++ : out0);

		// Increment output position
		opos += opos_inc;

		// output left channel
		clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

		// output right channel
		clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);
		
		obuf += 2;
	}
	return ST_SUCCESS;
}

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
	frac_t opos;

	/** fractional position increment in the output stream */
	frac_t opos_inc;

	/** last sample(s) in the input stream (left/right channel) */
	st_sample_t ilast0, ilast1;
	/** current sample(s) in the input stream (left/right channel) */
	st_sample_t icur0, icur1;

public:
	LinearRateConverter(st_rate_t inrate, st_rate_t outrate);
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
		return ST_SUCCESS;
	}
};


/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
LinearRateConverter<stereo, reverseStereo>::LinearRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if (inrate >= 65536 || outrate >= 65536) {
		error("rate effect can only handle rates < 65536");
	}

	opos = FRAC_ONE;

	// Compute the linear interpolation increment.
	// This will overflow if inrate >= 2^16, and underflow if outrate >= 2^16.
	// Also, if the quotient of the two rate becomes too small / too big, that
	// would cause problems, but since we rarely scale from 1 to 65536 Hz or vice
	// versa, I think we can live with that limiation ;-).
	opos_inc = (inrate << FRAC_BITS) / outrate;

	ilast0 = ilast1 = 0;
	icur0 = icur1 = 0;

	inLen = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<bool stereo, bool reverseStereo>
int LinearRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *ostart, *oend;

	ostart = obuf;
	oend = obuf + osamp * 2;

	while (obuf < oend) {

		// read enough input samples so that opos < 0
		while (FRAC_ONE <= opos) {
			// Check if we have to refill the buffer
			if (inLen == 0) {
				inPtr = inBuf;
				inLen = input.readBuffer(inBuf, ARRAYSIZE(inBuf));
				if (inLen <= 0)
					return ST_EOF;
			}
			inLen -= (stereo ? 2 : 1);
			ilast0 = icur0;
			icur0 = *inPtr++;
			if (stereo) {
				ilast1 = icur1;
				icur1 = *inPtr++;
			}
			opos -= FRAC_ONE;
		}

		// Loop as long as the outpos trails behind, and as long as there is
		// still space in the output buffer.
		while (opos < FRAC_ONE && obuf < oend) {
			// interpolate
			st_sample_t out0, out1;
			out0 = (st_sample_t)(ilast0 + (((icur0 - ilast0) * opos + FRAC_HALF) >> FRAC_BITS));
			out1 = (stereo ?
						  (st_sample_t)(ilast1 + (((icur1 - ilast1) * opos + FRAC_HALF) >> FRAC_BITS)) :
						  out0);

			// output left channel
			clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);
			
			obuf += 2;

			// Increment output position
			opos += opos_inc;
		}
	}
	return ST_SUCCESS;
}


#pragma mark -

/**
 * Audio rate converter that uses filtering techniques.
 *
 * This currently allows us to upsample by an integer factor with very little
 * spectral distortion.
 *
 * TODO: Optimise filter creation -- too slow for large upsampling factors
 */
template<bool stereo, bool reverseStereo>
class FilteringRateConverter : public RateConverter {
protected:
	FIRFilter *filt;
	
	/* Circular buffers for inputs which are currently in the filter */
	st_sample_t *inBuf;
	
	/* Offset into the circular buffer of the most recent input sample */
	uint16 inPos;
	
	/* 
	 * The number of samples we need to fetch into the input buffer from the
	 * AudioStream
	 */
	uint16 toFetch;
	
	/*
     * Fraction of the input frequency which should be used as passband for
	 * the filter design.
	 */
	double lowpassBW;
	
	/* Upsampling and downsampling factors */
	uint16 upFactor, downFactor;
	
	/* Subfilter length */
	uint16 subLen;
	
	/* The current bank that we're up to (for output samples) */
	uint16 currBank;
	
	/* The number of channels of audio */
	uint8 numChan;
	
	/*
	 * Used to adjust the DC filter gain, for the case of positive
	 * reinforcement in the output when the filter coefficients are negative.
	 */
	double kFudgeFactor;
	
	/* The maximum DC gain across all subfilters */
	double filtGain;
	
	/* Random number source for getting dither values */
	Common::RandomSource *rand;

public:
	FilteringRateConverter(st_rate_t inrate, st_rate_t outrate);
	~FilteringRateConverter() {
		delete filt;
		free(inBuf);
		delete rand;
	}
	int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
	int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r);
};

/*
 * Prepare processing.
 */
template<bool stereo, bool reverseStereo>
FilteringRateConverter<stereo, reverseStereo>::FilteringRateConverter(st_rate_t inrate, st_rate_t outrate) {
	/*
	 * Note: we'll first upsample the audio by upFactor and filter it, and
	 * then downsample it by downFactor.  This means we'll use upFactor
	 * different filter banks and only calculate one in every downFactor
	 * outputs.
	 */
	upFactor = outrate / gcd(inrate, outrate);
	downFactor = inrate / gcd(inrate, outrate);
	
	/* Initially we'll need to put some samples in the input buffer */
	toFetch = 1 + (downFactor - 1) / upFactor;
	
	/* We'll be ignoring the first downFactor - 1 samples */
	currBank = (downFactor - 1) % upFactor;
	
	// TODO: Make an editable way to set this value
	/* This sets the point in the input signal where attenuation will begin */
	lowpassBW = 0.925;
	
	/* We'll want to lowpass based on the most restrictive frequency */
	st_rate_t stopband = (outrate < inrate) ? outrate : inrate;
	
	// TODO: Make it so that this filter data can be reused by other
	//       converters. 
	/* Generate the filter coefficients */
	filt = new FIRFilter(lowpassBW * stopband / 2.0, stopband / 2.0,
						-40, 80, (uint32)inrate * upFactor, upFactor);
	
	uint32 len = filt->getLength();
	
	subLen = (len + (upFactor - 1)) / upFactor;
	
	/* Want to be able to divide the filter perfectly into subfilters */
	assert((len % subLen) == 0);
	
	/* Find the DC gain of each subfilter */
	double *gain = (double *)calloc(upFactor, sizeof(double));
	
	uint32 i;
	for (i = 0; i < len; i++) {
		/* 
		 * Using the commented-out line and setting kFudgeFactor to 1 will
		 * ensure that no clipping will occur, but this makes output volumes
		 * softer than those of other resamplers using otherwise equal
		 * options.
		 */
		//gain[i % upFactor] += fabs((filt->getCoeffs())[i]);
		gain[i % upFactor] += (filt->getCoeffs())[i];
	}
	
	/* Find the maximum of these subfilter gains */
	filtGain = 0;
	
	for (i = 0; i < upFactor; i++) {
		if (gain[i] > filtGain) {
			filtGain = gain[i];
		}
	}
	
	free(gain);
	
	// TODO: Empirically determined -- is there some reasoned approximation?
	kFudgeFactor = 0.75;
	
	numChan = (stereo ? 2 : 1);
	
	inBuf = (st_sample_t *)calloc(numChan * subLen, sizeof(st_sample_t));
	
	inPos = 0;
	
	rand = new Common::RandomSource();
}

template<bool stereo, bool reverseStereo>
int FilteringRateConverter<stereo, reverseStereo>::flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	st_sample_t *oend = obuf + osamp * 2;
	
	while (obuf < oend) {
		while (toFetch--) {
			/*
			 * We need to fetch a new input sample (two if this is a stereo
			 * stream).  We'll want to replace the oldest sample(s) in the
			 * circular buffer.  Since inPos points to the newest sample(s) in
			 * the buffer, the oldest sample(s) can be found directly before
			 * inPos.
			 */
			 
			/* Circularly decrement inPos by numChan */
			inPos = (inPos + (subLen - numChan)) % subLen;
			
			uint8 inLen;
			
			inLen = input.readBuffer(&inBuf[inPos], numChan);
			if (inLen == 0) {
				/* No more input samples */
				return this->drain(obuf, (oend - obuf) / 2, vol_l, vol_r);
			}
			
			assert(inLen == numChan);
		}
		
		double accum0 = 0;
		double accum1 = 0;
		
		uint16 i;
		
		/*
		 * Convolve the input samples with the filter to get the next
		 * outputs
		 */
		for (i = 0; i < subLen; i++) {
			accum0 += (double)inBuf[(inPos + numChan * i) % subLen]
						* (filt->getCoeffs())[currBank + i*upFactor];
			if (stereo) {
				accum1 += (double)inBuf[(inPos + numChan * i + 1) % subLen]
						* (filt->getCoeffs())[currBank + i*upFactor];
			}
		}
		
		/* 
		 * Cancel out the gain effects of the filter (approximated -- if we 
		 * ensure that there can be no clipping at all, the volume is far
		 * softer than the other resamplers produce)
		 */
		accum0 *= kFudgeFactor / filtGain;
		
		/* Scale down according to the volume settings. */
		accum0 *= (double)vol_l / Audio::Mixer::kMaxMixerVolume;
		
		/*
		 * The overall effect of the following code is to add dithering of up
		 * to +/- 0.5, using a triangular distribution centred at 0 (central
		 * limit theorem), then rounding to the nearest integer; however, due
		 * to the fact that the floating point -> integer cast will round
		 * towards 0, the implementation uses a triangular distribution
		 * centred at 0.5 (spreading out to 0 and 1) and then checks the sign
		 * of the sample to determine if the dither value should be added or
		 * subtracted (to give the desired overall effect of rounding to the
		 * nearest integer).
		 */
		
		/* RandomSource hiccups if you use -1U here, since it adds 1 to this */
		static const uint kRandMax = -2U; 
		
		double dither0 = ((accum0 >= 0) ? 1 : -1) * 
				( (double)(rand->getRandomNumber(kRandMax)) + 
				  (double)(rand->getRandomNumber(kRandMax)) )
				/ (2.0 * kRandMax);
		
		accum0 += dither0;
		
		/* Check for clipping and clamp values if necessary */
		if (accum0 > ST_SAMPLE_MAX) {
			debug(1, "Clipping: sample value is %g (should be maximally %g)", accum0, (double)ST_SAMPLE_MAX);
			accum0 = ST_SAMPLE_MAX;
		} else	if (accum0 < ST_SAMPLE_MIN) {
			debug(1, "Clipping: sample value is %g (should be minimally %g)", accum0, (double)ST_SAMPLE_MIN);
			accum0 = ST_SAMPLE_MIN;
		}
		
		/* Perform the same steps for the second channel if neccesary */
		if (stereo || vol_l != vol_r) {
			accum1 *= kFudgeFactor / filtGain;
			
			accum1 *= (double)vol_r / Audio::Mixer::kMaxMixerVolume;
			
			double dither1 = ((accum1 >= 0) ? 1 : -1) * 
					( (double)(rand->getRandomNumber(kRandMax)) + 
					(double)(rand->getRandomNumber(kRandMax)) )
					/ (2.0 * kRandMax);
			
			accum1 += dither1;
			
			if (accum1 > ST_SAMPLE_MAX) {
				debug(1, "Clipping: sample value is %g (should be maximally %g)", accum1, (double)ST_SAMPLE_MAX);
				accum1 = ST_SAMPLE_MAX;
			} else	if (accum1 < ST_SAMPLE_MIN) {
				debug(1, "Clipping: sample value is %g (should be minimally %g)", accum1, (double)ST_SAMPLE_MIN);
				accum1 = ST_SAMPLE_MIN;
			}
		} else {
			accum1 = accum0;
		}
		
		/* Output left channel */
		clampedAdd(obuf[reverseStereo    ], (int)accum0);
		
		/* output right channel */
		clampedAdd(obuf[reverseStereo ^ 1], (int)accum1);
		
		obuf += 2;
		
		/* Find out how many more input samples are needed next time */
		toFetch = (currBank + downFactor) / upFactor;
		
		/* Circularly increment currBank by downFactor */
		currBank = (currBank + downFactor) % upFactor;
	}
	
	return ST_SUCCESS;
}

template<bool stereo, bool reverseStereo>
int FilteringRateConverter<stereo, reverseStereo>::drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
	// TODO: implement this
	
	return ST_EOF;
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
		for (; len > 0; len -= (stereo ? 2 : 1)) {
			st_sample_t out0, out1;
			out0 = *ptr++;
			out1 = (stereo ? *ptr++ : out0);

			// output left channel
			clampedAdd(obuf[reverseStereo    ], (out0 * (int)vol_l) / Audio::Mixer::kMaxMixerVolume);

			// output right channel
			clampedAdd(obuf[reverseStereo ^ 1], (out1 * (int)vol_r) / Audio::Mixer::kMaxMixerVolume);
			
			obuf += 2;
		}
		return ST_SUCCESS;
	}

	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol_l, st_volume_t vol_r) {
		return ST_SUCCESS;
	}
};


#pragma mark -

static inline st_rate_t gcd(st_rate_t a, st_rate_t b) {
	while (b) {
		int temp = b;
		b = a % b;
		a = temp;
	}
	
	return a;
}

// TODO: Add options checking for filtering rate converters.
template<bool stereo, bool reverseStereo>
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate) {
	if (ConfMan.hasKey("rate_converter")
			&& (ConfMan.getInt("rate_converter") == kFilteringType)) {
		if (outrate != inrate) {
			return new FilteringRateConverter<stereo, reverseStereo>(inrate, outrate);
		}
	}
	
	if (inrate != outrate) {
		if ((inrate % outrate) == 0) {
			return new SimpleRateConverter<stereo, reverseStereo>(inrate, outrate);
		} else {
			return new LinearRateConverter<stereo, reverseStereo>(inrate, outrate);
		}
	} else {
		return new CopyRateConverter<stereo, reverseStereo>();
	}
}

/**
 * Create and return a RateConverter object for the specified input and output rates.
 */
RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo) {
	if (stereo) {
		if (reverseStereo)
			return makeRateConverter<true, true>(inrate, outrate);
		else
			return makeRateConverter<true, false>(inrate, outrate);
	} else
		return makeRateConverter<false, false>(inrate, outrate);
}

} // End of namespace Audio
