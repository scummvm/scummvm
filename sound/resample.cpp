/*
* July 5, 1991
* Copyright 1991 Lance Norskog And Sundry Contributors
* This source code is freely redistributable and may be used for
* any purpose.	 This copyright notice must be maintained. 
* Lance Norskog And Sundry Contributors are not responsible for 
* the consequences of using this software.
*/

/*
 * Sound Tools rate change effect file.
 * Spiffy rate changer using Smith & Wesson Bandwidth-Limited Interpolation.
 * The algorithm is described in "Bandlimited Interpolation -
 * Introduction and Algorithm" by Julian O. Smith III.
 * Available on ccrma-ftp.stanford.edu as
 * pub/BandlimitedInterpolation.eps.Z or similar.
 *
 * The latest stand alone version of this algorithm can be found
 * at ftp://ccrma-ftp.stanford.edu/pub/NeXT/
 * under the name of resample-version.number.tar.Z
 *
 * NOTE: There is a newer version of the resample routine then what
 * this file was originally based on.  Those adventurous might be
 * interested in reviewing its improvesments and porting it to this
 * version.
 */

/* Fixed bug: roll off frequency was wrong, too high by 2 when upsampling,
 * too low by 2 when downsampling.
 * Andreas Wilde, 12. Feb. 1999, andreas@eakaw2.et.tu-dresden.de
*/

/*
 * October 29, 1999
 * Various changes, bugfixes(?), increased precision, by Stan Brooks.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 */ 
/*
 * SJB: [11/25/99]
 * TODO: another idea for improvement...
 * note that upsampling usually doesn't require interpolation,
 * therefore is faster and more accurate than downsampling.
 * Downsampling by an integer factor is also simple, since
 * it just involves decimation if the input is already 
 * lowpass-filtered to the output Nyquist freqency.
 * Get the idea? :)
 */

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "rate.h"

/* resample includes */
#include "resample.h"

typedef struct {
	byte priv[1024];
} eff_struct;
typedef eff_struct *eff_t;

/* this Float MUST match that in filter.c */
#define Float double/*float*/

/* largest factor for which exact-coefficients upsampling will be used */
#define NQMAX 511

#define BUFFSIZE 8192 /*16384*/	 /* Total I/O buffer size */

/* Private data for Lerp via LCM file */
typedef struct resamplestuff {
	double Factor;     /* Factor = Fout/Fin sample rates */
	double rolloff;    /* roll-off frequency */
	double beta;	      /* passband/stopband tuning magic */
	int quadr;	      /* non-zero to use qprodUD quadratic interpolation */
	long Nmult;
	long Nwing;
	long Nq;
	Float *Imp;	      /* impulse [Nwing+1] Filter coefficients */

	double Time;	      /* Current time/pos in input sample */
	long dhb;

	long a, b;	      /* gcd-reduced input,output rates	  */
	long t;	      /* Current time/pos for exact-coeff's method */

	long Xh;	      /* number of past/future samples needed by filter	 */
	long Xoff;	      /* Xh plus some room for creep  */
	long Xread;	      /* X[Xread] is start-position to enter new samples */
	long Xp;	      /* X[Xp] is position to start filter application	 */
	long Xsize, Ysize;  /* size (Floats) of X[],Y[]	  */
	long Yposition;		/* FIXME: offset into Y buffer */
	Float *X, *Y;      /* I/O buffers */
} *resample_t;

static void LpFilter(double c[],
                     long N,
                     double frq,
                     double Beta,
                     long Num);

/* makeFilter is used by filter.c */
int makeFilter(Float Imp[],
               long Nwing,
               double Froll,
               double Beta,
               long Num,
               int Normalize);

static long SrcUD(resample_t r, long Nx);
static long SrcEX(resample_t r, long Nx);

/* here for linear interp.  might be useful for other things */
static st_rate_t st_gcd(st_rate_t a, st_rate_t b)
{
	if (b == 0)
		return a;
	else
		return st_gcd(b, a % b);
}


/*
 * Process options
 */
