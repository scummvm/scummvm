/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include "stdafx.h"
#include "sound/rate.h"


#define FRAC_BITS 16


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
	bool _reverseStereo;

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
	int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol);
	int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
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
	opos = 0;

	/* increment */
	incr = (inrate << FRAC_BITS) / outrate;

	opos_inc_frac = incr & ((1UL << FRAC_BITS) - 1);
	opos_inc = incr >> FRAC_BITS;

	ipos = 0;

	ilast[0] = ilast[1] = 0;
	icur[0] = icur[1] = 0;
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<bool stereo, bool reverseStereo>
int LinearRateConverter<stereo, reverseStereo>::flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol)
{
	st_sample_t *ostart, *oend;
	st_sample_t out;
	unsigned long tmp;

	ostart = obuf;
	oend = obuf + *osamp * 2;

	while (obuf < oend && !input.eof()) {

		// read enough input samples so that ipos > opos
		while (ipos <= opos + 1) {
			ilast[0] = icur[0];
			icur[0] = input.read();
			if (stereo) {
				ilast[1] = icur[1];
				icur[1] = input.read();
			}
			ipos++;

			// Abort if we reached the end of the input buffer
			if (input.eof())
				goto the_end;
		}

		// Loop as long as the outpos trails behind, and as long as there is
		// still space in the output buffer.
		while (ipos > opos + 1) {

			// interpolate
			out = (st_sample_t) (ilast[reverseStereo ? 1 : 0] + (((icur[reverseStereo ? 1 : 0] - ilast[reverseStereo ? 1 : 0]) * opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));
			// adjust volume
			out = out * vol / 256;
	
			// output left channel sample
			clampedAdd(*obuf++, out);
			
			if (stereo) {
				// interpolate
				out = (st_sample_t) (ilast[reverseStereo ? 0 : 1] + (((icur[reverseStereo ? 0 : 1] - ilast[reverseStereo ? 0 : 1]) * opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));
				// adjust volume
				out = out * vol / 256;
			}
	
			// output right channel sample
			clampedAdd(*obuf++, out);
	
			// Increment output position
			tmp = opos_frac + opos_inc_frac;
			opos += opos_inc + (tmp >> FRAC_BITS);
			opos_frac = tmp & ((1UL << FRAC_BITS) - 1);

			// Abort if we reached the end of the output buffer
			if (obuf >= oend)
				goto the_end;
		}
	}

the_end:
	*osamp = (obuf - ostart) / 2;
	return (ST_SUCCESS);
}


#pragma mark -


/**
 * Simple audio rate converter for the case that the inrate equals the outrate.
 */
template<bool stereo, bool reverseStereo>
class CopyRateConverter : public RateConverter {
public:
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
		int16 tmp[2];
		st_size_t len = *osamp;
		assert(input.isStereo() == stereo);
		while (!input.eof() && len--) {
			tmp[0] = tmp[1] = input.read() * vol / 256;
			if (stereo)
				tmp[reverseStereo ? 0 : 1] = input.read() * vol / 256;
			clampedAdd(*obuf++, tmp[0]);
			clampedAdd(*obuf++, tmp[1]);
		}
		return (ST_SUCCESS);
	}
	virtual int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
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
