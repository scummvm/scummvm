/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Based on eos' (I)FFT code which is in turn
// based upon the (I)FFT code in FFmpeg
// Copyright (c) 2008 Loren Merritt
// Copyright (c) 2002 Fabrice Bellard
// Partly based on libdjbfft by D. J. Bernstein

#ifndef MATH_FFT_H
#define MATH_FFT_H

#include "common/scummsys.h"

namespace Math {

/**
 * @defgroup math_fft Fast Fourier Transform (FFT)
 * @ingroup math
 *
 * @brief  API for the FFT algorithm.
 *
 * @{
 */

class CosineTable;
struct Complex;

/**
 * (Inverse) Fast Fourier Transform.
 *
 * Used in engines:
 *  - SCUMM
 */
class FFT {
public:
	FFT(int bits, int inverse);
	~FFT();

	const uint16 *getRevTab() const;

	/** Perform the permutation needed BEFORE calling calc(). */
	void permute(Complex *z);

	/** Perform a complex FFT.
	 *
	 *  The input data must be permuted before.
	 *  No 1.0/sqrt(n) normalization is done.
	 */
	void calc(Complex *z);

private:
	int _bits;
	int _inverse;

	uint16 *_revTab;

	Complex *_expTab;
	Complex *_tmpBuf;

	int _splitRadix;

	static int splitRadixPermutation(int i, int n, int inverse);

	CosineTable *_cosTables[13];

	void fft4(Complex *z);
	void fft8(Complex *z);
	void fft16(Complex *z);
	void fft(int n, int logn, Complex *z);
};

/** @} */

} // End of namespace Math

#endif // MATH_FFT_H
