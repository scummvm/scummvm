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

#include "twp/btea.h"

namespace Twp {

#define DELTA 0x9e3779b9
#define MX (((z >> 5 ^ y << 2) + (y >> 3 ^ z << 4)) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z)))

void BTEACrypto::encrypt(uint32 *v, int n, const uint32 *key) {
	btea(v, n, key);
}

void BTEACrypto::decrypt(uint32 *v, int n, const uint32 *key) {
	btea(v, -n, key);
}

// This method comes from https://en.wikipedia.org/wiki/XXTEA
void BTEACrypto::btea(uint32 *v, int n, const uint32 *key) {
	uint32 y, z, sum;
	unsigned p, rounds, e;
	if (n > 1) { /* Coding Part */
		rounds = 6 + 52 / n;
		sum = 0;
		z = v[n - 1];
		do {
			sum += DELTA;
			e = (sum >> 2) & 3;
			for (p = 0; p < static_cast<uint32>(n - 1); p++) {
				y = v[p + 1];
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n - 1] += MX;
		} while (--rounds);
	} else if (n < -1) { /* Decoding Part */
		n = -n;
		rounds = 6 + 52 / n;
		sum = rounds * DELTA;
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p = n - 1; p > 0; p--) {
				z = v[p - 1];
				y = v[p] -= MX;
			}
			z = v[n - 1];
			y = v[0] -= MX;
			sum -= DELTA;
		} while (--rounds);
	}
}
} // namespace Twp
