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

#ifndef SOUND_RESAMPLE_H
#define SOUND_RESAMPLE_H

#include "sound/rate.h"


/* this Float MUST match that in filter.c */
#define Float double/*float*/

// From resample's stddef.h
typedef int16	HWORD;
typedef uint16	UHWORD;
typedef int32	WORD;
typedef uint32	UWORD;

#define MAX_HWORD (32767)
#define MIN_HWORD (-32768)


#define MAXNWING   8192


/* Private data for Lerp via LCM file */
typedef struct resamplestuff {
	double Factor;     /* Factor = Fout/Fin sample rates */
	int quadr;	      /* non-zero to use qprodUD quadratic interpolation */


	long Nq;

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


/** High quality rate conversion algorithm, based on SoX (http://sox.sourceforge.net). */
class ResampleRateConverter : public RateConverter {
protected:
	resamplestuff rstuff;

	int quadr;	      /* non-zero to use qprodUD quadratic interpolation */

    UHWORD LpScl;	/* Unity-gain scale factor */
    UHWORD Nwing;	/* Filter table size */
    UHWORD Nmult;	/* Filter length for up-conversions */
    HWORD Imp[MAXNWING];		/* Filter coefficients */
    HWORD ImpD[MAXNWING];	/* ImpD[n] = Imp[n+1]-Imp[n] */
	
	HWORD *X1, *Y1;
	HWORD *X2, *Y2;
	
	UWORD Time;	      /* Current time/pos in input sample */

public:
	ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality);
	~ResampleRateConverter();
	virtual int flow(AudioStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
	virtual int drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol);
};


#endif
