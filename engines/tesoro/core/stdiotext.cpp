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

#include "tesoro/core/stdiotext.h"

namespace Tesoro {

void StdioText::skipSpace() {
	int c;
	do {
		c = get();
	} while (isSpace(c));
	unget(c);
}

static int digitValue(int c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 99;
}

bool StdioText::readInt(int32 &value) {
	skipSpace();
	int c = get();
	bool negative = false;
	if (c == '-' || c == '+') {
		negative = c == '-';
		c = get();
	}
	uint base = 10;
	uint32 v = 0;
	bool digits = false;
	if (c == '0') {
		digits = true;
		c = get();
		if (c == 'x' || c == 'X') {
			base = 16;
			c = get();
		} else {
			base = 8;
		}
	}
	while (digitValue(c) < (int)base) {
		v = v * base + digitValue(c);
		digits = true;
		c = get();
	}
	unget(c);
	if (!digits)
		return false;
	value = (int32)(negative ? 0u - v : v);
	return true;
}

/** Reads a %[...] conversion's set from the format and the matching input; returns the number of characters stored. */
uint StdioText::readSet(const char *&fmt, char *dst) {
	bool inSet[256] = {};
	++fmt;						// past '['
	const bool negate = *fmt == '^';
	if (negate)
		++fmt;
	bool first = true;
	while (*fmt && (*fmt != ']' || first)) {
		first = false;
		const uint lo = (byte)*fmt;
		if (fmt[1] == '-' && fmt[2] && fmt[2] != ']') {
			const uint hi = (byte)fmt[2];
			for (uint ch = lo; ch <= hi; ++ch)
				inSet[ch] = true;
			fmt += 3;
		} else {
			inSet[lo] = true;
			++fmt;
		}
	}
	uint n = 0;
	for (;;) {
		const int c = get();
		if (c == -1)
			break;
		if (inSet[c] == negate) {
			unget(c);
			break;
		}
		dst[n++] = (char)c;
	}
	if (n)
		dst[n] = '\0';
	return n;
}

void StdioText::scan(const char *fmt, void *a0, void *a1, void *a2) {
	void *args[3] = { a0, a1, a2 };
	uint next = 0;
	for (; *fmt; ++fmt) {
		if (isSpace((byte)*fmt)) {
			skipSpace();
			continue;
		}
		if (*fmt != '%') {
			const int c = get();
			if (c != (byte)*fmt) {
				unget(c);
				return;
			}
			continue;
		}
		++fmt;
		if (*fmt == 'i') {
			int32 v;
			if (!readInt(v))
				return;
			WRITE_LE_UINT32(args[next++], (uint32)v);
		} else if (*fmt == 's') {
			skipSpace();
			char *dst = (char *)args[next++];
			uint n = 0;
			for (;;) {
				const int c = get();
				if (c == -1 || isSpace(c)) {
					unget(c);
					break;
				}
				dst[n++] = (char)c;
			}
			if (!n)
				return;
			dst[n] = '\0';
		} else if (*fmt == '[') {
			if (!readSet(fmt, (char *)args[next++]))
				return;
		} else {
			return;
		}
	}
}

} // End of namespace Tesoro
