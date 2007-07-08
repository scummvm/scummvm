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
 
/* Filter coefficient generation using Kaiser filter design */
 
#include "common/stdafx.h"

#include "sound/filter.h"

namespace Audio {

static double bessel_i0(double x);

/* 
 * Modified Bessel function of the first type and zeroth order.  This is
 * calculated using an iterative expansion of I_0(x) = sum from n=0:infinity
 * of ( (1/4) * x^2 )^n / (n!)^2, which can be written as the sum from 
 * n=0:infinity of ( ((x/2)^(n - 1)) / (n - 1)! )^2 * (x / (2 * n))^2
 *                |---------previous term----------| |---new factor--|
 *                |------------------current term--------------------|
 */
static double bessel_i0(double x) { 
	uint16 n;
	
	/* Base case for n == 0 */
	double term = 1;
	double accumulator = 1;
	
	/* I_0(-x) = I_0(x) */
	x = fabs(x);
	
	/* 
	 * Will return positive infinity for x greater than ~714, so save some
	 * calculations in this case.
	 */
	if (x > 714) {
		return HUGE_VAL;	// positive infinity, double precision
	}
	
	/* x + 10 gives a reasonable bound */
	for (n = 1; n < (int)(x + 10); n++) {
		term *= (x / (2 * n));
		term *= (x / (2 * n));
		accumulator += term;
	}
	
	return accumulator;
}

/* Returns the tolerable ripple in an equiripple filter */
inline double FIRFilter::equiripple(
			double dBPassbandRipple,
			double dBStopbandAtten) {
	return fmin(dBPassbandRipple, -dBStopbandAtten);
}

/*
 * Estimated window length using the Kaiser design formula.
 * Assumption: window length <= 65535 taps.
 */
inline uint16 FIRFilter::windowLength(
					double ripple,
					double transitionBW,
					uint16 samplingFreq) {
	double length = (samplingFreq * (-ripple - 7.95)) / (14.36 * transitionBW);
	
	assert(length > 0);
	
	return (uint16)(length + 1);
}

/*
 * Derives the coefficients for a Kaiser window of the given properties, using
 * standard Kaiser design formulae.
 */
void FIRFilter::windowDesign(double *coeffs, uint16 length, double ripple) {
	uint16 i;
	
	double alpha = -ripple;
	double beta;
	
	if (alpha >= 50) {
		beta = 0.1102 * (alpha - 8.7);
	} else if (alpha > 21) {
		beta = 0.5842 * pow((double)alpha - 21, 0.4) + 0.07886 * (alpha - 21);
	} else {
		beta = 0;
	}
	
	for (i = 0; i <= (length - 1) / 2; i++) {
		int16 n = i - (length - 1) / 2;
		
		/*
		 * Both this window and the later filter are even symmetric, so we
		 * only need to calculate half of the window for now.
		 */
		coeffs[i] = 
			bessel_i0(beta * sqrt(1 - pow((2.0 * n) / (length - 1), 2.0)))
			/ bessel_i0(beta);
	}
}

inline double FIRFilter::sinc(double arg) {
	if (arg == 0) {
		/* continuous extension of sin(arg) / arg */
		return 1;
	} else {
		return sin(arg) / arg;
	}
}

/*
 * Generates the filter coefficients for a windowed lowpass filter by applying
 * the provided window to the ideal lowpass (sinc) filter
 */
void FIRFilter::LPDesign(
		double *coeffs,
		uint16 length,
		double passbandEdge,
		double stopbandEdge,
		uint16 samplingFreq) {
	uint16 i;
	
	for (i = 0; i <= (length - 1) / 2; i++) {
		int16 n = i - (length - 1) / 2;
		
		/*
		 * Use an ideal transition halfway between the passband and stopband
		 * edges
		 */
		double bandwidth = (passbandEdge + stopbandEdge) / samplingFreq;
		// == 2 * ((passbandEdge + stopbandEdge) / 2) / samplingFreq
		
		coeffs[i] *= bandwidth *  sinc(M_PI * bandwidth * n);
		/* Filter is even symmetric */
		coeffs[length - i - 1] = coeffs[i];
	}
}

/* Generates lowpass filter coefficients using the parameters provided */
FIRFilter::FIRFilter(double passbandEdge, double stopbandEdge,
					double dBPassbandRipple, double dBStopbandAtten,
					uint16 samplingFreq)
		: _passbandEdge(passbandEdge), _stopbandEdge(stopbandEdge),
		  _dBPassbandRipple(dBPassbandRipple),
		  _dBStopbandAtten(dBStopbandAtten),
		  _samplingFreq(samplingFreq), _ripple(0), _length(0), _coeffs(0) {
	/* Find the amount of ripple that the filter will produce */
	_ripple = equiripple(_dBPassbandRipple, _dBStopbandAtten);
	
	/* Find the number of coefficients in the window */
	_length = 
		windowLength(_ripple, _stopbandEdge - _passbandEdge, _samplingFreq);
	
	/* Calculate the window coefficients */
	_coeffs = (double *)malloc(_length * sizeof(double));
	windowDesign(_coeffs, _length, _ripple);
	
	/* Generate the coefficients of a low pass filter using this window */
	LPDesign(_coeffs, _length, _passbandEdge, _stopbandEdge, _samplingFreq);
}

} // End of namespace Audio
