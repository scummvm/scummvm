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

/**
 * Copyright (C) 2011 by Ben Noordhuis <info@bnoordhuis.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "common/punycode.h"
#include "common/debug.h"
#include "common/util.h"

namespace Common {

/* punycode parameters, see http://tools.ietf.org/html/rfc3492#section-5 */
#define BASE 36
#define TMIN 1
#define TMAX 26
#define SKEW 38
#define DAMP 700
#define INITIAL_N 128
#define INITIAL_BIAS 72

static uint32_t adapt_bias(uint32_t delta, unsigned n_points, int is_first) {
	uint32_t k;

	delta /= is_first ? DAMP : 2;
	delta += delta / n_points;

	/* while delta > 455: delta /= 35 */
	for (k = 0; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE) {
		delta /= (BASE - TMIN);
	}

	return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
}

static char encode_digit(int c) {
	assert(c >= 0 && c <= BASE - TMIN);
	if (c > 25) {
		return c + 26; /* '0'..'9' */
	} else {
		return c + 'a'; /* 'a'..'z' */
	}
}

/* Encode as a generalized variable-length integer. Returns number of bytes written. */
static size_t encode_var_int(const size_t bias, const size_t delta, char *const dst, size_t dstlen) {
	size_t i, k, q, t;

	i = 0;
	k = BASE;
	q = delta;

	while (i < dstlen) {
		if (k <= bias) {
			t = TMIN;
		} else if (k >= bias + TMAX) {
			t = TMAX;
		} else {
			t = k - bias;
		}

		if (q < t) {
			break;
		}

		dst[i++] = encode_digit(t + (q - t) % (BASE - t));

		q = (q - t) / (BASE - t);
		k += BASE;
	}

	if (i < dstlen) {
		dst[i++] = encode_digit(q);
	}

	return i;
}

static size_t decode_digit(uint32_t v) {
	if (Common::isDigit(v)) {
		return 26 + (v - '0');
	}
	if (Common::isLower(v)) {
		return v - 'a';
	}
	if (Common::isUpper(v)) {
		return v - 'A';
	}
	return SIZE_MAX;
}

size_t punycode_encode(const uint32_t *const src, const size_t srclen, char *const dst, size_t *const dstlen) {
	size_t b, h;
	size_t delta, bias;
	size_t m, n;
	size_t si, di;

	for (si = 0, di = 0; si < srclen && di < *dstlen; si++) {
		if (src[si] < 128) {
			dst[di++] = src[si];
		}
	}

	b = h = di;

	/* Write out delimiter if any basic code points were processed. */
	if (di > 0 && di < *dstlen) {
		dst[di++] = '-';
	}

	n = INITIAL_N;
	bias = INITIAL_BIAS;
	delta = 0;

	for (; h < srclen && di < *dstlen; n++, delta++) {
		/* Find next smallest non-basic code point. */
		for (m = SIZE_MAX, si = 0; si < srclen; si++) {
			if (src[si] >= n && src[si] < m) {
				m = src[si];
			}
		}

		if ((m - n) > (SIZE_MAX - delta) / (h + 1)) {
			/* OVERFLOW */
			assert(0 && "OVERFLOW");
			goto fail;
		}

		delta += (m - n) * (h + 1);
		n = m;

		for (si = 0; si < srclen; si++) {
			if (src[si] < n) {
				if (++delta == 0) {
					/* OVERFLOW */
					assert(0 && "OVERFLOW");
					goto fail;
				}
			}
			else if (src[si] == n) {
				di += encode_var_int(bias, delta, &dst[di], *dstlen - di);
				bias = adapt_bias(delta, h + 1, h == b);
				delta = 0;
				h++;
			}
		}
	}

fail:
	/* Tell the caller how many bytes were written to the output buffer. */
	*dstlen = di;

	/* Return how many Unicode code points were converted. */
	return si;
}

bool punycode_hasprefix(const String src) {
	return src.hasPrefix("xn--");
}

bool punycode_needEncode(const String src) {
	for (int si = 0; si < src.size(); si++) {
		if (src[si] & 0x80 || src[si] < 0x20) {
			return true;
		}
	}

	return false;
}

String punycode_decode(const String src1) {
	if (!src1.hasPrefix("xn--"))
		return src1;

	String src(&src1.c_str()[4]); // Skip the prefix for simplification
	int srclen = src.size();

	/* Ensure that the input contains only ASCII characters. */
	for (int si = 0; si < srclen; si++) {
		if (src[si] & 0x80) {
			return src1;
		}
	}

	size_t di = src.findLastOf('-');

	if (di == String::npos)
		return src;

	String dst;

	for (int i = 0; i < di; i++) {
		dst += src[i];
	}

	size_t b = di;
	int i = 0;
	int n = INITIAL_N;
	int bias = INITIAL_BIAS;

	for (int si = b + (b > 0 ? 1 : 0); si < srclen; di++) {
		int org_i = i;

		for (int w = 1, k = BASE; true; k += BASE) {
			int digit = decode_digit(src[si++]);

			if (digit == SIZE_MAX) {
				warning("punycode_decode: incorrect digit");
				return src1;
			}

			if (digit > (SIZE_MAX - i) / w) {
				/* OVERFLOW */
				warning("punycode_decode: overflow1");
				return src1;
			}

			i += digit * w;
			int t;

			if (k <= bias) {
				t = TMIN;
			} else if (k >= bias + TMAX) {
				t = TMAX;
			} else {
				t = k - bias;
			}

			if (digit < t) {
				break;
			}

			if (w > SIZE_MAX / (BASE - t)) {
				/* OVERFLOW */
				warning("punycode_decode: overflow2");
				return src1;
			}

			w *= BASE - t;
		}

		bias = adapt_bias(i - org_i, di + 1, org_i == 0);

		if (i / (di + 1) > SIZE_MAX - n) {
			/* OVERFLOW */
				warning("punycode_decode: overflow3");
			return src1;
		}

		n += i / (di + 1);
		i %= (di + 1);

		String dst1(dst.c_str(), i);
		dst1 += (char )n;
		dst1 += String(&dst.c_str()[i]);
		dst = dst1;
		i++;
	}

	return dst;
}

String punycode_decodefilename(const String src1) {
	String dst;
	String src = punycode_decode(src1);

	// Check if the string did not change which could be
	// also on decoding failure
	if (src == src1)
		return src;

	for (int i = 0; i < src.size(); i++) {
		if ((byte)src[i] == 0x81 && i + 1 < src.size()) {
			i++;
			if (src[i] == 0x79)
				dst += 0x81;
			else
				dst += src[i] - 0x80;
		} else {
			dst + src[i];
		}
	}

	return dst;
}


} // end of namespace Common
