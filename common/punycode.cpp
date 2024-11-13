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

// punycode parameters, see https://datatracker.ietf.org/doc/html/rfc3492#section-5
#define BASE 36
#define TMIN 1
#define TMAX 26
#define SKEW 38
#define DAMP 700
#define INITIAL_N 128
#define INITIAL_BIAS 72
#define SMAX 2147483647 // maximum Unicode code point

#define SPECIAL_SYMBOLS "/\":*|\\?%<>\x7f"

static uint32 adapt_bias(uint32 delta, unsigned n_points, int is_first) {
	uint32 k;

	delta /= is_first ? DAMP : 2;
	delta += delta / n_points;

	// while delta > 455: delta /= 35
	for (k = 0; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE) {
		delta /= (BASE - TMIN);
	}

	return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
}

static char encode_digit(int c) {
	assert(c >= 0 && c <= BASE - TMIN);
	if (c > 25) {
		return c + 22; // '0'..'9'
	} else {
		return c + 0x61; // 'a'..'z'
	}
}

// Encode as a generalized variable-length integer. Returns number of bytes written.
static String encode_var_int(const size_t bias, const size_t delta) {
	size_t k, q, t;
	String dst;

	k = BASE;
	q = delta;

	while (true) {
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

		dst += encode_digit(t + (q - t) % (BASE - t));

		q = (q - t) / (BASE - t);
		k += BASE;
	}

	dst += encode_digit(q);

	return dst;
}

static size_t decode_digit(uint32 v) {
	if (Common::isDigit(v)) {
		return 26 + (v - '0');
	}
	if (Common::isLower(v)) {
		return v - 'a';
	}
	if (Common::isUpper(v)) {
		return v - 'A';
	}
	return SMAX;
}

String punycode_encode(const U32String &src) {
	size_t srclen = src.size();
	size_t h = 0, si;
	String dst = "xn--";

	if (!srclen)
		return src;

	for (si = 0; si < srclen; si++) {
		if (src[si] < 128) {
			dst += src[si];
			h++;
		}
	}

	size_t b = h;

	// If every character is ASCII, return the original string.
	if (h == srclen) {
		// If string ends with space or dot, still punycode it
		// because certain FSes do not support files with those
		// endings
		if (src[h - 1] == ' ' || src[h - 1] == '.')
			return dst + '-';
		if (srclen > 4 && src[0] == 'x' && src[1] == 'n' &&
			src[2] == '-' && src[3] == '-')
			return dst + '-';


		return src;
	}

	// If we have any ASCII characters, add '-' to separate them from
	// the non-ASCII character insertions.
	if (h > 0)
		dst += "-";

	size_t n = INITIAL_N;
	size_t bias = INITIAL_BIAS;
	size_t delta = 0;
	size_t m;

	for (; h < srclen; n++, delta++) {
		// Find next smallest non-basic code point.
		for (m = SMAX, si = 0; si < srclen; si++) {
			if (src[si] >= n && src[si] < m) {
				m = src[si];
			}
		}

		if ((m - n) > (SMAX - delta) / (h + 1)) {
			// OVERFLOW
			warning("punycode_encode: overflow1 for string (%s)", src.encode().c_str());
			return src;
		}

		delta += (m - n) * (h + 1);
		n = m;

		for (si = 0; si < srclen; si++) {
			if (src[si] < n) {
				if (++delta == 0) {
					// OVERFLOW
					warning("punycode_encode: overflow2 for string (%s)", src.encode().c_str());
					return src;
				}
			} else if (src[si] == n) {
				dst += encode_var_int(bias, delta);
				bias = adapt_bias(delta, h + 1, h == b);
				delta = 0;
				h++;
			}
		}
	}

	// Return how many Unicode code points were converted.
	return dst;
}

bool punycode_needEncode(const String &src) {
	if (!src.size())
		return false;

	// If name begins with xn-- this could become ambiguous
	if (src.size() > 4 && src[0] == 'x' && src[1] == 'n' &&
		src[2] == '-' && src[3] == '-')
		return true;

	for (uint si = 0; si < src.size(); si++) {
		if (src[si] & 0x80 || src[si] < 0x20 || strchr(SPECIAL_SYMBOLS, src[si])) {
			return true;
		}
	}

	// If name ends with space or dot, we have to encode it
	if (src[src.size() - 1] == ' ' || src[src.size() - 1] == '.')
		return true;

	return false;
}

