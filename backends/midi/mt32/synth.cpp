/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 * Based on Tristan's conversion of Canadacow's code
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

#include "stdafx.h"

#include "backends/midi/mt32/synth.h"
#include "backends/midi/mt32/partial.h"
#include "backends/midi/mt32/freeverb.h"
#include "common/system.h"
#include "common/file.h"

#define NOMANSLAND

// **************************
//   CSynth reporting codes
// **************************

// files missing
#define ERR_PRESET1   1
#define ERR_PRESET2   2
#define ERR_DRUMPAT   3
#define ERR_PATCHLOG  4
#define ERR_MT32ROM   5

// HW spec
#define PRESENT_SSE       6
#define PRESENT_3DNOW     7
#define USING_SSE         8
#define USING_3DNOW       9

// General info
#define LCD_MESSAGE       10
#define DEV_RESET         11
#define DEV_RECONFIG      12
#define NEW_REVERB_MODE   13
#define NEW_REVERB_TIME   14
#define NEW_REVERB_LEVEL  15

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

static union mt32ramFormat {
	memParams params;
	memBanks patchabs;
	memAbsolute memabs;
	// System memory 10
	// Display 20
	// Reset 7F
} ALIGN_PACKED mt32ram, mt32default;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

int axtoi(char *str) {
	int result = 0;

	while (*str) {
		char hex = *str++;
		int digit;

		if (hex >= '0' && hex <= '9')
			digit = hex - '0';
		else if (hex >= 'a' && hex <= 'f')
			digit = 10 + hex - 'a';
		else if (hex >= 'A' && hex <= 'F')
			digit = 10 + hex - 'A';
		else
			break;

		result = (result << 4) | digit;
	}

	return result;
}

struct FILTER {
	unsigned int length;	// size of filter
	float *history;		// pointer to history in filter
	float *coef;		// pointer to coefficients of filter
};

#define FILTER_SECTIONS 2	// 2 filter sections for 24 db/oct filter

struct BIQUAD {
	double a0, a1, a2;	// numerator coefficients
	double b0, b1, b2;	// denominator coefficients
};

// Filter prototype coefficients, 1 for each filter section
BIQUAD ProtoCoef[FILTER_SECTIONS];

/**
 * Pre-warp the coefficients of a numerator or denominator. Note that a0 is
 * assumed to be 1, so there is no wrapping of it.
 */

void prewarp(double *a0, double *a1, double *a2, double fc, double fs) {
	double wp;

	wp = 2.0 * fs * tan(PI * fc / fs);

	*a2 = (*a2) / (wp * wp);
	*a1 = (*a1) / wp;
}

/*
 * ----------------------------------------------------------
 * bilinear()
 *
 * Transform the numerator and denominator coefficients
 * of s-domain biquad section into corresponding
 * z-domain coefficients.
 *
 *      Store the 4 IIR coefficients in array pointed by coef
 *      in following order:
 *             beta1, beta2    (denominator)
 *             alpha1, alpha2  (numerator)
 *
 * Arguments:
 *             a0-a2   - s-domain numerator coefficients
 *             b0-b2   - s-domain denominator coefficients
 *             k               - filter gain factor. initially set to 1
 *                                and modified by each biquad section in such
 *                                a way, as to make it the coefficient by
 *                                which to multiply the overall filter gain
 *                                in order to achieve a desired overall filter gain,
 *                                specified in initial value of k.
 *             fs             - sampling rate (Hz)
 *             coef    - array of z-domain coefficients to be filled in.
 *
 * Return:
 *             On return, set coef z-domain coefficients
 * ----------------------------------------------------------
 */
void bilinear(
    double a0, double a1, double a2,    /* numerator coefficients */
    double b0, double b1, double b2,    /* denominator coefficients */
    double *k,           /* overall gain factor */
    double fs,           /* sampling rate */
    float *coef         /* pointer to 4 iir coefficients */
)
{
    double ad, bd;

                 /* alpha (Numerator in s-domain) */
    ad = 4. * a2 * fs * fs + 2. * a1 * fs + a0;
                 /* beta (Denominator in s-domain) */
    bd = 4. * b2 * fs * fs + 2. * b1* fs + b0;

                 /* update gain constant for this section */
    *k *= ad/bd;

                 /* Denominator */
    *coef++ = (2. * b0 - 8. * b2 * fs * fs)
                           / bd;         /* beta1 */
    *coef++ = (4. * b2 * fs * fs - 2. * b1 * fs + b0)
                           / bd; /* beta2 */

                 /* Nominator */
    *coef++ = (2. * a0 - 8. * a2 * fs * fs)
                           / ad;         /* alpha1 */
    *coef = (4. * a2 * fs * fs - 2. * a1 * fs + a0)
                           / ad;   /* alpha2 */
}

void szxform(
    double *a0, double *a1, double *a2, /* numerator coefficients */
    double *b0, double *b1, double *b2, /* denominator coefficients */
    double fc,         /* Filter cutoff frequency */
    double fs,         /* sampling rate */
    double *k,         /* overall gain factor */
    float *coef)         /* pointer to 4 iir coefficients */
{
                 /* Calculate a1 and a2 and overwrite the original values */
        prewarp(a0, a1, a2, fc, fs);
        prewarp(b0, b1, b2, fc, fs);
        bilinear(*a0, *a1, *a2, *b0, *b1, *b2, k, fs, coef);
}



#ifdef HAVE_X86
#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
bool DetectSIMD()
{

	bool found_simd;
	_asm

	{

		pushfd
		pop eax // get EFLAGS into eax
		mov ebx,eax // keep a copy
		xor eax,0x200000
		// toggle CPUID bit

		push eax
		popfd // set new EFLAGS
		pushfd
		pop eax // EFLAGS back into eax

		xor eax,ebx
		// have we changed the ID bit?

		je NO_SIMD
		// No, no CPUID instruction

		// we could toggle the
		// ID bit so CPUID is present
		mov eax,1

		cpuid // get processor features
		test edx,1<<25 // check the SIMD bit
		jz NO_SIMD
		mov found_simd,1
		jmp DONE
		NO_SIMD:
		mov found_simd,0
		DONE:
	}

	return found_simd;

}

bool Detect3DNow() {
	bool found3D = false;
	__asm {
		pushfd
		pop eax
		mov edx, eax
		xor eax, 00200000h
		push eax
		popfd
		pushfd
		pop eax
		xor eax, edx
		jz NO_3DNOW

		mov eax, 80000000h
		cpuid

		cmp eax, 80000000h
		jbe NO_3DNOW

		mov eax, 80000001h
		cpuid
		test edx, 80000000h
		jz NO_3DNOW
		mov found3D, 1
NO_3DNOW:

	}
	return found3D;
}
#else
bool DetectSIMD()
{
	return atti386_DetectSIMD();
}
bool Detect3DNow()
{
	return atti386_Detect3DNow();
}
#endif
#endif

#ifdef NOMANSLAND

//#define SETRATE 32000
#define SETRATE myProp.SampleRate
//#define SETRATE 44100

// Used to regenerate waveform file after sampling rate change
#define MAKEWAVES 0

// Used to dump drum patches to syx file for viewing
#define DUMPDRUMS 0

#define USEREVERB 1

// Debuging stuff
// Shows the instruments played
#define DISPLAYINSTR 0
// Shows number of partials MT-32 is playing
#define MONITORPARTIALS 1
// Dump syx file of temp tibres right before reset
#define SAVECUSTOM 0


	
// Constant tuning for now
#define TUNING 440.0
#define SAMPLETUNING 207.64
#define MIDDLEC 60

#define ROMSIZE 512*1024
#define PCMSIZE ROMSIZE/2
#define GRAN 512
#define LN 2.30258509

#define MAXPOLY 64
#define MAXPARTIALS 32

// Reverb room sizes (in metres)

#define REV_ROOMSIZE 2.50f
#define REV_HALLSIZE 3.60f
#define REV_PLATESIZE 1.50f
#define REV_TAPSIZE 1.0f

// Reverb t60 coeff

#define REV_ROOMT60 ( REV_ROOMSIZE * REV_ROOMSIZE * REV_ROOMSIZE ) / 5
#define REV_HALLT60 ( REV_HALLSIZE * REV_HALLSIZE * REV_HALLSIZE ) / 5
#define REV_PLATET60 ( REV_PLATESIZE * REV_PLATESIZE * REV_PLATESIZE ) / 7
#define REV_TAPT60 ( REV_TAPSIZE * REV_TAPSIZE * REV_TAPSIZE ) / 1

//#define HLRATIO 2.0f

#define SYSEX_SIZE 512

// These are all the filters I tried without much success
int16 Moog1(int16 wg, float *hist, float usefilt, float res) {
	float f, p, q;             //filter coefficients
	float t1, t2;              //temporary buffers

	// Set coefficients given frequency & resonance [0.0...1.0]
	
	float frequency = usefilt;
	float in = (float)wg/32767.0;
	float resonance = res / 31.0;
	resonance = resonance * resonance;

	q = 1.0f - frequency;
	//p = frequency + 0.8f * frequency * q;
	p = frequency + 0.8f * frequency * q;
	
	f = p + p - 1.0f;
	q = resonance * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));

	// Filter (in [-1.0...+1.0])

	in -= q * hist[4];                          //feedback
	t1 = hist[1];
	hist[1] = (in + hist[0]) * p - hist[1] * f;
	t2 = hist[2];
	hist[2] = (hist[1] + t1) * p - hist[2] * f;
	t1 = hist[3];
	hist[3] = (hist[2] + t2) * p - hist[3] * f;
	hist[4] = (hist[3] + t1) * p - hist[4] * f;
	
	hist[4] = hist[4] - hist[4] * hist[4] * hist[4] * 0.166667f;    //clipping
	hist[0] = in;
	//LOG_MSG("In %d Hist: %f", wg, hist[4]*32767);
	return (int16)(hist[4]*32767.0);
}

int16 Moog2(int16 wg, float *hist, float usefilt, float resonance) {
	
  float res = resonance / 30.0;
  double f = usefilt;
  double invf = 1.0 - f;
  double fb = res * (1.0 - 0.15 * f * f);
  float input = (float)wg/32767.0;
  input -= hist[4] * fb;
  input *= 0.35013 * (f*f)*(f*f);
  hist[1] = input + 0.3 * hist[5] + (invf) * hist[1]; // Pole 1
  hist[5]  = input;
  hist[2] = hist[1] + 0.3 * hist[6] + (invf) * hist[2];  // Pole 2
  hist[6]  = hist[1];
  hist[3] = hist[2] + 0.3 * hist[7] + (invf) * hist[3];  // Pole 3
  hist[7]  = hist[2];
  hist[4] = hist[3] + 0.3 * hist[0] + (invf) * hist[4];  // Pole 4
  hist[0]  = hist[3];
  return (int16)(hist[4]*32767.0);
}

int16 simpleLowpass(int16 wg, float *hist, float usefilt, float resonance) {

	float in = (float)wg/32767.0;
	float res_lp = resonance / 31.0;
	res_lp = res_lp * res_lp;
	float cut_lp = usefilt;
	float n1, n2, fb_lp;

	n1 = hist[0];
	n2 = hist[1];

	fb_lp = res_lp+res_lp/(1-cut_lp);
	n1=n1+cut_lp*(in-n1+fb_lp*(n1-n2));
	n2=n2+cut_lp*(n1-n2);

	hist[0] = n1;
	hist[1] = n2;

	return (int)(n2*32767.0);

}

/* Reverb stuff */

#define NUM_COMBS 6

typedef struct {
	float coef;
	float lastval;
} LOWPASS_STATE;

typedef struct {
	float tau;
	float g;
	float gsqu;
	float *delbuf;
	int bufsiz;
	int bufpos;
} COMB_STATE;

typedef struct {
	int           lastsamp;
	int           cursamp;
	int           done;
	LOWPASS_STATE lowpass[NUM_COMBS];
	COMB_STATE    comb[NUM_COMBS];
	COMB_STATE    allpass[2];
} ST_REVERB;

class Reverb {
private:

	ST_REVERB *revstate;

	int SR;

public:

	Reverb(float t60, float hlratio, float dur, float hall_f, int smpr);

	~Reverb();

	void run(float *lchan, float *rchan, float revfrac);

	float lowpass(float input, LOWPASS_STATE *state);

	float lpcomb(float input, LOWPASS_STATE* lpstate, COMB_STATE* cstate);
	float allpassfilt(float input, COMB_STATE* state);
};

/*
t60    = reverb time
hlratio  = ratio of low freq t60 to high freq t60
dur    = duration of event/dealloc. on last samp
hall_fact= mult. factor for delay times
revstate =   running values for event reverb
*/
Reverb::Reverb(float t60, float hlratio, float dur, float hall_fact, int sampling_rate)
{
	revstate = new ST_REVERB;
	SR = sampling_rate;
	int i;
	float glow[NUM_COMBS], ghi[NUM_COMBS];
	/* initialize sample counter and compute last sample    */
	revstate->cursamp=0;
	revstate->lastsamp = (int)(dur*(float)SR);
	revstate->done=0;

	/* ALLPASS INITIALIZATIONS */
	revstate->allpass[0].tau = .006 * hall_fact;
	revstate->allpass[1].tau = .0065 * hall_fact;

	/* allocate allpass delay buffers and head/tail ptr.    */
	for(i=0; i<2; i++){
		revstate->allpass[i].bufsiz = (int) (revstate->allpass[i].tau*SR + .5);
		revstate->allpass[i].delbuf =
		new float[revstate->allpass[i].bufsiz];
		memset(revstate->allpass[i].delbuf, 0,
		revstate->allpass[i].bufsiz*sizeof(float));
		revstate->allpass[i].bufpos = -1;
	}

	revstate->allpass[0].g = .71f;
	revstate->allpass[1].g = .7f;

	revstate->allpass[0].gsqu =
	revstate->allpass[0].g * revstate->allpass[0].g;
	revstate->allpass[1].gsqu =
	revstate->allpass[1].g * revstate->allpass[1].g;

	/* COMB AND IIR LOWPASS FILTER INITIALIZATIONS */

	revstate->comb[0].tau = .0050 * hall_fact;
	revstate->comb[1].tau = .0068 * hall_fact;
	revstate->comb[2].tau = .0056 * hall_fact;
	revstate->comb[3].tau = .0072 * hall_fact;
	revstate->comb[4].tau = .0061 * hall_fact;
	revstate->comb[5].tau = .0078 * hall_fact;

	/* allocate comb delay buffers and head/tail ptr.       */
	for(i=0; i<NUM_COMBS; i++)  {
		revstate->comb[i].bufsiz = (int)(revstate->comb[i].tau * SR + .5);

		revstate->comb[i].delbuf =
		new float[revstate->comb[i].bufsiz];
		memset(revstate->comb[i].delbuf, 0,
		revstate->comb[i].bufsiz*sizeof(float));

		revstate->comb[i].bufpos = -1;

		revstate->lowpass[i].lastval = 0.;
	}

	/* if hlratio set by user, set various values            */
	if (hlratio != 0.)  {
		for(i=0; i<NUM_COMBS; i++)  {

		/* compute reverb attenuation factor for hi and low      */
		/* frequency reverberation times                         */
		glow[i] =
		pow(10.,(-3. * revstate->comb[i].tau) / t60);
		ghi[i]  =
		pow(10.,(-3. * revstate->comb[i].tau)/( t60 * hlratio));

		/* compute recursive lowpass factor and comb attenuation */
		/* factor to produce the correct reverberation time for  */
		/* both hi and low frequencies                           */
		revstate->lowpass[i].coef = (glow[i] - ghi[i])/(glow[i] + ghi[i]);
		revstate->comb[i].g = glow[i] * (1. - revstate->lowpass[i].coef);
		}
	}
	/* else, use default g's and coef's                      */
	else  {
		revstate->lowpass[0].coef = .24f; revstate->lowpass[1].coef = .26f;
		revstate->lowpass[2].coef = .28f; revstate->lowpass[3].coef = .29f;
		revstate->lowpass[4].coef = .30f; revstate->lowpass[5].coef = .32f;

		for(i=0; i<6; i++)  {

			/* compute reverb attenuation factor and comb      */
			/* attenuation factor based on default coef        */
			glow[i] =
			pow(10., (-3. * revstate->comb[i].tau) / t60);
			revstate->comb[i].g = glow[i] *
			(1. - revstate->lowpass[i].coef);
		}
	}
}

Reverb:: ~Reverb()
{
	int i;

	for(i=0; i<NUM_COMBS; i++) delete[] revstate->comb[i].delbuf;
	for(i=0; i<2; i++) delete[] revstate->allpass[i].delbuf;
	delete revstate;
}


INLINE void Reverb::run(float *lchan, float *rchan, float  revfrac)
	/*  lchan,rchan      non-reverberated input samples       */
	/*  revfrac           percent of output to be reverberated */
	{
	int i;
	float lchanrev, rchanrev, tot=0;

	//cout << " in run \n";

	if (revstate->done) {
		*lchan = 0.0;
		*rchan = 0.0;
		return;
	}

	for (i=0; i<NUM_COMBS; i++)
		tot = tot + lpcomb( (*lchan) + (*rchan),
		&(revstate->lowpass[i]),
		&(revstate->comb[i])
		);

	tot = tot/(float)NUM_COMBS;


	lchanrev = allpassfilt(tot * .7, &(revstate->allpass[0]));
	//  rchanrev = lchanrev ;
	rchanrev = allpassfilt(tot * .7, &(revstate->allpass[1]));

	if (revstate->cursamp == revstate->lastsamp)   {
		for(i=0; i<NUM_COMBS; i++) delete[] revstate->comb[i].delbuf;
		for(i=0; i<2; i++) delete[] revstate->allpass[i].delbuf;
		revstate->done = 1;
	}

	(revstate->cursamp)++;

	*lchan = lchanrev*revfrac + (*lchan)*(1. - revfrac) ;
	*rchan = rchanrev*revfrac + (*rchan)*(1. - revfrac) ;

	//*lchan = lchanrev*revfrac + (*lchan) ;
	//*rchan = rchanrev*revfrac + (*rchan) ;

	// cout << "lchan = \t" << *lchan <<endl;
	//cout << "rchan = \t" << *rchan <<endl;


}

