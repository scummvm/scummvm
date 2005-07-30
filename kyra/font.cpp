/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "kyra/resource.h"
#include "common/stream.h"
#include "common/array.h"

#ifdef DUMP_FILES
#include <cstdio>
#endif

namespace Kyra {
const uint16 FontHeader_Magic1 = 0x0500;
const uint16 FontHeader_Magic2 = 0x000e;
const uint16 FontHeader_Magic3 = 0x0014;

Font::Font(uint8* buffer, uint32 size) {
	if (!buffer) {
		error("resource created without data");
	}

	_buffer = buffer;

	Common::MemoryReadStream bufferstream(buffer, size);

	_fontHeader._size = bufferstream.readUint16LE();
	_fontHeader._magic1 = bufferstream.readUint16LE();
	_fontHeader._magic2 = bufferstream.readUint16LE();
	_fontHeader._magic3 = bufferstream.readUint16LE();
	_fontHeader._charWidthOffset = bufferstream.readUint16LE();
	_fontHeader._charBitsOffset = bufferstream.readUint16LE();
	_fontHeader._charHeightOffset = bufferstream.readUint16LE();
	_fontHeader._version = bufferstream.readUint16LE();
	_fontHeader._countChars = bufferstream.readUint16LE();
	_fontHeader._width = bufferstream.readByte();
	_fontHeader._height = bufferstream.readByte();

	// tests for the magic values
	if (_fontHeader._magic1 != FontHeader_Magic1 || _fontHeader._magic2 != FontHeader_Magic2 ||
		_fontHeader._magic3 != FontHeader_Magic3) {
		error("magic vars in the fontheader are corrupt\n"
			  "_magic1 = 0x%x, _magic2 = 0x%x, _magic3 = 0x%x",
			_fontHeader._magic1, _fontHeader._magic2, _fontHeader._magic3);
	}

	// init all the pointers
	_offsetTable = (uint16*)&buffer[bufferstream.pos()];
	_charWidth = &buffer[_fontHeader._charWidthOffset];
	_charHeight = (uint16*)&buffer[_fontHeader._charHeightOffset];
	_charBits = &buffer[_fontHeader._charBitsOffset];

	// now prerender =)
	preRenderAllChars(bufferstream.pos());

	// This value seems to be a version or language variable
	// Known Values
	// ------------
	// Russian Floppy:			0x1010
	// German Floppy and English/German CD:	0x1011
	debug("Font::_version = 0x%x", _fontHeader._version);

	delete [] _buffer;
	_buffer = 0;
	_offsetTable = 0;
	_charHeight = 0;
	_charWidth = 0;
	_charBits = 0;
}

Font::~Font() {
	// FIXME: Release memory of the prerendered chars
}

uint32 Font::getStringWidth(const char* string, char terminator) {
	uint32 strsize;

	for (strsize = 0; string[strsize] != terminator && string[strsize] != '\0'; ++strsize)
		;

	uint32 stringwidth = 0;

	for (uint32 pos = 0; pos < strsize; ++pos) {
		stringwidth += _preRenderedChars[string[pos]].width;
	}

	return stringwidth;
}

const uint8* Font::getChar(char c, uint8* width, uint8* height, uint8* heightadd) {
	PreRenderedChar& c_ = _preRenderedChars[c];

	*width = c_.width;
	*height = c_.height;
	*heightadd = c_.heightadd;

	return c_.c;
}

// splits up the String in a word
const char* Font::getNextWord(const char* string, uint32* size) {
	uint32 startpos = 0;
	*size = 0;

	// gets start of the word
	for (; string[startpos] == ' '; ++startpos)
		;

	// not counting size
	for (*size = 0; string[startpos + *size] != ' ' && string[startpos + *size] != '\0'; ++(*size))
		;

	++(*size);

	return &string[startpos];
}

// Move this to Font declaration?
struct WordChunk {
	const char* _string;
	uint32 _size;
};

void Font::drawStringToPlane(const char* string,
				uint8* plane, uint16 planewidth, uint16 planeheight,
				uint16 x, uint16 y, uint8 color) {

	// lets do it word after word
	Common::Array<WordChunk> words;

	uint32 lastPos = 0;
	uint32 lastSize = 0;
	uint32 strlgt = strlen(string);

	while (true) {
		WordChunk newchunk;
		newchunk._string = getNextWord(&string[lastPos], &lastSize);
		newchunk._size = lastSize;
		lastPos += lastSize;

		words.push_back(newchunk);

		if (lastPos >= strlgt)
			break;
	}

	uint16 current_x = x, current_y = y;
	uint8 heighest = 0;

	const uint8* src = 0;
	uint8 width = 0, height = 0, heightadd = 0;

	// now the have alle of these words
	for (uint32 tmp = 0; tmp < words.size(); ++tmp) {
		lastSize = getStringWidth(words[tmp]._string, ' ');

		// adjust x position
		if (current_x + lastSize >= planewidth) {
			// hmm lets move it a bit to the left
			if (current_x == x && (int16)planewidth - (int16)lastSize >= 0) {
				current_x = planewidth - lastSize;
			} else {
				current_x = x;
				if (heighest)
					current_y += heighest + 2;
				else // now we are using just the fist char :)
					current_y += _preRenderedChars[words[tmp]._string[0]].height;
				heighest = 0;
			}
		}

		// TODO: maybe test if current_y >= planeheight ?

		// output word :)
		for (lastPos = 0; lastPos < words[tmp]._size; ++lastPos) {
			if (words[tmp]._string[lastPos] == '\0')
				break;

			// gets our char :)
			src = getChar(words[tmp]._string[lastPos], &width, &height, &heightadd);

			// lets draw our char
			drawCharToPlane(src, color, width, height, plane, planewidth, planeheight, current_x, current_y + heightadd);

			current_x += width;
			heighest = MAX(heighest, height);
		}
	}
}

void Font::drawCharToPlane(const uint8* c, uint8 color, uint8 width, uint8 height,
				uint8* plane, uint16 planewidth, uint16 planeheight, uint16 x, uint16 y) {
	const uint8* src = c;

	// blit them to the screen
	for (uint8 yadd = 0; yadd < height; ++yadd) {
		for (uint8 xadd = 0; xadd < width; ++xadd) {
			switch(*src) {
			case 1:
				plane[(y + yadd) * planewidth + x + xadd] = color;
				break;

			case 2:
				plane[(y + yadd) * planewidth + x + xadd] = 14;
				break;

			case 3:
				plane[(y + yadd) * planewidth + x + xadd] = 0;
				break;

			default:
				// nothing to do now
				break;
			};

			++src;
		}
	}
}

void Font::preRenderAllChars(uint16 offsetTableOffset) {
	uint16 startOffset = _offsetTable[0];
	uint16 currentOffset = offsetTableOffset;
	uint8 currentChar = 0;

	for (; currentOffset < startOffset; ++currentChar, currentOffset += sizeof(uint16)) {
		// lets prerender the char :)

		PreRenderedChar newChar;

		newChar.height = READ_LE_UINT16(&_charHeight[currentChar]) >> 8;
		newChar.width = _charWidth[currentChar];
		newChar.heightadd = READ_LE_UINT16(&_charHeight[currentChar]) & 0xFF;
		newChar.c = new uint8[newChar.height * newChar.width];
		assert(newChar.c);
		memset(newChar.c, 0, sizeof(uint8) * newChar.height * newChar.width);

		uint8* src = _buffer + READ_LE_UINT16(&_offsetTable[currentChar]);
		uint8* dst = &newChar.c[0];
		uint8 index = 0;

#ifdef DUMP_FILES
		static char filename[32] = { 0 };
		sprintf(filename, "dumps/char%d.dmp", currentChar);
		FILE* dump = fopen(filename, "w+");
		assert(dump);

		fprintf(dump, "This should be a '%c'\n", currentChar);
#endif

		// prerender the char
		for (uint8 yadd = 0; yadd < newChar.height; ++yadd) {
			for (uint8 xadd = 0; xadd < newChar.width; ++xadd) {
				if (xadd % 2) {
					index = ((*src) & 0xF0) >> 4;
					++src;
				} else {
					index = (*src) & 0x0F;
				}

				switch(index) {
				case 1:
#ifdef DUMP_FILES
					fprintf(dump, "#");
#endif
					dst[yadd * newChar.width + xadd] = 1;
					break;

				case 2:
#ifdef DUMP_FILES
					fprintf(dump, "$");
#endif
					dst[yadd * newChar.width + xadd] = 2;
					break;

				case 3:
#ifdef DUMP_FILES
					fprintf(dump, "§");
#endif
					dst[yadd * newChar.width + xadd] = 3;
					break;

				default:
#ifdef DUMP_FILES
					fprintf(dump, "%d", index);
#endif
					break;
				};
			}

			if (newChar.width % 2) {
				++src;
			}
#ifdef DUMP_FILES
			fprintf(dump, "\n");
#endif
		}

#ifdef DUMP_FILES
		fprintf(dump, "\nThis is the created map:\n");
		// now print the whole thing again
		for (uint8 yadd = 0; yadd < newChar.height; ++yadd) {
			for (uint8 xadd = 0; xadd < newChar.width; ++xadd) {
				fprintf(dump, "%d", dst[yadd * newChar.width + xadd]);
			}
			fprintf(dump, "\n");
		}
		fclose(dump);
#endif

		_preRenderedChars[currentChar] = newChar;

		if (currentChar == 255) {
			break;
		}
	}
}
} // end of namespace Kyra

