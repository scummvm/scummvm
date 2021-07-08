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

#include "ags/lib/allegro/unicode.h"
#include "common/textconsole.h"

namespace AGS3 {

void set_uformat(int format) {
	// TODO: implementation
}

size_t ustrsize(const char *s) {
	return strlen(s);
}

/* utf8_getc:
 * Reads a character from a UTF - 8 string.
 */
/*static*/ int utf8_getc(const char *s) {
	int c = *((unsigned char *)(s++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((unsigned char *)(s++));

			if ((!(t & 0x80)) || (t & 0x40))
				return '^';

			c = (c << 6) | (t & 0x3F);
		}
	}

	return c;
}



/* utf8_getx:
 *  Reads a character from a UTF-8 string, advancing the pointer position.
 */
/*static*/ int utf8_getx(char **s) {
	int c = *((unsigned char *)((*s)++));
	int n, t;

	if (c & 0x80) {
		n = 1;
		while (c & (0x80 >> n))
			n++;

		c &= (1 << (8 - n)) - 1;

		while (--n > 0) {
			t = *((unsigned char *)((*s)++));

			if ((!(t & 0x80)) || (t & 0x40)) {
				(*s)--;
				return '^';
			}

			c = (c << 6) | (t & 0x3F);
		}
	}

	return c;
}



/* utf8_setc:
 *  Sets a character in a UTF-8 string.
 */
/*static*/ int utf8_setc(char *s, int c) {
	int size, bits, b, i;

	if (c < 128) {
		*s = c;
		return 1;
	}

	bits = 7;
	while (c >= (1 << bits))
		bits++;

	size = 2;
	b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	b -= (7 - size);
	s[0] = c >> b;

	for (i = 0; i < size; i++)
		s[0] |= (0x80 >> i);

	for (i = 1; i < size; i++) {
		b -= 6;
		s[i] = 0x80 | ((c >> b) & 0x3F);
	}

	return size;
}



/* utf8_width:
 *  Returns the width of a UTF-8 character.
 */
/*static*/ int utf8_width(const char *s) {
	int c = *((unsigned char *)s);
	int n = 1;

	if (c & 0x80) {
		while (c & (0x80 >> n))
			n++;
	}

	return n;
}



/* utf8_cwidth:
 *  Returns the width of a UTF-8 character.
 */
/*static*/ int utf8_cwidth(int c) {
	int size, bits, b;

	if (c < 128)
		return 1;

	bits = 7;
	while (c >= (1 << bits))
		bits++;

	size = 2;
	b = 11;

	while (b < bits) {
		size++;
		b += 5;
	}

	return size;
}

/* utf8_isok:
 *  Checks whether this character can be encoded in UTF-8 format.
 */
/*static*/ int utf8_isok(int c) {
	return true;
}

} // namespace AGS3