INLINE float Reverb::lowpass(float input, LOWPASS_STATE *state)
{
	/* simple IIR lowpass filter algorithm              */
	/* y(n) = x(n) + coef * y(n-1)                      */
	state->lastval = (input + state->coef * state->lastval);
	return(state->lastval);
}

INLINE float Reverb::lpcomb(float input, LOWPASS_STATE *lpstate, COMB_STATE *cstate)
{

	float temp;

	/* move head-tail pointer in circular queue    */
	cstate->bufpos = (cstate->bufpos + 1) % cstate->bufsiz;

	/* pop circular queue                          */
	temp = cstate->delbuf[cstate->bufpos];

	/* add new value to end of queue               */
	lpstate->lastval = (cstate->delbuf[cstate->bufpos] + lpstate->coef * lpstate->lastval);

	cstate->delbuf[cstate->bufpos] =
	input +  cstate->g *
	//lowpass(cstate->delbuf[cstate->bufpos], lpstate);
	lpstate->lastval;

	/* return popped value                         */
	return(temp);

}

INLINE float Reverb::allpassfilt(float input, COMB_STATE* state)
{
	float temp;

	/* move head-tail pointer in circular queue          */
	state->bufpos = (state->bufpos + 1) % state->bufsiz;

	/* pop circular queue                                */
	temp = state->delbuf[state->bufpos];

	/* add new value to end of queue                     */
	state->delbuf[state->bufpos] = input +
	state->g * state->delbuf[state->bufpos];

	/* return a sum of the current in with the delay out */
	return(-1.* state->g * input + (1. - state->gsqu) * temp);

}

/* End reverb stuff */



/* Begin filter stuff */

void InitFilter(float fs, float fc, float *icoeff, float Q, float resfac) {
	
	float *coef;
	unsigned nInd;
	double   a0, a1, a2, b0, b1, b2;
	double   k;           /* overall gain factor */

	/* Section 1 */
        ProtoCoef[0].a0 = 1.0;
        ProtoCoef[0].a1 = 0;
        ProtoCoef[0].a2 = 0;
        ProtoCoef[0].b0 = 1.0;
        ProtoCoef[0].b1 = 0.765367;
        ProtoCoef[0].b2 = 1.0;


	/* Section 2 */
        ProtoCoef[1].a0 = 1.0;
        ProtoCoef[1].a1 = 0;
        ProtoCoef[1].a2 = 0;
        ProtoCoef[1].b0 = 1.0;
        ProtoCoef[1].b1 = 1.847759;
        ProtoCoef[1].b2 = 1.0;

      k = 1.5;          /* Set overall filter gain */
      coef = icoeff+1;     /* Skip k, or gain */

	for (nInd = 0; nInd < 2; nInd++)
    {
         a0 = ProtoCoef[nInd].a0;
         a1 = ProtoCoef[nInd].a1;
         a2 = ProtoCoef[nInd].a2;

         b0 = ProtoCoef[nInd].b0;
         b1 = ProtoCoef[nInd].b1 / Q;      /* Divide by resonance or Q
*/
         b2 = ProtoCoef[nInd].b2;
         szxform(&a0, &a1, &a2, &b0, &b1, &b2, fc, fs, &k, coef);
         coef += 4;                       /* Point to next filter
section */
	}
	icoeff[0] = k;
}


#if FILTER_FLOAT == 1

iir_filter_type usefilter;

#if defined(WIN32) && !(defined(__CYGWIN__) || defined(__MINGW32__))
float iir_filter_sse(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
    float *hist2_ptr;
    float output;

    hist2_ptr = hist1_ptr + 1;           /* next history */

        /* 1st number of coefficients array is overall input scale factor,
         * or filter gain */
    output = (input) * (*coef_ptr++);

	__asm {

		movss xmm1,	output

		mov eax, coef_ptr
		movups xmm2, [eax]

		mov eax, hist1_ptr
		movlps xmm3, [eax]
		shufps xmm3, xmm3, 44h
		// hist2_ptr, hist1_ptr, hist2_ptr, hist1_ptr

		mulps xmm2, xmm3

		subss xmm1, xmm2
		// Rotate elements right
		shufps xmm2, xmm2, 39h
		subss xmm1, xmm2
		
		// Store new_hist
		movss DWORD PTR [eax], xmm1

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Store previous hist
		movss DWORD PTR [eax+4], xmm3

		add coef_ptr, 16
		add hist1_ptr, 8

		mov eax, coef_ptr
		movups xmm2, [eax]

		mov eax, hist1_ptr
		movlps xmm3, [eax]
		shufps xmm3, xmm3, 44h
		// hist2_ptr, hist1_ptr, hist2_ptr, hist1_ptr

		mulps xmm2, xmm3

		subss xmm1, xmm2
		// Rotate elements right
		shufps xmm2, xmm2, 39h
		subss xmm1, xmm2
		
		// Store new_hist
		movss DWORD PTR [eax], xmm1

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Rotate elements right
		shufps xmm2, xmm2, 39h
		addss xmm1, xmm2

		// Store previous hist
		movss DWORD PTR [eax+4], xmm3

		movss output, xmm1
	}

	output *= ResonInv[revLevel];

    return(output);

}

float iir_filter_3dnow(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
    float *hist2_ptr;
    float output;
	float tmp;

    hist2_ptr = hist1_ptr + 1;           /* next history */

        /* 1st number of coefficients array is overall input scale factor,
         * or filter gain */
    output = (input) * (*coef_ptr++);

	// I find it very sad that 3DNow requires twice as many instructions as Intel's SSE
	// Intel does have the upper hand here.
	__asm {
		movq mm1, output
		mov ebx, coef_ptr
		movq mm2, [ebx]
		
		mov eax, hist1_ptr;
		movq mm3, [eax]

		pfmul mm2, mm3
		pfsub mm1, mm2

		psrlq mm2, 32
		pfsub mm1, mm2

		// Store new hist
		movd tmp, mm1

		add ebx, 8
		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfadd mm1, mm2

		psrlq mm2, 32
		pfadd mm1, mm2

		push tmp
		pop DWORD PTR [eax]

		movd DWORD PTR [eax+4], mm3

		add ebx, 8
		add eax, 8

		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfsub mm1, mm2

		psrlq mm2, 32
		pfsub mm1, mm2

		// Store new hist
		movd tmp, mm1

		add ebx, 8
		movq mm2, [ebx]
		movq mm3, [eax]

		pfmul mm2, mm3
		pfadd mm1, mm2

		psrlq mm2, 32
		pfadd mm1, mm2

		push tmp
		pop DWORD PTR [eax]
		movd DWORD PTR [eax+4], mm3

		movd output, mm1

		femms
	}

	output *= ResonInv[revLevel];

    return(output);
}
#else

#ifdef HAVE_X86
float iir_filter_sse(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
	float *hist2_ptr;
	float output;

	hist2_ptr = hist1_ptr + 1;           /* next history */
	
        /* 1st number of coefficients array is overall input scale factor,
         * or filter gain */
	output = (input) * (*coef_ptr++);

	output = atti386_iir_filter_sse(&output, hist1_ptr, coef_ptr);	
	output *= ResonInv[revLevel];

	return(output);
}

float iir_filter_3dnow(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
	float *hist2_ptr;
	float output;
	
	hist2_ptr = hist1_ptr + 1;           /* next history */

        /* 1st number of coefficients array is overall input scale factor,
         * or filter gain */
	output = (input) * (*coef_ptr++);
	
	output = atti386_iir_filter_3DNow(output, hist1_ptr, coef_ptr);	
		
	output *= ResonInv[revLevel];

    return(output);
}
#endif

#endif

float iir_filter_normal(float input,float *hist1_ptr, float *coef_ptr, int revLevel)
{
    float *hist2_ptr;
    float output,new_hist;

    hist2_ptr = hist1_ptr + 1;           /* next history */

        /* 1st number of coefficients array is overall input scale factor,
         * or filter gain */
    output = (input) * (*coef_ptr++);

    output = output - *hist1_ptr * (*coef_ptr++);
    new_hist = output - *hist2_ptr * (*coef_ptr++);    /* poles */

    output = new_hist + *hist1_ptr * (*coef_ptr++);
    output = output + *hist2_ptr * (*coef_ptr++);      /* zeros */

    *hist2_ptr++ = *hist1_ptr;
    *hist1_ptr++ = new_hist;
    hist1_ptr++;
    hist2_ptr++;

	// i = 1
    output = output - *hist1_ptr * (*coef_ptr++);
    new_hist = output - *hist2_ptr * (*coef_ptr++);    /* poles */

    output = new_hist + *hist1_ptr * (*coef_ptr++);
    output = output + *hist2_ptr * (*coef_ptr++);      /* zeros */
	
    *hist2_ptr++ = *hist1_ptr;
    *hist1_ptr++ = new_hist;

	output *= ResonInv[revLevel];

    return(output);
}

#endif

#if FILTER_64BIT == 1
// 64-bit version
long iir_filter(long input, int64 *hist1_ptr, int64 *coef_ptr)
{
    unsigned int i;
    int64 *hist2_ptr;
	int64 output,new_hist,history1,history2;

    hist2_ptr = hist1_ptr + 1;           // next history

    // 1st number of coefficients array is overall input scale factor,
    // or filter gain
    output = (input * (*coef_ptr++));

    for (i = 0 ; i < 2; i++)
        {
        history1 = *hist1_ptr;           // history values
        history2 = *hist2_ptr;

        output = output - ((history1 * (*coef_ptr++))>>20);
        new_hist = output - ((history2 * (*coef_ptr++))>>20);    // poles

        output = new_hist + ((history1 * (*coef_ptr++))>>20);
        output = output + ((history2 * (*coef_ptr++))>>20);      // zeros

        *hist2_ptr++ = *hist1_ptr;
        *hist1_ptr++ = new_hist;
        hist1_ptr++;
        hist2_ptr++;
    }

    return(output>>20);
}

#endif

#if FILTER_INT == 1
long iir_filter(long input, signed long *hist1_ptr, signed long *coef_ptr)
{
    unsigned int i;
    signed long *hist2_ptr;
	signed long output,new_hist,history1,history2;

    hist2_ptr = hist1_ptr + 1;           // next history

    // 1st number of coefficients array is overall input scale factor,
    // or filter gain
    output = (input * (*coef_ptr++));

    for (i = 0 ; i < 2; i++)
        {
        history1 = *hist1_ptr;           // history values
        history2 = *hist2_ptr;

        output = output - ((history1 * (*coef_ptr++))>>10);
        new_hist = output - ((history2 * (*coef_ptr++))>>10);    // poles

        output = new_hist + ((history1 * (*coef_ptr++))>>10);
        output = output + ((history2 * (*coef_ptr++))>>10);      // zeros

        *hist2_ptr++ = *hist1_ptr;
        *hist1_ptr++ = new_hist;
        hist1_ptr++;
        hist2_ptr++;
    }

    return(output>>10);
}
#endif

/* end filter stuff */

partialFormat PCM[54];
partialTable PCMList[128];
uint32 PCMReassign[55];
int32 PCMLoopTable[55];

timbreParam drums[30];

int16 romfile[PCMSIZE+GRAN]; // 256K
static int16 chantable[32]; // 64 bytes
static int16 miditable[9]; // 18 bytes

static CPartialMT32 *partTable[MAXPARTIALS];
static int32 PartialReserveTable[32];

// For debuging partial allocation
//static FILE *pInfo;
struct partUsage {
	int active[32];
	int assign[32];
	int owner[32];
	int status[32];
};

static int32 activeChannels;

// Some optimization stuff
int32 divtable[256];			// 1K
int32 smalldivtable[256];		// 1K
static int16 freqtable[256];			// 512 bytes
static uint32 sqrtable[101];			// 404 bytes
static int32 keytable[256];			// 1K
static uint32 wavtable[256];			// 1K
uint32 wavtabler[64][256];		// 64K
uint32 looptabler[16][16][256];	// 256K
static uint32 drumdelta[256];			// 1K
int16 sintable[65536];			// 128K
static int32 ptable[101];				// 404 bytes
static int32 lfotable[101];			// 404 bytes
int32 penvtable[16][128];		// 8K
static int32 fildeptable[5][128];		// 3K
static int32 timekeytable[5][128];		// 3K
static int32 filveltable[128][128];	// 64K
static int32 veltkeytable[5][128];		// 3K
int32 pulsetable[101];			// 400 bytes
int32 pulseoffset[101];			// 400 bytes
int32 sawtable[128][128];		// 64K
static int32 restable[201];			// 804 bytes
//static int32 biastable[13];			// 56 bytes
static int32 ampbiastable[16][128];	// 8K
static int32 fbiastable[16][128];		// 8K
static int filttable[2][128][256];		// 256K
static int nfilttable[128][128][128];           // 64K
float filtcoeff[FILTERGRAN][32][16];	// 512K - hmmm
#if FILTER_64BIT == 1
static int64 filtcoefffix[FILTERGRAN][32][16];
#endif
#if FILTER_INT == 1
static int32 filtcoefffix[FILTERGRAN][32][16];
#endif
static float revtable[8];				// 16 bytes
static int32 finetable[201];			// 804 bytes
uint32 lfoptable[101][128];		// 32K
int32 ampveltable[128][64];		// 32K
int32 pwveltable[15][128];
static int32 envtimetable[101];		// 404 bytes
static int32 decaytimetable[101];		// 404 bytes
static int32 lasttimetable[101];		// 404 bytes
int32 amptable[129];			// 516 bytes
static int32 voltable[129];			// 516 bytes
static float padjtable[51];				// 204 bytes
static int32 bendtable[49];			// 195 bytes
float ResonFactor[32];
float ResonInv[32];

int16 smallnoise[441]; // 4410 bytes at 44Khz
int32 samplepos = 0;

int16* waveforms[4][256];		// 2K
uint32  waveformsize[4][256];
int16 tmpforms[4][65536];				// 128K
int16 finalforms[4][8192];				// 64K

// Corresponding drum patches as matched to keyboard
int8 DrumTable[42] = {
    0, 0, 10, 1, 11, 5, 4, 6, 4, 29, 3, 7, 3, 2, 8, 2, 9, -1, -1, 22,
	-1, 12, -1, -1, -1, 18, 19, 13, 14, 15, 16, 17, 20, 21, 27, 24,
	26, 25, 28, -1, 23, -1 };

// Pan-pot position of drums
int16 drmPanTable[42] = {
    64, 64, 72, 64,  48, 72, 24, 72, 24, 72, 48, 72, 48, 96, 72, 96, 48,  1,  1,  40,
	1,  64,  1,  1,  1,  104, 88,  48,  40,  32,  64,  80, 104 ,  104,  40,  88,
    40,  40,  32,  1,  16,  1 };

uint8 PartialStruct[13] = {
	0, 0, 2, 2, 1, 3,
	3, 0, 3, 0, 2, 1, 3 };

uint8 PartMixStruct[13] = {
	0, 1, 0, 1, 1, 0,
	1, 3, 3, 2, 2, 2, 2 };

uint8 InitInstr[8] = {
	68, 48, 95, 78, 41, 3, 110, 122};

int8 LoopPatterns[16][16] = {
	{ 2,3,4,5,6,7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 8,9,10,11,12,13,14,15,16,-1,-1,-1,-1,-1,-1,-1 },
	{ 17,18,19,20,21,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 22,23,24,25,26,27,28,29,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 30,31,32,33,34,35,36,37,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 45,46,47,48,49,50,51,52,53,-1,-1,-1,-1,-1,-1,-1 },
	{ 15,11,12,13,14,15,16,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 30,35,32,33,34,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 2,3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }
};


int32 LoopPatternTuning[16][16] = {
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A, 0x1294A,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A, 0x1294A,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A, 0x1294A,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A, 0x1294A,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A, 0x1294A,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x2590B,0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,0x1294A,0x1294A,0x1294A,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ 0x1294A,0x1294A,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 },
	{ -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1 }

};


// These are division constants for the TVF depth key follow
uint32 depexp[5] = {3000,950,485,255,138};

//Amplitude time velocity follow exponential coefficients
double tvcatconst[5] = {0.0, 0.002791309, 0.005942882, 0.012652792, 0.026938637};
double tvcatmult[5] = {1.0, 1.072662811, 1.169129367, 1.288579123, 1.229630539};

//Envelope time keyfollow exponential coefficients
double tkcatconst[5] = {0.0, 0.005853144, 0.011148054, 0.019086143, 0.043333215};
double tkcatmult[5] = {1.0, 1.058245688, 1.048488989, 1.016049301, 1.097538067};

// This caches the drum information
patchCache drumCache[30][4];
int drumPan[30][2];

bool isEnabled;


Reverb *myReverb;
revmodel *newReverb;
bool usingSIMD;

int16 mastervolume;

uint32 curRevMode;
uint32 curRevTime;
uint32 curRevLevel;

uint32 partialsPlayed; // Variable containing the whole count of partials played
uint32 avgPartials;	   // Tally of average number of partials a second
int32 partialChan[9]; // The count of partials played per channel

#if SAVECUSTOM == 1
uint32 filenum = 0;
#endif

/*
t60    = reverb time
hlratio  = ratio of low freq t60 to high freq t60
dur    = duration of event/dealloc. on last samp
hall_fact= mult. factor for delay times
revstate =   running values for event reverb
*/

// t60 follows standard sabine equation
// t60 = .049 * (V / A)
// Where V = is the volume of the enclosure in cubic feet
// And A is the absorbtion of the room in square feet
// Using metric measurements (metres) the .049 is replaced with 0.161

		

