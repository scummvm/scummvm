/* Copyright (c) 2003-2005 Various contributors
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


// FIXME: Avoid using rand
#define FORBIDDEN_SYMBOL_EXCEPTION_rand

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mt32emu.h"

#if defined(MACOSX) || defined(SOLARIS) || defined(__MINGW32__)
// Older versions of Mac OS X didn't supply a powf function, so using it
// will cause a binary incompatibility when trying to run a binary built
// on a newer OS X release on an older one. And Solaris 8 doesn't provide
// powf, floorf, fabsf etc. at all.
// Cross-compiled MinGW32 toolchains suffer from a cross-compile bug in
// libstdc++. math/stubs.o should be empty, but it comes with a symbol for
// powf, resulting in a linker error because of multiple definitions.
// Hence we re-define them here. The only potential drawback is that it
// might be a little bit slower this way.
#define powf(x,y)	((float)pow(x,y))
#define floorf(x)	((float)floor(x))
#define fabsf(x)	((float)fabs(x))
#endif

#define FIXEDPOINT_MAKE(x, point) ((Bit32u)((1 << point) * x))

namespace MT32Emu {

//Amplitude time velocity follow exponential coefficients
static const double tvcatconst[5] = {0.0, 0.002791309, 0.005942882, 0.012652792, 0.026938637};
static const double tvcatmult[5] = {1.0, 1.072662811, 1.169129367, 1.288579123, 1.229630539};

// These are division constants for the TVF depth key follow
static const Bit32u depexp[5] = {3000, 950, 485, 255, 138};

//Envelope time keyfollow exponential coefficients
static const double tkcatconst[5] = {0.0, 0.005853144, 0.011148054, 0.019086143, 0.043333215};
static const double tkcatmult[5] = {1.0, 1.058245688, 1.048488989, 1.016049301, 1.097538067};

// Begin filter stuff

// Pre-warp the coefficients of a numerator or denominator.
// Note that a0 is assumed to be 1, so there is no wrapping
// of it.
static void prewarp(double *a1, double *a2, double fc, double fs) {
	double wp;

	wp = 2.0 * fs * tan(DOUBLE_PI * fc / fs);

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

void Tables::initFiltCoeff(float samplerate) {
	for (int j = 0; j < FILTERGRAN; j++) {
		for (int res = 0; res < 31; res++) {
			float tres = resonanceFactor[res];
			initFilter((float)samplerate, (((float)(j+1.0)/FILTERGRAN)) * ((float)samplerate/2), filtCoeff[j][res], tres);
		}
	}
}

void Tables::initEnvelopes(float samplerate) {
	for (int lf = 0; lf <= 100; lf++) {
		float elf = (float)lf;

		// General envelope
		// This formula fits observation of the CM-32L by +/- 0.03s or so for the second time value in the filter,
		// when all other times were 0 and all levels were 100. Note that variations occur depending on the level
		// delta of the section, which we're not fully emulating.
		float seconds = powf(2.0f, (elf / 8.0f) + 7.0f) / 32768.0f;
		int samples = (int)(seconds * samplerate);
		envTime[lf] = samples;

		// Cap on envelope times depending on the level delta
		if (elf == 0) {
			envDeltaMaxTime[lf] = 63;
		} else {
			float cap = 11.0f * (float)log(elf) + 64;
			if (cap > 100.0f) {
				cap = 100.0f;
			}
			envDeltaMaxTime[lf] = (int)cap;
		}


		// This (approximately) represents the time durations when the target level is 0.
		// Not sure why this is a special case, but it's seen to be from the real thing.
		seconds = powf(2, (elf / 8.0f) + 6) / 32768.0f;
		envDecayTime[lf]  = (int)(seconds * samplerate);

		// I am certain of this:  Verified by hand LFO log
		lfoPeriod[lf] = (Bit32u)(((float)samplerate) / (powf(1.088883372f, (float)lf) * 0.021236044f));
	}
}

void Tables::initMT32ConstantTables(Synth *synth) {
	int lf;
	synth->printDebug("Initializing Pitch Tables");
	for (lf = -108; lf <= 108; lf++) {
		tvfKeyfollowMult[lf + 108] = (int)(256 * powf(2.0f, (float)(lf / 24.0f)));
		//synth->printDebug("KT %d = %d", f, keytable[f+108]);
	}

	for (int res = 0; res < 31; res++) {
		resonanceFactor[res] = powf((float)res / 30.0f, 5.0f) + 1.0f;
	}

	int period = 65536;

	for (int ang = 0; ang < period; ang++) {
		int halfang = (period / 2);
		int angval = ang % halfang;
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
				float ff = (float)(exp(3.5f * tvcatconst[dep] * (59.0f - (float)velt)) * tvcatmult[dep]);
				tempdep = 256.0f * ff;
				envTimeVelfollowMult[dep][velt] = (int)tempdep;
				//if ((velt % 16) == 0) {
				//	synth->printDebug("Key %d, depth %d, factor %d", velt, dep, (int)tempdep);
				//}
			} else
				envTimeVelfollowMult[dep][velt] = 256;
		}

		for (dep = -7; dep < 8; dep++) {
			float fldep = (float)abs(dep) / 7.0f;
			fldep = powf(fldep,2.5f);
			if (dep < 0)
				fldep = fldep * -1.0f;
			pwVelfollowAdd[dep+7][velt] = Bit32s((fldep * (float)velt * 100) / 128.0);
		}
	}

	for (dep = 0; dep <= 100; dep++) {
		for (velt = 0; velt < 128; velt++) {
			float fdep = (float)dep * 0.000347013f; // Another MT-32 constant
			float fv = ((float)velt - 64.0f)/7.26f;
			float flogdep = powf(10, fdep * fv);
			float fbase;

			if (velt > 64)
				synth->tables.tvfVelfollowMult[velt][dep] = (int)(flogdep * 256.0);
			else {
				//lff = 1 - (pow(((128.0 - (float)lf) / 64.0),.25) * ((float)velt / 96));
				fbase = 1 - (powf(((float)dep / 100.0f),.25f) * ((float)(64-velt) / 96.0f));
				synth->tables.tvfVelfollowMult[velt][dep] = (int)(fbase * 256.0);
			}
			//synth->printDebug("Filvel dep %d velt %d = %x", dep, velt, filveltable[velt][dep]);
		}
	}

	for (lf = 0; lf < 128; lf++) {
		float veloFract = lf / 127.0f;
		for (int velsens = 0; velsens <= 100; velsens++) {
			float sensFract = (velsens - 50) / 50.0f;
			if (velsens < 50) {
				tvaVelfollowMult[lf][velsens] = FIXEDPOINT_MAKE(1.0f / powf(2.0f, veloFract * -sensFract * 127.0f / 20.0f), 8);
			} else {
				tvaVelfollowMult[lf][velsens] = FIXEDPOINT_MAKE(1.0f / powf(2.0f, (1.0f - veloFract) * sensFract * 127.0f / 20.0f), 8);
			}
		}
	}

	for (lf = 0; lf <= 100; lf++) {
		// Converts the 0-100 range used by the MT-32 to volume multiplier
		volumeMult[lf] = FIXEDPOINT_MAKE(powf((float)lf / 100.0f, FLOAT_LN), 7);
	}

	for (lf = 0; lf <= 100; lf++) {
		float mv = lf / 100.0f;
		float pt = mv - 0.5f;
		if (pt < 0)
			pt = 0;

		// Original (CC version)
		//pwFactor[lf] = (int)(pt * 210.04f) + 128;

		// Approximation from sample comparison
		pwFactor[lf] = (int)(pt * 179.0f) + 128;
	}

	for (unsigned int i = 0; i < MAX_SAMPLE_OUTPUT; i++) {
		int myRand;
		myRand = rand();
		//myRand = ((myRand - 16383) * 7168) >> 16;
		// This one is slower but works with all values of RAND_MAX
		myRand = (int)((myRand - RAND_MAX / 2) / (float)RAND_MAX * (7168 / 2));
		//FIXME:KG: Original ultimately set the lowest two bits to 0, for no obvious reason
		noiseBuf[i] = (Bit16s)myRand;
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
		//synth->printDebug("lf %d = padj %f", lf, (double)padjtable[lf]);
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
				lfp = (float)exp(0.713619942f * tlf) / 407.4945111f;

				if (depat < 50)
					finalval = 4096.0f * powf(2, -lfp);
				else
					finalval = 4096.0f * powf(2, lfp);
				pval = (int)finalval;

				pitchEnvVal[lf][depat] = pval;
				//synth->printDebug("lf %d depat %d pval %d tlf %f lfp %f", lf,depat,pval, (double)tlf, (double)lfp);
			} else {
				pitchEnvVal[lf][depat] = 4096;
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

			lfoShift[lf][depat] = pval;

			//synth->printDebug("lf %d depat %d pval %x", lf,depat,pval);
		}
	}

	for (lf = 0; lf <= 12; lf++) {
		for (int distval = 0; distval < 128; distval++) {
			float amplog, dval;
			if (lf == 0) {
				amplog = 0;
				dval = 1;
				tvaBiasMult[lf][distval] = 256;
			} else {
				/*
				amplog = powf(1.431817011f, (float)lf) / FLOAT_PI;
				dval = ((128.0f - (float)distval) / 128.0f);
				amplog = exp(amplog);
				dval = powf(amplog, dval) / amplog;
				tvaBiasMult[lf][distval] = (int)(dval * 256.0);
				*/
				// Lets assume for a second it's linear

				// Distance of full volume reduction
				amplog = (float)(12.0f / (float)lf) * 24.0f;
				if (distval > amplog) {
					tvaBiasMult[lf][distval] = 0;
				} else {
					dval = (amplog - (float)distval) / amplog;
					tvaBiasMult[lf][distval] = (int)(dval * 256.0f);
				}
			}
			//synth->printDebug("Ampbias lf %d distval %d = %f (%x) %f", lf, distval, (double)dval, tvaBiasMult[lf][distval],(double)amplog);
		}
	}

	for (lf = 0; lf <= 14; lf++) {
		for (int distval = 0; distval < 128; distval++) {
			float filval = fabsf((float)((lf - 7) * 12) / 7.0f);
			float amplog, dval;
			if (lf == 7) {
				amplog = 0;
				dval = 1;
				tvfBiasMult[lf][distval] = 256;
			} else {
				//amplog = pow(1.431817011, filval) / FLOAT_PI;
				amplog = powf(1.531817011f, filval) / FLOAT_PI;
				dval = (128.0f - (float)distval) / 128.0f;
				amplog = (float)exp(amplog);
				dval = powf(amplog,dval)/amplog;
				if (lf < 8) {
					tvfBiasMult[lf][distval] = (int)(dval * 256.0f);
				} else {
					dval = powf(dval, 0.3333333f);
					if (dval < 0.01f)
						dval = 0.01f;
					dval = 1 / dval;
					tvfBiasMult[lf][distval] = (int)(dval * 256.0f);
				}
			}
			//synth->printDebug("Fbias lf %d distval %d = %f (%x) %f", lf, distval, (double)dval, tvfBiasMult[lf][distval],(double)amplog);
		}
	}
}

