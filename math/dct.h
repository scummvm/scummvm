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

// Based on eos' (I)RDFT code which is in turn
// Based upon the (I)DCT code in FFmpeg
// Copyright (c) 2009 Peter Ross <pross@xvid.org>
// Copyright (c) 2010 Alex Converse <alex.converse@gmail.com>
// Copyright (c) 2010 Vitor Sessak

#ifndef MATH_DCT_H
#define MATH_DCT_H

#include "math/cosinetables.h"

namespace Math {

class RDFT;

/**
 * @defgroup math_dct Discrete Cosine Transforms
 * @ingroup math
 *
 * @brief  Discrete Cosine Transforms.
 *
 * @{
 */

/**
 * (Inverse) Discrete Cosine Transforms.
 *
 * Used in engines:
 *  - Scumm
 */
class DCT {
public:
	enum TransformType {
		DCT_II,
		DCT_III,
		DCT_I,
		DST_I
	};

	DCT(int bits, TransformType trans);
	~DCT();

	void calc(float *data);

private:
	int _bits;
	TransformType _trans;

	CosineTable _cos;
	const float *_tCos;

	float *_csc2;

	RDFT *_rdft;

	void calcDCTI  (float *data);
	void calcDCTII (float *data);
	void calcDCTIII(float *data);
	void calcDSTI  (float *data);
};

/** @} */

} // End of namespace Math

#endif // MATH_DCT_H