U32String punycode_decode(const String &src1, bool *error) {
	if (error) *error = true;
	if (!src1.hasPrefix("xn--")) {
		return src1;
	}

	const char *src = src1.c_str() + 4; // Skip the prefix for simplification
	uint srclen = src1.size() - 4;

	// Ensure that the input contains only ASCII characters.
	for (uint si = 0; si < srclen; si++) {
		if (src[si] & 0x80) {
			return src1;
		}
	}

	// Look for start insertions
	const char *startinsert = strrchr(src, '-');
	// If we have no '-', the entire string is non-ASCII character insertions.
	if (!startinsert) {
		startinsert = src;
	} else {
		startinsert++;
	}

	const char *tail = src + srclen;

	while(true) {
		// Check that the insertions string is valid
		const char *p;
		for(p = startinsert; p < tail; p++) {
			if (!((*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z'))) {
				// Invalid code
				break;
			}
		}
		if (p == tail) {
			// Everything was good
			break;
		}

		if (*p == '.') {
			// Until then we got valid insertions, chop off what looks like an extension
			// We are now good
			tail = p;
			srclen = tail - src;
			break;
		}

		// We got something invalid: assume it was garbage from start
		// Look for another insertions start
		if (startinsert == src) {
			warning("punycode_decode: malformed string for string (%s)", src1.c_str());
			return src1;
		}

		// Move back to the dash
		startinsert--;
		tail = startinsert;
		for(; startinsert > src; startinsert--) {
			if (*(startinsert - 1) == '-') {
				break;
			}
		}
		// Try another round
	}

	// Create dst with ASCII characters
	U32String dst(src, startinsert == src ? src : startinsert - 1);

	const char *b = startinsert;
	size_t di = dst.size();
	size_t i = 0;
	size_t n = INITIAL_N;
	size_t bias = INITIAL_BIAS;

	for (const char *si = b; si < tail; di++) {
		size_t org_i = i;

		for (size_t w = 1, k = BASE; true; k += BASE) {
			if (si >= tail) {
				warning("punycode_decode: incorrect digit for string (%s)", src1.c_str());
				return src1;
			}

			size_t digit = decode_digit(*si++);

			if (digit == SMAX) {
				warning("punycode_decode: incorrect digit2 for string (%s)", src1.c_str());
				return src1;
			}

			if (digit > (SMAX - i) / w) {
				// OVERFLOW
				warning("punycode_decode: overflow1 for string (%s)", src1.c_str());
				return src1;
			}

			i += digit * w;
			size_t t;

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

			if (w > SMAX / (BASE - t)) {
				// OVERFLOW
				warning("punycode_decode: overflow2 for string (%s)", src1.c_str());
				return src1;
			}

			w *= BASE - t;
		}

		bias = adapt_bias(i - org_i, di + 1, org_i == 0);

		if (i / (di + 1) > SMAX - n) {
			// OVERFLOW
			warning("punycode_decode: overflow3 for string (%s)", src1.c_str());
			return src1;
		}

		n += i / (di + 1);
		i %= (di + 1);

		dst.insertChar(n, i);
		i++;
	}

	// If we chopped off tail, readd it here
	dst += Common::U32String(tail);

	debug(9, "punycode_decode: returning %s", Common::U32String(dst).encode().c_str());
	if (error) *error = false;
	return dst;
}

String punycode_encodefilename(const U32String &src) {
	U32String dst;

	for (uint i = 0; i < src.size(); i++) {
		if (src[i] == 0x81) {	// In case we have our escape character present
			dst += 0x81;
			dst += 0x79;
		// Encode special symbols and non-printables
		} else if ((src[i] < 0x80 && strchr(SPECIAL_SYMBOLS, (byte)src[i])) || src[i] < 0x20) {
			dst += 0x81;
			dst += src[i] + 0x80;
		} else {
			dst += src[i];
		}
	}

	return punycode_encode(dst);
}

U32String punycode_decodefilename(const String &src) {
	bool error;
	U32String dst = punycode_decode(src, &error);

	if (error)
		return dst;

	uint32 i = dst.find(0x81, 0);
	while (i < dst.size() - 1) {
		U32String::value_type c = dst[i + 1];
		if (c == 0x79) {
			// 0x81 0x79 means 0x81: delete 0x79
			dst.deleteChar(i + 1);
		} else {
			// 0x81 0xXX means 0xXX - 0x80: delete 0x81 and change 0xXX
			dst.deleteChar(i);
			dst.setChar(c - 0x80, i);
		}
		i = dst.find(0x81, i + 1);
	}

	return dst;
}

} // end of namespace Common
