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

#include "sci/sci.h"
#include "sci/exereader.h"

namespace Sci {

int _bitCount;
uint16 _bits;

Common::Platform getGameExePlatform(Common::SeekableReadStream *exeStream) {
	byte magic[4];
	// Make sure that the executable is at least 4KB big
	if (exeStream->size() < 4096)
		return Common::kPlatformUnknown;

	// Read exe header
	exeStream->read(magic, 4);

	// Check if the header contains known magic bytes

	// Information obtained from http://magicdb.org/magic.db
	// Check if it's a DOS executable
	if (magic[0] == 'M' && magic[1] == 'Z') {
		return Common::kPlatformPC;
	}

	// Check if it's an Amiga executable
	if ((magic[2] == 0x03 && magic[3] == 0xF3) ||
		(magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')) {
		return Common::kPlatformAmiga;
	}

	// Check if it's an Atari executable
	if ((magic[0] == 0x60 && magic[1] == 0x1A))
		return Common::kPlatformAtariST;

	// Check if it's a Mac exe

	// Resource map offset
	int32 offset = (int32)READ_BE_UINT32(magic);
	offset += 28;
	if (exeStream->size() <= offset)
		return Common::kPlatformUnknown;

	// Skip number of types in map
	exeStream->skip(2);
//	uint16 val = exeStream->readUint16BE() + 1;
	exeStream->skip(2);

	// Keep reading till we find the "CODE" bit
	while (!exeStream->eos()) {
		exeStream->skip(4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;

		exeStream->read(magic, 4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;

		if (!memcmp(magic, "CODE", 4)) {
			return Common::kPlatformMacintosh;
		}
		// Skip to the next list entry
		exeStream->skip(4);
		if (exeStream->eos())
			return Common::kPlatformUnknown;
	}

	// If we've reached here, the file type is unknown
	return Common::kPlatformUnknown;
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
	filepos <<= 4;

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

uint getBit(Common::SeekableReadStream *input) {
	uint bit = _bits & 1;
	_bitCount--;

	if (_bitCount <= 0) {
		_bits = input->readByte();
		_bits |= input->readByte() << 8;

		if (_bitCount == -1) { // special case for first bit word
			bit = _bits & 1;
			_bits >>= 1;
		}

		_bitCount += 16;
	} else
		_bits >>= 1;

	return bit;
}

Common::String readSciVersionFromExe(Common::SeekableReadStream *exeStream, Common::Platform platform) {
	int len = exeStream->size();
	unsigned char *buffer = NULL;

	// Read the executable
	bool isLZEXE = isLZEXECompressed(exeStream);

	if (!isLZEXE) {
		buffer = new unsigned char[exeStream->size()];

		exeStream->seek(0, SEEK_SET);
		exeStream->read(buffer, exeStream->size());
	} else {
		buffer = new unsigned char[exeStream->size() * 3];
		_bitCount = 0;

		// Skip LZEXE header
		exeStream->seek(32, SEEK_SET);

		int pos = 0;
		int repeat;
		short offset;

		while (1) {
			if (exeStream->ioFailed()) {
				warning("Error reading from input file");
				delete[] buffer;
				return NULL;
			}

			if (getBit(exeStream)) {
				buffer[pos++] = exeStream->readByte();
			} else {
				if (getBit(exeStream)) {
					byte tmp[2];
					exeStream->read(tmp, 2);
					repeat = (tmp[1] & 0x07);
					offset = ((tmp[1] & ~0x07) << 5) | tmp[0] | 0xE000;

					if (repeat == 0) {
						repeat = exeStream->readByte();

						if (repeat == 0) {
							len = pos;
							break;
						}
						else if (repeat == 1)
							continue;
						else
							repeat++;
					} else
						repeat += 2;
				} else {
					repeat = getBit(exeStream) << 1;
					repeat += getBit(exeStream) + 2;
					offset = exeStream->readByte() | 0xFF00;
				}

				while (repeat > 0) {
					buffer[pos] = buffer[pos + offset];
					pos++;
					repeat--;
				}
			}
		}
	}

	// Find SCI version number

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


	int accept;
	unsigned char *buf = buffer;

	// String-encoded result, copied from buffer
	char currentString[10];
	Common::String resultString;

	for (int i = 0; i < len; i++) {
		unsigned char ch = *buf++;
		// By default, we don't like this character
		accept = 0;

		if (isalnum(ch)) {
			accept = (state != 1 && state != 5 && state != 9);
		} else if (ch == '.') {
			accept = (state == 1 || state == 5);
		} else if (state == 9) {
			// Terminate string
			currentString[9] = 0;

			// Return the current string if it's parseable
			SciVersion version;
			if (getSciVersionFromString(currentString, &version, platform)) {
				delete[] buffer;
				return currentString;
			}

			// Save the found string and continue searching
			resultString = currentString;
		}

		if (accept)
			currentString[state++] = ch;
		else
			state = 0;
	}

	delete[] buffer;
	return resultString;
}

bool getSciVersionFromString(Common::String versionString, SciVersion *version, Common::Platform platform) {
	*version = SCI_VERSION_AUTODETECT;

	if (platform == Common::kPlatformAmiga) {
		if (versionString.hasPrefix("1.002.")) {
			*version = SCI_VERSION_0;
		} else if (versionString.hasPrefix("1.003.")) {
			*version = SCI_VERSION_01;
		} else if (versionString.hasPrefix("1.004.")) {
			*version = SCI_VERSION_01;
		} else if (versionString.hasPrefix("1.005.")) {
			*version = SCI_VERSION_1;
		} else if (versionString == "x.yyy.zzz") {
			// How to map it?
		} else {
			return false;
		}
	} else if (versionString.hasPrefix("0.000.")) {
		*version = SCI_VERSION_0;
	} else if (versionString.hasPrefix("S.old.")) {
		*version = SCI_VERSION_01;
	} else if (versionString.hasPrefix("1.000.")) {
		*version = SCI_VERSION_1;
	} else if (versionString.hasPrefix("1.001.")) {
		*version = SCI_VERSION_1_1;
	} else if (versionString.hasPrefix("2.000.")
		|| versionString.hasPrefix("2.100.")
		|| versionString.hasPrefix("3.000.")) {
		*version = SCI_VERSION_32;
	} else if (versionString.hasPrefix("1.ECO.")
		|| versionString.hasPrefix("1.SQ1.")
		|| versionString.hasPrefix("1.SQ4.")
		|| versionString.hasPrefix("1.LS5.")
		|| versionString.hasPrefix("1.pq3.")
		|| versionString.hasPrefix("FAIRY.")
		|| versionString.hasPrefix("T.A00.")) {
		*version = SCI_VERSION_1;
	} else if (versionString.hasPrefix("L.rry.")
		|| versionString.hasPrefix("l.cfs.")) {
		*version = SCI_VERSION_1_1;
	} else if (versionString == "x.yyy.zzz") {
		// How to map it?
	} else {
		// Unknown or not a version number
		return false;
	}

	return true;
}

} // End of namespace Sci