void InitReverb(uint32 newRevMode, uint32 newRevTime, uint32 sampRate) {
	if(newReverb != NULL) delete newReverb;
	newReverb = new revmodel();
	
	/*
	if(myReverb != NULL) delete myReverb;

	float t60;
	switch(newRevMode) {
	case 0:
		t60 = .161f * REV_ROOMT60;
		break;
	case 1:
		t60 = .161f * REV_HALLT60;
		break;
	case 2:
		t60 = .161f * REV_PLATET60;
		break;
	case 3:
		t60 = .161f * REV_TAPT60;
		break;
	default:
		t60 = .161f * REV_ROOMT60;
		break;
	}

	t60 = t60 * 0.625;
	myReverb = new Reverb(t60,HLRATIO,sampRate/(8/(newRevTime+1)),(newRevTime+1),sampRate);
	*/
	curRevTime = newRevTime;
	curRevMode = newRevMode;

	switch(newRevMode) {
	case 0:
		newReverb->setroomsize((float).1);
		newReverb->setdamp((float).75);
		break;
	case 1:
		newReverb->setroomsize((float).5);
		newReverb->setdamp((float).5);
		break;
	case 2:
		newReverb->setroomsize((float).5);
		newReverb->setdamp((float).1);
		break;
	case 3:
		newReverb->setroomsize((float)1);
		newReverb->setdamp((float).75);
		break;
	default:
		newReverb->setroomsize((float).1);
		newReverb->setdamp((float).5);
		break;
	}
	newReverb->setdry(1);
	newReverb->setwet((float)mt32ram.params.system.reverbLevel / 8.0);
	newReverb->setwidth((float)curRevTime / 8.0);


}

class MidiChannel {
public:
	bool isPlaying;

	volset volumesetting;

	int reverbat, reverbleft, reverbright;
	int panpot;

	patchCache pcache[4];

	uint32 bend;
	int32 volume;

	dpoly polyTable[MAXPOLY];


private:
	bool isRy;
	int sampRate;
	int freq;
	int channum;
	int partialCount;
	int velocity;
	long sndoff;
	int octave;
	int note;

	int patch;
	char currentInstr[20];

	int keyshift;
	int assignmode;
	int storedpatch;
	bool sustain;
	bool init;

	uint32 P1Mix;
	uint32 P2Mix;

	bool holdpedal;


public:
	MidiChannel(int samp, int cnum);
	//short getSample(short *lspecial, short *rspecial);
	void PlayPoly(int m, int f,int vel);
	void PlayDrum(int m, int f,int vel);
	void InitKeyFollow(dpoly *tpoly);
	void PlayNote(int f, int vel);
	void StopNote(int f, int vel);
	void AllStop();
	void SetVolume(int vol);
	void SetPan(int vol);
	void SetBend(int vol);
	void SetModulation(int vol);
	void SetPatch(int patchnum,int drumNum);
	void SetHoldPedal(bool pedalval);
	void StopPedalHold();
	void RefreshPatch();
	void CheckNoteList();

	int FixKeyfollow(int srckey, int *dir);
	int FixBiaslevel(int srcpnt, int *dir);

	//int32 getPitchEnvelope(dpoly::partialStatus *pStat, dpoly *poly, bool inDecay);
	//int32 getAmpEnvelope(dpoly::partialStatus *pStat, dpoly *poly, bool inDecay);
	//int32 getFiltEnvelope(int16 wg, dpoly::partialStatus *pStat, dpoly *poly, bool inDecay);

	//void StartDecay(int envnum, int32 startval, dpoly::partialStatus *pStat, dpoly *poly);

};

void MidiChannel::SetHoldPedal(bool pedalval) {
	holdpedal = pedalval;
}

void MidiChannel::SetBend(int vol) {

	//int tmpbend = ((vol - 0x2000) * (int)mt32ram.params.pSettings[storedpatch].benderRange) >> 13;
	//bend = bendtable[tmpbend+24];
	
	float bend_range = (float)mt32ram.params.pSettings[storedpatch].benderRange / 24;
	bend = 4096 + (int)((float)(vol - 8192) * bend_range);	
}

void MidiChannel::SetModulation(int vol) {

	// Just a bloody guess, as always, before I get things figured out
	int t;
	for(t=0;t<4;t++) {
		if(pcache[t].playPartial) {
			int newrate = (pcache[t].modsense * vol) >> 7;
			//pcache[t].lfoperiod = lfotable[newrate];
			pcache[t].lfodepth = newrate;
		}
	}

}

INLINE void StartDecay(int envnum, int32 startval, dpoly::partialStatus *pStat, dpoly *poly) {

	patchCache *tcache = pStat->tcache;
	dpoly::partialStatus::envstatus *tStat  = &pStat->envs[envnum];
	
	tStat->sustaining = false;
	tStat->decaying = true;
	tStat->envpos = 0;
	tStat->envbase = startval;

	switch(envnum) {
	case AMPENV:
		tStat->envsize = (decaytimetable[(int)tcache->ampEnv.envtime[4]] * timekeytable[(int)tcache->ampEnv.envtkf][poly->freqnum]) >> 8;
		tStat->envdist = -startval;
		break;
	case FILTENV:
		tStat->envsize = (decaytimetable[(int)tcache->filtEnv.envtime[4]] * timekeytable[(int)tcache->filtEnv.envtkf][poly->freqnum]) >> 8;
		tStat->envdist = -startval;
		break;
	case PITCHENV:
		tStat->envsize = (decaytimetable[(int)tcache->pitchEnv.time[3]] * timekeytable[(int)tcache->pitchEnv.timekeyfollow][poly->freqnum]) >> 8 ;
		tStat->envdist = tcache->pitchEnv.level[4] - startval;
		break;
	default:
		break;
	}
	tStat->envsize++;


}





INLINE int32 getAmpEnvelope(dpoly::partialStatus *pStat, dpoly *poly) {
	int32 tc;

	patchCache *tcache = pStat->tcache;
	dpoly::partialStatus::envstatus *tStat  = &pStat->envs[AMPENV];

	if(!pStat->playPartial) return 0;

	if(tStat->decaying) {

		if(pStat->isDecayed) {
			pStat->playPartial = false;

			tc = 0;
		} else {
			tc = tStat->envbase;
			tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
			if (tc < 0)
				tc = 0;
			if((tStat->envpos >= tStat->envsize) || (tc == 0)){
				pStat->PCMDone = true;
				pStat->isDecayed = true;
				pStat->playPartial = false;
			}
		}

	} else {

		if((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
			if(tStat->envstat==-1) {
				tStat->envbase = 0;
			} else {
				tStat->envbase = tcache->ampEnv.envlevel[tStat->envstat];
			}
			tStat->envstat++;
			tStat->envpos = 0;

			switch(tStat->envstat) {
			case 0:
				//Spot for velocity time follow
				//Only used for first attack
				tStat->envsize = (envtimetable[(int)tcache->ampEnv.envtime[tStat->envstat]] * veltkeytable[(int)tcache->ampEnv.envvkf][poly->vel]) >> 8;
				//LOG_MSG("Envstat %d, size %d, %d %d", tStat->envstat, tStat->envsize, tcache->ampEnv.envtime[tStat->envstat], veltkeytable[tcache->ampEnv.envvkf][poly->vel]);
				break;
			case 3:
				// Final attack envelope uses same time table as the decay
				//tStat->envsize = decaytimetable[tcache->ampEnv.envtime[tStat->envstat]];
				tStat->envsize = lasttimetable[(int)tcache->ampEnv.envtime[(int)tStat->envstat]];
				//LOG_MSG("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			case 4:
				//LOG_MSG("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				tc =tcache->ampsustain;
				if(!poly->sustain)
					StartDecay(AMPENV, tc, pStat, poly);
				else
					tStat->sustaining = true;

				goto PastCalc;
			default:
				//Spot for timekey follow
				//Only used in subsquent envelope parameters, including the decay
				tStat->envsize = (envtimetable[(int)tcache->ampEnv.envtime[tStat->envstat]] * timekeytable[(int)tcache->ampEnv.envtkf][poly->freqnum]) >> 8;

				//LOG_MSG("Envstat %d, size %d", tStat->envstat, tStat->envsize);
				break;
			}

			tStat->envsize++;
			tStat->envdist = tcache->ampEnv.envlevel[tStat->envstat] - tStat->envbase;

			if(tStat->envdist != 0) {
				tStat->counter = abs(tStat->envsize / tStat->envdist);
				//LOG_MSG("Pos %d, envsize %d envdist %d", tStat->envstat, tStat->envsize, tStat->envdist);
			} else {
				tStat->counter = 0;
				//LOG_MSG("Pos %d, envsize %d envdist %d", tStat->envstat, tStat->envsize, tStat->envdist);
			}
		}

		tc = tStat->envbase;
		tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));
		tStat->count = tStat->counter;


PastCalc:
		
		tc = (tc * (int32)tcache->amplevel) >> 7;

	}
		
	// Prevlevel storage is bottle neck
	tStat->prevlevel = tc;

	//Bias level crap stuff now
	//I unrolled the loop

	int dist,bias;

	// Bias 1
	if(tcache->ampblevel[0]!=0) {
		bias = tcache->ampbias[0];
		if(tcache->ampdir[0]==0) {
			// < Bias
			if(poly->freqnum < bias) {
				dist = bias-poly->freqnum;
				tc = (tc * ampbiastable[tcache->ampblevel[0]][dist]) >> 8;

			}
		} else {
			// > Bias
			if(poly->freqnum > bias) {
				dist = poly->freqnum-bias;
				tc = (tc * ampbiastable[tcache->ampblevel[0]][dist]) >> 8;
			}
		}
	}
	
	//Bias 2
	if(tcache->ampblevel[1]!=0) {
		bias = tcache->ampbias[1];
		if(tcache->ampdir[1]==0) {
			// < Bias
			if(poly->freqnum < bias) {
				dist = bias-poly->freqnum;
				tc = (tc * ampbiastable[tcache->ampblevel[1]][dist]) >> 8;

			}
		} else {
			// > Bias
			if(poly->freqnum > bias) {
				dist = poly->freqnum-bias;
				tc = (tc * ampbiastable[tcache->ampblevel[1]][dist]) >> 8;
			}
		}
	}



	return tc;
}


INLINE int32 getPitchEnvelope(dpoly::partialStatus *pStat, dpoly *poly) {
	patchCache *tcache = pStat->tcache;
	dpoly::partialStatus::envstatus *tStat  = &pStat->envs[PITCHENV];

	int32 tc;
	pStat->pitchsustain = false;
	if(tStat->decaying) {

		if((pStat->isDecayed) || (tStat->envpos >= tStat->envsize)) {
			tc = tcache->pitchEnv.level[4];
		} else {
			tc = tStat->envbase;
			tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));		
		}
	} else {

		if(tStat->envstat==3) {
			tc =tcache->pitchsustain;
			if(poly->sustain) {
				pStat->pitchsustain = true;
			} else {
				StartDecay(PITCHENV, tc, pStat, poly);
			}

		} else {

			if((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
				tStat->envstat++;

				tStat->envbase = tcache->pitchEnv.level[tStat->envstat];
				tStat->envsize = (envtimetable[(int)tcache->pitchEnv.time[tStat->envstat]] * timekeytable[(int)tcache->pitchEnv.timekeyfollow][poly->freqnum]) >> 8;


				tStat->envpos = 0;
				tStat->envsize++;
				tStat->envdist = tcache->pitchEnv.level[tStat->envstat+1] - tStat->envbase;
			}

			tc = tStat->envbase;
			tc = (tc + ((tStat->envdist * tStat->envpos) / tStat->envsize));

		}
		tStat->prevlevel = tc;


	}
	
	return tc;

}


INLINE int32 getFiltEnvelope(int16 wg, dpoly::partialStatus *pStat, dpoly *poly) {

	int reshigh;

	//float *hist = pStat->history;
	//int64 *hist = pStat->history;
	//long *hist = pStat->history;
	int filt,cutoff,depth,keyfollow, realfollow;

	patchCache *tcache = pStat->tcache;
	dpoly::partialStatus::envstatus *tStat  = &pStat->envs[FILTENV];

	keyfollow = pStat->filtval;
	realfollow = pStat->realval;

	int fr = poly->freqnum;

	if(tStat->decaying) {
		if(pStat->isDecayed) {
			reshigh = 0;
		} else {
			reshigh = tStat->envbase;
			reshigh = (reshigh + ((tStat->envdist * tStat->envpos) / tStat->envsize));
			if(tStat->envpos >= tStat->envsize) reshigh = 0;
		}

	} else {
		if(tStat->envstat==4) {
			reshigh =tcache->filtsustain;
			if(!poly->sustain) StartDecay(FILTENV, reshigh, pStat, poly);
		} else {

			if((tStat->envstat==-1) || (tStat->envpos >= tStat->envsize)) {
				if(tStat->envstat==-1) {
					tStat->envbase = 0;
				} else {
					tStat->envbase = tcache->filtEnv.envlevel[tStat->envstat];
				}
				tStat->envstat++;
				tStat->envpos = 0;
				if(tStat->envstat==3) {
					tStat->envsize = lasttimetable[(int)tcache->filtEnv.envtime[tStat->envstat]];
				} else {
					tStat->envsize = (envtimetable[(int)tcache->filtEnv.envtime[tStat->envstat]] * timekeytable[(int)tcache->filtEnv.envtkf][poly->freqnum]) >> 8;
				}

				tStat->envsize++;
				tStat->envdist = tcache->filtEnv.envlevel[tStat->envstat] - tStat->envbase;
			}

			reshigh = tStat->envbase;
			reshigh = (reshigh + ((tStat->envdist * tStat->envpos) / tStat->envsize));

		}
		tStat->prevlevel = reshigh;
	}

	cutoff = (tcache->filtEnv.cutoff);

	//if(tcache->waveform==1) reshigh = (reshigh * 3) >> 2;

	depth = (tcache->filtEnv.envdepth);

	//int sensedep = (depth * 127-tcache->filtEnv.envsense) >> 7;
	depth = (depth * filveltable[poly->vel][(int)tcache->filtEnv.envsense]) >> 8;

	int bias = tcache->tvfbias;
	int dist;


	if(bias!=0) {
		//LOG_MSG("Cutoff before %d", cutoff);
		if(tcache->tvfdir == 0) {
			if(fr < bias) {
				dist = bias - fr;
				cutoff = (cutoff * fbiastable[tcache->tvfblevel][dist]) >> 8;
				
			}
		} else {
			// > Bias
			if(fr > bias) {
				dist = fr - bias;
				cutoff = (cutoff * fbiastable[tcache->tvfblevel][dist]) >> 8;
			}
			
		}
		//LOG_MSG("Cutoff after %d", cutoff);
	
	}

	
	
	depth = (depth * fildeptable[tcache->tvfdepth][fr]) >> 8;
	reshigh = (reshigh * depth) >> 7;

	
	int32 tmp;
	
	cutoff *= keyfollow;
	cutoff /= realfollow;
	
	reshigh *= keyfollow;
	reshigh /= realfollow;
	
	if(cutoff>100) cutoff = 100;
	if(reshigh>100) reshigh = 100;
	if(cutoff<0) cutoff = 0;
	if(reshigh<0) reshigh = 0;
	tmp = nfilttable[fr][cutoff][reshigh];	
	//tmp *= keyfollow;
	//tmp /= realfollow;
	
	//LOG_MSG("Cutoff %d, tmp %d, freq %d", cutoff, tmp, tmp * 256);
	
	return tmp;
	//reshigh = (reshigh * depth * 6)>>10;
	
	filt = (cutoff + reshigh) * keyfollow;
	filt = filt / realfollow;
	



	if(filt<0) filt = 0;
	if(filt>=200) filt = 199;
	tmp = filttable[(int)tcache->waveform][fr][filt];
	return tmp;

}

MidiChannel::MidiChannel(int samp, int cnum) {
	isRy = holdpedal = isPlaying = false;
	volumesetting.rightvol = volumesetting.leftvol = volumesetting.rightvol2 = volumesetting.leftvol2 = 32767;
	patch = storedpatch = 0;
	sampRate = samp;
	channum = cnum;
	volume = 102;
	panpot = 64;
	init = true;
	bend = 0x1000;
	memset(polyTable,0,sizeof(polyTable));
	memset(pcache, 0, sizeof(pcache));
	
	if(cnum==8) {
		isRy = true;
		int pan;
		volume = 102;
		// Cache drum patches
		int q,m;
		for(q=0;q<30;q++) {
			SetPatch(0,q);
			for(m=0;m<42;m++) {
				if(DrumTable[m]==q) {
					pan = drmPanTable[m];
					if(pan<64) {
						drumPan[q][0] = 32767;                // lv
						drumPan[q][1] = pan << 9;           // rv
					} else {
						drumPan[q][0] = (63-(pan-63)) << 9; // lv
						drumPan[q][1] = 32767;                // rv
					}
					break;
				}
			}
		}
	}
	init = false;
}

INLINE int MidiChannel::FixBiaslevel(int srcpnt, int *dir) {
	int noteat = srcpnt & 63;
	int outnote;
	*dir = 1;
	if(srcpnt < 64) *dir = 0;
	outnote = 33 + noteat;
	//LOG_MSG("Bias note %d, dir %d", outnote, *dir);

	return outnote;
	
}

INLINE int MidiChannel::FixKeyfollow(int srckey, int *dir) {
	if (srckey>=0 && srckey<=16) {
		//int keyfix[17] = { 256, 128, 64, 0, 32, 64, 96, 128, 128+32, 192, 192+32, 256, 256+64, 256+128, 512, 259, 269 };
		int keyfix[17] = { 256*16, 128*16, 64*16, 0, 32*16, 64*16, 96*16, 128*16, (128+32)*16, 192*16, (192+32)*16, 256*16, (256+64)*16, (256+128)*16, (512)*16, 4100, 4116};

		if (srckey<3)
			*dir = -1;
		else if (srckey==3)
			*dir = 0;
		else
			*dir = 1;

		return keyfix[srckey];
	} else {
		//LOG_MSG("Missed key: %d", srckey);
		return 256;
	}
}


void MidiChannel::RefreshPatch() {
	SetPatch(storedpatch,-1);
}