int st_resample_getopts(eff_t effp, int n, const char **argv) {
	resample_t r = (resample_t) effp->priv;

	/* These defaults are conservative with respect to aliasing. */
	r->rolloff = 0.80;
	r->beta = 16; /* anything <=2 means Nutall window */
	r->quadr = 0;
	r->Nmult = 45;

	/* This used to fail, but with sox-12.15 it works. AW */
	if ((n >= 1)) {
		if (!strcmp(argv[0], "-qs")) {
			r->quadr = 1;
			n--;
			argv++;
		} else if (!strcmp(argv[0], "-q")) {
			r->rolloff = 0.875;
			r->quadr = 1;
			r->Nmult = 75;
			n--;
			argv++;
		} else if (!strcmp(argv[0], "-ql")) {
			r->rolloff = 0.94;
			r->quadr = 1;
			r->Nmult = 149;
			n--;
			argv++;
		}
	}

	if ((n >= 1) && (sscanf(argv[0], "%lf", &r->rolloff) != 1)) {
		st_fail("Usage: resample [ rolloff [ beta ] ]");
		return (ST_EOF);
	} else if ((r->rolloff <= 0.01) || (r->rolloff >= 1.0)) {
		st_fail("resample: rolloff factor (%f) no good, should be 0.01<x<1.0", r->rolloff);
		return (ST_EOF);
	}

	if ((n >= 2) && !sscanf(argv[1], "%lf", &r->beta)) {
		st_fail("Usage: resample [ rolloff [ beta ] ]");
		return (ST_EOF);
	} else if (r->beta <= 2.0) {
		r->beta = 0;
		st_report("resample opts: Nuttall window, cutoff %f\n", r->rolloff);
	} else {
		st_report("resample opts: Kaiser window, cutoff %f, beta %f\n", r->rolloff, r->beta);
	}
	return (ST_SUCCESS);
}

/*
 * Prepare processing.
 */
