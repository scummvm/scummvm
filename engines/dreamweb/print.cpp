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

void DreamBase::printBoth(const Frame *charSet, uint16 *x, uint16 y, uint8 c, uint8 nextChar) {
	uint16 newX = *x;
	uint8 width, height;
	printChar(charSet, &newX, y, c, nextChar, &width, &height);
	multiDump(*x, y, width, height);
	*x = newX;
}

uint8 DreamBase::getNextWord(const Frame *charSet, const uint8 *string, uint8 *totalWidth, uint8 *charCount) {
	*totalWidth = 0;
	*charCount = 0;
	while (true) {
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
			width = kernChars(firstChar, secondChar, width);
			*totalWidth += width;
		}
	}
}

void DreamBase::printChar(const Frame *charSet, uint16* x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height) {
	if (c == 255)
		return;

	uint8 dummyWidth, dummyHeight;
	if (width == NULL)
		width = &dummyWidth;
	if (height == NULL)
		height = &dummyHeight;
	if (data.byte(kForeignrelease))
		y -= 3;
	uint16 tmp = c - 32 + data.word(kCharshift);
	showFrame(charSet, *x, y, tmp & 0x1ff, (tmp >> 8) & 0xfe, width, height);
	if (data.byte(kKerning), 0)
		*width = kernChars(c, nextChar, *width);
	(*x) += *width;
}

void DreamBase::printChar(const Frame *charSet, uint16 x, uint16 y, uint8 c, uint8 nextChar, uint8 *width, uint8 *height) {
	printChar(charSet, &x, y, c, nextChar, width, height);
}

uint8 DreamBase::printSlow(const uint8 *string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	data.byte(kPointerframe) = 1;
	data.byte(kPointermode) = 3;
	const Frame* charSet = (const Frame *)getSegment(data.word(kCharset1)).ptr(0, 0);
	do {
		uint16 offset = x;
		uint16 charCount = getNumber(charSet, string, maxWidth, centered, &offset);
		do {
			uint8 c0 = string[0];
			uint8 c1 = string[1];
			uint8 c2 = string[2];
			c0 = engine->modifyChar(c0);
			printBoth(charSet, &offset, y, c0, c1);
			if ((c1 == 0) || (c1 == ':')) {
				return 0;
			}
			if (charCount != 1) {
				c1 = engine->modifyChar(c1);
				data.word(kCharshift) = 91;
				uint16 offset2 = offset;
				printBoth(charSet, &offset2, y, c1, c2);
				data.word(kCharshift) = 0;
				for (int i=0; i<2; ++i) {
					uint16 mouseState = waitFrames();
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

void DreamGenContext::printDirect() {
	uint16 y = bx;
	const uint8 *initialString = es.ptr(si, 0);
	const uint8 *string = initialString;
	al = DreamBase::printDirect(&string, di, &y, dl, (bool)(dl & 1));
	si += (string - initialString);
	bx = y;
}

uint8 DreamBase::printDirect(const uint8* string, uint16 x, uint16 y, uint8 maxWidth, bool centered) {
	return printDirect(&string, x, &y, maxWidth, centered);
}

uint8 DreamBase::printDirect(const uint8** string, uint16 x, uint16 *y, uint8 maxWidth, bool centered) {
	data.word(kLastxpos) = x;
	const Frame *charSet = engine->currentCharset();
	while (true) {
		uint16 offset = x;
		uint8 charCount = getNumber(charSet, *string, maxWidth, centered, &offset);
		uint16 i = offset;
		do {
			uint8 c = (*string)[0];
			uint8 nextChar = (*string)[1];
			++(*string);
			if ((c == 0) || (c == ':')) {
				return c;
			}
			c = engine->modifyChar(c);
			uint8 width, height;
			printChar(charSet, &i, *y, c, nextChar, &width, &height);
			data.word(kLastxpos) = i;
			--charCount;
		} while (charCount);
		*y += data.word(kLinespacing);
	}
}

uint8 DreamBase::getNumber(const Frame *charSet, const uint8 *string, uint16 maxWidth, bool centered, uint16* offset) {
	uint8 totalWidth = 0;
	uint8 charCount = 0;
	while (true) {
		uint8 wordTotalWidth, wordCharCount;
		uint8 done = getNextWord(charSet, string, &wordTotalWidth, &wordCharCount);
		string += wordCharCount;

		uint16 tmp = totalWidth + wordTotalWidth - 10;
		if (done == 1) { //endoftext
			if (tmp < maxWidth) {
				totalWidth += wordTotalWidth;
				charCount += wordCharCount;
			}

			if (centered) {
				tmp = (maxWidth & 0xfe) + 2 + 20 - totalWidth;
				tmp /= 2;
			} else {
				tmp = 0;
			}
			*offset += tmp;
			return charCount;
		}
		if (tmp >= maxWidth) { //gotoverend
			if (centered) {
				tmp = (maxWidth & 0xfe) - totalWidth + 20;
				tmp /= 2;
			} else {
				tmp = 0;
			}
			*offset += tmp;
			return charCount;
		}
		totalWidth += wordTotalWidth;
		charCount += wordCharCount;
	}
}

uint8 DreamBase::kernChars(uint8 firstChar, uint8 secondChar, uint8 width) {
	if ((firstChar == 'a') || (firstChar == 'u')) {
		if ((secondChar == 'n') || (secondChar == 't') || (secondChar == 'r') || (secondChar == 'i') || (secondChar == 'l'))
			return width-1;
	}
	return width;
}

uint16 DreamBase::waitFrames() {
	readMouse();
	showPointer();
	vSync();
	dumpPointer();
	delPointer();
	return data.word(kMousebutton);
}

void DreamGenContext::monPrint() {
	uint16 originalBx = bx;
	const char *string = (const char *)es.ptr(bx, 0);
	const char *nextString = monPrint(string);
	bx = originalBx + (nextString - string);
}

const char *DreamBase::monPrint(const char *string) {
	data.byte(kKerning) = 1;
	uint16 x = data.word(kMonadx);
	Frame *charset = engine->tempCharset();
	const char *iterator = string;
	bool done = false;
	while (!done) {

		uint16 count = getNumber(charset, (const uint8 *)iterator, 166, false, &x);
		do {	
			char c = *iterator++;
			if (c == ':')
				break;
			if ((c == 0) || (c == '"') || (c == '=')) {
				done = true;
				break;
			}
			if (c == '%') {
				data.byte(kLasttrigger) = *iterator;
				iterator += 2;
				done = true;
				break;
			}
			c = engine->modifyChar(c);
			printChar(charset, &x, data.word(kMonady), c, 0, NULL, NULL);
			data.word(kCurslocx) = x;
			data.word(kCurslocy) = data.word(kMonady);
			data.word(kMaintimer) = 1;
			printCurs();
			vSync();
			lockMon();
			delCurs();
		} while (--count);

		x = data.word(kMonadx);
		scrollMonitor();
		data.word(kCurslocx) = data.word(kMonadx);
	}

	data.byte(kKerning) = 0;
	return iterator;
}

} // End of namespace DreamGen
