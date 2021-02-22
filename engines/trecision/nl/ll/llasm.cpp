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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

namespace Trecision {

void wordcopy(void *dest, void *src, uint32 len) {
	uint16 *d = (uint16 *)dest, *s = (uint16 *)src;
	for (uint32 i = 0; i < len; i++)
		*d++ = *s++;
}
void longcopy(void *dest, void *src, uint32 len) {
	uint32 *d = (uint32 *)dest, *s = (uint32 *)src;
	for (uint32 i = 0; i < len; i++)
		*d++ = *s++;
}
void wordset(void *dest, uint16 value, uint32 len) {
	uint16 *d = (uint16 *)dest;
	for (uint32 i = 0; i < len; i++)
		*d++ = value;
}
void longset(void *dest, uint32 value, uint32 len) {
	uint32 *d = (uint32 *)dest;
	for (uint32 i = 0; i < len; i++)
		*d++ = value;
}

void byte2word(void *dest, void *src, void *data, uint32 len) {
	uint16 *d = (uint16 *)dest, *p = (uint16 *)data;
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++)
		*d++ = p[*s++];
}
void byte2long(void *dest, void *src, void *data, uint32 len) {
	uint32 *d = (uint32 *)dest, *p = (uint32 *)data;
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++)
		*d++ = p[*s++];
}
void byte2wordm(void *dest, void *src, void *data, uint32 len) {
	uint16 *d = (uint16 *)dest, *p = (uint16 *)data;
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++) {
		uint8 v = *s++;
		if (v == 0)
			d++;
		else
			*d++ = p[v];
	}
}
void byte2wordn(void *dest, void *src, void *data, uint32 len) {
	uint16 *d = (uint16 *)dest, *p = (uint16 *)data, *t = (uint16 *)((uint8 *)(dest) - 537600);
	uint8 *s = (uint8 *)src;
	for (uint32 i = 0; i < len; i++) {
		uint8 v = *s++;
		if (v == 0)
			*d++ = *t++;
		else {
			*d++ = p[v];
			t++;
		}
	}
}

} // End of namespace Trecision
