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
	unsigned long opos_frac;  /* fractional position of the output stream in input stream unit */
	unsigned long opos;

	unsigned long opos_inc_frac;  /* fractional position increment in the output stream */
	unsigned long opos_inc;

	unsigned long ipos;	 /* position in the input stream (integer) */

	st_sample_t ilast[2]; /* last sample(s) in the input stream (left/right channel) */
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

	rate->ilast[0] = 0;
	rate->ilast[1] = 0;
	return (ST_SUCCESS);
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
template<int channels>
int st_rate_flow(eff_t effp, AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol)
{
	rate_t rate = (rate_t) effp->priv;
	st_sample_t *ostart, *oend;
	st_sample_t ilast[2], icur[2], out;
	unsigned long tmp;
	int i;

	assert(channels == 1 || channels == 2);

	for (i = 0; i < channels; i++)
		ilast[i] = rate->ilast[i];

	ostart = obuf;
	oend = obuf + *osamp;

	while (obuf < oend && !input.eof()) {

		/* read enough input samples so that ipos > opos */
		while (rate->ipos <= rate->opos) {
			for (i = 0; i < channels; i++)
				ilast[i] = input.read();
			rate->ipos++;
			/* See if we finished the input buffer yet */

			if (input.eof())
				goto the_end;
		}

		// read the input sample(s)
		icur[0] = input.read();
		if (channels == 2) {
			if (input.eof())
				goto the_end;	// Shouldn't happen if data comes pair-wise
			icur[1] = input.read();
		}

		while (rate->ipos > rate->opos) {
			for (i = 0; i < channels; i++) {
				// interpolate
				out = ilast[i] + (((icur[i] - ilast[i]) * rate->opos_frac + (1UL << (FRAC_BITS-1))) >> FRAC_BITS);
		
				// adjust volume
				out = out * vol / 256;
		
				// output left channel sample
				clampedAdd(*obuf++, out);
			}
	
			// For mono input, repeat the sample to produce stereo output
			if (channels == 1)
				clampedAdd(*obuf++, out);
	
			// Increment output position
			tmp = rate->opos_frac + rate->opos_inc_frac;
			rate->opos = rate->opos + rate->opos_inc + (tmp >> FRAC_BITS);
			rate->opos_frac = tmp & ((1UL << FRAC_BITS) - 1);
		}

		// Increment input position again (for the sample we read now)
		rate->ipos++;
		for (i = 0; i < channels; i++)
			ilast[i] = icur[i];
	}

the_end:
	*osamp = obuf - ostart;
	for (i = 0; i < channels; i++)
		rate->ilast[i] = ilast[i];
	return (ST_SUCCESS);
}


#pragma mark -


LinearRateConverter::LinearRateConverter(st_rate_t inrate, st_rate_t outrate) {
	st_rate_start(&effp, inrate, outrate);
}

int LinearRateConverter::flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	if (input.isStereo())
		return st_rate_flow<2>(&effp, input, obuf, osamp, vol);
	else
		return st_rate_flow<1>(&effp, input, obuf, osamp, vol);
}

int LinearRateConverter::drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	return (ST_SUCCESS);
}