// Per-note table initialisation follows

static void initSaw(NoteLookup *noteLookup, Bit32s div2) {
	int tmpdiv = div2 << 16;
	for (int rsaw = 0; rsaw <= 100; rsaw++) {
		float fsaw;
		if (rsaw < 50)
			fsaw = 50.0f;
		else
			fsaw = (float)rsaw;

		//(66 - (((A8 - 50) / 50) ^ 0.63) * 50) / 132
		float sawfact = (66.0f - (powf((fsaw - 50.0f) / 50.0f, 0.63f) * 50.0f)) / 132.0f;
		noteLookup->sawTable[rsaw] = (int)(sawfact * (float)tmpdiv) >> 16;
		//synth->printDebug("F %d divtable %d saw %d sawtable %d", f, div, rsaw, sawtable[f][rsaw]);
	}
}

static void initDep(KeyLookup *keyLookup, float f) {
	for (int dep = 0; dep < 5; dep++) {
		if (dep == 0) {
			keyLookup->envDepthMult[dep] = 256;
			keyLookup->envTimeMult[dep] = 256;
		} else {
			float depfac = 3000.0f;
			float ff, tempdep;
			depfac = (float)depexp[dep];

			ff = (f - (float)MIDDLEC) / depfac;
			tempdep = powf(2, ff) * 256.0f;
			keyLookup->envDepthMult[dep] = (int)tempdep;

			ff = (float)(exp(tkcatconst[dep] * ((float)MIDDLEC - f)) * tkcatmult[dep]);
			keyLookup->envTimeMult[dep] = (int)(ff * 256.0f);
		}
	}
	//synth->printDebug("F %f d1 %x d2 %x d3 %x d4 %x d5 %x", (double)f, noteLookup->fildepTable[0], noteLookup->fildepTable[1], noteLookup->fildepTable[2], noteLookup->fildepTable[3], noteLookup->fildepTable[4]);
}

