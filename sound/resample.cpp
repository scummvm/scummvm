
#include "stdafx.h"
#include <math.h>
#include "sound/resample.h"
#include "sound/audiostream.h"


#pragma mark -



/**
 * Calculates the filter coeffs for a Kaiser-windowed low-pass filter with a
 * given roll-off frequency. These coeffs are stored into a array of doubles.
 *
 * reference: "Digital Filters, 2nd edition"
 *            R.W. Hamming, pp. 178-179
 *
 * LpFilter() computes the coeffs of a Kaiser-windowed low pass filter with
 *    the following characteristics:
 *
 *       c[]  = array in which to store computed coeffs
 *       frq  = roll-off frequency of filter
 *       N    = Half the window length in number of coeffs
 *       Beta = parameter of Kaiser window
 *       Num  = number of coeffs before 1/frq
 *
 * Beta trades the rejection of the lowpass filter against the transition
 *    width from passband to stopband.  Larger Beta means a slower
 *    transition and greater stopband rejection.  See Rabiner and Gold
 *    (Theory and Application of DSP) under Kaiser windows for more about
 *    Beta.  The following table from Rabiner and Gold gives some feel
 *    for the effect of Beta:
 *
 * All ripples in dB, width of transition band = D*N where N = window length
 *
 *               BETA    D       PB RIP   SB RIP
 *               2.120   1.50  +-0.27      -30
 *               3.384   2.23    0.0864    -40
 *               4.538   2.93    0.0274    -50
 *               5.658   3.62    0.00868   -60
 *               6.764   4.32    0.00275   -70
 *               7.865   5.0     0.000868  -80
 *               8.960   5.7     0.000275  -90
 *               10.056  6.4     0.000087  -100
 */
static void LpFilter(double c[], int N, double frq, double Beta, int Num);

/**
 * Calls LpFilter() to create a filter, then scales the double coeffs into an
 * array of half words.
 * ERROR return codes:
 *    0 - no error
 *    1 - Nwing too large (Nwing is > MAXNWING)
 *    2 - Froll is not in interval [0:1)
 *    3 - Beta is < 1.0
 *    4 - LpScl will not fit in 16-bits
 */
static int makeFilter(HWORD Imp[], HWORD ImpD[], UHWORD *LpScl, UHWORD Nwing,
	       double Froll, double Beta);

static int32 FilterUp(HWORD Imp[], HWORD ImpD[], UHWORD Nwing, bool Interp,
	      HWORD *Xp, HWORD Inc, HWORD Ph);

static int32 FilterUD(HWORD Imp[], HWORD ImpD[], UHWORD Nwing, bool Interp,
	      HWORD *Xp, HWORD Ph, HWORD Inc, UHWORD dhb);



#pragma mark -


/*
 *
 * The configuration constants below govern
 * the number of bits in the input sample and filter coefficients, the 
 * number of bits to the right of the binary-point for fixed-point math, etc.
 *
 */

/* Conversion constants */
#define Nhc       8
#define Na        7
#define Np       (Nhc+Na)
#define Npc      (1<<Nhc)
#define Amask    ((1<<Na)-1)
#define Pmask    ((1<<Np)-1)
#define Nh       16
#define Nb       16
#define Nhxn     14
#define Nhg      (Nh-Nhxn)
#define NLpScl   13