int st_resample_start(eff_t effp, st_rate_t inrate, st_rate_t outrate) {
	resample_t r = (resample_t) effp->priv;
	long Xoff, gcdrate;
	int i;

	if (inrate == outrate) {
		st_fail("Input and Output rates must be different to use resample effect");
		return (ST_EOF);
	}

	r->Factor = (double)outrate / (double)inrate;

	gcdrate = st_gcd(inrate, outrate);
	r->a = inrate / gcdrate;
	r->b = outrate / gcdrate;

	if (r->a <= r->b && r->b <= NQMAX) {
		r->quadr = -1; /* exact coeff's	  */
		r->Nq = r->b;  /* MAX(r->a,r->b);	*/
	} else {
		r->Nq = Nc; /* for now */
	}

	/* Check for illegal constants */
# if 0
	if (Lp >= 16)
		st_fail("Error: Lp>=16");
	if (Nb + Nhg + NLpScl >= 32)
		st_fail("Error: Nb+Nhg+NLpScl>=32");
	if (Nh + Nb > 32)
		st_fail("Error: Nh+Nb>32");
# endif

	/* Nwing: # of filter coeffs in right wing */
	r->Nwing = r->Nq * (r->Nmult / 2 + 1) + 1;

	r->Imp = (Float *)malloc(sizeof(Float) * (r->Nwing + 2)) + 1;
	/* need Imp[-1] and Imp[Nwing] for quadratic interpolation */
	/* returns error # <=0, or adjusted wing-len > 0 */
	i = makeFilter(r->Imp, r->Nwing, r->rolloff, r->beta, r->Nq, 1);
	if (i <= 0) {
		st_fail("resample: Unable to make filter\n");
		return (ST_EOF);
	}

	st_report("Nmult: %ld, Nwing: %ld, Nq: %ld\n",r->Nmult,r->Nwing,r->Nq);	// FIXME

	if (r->quadr < 0) { /* exact coeff's method */
		r->Xh = r->Nwing / r->b;
		st_report("resample: rate ratio %ld:%ld, coeff interpolation not needed\n", r->a, r->b);
	} else {
		r->dhb = Np;	/* Fixed-point Filter sampling-time-increment */
		if (r->Factor < 1.0)
			r->dhb = (long)(r->Factor * Np + 0.5);
		r->Xh = (r->Nwing << La) / r->dhb;
		/* (Xh * dhb)>>La is max index into Imp[] */
	}

	/* reach of LP filter wings + some creeping room */
	Xoff = r->Xh + 10;
	r->Xoff = Xoff;

	/* Current "now"-sample pointer for input to filter */
	r->Xp = Xoff;
	/* Position in input array to read into */
	r->Xread = Xoff;
	/* Current-time pointer for converter */
	r->Time = Xoff;
	if (r->quadr < 0) { /* exact coeff's method */
		r->t = Xoff * r->Nq;
	}
	i = BUFFSIZE - 2 * Xoff;
	if (i < r->Factor + 1.0 / r->Factor)	/* Check input buffer size */
	{
		st_fail("Factor is too small or large for BUFFSIZE");
		return (ST_EOF);
	}

	r->Xsize = (long)(2 * Xoff + i / (1.0 + r->Factor));
	r->Ysize = BUFFSIZE - r->Xsize;
	st_report("Xsize %ld, Ysize %ld, Xoff %ld",r->Xsize,r->Ysize,r->Xoff);	// FIXME

	r->X = (Float *) malloc(sizeof(Float) * (BUFFSIZE));
	r->Y = r->X + r->Xsize;
	r->Yposition = 0;

	/* Need Xoff zeros at beginning of sample */
	for (i = 0; i < Xoff; i++)
		r->X[i] = 0;
	return (ST_SUCCESS);
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */
int st_resample_flow(eff_t effp, AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	resample_t r = (resample_t) effp->priv;
	long i, k, last;
	long Nout = 0;	// The number of bytes we effectively output
	long Nx;		// The number of bytes we will read from input
	long Nproc;		// The number of bytes we process to generate Nout output bytes
	const long obufSize = *osamp;

TODO: adjust for the changes made to AudioInputStream; add support for stereo
initially, could just average the left/right channel -> bad for quality of course,
but easiest to implement and would get this going again.
Next step is to duplicate the X/Y buffers... a lot of computations don't care about
how many channels there are anyway, they could just be ran twice, e.g. SrcEX and SrcUD.
But better for efficiency would be to rewrite those to deal with 2 channels, too.
Because esp in SrcEX/SrcUD, only very few computations depend on the input data,
and dealing with both channels in parallel should only be a little slower than dealing
with them alone

	// Constrain amount we actually process
	//fprintf(stderr,"Xp %d, Xread %d\n",r->Xp, r->Xread);

	// Initially assume we process the full X buffer starting at the filter
	// start position.
	Nproc = r->Xsize - r->Xp;

	// Nproc is bounded indirectly by the size of output buffer, and also by
	// the remaining size of the Y buffer (whichever is smaller).
	// We round up for the output buffer, because we want to generate enough
	// bytes to fill it.
	i = MIN((long)((r->Ysize - r->Yposition) / r->Factor), (long)ceil((obufSize - r->Yposition) / r->Factor));
	if (Nproc > i)
		Nproc = i;

	// Now that we know how many bytes we want to process, we determine
	// how many bytes to read. We already have Xread bytes in our input
	// buffer, so we need Nproc - r->Xread more bytes.
	Nx = Nproc - r->Xread + r->Xoff + r->Xp; // FIXME: Fingolfin thinks this is the correct thing, not what's in the next line!
//	Nx = Nproc - r->Xread; /* space for right-wing future-data */
	if (Nx <= 0) {
		st_fail("resample: Can not handle this sample rate change. Nx not positive: %d", Nx);
		return (ST_EOF);
	}

	// Read in up to Nx bytes
	for (i = r->Xread; i < Nx + r->Xread && !input.eos(); i++) {
		r->X[i] = (Float)input.read();
	}
	Nx = i - r->Xread;	// Compute how many samples we actually read

	fprintf(stderr,"Nx %d\n",Nx);


	last = Nx + r->Xread;	// 'last' is the idx after the last valid byte in X (i.e. number of bytes are in buffer X right now)
	
	// Finally compute the effective number of bytes to process
	Nproc = last - r->Xoff - r->Xp;

	if (Nproc <= 0) {
		/* fill in starting here next time */
		r->Xread = last;
		/* leave *isamp alone, we consumed it */
		*osamp = 0;
		return (ST_SUCCESS);
	}
	if (r->quadr < 0) { /* exact coeff's method */
		long creep;
		Nout = SrcEX(r, Nproc) + r->Yposition;
		fprintf(stderr,"Nproc %d --> %d\n",Nproc,Nout);
		/* Move converter Nproc samples back in time */
		r->t -= Nproc * r->b;
		/* Advance by number of samples processed */
		r->Xp += Nproc;
		/* Calc time accumulation in Time */
		creep = r->t / r->b - r->Xoff;
		if (creep) {
			r->t -= creep * r->b;	 /* Remove time accumulation   */
			r->Xp += creep;	 /* and add it to read pointer */
			fprintf(stderr,"Nproc %ld, creep %ld\n",Nproc,creep);
		}
	} else { /* approx coeff's method */
		long creep;
		Nout = SrcUD(r, Nproc) + r->Yposition;
		fprintf(stderr,"Nproc %d --> %d\n",Nproc,Nout);
		/* Move converter Nproc samples back in time */
		r->Time -= Nproc;
		/* Advance by number of samples processed */
		r->Xp += Nproc;
		/* Calc time accumulation in Time */
		creep = (long)(r->Time - r->Xoff);
		if (creep) {
			r->Time -= creep;   /* Remove time accumulation   */
			r->Xp += creep;     /* and add it to read pointer */
			fprintf(stderr,"Nproc %ld, creep %ld\n",Nproc,creep);
		}
	}

	/* Copy back portion of input signal that must be re-used */
	k = r->Xp - r->Xoff;
	//fprintf(stderr,"k %d, last %d\n",k,last);
	for (i = 0; i < last - k; i++)
		r->X[i] = r->X[i + k];

	/* Pos in input buff to read new data into */
	r->Xread = i;
	r->Xp = r->Xoff;

printf("osamp = %ld, Nout = %ld\n", obufSize, Nout);
	long numOutSamples = MIN(obufSize, Nout);
	for (i = 0; i < numOutSamples; i++) {
		int sample = (int)(r->Y[i] * vol / 256);
		clampedAdd(*obuf++, sample);
#if 1	// FIXME: Hack to generate stereo output
//		clampedAdd(*obuf++, sample);
		*obuf++;
#endif
	}

	// Move down the remaining Y bytes
	for (i = numOutSamples; i < Nout; i++) {
		r->Y[i-numOutSamples] = r->Y[i];
	}
	if (Nout > numOutSamples)
		r->Yposition = Nout - numOutSamples;
	else
		r->Yposition = 0;
	
	// Finally set *osamp to the number of samples we put into the output buffer
	*osamp = numOutSamples;

	return (ST_SUCCESS);
}

/*
 * Process tail of input samples.
 */
int st_resample_drain(eff_t effp, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	resample_t r = (resample_t) effp->priv;
	long osamp_res;
	st_sample_t *Obuf;
	int rc;

	/*fprintf(stderr,"Xoff %d, Xt %d  <--- DRAIN\n",r->Xoff, r->Xt);*/

	/* stuff end with Xoff zeros */
	ZeroInputStream zero(r->Xoff);
	osamp_res = *osamp;
	Obuf = obuf;
	while (!zero.eos() && osamp_res > 0) {
		st_sample_t Osamp;
		Osamp = osamp_res;
		rc = st_resample_flow(effp, zero, Obuf, (st_size_t *) & Osamp, vol);
		if (rc)
			return rc;
		/*fprintf(stderr,"DRAIN isamp,osamp	(%d,%d) -> (%d,%d)\n",
		    isamp_res,osamp_res,Isamp,Osamp);*/
		Obuf += Osamp;
		osamp_res -= Osamp;
	}
	*osamp -= osamp_res;
	fprintf(stderr,"DRAIN osamp %d\n", *osamp);
	if (!zero.eos())
		st_warn("drain overran obuf\n");
	fflush(stderr);
	return (ST_SUCCESS);
}

/*
 * Do anything required when you stop reading samples.	
 * Don't close input file! 
 */
int st_resample_stop(eff_t effp) {
	resample_t r = (resample_t) effp->priv;

	free(r->Imp - 1);
	free(r->X);
	/* free(r->Y); Y is in same block starting at X */
	return (ST_SUCCESS);
}

/* over 90% of CPU time spent in this iprodUD() function */
/* quadratic interpolation */
static double qprodUD(const Float Imp[], const Float *Xp, long Inc, double T0,
                      long dhb, long ct) {
	const double f = 1.0 / (1 << La);
	double v;
	long Ho;

	Ho = (long)(T0 * dhb);
	Ho += (ct - 1) * dhb; /* so Float sum starts with smallest coef's */
	Xp += (ct - 1) * Inc;
	v = 0;
	do {
		Float coef;
		long Hoh;
		Hoh = Ho >> La;
		coef = Imp[Hoh];
		{
			Float dm, dp, t;
			dm = coef - Imp[Hoh - 1];
			dp = Imp[Hoh + 1] - coef;
			t = (Ho & Amask) * f;
			coef += ((dp - dm) * t + (dp + dm)) * t * 0.5;
		}
		/* filter coef, lower La bits by quadratic interpolation */
		v += coef * *Xp;   /* sum coeff * input sample */
		Xp -= Inc;	   /* Input signal step. NO CHECK ON ARRAY BOUNDS */
		Ho -= dhb;	   /* IR step */
	} while (--ct);
	return v;
}

/* linear interpolation */
static double iprodUD(const Float Imp[], const Float *Xp, long Inc,
                      double T0, long dhb, long ct) {
	const double f = 1.0 / (1 << La);
	double v;
	long Ho;

	Ho = (long)(T0 * dhb);
	Ho += (ct - 1) * dhb; /* so Float sum starts with smallest coef's */
	Xp += (ct - 1) * Inc;
	v = 0;
	do {
		Float coef;
		long Hoh;
		Hoh = Ho >> La;
		/* if (Hoh >= End) break; */
		coef = Imp[Hoh] + (Imp[Hoh + 1] - Imp[Hoh]) * (Ho & Amask) * f;
		/* filter coef, lower La bits by linear interpolation */
		v += coef * *Xp;   /* sum coeff * input sample */
		Xp -= Inc;	   /* Input signal step. NO CHECK ON ARRAY BOUNDS */
		Ho -= dhb;	   /* IR step */
	} while (--ct);
	return v;
}

/* From resample:filters.c */
/* Sampling rate conversion subroutine */

static long SrcUD(resample_t r, long Nx) {
	Float *Ystart, *Y;
	double Factor;
	double dt;		       /* Step through input signal */
	double time;
	double (*prodUD)(const Float Imp[], const Float *Xp, long Inc, double T0, long dhb, long ct);
	int n;

	prodUD = (r->quadr) ? qprodUD : iprodUD; /* quadratic or linear interp */
	Factor = r->Factor;
	time = r->Time;
	dt = 1.0 / Factor;	   /* Output sampling period */
	//fprintf(stderr,"Factor %f, dt %f, ",Factor,dt);
	//fprintf(stderr,"Time %f, ",r->Time);
	/* (Xh * dhb)>>La is max index into Imp[] */
	/*fprintf(stderr,"ct=%d\n",ct);*/
	//fprintf(stderr,"ct=%.2f %d\n",(double)r->Nwing*Na/r->dhb, r->Xh);
	//fprintf(stderr,"ct=%ld, T=%.6f, dhb=%6f, dt=%.6f\n", r->Xh, time-floor(time),(double)r->dhb/Na,dt);
	Ystart = Y = r->Y + r->Yposition;
	n = (int)ceil((double)Nx / dt);
	while (n--) {
		Float *Xp;
		double v;
		double T;
		T = time - floor(time);	   /* fractional part of Time */
		Xp = r->X + (long)time;	   /* Ptr to current input sample */

		/* Past  inner product: */
		v = (*prodUD)(r->Imp, Xp, -1, T, r->dhb, r->Xh); /* needs Np*Nmult in 31 bits */
		/* Future inner product: */
		v += (*prodUD)(r->Imp, Xp + 1, 1, (1.0 - T), r->dhb, r->Xh); /* prefer even total */

		if (Factor < 1)
			v *= Factor;
		*Y++ = v;		     /* Deposit output */
		time += dt;	     /* Move to next sample by time increment */
	}
	r->Time = time;
	fprintf(stderr,"Time %f\n",r->Time);
	return (Y - Ystart);	       /* Return the number of output samples */
}

/* exact coeff's */
static double prodEX(const Float Imp[], const Float *Xp,
                     long Inc, long T0, long dhb, long ct) {
	double v;
	const Float *Cp;

	Cp = Imp + (ct - 1) * dhb + T0; /* so Float sum starts with smallest coef's */
	Xp += (ct - 1) * Inc;
	v = 0;
	do {
		v += *Cp * *Xp;   /* sum coeff * input sample */
		Cp -= dhb;	   /* IR step */
		Xp -= Inc;	   /* Input signal step. */
	} while (--ct);
	return v;
}

static long SrcEX(resample_t r, long Nx) {
	Float *Ystart, *Y;
	double Factor;
	long a, b;
	long time;
	int n;

	Factor = r->Factor;
	time = r->t;
	a = r->a;
	b = r->b;
	Ystart = Y = r->Y + r->Yposition;
	n = (Nx * b + (a - 1)) / a;
	while (n--) {
		Float *Xp;
		double v;
		long T;
		T = time % b;		   /* fractional part of Time */
		Xp = r->X + (time / b);	   /* Ptr to current input sample */

		/* Past	 inner product: */
		v = prodEX(r->Imp, Xp, -1, T, b, r->Xh);
		/* Future inner product: */
		v += prodEX(r->Imp, Xp + 1, 1, b - T, b, r->Xh);

		if (Factor < 1)
			v *= Factor;
		*Y++ = v;	      /* Deposit output */
		time += a;	      /* Move to next sample by time increment */
	}
	r->t = time;
	return (Y - Ystart);	       /* Return the number of output samples */
}

int makeFilter(Float Imp[], long Nwing, double Froll, double Beta,
               long Num, int Normalize) {
	double *ImpR;
	long Mwing, i;

	if (Nwing > MAXNWING)		      /* Check for valid parameters */
		return ( -1);
	if ((Froll <= 0) || (Froll > 1))
		return ( -2);

	/* it does help accuracy a bit to have the window stop at
	 * a zero-crossing of the sinc function */
	Mwing = (long)(floor((double)Nwing / (Num / Froll)) * (Num / Froll) + 0.5);
	if (Mwing == 0)
		return ( -4);

	ImpR = (double *) malloc(sizeof(double) * Mwing);

	/* Design a Nuttall or Kaiser windowed Sinc low-pass filter */
	LpFilter(ImpR, Mwing, Froll, Beta, Num);

	if (Normalize) { /* 'correct' the DC gain of the lowpass filter */
		long Dh;
		double DCgain;
		DCgain = 0;
		Dh = Num;			 /* Filter sampling period for factors>=1 */
		for (i = Dh; i < Mwing; i += Dh)
			DCgain += ImpR[i];
		DCgain = 2 * DCgain + ImpR[0];    /* DC gain of real coefficients */
		st_report("DCgain err=%.12f",DCgain-1.0);	// FIXME

		DCgain = 1.0 / DCgain;
		for (i = 0; i < Mwing; i++)
			Imp[i] = ImpR[i] * DCgain;

	} else {
		for (i = 0; i < Mwing; i++)
			Imp[i] = ImpR[i];
	}
	free(ImpR);
	for (i = Mwing; i <= Nwing; i++)
		Imp[i] = 0;
	/* Imp[Mwing] and Imp[-1] needed for quadratic interpolation */
	Imp[ -1] = Imp[1];

	return (Mwing);
}

/* LpFilter()
 *
 * reference: "Digital Filters, 2nd edition"
 *	      R.W. Hamming, pp. 178-179
 *
 * Izero() computes the 0th order modified bessel function of the first kind.
 *    (Needed to compute Kaiser window).
 *
 * LpFilter() computes the coeffs of a Kaiser-windowed low pass filter with
 *    the following characteristics:
 *
 *	 c[]  = array in which to store computed coeffs
 *	 frq  = roll-off frequency of filter
 *	 N    = Half the window length in number of coeffs
 *	 Beta = parameter of Kaiser window
 *	 Num  = number of coeffs before 1/frq
 *
 * Beta trades the rejection of the lowpass filter against the transition
 *    width from passband to stopband.	Larger Beta means a slower
 *    transition and greater stopband rejection.  See Rabiner and Gold
 *    (Theory and Application of DSP) under Kaiser windows for more about
 *    Beta.  The following table from Rabiner and Gold gives some feel
 *    for the effect of Beta:
 *
 * All ripples in dB, width of transition band = D*N where N = window length
 *
 *		 BETA	 D	 PB RIP	  SB RIP
 *		 2.120	 1.50  +-0.27	   -30
 *		 3.384	 2.23	 0.0864	   -40
 *		 4.538	 2.93	 0.0274	   -50
 *		 5.658	 3.62	 0.00868   -60
 *		 6.764	 4.32	 0.00275   -70
 *		 7.865	 5.0	 0.000868  -80
 *		 8.960	 5.7	 0.000275  -90
 *		 10.056	 6.4	 0.000087  -100
 */


#define IzeroEPSILON 1E-21		 /* Max error acceptable in Izero */

static double Izero(double x) {
	double sum, u, halfx, temp;
	long n;

	sum = u = n = 1;
	halfx = x / 2.0;
	do {
		temp = halfx / (double)n;
		n += 1;
		temp *= temp;
		u *= temp;
		sum += u;
	} while (u >= IzeroEPSILON*sum);
	return (sum);
}

static void LpFilter(double *c, long N, double frq, double Beta, long Num) {
	long i;

	/* Calculate filter coeffs: */
	c[0] = frq;
	for (i = 1; i < N; i++) {
		double x = M_PI * (double)i / (double)(Num);
		c[i] = sin(x * frq) / x;
	}

	if (Beta > 2) { /* Apply Kaiser window to filter coeffs: */
		double IBeta = 1.0 / Izero(Beta);
		for (i = 1; i < N; i++) {
			double x = (double)i / (double)(N);
			c[i] *= Izero(Beta * sqrt(1.0 - x * x)) * IBeta;
		}
	} else { /* Apply Nuttall window: */
		for (i = 0; i < N; i++) {
			double x = M_PI * i / N;
			c[i] *= 0.36335819 + 0.4891775 * cos(x) + 0.1365995 * cos(2 * x) + 0.0106411 * cos(3 * x);
		}
	}
}


#pragma mark -


class ResampleRateConverter : public RateConverter {
protected:
	eff_struct effp;
public:
	ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality);
	~ResampleRateConverter();
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
};


ResampleRateConverter::ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality) {
	// FIXME: quality is for now a nasty hack.
	// Valid values are 0,1,2,3 (everything else is treated like 0 for now)
	const char *arg = 0;
	switch (quality) {
	case 1: arg = "-qs"; break;
	case 2: arg = "-q"; break;
	case 3: arg = "-ql"; break;
	}
	st_resample_getopts(&effp, arg ? 1 : 0, &arg);
	st_resample_start(&effp, inrate, outrate);
}

ResampleRateConverter::~ResampleRateConverter() {
	st_resample_stop(&effp);
}

int ResampleRateConverter::flow(AudioInputStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
	return st_resample_flow(&effp, input, obuf, &osamp, vol);
}

int ResampleRateConverter::drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
	return st_resample_drain(&effp, obuf, &osamp, vol);
}