Bit16s Tables::clampWF(Synth *synth, const char *n, float ampVal, double input) {
	Bit32s x = (Bit32s)(input * ampVal);
	if (x < -ampVal - 1) {
		synth->printDebug("%s==%d<-WGAMP-1!", n, x);
		x = (Bit32s)(-ampVal - 1);
	} else if (x > ampVal) {
		synth->printDebug("%s==%d>WGAMP!", n, x);
		x = (Bit32s)ampVal;
	}
	return (Bit16s)x;
}

File *Tables::initWave(Synth *synth, NoteLookup *noteLookup, float ampVal, float div2, File *file) {
	int iDiv2 = (int)div2;
	noteLookup->waveformSize[0] = iDiv2 << 1;
	noteLookup->waveformSize[1] = iDiv2 << 1;
	noteLookup->waveformSize[2] = iDiv2 << 2;
	for (int i = 0; i < 3; i++) {
		if (noteLookup->waveforms[i] == NULL) {
			noteLookup->waveforms[i] = new Bit16s[noteLookup->waveformSize[i]];
		}
	}
	if (file != NULL) {
		for (int i = 0; i < 3 && file != NULL; i++) {
			size_t len = noteLookup->waveformSize[i];
			for (unsigned int j = 0; j < len; j++) {
				if (!file->readBit16u((Bit16u *)&noteLookup->waveforms[i][j])) {
					synth->printDebug("Error reading wave file cache!");
					file->close();
					file = NULL;
					break;
				}
			}
		}
	}
	if (file == NULL) {
		double sd = DOUBLE_PI / div2;

		for (int fa = 0; fa < (iDiv2 << 1); fa++) {
			// sa ranges from 0 to 2PI
			double sa = fa * sd;

			// Calculate a sample for the bandlimited sawtooth wave
			double saw = 0.0;
			int sincs = iDiv2 >> 1;
			double sinus = 1.0;
			for (int sincNum = 1; sincNum <= sincs; sincNum++) {
				saw += sin(sinus * sa) / sinus;
				sinus++;
			}

			// This works pretty well
			// Multiplied by 0.84 so that the spikes caused by bandlimiting don't overdrive the amplitude
			noteLookup->waveforms[0][fa] = clampWF(synth, "saw", ampVal, -saw / (0.5 * DOUBLE_PI) * 0.84);
			noteLookup->waveforms[1][fa] = clampWF(synth, "cos", ampVal, -cos(sa / 2.0));
			noteLookup->waveforms[2][fa * 2] = clampWF(synth, "cosoff_0", ampVal, -cos(sa - DOUBLE_PI));
			noteLookup->waveforms[2][fa * 2 + 1] = clampWF(synth, "cosoff_1", ampVal, -cos((sa + (sd / 2)) - DOUBLE_PI));
		}
	}
	return file;
}