void MidiChannel::SetPatch(int patchnum,int drumNum) {
	int i, j, k, pcm,t;
	
	//int chanoff = channum;
	/* TRISTAN: flush all partials on this channel. This is a hack. */
//	for(i=0; i < MAXPARTIALS; i++)
//		if(partTable[i]->ownerChan == channum)
//			partTable[i]->isActive = false;

	/* TRISTAN: check if any partials are still playing on this channel, if      *
	 * so then duplicate the cached data from the channel to the partial so that *
	 * we can change the channels cache without affecting the partial. Hopefully *
	 * this is fairly rare.                                                      */
	if (storedpatch != patchnum)
		for (i = 0; i < MAXPARTIALS; i++)
			if(partTable[i]->ownerChan == channum)
				if (partTable[i]->isActive)
				{
					/* copy cache data */
					for (j = 0; j < 4; j++)
						partTable[i]->cachebackup[j] = pcache[j];

					/* update pointers */
					for (j = 0; j < partTable[i]->timbreNum; j++)
						for (k = 0; k < 4; k++)
							partTable[i]->tmppoly[j].pStatus[k].tcache = partTable[i]->cachebackup + k;

					partTable[i]->tcache = partTable[i]->cachebackup;
				}

	storedpatch = patchnum;

	patch = (mt32ram.params.pSettings[patchnum].timbreGroup * 64) + mt32ram.params.pSettings[patchnum].timbreNum;

	timbreParam timSrc;
	//timSrc = mt32ram.params.patch[patch];
	if (drumNum==-1) {
		timSrc = mt32ram.params.timTemp[channum];
		memset(&currentInstr,0,16);
		memcpy(&currentInstr,timSrc.common.name,10);


	} else {
		// This is to cache all the drum tibres ahead of time
		timSrc = drums[drumNum];
	}
	//LOG_MSG("Reloading patch %d", channum);
	sustain = (timSrc.common.nosustain == 0);
	P1Mix = PartMixStruct[(int)timSrc.common.pstruct12];
	P2Mix = PartMixStruct[(int)timSrc.common.pstruct34];

	//sustain = true;
	partialCount = 0;

	for(t=0;t<4;t++) {

		// Calculate and cache common parameters
		pcm = timSrc.partial[t].wg.pcmwave;
		pcache[t].rawPCM = pcm;

		pcache[t].convPCM = PCMList[pcm];
		pcache[t].useBender = (timSrc.partial[t].wg.bender == 1);

		// pcm > -1
		switch (t) {
		case 0:
			pcache[t].PCMPartial = (PartialStruct[(int)timSrc.common.pstruct12] >> 1) & 0x1;
			break;
		case 1:
			pcache[t].PCMPartial = PartialStruct[(int)timSrc.common.pstruct12] & 0x1;
			break;
		case 2:
			pcache[t].PCMPartial = (PartialStruct[(int)timSrc.common.pstruct34] >> 1) & 0x1;
			break;
		case 3:
			pcache[t].PCMPartial = PartialStruct[(int)timSrc.common.pstruct34] & 0x1;
			break;
		default:
			break;
		}


		if( ((timSrc.common.pmute >> (t)) & 0x1) == 1 ) {
			pcache[t].playPartial = true;
			pcache[t].usePartial = true;

			// Hack and a half... needed so drum partial numbers come through
			pcache[0].partCount = t+1;

			partialCount++;
		} else {
			pcache[t].playPartial = false;
			pcache[t].usePartial = false;
			continue;
		}

		pcache[t].sustain = (timSrc.common.nosustain == 0);
		pcache[t].waveform = timSrc.partial[t].wg.waveform;
		pcache[t].pulsewidth = timSrc.partial[t].wg.pulsewid;
		pcache[t].pwsens = timSrc.partial[t].wg.pwvelo;
		pcache[t].pitchkeyfollow = FixKeyfollow(timSrc.partial[t].wg.keyfollow, &pcache[t].pitchkeydir);

		// Calculate and cache pitch stuff
		pcache[t].pitchshift = (timSrc.partial[t].wg.coarse+mt32ram.params.pSettings[patchnum].keyShift);
		int32 pFine, tFine, fShift;
		pFine = (int32)timSrc.partial[t].wg.fine;
		tFine = (int32)mt32ram.params.pSettings[patchnum].fineTune;
		fShift = ((pFine - 50) + (tFine - 50)) + 100;
		pcache[t].fineshift = finetable[fShift];

		keyshift = mt32ram.params.pSettings[patchnum].keyShift-24;
		assignmode = mt32ram.params.pSettings[patchnum].assignMode;

		pcache[t].pitchEnv = timSrc.partial[t].env;
		pcache[t].pitchEnv.sensitivity = (int)((float)pcache[t].pitchEnv.sensitivity*1.27);
		pcache[t].pitchsustain = pcache[t].pitchEnv.level[3];

		// Calculate and cache TVA envelope stuff
		pcache[t].ampEnv = timSrc.partial[t].tva;
		int l;
		for(l=0;l<4;l++) {
			pcache[t].ampEnv.envlevel[l] = (int)((float)pcache[t].ampEnv.envlevel[l]*1.27);
		}
		pcache[t].ampEnv.level = (int)((float)pcache[t].ampEnv.level*1.27);
		float tvelo = ((float)pcache[t].ampEnv.velosens/100.0);
		float velo = (fabs(tvelo-.5)/.5);
		pcache[t].ampenvdir = 0;
		if(tvelo<.5) pcache[t].ampenvdir = 1;
		velo *= 63.0;
		pcache[t].ampEnv.velosens = (int)(velo);

		pcache[t].ampbias[0] = FixBiaslevel(pcache[t].ampEnv.biaspoint1, &pcache[t].ampdir[0]);
		pcache[t].ampblevel[0] = 12-pcache[t].ampEnv.biaslevel1;
		pcache[t].ampbias[1] = FixBiaslevel(pcache[t].ampEnv.biaspoint2, &pcache[t].ampdir[1]);
		pcache[t].ampblevel[1] = 12-pcache[t].ampEnv.biaslevel2;
		pcache[t].ampdepth = pcache[t].ampEnv.envvkf * pcache[t].ampEnv.envvkf;
		pcache[t].ampsustain = pcache[t].ampEnv.envlevel[3];
		pcache[t].amplevel = pcache[t].ampEnv.level;


		// Calculate and cache filter stuff
		pcache[t].filtEnv = timSrc.partial[t].tvf;
		pcache[t].tvfdepth = pcache[t].filtEnv.envdkf;
		pcache[t].filtkeyfollow  = FixKeyfollow(pcache[t].filtEnv.keyfollow, &pcache[t].keydir);
		pcache[t].filtEnv.envdepth = (int)((float)pcache[t].filtEnv.envdepth * 1.27);
		pcache[t].tvfbias = FixBiaslevel(pcache[t].filtEnv.biaspoint, &pcache[t].tvfdir);
		pcache[t].tvfblevel = pcache[t].filtEnv.biaslevel;
		pcache[t].filtsustain  = pcache[t].filtEnv.envlevel[3];

		// Calculate and cache LFO stuff
		//pcache[t].lfodepth = (int)((float)timSrc.partial[t].lfo.depth * 1.27);
		pcache[t].lfodepth = timSrc.partial[t].lfo.depth;
		pcache[t].lfoperiod = lfotable[(int)timSrc.partial[t].lfo.rate];
		pcache[t].lforate = timSrc.partial[t].lfo.rate;
		pcache[t].modsense = timSrc.partial[t].lfo.modsense;

	}
	//LOG_MSG("Res 1: %d 2: %d 3: %d 4: %d", pcache[0].waveform, pcache[1].waveform, pcache[2].waveform, pcache[3].waveform);

	if(drumNum!=-1) memcpy(drumCache[drumNum],pcache,sizeof(pcache));
	if(!init) AllStop();

	//LOG_MSG("Channel #%d set instrument: %s - %d - %d - %d - %d - pc %d", chanoff, currentInstr, timSrc.partial[0].wg.pcmwave, timSrc.partial[1].wg.pcmwave, timSrc.partial[2].wg.pcmwave, timSrc.partial[3].wg.pcmwave, pcache[0].partCount);			
	
}

void MidiChannel::SetVolume(int vol) {

	volume = voltable[vol];
	
}

void MidiChannel::SetPan(int pan) {
	panpot = pan;

	if(pan<64) {
		volumesetting.leftvol = 32767;
		volumesetting.rightvol = pan << 9;

	}
	if(pan>=64) {
		volumesetting.rightvol = 32767;
		volumesetting.leftvol = (63-(pan-63)) << 9;

	}



	//LOG(LOG_ERROR|LOG_MISC,"Pan %d",panpot);
}

INLINE int16 RingMod(int16 p1, int16 p2, bool useFirst) {
	if(useFirst) {
		//return (int16)( ( ((float)p1/32767.0) * ((float)p2/32767.0) ) * 32767);
		return (int16)( ((int32)p1 * (int32)p2) >> 15);

	} else {
		// An interesting undocumented feature of the MT-32
		// Putting ring mod on a muted partial introduces noise to the ring modulator
		// Dune 2 makes use of this
		return (int16)( ((int32)smallnoise[samplepos/100] * (int32)p2) >> 15);
	}
}


INLINE void MidiChannel::InitKeyFollow(dpoly *tpoly) {
	// Setup partial keyfollow
	int keyfollow = 0;
	int tmpval = tpoly->freqnum,t;
	int keyedval;

	// Note follow relative to middle C

	for(t=0;t<4;t++) {

		// Calculate keyfollow for pitch
		switch(pcache[t].pitchkeydir) {
		case -1:
			keyfollow = ((int)((MIDDLEC*2-tmpval*2)/2) * pcache[t].pitchkeyfollow) >> 12;
			break;
		case 0:
			keyfollow =  0;
			break;
		case 1:
			keyfollow = ((int)((tmpval*2-MIDDLEC*2)/2)*pcache[t].pitchkeyfollow)>>12;
			break;
		}
		if((pcache[t].pitchkeyfollow>4096) && (pcache[t].pitchkeyfollow<4200)) {
			// Be sure to round up on keys below MIDDLEC
			if(((tmpval*2-MIDDLEC*2)/2) < 0) keyfollow++;
		}
		keyedval = (keyfollow + pcache[t].pitchshift);
		if(keyedval>108) keyedval = 108;
		if(keyedval<12) keyedval = 12;

		tpoly->pStatus[t].keyedval = keyedval;
		tpoly->pStatus[t].noteval  = tmpval;

		// Calculate keyfollow for filter

		int realfol = ((tmpval*2)-(MIDDLEC*2))/2;
		switch(pcache[t].keydir) {
		case -1:
			keyfollow = ((int)((MIDDLEC*2-tmpval*2)/2) * pcache[t].filtkeyfollow )>>12;
			break;
		case 0:
			keyfollow = tmpval;
			break;
		case 1:
			keyfollow = ((int)((tmpval*2-(MIDDLEC*2))/2) * pcache[t].filtkeyfollow )>>12;
			break;
		}

		if (keyfollow>108) keyfollow=108;
		if (keyfollow<-108) keyfollow =-108;
		tpoly->pStatus[t].filtnoval = keyfollow+108;
		tpoly->pStatus[t].filtval = keytable[keyfollow+108];
		tpoly->pStatus[t].realval = keytable[realfol+108];

	}


}

CPartialMT32 * AllocPartial(int chanNum) {
	int i;

	CPartialMT32 *outPart = NULL;

#if MAXPARTIALS == 32
	for(i=0;i<MAXPARTIALS;i++) {
		if(PartialReserveTable[i] == chanNum) {
			if(outPart == NULL) {
				if(!partTable[i]->isActive) {
					outPart = partTable[i];
				}
			}
		}
	}
	if(outPart != NULL) {
		outPart->isActive = true;
		outPart->age = 0;
	} else {
		for(i=0;i<MAXPARTIALS;i++) {
			if(!partTable[i]->isActive) {
				outPart = partTable[i];
				outPart->isActive = true;
				outPart->age = 0;
				break;
			}
		}
	}
#else
	for(i=0;i<MAXPARTIALS;i++) {
		if(outPart == NULL) {
			if(!partTable[i]->isActive) {
				outPart = partTable[i];
			}
		}
	}
	if(outPart != NULL) {
		outPart->isActive = true;
		outPart->age = 0;
	}

#endif
	return outPart;
}

int GetFreePartialCount(void) {
	int i;
	int count = 0;
	memset(partialChan,0,sizeof(partialChan));
	for(i=0;i<MAXPARTIALS;i++) {
		if(!partTable[i]->isActive) {
			count++;
		} else {
			partialChan[partTable[i]->ownerChan]++;
		}
	}
	return count;
}


INLINE void MidiChannel::PlayPoly(int m, int f,int vel) {
	
	CPartialMT32 *tmpParts[4];
	f += keyshift;
	if((f<0) || (f >255)) return;
	freq = freqtable[f];
	dpoly *tpoly = &polyTable[m];

	tpoly->isPlaying = true;
	tpoly->isDecay = false;
	tpoly->isActive = true;
	tpoly->pcmoff.pcmabs = 0;
	tpoly->freq = freq;
	tpoly->freqnum = f;
	tpoly->pcmdelta = 0x100;
	tpoly->age = 0;
	tpoly->vel = vel;
	tpoly->chan = this->channum;
	tpoly->pedalhold = false;
	tpoly->firstsamp = true;
	memset(tpoly->pStatus,0,sizeof(tpoly->pStatus));
	int x,e;

	for(x=0;x<4;x++) {

		tpoly->pStatus[x].partNum = x;
		tpoly->pStatus[x].lfopos = 0;
		tpoly->pStatus[x].playPartial = pcache[x].playPartial;
		tpoly->pStatus[x].usePartial = pcache[x].usePartial;
		tpoly->pStatus[x].tcache = &pcache[x];
		tpoly->pStatus[x].pulsewidth = pcache[x].pulsewidth + pwveltable[pcache[x].pwsens][vel];
		
		if(tpoly->pStatus[x].pulsewidth > 100) tpoly->pStatus[x].pulsewidth = 100;
		if(tpoly->pStatus[x].pulsewidth < 0) tpoly->pStatus[x].pulsewidth = 0;

		if(pcache[x].playPartial) {
			tmpParts[x] = AllocPartial(channum);
		} else {
			tmpParts[x] = NULL;
		}
		tpoly->pStatus[x].myPart = (void *)tmpParts[x];

		for(e=0;e<4;e++) {
			tpoly->pStatus[x].envs[e].envstat = -1;
			tpoly->pStatus[x].envs[e].sustaining = false;
			tpoly->pStatus[x].envs[e].decaying = false;
			tpoly->pStatus[x].envs[e].envpos = 0;
			tpoly->pStatus[x].envs[e].count = 0;
			tpoly->pStatus[x].envs[e].counter = 0;
			

		}
	}

	bool allnull = true;
	for(x=0;x<4;x++) {
		//if(tmpParts[x] != NULL) allnull = false;
		if(pcache[x].playPartial) allnull = false;
	}
	//if(allnull) LOG_MSG("No paritals to play for %s", this->currentInstr);

	tpoly->partCount = pcache[0].partCount;
	tpoly->P1Mix = P1Mix;
	tpoly->P2Mix = P2Mix;
	tpoly->sustain = sustain;
	tpoly->isRy = false;
	tpoly->bendptr = &bend;
	tpoly->volumeptr = &volume;
	tpoly->pansetptr = &volumesetting;

	InitKeyFollow(tpoly);

	for(x=0;x<4;x++) {
		if(tmpParts[x] != NULL) {
			int pairPart, useMix, partNum;
			switch(x) {
			case 0:
				useMix = P1Mix;
				partNum = 0;
				pairPart = 1;
				break;
			case 1:
				useMix = P1Mix;
				partNum = 1;
				pairPart = 0;
				break;
			case 2:
				useMix = P2Mix;
				partNum = 0;
				pairPart = 3;
				break;
			case 3:
				useMix = P2Mix;
				partNum = 1;
				pairPart = 2;
				break;
			default:
				useMix = P1Mix;
				partNum = 0;
				pairPart = 0;
				break;
			}
			tmpParts[x]->startPartial(tpoly,tpoly->pStatus[x].tcache,&tpoly->pStatus[x],tmpParts[pairPart],useMix,partNum,channum,x);
			tpoly->partActive[x] = true;
		} else {
			tpoly->partActive[x] = false;
		}
	}

#if DISPLAYINSTR == 1
	memset(&currentInstr,0,16);
	memcpy(&currentInstr,mt32ram.params.patch[patch].common.name,10);
	//LOG_MSG("MT32 chan %d (\"%s\") s note poly %d - Vel %d Freq %d Vol %d", channum, currentInstr, m, vel, f, volume);
#endif
}

INLINE void MidiChannel::PlayDrum(int m, int f,int vel) {
	if(!((f>=35) && (f<= 76))) return;
	CPartialMT32 *tmpParts[4];
	freq = freqtable[60];
	dpoly *tpoly = &polyTable[m];

	tpoly->drumnum = f;
	tpoly->isPlaying = true;
	tpoly->isDecay = false;
	tpoly->isActive = true;
	tpoly->pcmnum = DrumTable[f-35];
	tpoly->pcmoff.pcmabs = 0;
	tpoly->freq = freq;
	tpoly->freqnum = 60;
	tpoly->pcmdelta = 0x100;
	tpoly->age = 0;
	tpoly->vel = vel;
	tpoly->chan = this->channum;
	tpoly->pedalhold = false;
	tpoly->firstsamp = true;
	memset(tpoly->pStatus,0,sizeof(tpoly->pStatus));
	memcpy(pcache,drumCache[tpoly->pcmnum],sizeof(pcache));
	int x,e;
	for(x=0;x<4;x++) {
		tpoly->pStatus[x].partNum = x;
		tpoly->pStatus[x].playPartial = pcache[x].playPartial;
		tpoly->pStatus[x].usePartial = pcache[x].usePartial;
		tpoly->pStatus[x].tcache = &drumCache[tpoly->pcmnum][x];

		if(pcache[x].playPartial) {
			tmpParts[x] = AllocPartial(channum);
		} else {
			tmpParts[x] = NULL;
		}
		tpoly->pStatus[x].myPart = (void *)tmpParts[x];

		for(e=0;e<4;e++) {
			tpoly->pStatus[x].envs[e].envstat = -1;
			tpoly->pStatus[x].envs[e].count = 0;
			tpoly->pStatus[x].envs[e].counter = 0;
		}
	}

	tpoly->P1Mix = PartMixStruct[(int)drums[tpoly->pcmnum].common.pstruct12];
	tpoly->P2Mix = PartMixStruct[(int)drums[tpoly->pcmnum].common.pstruct34];
	tpoly->sustain = (drums[tpoly->pcmnum].common.nosustain == 0);
	tpoly->isRy = true;
	tpoly->bendptr = &tpoly->drumbend;
	tpoly->drumbend = 0x1000;

	tpoly->partCount = pcache[0].partCount;
	tpoly->volumeptr = &volume;
	tpoly->pansetptr = &volumesetting;

	InitKeyFollow(tpoly);

	for(x=0;x<4;x++) {
		if(tmpParts[x] != NULL) {
			int pairPart, useMix, partNum;
			switch(x) {
			case 0:
				useMix = P1Mix;
				partNum = 0;
				pairPart = 1;
				break;
			case 1:
				useMix = P1Mix;
				partNum = 1;
				pairPart = 0;
				break;
			case 2:
				useMix = P2Mix;
				partNum = 0;
				pairPart = 3;
				break;
			case 3:
				useMix = P2Mix;
				partNum = 1;
				pairPart = 2;
				break;
			default:
				useMix = P1Mix;
				partNum = 0;
				pairPart = 0;
				break;
			}
			tmpParts[x]->startPartial(tpoly,tpoly->pStatus[x].tcache,&tpoly->pStatus[x],tmpParts[pairPart],useMix,partNum,channum,x);
		}
	}

#if DISPLAYINSTR == 1
	memset(&currentInstr,0,16);
	memcpy(&currentInstr,drums[tpoly->pcmnum].common.name,10);
	//LOG_MSG("MT32 drum chan (f %d = %d) (\"%s\") starting note poly %d - Velocity %d", f, tpoly->pcmnum, currentInstr, m, vel);
#endif
}

