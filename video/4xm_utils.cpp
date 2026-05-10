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

#include "video/4xm_utils.h"
#include "common/bitstream.h"
#include "common/debug.h"
#include "common/memstream.h"

namespace Video {
namespace FourXM {

#define FIX_1_082392200 70936
#define FIX_1_414213562 92682
#define FIX_1_847759065 121095
#define FIX_2_613125930 171254

#define MULTIPLY(var, const) ((int)((var) * (unsigned)(const)) >> 16)

void idct(int16_t block[64], int shift) {
	int tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	int tmp10, tmp11, tmp12, tmp13;
	int z5, z10, z11, z12, z13;
	int i;
	int temp[64];

	for (i = 0; i < 8; i++) {
		tmp10 = block[8 * 0 + i] + block[8 * 4 + i];
		tmp11 = block[8 * 0 + i] - block[8 * 4 + i];

		tmp13 = block[8 * 2 + i] + block[8 * 6 + i];
		tmp12 = MULTIPLY(block[8 * 2 + i] - block[8 * 6 + i], FIX_1_414213562) - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = block[8 * 5 + i] + block[8 * 3 + i];
		z10 = block[8 * 5 + i] - block[8 * 3 + i];
		z11 = block[8 * 1 + i] + block[8 * 7 + i];
		z12 = block[8 * 1 + i] - block[8 * 7 + i];

		tmp7 = z11 + z13;
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065);
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5;
		tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		temp[8 * 0 + i] = tmp0 + tmp7;
		temp[8 * 7 + i] = tmp0 - tmp7;
		temp[8 * 1 + i] = tmp1 + tmp6;
		temp[8 * 6 + i] = tmp1 - tmp6;
		temp[8 * 2 + i] = tmp2 + tmp5;
		temp[8 * 5 + i] = tmp2 - tmp5;
		temp[8 * 4 + i] = tmp3 + tmp4;
		temp[8 * 3 + i] = tmp3 - tmp4;
	}

	for (i = 0; i < 8 * 8; i += 8) {
		tmp10 = temp[0 + i] + temp[4 + i];
		tmp11 = temp[0 + i] - temp[4 + i];

		tmp13 = temp[2 + i] + temp[6 + i];
		tmp12 = MULTIPLY(temp[2 + i] - temp[6 + i], FIX_1_414213562) - tmp13;

		tmp0 = tmp10 + tmp13;
		tmp3 = tmp10 - tmp13;
		tmp1 = tmp11 + tmp12;
		tmp2 = tmp11 - tmp12;

		z13 = temp[5 + i] + temp[3 + i];
		z10 = temp[5 + i] - temp[3 + i];
		z11 = temp[1 + i] + temp[7 + i];
		z12 = temp[1 + i] - temp[7 + i];

		tmp7 = z11 + z13;
		tmp11 = MULTIPLY(z11 - z13, FIX_1_414213562);

		z5 = MULTIPLY(z10 + z12, FIX_1_847759065);
		tmp10 = MULTIPLY(z12, FIX_1_082392200) - z5;
		tmp12 = MULTIPLY(z10, -FIX_2_613125930) + z5;

		tmp6 = tmp12 - tmp7;
		tmp5 = tmp11 - tmp6;
		tmp4 = tmp10 + tmp5;

		block[0 + i] = (tmp0 + tmp7) >> shift;
		block[7 + i] = (tmp0 - tmp7) >> shift;
		block[1 + i] = (tmp1 + tmp6) >> shift;
		block[6 + i] = (tmp1 - tmp6) >> shift;
		block[2 + i] = (tmp2 + tmp5) >> shift;
		block[5 + i] = (tmp2 - tmp5) >> shift;
		block[4 + i] = (tmp3 + tmp4) >> shift;
		block[3 + i] = (tmp3 - tmp4) >> shift;
	}
}

} // namespace FourXM
} // namespace Video