static void initFiltTable(NoteLookup *noteLookup, float freq, float rate) {
	for (int tr = 0; tr <= 200; tr++) {
		float ftr = (float)tr;

		// Verified exact on MT-32
		if (tr > 100)
			ftr = 100.0f + (powf((ftr - 100.0f) / 100.0f, 3.0f) * 100.0f);

		// I think this is the one
		float brsq = powf(10.0f, (tr / 50.0f) - 1.0f);
		noteLookup->filtTable[0][tr] = (int)((freq * brsq) / (rate / 2) * FILTERGRAN);
		if (noteLookup->filtTable[0][tr]>=((FILTERGRAN*15)/16))
			noteLookup->filtTable[0][tr] = ((FILTERGRAN*15)/16);

		float brsa = powf(10.0f, ((tr / 55.0f) - 1.0f)) / 2.0f;
		noteLookup->filtTable[1][tr] = (int)((freq * brsa) / (rate / 2) * FILTERGRAN);
		if (noteLookup->filtTable[1][tr]>=((FILTERGRAN*15)/16))
			noteLookup->filtTable[1][tr] = ((FILTERGRAN*15)/16);
	}
}

static void initNFiltTable(NoteLookup *noteLookup, float freq, float rate) {
	for (int cf = 0; cf <= 100; cf++) {
		float cfmult = (float)cf;

		for (int tf = 0;tf <= 100; tf++) {
			float tfadd = (float)tf;

			//float freqsum = exp((cfmult + tfadd) / 30.0f) / 4.0f;
			//float freqsum = 0.15f * exp(0.45f * ((cfmult + tfadd) / 10.0f));

			float freqsum = powf(2.0f, ((cfmult + tfadd) - 40.0f) / 16.0f);

			noteLookup->nfiltTable[cf][tf] = (int)((freq * freqsum) / (rate / 2) * FILTERGRAN);
			if (noteLookup->nfiltTable[cf][tf] >= ((FILTERGRAN * 15) / 16))
				noteLookup->nfiltTable[cf][tf] = ((FILTERGRAN * 15) / 16);
		}
	}
}