/*
bool FreePartials(int needed, int chanNum) {
	int i;
	int myChanPrior = (int)mt32ram.params.system.reserveSettings[chanNum];
	if(myChanPrior<partialChan[chanNum]) {
		//This can have more channels, must kill off those with less priority
		int most, mostchan;
		while(needed > 0) {
			int selectChan = -1;
			//Find the worst offender with more partials than allocated and kill them
			most = -1;
			mostchan = -1;
			int diff;

			for(i=0;i<9;i++) {
				diff = partialChan[i] - (int)mt32ram.params.system.reserveSettings[i];

				if(diff>0) {
					if(diff>most) {
						most = diff;
						mostchan = i;
					}
				}
			}
			selectChan = mostchan;
			if(selectChan==-1) {
				// All channels are within the allocated limits, you suck
				// Look for first partial not of this channel that's decaying perhaps?
				return false;
			}
			bool found;
			int oldest;
			int oldnum;
			while(partialChan[selectChan] > (int)mt32ram.params.system.reserveSettings[selectChan]) {
				oldest = -1;
				oldnum = -1;
				found = false;
				for(i=0;i<32;i++) {
					if(partTable[i]->isActive) {
						if(partTable[i]->ownerChan == selectChan) {
							found = true;
							if(partTable[i]->age > oldest) {
								oldest = partTable[i]->age;
								oldnum = i;
							}
						}
					}
				}
				if(!found) break;
				partTable[oldnum]->stopPartial();
				--partialChan[selectChan];
				--needed;
			}

		}
		return true;

	} else {
		//This channel has reached its max, must kill off its own
		bool found;
		int oldest;
		int oldnum;
		while(needed > 0) {
			oldest = -1;
			oldnum = -1;
			found = false;
			for(i=0;i<32;i++) {
				if(partTable[i]->isActive) {
					if(partTable[i]->ownerChan == chanNum) {
						found = true;
						if(partTable[i]->age > oldest) {
							oldest = partTable[i]->age;
							oldnum = i;
						}
					}
				}
			}
			if(!found) break;
			partTable[oldnum]->stopPartial();
			--needed;
		}
		// Couldn't free enough partials, sorry
		if(needed>0) return false;
		return true;
	}

}
*/
bool FreePartials(int needed, int chanNum) {
	
	int i;
#if MAXPARTIALS == 32
	// Reclaim partials reserved for this channel
	// Kill those that are already decaying first
	/*
	for(i=0;i<32;i++) {
		if(PartialReserveTable[i] == chanNum) {
			if(partTable[i]->ownerChan != chanNum) {
				if(partTable[i]->partCache->envs[AMPENV].decaying) {
					partTable[i]->isActive = false;
					--needed;
					if(needed<=0) return true;
				}
			}
		}

	}*/
	// Then kill those with the lowest channel priority --- oldest at the moment
	bool found;
	int64 prior, priornum;
	dpoly *killPoly;
	found = true;
	while(found) {
		found = false;
		prior = -1;
		priornum = -1;

		for(i=0;i<32;i++) {
			if(PartialReserveTable[i] == chanNum) {
				if(partTable[i]->isActive) {
					if(partTable[i]->ownerChan != chanNum) {
						/*
						if(mt32ram.params.system.reserveSettings[partTable[i]->ownerChan] < prior) {
							prior = mt32ram.params.system.reserveSettings[partTable[i]->ownerChan];
							priornum = i;
						}*/
						if(partTable[i]->age > prior) {
							prior = partTable[i]->age;
							priornum = i;
						}

						found = true;
					}
				}
			}
		}
		if(priornum != -1) {
 			partTable[priornum]->isActive = false;
			
			killPoly = partTable[priornum]->tmppoly;
			killPoly->partActive[partTable[priornum]->timbreNum] = false;
			killPoly->isActive = killPoly->partActive[0] || killPoly->partActive[1] || killPoly->partActive[2] || killPoly->partActive[3];
			--needed;
			if(needed<=0) return true;
		}
	}


	// Kill off the oldest partials within this channel
	int64 oldest, oldlist;

	while(needed>0) {
		oldest = -1;
		oldlist = -1;
		for(i=0;i<32;i++) {
			if(partTable[i]->isActive) {
				if(partTable[i]->ownerChan == chanNum) {
					if(partTable[i]->age > oldest) {
						oldest = partTable[i]->age;
						oldlist = i;
					}
				}
			}
		}
		if(oldlist != -1) {
			partTable[oldlist]->isActive = false;
			killPoly = partTable[oldlist]->tmppoly;
			killPoly->partActive[partTable[oldlist]->timbreNum] = false;
			killPoly->isActive = killPoly->partActive[0] || killPoly->partActive[1] || killPoly->partActive[2] || killPoly->partActive[3];
			--needed;
		} else {
			break;
		}
	}
	if(needed<=0) return true;


	return false;
#else
	//No priority table when not using standard MT-32 configuration
	// Kill off the oldest partials within this channel
	int oldest, oldlist;
	dpoly *killPoly;
	oldest = -1;
	oldlist = -1;
	while(needed>0) {
		for(i=0;i<MAXPARTIALS;i++) {
			if(partTable[i]->ownerChan == chanNum) {
				if(partTable[i]->age > oldest) {
					oldest = partTable[i]->age;
					oldlist = i;
				}
			}
		}
		if(oldlist != -1) {
			partTable[oldlist]->isActive = false;
			killPoly = partTable[oldlist]->tmppoly;
			killPoly->partActive[partTable[oldlist]->timbreNum] = false;
			killPoly->isActive = killPoly->partActive[0] || killPoly->partActive[1] || killPoly->partActive[2] || killPoly->partActive[3];
			--needed;
		} else {
			break;
		}
	}
	if(needed<=0) return true;
	//LOG_MSG("Out of paritals!");
	return false;
#endif


}


void MidiChannel::CheckNoteList() {
	int q,t;
	for(q=0;q<MAXPOLY;q++) {
		if(polyTable[q].isActive) {
			bool isActive = false;
			CPartialMT32 * tmpPart;
			for(t=0;t<4;t++) {
				tmpPart = (CPartialMT32 *)polyTable[q].pStatus[t].myPart;
				if(tmpPart != NULL) {
					if(tmpPart->ownerChan == channum) {
						isActive = isActive || tmpPart->isActive;
					}
				}
			}
			polyTable[q].isActive = isActive;

		}
	}
}

INLINE void MidiChannel::PlayNote(int f,int vel) {
	int m;
	sndoff=0;
	velocity = vel;

	isPlaying = false;

	//if(channum!=0) return;

	if(isRy) memcpy(pcache,drumCache[DrumTable[f-35]],sizeof(pcache));

	// POLY1 mode, Single Assign
	// Haven't found any software that uses any of the other poly modes
	if(!isRy) {
		for(m=0;m<MAXPOLY;m++) {
			if((polyTable[m].isActive) && (polyTable[m].freqnum == f)) {
				StopNote(f,vel);
				break;
			}
		}
	}

	int needPartials = pcache[0].partCount;

	if(needPartials > GetFreePartialCount()) {
		if(!FreePartials(needPartials, channum)) {
			// Before we quit, see if there are other channels willing to donate
			if(needPartials > GetFreePartialCount()) {
				// Unable to get needed partials to play this note
				return;
			}
		}
	}

	// Find free note allocator
	for(m=0;m<MAXPOLY;m++) {
		if(!polyTable[m].isActive){
			isPlaying=true;
			if (!isRy) {
				PlayPoly(m,f,vel);
			} else {
				if(DrumTable[f-35]>-1) PlayDrum(m,f,vel);
			}
			break;
		}
	}


}
void MidiChannel::AllStop() {
	int q,t;
	for(q=0;q<MAXPOLY;q++) {
		dpoly *tpoly = &polyTable[q];
		if(tpoly->isPlaying) {
			tpoly->isDecay = true;
			for(t=0;t<4;t++) {

				//memset(tpoly->pStatus[t].decay,0,sizeof(tpoly->pStatus[t].decay));
				//tpoly->pStatus[t].isDecayed = 0;
				//memset(tpoly->pStatus[t].decaying,true,sizeof(tpoly->pStatus[t].decaying));

				StartDecay(AMPENV,tpoly->pStatus[t].envs[AMPENV].prevlevel, &tpoly->pStatus[t], tpoly);
				StartDecay(FILTENV,tpoly->pStatus[t].envs[FILTENV].prevlevel, &tpoly->pStatus[t], tpoly);
				StartDecay(PITCHENV,tpoly->pStatus[t].envs[PITCHENV].prevlevel, &tpoly->pStatus[t], tpoly);

				tpoly->pStatus[t].pitchsustain = false;
			}
			tpoly->isPlaying = false;
		}

	}

}

void MidiChannel::StopPedalHold() {
	int q;
	for(q=0;q<MAXPOLY;q++) {
		dpoly *tpoly;
		tpoly = &polyTable[q];
		if (tpoly->pedalhold) StopNote(tpoly->freqnum,0);
	}

}

void MidiChannel::StopNote(int f,int vel) {
	// Find oldest note... yes, the MT-32 can be reconfigured to kill different note first
	// This is simplest
	int oldest = -1;
	int oldage = 0;
	int count = 0;
	int q,t;
	bool found = false;
	dpoly *tpoly;

	// Non-sustaining instruments ignore stop note commands.
	// They die away eventually anyway
	//if(!tpoly->sustain) return;

	//LOG_MSG("MT32 chan %d (\"%s\") stopping note %d", this->channum, currentInstr, f);
		
	for(q=0;q<MAXPOLY;q++) {
		tpoly = &polyTable[q];
		
		if(tpoly->isPlaying)  {
			if(tpoly->freqnum == f) {
				if (holdpedal) {
					tpoly->pedalhold = true;

				} else {
					if(tpoly->sustain) {
						tpoly->isDecay = true;


						for(t=0;t<4;t++) {
							//memset(tpoly->pStatus[t].decay,0,sizeof(tpoly->pStatus[t].decay));
							//tpoly->pStatus[t].isDecayed = 0;
							//memset(tpoly->pStatus[t].decaying,true,sizeof(tpoly->pStatus[t].decaying));
							StartDecay(AMPENV,tpoly->pStatus[t].envs[AMPENV].prevlevel, &tpoly->pStatus[t], tpoly);
							StartDecay(FILTENV,tpoly->pStatus[t].envs[FILTENV].prevlevel, &tpoly->pStatus[t], tpoly);
							StartDecay(PITCHENV,tpoly->pStatus[t].envs[PITCHENV].prevlevel, &tpoly->pStatus[t], tpoly);

							tpoly->pStatus[t].pitchsustain = false;
						}					
						tpoly->isPlaying = false;
					}
					//return;
				}
				found = true;
				

			}
		}

	}

	if(f!=-1) return;
	oldest = -1;
	for(q=0;q<MAXPOLY;q++) {
		tpoly = &polyTable[q];

		if((tpoly->isPlaying) && (!tpoly->isDecay) && (tpoly->chan==channum)) {
			if(tpoly->age>=oldage) {
				oldage = tpoly->age;
				oldest = q;
			}
			count++;
		}
	}

	if(oldest!=-1) {
		tpoly = &polyTable[oldest];
		tpoly->isDecay = true;
		for(t=0;t<4;t++) {
			//memset(tpoly->pStatus[t].decay,0,sizeof(tpoly->pStatus[t].decay));
			//tpoly->pStatus[t].isDecayed = 0;
			//memset(tpoly->pStatus[t].decaying,true,sizeof(tpoly->pStatus[t].decaying));
			StartDecay(AMPENV,tpoly->pStatus[t].envs[AMPENV].prevlevel, &tpoly->pStatus[t], tpoly);
			StartDecay(FILTENV,tpoly->pStatus[t].envs[FILTENV].prevlevel, &tpoly->pStatus[t], tpoly);
			StartDecay(PITCHENV,tpoly->pStatus[t].envs[PITCHENV].prevlevel, &tpoly->pStatus[t], tpoly);

			tpoly->pStatus[t].pitchsustain = false;
		}
		tpoly->isPlaying = false;
		//LOG(LOG_MISC|LOG_ERROR,"MT32 chan %d stopping note %d, %d remaining", this->channum, oldest, count-1);
	}

}

MidiChannel *mchan[16];

#endif	


