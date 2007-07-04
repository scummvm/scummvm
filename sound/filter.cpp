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
 * $URL:  $
 * $Id:  $
 *
 */
 
/* Filter coefficient generation using Kaiser filter design */
// TODO: Namespace, class, etc.
 
#include "common/stdafx.h"

#include "sound/filter.h"

static double bessel_i0(double x);

static double equiripple(
			double dBPassbandRipple,
			double dBStopbandAtten);

static uint16 windowLength(
			double ripple,
			double transitionBW,
			double samplingFreq);

static void windowDesign(double *coeffs, uint16 length, double ripple);
 
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
static inline double equiripple(
			double dBPassbandRipple,
			double dBStopbandAtten) {
	return fmin(dBPassbandRipple, -dBStopbandAtten);
}

/*
 * Estimated window length using the Kaiser design formula.
 * Assumption: window length <= 65535 taps.
 */
static inline uint16 windowLength(
					double ripple,
					double transitionBW,
					double samplingFreq) {
	double length = (samplingFreq * (-ripple - 7.95)) / (14.36 * transitionBW);
	
	return (uint16)(length + 1);
}

/*
 * Derives the coefficients for a Kaiser window of the given properties, using
 * standard Kaiser design formulae.
 */
// TODO: check that the I_0 outputs haven't shot off to infinity
static void windowDesign(double *coeffs, uint16 length, double ripple) {
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
		
		coeffs[i] = 
			bessel_i0(beta * sqrt(1 - pow((2.0 * n) / (length - 1), 2.0)))
			/ bessel_i0(beta);
		coeffs[length - i - 1] = coeffs[i];
	}
}

/*
 * Generates a Kaiser window using the parameters expected to be available to
 * a rate converter.
 */
// TODO: Write a nice interface for this :)
void kaiserWindow(
		double passbandEdge,
		double stopbandEdge,
		double dBPassbandRipple,
		double dBStopbandAtten,
		double samplingFreq) {
	/* Find the amount of ripple that the filter will produce */
	double ripple = equiripple(dBPassbandRipple, dBStopbandAtten);
	
	/* Find the number of coefficients in the window */
	uint16 length = windowLength(ripple, stopbandEdge - passbandEdge, samplingFreq);
	
	/* Calculate the window coefficients */
	double *coeffs = (double *)malloc(length * sizeof(double));
	windowDesign(coeffs, length, ripple);
}
