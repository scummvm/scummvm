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
 * $URL$
 * $Id$
 *
 */

#include "common/endian.h"

#include "sci/exereader.h"
#include "sci/include/versions.h"

//namespace Sci {

#define VERSION_DETECT_BUF_SIZE 4096

// LZEXE related defines
// The LZEXE code is based on public domain code by Mitugu Kurizono

/* The amount of most recent data (in bytes) that we need to keep in the
** buffer. lzexe compression is based on copying chunks of previous data to
** form new data.
*/
#define LZEXE_WINDOW VERSION_DETECT_BUF_SIZE * 2

/* Buffer size. */
#define LZEXE_BUFFER_SIZE VERSION_DETECT_BUF_SIZE * 3

/* Maximum amount of data (in bytes) that can be in the buffer at the start
** of the decompression loop. The maximum amount of data that can be added
** to the buffer during a single step of the loop is 256 bytes.
*/
#define LZEXE_BUFFER_MAX (LZEXE_BUFFER_SIZE - 256)

int curBits;
byte lzexeBuf[2];

bool isGameExe(Common::SeekableReadStream *exeStream) {
	byte magic[4];
	if (exeStream->size() < VERSION_DETECT_BUF_SIZE)
		return false;

	// Read exe header
	exeStream->read(magic, 4);

	// Check if the header contains known magic bytes

	// Information obtained from http://magicdb.org/magic.db
	// Check if it's a DOS executable
	if (magic[0] == 'M' && magic[1] == 'Z') {
		return true;
	}

	// Check if it's an Amiga executable
	if ((magic[0] == 0x03 && magic[1] == 0xF3) ||
		(magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) {
		return true;
	}

	// Check if it's an Atari executable
	if ((magic[0] == 0x60 && magic[1] == 0x1A))
		return true;

	// Check if it's a Mac exe

	// Resource map offset
	int32 offset = (int32)READ_BE_UINT32(magic);
	offset += 28;
	if (exeStream->size() <= offset)
		return false;

	// Skip number of types in map
	exeStream->skip(2);
	uint16 val = exeStream->readUint16BE() + 1;

	// Keep reading till we find the "CODE" bit
	while (!exeStream->eos()) {
		exeStream->skip(4);
		if (exeStream->eos())
			return false;

		exeStream->read(magic, 4);
		if (exeStream->eos())
			return false;

		if (!memcmp(magic, "CODE", 4)) {
			return true;
		}
		// Skip to the next list entry
		exeStream->skip(4);
		if (exeStream->eos())
			return false;
	}

	// If we've reached here, the file type is unknown
	return false;
}

bool isLZEXECompressed(Common::SeekableReadStream *exeStream) {
	uint32 filepos = 0;

	exeStream->seek(0, SEEK_SET);

	// First 2 bytes should be "MZ" (0x5A4D)
	if (exeStream->readUint16LE() != 0x5A4D)	// at pos 0, +2
		return false;

	exeStream->skip(6);

	// Header size should be 2
	filepos = exeStream->readUint16LE();
	if (filepos != 2)							// at pos 8, +2
		return false;

	exeStream->skip(12);

	// Calculate code segment offset in exe file
	filepos += exeStream->readUint16LE();		// at pos 22, +2

	// First relocation item offset should be 0x1c
	if (exeStream->readUint16LE() != 0x1c)		// at pos 24, +2
		return false;

	// Number of overlays should be 0
	if (exeStream->readUint16LE() != 0)			// at pos 26, +2
		return false;

	// Look for LZEXE signature
	byte magic[4];
	exeStream->read(magic, 4);

	if (memcmp(magic, "LZ09", 4) && memcmp(magic, "LZ91", 4))
		return false;

	// Seek to offset 8 of info table at start of code segment
	exeStream->seek(filepos + 8, SEEK_SET);
	if (exeStream->err())
		return false;

	// Read size of compressed data in paragraphs
	uint16 size = exeStream->readUint16LE();

	// Move file pointer to start of compressed data
	filepos -= size << 4;
	exeStream->seek(filepos, SEEK_SET);
	if (exeStream->err())
		return false;

	// All conditions met, this is an LZEXE packed file
	// We are currently at the start of the compressed file data
	return true;
}

byte getBitFromlzexe(Common::SeekableReadStream *exeStream) {
	byte result = lzexeBuf[1] & 1;

	if (--curBits == 0) {
		lzexeBuf[0] = exeStream->readByte();
		lzexeBuf[1] = exeStream->readByte();
		curBits = 16;
	} else {
		// Shift buffer to the right by 1 bit
		uint16 curBuffer = (lzexeBuf[0] << 8) | lzexeBuf[1];
		curBuffer >>= 1;
		lzexeBuf[0] = (curBuffer >> 8) & 0xFF;
		lzexeBuf[1] = curBuffer & 0xFF;
	}

	return result;
}

void readLzexe(Common::SeekableReadStream *exeStream, byte *buffer) {
	// TODO: finish this (from lzexe_read)
	printf("TODO: LZEXE support\n");
	return;
#if 0
	int done = 0;

	while (done != VERSION_DETECT_BUF_SIZE) {
		int size, copy, i;
		int left = count - done;

		if (!lzexe_decompress(exeStream))
			return done;

		/* Total amount of bytes in buffer. */
		//size = handle->bufptr - handle->buffer;

		// If we're not at end of data we need to maintain the window
		if (!exeStream->eos())
			copy = size - LZEXE_WINDOW;
		else {
			if (size == 0)
				/* No data left. */
				return done;

			copy = size;
		}

		/* Do not copy more than requested. */
		if (copy > left)
			copy = left;

		memcpy((char *) buffer + done, handle->buffer, copy);

		/* Move remaining data to start of buffer. */
		for (i = copy; i < size; i++)
			handle->buffer[i - copy] = handle->buffer[i];

		handle->bufptr -= copy;
		done += copy;
	}

	return done;
#endif
}

void readExe(Common::SeekableReadStream *exeStream, byte *buffer) {
	bool isLZEXE = isLZEXECompressed(exeStream);

	if (!isLZEXE) {
		// Read the last VERSION_DETECT_BUF_SIZE bytes
		exeStream->seek(exeStream->size() - VERSION_DETECT_BUF_SIZE, SEEK_SET);
		exeStream->read(buffer, VERSION_DETECT_BUF_SIZE);
	} else {
		// Read the two initial bytes
		lzexeBuf[0] = exeStream->readByte();
		lzexeBuf[1] = exeStream->readByte();

		curBits = 16;

		readLzexe(exeStream, buffer);
	}
}

bool readSciVersionFromExe(Common::SeekableReadStream *exeStream, int *version) {
	byte buffer[VERSION_DETECT_BUF_SIZE];
	char result_string[10]; /* string-encoded result, copied from buf */
	int state = 0;
	/* 'state' encodes how far we have matched the version pattern
	**   "n.nnn.nnn"
	**
	**   n.nnn.nnn
	**  0123456789
	**
	** Since we cannot be certain that the pattern does not begin with an
	** alphanumeric character, some states are ambiguous.
	** The pattern is expected to be terminated with a non-alphanumeric
	** character.
	*/

	readExe(exeStream, buffer);

	int accept;

	for (int i = 0; i < VERSION_DETECT_BUF_SIZE; i++) {
		const char ch = buffer[i];
		accept = 0; /* By default, we don't like this character */

		if (isalnum((unsigned char) ch)) {
			accept = (state != 1
			          && state != 5
			          && state != 9);
		} else if (ch == '.') {
			accept = (state == 1
			          || state == 5);
		} else if (state == 9) {
			result_string[9] = 0; /* terminate string */

			if (!version_parse(result_string, version)) {
				return true;	// success
			}

			// Continue searching
		}

		if (accept)
			result_string[state++] = ch;
		else
			state = 0;
	}

	return false; // failure
}

#if 0
//TODO
// Code from exe_lzexe.cpp (left for reference, to be converted)

static int
lzexe_decompress(exe_handle_t *handle) {
	while (!handle->eod
	        && handle->bufptr - handle->buffer <= LZEXE_BUFFER_MAX) {
		int bit;
		int len, span;

		if (!lzexe_get_bit(handle, &bit))
			return 0;

		if (bit) {
			/* 1: copy byte verbatim. */

			int data;

			if (!lzexe_read_uint8(handle->f, &data))
				return 0;

			*handle->bufptr++ = data;

			continue;
		}

		if (!lzexe_get_bit(handle, &bit))
			return 0;

		if (!bit) {
			/* 00: copy small block. */

			/* Next two bits indicate block length - 2. */
			if (!lzexe_get_bit(handle, &bit))
				return 0;

			len = bit << 1;

			if (!lzexe_get_bit(handle, &bit))
				return 0;

			len |= bit;
			len += 2;

			/* Read span byte. This forms the low byte of a
			** negative two's compliment value.
			*/
			if (!lzexe_read_uint8(handle->f, &span))
				return 0;

			/* Convert to negative integer. */
			span -= 256;
		} else {
			/* 01: copy large block. */
			int data;

			/* Read low byte of span value. */
			if (!lzexe_read_uint8(handle->f, &span))
				return 0;

			/* Read next byte. Bits [7..3] contain bits [12..8]
			** of span value. Bits [2..0] contain block length -
			** 2.
			*/
			if (!lzexe_read_uint8(handle->f, &data))
				return 0;
			span |= (data & 0xf8) << 5;
			/* Convert to negative integer. */
			span -= 8192;

			len = (data & 7) + 2;

			if (len == 2) {
				/* Next byte is block length value - 1. */
				if (!lzexe_read_uint8(handle->f, &len))
					return 0;

				if (len == 0) {
					/* End of data reached. */
					handle->eod = 1;
					break;
				}

				if (len == 1)
					/* Segment change marker. */
					continue;

				len++;
			}
		}

		assert(handle->bufptr + span >= handle->buffer);

		/* Copy block. */
		while (len-- > 0) {
			*handle->bufptr = *(handle->bufptr + span);
			handle->bufptr++;
		}
	}

	return 1;
}

static int
lzexe_read(exe_handle_t *handle, void *buf, int count) {
	int done = 0;

	while (done != count) {
		int size, copy, i;
		int left = count - done;

		if (!lzexe_decompress(handle))
			return done;

		/* Total amount of bytes in buffer. */
		size = handle->bufptr - handle->buffer;

		/* If we're not at end of data we need to maintain the
		** window.
		*/
		if (!handle->eod)
			copy = size - LZEXE_WINDOW;
		else {
			if (size == 0)
				/* No data left. */
				return done;

			copy = size;
		}

		/* Do not copy more than requested. */
		if (copy > left)
			copy = left;

		memcpy((char *) buf + done, handle->buffer, copy);

		/* Move remaining data to start of buffer. */
		for (i = copy; i < size; i++)
			handle->buffer[i - copy] = handle->buffer[i];

		handle->bufptr -= copy;
		done += copy;
	}

	return done;
}

#endif

//} // End of namespace Sci
