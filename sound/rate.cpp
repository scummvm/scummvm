/*
* August 21, 1998
* Copyright 1998 Fabrice Bellard.
*
* [Rewrote completly the code of Lance Norskog And Sundry
* Contributors with a more efficient algorithm.]
*
* This source code is freely redistributable and may be used for
* any purpose.	 This copyright notice must be maintained. 
* Lance Norskog And Sundry Contributors are not responsible for 
* the consequences of using this software.  
*/

/*
 * Sound Tools rate change effect file.
 */

#include "stdafx.h"
#include "rate.h"

#include <math.h>

/*
 * Linear Interpolation.
 *
 * The use of fractional increment allows us to use no buffer. It
 * avoid the problems at the end of the buffer we had with the old
 * method which stored a possibly big buffer of size
 * lcm(in_rate,out_rate).
 *
 * Limited to 16 bit samples and sampling frequency <= 65535 Hz. If
 * the input & output frequencies are equal, a delay of one sample is
 * introduced.	Limited to processing 32-bit count worth of samples.
 *
 * 1 << FRAC_BITS evaluating to zero in several places.	 Changed with
 * an (unsigned long) cast to make it safe.  MarkMLl 2/1/99
 *
 * Replaced all uses of floating point arithmetic by fixed point
 * calculations (Max Horn 2003-07-18).
 */

#define FRAC_BITS 16

/* Private data */

typedef struct ratestuff
{
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
} *rate_t;

/*
 * Prepare processing.
 */
int st_rate_start(eff_t effp, st_rate_t inrate, st_rate_t outrate)
{
	rate_t rate = (rate_t) effp->priv;
	unsigned long incr;

	if (inrate == outrate) {
		st_fail("Input and Output rates must be different to use rate effect");
		return (ST_EOF);
	}

	if (inrate >= 65536 || outrate >= 65536) {
		st_fail("rate effect can only handle rates < 65536");
		return (ST_EOF);
	}

	rate->opos_frac = 0;
	rate->opos = 0;

	/* increment */
	incr = (inrate << FRAC_BITS) / outrate;

	rate->opos_inc_frac = incr & ((1UL << FRAC_BITS) - 1);
	rate->opos_inc = incr >> FRAC_BITS;

	rate->ipos = 0;

	rate->ilast[0] = rate->ilast[1] = 0;
	rate->icur[0] = rate->icur[1] = 0;

	return (ST_SUCCESS);
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<bool stereo, int leftChannel>
int st_rate_flow(eff_t effp, AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol)
{
	rate_t rate = (rate_t) effp->priv;
	st_sample_t *ostart, *oend;
	st_sample_t out;
	unsigned long tmp;

	assert(leftChannel == 0 || leftChannel == 1);

	ostart = obuf;
	oend = obuf + *osamp * 2;

	while (obuf < oend && !input.eof()) {

		// read enough input samples so that ipos > opos
		while (rate->ipos <= rate->opos + 1) {
			rate->ilast[0] = rate->icur[0];
			rate->icur[0] = input.read();
			if (stereo) {
				rate->ilast[1] = rate->icur[1];
				rate->icur[1] = input.read();
			}
			rate->ipos++;

			// Abort if we reached the end of the input buffer
			if (input.eof())
				goto the_end;
		}

		// Loop as long as the outpos trails behind, and as long as there is
		// still space in the output buffer.
		while (rate->ipos > rate->opos + 1) {

			// interpolate
			out = (st_sample_t) (rate->ilast[leftChannel] + (((rate->icur[leftChannel] - rate->ilast[leftChannel]) * rate->opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));
			// adjust volume
			out = out * vol / 256;
	
			// output left channel sample
			clampedAdd(*obuf++, out);
			
			if (stereo) {
				// interpolate
				out = (st_sample_t) (rate->ilast[1-leftChannel] + (((rate->icur[1-leftChannel] - rate->ilast[1-leftChannel]) * rate->opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS));
				// adjust volume
				out = out * vol / 256;
			}
	
			// output right channel sample
			clampedAdd(*obuf++, out);
	
			// Increment output position
			tmp = rate->opos_frac + rate->opos_inc_frac;
			rate->opos += rate->opos_inc + (tmp >> FRAC_BITS);
			rate->opos_frac = tmp & ((1UL << FRAC_BITS) - 1);

			// Abort if we reached the end of the output buffer
			if (obuf >= oend)
				goto the_end;
		}
	}

the_end:
	*osamp = (obuf - ostart) / 2;
	return (ST_SUCCESS);
}

LinearRateConverter::LinearRateConverter(st_rate_t inrate, st_rate_t outrate, bool reverseStereo) {
	_reverseStereo = reverseStereo;
	st_rate_start(&effp, inrate, outrate);
}

int LinearRateConverter::flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	if (input.isStereo()) {
		if (_reverseStereo)
			return st_rate_flow<true, 1>(&effp, input, obuf, osamp, vol);
		else
			return st_rate_flow<true, 0>(&effp, input, obuf, osamp, vol);
	} else
		return st_rate_flow<false, 0>(&effp, input, obuf, osamp, vol);
}

int LinearRateConverter::drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	return (ST_SUCCESS);
}
