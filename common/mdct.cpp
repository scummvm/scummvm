/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on xoreos' (I)RDFT code which is in turn
// Based upon the (I)MDCT code in FFmpeg
// Copyright (c) 2002 Fabrice Bellard

/** @file common/mdct.cpp
 *  (Inverse) Modified Discrete Cosine Transforms.
 */

#include "common/math.h"
#include "common/util.h"
#include "common/fft.h"
#include "common/mdct.h"

namespace Common {

MDCT::MDCT(int bits, bool inverse, double scale) : _bits(bits), _fft(0) {
	_size = 1 << bits;

	_fft = new FFT(_bits - 2, inverse);

	const int size2 = _size >> 1;
	const int size4 = _size >> 2;

	_tCos = new float[size2];
	_tSin = _tCos + size4;

	const double theta = 1.0 / 8.0 + (scale < 0 ? size4 : 0);

	scale = sqrt(ABS(scale));
	for (int i = 0; i < size4; i++) {
		const double alpha = 2 * (float)M_PI * (i + theta) / _size;

		_tCos[i] = -cos(alpha) * scale;
		_tSin[i] = -sin(alpha) * scale;
	}
}

MDCT::~MDCT() {
	delete[] _tCos;
	delete _fft;
}

#define CMUL(dre, dim, are, aim, bre, bim) do { \
		(dre) = (are) * (bre) - (aim) * (bim);  \
		(dim) = (are) * (bim) + (aim) * (bre);  \
	} while (0)

void MDCT::calcMDCT(float *output, const float *input) {
	Complex *x = (Complex *) output;

	const int size2 = _size >> 1;
	const int size4 = _size >> 2;
	const int size8 = _size >> 3;
	const int size3 = _size * 3;

	const uint16 *revTab = _fft->getRevTab();

	// Pre rotation
	for (int i = 0; i < size8; i++) {
		float re = -input[2 * i + size3] - input[size3 - 1 - 2 * i];
		float im = -input[2 * i + size4] + input[size4 - 1 - 2 * i];
		int    j = revTab[i];

		CMUL(x[j].re, x[j].im, re, im, -_tCos[i], _tSin[i]);

		re =  input[2 * i        ] - input[size2 - 1 - 2 * i];
		im = -input[2 * i + size2] - input[_size - 1 - 2 * i];
		 j = revTab[size8 + i];

		CMUL(x[j].re, x[j].im, re, im, -_tCos[size8 + i], _tSin[size8 + i]);
	}

	_fft->calc(x);

	// Post rotation
	for (int i = 0; i < size8; i++) {
		float r0, i0, r1, i1;

		CMUL(i1, r0, x[size8-i-1].re, x[size8-i-1].im, -_tSin[size8-i-1], -_tCos[size8-i-1]);
		CMUL(i0, r1, x[size8+i  ].re, x[size8+i  ].im, -_tSin[size8+i  ], -_tCos[size8+i  ]);

		x[size8 - i - 1].re = r0;
		x[size8 - i - 1].im = i0;
		x[size8 + i    ].re = r1;
		x[size8 + i    ].im = i1;
	}
}

void MDCT::calcIMDCT(float *output, const float *input) {
	const int size2 = _size >> 1;
	const int size4 = _size >> 2;

	calcHalfIMDCT(output + size4, input);

	for (int k = 0; k < size4; k++) {
		output[        k    ] = -output[size2 - k - 1];
		output[_size - k - 1] =  output[size2 + k    ];
	}
}

void MDCT::calcHalfIMDCT(float *output, const float *input) {
	Complex *z = (Complex *) output;

	const int size2 = _size >> 1;
	const int size4 = _size >> 2;
	const int size8 = _size >> 3;

	const uint16 *revTab = _fft->getRevTab();

	// Pre rotation
	const float *in1 = input;
	const float *in2 = input + size2 - 1;
	for (int k = 0; k < size4; k++) {
		const int j = revTab[k];

		CMUL(z[j].re, z[j].im, *in2, *in1, _tCos[k], _tSin[k]);

		in1 += 2;
		in2 -= 2;
	}

	_fft->calc(z);

	// Post rotation + reordering
	for (int k = 0; k < size8; k++) {
		float r0, i0, r1, i1;

		CMUL(r0, i1, z[size8-k-1].im, z[size8-k-1].re, _tSin[size8-k-1], _tCos[size8-k-1]);
		CMUL(r1, i0, z[size8+k  ].im, z[size8+k  ].re, _tSin[size8+k  ], _tCos[size8+k  ]);

		z[size8 - k - 1].re = r0;
		z[size8 - k - 1].im = i0;
		z[size8 + k    ].re = r1;
		z[size8 + k    ].im = i1;
	}
}

} // End of namespace Common