bool CSynthMT32::InitTables(const char *baseDir ) {

#ifdef NOMANSLAND

	int noteat,f;

	//LOG_MSG("MT-32 Initializing Pitch Tables");
	for(f=-108;f<109;f++) {
		keytable[f+108] = (int)(256 * pow((float)2,(float)f/24.0));
		//LOG_MSG("KT %d = %d", f, keytable[f+108]);

	}
	float ff;
	for(f=0;f<=101;f++) {
		ff = (float)f/100.00;
		sqrtable[f] = (int)(100*sqrt(ff));
		float crapff = ff * (22000.0/32000.0);
		if (crapff>1.0) crapff = 1.0;
		//filttable[f] = (ff) * (22000.0/16000.0);

	}

	for(f=0;f<8;f++) {
		ff = (float)f/14.00;
		revtable[f] = (ff);
	}
	File fp;
#if MAKEWAVES == 1
	fp.open("waveforms.raw", File::kFileWriteMode);
#else
	fp.open("waveforms.raw");
#endif
	if(!fp.isOpen()) {
		// TODO : Fail driver init
		return false;

		error("Unable to open waveforms.raw");
		exit(0);
	}

	for(f=12;f<109;f++) {

		//int octave = (f / 12) - 6;
		//int note = f % 12;

		//int cents = (octave * 100) + (note * 100);
		//int freq = (int)((double)262 * pow((double)2,(double)((double)cents/1200)));
		int freq = (int)(TUNING * pow( 2.0, ((double)f - 69.0) / 12.0 ));
		freqtable[f] = freq;
		divtable[f] = (int)(  ((float)SETRATE / (float)(freq)));
		smalldivtable[f] = divtable[f] << 8;
		divtable[f] = divtable[f] << 16;
		int rsaw,dep;

		for(rsaw=0;rsaw<=100;rsaw++) {
			//(66-(((A8-50)/50)^.63)*50)/132
			float fsaw = (float)rsaw;
			if(rsaw<50) fsaw = 50.0;
			int tmpdiv = divtable[f] << 1;

			float sawfact = (66.0-(pow((fsaw-50.0)/50,.63)*50.0))/132.0;
			sawtable[f][rsaw] = (int)(sawfact * (float)tmpdiv) >> 16;
			//LOG_MSG("F %d divtable %d saw %d sawtable %d", f, divtable[f]>>16, rsaw, sawtable[f][rsaw]);

		}

		
		for(dep=0;dep<5;dep++) {
			if(dep>0) {
				float depfac = 3000;
				float ff1, tempdep;
				depfac = (float)depexp[dep];

				ff1 = ((float)f - (float)MIDDLEC) / depfac;
				tempdep = pow((float)2,(float)ff) * 256;
				fildeptable[dep][f] = (int)tempdep;

				ff1 = exp(tkcatconst[dep] * ((float)MIDDLEC-(float)f)) * tkcatmult[dep];
				timekeytable[dep][f] = (int)(ff1 * 256);

			} else {
				fildeptable[dep][f] = 256;
				timekeytable[dep][f] = 256;
			}
		}
		//LOG_MSG("F %d d1 %x d2 %x d3 %x d4 %x d5 %x", f, fildeptable[0][f],fildeptable[1][f],fildeptable[2][f],fildeptable[3][f],fildeptable[4][f]);


		
		
		noteat = 69-12;
		
#if MAKEWAVES ==1 			
		double ampsize = WGAMP;
		int halfdiv = divtable[f] >> 1;
		int fa=0;

		float period = ((float)SETRATE / ((float)freq));
		float m=2*(int)(period/2)+1.0f;
		float k=(int)(((float)50.0/100.0)*period);
		double sd = (2.0*PI)/((((float)divtable[f]/65536.0)) * 4.0);
		double sa = 0.0;

		//LOG_MSG("F %d sd %f div %d", f, sd, divtable[f]);

		int j;
		float dumbfire;
		double square=0.0f;
		double saw = 0.0f;

		memset(waveformsize, 0,sizeof(tmpforms));

		while(sa<=(2.0*PI)) {
			float sqp;

			if(sa<PI) {
				sqp = -1;
				sqp = sqp + (.25 * (sa/PI));
			} else {
				sqp=1;
				sqp = sqp - (.25 * ((sa-PI)/PI));
			}

			square=0;
			saw = 0;
			bool odd = true;
			for(int32 sinus=1;(sinus*freq)<(SETRATE);sinus++) {
				float sinusval = (((1.0/((float)sinus))*(sin(((float)sinus)*sa))));
				saw=saw + sinusval;
			}

			
			dumbfire = sa/2;
			
			//This works pretty good
			tmpforms[2][fa] +=  cos(dumbfire) * -ampsize;
			tmpforms[3][(fa*2)] +=  cos(sa-PI) * -ampsize;
			tmpforms[3][(fa*2)+1] +=  cos((sa+(sd/2))-PI) * -ampsize;
			
			tmpforms[0][fa] +=  (saw * -ampsize)/2;
			tmpforms[1][fa] +=  (saw * ampsize)/2;
			
			//tmpforms[1][fa>>4] += saw * ampsize;


			//waveforms[1][f][fa] = ((prevv2 - prevv)) * ampsize;


			fa++;
			sa+=sd;
		}
		//LOG_MSG("f num %d freq %d and fa %d", f, freq, fa);


		waveformsize[0][f] = waveformsize[1][f] = waveformsize[2][f] = fa*2;
		waveformsize[3][f] = fa*4;
		
		for (int i = 0; i < 4; ++i) {
			waveforms[i][f] = (int16 *)malloc(waveformsize[i][f]);
			memcpy(waveforms[i][f], &tmpforms[i][0],waveformsize[i][f]);
			// TODO / FIXME: The following code is not endian safe!
			out = fp.write(waveforms[i][f],waveformsize[i][f]);
		}
#else
		waveformsize[0][f] = waveformsize[1][f] = waveformsize[2][f] = divtable[f]>>13;
		waveformsize[3][f] = divtable[f]>>12;

		for (int i = 0; i < 4; ++i) {
			waveforms[i][f] = (int16 *)malloc(waveformsize[i][f]);
			for (uint j = 0; j < waveformsize[i][f]/2; ++j)
				waveforms[i][f][j] = fp.readSint16LE();
		}
#endif


		// Create the pitch tables

		float tuner = (32000.0 / (float)SETRATE) * 65536.0;

		wavtable[f] = (int)(tuner * ((float)freq/(float)SAMPLETUNING)); // C4 Tuning?;
		drumdelta[f] = (int)(tuner * ((float)freq/(float)SAMPLETUNING)); // C4 Tuning?;
		int pc,lp,tr,ln;
		for(pc=0;pc<54;pc++) {
			wavtabler[pc][f] = (int)(tuner * ((float)freq/PCM[pc].tune));
		}
		for(lp=0;lp<16;lp++) {
			for(ln=0;ln<16;ln++) {
				looptabler[lp][ln][f] = (int)((float)LoopPatternTuning[lp][ln] * ((float)freq/(float)220.0));
			}	
		}

		for(tr=0;tr<=200;tr++) {
			float brsq, brsa;
			float ftr = (float)tr;
			
			// Verified exact on MT-32
			if(tr>100) ftr=100+(pow(((ftr-100)/100),3)*100);
			brsq = exp(0.0464 * ftr) / 5;
			//EXP((90*0.0175)^2)
			brsq = exp(pow(((float)tr * 0.0153),2));

			// I think this is the one
			brsq = pow((float)10,((float)(tr/50.0)-1));

			//brsa = exp(0.04 * ftr) / 5;

			//Last good one that worked
			//brsa = exp(pow(((float)tr * 0.0133),2));
			brsa = exp(pow(((float)tr * 0.005),3)) * 125;

			brsa = pow((float)10,((float)(tr/46.5)-1))/2;
 
			//brsa = pow(exp((ftr-40)/40),1.15);
			//brsq = exp((ftr-35)/35);
			//brsq = exp((ftr-35)/23);
			//brsa = exp((ftr-35)/23);
			//brsq = pow((ftr / 200), LN) * 48.0;
			//brsa = pow((ftr / 200), LN) * 32.0;
			//brsq = brsa =
			
			filttable[0][f][tr] = (int)(((float)freq * brsq)/(float)(SETRATE/2)*FILTERGRAN);
			if(filttable[0][f][tr]>=((FILTERGRAN*15)/16)) filttable[0][f][tr] = ((FILTERGRAN*15)/16);
			filttable[1][f][tr] = (int)(((float)freq * brsa)/(float)(SETRATE/2)*FILTERGRAN);
			if(filttable[1][f][tr]>=((FILTERGRAN*15)/16)) filttable[1][f][tr] = ((FILTERGRAN*15)/16);
			
		}

		int cf, tf;									
		for(cf=0;cf<=100;cf++) {
			float cfmult = (float)cf;
			
			for(tf=0;tf<=100;tf++) {
						
				//float tfadd = exp((((float)tf / 100.0) - 1.03) * 3.0) * 100;
				//float tfadd = (pow(((float)tf /100.0),4) * 100.0) - 10;
				float tfadd = tf - 0;
						
				if (tfadd < 0) tfadd = 0;
				float freqsum = exp((cfmult + tfadd) / 30.0) / 4.0;
				//float freqsum = exp((cfmult + tfadd) / 33.7) / 3.0;
		
				
				nfilttable[f][cf][tf] = (int)(((float)freq * freqsum)/(float)(SETRATE/2)*FILTERGRAN);
				if(nfilttable[f][cf][tf]>=((FILTERGRAN*15)/16)) nfilttable[f][cf][tf] = ((FILTERGRAN*15)/16);
			}
		}
	}
	
	fp.close();

	int j,res;
	float fres, tres;
	for(res=0;res<31;res++) {
		fres = (float)res/30.0;
		ResonFactor[res] = (pow((float)2,log(pow((float)fres,(float)16))) * 2.5)+1.0;
		ResonInv[res] = 1 / ResonFactor[res];

	}

	for(j=0;j<FILTERGRAN;j++) {
		for(res=0;res<31;res++) {
			tres = ResonFactor[res];
			InitFilter((float)SETRATE, (((float)(j+1.0)/FILTERGRAN)) * ((float)SETRATE/2), filtcoeff[j][res],tres, fres);
			
			// 64-bit variant
#if FILTER_64BIT == 1
			for(int co=0;co<9;co++) {
				filtcoefffix[j][res][co] = (int64)(filtcoeff[j][res][co] * pow(2,20));

			}
#endif

#if FILTER_INT == 1
			for(int co=0;co<9;co++) {
				filtcoefffix[j][res][co] = (long)(filtcoeff[j][res][co] * pow(2,10));

			}
#endif
		}
		
	}

	int period = 65536;
	
	int ang;
	for(ang=0;ang<period;ang++) {
		int halfang = (period / 2);
		int angval  = ang % halfang;
		float tval = (((float)angval / (float)halfang) - 0.5) * 2;
		if(ang>=halfang) tval = -tval;
		sintable[ang] = (int)(tval * 50.0)+50;

	}


	//for(ang=0;ang<period;ang++) sintable[period] *= 50;
	int velt, dep;
	float tempdep;
	for(velt=0;velt<128;velt++) {
		for(dep=0;dep<5;dep++) {
			if(dep>0) {
				float ff1 = exp(3.5*tvcatconst[dep] * (59.0-(float)velt)) * tvcatmult[dep];
				tempdep = 256.0 * ff1;
				veltkeytable[dep][velt] = (int)tempdep;
				if((velt % 16) == 0) {
					//LOG_MSG("Key %d, depth %d, factor %d", velt, dep, (int)tempdep);
				}
			} else {
				veltkeytable[dep][velt] = 256;
			}
		}
#define divpart 14.285714285714285714285714285714

		for(dep=-7;dep<8;dep++) {
			float fldep = fabs((float)dep) / 7.0;
			fldep = pow((float)fldep,(float)2.5);
			if(dep<0)  fldep = fldep * -1.0;
			pwveltable[dep+7][velt] = int32((fldep * (float)velt * 100) / 128.0);
		
		}
	}

	for(dep=0;dep<=100;dep++) {
		for(velt=0;velt<128;velt++) {
			float fdep = (float)dep * 0.000347013; // Another MT-32 constant
			float fv = ((float)velt - 64.0)/7.26;
			float flogdep = pow((float)10, (float)(fdep * fv));
			float fbase;

			if(velt>64) {
				filveltable[velt][dep] = (int)(flogdep * 256.0);
			} else {
				//lff = 1 - (pow(((128.0 - (float)lf) / 64.0),.25) * ((float)velt / 96));
				fbase = 1 - (pow(((float)dep / 100.0),.25) * ((float)(64-velt) / 96.0));
				filveltable[velt][dep] = (int)(fbase * 256.0);				
				
			}

			//LOG_MSG("Filvel dep %d velt %d = %x", dep, velt, filveltable[velt][dep]);
		}
	}

	int lf;
	for(lf=0;lf<=100;lf++) {
		float elf = (float)lf;

		// General envelope
		float logtime = elf * 0.088362939;
		envtimetable[lf] = (int)((exp(logtime)/312.12) * (float)SETRATE);

		// Decay envelope -- shorter for some reason
		// This is also the timing for the envelope right before the
		// amp and filter envelope sustains

		lasttimetable[lf] = decaytimetable[lf] = (int)((exp(logtime)/(312.12*2)) * (float)SETRATE);

		//lasttimetable[lf] = (int)((exp(logtime)/(312.12*6)) * (float)SETRATE);

		// Finetuning table
		//finetable[lf] = (int) ((pow(2, (((float)lf/100.0)-0.5)/6.0))*4096.0);
	
	}
	for(lf=0;lf<=200;lf++) {
		finetable[lf] = (int) ((pow((float)2, (float)((((float)lf/200.0)-1.0)/12.0))*4096.0));
	
	}
	for(lf=0;lf<=48;lf++) {
		bendtable[lf] = (int) ((pow((float)2, (float)((((float)lf/12.0)-2.0)))*4096.0));
	
	}

	float lff;
	for(lf=0;lf<128;lf++) {
		for(velt = 0;velt<64;velt++) {
			lff = 1 - (pow(((128.0 - (float)lf) / 64.0),.25) * ((float)velt / 96));
			//lff = ((128.0 - (float)lf) / 128.0) * ((float)velt / 64);
			//lff = 1.0 / pow(10, lff/2);
			
			
			
			ampveltable[lf][velt] =(int)(lff * 256.0);
			//LOG_MSG("Ampveltable: %d, %d = %d", lf, velt, ampveltable[lf][velt]);
		}
	}
	for(lf=0;lf<=127;lf++) {
		restable[lf] = (int)( (pow((float)2,pow((float)(lf/127.0),(float)2.5))-1)*100  );
		//LOG_MSG("lf %d = amp %d", lf, restable[lf]);
	}

	for(lf=0;lf<=127;lf++) {
		//amptable[lf] = (int)( (pow(2,pow((float)lf/127.0,1))-1)*127  );
		//amptable[lf] = lf;
		//float pubical = (float)lf/127.0; // Yes, as in boobical pubical (because its cubical!)

		// Converting MIDI to volume.
		// Thanks Microsoft DDK
		// value =	2^(log10((index/127)^4))*127 where index = 0..127
		amptable[lf] = voltable[lf] = (int)(127.0 * pow((float)2,log(pow((float)(lf/127.0),(float)4))));

		// Nope, lets try this again
		//amptable[lf] = voltable[lf] = (int)(127.0 * log10((float)lf/12.70));

		//amptable[lf] = (int)(pubical * pubical * pubical * 127.0);

		// Once more...

		//float indec = 128.0 - (float)lf;
		//indec = -(indec / 1.33333);
		voltable[lf] = amptable[lf] = (int)(127.0 * pow((float)lf/127.0,LN));
	
		//indec = 40 * log((float)lf / 127.0);
		//voltable[lf] = (int)(127.0 * exp(indec/40.0));
		
		//LOG_MSG("lf %d = vol %d", lf, voltable[lf]);
	}
	for(lf=0;lf<441;lf++) {
		int myRand;
		myRand = rand();
		myRand = ((myRand - 16383) * WGAMP) >> 18;
		smallnoise[lf] = (int16)myRand;
	}

	for(lf=0;lf<=100;lf++) {
		ptable[lf] = (int)(pow((float)2,(float)((float)(lf-50)/25.0)) * 256);

	}
	float tdist;
	for(lf=0;lf<=50;lf++) {
		if(lf==0)
			padjtable[lf] = 7;
		else if (lf==1)
			padjtable[lf] = 6;
		else if (lf==2)
			padjtable[lf] = 5;
		else if (lf==3)
			padjtable[lf] = 4;
		else if (lf==4)
			padjtable[lf] = 4-(.333333f);
		else if (lf==5)
			padjtable[lf] = 4-(.333333f*2);
		else if (lf==6)
			padjtable[lf] = 3;
		else if ((lf>6) && (lf<=12)) {
			tdist = (lf-6.0) / 6.0;
			padjtable[lf] = 3.0 - tdist;
		} else if ((lf>12) && (lf<=25)) {
			tdist = (lf-12.0) / 13.0;
			padjtable[lf] = 2.0 - tdist;

		} else {
			tdist = (lf-25.0) / 25.0;
			padjtable[lf] = 1.0 - tdist;
		}
		//LOG_MSG("lf %d = padj %f", lf, padjtable[lf]);
	}
	for(lf=0;lf<=100;lf++) {
		float mv = (float)lf / 100.0;
		float pt = mv-0.5;
		if(pt<0) pt = 0;

		pulsetable[lf] = (int)((pt) * 215.04) + 128;
		pulseoffset[lf] = (int)(pt * WGAMP);

		/*
		// I am certain of this:  Verified by hand LFO log */
			 lfotable[lf] = (int32)(((float)SETRATE) / (pow((float)1.088883372,(float)lf) * 0.021236044));
		
		//LOG_MSG("lf %d = lfo %d pulsetable %d", lf, lfotable[lf], pulsetable[lf]);
	}

	float lfp, depf, finalval, tlf;
	int depat, pval, depti;
	for(lf=0;lf<=10;lf++) {
		// I believe the depth is cubed or something
		
		for(depat=0;depat<=100;depat++) {
			if(lf>0) {
				depti = abs(depat-50);
				tlf = (float)lf - padjtable[depti];
				if(tlf<0) tlf = 0;
				lfp = exp(0.713619942 * tlf) / 407.4945111;
				
				if(depat<50)
					finalval = 4096.0 * pow((float)2,(float)-lfp);
				else
					finalval = 4096.0 * pow((float)2, (float)lfp);
				pval = (int)(finalval);

				penvtable[lf][depat] = pval;
			} else {
				penvtable[lf][depat] = 4096;

			}

			//LOG_MSG("lf %d depat %d pval %d tlf %f lfp %f", lf,depat,pval, tlf, lfp);
		
		}
	}
	for(lf=0;lf<=100;lf++) {
		// Maybe its linear
		// It is - verified on MT-32 - one of the few things linear
		lfp = ((float)lf * .1904) / 310.55;

		for(depat=0;depat<=100;depat++) {
			depf = ((float)depat - 50.0) / 50.0;
			//finalval = pow(2, lfp * depf * .5);
			finalval = 4096.0 + (4096.0 * lfp * depf);

			pval = (int)(finalval);

			lfoptable[lf][depat] = pval;

			//LOG_MSG("lf %d depat %d pval %x", lf,depat,pval);

		}
	}


	int distval;
	float amplog, dval;
	
	for(lf=0;lf<=12;lf++) {
		for(distval=0;distval<128;distval++) {
			if(lf==0) {
				amplog = 0;
				dval = 1;
				ampbiastable[lf][distval] = 256;
			} else {
				amplog = pow((float)1.431817011,(float)lf) / PI;
				dval = ((128.0-(float)distval)/128.0);
				amplog = exp(amplog);
				dval = pow(amplog,dval)/amplog;
				ampbiastable[lf][distval] = (int)(dval * 256.0);
			}
			//LOG_MSG("Ampbias lf %d distval %d = %f (%x) %f", lf, distval, dval, ampbiastable[lf][distval],amplog);
		}

		

	}

	for(lf=0;lf<=14;lf++) {
		for(distval=0;distval<128;distval++) {
			float filval = fabs((((float)lf - 7.0) * 12.0) / 7.0);

			if(lf==7) {
				amplog = 0;
				dval = 1;
				fbiastable[lf][distval] = 256;
			} else {
				//amplog = pow(1.431817011,filval) / PI;
				amplog = pow((float)1.531817011, (float)filval) / PI;
				dval = ((128.0-(float)distval)/128.0);
				amplog = exp(amplog);
				dval = pow(amplog,dval)/amplog;
				if(lf<8) {
					fbiastable[lf][distval] = (int)(dval * 256.0);
				} else {
					dval = pow((float)dval, (float).3333333);
					if(dval<.01) dval = .01f;
					dval = 1 / dval;
					fbiastable[lf][distval] = (int)(dval * 256.0);
				}
			}
			//LOG_MSG("Fbias lf %d distval %d = %f (%x) %f", lf, distval, dval, fbiastable[lf][distval],amplog);
		}

		

	}

	// Benchmark 3DNow, Floating point, and SSE filters
/*
	uint32 bench;
	__time64_t start, end;
	float histval[50];

	_time64(&start);
	for(bench=0;bench<20000000;bench++) {
	        iir_filter_sse(0,&histval[0],filtcoeff[0][0],0);
	}
	_time64(&end);
	//LOG_MSG("Bench completed in %ld seconds", end - start);
*/
	

#endif

	return true;
}