/* Description of constants:
 *
 * Npc - is the number of look-up values available for the lowpass filter
 *    between the beginning of its impulse response and the "cutoff time"
 *    of the filter.  The cutoff time is defined as the reciprocal of the
 *    lowpass-filter cut off frequence in Hz.  For example, if the
 *    lowpass filter were a sinc function, Npc would be the index of the
 *    impulse-response lookup-table corresponding to the first zero-
 *    crossing of the sinc function.  (The inverse first zero-crossing
 *    time of a sinc function equals its nominal cutoff frequency in Hz.)
 *    Npc must be a power of 2 due to the details of the current
 *    implementation. The default value of 512 is sufficiently high that
 *    using linear interpolation to fill in between the table entries
 *    gives approximately 16-bit accuracy in filter coefficients.
 *
 * Nhc - is log base 2 of Npc.
 *
 * Na - is the number of bits devoted to linear interpolation of the
 *    filter coefficients.
 *
 * Np - is Na + Nhc, the number of bits to the right of the binary point
 *    in the integer "time" variable. To the left of the point, it indexes
 *    the input array (X), and to the right, it is interpreted as a number
 *    between 0 and 1 sample of the input X.  Np must be less than 16 in
 *    this implementation.
 *
 * Nh - is the number of bits in the filter coefficients. The sum of Nh and
 *    the number of bits in the input data (typically 16) cannot exceed 32.
 *    Thus Nh should be 16.  The largest filter coefficient should nearly
 *    fill 16 bits (32767).
 *
 * Nb - is the number of bits in the input data. The sum of Nb and Nh cannot
 *    exceed 32.
 *
 * Nhxn - is the number of bits to right shift after multiplying each input
 *    sample times a filter coefficient. It can be as great as Nh and as
 *    small as 0. Nhxn = Nh-2 gives 2 guard bits in the multiply-add
 *    accumulation.  If Nhxn=0, the accumulation will soon overflow 32 bits.
 *
 * Nhg - is the number of guard bits in mpy-add accumulation (equal to Nh-Nhxn)
 *
 * NLpScl - is the number of bits allocated to the unity-gain normalization
 *    factor.  The output of the lowpass filter is multiplied by LpScl and
 *    then right-shifted NLpScl bits. To avoid overflow, we must have 
 *    Nb+Nhg+NLpScl < 32.
 */


#pragma mark -


#define IBUFFSIZE 4096                         /* Input buffer size */

static inline HWORD WordToHword(int32 v, int scl)
{
    HWORD out;

    v = (v + (1 << (NLpScl-1))) >> NLpScl;	// Round & scale

    if (v>MAX_HWORD) {
        v = MAX_HWORD;
    } else if (v < MIN_HWORD) {
        v = MIN_HWORD;
    }   
    out = (HWORD) v;
    return out;
}

/* Sampling rate up-conversion only subroutine;
 * Slightly faster than down-conversion;
 */
