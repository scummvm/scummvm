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

#ifndef PHOENIXVR_DCT_H
#define PHOENIXVR_DCT_H

#include "common/scummsys.h"
#include <cmath>

namespace PhoenixVR {

template<uint Bits>
class DCT2DIII {
	static constexpr uint N = 1 << Bits;
	static constexpr uint N2 = 1 << (Bits - 1);
	static constexpr uint N4 = 1 << (Bits - 2);

	float _cos[N2];

	float dctCos(uint idx) const {
		return _cos[idx % N2];
	}
	float dctCos(uint i, uint j) const {
		return dctCos(((i << 1) + 1) * j);
	}

public:
	DCT2DIII() {
		for (uint i = 0; i != N2; ++i) {
			_cos[i] = std::cos(i * M_PI / N4);
		}
	}

	void calc(const float *src, float *dst) const {
		const float sqrt12 = M_SQRT1_2;
		auto *dstPtr = dst;
		for (byte y = 0; y < 8; ++y) {
			for (byte x = 0; x < 8; ++x) {
				float row = 0;
				float cosX0 = dctCos(x, 0);
				for (byte v = 0; v < 8; ++v) {
					auto *src_col = src + (v << 3);
					float col = *src_col++ * cosX0 * sqrt12;
					for (byte u = 1; u < 8; ++u) {
						col += *src_col++ * dctCos(x, u);
					}
					row += col * dctCos(y, v) * (v != 0 ? 1 : sqrt12);
				}
				*dstPtr++ = row / 4;
			}
		}
		for (uint i = 0; i != 64; ++i)
			dst[i] /= 4;
	}
};

} // namespace PhoenixVR

#endif
