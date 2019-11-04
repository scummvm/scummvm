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

#include "common/system.h"
#include "engines/chewy/text.h"

text::text() {
}
text::~text() {
}

void text::crypt(char *txt, uint32 size) {
	uint8 *sp;
	uint32 i;
	sp = (uint8 *)txt;
	for (i = 0; i < size; i++) {
		*sp = -(*sp);
		++sp;
	}
}

char *text::str_pos(char *txt_adr, int16 pos) {
	char *ptr;
	int16 i;
	ptr = txt_adr;
	for (i = 0; i < pos;) {
		if (*ptr == 0)
			++i;
		++ptr;
	}
	return (ptr);
}