File *Tables::initNote(Synth *synth, NoteLookup *noteLookup, float note, float rate, float masterTune, PCMWaveEntry *pcmWaves, File *file) {
	float freq = (float)(masterTune * pow(2.0, ((double)note - MIDDLEA) / 12.0));
	float div2 = rate * 2.0f / freq;
	noteLookup->div2 = (int)div2;

	if (noteLookup->div2 == 0)
		noteLookup->div2 = 1;

	initSaw(noteLookup, noteLookup->div2);

	//synth->printDebug("Note %f; freq=%f, div=%f", (double)note, (double)freq, (double) rate / freq);
	file = initWave(synth, noteLookup, WGAMP, div2, file);

	// Create the pitch tables
	if (noteLookup->wavTable == NULL)
		noteLookup->wavTable = new Bit32u[synth->controlROMMap->pcmCount];
	double rateMult = 32000.0 / rate;
	double tuner = freq * 65536.0f;
	for (int pc = 0; pc < synth->controlROMMap->pcmCount; pc++) {
		noteLookup->wavTable[pc] = (int)(tuner / pcmWaves[pc].tune * rateMult);
	}

	initFiltTable(noteLookup, freq, rate);
	initNFiltTable(noteLookup, freq, rate);
	return file;
}

bool Tables::initNotes(Synth *synth, PCMWaveEntry *pcmWaves, float rate, float masterTune) {
	const char *NoteNames[12] = {
		"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "
	};
	char filename[64];
	int intRate = (int)rate;
	char version[4] = {0, 0, 0, 5};
	sprintf(filename, "waveformcache-%d-%.2f.raw", intRate, (double)masterTune);

	File *file = NULL;
	char header[20];
	memcpy(header, "MT32WAVE", 8);
	int pos = 8;
	// Version...
	for (int i = 0; i < 4; i++)
		header[pos++] = version[i];
	header[pos++] = (char)((intRate >> 24) & 0xFF);
	header[pos++] = (char)((intRate >> 16) & 0xFF);
	header[pos++] = (char)((intRate >> 8) & 0xFF);
	header[pos++] = (char)(intRate & 0xFF);
	int intTuning = (int)masterTune;
	header[pos++] = (char)((intTuning >> 8) & 0xFF);
	header[pos++] = (char)(intTuning & 0xFF);
	header[pos++] = 0;
	header[pos] = (char)((masterTune - intTuning) * 10);
#if MT32EMU_WAVECACHEMODE < 2
	bool reading = false;
	file = synth->openFile(filename, File::OpenMode_read);
	if (file != NULL) {
		char fileHeader[20];
		if (file->read(fileHeader, 20) == 20) {
			if (memcmp(fileHeader, header, 20) == 0) {
				Bit16u endianCheck;
				if (file->readBit16u(&endianCheck)) {
					if (endianCheck == 1) {
						reading = true;
					} else {
						synth->printDebug("Endian check in %s does not match expected", filename);
					}
				} else {
					synth->printDebug("Unable to read endian check in %s", filename);
				}
			} else {
				synth->printDebug("Header of %s does not match expected", filename);
			}
		} else {
			synth->printDebug("Error reading 16 bytes of %s", filename);
		}
		if (!reading) {
			file->close();
			file = NULL;
		}
	} else {
		synth->printDebug("Unable to open %s for reading", filename);
	}
#endif

	float progress = 0.0f;
	bool abort = false;
	synth->report(ReportType_progressInit, &progress);
	for (int f = LOWEST_NOTE; f <= HIGHEST_NOTE; f++) {
		synth->printDebug("Initializing note %s%d", NoteNames[f % 12], (f / 12) - 2);
		NoteLookup *noteLookup = &noteLookups[f - LOWEST_NOTE];
		file = initNote(synth, noteLookup, (float)f, rate, masterTune, pcmWaves, file);
		progress = (f - LOWEST_NOTE + 1) / (float)NUM_NOTES;
		abort = synth->report(ReportType_progressInit, &progress) != 0;
		if (abort)
			break;
	}

#if MT32EMU_WAVECACHEMODE == 0 || MT32EMU_WAVECACHEMODE == 2
	if (file == NULL) {
		file = synth->openFile(filename, File::OpenMode_write);
		if (file != NULL) {
			if (file->write(header, 20) == 20 && file->writeBit16u(1)) {
				for (int f = 0; f < NUM_NOTES; f++) {
					for (int i = 0; i < 3 && file != NULL; i++) {
						int len = noteLookups[f].waveformSize[i];
						for (int j = 0; j < len; j++) {
							if (!file->writeBit16u(noteLookups[f].waveforms[i][j])) {
								synth->printDebug("Error writing waveform cache file");
								file->close();
								file = NULL;
								break;
							}
						}
					}
				}
			} else {
				synth->printDebug("Error writing 16-byte header to %s - won't continue saving", filename);
			}
		} else {
			synth->printDebug("Unable to open %s for writing - won't be created", filename);
		}
	}
#endif

	if (file != NULL)
		synth->closeFile(file);
	return !abort;
}

