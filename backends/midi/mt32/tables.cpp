/* Copyright (c) 2003-2004 Various contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mt32emu.h"

// Determines how the waveform cache file is handled (must be regenerated after sampling rate change)
#define WAVECACHEMODE 0 // Load existing cache if possible, otherwise generate and save cache
//#define WAVECACHEMODE 1 // Load existing cache if possible, otherwise generage but don't save cache
//#define WAVECACHEMODE 2 // Ignore existing cache, generate and save cache
//#define WAVECACHEMODE 3 // Ignore existing cache, generate but don't save cache

// Constant tuning for now
#define TUNING 440.0f

namespace MT32Emu {

//Amplitude time velocity follow exponential coefficients
const double tvcatconst[5] = {0.0, 0.002791309, 0.005942882, 0.012652792, 0.026938637};
const double tvcatmult[5] = {1.0, 1.072662811, 1.169129367, 1.288579123, 1.229630539};

const Bit8s LoopPatterns[9][10] = {
	{ 2,  3, 4,   5,  6,  7, -1, -1, -1, -1},
	{ 8,  9, 10, 11, 12, 13, 14, 15, 16, -1},
	{17, 18, 19, 20, 21, -1, -1, -1, -1, -1},
	{22, 23, 24, 25, 26, 27, 28, 29, -1, -1},
	{30, 31, 32, 33, 34, 35, 36, 37, -1, -1},
	{45, 46, 47, 48, 49, 50, 51, 52, 53, -1},
	{15, 11, 12, 13, 14, 15, 16, -1, -1, -1},
	{30, 35, 32, 33, 34, -1, -1, -1, -1, -1},
	{ 2,  3, -1, -1, -1, -1, -1, -1, -1, -1},
};

static const Bit32s LoopPatternTuning[9][10] = {
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1,      -1,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1,      -1,      -1,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1},
	{0x2590B, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1,      -1},
	{0x1294A, 0x1294A, 0x1294A, 0x1294A, 0x1294A,      -1,      -1,      -1,      -1,      -1},
	{0x1294A, 0x1294A,      -1,      -1,      -1,      -1,      -1,      -1,      -1,      -1},
};

// These are division constants for the TVF depth key follow
static const Bit32u depexp[5] = {3000, 950, 485, 255, 138};

//Envelope time keyfollow exponential coefficients
static const double tkcatconst[5] = {0.0, 0.005853144, 0.011148054, 0.019086143, 0.043333215};
static const double tkcatmult[5] = {1.0, 1.058245688, 1.048488989, 1.016049301, 1.097538067};

static float initialisedSampleRate = 0.0f;

Bit16s smallnoise[MAX_SAMPLE_OUTPUT];

// Some optimization stuff
Bit32s keytable[217];
Bit16s sintable[65536];
Bit32u lfotable[101];
Bit32s penvtable[16][101];
Bit32s filveltable[128][101];
Bit32s veltkeytable[5][128];
Bit32s pulsetable[101];
Bit32s pulseoffset[101];
Bit32s ampbiastable[13][128];
Bit32s fbiastable[15][128];
float filtcoeff[FILTERGRAN][31][8];
Bit32s finetable[201];
Bit32u lfoptable[101][101];
Bit32s ampveltable[128][64];
Bit32s pwveltable[15][128];
Bit32s envtimetable[101];
Bit32s decaytimetable[101];
Bit32s lasttimetable[101];
Bit32s voltable[128];
Bit32s bendtable[49];
float ResonFactor[31];
float ResonInv[31];

// Per-note initialisation tables follow

Bit16s freqtable[NUMNOTES];
Bit32s fildeptable[5][NUMNOTES];
Bit32s timekeytable[5][NUMNOTES];
int filttable[2][NUMNOTES][201];
int nfilttable[NUMNOTES][101][101];

Bit32s divtable[NUMNOTES];
Bit32s smalldivtable[NUMNOTES];
Bit32u wavtabler[54][NUMNOTES];
Bit32u looptabler[9][10][NUMNOTES];
Bit32s sawtable[NUMNOTES][101];

Bit16s *waveforms[4][NUMNOTES];
Bit32u waveformsize[4][NUMNOTES];
static Bit16s tmpforms[4][65536];


// Begin filter stuff

// Pre-warp the coefficients of a numerator or denominator.
// Note that a0 is assumed to be 1, so there is no wrapping
// of it.
static void prewarp(double *a1, double *a2, double fc, double fs) {
	double wp;

	wp = 2.0 * fs * tan(PI * fc / fs);

	*a2 = *a2 / (wp * wp);
	*a1 = *a1 / wp;
}

// Transform the numerator and denominator coefficients
// of s-domain biquad section into corresponding
// z-domain coefficients.
//
//      Store the 4 IIR coefficients in array pointed by coef
//      in following order:
//             beta1, beta2    (denominator)
//             alpha1, alpha2  (numerator)
//
// Arguments:
//             a0-a2   - s-domain numerator coefficients
//             b0-b2   - s-domain denominator coefficients
//             k               - filter gain factor. initially set to 1
//                                and modified by each biquad section in such
//                                a way, as to make it the coefficient by
//                                which to multiply the overall filter gain
//                                in order to achieve a desired overall filter gain,
//                                specified in initial value of k.
//             fs             - sampling rate (Hz)
//             coef    - array of z-domain coefficients to be filled in.
//
// Return:
//             On return, set coef z-domain coefficients
static void bilinear(double a0, double a1, double a2, double b0, double b1, double b2, double *k, double fs, float *coef) {
	double ad, bd;

	// alpha (Numerator in s-domain)
	ad = 4. * a2 * fs * fs + 2. * a1 * fs + a0;
	// beta (Denominator in s-domain)
	bd = 4. * b2 * fs * fs + 2. * b1* fs + b0;

	// update gain constant for this section
	*k *= ad/bd;

	// Denominator
	*coef++ = (float)((2. * b0 - 8. * b2 * fs * fs) / bd);           // beta1
	*coef++ = (float)((4. * b2 * fs * fs - 2. * b1 * fs + b0) / bd); // beta2

	// Nominator
	*coef++ = (float)((2. * a0 - 8. * a2 * fs * fs) / ad);           // alpha1
	*coef = (float)((4. * a2 * fs * fs - 2. * a1 * fs + a0) / ad);   // alpha2
}

// a0-a2: numerator coefficients
// b0-b2: denominator coefficients
// fc: Filter cutoff frequency
// fs: sampling rate
// k: overall gain factor
// coef: pointer to 4 iir coefficients
static void szxform(double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, double fc, double fs, double *k, float *coef) {
	// Calculate a1 and a2 and overwrite the original values
	prewarp(a1, a2, fc, fs);
	prewarp(b1, b2, fc, fs);
	bilinear(*a0, *a1, *a2, *b0, *b1, *b2, k, fs, coef);
}

static void initFilter(float fs, float fc, float *icoeff, float Q) {
	float *coef;
	double a0, a1, a2, b0, b1, b2;

	double k = 1.5;    // Set overall filter gain factor
	coef = icoeff + 1; // Skip k, or gain

	// Section 1
	a0 = 1.0;
	a1 = 0;
	a2 = 0;
	b0 = 1.0;
	b1 = 0.765367 / Q; // Divide by resonance or Q
	b2 = 1.0;
	szxform(&a0, &a1, &a2, &b0, &b1, &b2, fc, fs, &k, coef);
	coef += 4;         // Point to next filter section

	// Section 2
	a0 = 1.0;
	a1 = 0;
	a2 = 0;
	b0 = 1.0;
	b1 = 1.847759 / Q;
	b2 = 1.0;
	szxform(&a0, &a1, &a2, &b0, &b1, &b2, fc, fs, &k, coef);

	icoeff[0] = (float)k;
}

static void initFiltCoeff(float samplerate) {
	for (int j = 0; j < FILTERGRAN; j++) {
		for (int res = 0; res < 31; res++) {
			float tres = ResonFactor[res];
			initFilter((float)samplerate, (((float)(j+1.0)/FILTERGRAN)) * ((float)samplerate/2), filtcoeff[j][res], tres);
		}
	}
}

static void initEnvelopes(float samplerate) {
	for (int lf = 0; lf <= 100; lf++) {
		float elf = (float)lf;

		// General envelope
		float logtime = elf * 0.088362939f;
		envtimetable[lf] = (int)((exp(logtime)/312.12) * (float)samplerate);

		// Decay envelope -- shorter for some reason
		// This is also the timing for the envelope right before the
		// amp and filter envelope sustains

		lasttimetable[lf] = decaytimetable[lf] = (int)((exp(logtime)/(312.12*2)) * (float)samplerate);
		//lasttimetable[lf] = (int)((exp(logtime)/(312.12*6)) * (float)samplerate);

		float mv = (float)lf / 100.0f;
		float pt = mv-0.5f;
		if (pt < 0)
			pt = 0;

		pulsetable[lf] = (int)((pt) * 215.04f) + 128;

		// I am certain of this:  Verified by hand LFO log
		lfotable[lf] = (Bit32u)(((float)samplerate) / (pow(1.088883372f,(float)lf) * 0.021236044f));

		//LOG(LOG_ERROR|LOG_MISC,"lf %d = lfo %d pulsetable %d", lf, lfotable[lf], pulsetable[lf]);
	}
}

void TableInitialiser::initMT32ConstantTables(Synth *synth) {
	if (initialisedSampleRate > 0.0f) {
		return;
	}
	int lf;
	synth->printDebug("Initialising Pitch Tables");
	for (lf = -108; lf <= 108; lf++) {
		keytable[lf + 108] = (int)(256 * powf(2.0f, (float)(lf / 24.0f)));
		//synth->printDebug("KT %d = %d", f, keytable[f+108]);
	}

	int res;
	float fres;
	for (res = 0; res < 31; res++) {
		fres = (float)res / 30.0f;
		ResonFactor[res] = (powf(2.0f, logf(powf(fres, 16.0f))) * 2.5f) + 1.0f;
		ResonInv[res] = 1 / ResonFactor[res];
	}

	int period = 65536;

	for (int ang = 0; ang < period; ang++) {
		int halfang = (period / 2);
		int angval  = ang % halfang;
		float tval = (((float)angval / (float)halfang) - 0.5f) * 2;
		if (ang >= halfang)
			tval = -tval;
		sintable[ang] = (Bit16s)(tval * 50.0f) + 50;
	}

	int velt, dep;
	float tempdep;
	for (velt = 0; velt < 128; velt++) {
		for (dep = 0; dep < 5; dep++) {
			if (dep > 0) {
				float ff = (float)(exp(3.5f*tvcatconst[dep] * (59.0f-(float)velt)) * tvcatmult[dep]);
				tempdep = 256.0f * ff;
				veltkeytable[dep][velt] = (int)tempdep;
				//if ((velt % 16) == 0) {
				//	synth->printDebug("Key %d, depth %d, factor %d", velt, dep, (int)tempdep);
				//}
			} else
				veltkeytable[dep][velt] = 256;
		}

		for (dep = -7; dep < 8; dep++) {
			float fldep = (float)abs(dep) / 7.0f;
			fldep = powf(fldep,2.5f);
			if (dep < 0)
				fldep = fldep * -1.0f;
			pwveltable[dep+7][velt] = Bit32s((fldep * (float)velt * 100) / 128.0);
		}
	}

	for (dep = 0; dep <= 100; dep++) {
		for (velt = 0; velt < 128; velt++) {
			float fdep = (float)dep * 0.000347013f; // Another MT-32 constant
			float fv = ((float)velt - 64.0f)/7.26f;
			float flogdep = powf(10, fdep * fv);
			float fbase;

			if (velt > 64)
				filveltable[velt][dep] = (int)(flogdep * 256.0);
			else {
				//lff = 1 - (pow(((128.0 - (float)lf) / 64.0),.25) * ((float)velt / 96));
				fbase = 1 - (powf(((float)dep / 100.0f),.25f) * ((float)(64-velt) / 96.0f));
				filveltable[velt][dep] = (int)(fbase * 256.0);
			}
			//synth->printDebug("Filvel dep %d velt %d = %x", dep, velt, filveltable[velt][dep]);
		}
	}

	for (lf = 0; lf <= 200; lf++) {
		//FIXME:KG: I'm fairly sure this is wrong... lf=100 should yield no fine-tuning (4096)?
		finetable[lf] = (int)((powf(2.0f, (((float)lf / 200.0f) - 1.0f) / 12.0f)) * 4096.0f);
	}
	for (lf = 0; lf <= 48; lf++)
		bendtable[lf] = (int)((powf(2.0f, (((float)lf / 12.0f) - 2.0f))) * 4096.0f);

	float lff;
	for (lf = 0; lf < 128; lf++) {
		for (velt = 0; velt < 64; velt++) {
			lff = 1 - (powf(((128.0f - (float)lf) / 64.0f), 0.25f) * ((float)velt / 96));
			ampveltable[lf][velt] = (int)(lff * 256.0);
			//synth->printDebug("Ampveltable: %d, %d = %d", lf, velt, ampveltable[lf][velt]);
		}
	}

	for (lf = 0; lf < 128; lf++) {
		// Converts MIDI velocity to volume.
		voltable[lf] = (int)(127.0 * pow((float)lf / 127.0, LN));
	}
	for (lf = 0; lf < MAX_SAMPLE_OUTPUT; lf++) {
		int myRand;
		myRand = rand();
		int origRand = myRand;
		//myRand = ((myRand - 16383) * WGAMP) >> 16;
		// This one is slower but works with all values of RAND_MAX
		myRand = (int)((origRand - RAND_MAX / 2) / (float)RAND_MAX * (WGAMP / 2));
		//FIXME:KG: Original ultimately set the lowest two bits to 0, for no obvious reason
		smallnoise[lf] = (Bit16s)myRand;
	}

	float tdist;
	float padjtable[51];
	for (lf = 0; lf <= 50; lf++) {
		if (lf == 0)
			padjtable[lf] = 7;
		else if (lf == 1)
			padjtable[lf] = 6;
		else if (lf == 2)
			padjtable[lf] = 5;
		else if (lf == 3)
			padjtable[lf] = 4;
		else if (lf == 4)
			padjtable[lf] = 4 - (0.333333f);
		else if (lf == 5)
			padjtable[lf] = 4 - (0.333333f * 2);
		else if (lf == 6)
			padjtable[lf] = 3;
		else if ((lf > 6) && (lf <= 12)) {
			tdist = (lf-6.0f) / 6.0f;
			padjtable[lf] = 3.0f - tdist;
		} else if ((lf > 12) && (lf <= 25)) {
			tdist = (lf - 12.0f) / 13.0f;
			padjtable[lf] = 2.0f - tdist;
		} else {
			tdist = (lf - 25.0f) / 25.0f;
			padjtable[lf] = 1.0f - tdist;
		}
		//synth->printDebug("lf %d = padj %f", lf, padjtable[lf]);
	}

	float lfp, depf, finalval, tlf;
	int depat, pval, depti;
	for (lf = 0; lf <= 10; lf++) {
		// I believe the depth is cubed or something

		for (depat = 0; depat <= 100; depat++) {
			if (lf > 0) {
				depti = abs(depat - 50);
				tlf = (float)lf - padjtable[depti];
				if (tlf < 0)
					tlf = 0;
				lfp = expf(0.713619942f * tlf) / 407.4945111f;

				if (depat < 50)
					finalval = 4096.0f * powf(2, -lfp);
				else
					finalval = 4096.0f * powf(2, lfp);
				pval = (int)finalval;

				penvtable[lf][depat] = pval;
				//synth->printDebug("lf %d depat %d pval %d tlf %f lfp %f", lf,depat,pval, tlf, lfp);
			} else {
				penvtable[lf][depat] = 4096;
				//synth->printDebug("lf %d depat %d pval 4096", lf, depat);
			}
		}
	}
	for (lf = 0; lf <= 100; lf++) {
		// It's linear - verified on MT-32 - one of the few things linear
		lfp = ((float)lf * 0.1904f) / 310.55f;

		for (depat = 0; depat <= 100; depat++) {
			depf = ((float)depat - 50.0f) / 50.0f;
			//finalval = pow(2, lfp * depf * .5);
			finalval = 4096.0f + (4096.0f * lfp * depf);

			pval = (int)finalval;

			lfoptable[lf][depat] = pval;

			//synth->printDebug("lf %d depat %d pval %x", lf,depat,pval);
		}
	}

	for (lf = 0; lf <= 12; lf++) {
		for (int distval = 0; distval < 128; distval++) {
			float amplog, dval;
			if (lf == 0) {
				amplog = 0;
				dval = 1;
				ampbiastable[lf][distval] = 256;
			} else {
				amplog = powf(1.431817011f, (float)lf) / (float)PI;
				dval = ((128.0f - (float)distval) / 128.0f);
				amplog = expf(amplog);
				dval = powf(amplog, dval) / amplog;
				ampbiastable[lf][distval] = (int)(dval * 256.0);
			}
			//synth->printDebug("Ampbias lf %d distval %d = %f (%x) %f", lf, distval, dval, ampbiastable[lf][distval],amplog);
		}
	}

	for (lf = 0; lf <= 14; lf++) {
		for (int distval = 0; distval < 128; distval++) {
			float filval = fabsf((float)((lf - 7) * 12) / 7.0f);
			float amplog, dval;
			if (lf == 7) {
				amplog = 0;
				dval = 1;
				fbiastable[lf][distval] = 256;
			} else {
				//amplog = pow(1.431817011, filval) / (float)PI;
				amplog = powf(1.531817011f, filval) / (float)PI;
				dval = (128.0f - (float)distval) / 128.0f;
				amplog = expf(amplog);
				dval = powf(amplog,dval)/amplog;
				if (lf < 8) {
					fbiastable[lf][distval] = (int)(dval * 256.0f);
				} else {
					dval = powf(dval, 0.3333333f);
					if (dval < 0.01f)
						dval = 0.01f;
					dval = 1 / dval;
					fbiastable[lf][distval] = (int)(dval * 256.0f);
				}
			}
			//synth->printDebug("Fbias lf %d distval %d = %f (%x) %f", lf, distval, dval, fbiastable[lf][distval],amplog);
		}
	}
}

// Per-note table initialisation follows

static void initSaw(int f, Bit32s div) {
	for (int rsaw = 0; rsaw <= 100; rsaw++) {
		float fsaw;
		if (rsaw < 50)
			fsaw = 50.0f;
		else
			fsaw = (float)rsaw;
		int tmpdiv = div << 1;

		//(66 - (((A8 - 50) / 50) ^ 0.63) * 50) / 132
		float sawfact = (66.0f - (powf((fsaw - 50.0f) / 50.0f, 0.63f) * 50.0f)) / 132.0f;
		sawtable[f][rsaw] = (int)(sawfact * (float)tmpdiv) >> 16;
		//synth->printDebug("F %d divtable %d saw %d sawtable %d", f, div>>16, rsaw, sawtable[f][rsaw]);
	}
}

static void initDep(int f) {
	for (int dep = 0; dep < 5; dep++) {
		if (dep == 0) {
			fildeptable[dep][f] = 256;
			timekeytable[dep][f] = 256;
		} else {
			float depfac = 3000.0f;
			float ff, tempdep;
			depfac = (float)depexp[dep];

			ff = ((float)f - (float)MIDDLEC) / depfac;
			tempdep = powf(2, ff) * 256.0f;
			fildeptable[dep][f] = (int)tempdep;

			ff = (float)(exp(tkcatconst[dep] * ((float)MIDDLEC-(float)f)) * tkcatmult[dep]);
			timekeytable[dep][f] = (int)(ff * 256.0f);
		}
	}
	//synth->printDebug("F %d d1 %x d2 %x d3 %x d4 %x d5 %x", f, fildeptable[0][f],fildeptable[1][f],fildeptable[2][f],fildeptable[3][f],fildeptable[4][f]);
}

void TableInitialiser::initWave(Synth *synth, File *file, bool reading, bool writing, int f, float freq, float rate, double ampsize, Bit32s div) {
	double sd = (2.0*PI)/((((float)div/65536.0)) * 4.0);
	waveformsize[0][f] = div >> 14;
	waveformsize[1][f] = div >> 14;
	waveformsize[2][f] = div >> 14;
	waveformsize[3][f] = div >> 13;
	for (int i = 0; i < 4; i++)
		waveforms[i][f] = new Bit16s[waveformsize[i][f]];
	if (reading) {
		for (int i = 0; i < 4; i++) {
			size_t len = waveformsize[i][f] * sizeof(Bit16s);
			if (file->read(waveforms[i][f], len) != len) {
				synth->printDebug("Error reading wave file cache!");
				break;
			}
		}
	} else {
		float dumbfire;
		double saw = 0.0f;
		double sa = 0.0;
		int fa = 0;
		memset(tmpforms, 0,sizeof(tmpforms));

		while (sa <= (2.0 * PI)) {
			float sqp;

			if (sa < PI) {
				sqp = -1;
				sqp = (float)(sqp + (0.25 * (sa/PI)));
			} else {
				sqp=1;
				sqp = (float)(sqp - (0.25 * ((sa-PI)/PI)));
			}

			saw = 0;
			for (int sinus = 1; sinus * freq < rate; sinus++) {
				double fsinus = (double)sinus;
				saw += (1 / fsinus) * sin(fsinus * sa);
			}

			dumbfire = (float)(sa / 2);

			//This works pretty good
			tmpforms[0][fa] = (Bit16s)(saw * -ampsize / 2);
			tmpforms[1][fa] = (Bit16s)(saw * ampsize / 2);

			tmpforms[2][fa] = (Bit16s)(cos(dumbfire) * -ampsize);
			tmpforms[3][fa * 2] = (Bit16s)(cos(sa - PI) * -ampsize);
			tmpforms[3][fa * 2 + 1] = (Bit16s)(cos((sa + (sd / 2)) - PI) * -ampsize);

			fa++;
			sa += sd;
		}

		//synth->printDebug("f num %d freq %f and fa %d, div>>13=%d", f, freq, fa, div>>13);

		for (int i = 0; i < 4; i++)
			memcpy(waveforms[i][f], tmpforms[i], waveformsize[i][f] * sizeof (Bit16s));

		if (writing) {
			for (int i = 0; i < 4; i++) {
				int len = waveformsize[i][f] * sizeof(Bit16s);
				if (!file->write(waveforms[i][f], len)) {
					synth->printDebug("Error writing waveform cache file");
					break;
				}
			}
		}
	}
}

static void initFiltTable(int f, float freq, float rate) {
	for (int tr = 0; tr <= 200; tr++) {
		float ftr = (float)tr;

		// Verified exact on MT-32
		if (tr > 100)
			ftr = 100.0f + (powf((ftr - 100.0f) / 100.0f, 3.0f) * 100.0f);

		// I think this is the one
		float brsq = powf(10.0f, (tr / 50.0f) - 1.0f);
		filttable[0][f][tr] = (int)((freq * brsq) / (rate / 2) * FILTERGRAN);
		if (filttable[0][f][tr]>=((FILTERGRAN*15)/16))
			filttable[0][f][tr] = ((FILTERGRAN*15)/16);

		float brsa = powf(10.0f, ((tr / 55.0f) - 1.0f)) / 2.0f;
		filttable[1][f][tr] = (int)((freq * brsa) / (rate / 2) * FILTERGRAN);
		if (filttable[1][f][tr]>=((FILTERGRAN*15)/16))
			filttable[1][f][tr] = ((FILTERGRAN*15)/16);
	}
}

static void initNFiltTable(int f, float freq, float rate) {
	for (int cf = 0; cf <= 100; cf++) {
		float cfmult = (float)cf;

		for (int tf = 0;tf <= 100; tf++) {
			float tfadd = (float)(tf - 0);
			if (tfadd < 0)
				tfadd = 0;

			float freqsum = expf((cfmult + tfadd) / 30.0f) / 4.0f;

			nfilttable[f][cf][tf] = (int)((freq * freqsum) / (rate / 2)*FILTERGRAN);
			if (nfilttable[f][cf][tf] >= ((FILTERGRAN * 15) / 16))
				nfilttable[f][cf][tf] = ((FILTERGRAN * 15) / 16);
		}
	}
}

void TableInitialiser::initNotes(Synth *synth, sampleFormat pcms[54], float rate) {
	char filename[32];
	int intRate = (int)rate;
	sprintf(filename, "waveformcache-%d.raw", intRate);

	File *file = NULL;
	bool reading = false, writing = false;
	char header[16];
	strncpy(header, "MT32WAVE", 8);
	int pos = 8;
	// Version...
	for (int i = 0; i < 4; i++)
		header[pos++] = 0;
	header[pos++] = (char)(intRate & 0xFF);
	header[pos++] = (char)((intRate >> 8) & 0xFF);
	header[pos++] = (char)((intRate >> 16) & 0xFF);
	header[pos] = (char)((intRate >> 24) & 0xFF);
#if WAVECACHEMODE < 2
	file = synth->openFile(filename, File::OpenMode_read);
	if (file != NULL) {
		char fileHeader[16];
		if (file->read(fileHeader, 16) == 16) {
			if (memcmp(fileHeader, header, 16) == 0) {
				reading = true;
			} else {
				synth->printDebug("Header of %s does not match expected - will generate", filename);
			}
		} else {
			synth->printDebug("Error reading 16 bytes of %s - will generate", filename);
		}
	} else {
		synth->printDebug("Unable to open %s for reading - will generate", filename);
	}
#endif
#if WAVECACHEMODE == 0 || WAVECACHEMODE == 2
	if (!reading) {
		file = synth->openFile(filename, File::OpenMode_write);
		if (file != NULL) {
			if (file->write(header, 16) == 16) {
				writing = true;
			} else {
				synth->printDebug("Error writing 16-byte header to %s - won't continue saving", filename);
			}
		} else {
			synth->printDebug("Unable to open %s for writing - won't be created", filename);
		}
	}
#endif

	double ampsize = WGAMP;
	for (int f = 12; f < 109; f++) {
		float freq = (float)(TUNING * pow(2.0, ((double)f - 69.0) / 12.0));
		freqtable[f] = (Bit16s)freq;
		divtable[f] = (int)(rate / freq);
		smalldivtable[f] = divtable[f] << 8;
		divtable[f] = divtable[f] << 16;

		initSaw(f, divtable[f]);
		initDep(f);

		//synth->printDebug("F %d sd %f div %d", f, sd, divtable[f]);
		initWave(synth, file, reading, writing, f, freq, rate, ampsize, divtable[f]);

		// Create the pitch tables

		float tuner = (32000.0f / rate) * 65536.0f;
		for (int pc = 0; pc < 54; pc++)
			wavtabler[pc][f] = (int)(tuner * (freq / pcms[pc].tune));
		for (int lp = 0; lp < 9; lp++) {
			for (int ln = 0; ln < 10; ln++) {
				// FIXME:KG: Surely this needs to be adjusted for the rate?
				looptabler[lp][ln][f] = (int)((float)LoopPatternTuning[lp][ln] * (freq / 220.0f));
			}
		}

		initFiltTable(f, freq, rate);
		initNFiltTable(f, freq, rate);
	}
	if (file != NULL)
		synth->closeFile(file);
}

void TableInitialiser::initMT32Tables(Synth *synth, sampleFormat pcms[54], float sampleRate) {
	if (sampleRate <= 0.0f) {
		synth->printDebug("Bad sampleRate (%d <= 0.0f)", sampleRate);
	}
	if (initialisedSampleRate == 0.0f) {
		initMT32ConstantTables(synth);
		initFiltCoeff(sampleRate);
		initEnvelopes(sampleRate);
		initialisedSampleRate = sampleRate;
	}
	// This always needs to be done, to allocate the waveforms
	initNotes(synth, pcms, sampleRate);
}

}
