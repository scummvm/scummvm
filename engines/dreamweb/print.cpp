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

#include "dreamweb/dreamweb.h"

namespace DreamGen {

void DreamGenContext::printboth(const Frame *charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar) {
	uint16 newX = *x;
	uint8 width, height;
	printchar(charSet, &newX, y, c, nextChar, &width, &height);
	multidump(*x, y, width, height);
	*x = newX;
}

uint8 DreamGenContext::getnextword(const Frame *charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount) {
	*totalWidth = 0;
	*charCount = 0;
	while(true) {
		uint8 firstChar = *string;
		++string;
		++*charCount;
		if ((firstChar == ':') || (firstChar == 0)) { //endall
			*totalWidth += 6;
			return 1;
		}
		if (firstChar == 32) { //endword
			*totalWidth += 6;
			return 0;
		}
		firstChar = engine->modifyChar(firstChar);
		if (firstChar != 255) {
			uint8 secondChar = *string;
			uint8 width = charSet[firstChar - 32 + data.word(kCharshift)].width;
			width = kernchars(firstChar, secondChar, width);
			*totalWidth += width;
		}
	}
}

void DreamGenContext::printchar() {
	uint16 x = di;
	uint8 width, height;
	printchar((const Frame *)ds.ptr(0, 0), &x, bx, al, ah, &width, &height);
	di = x;
	cl = width;
	ch = height;
}

void DreamGenContext::printchar(const Frame *charSet, uint16* x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height) {
	if (c == 255)
		return;
	push(si);
	push(di);
	if (data.byte(kForeignrelease) != 0)
		y -= 3;
	uint16 tmp = c - 32 + data.word(kCharshift);
	showframe(charSet, *x, y, tmp & 0x1ff, (tmp >> 8) & 0xfe, width, height);
	di = pop();
	si = pop();
	_cmp(data.byte(kKerning), 0);
	if (flags.z())
		*width = kernchars(c, nextChar, *width);
	(*x) += *width;
}

void DreamGenContext::printslow() {
	al = printslow(es.ptr(si, 0), di, bx, dl, (bool)(dl & 1));
}

uint8 DreamGenContext::printslow(const uint8 *string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	data.byte(kPointerframe) = 1;
	data.byte(kPointermode) = 3;
	const Frame* charSet = (const Frame *)segRef(data.word(kCharset1)).ptr(0, 0);
	do {
		uint16 offset = x;
		uint16 charCount = getnumber(charSet, string, maxWidth, centered, &offset);
		do {
			uint8 c0 = string[0];
			uint8 c1 = string[1];
			uint8 c2 = string[2];
			c0 = engine->modifyChar(c0);
			printboth(charSet, &offset, y, c0, c1);
			if ((c1 == 0) || (c1 == ':')) {
				return 0;
			}
			if (charCount != 1) {
				c1 = engine->modifyChar(c1);
				data.word(kCharshift) = 91;
				uint16 offset2 = offset;
				printboth(charSet, &offset2, y, c1, c2);
				data.word(kCharshift) = 0;
				for (int i=0; i<2; ++i) {
					uint16 mouseState = waitframes();
					if (data.byte(kQuitrequested))
						return 0;
					if (mouseState == 0)
						continue;
					if (mouseState != data.word(kOldbutton)) {
						return 1;
					}
				}
			}

			++string;
			--charCount;
		} while (charCount);
		y += 10;
	} while (true);
}

void DreamGenContext::printdirect() {
	uint16 y = bx;
	uint16 initialSi = si;
	const uint8 *initialString = es.ptr(si, 0);
	const uint8 *string = initialString;
	printdirect(&string, di, &y, dl, (bool)(dl & 1));
	si = initialSi + (string - initialString);
	bx = y;
}

void DreamGenContext::printdirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	printdirect(&string, x, &y, maxWidth, centered);
}

void DreamGenContext::printdirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
	data.word(kLastxpos) = x;
	const Frame *charSet = (const Frame *)segRef(data.word(kCurrentset)).ptr(0, 0);
	while (true) {
		uint16 offset = x;
		uint8 charCount = getnumber(charSet, *string, maxWidth, centered, &offset);
		uint16 i = offset;
		do {
			uint8 c = (*string)[0];
			uint8 nextChar = (*string)[1];
			++(*string);
			if ((c == 0) || (c == ':')) {
				return;
			}
			c = engine->modifyChar(c);
			uint8 width, height;
			printchar(charSet, &i, *y, c, nextChar, &width, &height);
			data.word(kLastxpos) = i;
			--charCount;
		} while(charCount);
		*y += data.word(kLinespacing);
	}
}

void DreamGenContext::getnumber() {
	uint16 offset = di;
	cl = getnumber((Frame *)ds.ptr(0, 0), es.ptr(si, 0), dl, (bool)(dl & 1), &offset);
	di = offset;
}

uint8 DreamGenContext::getnumber(const Frame *charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16* offset) {
	uint8 totalWidth = 0;
	uint8 charCount = 0;
	while (true) {
		uint8 wordTotalWidth, wordCharCount;
		uint8 done = getnextword(charSet, string, &wordTotalWidth, &wordCharCount);
		string += wordCharCount;

		if (done == 1) { //endoftext
			ax = totalWidth + wordTotalWidth - 10;
			if (ax < maxWidth) {
				totalWidth += wordTotalWidth;
				charCount += wordCharCount;
			}

			if (centered) {
				ax = (maxWidth & 0xfe) + 2 + 20 - totalWidth;
				ax /= 2;
			} else {
				ax = 0;
			}
			*offset += ax;
			return charCount;
		}
		ax = totalWidth + wordTotalWidth - 10;
		if (ax >= maxWidth) { //gotoverend
			if (centered) {
				ax = (maxWidth & 0xfe) - totalWidth + 20;
				ax /= 2;
			} else {
				ax = 0;
			}
			*offset += ax;
			return charCount;
		}
		totalWidth += wordTotalWidth;
		charCount += wordCharCount;
	}
}

uint8 DreamGenContext::kernchars(uint8 firstChar, uint8 secondChar, uint8 width) {
	if ((firstChar == 'a') || (al == 'u')) {
		if ((secondChar == 'n') || (secondChar == 't') || (secondChar == 'r') || (secondChar == 'i') || (secondChar == 'l'))
			return width-1;
	}
	return width;
}

uint16 DreamGenContext::waitframes() {
	readmouse();
	showpointer();
	vsync();
	dumppointer();
	delpointer();
	return data.word(kMousebutton);
}

} /*namespace dreamgen */