static int SrcUp(HWORD X[], HWORD Y[], double factor, UWORD *Time,
                 UHWORD Nx, UHWORD Nwing, UHWORD LpScl,
                 HWORD Imp[], HWORD ImpD[], bool Interp)
{
    HWORD *Xp, *Ystart;
    int32 v;
    
    double dt;                  /* Step through input signal */ 
    UWORD dtb;                  /* Fixed-point version of Dt */
    UWORD endTime;              /* When Time reaches EndTime, return to user */
    
    dt = 1.0/factor;            /* Output sampling period */
    dtb = (UWORD)(dt*(1<<Np) + 0.5);     /* Fixed-point representation */
    
    Ystart = Y;
    endTime = *Time + (1<<Np)*(WORD)Nx;
    while (*Time < endTime)
    {
        Xp = &X[*Time>>Np];      /* Ptr to current input sample */
        /* Perform left-wing inner product */
        v = FilterUp(Imp, ImpD, Nwing, Interp, Xp, (HWORD)(*Time&Pmask),-1);
        /* Perform right-wing inner product */
        v += FilterUp(Imp, ImpD, Nwing, Interp, Xp+1, 
		      /* previous (triggers warning): (HWORD)((-*Time)&Pmask),1); */
                      (HWORD)((((*Time)^Pmask)+1)&Pmask),1);
        v >>= Nhg;              /* Make guard bits */
        v *= LpScl;             /* Normalize for unity filter gain */
        *Y++ = WordToHword(v,NLpScl);   /* strip guard bits, deposit output */
        *Time += dtb;           /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}


/* Sampling rate conversion subroutine */

static int SrcUD(HWORD X[], HWORD Y[], double factor, UWORD *Time,
                 UHWORD Nx, UHWORD Nwing, UHWORD LpScl,
                 HWORD Imp[], HWORD ImpD[], bool Interp)
{
    HWORD *Xp, *Ystart;
    int32 v;
    
    double dh;                  /* Step through filter impulse response */
    double dt;                  /* Step through input signal */
    UWORD endTime;              /* When Time reaches EndTime, return to user */
    UWORD dhb, dtb;             /* Fixed-point versions of Dh,Dt */
    
    dt = 1.0/factor;            /* Output sampling period */
    dtb = (UWORD)(dt*(1<<Np) + 0.5);     /* Fixed-point representation */
    
    dh = MIN((double)Npc, factor*Npc);  /* Filter sampling period */
    dhb = (UWORD)(dh*(1<<Na) + 0.5);     /* Fixed-point representation */
    
    Ystart = Y;
    endTime = *Time + (1<<Np)*(WORD)Nx;
    while (*Time < endTime)
    {
        Xp = &X[*Time>>Np];     /* Ptr to current input sample */
        v = FilterUD(Imp, ImpD, Nwing, Interp, Xp, (HWORD)(*Time&Pmask),
                     -1, dhb);  /* Perform left-wing inner product */
        v += FilterUD(Imp, ImpD, Nwing, Interp, Xp+1, 
		      /* previous (triggers warning): (HWORD)((-*Time)&Pmask), */
                      (HWORD)((((*Time)^Pmask)+1)&Pmask),
                      1, dhb);  /* Perform right-wing inner product */
        v >>= Nhg;              /* Make guard bits */
        v *= LpScl;             /* Normalize for unity filter gain */
        *Y++ = WordToHword(v,NLpScl);   /* strip guard bits, deposit output */
        *Time += dtb;           /* Move to next sample by time increment */
    }
    return (Y - Ystart);        /* Return the number of output samples */
}


#pragma mark -


#define IzeroEPSILON 1E-21               /* Max error acceptable in Izero */

static double Izero(double x)
{
   double sum, u, halfx, temp;
   int n;

   sum = u = n = 1;
   halfx = x/2.0;
   do {
      temp = halfx/(double)n;
      n += 1;
      temp *= temp;
      u *= temp;
      sum += u;
      } while (u >= IzeroEPSILON*sum);
   return(sum);
}


void LpFilter(double c[], int N, double frq, double Beta, int Num)
{
   double IBeta, temp, inm1;
   int i;

   /* Calculate ideal lowpass filter impulse response coefficients: */
   c[0] = 2.0*frq;
   for (i=1; i<N; i++) {
       temp = PI*(double)i/(double)Num;
       c[i] = sin(2.0*temp*frq)/temp; /* Analog sinc function, cutoff = frq */
   }

   /* 
    * Calculate and Apply Kaiser window to ideal lowpass filter.
    * Note: last window value is IBeta which is NOT zero.
    * You're supposed to really truncate the window here, not ramp
    * it to zero. This helps reduce the first sidelobe. 
    */
   IBeta = 1.0/Izero(Beta);
   inm1 = 1.0/((double)(N-1));
   for (i=1; i<N; i++) {
       temp = (double)i * inm1;
       c[i] *= Izero(Beta*sqrt(1.0-temp*temp)) * IBeta;
   }
}

static double ImpR[MAXNWING];

int makeFilter(HWORD Imp[], HWORD ImpD[], UHWORD *LpScl, UHWORD Nwing,
	       double Froll, double Beta)
{
   double DCgain, Scl, Maxh;
   HWORD Dh;
   int i, temp;

   if (Nwing > MAXNWING)                      /* Check for valid parameters */
      return(1);
   if ((Froll<=0) || (Froll>1))
      return(2);
   if (Beta < 1)
      return(3);

   /* 
    * Design Kaiser-windowed sinc-function low-pass filter 
    */
   LpFilter(ImpR, (int)Nwing, 0.5*Froll, Beta, Npc); 

   /* Compute the DC gain of the lowpass filter, and its maximum coefficient
    * magnitude. Scale the coefficients so that the maximum coeffiecient just
    * fits in Nh-bit fixed-point, and compute LpScl as the NLpScl-bit (signed)
    * scale factor which when multiplied by the output of the lowpass filter
    * gives unity gain. */
   DCgain = 0;
   Dh = Npc;                       /* Filter sampling period for factors>=1 */
   for (i=Dh; i<Nwing; i+=Dh)
      DCgain += ImpR[i];
   DCgain = 2*DCgain + ImpR[0];    /* DC gain of real coefficients */

   for (Maxh=i=0; i<Nwing; i++)
      Maxh = MAX(Maxh, fabs(ImpR[i]));

   Scl = ((1<<(Nh-1))-1)/Maxh;     /* Map largest coeff to 16-bit maximum */
   temp = (int)fabs((1<<(NLpScl+Nh))/(DCgain*Scl));
   if (temp >= 1<<16)
      return(4);                   /* Filter scale factor overflows UHWORD */
   *LpScl = temp;

   /* Scale filter coefficients for Nh bits and convert to integer */
   if (ImpR[0] < 0)                /* Need pos 1st value for LpScl storage */
      Scl = -Scl;
   for (i=0; i<Nwing; i++)         /* Scale them */
      ImpR[i] *= Scl;
   for (i=0; i<Nwing; i++)         /* Round them */
      Imp[i] = (HWORD)(ImpR[i] + 0.5);

   /* ImpD makes linear interpolation of the filter coefficients faster */
   for (i=0; i<Nwing-1; i++)
      ImpD[i] = Imp[i+1] - Imp[i];
   ImpD[Nwing-1] = - Imp[Nwing-1];      /* Last coeff. not interpolated */

   return(0);
}


#pragma mark -


int32 FilterUp(HWORD Imp[], HWORD ImpD[], 
		     UHWORD Nwing, bool Interp,
		     HWORD *Xp, HWORD Ph, HWORD Inc)
{
    HWORD *Hp, *Hdp = NULL, *End;
    HWORD a = 0;
    int32 v, t;
    
    v=0;
    Hp = &Imp[Ph>>Na];
    End = &Imp[Nwing];
    if (Interp) {
	Hdp = &ImpD[Ph>>Na];
	a = Ph & Amask;
    }
    if (Inc == 1)		/* If doing right wing...              */
    {				/* ...drop extra coeff, so when Ph is  */
	End--;			/*    0.5, we don't do too many mult's */
	if (Ph == 0)		/* If the phase is zero...           */
	{			/* ...then we've already skipped the */
	    Hp += Npc;		/*    first sample, so we must also  */
	    Hdp += Npc;		/*    skip ahead in Imp[] and ImpD[] */
	}
    }
    if (Interp)
      while (Hp < End) {
	  t = *Hp;		/* Get filter coeff */
	  t += (((WORD)*Hdp)*a)>>Na; /* t is now interp'd filter coeff */
	  Hdp += Npc;		/* Filter coeff differences step */
	  t *= *Xp;		/* Mult coeff by input sample */
	  if (t & (1<<(Nhxn-1)))  /* Round, if needed */
	    t += (1<<(Nhxn-1));
	  t >>= Nhxn;		/* Leave some guard bits, but come back some */
	  v += t;			/* The filter output */
	  Hp += Npc;		/* Filter coeff step */
	  Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      } 
    else 
      while (Hp < End) {
	  t = *Hp;		/* Get filter coeff */
	  t *= *Xp;		/* Mult coeff by input sample */
	  if (t & (1<<(Nhxn-1)))  /* Round, if needed */
	    t += (1<<(Nhxn-1));
	  t >>= Nhxn;		/* Leave some guard bits, but come back some */
	  v += t;			/* The filter output */
	  Hp += Npc;		/* Filter coeff step */
	  Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
    return(v);
}

int32 FilterUD( HWORD Imp[], HWORD ImpD[],
		     UHWORD Nwing, bool Interp,
		     HWORD *Xp, HWORD Ph, HWORD Inc, UHWORD dhb)
{
    HWORD a;
    HWORD *Hp, *Hdp, *End;
    WORD v, t;
    UWORD Ho;
    
    v=0;
    Ho = (Ph*(UWORD)dhb)>>Np;
    End = &Imp[Nwing];
    if (Inc == 1)		/* If doing right wing...              */
    {				/* ...drop extra coeff, so when Ph is  */
	End--;			/*    0.5, we don't do too many mult's */
	if (Ph == 0)		/* If the phase is zero...           */
	  Ho += dhb;		/* ...then we've already skipped the */
    }				/*    first sample, so we must also  */
				/*    skip ahead in Imp[] and ImpD[] */
    if (Interp)
      while ((Hp = &Imp[Ho>>Na]) < End) {
	  t = *Hp;		/* Get IR sample */
	  Hdp = &ImpD[Ho>>Na];  /* get interp (lower Na) bits from diff table*/
	  a = Ho & Amask;	/* a is logically between 0 and 1 */
	  t += (((WORD)*Hdp)*a)>>Na; /* t is now interp'd filter coeff */
	  t *= *Xp;		/* Mult coeff by input sample */
	  if (t & 1<<(Nhxn-1))	/* Round, if needed */
	    t += 1<<(Nhxn-1);
	  t >>= Nhxn;		/* Leave some guard bits, but come back some */
	  v += t;			/* The filter output */
	  Ho += dhb;		/* IR step */
	  Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
    else 
      while ((Hp = &Imp[Ho>>Na]) < End) {
	  t = *Hp;		/* Get IR sample */
	  t *= *Xp;		/* Mult coeff by input sample */
	  if (t & 1<<(Nhxn-1))	/* Round, if needed */
	    t += 1<<(Nhxn-1);
	  t >>= Nhxn;		/* Leave some guard bits, but come back some */
	  v += t;			/* The filter output */
	  Ho += dhb;		/* IR step */
	  Xp += Inc;		/* Input signal step. NO CHECK ON BOUNDS */
      }
    return(v);
}


#pragma mark -


#if 0
static int resampleWithFilter(  /* number of output samples returned */
    double factor,              /* factor = outSampleRate/inSampleRate */
    int infd,                   /* input and output file descriptors */
    int outfd,
    int inCount,                /* number of input samples to convert */
    int outCount,               /* number of output samples to compute */
    int nChans,                 /* number of sound channels (1 or 2) */
    bool interpFilt,            /* TRUE means interpolate filter coeffs */
    HWORD Imp[], HWORD ImpD[],
    UHWORD LpScl, UHWORD Nmult, UHWORD Nwing)
{
    UWORD Time, Time2;          /* Current time/pos in input sample */
    UHWORD Xp, Ncreep, Xoff, Xread;
    int OBUFFSIZE = (int)(((double)IBUFFSIZE)*factor+2.0);
    HWORD X1[IBUFFSIZE], Y1[OBUFFSIZE]; /* I/O buffers */
    HWORD X2[IBUFFSIZE], Y2[OBUFFSIZE]; /* I/O buffers */
    UHWORD Nout, Nx;
    int i, Ycount, last;
    
    MUS_SAMPLE_TYPE **obufs = sndlib_allocate_buffers(nChans, OBUFFSIZE);
    if (obufs == NULL)
        return err_ret("Can't allocate output buffers");

    /* Account for increased filter gain when using factors less than 1 */
    if (factor < 1)
      LpScl = LpScl*factor + 0.5;

    /* Calc reach of LP filter wing & give some creeping room */
    Xoff = ((Nmult+1)/2.0) * MAX(1.0,1.0/factor) + 10;

    if (IBUFFSIZE < 2*Xoff)      /* Check input buffer size */
      return err_ret("IBUFFSIZE (or factor) is too small");

    Nx = IBUFFSIZE - 2*Xoff;     /* # of samples to process each iteration */
    
    last = 0;                   /* Have not read last input sample yet */
    Ycount = 0;                 /* Current sample and length of output file */
    Xp = Xoff;                  /* Current "now"-sample pointer for input */
    Xread = Xoff;               /* Position in input array to read into */
    Time = (Xoff<<Np);          /* Current-time pointer for converter */
    
    for (i=0; i<Xoff; X1[i++]=0); /* Need Xoff zeros at begining of sample */
    for (i=0; i<Xoff; X2[i++]=0); /* Need Xoff zeros at begining of sample */
    
    do {
        if (!last)              /* If haven't read last sample yet */
        {
            last = readData(infd, inCount, X1, X2, IBUFFSIZE, 
                            nChans, (int)Xread);
            if (last && (last-Xoff<Nx)) { /* If last sample has been read... */
                Nx = last-Xoff; /* ...calc last sample affected by filter */
                if (Nx <= 0)
                  break;
            }
        }
        /* Resample stuff in input buffer */
        Time2 = Time;
        if (factor >= 1) {      /* SrcUp() is faster if we can use it */
            Nout=SrcUp(X1,Y1,factor,&Time,Nx,Nwing,LpScl,Imp,ImpD,interpFilt);
            if (nChans==2)
              Nout=SrcUp(X2,Y2,factor,&Time2,Nx,Nwing,LpScl,Imp,ImpD,
                         interpFilt);
        }
        else {
            Nout=SrcUD(X1,Y1,factor,&Time,Nx,Nwing,LpScl,Imp,ImpD,interpFilt);
            if (nChans==2)
              Nout=SrcUD(X2,Y2,factor,&Time2,Nx,Nwing,LpScl,Imp,ImpD,
                         interpFilt);
        }

        Time -= (Nx<<Np);       /* Move converter Nx samples back in time */
        Xp += Nx;               /* Advance by number of samples processed */
        Ncreep = (Time>>Np) - Xoff; /* Calc time accumulation in Time */
        if (Ncreep) {
            Time -= (Ncreep<<Np);    /* Remove time accumulation */
            Xp += Ncreep;            /* and add it to read pointer */
        }
        for (i=0; i<IBUFFSIZE-Xp+Xoff; i++) { /* Copy part of input signal */
            X1[i] = X1[i+Xp-Xoff]; /* that must be re-used */
            if (nChans==2)
              X2[i] = X2[i+Xp-Xoff]; /* that must be re-used */
        }
        if (last) {             /* If near end of sample... */
            last -= Xp;         /* ...keep track were it ends */
            if (!last)          /* Lengthen input by 1 sample if... */
              last++;           /* ...needed to keep flag TRUE */
        }
        Xread = i;              /* Pos in input buff to read new data into */
        Xp = Xoff;
        
        Ycount += Nout;
        if (Ycount>outCount) {
            Nout -= (Ycount-outCount);
            Ycount = outCount;
        }

        if (Nout > OBUFFSIZE) /* Check to see if output buff overflowed */
          return err_ret("Output array overflow");
        
        if (nChans==1) {
            for (i = 0; i < Nout; i++)
              obufs[0][i] = HWORD_TO_MUS_SAMPLE_TYPE(Y1[i]);
        } else {
            for (i = 0; i < Nout; i++) {
                obufs[0][i] = HWORD_TO_MUS_SAMPLE_TYPE(Y1[i]);
                obufs[1][i] = HWORD_TO_MUS_SAMPLE_TYPE(Y2[i]);
            }
        }
        /* NB: errors reported within sndlib */
        mus_file_write(outfd, 0, Nout - 1, nChans, obufs);

        printf(".");  fflush(stdout);

    } while (Ycount<outCount); /* Continue until done */

    return(Ycount);             /* Return # of samples in output file */
}
#endif


#pragma mark -


#if 0
/* here for linear interp.  might be useful for other things */
static st_rate_t st_gcd(st_rate_t a, st_rate_t b)
{
	if (b == 0)
		return a;
	else
		return st_gcd(b, a % b);
}


/*
 * Prepare processing.
 */
int st_resample_start(resample_t r, st_rate_t inrate, st_rate_t outrate) {
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
	i = makeFilter(r->Imp, r->Nwing, r->rolloff, r->beta, r->Nq);
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
int st_resample_flow(resample_t r, AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
	long i, k, last;
	long Nout = 0;	// The number of bytes we effectively output
	long Nx;		// The number of bytes we will read from input
	long Nproc;		// The number of bytes we process to generate Nout output bytes
	const long obufSize = *osamp;

/*
TODO: adjust for the changes made to AudioInputStream; add support for stereo
initially, could just average the left/right channel -> bad for quality of course,
but easiest to implement and would get this going again.
Next step is to duplicate the X/Y buffers... a lot of computations don't care about
how many channels there are anyway, they could just be ran twice, e.g. SrcEX and SrcUD.
But better for efficiency would be to rewrite those to deal with 2 channels, too.
Because esp in SrcEX/SrcUD, only very few computations depend on the input data,
and dealing with both channels in parallel should only be a little slower than dealing
with them alone
*/

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
int st_resample_drain(resample_t r, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
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
		rc = st_resample_flow(r, zero, Obuf, (st_size_t *) & Osamp, vol);
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
int st_resample_stop(resample_t r) {
	free(r->Imp - 1);
	free(r->X);
	/* free(r->Y); Y is in same block starting at X */
	return (ST_SUCCESS);
}

#endif

#pragma mark -


ResampleRateConverter::ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality) {
	// FIXME: quality is for now a nasty hack. Valid values are 0,1,2,3

	double rolloff;    /* roll-off frequency */
	double beta;	   /* passband/stopband tuning magic */

	switch (quality) {
	case 0:
		/* These defaults are conservative with respect to aliasing. */
		rolloff = 0.80;
		beta = 16;
		quadr = 0;
		Nmult = 45;
		break;
	case 1:
		rolloff = 0.80;
		beta = 16;
		quadr = 1;
		Nmult = 45;
		break;
	case 2:
		rolloff = 0.875;
		beta = 16;
		quadr = 1;
		Nmult = 75;
		break;
	case 3:
		rolloff = 0.94;
		beta = 16;
		quadr = 1;
		Nmult = 149;
		break;
	default:
		error("Illegal quality level %d\n", quality);
		break;
	}

	makeFilter(Imp, ImpD, &LpScl, Nmult, rolloff, beta);

    int OBUFFSIZE = (IBUFFSIZE * outrate / inrate + 2);
    X1 = (HWORD *)malloc(IBUFFSIZE);
    X2 = (HWORD *)malloc(IBUFFSIZE);
    Y1 = (HWORD *)malloc(OBUFFSIZE);
    Y2 = (HWORD *)malloc(OBUFFSIZE);
    
    // HACK this is invalid code but "fixes" a compiler warning for now
	double factor = outrate / (double)inrate;
	UHWORD Xp, /*Ncreep,*/ Xoff, Xread;
	UHWORD Nout, Nx;
	int Ycount, last;
	
	/* Account for increased filter gain when using factors less than 1 */
	if (factor < 1)
		LpScl = (UHWORD)(LpScl*factor + 0.5);
	
	/* Calc reach of LP filter wing & give some creeping room */
	Xoff = (UHWORD)(((Nmult+1)/2.0) * MAX(1.0,1.0/factor) + 10);
	
	if (IBUFFSIZE < 2*Xoff)      /* Check input buffer size */
		error("IBUFFSIZE (or factor) is too small");
	
	Nx = IBUFFSIZE - 2*Xoff;     /* # of samples to process each iteration */
	
	last = 0;                   /* Have not read last input sample yet */
	Ycount = 0;                 /* Current sample and length of output file */
	Xp = Xoff;                  /* Current "now"-sample pointer for input */
	Xread = Xoff;               /* Position in input array to read into */
	Time = (Xoff<<Np);          /* Current-time pointer for converter */
	
	Nout = SrcUp(X1, Y1, factor, &Time, Nx, Nwing, LpScl, Imp, ImpD, quadr != 0);
	Nout = SrcUD(X1, Y1, factor, &Time, Nx, Nwing, LpScl, Imp, ImpD, quadr != 0);

//	st_resample_start(&rstuff, inrate, outrate);
}

ResampleRateConverter::~ResampleRateConverter() {
//	st_resample_stop(&rstuff);
	free(X1);
	free(X2);
	free(Y1);
	free(Y2);
}

int ResampleRateConverter::flow(AudioInputStream &input, st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
//	return st_resample_flow(&rstuff, input, obuf, &osamp, vol);
	return 0;
}

int ResampleRateConverter::drain(st_sample_t *obuf, st_size_t osamp, st_volume_t vol) {
//	return st_resample_drain(&rstuff, obuf, &osamp, vol);
	return 0;
}