void Tables::freeNotes() {
	for (int t = 0; t < 3; t++) {
		for (int m = 0; m < NUM_NOTES; m++) {
			if (noteLookups[m].waveforms[t] != NULL) {
				delete[] noteLookups[m].waveforms[t];
				noteLookups[m].waveforms[t] = NULL;
				noteLookups[m].waveformSize[t] = 0;
			}
			if (noteLookups[m].wavTable != NULL) {
				delete[] noteLookups[m].wavTable;
				noteLookups[m].wavTable = NULL;
			}
		}
	}
	initializedMasterTune = 0.0f;
}

Tables::Tables() {
	initializedSampleRate = 0.0f;
	initializedMasterTune = 0.0f;
	memset(&noteLookups, 0, sizeof(noteLookups));
}

bool Tables::init(Synth *synth, PCMWaveEntry *pcmWaves, float sampleRate, float masterTune) {
	if (sampleRate <= 0.0f) {
		synth->printDebug("Bad sampleRate (%f <= 0.0f)", (double)sampleRate);
		return false;
	}
	if (initializedSampleRate == 0.0f) {
		initMT32ConstantTables(synth);
	}
	if (initializedSampleRate != sampleRate) {
		initFiltCoeff(sampleRate);
		initEnvelopes(sampleRate);
		for (int key = 12; key <= 108; key++) {
			initDep(&keyLookups[key - 12], (float)key);
		}
	}
	if (initializedSampleRate != sampleRate || initializedMasterTune != masterTune) {
		freeNotes();
		if (!initNotes(synth, pcmWaves, sampleRate, masterTune)) {
			return false;
		}
		initializedSampleRate = sampleRate;
		initializedMasterTune = masterTune;
	}
	return true;
}

}