bool RecalcWaveforms(char * baseDir, int sampRate, recalcStatusCallback callBack) {

#ifdef NOMANSLAND

	File fp;
	fp.open("waveforms.raw", File::kFileWriteMode);

	if(!fp.isOpen()) return false;

	double ampsize = WGAMP;
	int f;
	for(f=12;f<109;f++) {
		
		if(callBack != NULL) {
			int perc = ((f - 12) * 100) / 96;
			(callBack)(perc);
		}

		//int octave = (f / 12) - 6;
		//int note = f % 12;

		//int cents = (octave * 100) + (note * 100);
		int freq = (int)(TUNING * pow( 2.0, ((double)f - 69.0) / 12.0 ));
		freqtable[f] = freq;
		divtable[f] = (int)(  ((float)sampRate / (float)freq) );
		smalldivtable[f] = divtable[f] << 8;
		divtable[f] = divtable[f] << 16;

		//int halfdiv = divtable[f] >> 1;
		
		//float period = ((float)sampRate / ((float)freq));
		//float m=2*(int)(period/2)+1.0f;
		//float k=(int)(((float)50.0/100.0)*period);
		
		
		double sd = (2.0*PI)/((float)divtable[f]/4096.0);
		double sa = 0.0;

		
		int fa=0;


		int j;
		float dumbfire;
		double square=0.0f;
		double saw = 0.0f;

		memset(tmpforms, 0,sizeof(tmpforms));

		while(sa<=(2.0*PI)) {
			float sqp;

			if(sa<PI) {
				sqp = -1;
				sqp = sqp + (.25 * (sa/PI));
			} else {
				sqp=1;
				sqp = sqp - (.25 * ((sa-PI)/PI));
			}

			square=0;
			saw = 0;
			//bool odd = true;
			for(double sinus=1.0;sinus<256.0;sinus++) {
				float sinusval = (((1/(sinus))*(sin(sinus*sa))));
				if((sinus*freq)<(sampRate*2)) saw=saw + sinusval;
			}
			
			dumbfire = sa /2 ;
			
			//This works pretty good
			tmpforms[2][fa>>4] +=  (int16)(cos(dumbfire) * -ampsize);
			tmpforms[3][fa>>3] +=  (int16)(cos(sa-PI) * -ampsize);
			
			tmpforms[0][fa>>4] +=  (int16)(saw * -ampsize);
			tmpforms[1][fa>>4] +=  (int16)(saw * ampsize);

			fa++;
			sa+=sd;
		}

		for(j=0;j<=(divtable[f]>>16);j++) {
			finalforms[0][j] = tmpforms[0][j] >> 5;
			finalforms[1][j] = tmpforms[1][j] >> 5;
			finalforms[2][j] = tmpforms[2][j] >> 4;
		}
		for(j=0;j<=(divtable[f]>>15);j++) {
			finalforms[3][j] = tmpforms[3][j] >> 3;
		}
		
		int out;
		
		out = fp.write(finalforms[0],divtable[f]>>15);
		out = fp.write(finalforms[1],divtable[f]>>15);
		out = fp.write(finalforms[2],divtable[f]>>15);
		out = fp.write(finalforms[3],divtable[f]>>14);


	}
	fp.close();
#endif

	return true;
}

bool CSynthMT32::ClassicOpen(const char *baseDir, SynthProperties useProp) {

#ifdef NOMANSLAND

	if (isOpen) return false;
	int i;
	// Initalize patch information
	uint8 sysexBuf[SYSEX_SIZE];
	uint16 syslen = 0;
	
	bool inSys = false;

	File fp;
	uint8 c;

	myProp = useProp;

	usefilter = &iir_filter_normal;


	for(i=0;i<MAXPARTIALS;i++) {
		partTable[i] = new CPartialMT32(i);
	}

	//pInfo = fopen("partial.nfo","wb");


	//LOG_MSG("MT-32 Initializing patch banks");

	for(initmode=0;initmode<2;initmode++) {

		switch(initmode) {
		case 0:
			fp.open("Preset1.syx");
			if(!fp.isOpen()) {
				// TODO : Fail driver init
				error("Unable to open Preset1.syx");
				return false;

				//exit(0);
			}
			break;
		case 1:
			fp.open("Preset2.syx");
			if(!fp.isOpen()) {
				// TODO : Fail driver init
				error("Unable to open Preset2.syx");
				return false;

				//exit(0);
			}
			break;
		default:

			// TODO : Fail driver init
			return false;

			break;
		}

		while(!fp.eof()) {
			c = fp.readByte();
			sysexBuf[syslen] = c;
			syslen++;
			if(c==0xf0) {
				inSys = true;
			}
			if ((c==0xf7) && (inSys)) {
				PlaySysex(&sysexBuf[0],syslen);
				inSys = false;
				syslen = 0;
			}
		}
		fp.close();

	}

	//LOG_MSG("MT-32 Initializing Drums");

	File fDrums;
	fDrums.open("drumpat.rom");
	if(!fDrums.isOpen()) {
		// TODO : Fail driver init
		// printf("MT-32 Init Error - Missing drumpat.rom\n");
		error("Unable to open drumpat.rom");
		return false;

		//exit(0);
	}
	int drumnum=0;
	while(!fDrums.eof()) {
		//Read common area
		fDrums.read(&drums[drumnum].common,14);
		int t;
		for(t=0;t<4;t++) {
			if (((drums[drumnum].common.pmute >> t) & 0x1) == 0x1) {
				fDrums.read(&drums[drumnum].partial[t],58);
				//LOG_MSG("Loaded drum #%d - t %d", drumnum,t);
			}
		}
		//LOG_MSG("Loaded drum #%d - %s", drumnum,drums[drumnum].common.name);
		drumnum++;
	}
	fDrums.close();

#if DUMPDRUMS == 1
	fp.open("drumsys.syx", File::kFileWriteMode);
	char dumbtext[10], tmpb;
	memset(dumbtext,0,10);
	for(drumnum=0;drumnum<30;drumnum++) {
		// Sysex header
		tmpb = 0xf0; fp.write(&tmpb,1);
		tmpb = 0x41; fp.write(&tmpb,1);
		tmpb = 0x10; fp.write(&tmpb,1);
		tmpb = 0x16; fp.write(&tmpb,1);
		tmpb = 0x12; fp.write(&tmpb,1);

		int useaddr = drumnum * 256;
		char lsb = useaddr & 0x7f;
		char isb = (useaddr >> 7) & 0x7f;
		char msb = ((useaddr >> 14) & 0x7f) | 0x08;
		//Address
		fp.write(&msb, 1);
		fp.write(&isb, 1);
		fp.write(&lsb, 1);
		unsigned int checksum = msb + isb + lsb;

		//Data
		fp.write(&drums[drumnum].common,0xe);
		fp.write(&drums[drumnum].partial[0],0x3a);
		fp.write(&drums[drumnum].partial[1],0x3a);
		fp.write(&drums[drumnum].partial[2],0x3a);
		fp.write(&drums[drumnum].partial[3],0x3a);
		//Checksum
		char *dat = (char *)&drums[drumnum];
		int ch;
		for(ch=0;ch<246;ch++) checksum += *dat++;
		checksum = (checksum & 0x7f);
		if(checksum) checksum = 0x80 - checksum;

		fp.write(&checksum,1);

		//End of sysex
		tmpb = 0xf7; fp.write(&tmpb,1);
	}
	fp.close();
#endif
	
	//LOG_MSG("MT-32 Initializing Partials");

	File fPatch;
	fPatch.open("patchlog.cfg");

	if(!fPatch.isOpen()) {
		// TODO : Fail driver init
		// printf("MT-32 Init Error - Missing patchlog.cfg\n");
		error("Unable to open patchlog.cfg");
		return false;

		//exit(0);
	}

	for(i=0;i<54;i++) {
		PCMReassign[i] = i;
		PCM[i].tune = 220.0;
		PCM[i].ampval = 256;

	}
	//PCM[53].ampval = 128;
	if (!fPatch.eof()) {
		char tbuf[512];
		char *cp;
		fPatch.gets(tbuf,sizeof(tbuf));
		uint32 patchstart = 0; //axtoi(tbuf);
		uint32 patchend = 0;
		uint32 patchcount = 0;
		//int16 *romaddr = &romfile[0];
		while (!fPatch.eof()) {
			fPatch.gets(tbuf,sizeof(tbuf));
			cp = strtok(tbuf," \n\r");
			PCM[patchcount].loop = false;
			if(cp != NULL) {
				patchend = axtoi(cp);
				cp = strtok(NULL," \n\r");
				if(cp != NULL) {
					cp = strtok(NULL," \n\r");
					if(cp != NULL) {
						cp = strtok(NULL," \n\r");
						if (cp !=NULL) {
							int newpcm = atoi(cp);
							PCMReassign[newpcm] = patchcount;
							cp = strtok(NULL," \n\r");
							if(cp != NULL) {
								if(atoi(cp)==1) PCM[patchcount].loop = true;
								cp = strtok(NULL," \n\r");
									if(cp != NULL) {
										PCM[patchcount].tune = (float)atoi(cp) / 100.0;
										//LOG_MSG("PCM %d tuning at %f", patchcount, PCM[patchcount].tune);
									}
							}
						}
					}
				}
			}
			if (patchend==0) break;

			PCM[patchcount].addr =  patchstart;
			PCM[patchcount].len = patchend - patchstart;
			patchcount++;
			//printf("Patch %d %d %d %d\n", patchcount, patchstart, patchend, mt32ram.PCM[patchcount].len);
			patchstart = patchend;
			
		}
	} else {
		// TODO : Fail driver init
		return false;

		//exit(0);
	}

	fPatch.close();


	PCM[53].len = 1950;

	int pat = 0;
	int p;
	for(p=0;p<54;p++) {
		int pr = PCMReassign[p];
		if(!PCM[pr].loop) {
			PCMLoopTable[pat] = p;
			pat++;
		}
		if(pat==7) {
			PCMLoopTable[pat] = p;
			pat++;
		}
	}
	// These are the special combination loop patches
	for(p=46;p<=54;p++) {
		PCMLoopTable[pat] = -(p - 45);
		pat++;
	}
	
	//for(p=0;p<54;p++) LOG_MSG("Loop table %d = %d (%d)", p, PCMLoopTable[p], PCM[p].loop);

	// Generate official PCM list

	// Normal sounds
	pat = 0;
	for(p=0;p<54;p++) {
		PCMList[pat].addr = PCM[PCMReassign[p]].addr;
		PCMList[pat].len = PCM[PCMReassign[p]].len;
		PCMList[pat].loop = PCM[PCMReassign[p]].loop;
		PCMList[pat].aggSound = -1;
		PCMList[pat].pcmnum = PCMReassign[p];
		PCMList[pat].ampval = PCM[PCMReassign[p]].ampval;
		pat++;
	}

	// Drum specific sounds.  Not exactly sure yet how these are different
	for(p=0;p<20;p++) {
		PCMList[pat] = PCMList[p];
		pat++;
	}
	
	// Looped PCM sounds.  The last remaining 9 are aggregate sounds;
	for(p=0;p<54;p++) {
		if(PCMLoopTable[p]>-1) {
			PCMList[pat].addr = PCM[PCMReassign[PCMLoopTable[p]]].addr;
			PCMList[pat].ampval = PCM[PCMReassign[PCMLoopTable[p]]].ampval;
			PCMList[pat].len = PCM[PCMReassign[PCMLoopTable[p]]].len;
			PCMList[pat].pcmnum = PCMReassign[PCMLoopTable[p]];
			PCMList[pat].loop = true;
			PCMList[pat].aggSound = -1;
		} else {
			PCMList[pat].addr = 0;

			//Calculate aggregate length
			int aggsnd = (-PCMLoopTable[p])-1;
			int tmplen = 0;
			int sndpos = 0;
			while(LoopPatterns[aggsnd][sndpos] != -1) {
				tmplen += PCM[LoopPatterns[aggsnd][sndpos]].len;
				sndpos++;
			}
			
			PCMList[pat].len = tmplen;
			PCMList[pat].loop = true;
			PCMList[pat].aggSound = aggsnd;
			PCMList[pat].ampval = 0x100;
		}
		pat++;

	}

	//for(p=0;p<128;p++) LOG_MSG("PCM #%d addr 0x%x len %d loop %d aggSound %d pcmnum %d", p, PCMList[p].addr, PCMList[p].len, PCMList[p].loop, PCMList[p].aggSound, PCMList[p].pcmnum);

	//LOG_MSG("MT-32 Initializing ROM");
	File fIn;
	fIn.open("MT32_PCM.ROM");
#ifdef MT32OUT
	File fOut, fOutb;
	char tmpc;
	fOut.open("mt32out.raw",File::kFileWriteMode);
	fOutb.open("mt32out2.raw",File::kFileWriteMode);
#endif
	
	if(!fIn.isOpen()) {
		// TODO : Fail driver init
		// printf("MT-32 Init Error - Missing MT32_PCM.ROM\n");
		error("Unable to open MT32_PCM.ROM");
		return false;

		//exit(0);
	}
	i=0;
	//int32 maxamp = 0;
	while (!fIn.eof()) {
		int16 s, c1;
		
		s = fIn.readByte();
		c1 = fIn.readByte();

		/*
		int e,z,u,bit;
		
		int order[16] = {0, 9,1 ,2, 3, 4, 5, 6, 7, 10, 11,  12,13, 14, 15,8};
		
		e=0;
		z = 15;
		for(u=0;u<15;u++) {
			if((order[u]<8) && (order[u]>=0)) {
				bit = (s >> (7-order[u])) & 0x1;
			} else {
				if(order[u]>=8) {
					bit = (c1 >> (7-(order[u]-8))) & 0x1;
				} else {
					bit = 0;
				}
			}
			e = e | (bit << z);
			--z;
		}

		//if( (e & 0x1) != 0) printf("Last bit = %d\n", e & 0x1);
		//int16 e = (  ((s & 0x7f) << 4) | ((c1 & 0x40) << 6) | ((s & 0x80) << 6) | ((c1 & 0x3f))) << 2;
		if(e<0) e = -32767 - e;
		int ut = abs(e);
		int dif = 0x7fff - ut;
		x = exp(((float)((float)0x8000-(float)dif) / (float)0x1000));
		e = (int)((float)e * (x/3200));*/

		short e;
		int z;
		int bit;
		int u;
	
		static const int order[16] = {0, 9,1 ,2, 3, 4, 5, 6, 7, 10, 11,  12,13, 14, 15,8};
		
		e=0;
		z = 15;
		for(u=0;u<15;u++) {
			if((order[u]<8) && (order[u]>=0)) {
				bit = (s >> (7-order[u])) & 0x1;
			} else {
				if(order[u]>=8) {
					bit = (c1 >> (7-(order[u]-8))) & 0x1;
				} else {
					bit = 0;
				}
			}
			e = e | (bit << z);
			--z;
		}

#ifdef MT32OUT
		tmpc = e & 0xff; fOut.write(&tmpc, 1);
		tmpc = (e >> 8) & 0x7f; fOut.write(&tmpc, 1);
#endif		
		// File is encoded in dB, convert to PCM
		// MINDB = -96
		// MAXDB = -15
		float testval;
		testval = (float)((~e) & 0x7fff);
		testval = -(testval / 400.00);
		//testval = -(testval / 341.32291666666666666666666666667);
		//testval = -(testval / 400.00);
		float vol = pow((float)8,(float)(testval / 20)) * 32767;
		
		if (e>0) vol = -vol;
	
		romfile[i] = (int16)vol;

#ifdef MT32OUT
		tmpc = (int16)vol & 0xff; fOutb.write(&tmpc, 1);
		tmpc = (int16)vol >> 8; fOutb.write(&tmpc, 1);
#endif		

		i++;
	}
	//LOG_MSG("PCM amp = %d", maxamp);
#ifdef MT32OUT
	fOutb.close();
	fOut.close();
#endif
	fIn.close();
	int tmp;
	for(tmp=0;tmp<16;tmp++) {
		if((tmp>=1) && (tmp<=9)) {
			chantable[tmp] = tmp-1;
		} else {
			chantable[tmp] = -1;
		}
	}
	chantable[10] = 8;
	for(i=0;i<128;i++) {
		mt32ram.params.pSettings[i].timbreGroup = i >> 6;
		mt32ram.params.pSettings[i].timbreNum = i & 63;
	}

	// For resetting mt32 mid-execution
	memcpy(&mt32default, &mt32ram, sizeof(mt32ram));

	if (!InitTables(baseDir)) return false;
	if(myProp.UseDefault) {
		InitReverb(0,5,SETRATE);
	} else {
		InitReverb(myProp.RevType, myProp.RevTime,SETRATE);
	}

	for(i=0;i<10;i++) {
		mchan[i] = new MidiChannel(SETRATE,i);

		if(i<8) mchan[i]->SetPatch(InitInstr[i],-1);
		if(i>8) mchan[i]->SetPatch(InitInstr[i-9],-1);
	}
	activeChannels = 0;

#ifdef HAVE_X86
	bool useSSE = false, use3DNow = false;

	use3DNow = Detect3DNow();
	useSSE = DetectSIMD();
	
	if (useSSE) debug(1, "MT-32: SSE detected and enabled");
	if (use3DNow) debug(1, "MT-32: 3DNow! detected and enabled");
	
	if(use3DNow) {
		debug(1, "MT-32 using use SIMD (AMD 3DNow) extensions");
		usefilter = &iir_filter_3dnow;
	}

	if(useSSE) {
		debug(1, "MT-32 using SIMD (Intel SSE) extensions\n");
		usefilter = &iir_filter_sse;
		usingSIMD = true;
	}
#endif

	isOpen=true;
	isEnabled=false;

#endif

	return true;
}

void CSynthMT32::Close(void) {
	if (!isOpen) return;

#ifdef NOMANSLAND
	int t, m;
	for(t=0;t<4;t++) {
		for(m=0;m<256;m++) {
			if(waveforms[t][m]!=NULL) free(waveforms[t][m]);
		}
	}

#endif

	isOpen=false;


}

void CSynthMT32::PlayMsg(uint32 msg) {

#ifdef NOMANSLAND

	int h;
	int code = msg & 0xf0;
	int chan = msg & 0xf;
	int note = (msg & 0xff00) >> 8;
	int velocity = (msg & 0xff0000) >> 16;
	isEnabled= true;
	
	//if(chan!=0x9) {
	//	if(chan==12) return;
	//	chan = chan & 0x7;
	//
	//} else {
	//	chan = 8;
	//}
	//if (chan==0) return;
	//int prechan = chan;
	//if(code!=0xf0) LOG_MSG("Playing chan %d, code 0x%x note: 0x%x", chan, code, note);
	
	chan = chantable[chan];
	//LOG_MSG("Play msg on chan: %d = %d note: %x velocity: %x", chan, msg & 0xf, note, velocity);
	if(chan<0) {
		//LOG_MSG("Play msg 0x%x on unreg chan: %d = %x", chan, msg & 0xf);
		return;
	
	}
	if(chan>8) return;


	int patch;
	uint32 bend;

	//LOG_MSG("Midi code: 0x%x",msg);
	switch (code) {
	case 0x80:
		//LOG_MSG("Note OFF - Channel %d",chan);
		mchan[chan]->StopNote(note,velocity);
		break;
	case 0x90:
		//if(chan!=4) return;
		//LOG_MSG("Note ON - Channel %d, Note %d Vel %d",chan, note, velocity);
		
		if(velocity>0) {
			mchan[chan]->PlayNote(note,velocity);
		} else {
			mchan[chan]->StopNote(note,velocity);
		}


		break;
	case 0xb0: // Control change
		switch (note) {
		case 0x1:  // Modulation
			//LOG_MSG("Modulation: %d", velocity);
			mchan[chan]->SetModulation(velocity);
			break;
		case 0xb:
			//LOG_MSG("Expression set: %d", velocity);
			mchan[chan]->SetVolume(velocity);
			break;
		case 0x7:  // Set volume
			//if(chan!=3) return;
			//LOG_MSG("Volume set: %d", velocity);
			mchan[chan]->SetVolume(velocity);
			break;
		case 0xa:  // Pan
			mchan[chan]->SetPan(velocity);
			break;
		case 0x40: // Hold pedal
			if(velocity<64) {
				mchan[chan]->SetHoldPedal(false);
				mchan[chan]->StopPedalHold();
			} else {
				mchan[chan]->SetHoldPedal(true);
			}
			break;

		case 0x7b: // All notes off
			
			for(h=0;h<MAXPOLY;h++) {
				mchan[chan]->StopNote(-1,0);
			}
			break;
		case 0x79: // Reset all controllers
			break;

		default:
			//LOG_MSG("Control code: 0x%x - vel %x",note, velocity);
			break;
		}

		break;
	case 0xc0: // Patch setting
		char currentInstr[32];
		patch = (mt32ram.params.pSettings[note].timbreGroup * 64) + mt32ram.params.pSettings[note].timbreNum;
		memset(&currentInstr,0,16);
		memcpy(&currentInstr,mt32ram.params.patch[patch].common.name,10);

		//LOG_MSG("Set patch (%s) %d (%d) chan %d (%d) from ng %d, t %d", currentInstr, patch, note, chan, msg & 0xf, mt32ram.params.pSettings[note].timbreGroup, mt32ram.params.pSettings[note].timbreNum);
		if((chan>=0) && (chan<8)) mt32ram.params.timTemp[chan] = mt32ram.params.patch[patch];
		mchan[chan]->SetPatch(note,-1);
		break;
	case 0xe0: // Pitch bender
		bend = (velocity << 7) | (note);
		//LOG_MSG("Pitch bender %x", bend);
		mchan[chan]->SetBend(bend);
		break;
	default:
		//LOG_MSG("Undef Midi code: 0x%x - %x - %x",code, note, velocity);

		break;
	}

#endif
	//midiOutShortMsg(m_out, msg);
}

void CSynthMT32::PlaySysex(uint8 * sysex,uint32 len) {

#ifdef NOMANSLAND

	uint32 addr;
	uint32 *header;
	unsigned int off;
	int m;
	header = (uint32 *)(sysex+1);
	//int dummy = 0;                                                                  
	int32 lens = len;                                                              


	// For some reason commands in IMuseInternal::initMT32 do not have prefix byte
	if(READ_LE_UINT32(header) != 0x12161041) {
		if(READ_LE_UINT32(sysex) == 0x12161041) {
			header = (uint32 *)sysex;
			sysex--; // We don't access sysex[0], so it's safe
		}
	}

	if(READ_LE_UINT32(header) == 0x12161041) {
		addr = (sysex[5] << 16) | (sysex[6] << 8) | (sysex[7]);
		//LOG_MSG("Sysex addr: %x", addr);
		if (addr<0x30000) {
			//LOG_MSG("Channel temp area %x", addr);
		}
		if ((addr>=0x30000) && (addr<0x40000)) {
			off = ((addr & 0x7f00) >> 1) | (addr & 0xff);
			for(m=0;m<(lens-10);m++) {
				mt32ram.memabs.mt32memory[off+m] = sysex[8+m];
			}
			//LOG_MSG("Patch temp %d at %x - len %d", off/16, off % 16, len-10);

			if(initmode>1) {
				for(m=0;m<8;m++) {
					int tmppat;
					for(tmppat=0;tmppat<128;tmppat++) {
						if(mt32ram.params.pSettings[tmppat].timbreGroup == mt32ram.params.tmpSettings[m].timbreGroup) {
							if(mt32ram.params.pSettings[tmppat].timbreNum == mt32ram.params.tmpSettings[m].timbreNum) {
								//LOG_MSG("Setting %d to patch %d", m, tmppat);
								this->PlayMsg((tmppat << 8) | 0xc0 | m);

								break;
							}
						}
					}
					//LOG_MSG("Patch chan %d - Assign mode %d", m,mt32ram.params.tmpSettings[m].fineTune);
				}
			}
		}
		if ((addr>=0x40000) && (addr<0x50000)) {
			int toffat = sizeof(mt32ram.patchabs.pTemp) + sizeof(mt32ram.patchabs.rTemp);
			off = ((addr & 0x7f00) >> 1) | (addr & 0x7f);
			for(m=0;m<(lens-10);m++) {
				mt32ram.memabs.mt32memory[off+m+toffat] = sysex[8+m];
			}
			int chanoff = off / sizeof(timbreParam);

			//LOG_MSG("Timbre temp off %x offdiv %x - len %d", off, chanoff, len-10);
			if(mchan[chanoff]!=NULL) mchan[chanoff]->RefreshPatch();
		}

		if ((addr>=0x50000) && (addr<0x60000)) {
			off = (((addr & 0x7f00) >> 1) | (addr & 0xff)) + sizeof(mt32ram.patchabs.pTemp) + sizeof(mt32ram.patchabs.rTemp) + sizeof(mt32ram.patchabs.tTemp);

			for(m=0;m<(lens-10);m++) {
				mt32ram.memabs.mt32memory[off+m] = sysex[8+m];
			}
			//LOG_MSG("Patch area %d, assigning to %d, patch %d - len %d", off/16, mt32ram.params.pSettings[off/16].timbreGroup, mt32ram.params.pSettings[off/16].timbreNum, len-10);
		}
		if ((addr>=0x80000) && (addr<0x90000)) {
			// Timbre patches
			int tc = (addr & 0xfe00) >> 9;
			off = ((addr & 0x100) >> 1) | (addr & 0xff);
			int calcoff;
			int pn=0;
		
			switch(initmode) {
			case 0:
				calcoff = tc * sizeof(timbreParam);
				pn = tc;
				break;
			case 1:
				calcoff = (tc+ 64) * sizeof(timbreParam);
				pn = tc + 64;
				break;
			default:
				calcoff = (tc + 128) * sizeof(timbreParam);
				pn = tc + 128;
				break;
			}

			// Transfer sysex parameter data into MT-32 memory
			calcoff += (off + sizeof(mt32ram.patchabs.pTemp) + sizeof(mt32ram.patchabs.rTemp) + sizeof(mt32ram.patchabs.tTemp) + sizeof(mt32ram.patchabs.patchmemory));
			for(m=0;m<(lens-10);m++) {
				mt32ram.memabs.mt32memory[calcoff+m] = sysex[8+m];
			}
			//LOG_MSG("Loaded patch %s at pn %d", mt32ram.params.patch[pn].common.name, pn);
		}
		if ((addr>=0x100000) && (addr<=0x1fffff)) {
			off = ((addr & 0x100) >> 1) | (addr & 0xff);
			for(m=0;m<(lens-10);m++) {
				mt32ram.patchabs.systemBank[m+off] = sysex[8+m];
			}
			
			//LOG_MSG("System Reconfiguration:");
			memset(chantable,-1,sizeof(chantable));
			memset(miditable,-1,sizeof(miditable));

			for(m=0;m<9;m++) {
				//LOG_MSG("Channel %d set to MIDI channel %d",m,mt32ram.params.system.chanAssign[m]);
				if(mt32ram.params.system.chanAssign[m]==16) {
					mchan[m]->AllStop();
				} else {
					chantable[(int)mt32ram.params.system.chanAssign[m]]=m;
					miditable[m] = mt32ram.params.system.chanAssign[m];
				}
			}

			//LOG_MSG("Master Tune: %f", ((float)mt32ram.params.system.masterTune)*0.2+432.1);
			//LOG_MSG("Reverb mode: %d", mt32ram.params.system.reverbMode);
			//LOG_MSG("Reverb time: %d", mt32ram.params.system.reverbTime);
			//LOG_MSG("Reverb level: %d", mt32ram.params.system.reverbLevel);
			
			if(((uint32)mt32ram.params.system.reverbMode != curRevMode) || ((uint32)mt32ram.params.system.reverbTime!=curRevTime)) {
				if(myProp.UseDefault) {
					InitReverb(mt32ram.params.system.reverbMode, mt32ram.params.system.reverbTime,SETRATE);
					curRevLevel = mt32ram.params.system.reverbLevel;
				} else {
					InitReverb(myProp.RevType, myProp.RevTime,SETRATE);
					curRevLevel = myProp.RevLevel;
				}
			}

			
			char *rset = mt32ram.params.system.reserveSettings;
			//LOG_MSG("Partial reserve: 1=%d 2=%d 3=%d 4=%d 5=%d 6=%d 7=%d 8=%d 9=%d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
			int x,y,pr;
			pr = 0;
			for(x=0;x<9;x++) {
				for(y=0;y<rset[x];y++) {
					PartialReserveTable[pr] = x;
					pr++;
				}
			}
			//if(pr != 32) LOG_MSG("Partial Reserve Table with less than 32 partials reserved!");
			rset = mt32ram.params.system.chanAssign;
			//LOG_MSG("Chan assign: 1=%d 2=%d 3=%d 4=%d 5=%d 6=%d 7=%d 8=%d 9=%d", rset[0], rset[1], rset[2], rset[3], rset[4], rset[5], rset[6], rset[7], rset[8]);
			//LOG_MSG("Master volume: %d",mt32ram.params.system.masterVol);
			int16 tv = (int16)((float)mt32ram.params.system.masterVol * 327.0);
			mastervolume = tv;
			
		}
		if (addr==0x200000) {
			char buf[SYSEX_SIZE];
			memset(&buf,0,SYSEX_SIZE);
			memcpy(&buf,&sysex[8],lens-10);
			//LOG_MSG("MT-32 LCD Display: %s", buf);
			g_system->displayMessageOnOSD(buf);
		}
		if ((addr & 0xff0000) == 0x7f0000) {
			//LOG_MSG("MT-32 Reset");
			for (uint32 m1=0;m1<MAXPARTIALS;m1++) partTable[m1]->isActive = false;

			memcpy(&mt32ram, &mt32default, sizeof(mt32ram));
			isEnabled = false;
		}


	} else {
		// Header not intended for Roland MT-32
	}

#endif

}


int CSynthMT32::DumpSysex(char *filename) {
	File fp;
	char tmpc;
	fp.open(filename,File::kFileWriteMode);
	if(!fp.isOpen())
		return -1;
			
	int patchnum;
	for(patchnum=0;patchnum<64;patchnum++) {
		// Sysex header
		tmpc = 0xf0; fp.write(&tmpc, 1);
		tmpc = 0x41; fp.write(&tmpc, 1);
		tmpc = 0x10; fp.write(&tmpc, 1);
		tmpc = 0x16; fp.write(&tmpc, 1);
		tmpc = 0x12; fp.write(&tmpc, 1);
		
		int useaddr = patchnum * 256;
		char lsb = useaddr & 0x7f;
		char isb = (useaddr >> 7) & 0x7f;
		char msb = ((useaddr >> 14) & 0x7f) | 0x08;
		//Address
		fp.write(&msb, 1);
		fp.write(&isb, 1);
		fp.write(&lsb, 1);
		unsigned int checksum = msb + isb + lsb;
		
		//Data
		fp.write(&mt32ram.params.patch[patchnum+128].common,0xe);
		fp.write(&mt32ram.params.patch[patchnum+128].partial[0],0x3a);
		fp.write(&mt32ram.params.patch[patchnum+128].partial[1],0x3a);
		fp.write(&mt32ram.params.patch[patchnum+128].partial[2],0x3a);
		fp.write(&mt32ram.params.patch[patchnum+128].partial[3],0x3a);
		//Checksum
		char *dat = (char *)&mt32ram.params.patch[patchnum+128];
		int ch;
		for(ch=0;ch<246;ch++) checksum += *dat++;
		checksum = (checksum & 0x7f);
		if(checksum) checksum = 0x80 - checksum;
		
		fp.write(&checksum,1);
		
		//End of sysex
		tmpc = 0xf7; fp.write(&tmpc, 1);
	}
	fp.close();
	//LOG_MSG("Wrote temp patches to %s", usefile);
		
	return 0;
}



static int16 tmpBuffer[4096];
static float sndbufl[4096];
static float sndbufr[4096];
static float outbufl[4096];
static float outbufr[4096];

#if USE_MMX == 3
static float multFactor[4] = { 32767.0, 32767.0, 32767.0, 32767.0 };
#endif

void CSynthMT32::MT32_CallBack(uint8 * stream,uint32 len, int volume) {

#ifdef NOMANSLAND
	int32 i,m;
	int16 *snd, *useBuf;
	uint32 outlen;
	snd = (int16 *)stream;
	memset(stream,0,len*4);
	if(!isEnabled) return;
	useBuf = snd;

	outlen = len;

	assert(len < 1024); // tmpBuffer is 4096 bytes
	/*
	partUsage outUsage;
	for(i=0;i<32;i++) {
		if(partTable[i]->isActive) {
			outUsage.active[i] = -1;
		} else {
			outUsage.active[i] = 0;
		}
		outUsage.owner[i] = partTable[i]->ownerChan;
		outUsage.assign[i] = PartialReserveTable[i];
	}
	fwrite(&outUsage,sizeof(outUsage),1,pInfo);*/

	for(i=0;i<MAXPARTIALS;i++) partTable[i]->age++;

	for(i=0;i<MAXPARTIALS;i++) {

		if(partTable[i]->produceOutput(tmpBuffer,outlen)==true) {
#if USE_MMX == 0
			int16 *tmpoff = snd;
			int q = 0;
			for(m=0;m<(int32)outlen;m++) {
				tmpoff[q] += (int16)(((int32)tmpBuffer[q] * (int32)mastervolume)>>15);
				q++;
				tmpoff[q] += (int16)(((int32)tmpBuffer[q] * (int32)mastervolume)>>15);
				q++;

			}			
#else
			int tmplen = (outlen >> 1) + 4;
#ifdef I_ASM
			__asm {
				mov ecx, tmplen
				mov ax,mastervolume
				shl eax,16
				mov ax,mastervolume
				movd mm3,eax
				movd mm2,eax
				psllq mm3, 32
				por mm3,mm2
				mov esi, useBuf
				mov edi, snd
mixloop4:
				movq mm1, [esi]
				movq mm2, [edi]
				pmulhw mm1, mm3
				paddw mm1,mm2
				movq [edi], mm1
						
				add esi,8
				add edi,8

				dec ecx
				cmp ecx,0
				jg mixloop4
				emms
			}
#else
			atti386_produceOutput1(tmplen, mastervolume, useBuf, snd);			
#endif
#endif
		}
	}

	if(myProp.UseReverb) {
#if USE_MMX == 3
		if(!usingSIMD) {
#endif
			m=0;
			for(i=0;i<(int32)len;i++) {
				sndbufl[i] = (float)snd[m] / 32767.0;
				m++;
				sndbufr[i] = (float)snd[m] / 32767.0;
				m++;
			}
			newReverb->processreplace(sndbufl, sndbufr, outbufl, outbufr, len, 1);
			m=0;
			for(i=0;i<(int32)len;i++) {
				snd[m] = (int16)(outbufl[i] * 32767.0);
				m++;
				snd[m] = (int16)(outbufr[i] * 32767.0);
				m++;
			}
#if USE_MMX == 3
		} else {
#ifdef I_ASM			
			// Use SIMD instructions to quickly convert between integer and floating point
			__asm {
				mov ecx, len
				shr ecx, 1
				add ecx, 4
				push ecx

				mov esi, multFactor
				movups xmm1, [esi]

				// One speaker at a time
				mov esi, snd
				mov edi, sndbufl

convloop1:
				xor eax,eax
				mov ax,[snd]
				cwde  // Sign extended ax
				inc snd
				inc snd
				movd mm1,eax
				psrlq mm1, 32
				mov ax,[snd]
				inc snd
				inc snd
				movd mm2,eax
				por mm1,mm2

				dec ecx
				jnz convloop1

				pop ecx
				mov esi, snd
				mov edi, sndbufr
				inc esi
convloop2:
				
				dec ecx
				jnz convloop2

			}
#else
			atti386_produceOutput2(len, snd, sndbufl, sndbufr, multFactor);
#endif			
		}
#endif
	}

	// for(i=0;i<9;i++) { mchan[i]->CheckNoteList(); }
	
	for(i=0;i<MAXPARTIALS;i++) { partTable[i]->alreadyOutputed = false; }
	

#if MAXPARTIALS == 0
	// Reorganize partials
	CPartialMT32 *tmpPartial;
	int y;
resetSearch:
	for(i=0;i<MAXPARTIALS;i++) {
		// Try to relocate partials not assigned to the right channel
		if((partTable[i]->isActive) && (partTable[i]->ownerChan != PartialReserveTable[i])) {
			for(y=0;y<MAXPARTIALS;y++) {
				if((!partTable[y]->isActive) && (PartialReserveTable[y] == partTable[i]->ownerChan)) {
					// Found a free channel that can use this partial - swap it out
					tmpPartial = partTable[y];
					partTable[y] = partTable[i];
					partTable[i] = tmpPartial;
					goto resetSearch;
				}
			}
		}
	}
#endif

#if MONITORPARTIALS == 1
	samplepos+=outlen;
	if(samplepos>SETRATE*5) {
		samplepos = 0;
		int partUse[9];
		memset(partUse,0,sizeof(partUse));
		for(i=0;i<MAXPARTIALS;i++) {
			if(partTable[i]->isActive) partUse[partTable[i]->ownerChan]++;
		}
		//LOG_MSG("C1: %d C2: %d C3: %d C4 %d", partUse[0], partUse[1], partUse[2], partUse[3]);
		//LOG_MSG("C5: %d C6: %d C7: %d C8 %d", partUse[4], partUse[5], partUse[6], partUse[7]);
		//LOG_MSG("Rythmn: %d", partUse[8]);

	}

#endif


#endif


}
