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
 * This is a utility to unpack Delphine's Cinematique engine's archive files.
 * Should at least work with Future Wars and Operation Stealth.
 * Supports using Operation Stealth's 'vol.cnf' file as input for selecting
 * which archive files to unpack.
 *
 * Note that this isn't polished code so caveat emptor.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "cine_unpack.h"
#include "typedefs.h"
#include "endian.h"

////////////////////////////////////////////////////////////////////////////

uint32 CineUnpacker::readSource() {
	if (_src < _srcBegin || _src + 4 > _srcEnd) {
		_error = true;
		return 0; // The source pointer is out of bounds, returning a default value
	}
	uint32 value = READ_BE_UINT32(_src);
	_src -= 4;
	return value;
}

uint CineUnpacker::rcr(bool inputCarry) {
	uint outputCarry = (_chunk32b & 1);
	_chunk32b >>= 1;
	if (inputCarry) {
		_chunk32b |= 0x80000000;
	}
	return outputCarry;
}

uint CineUnpacker::nextBit() {
	uint carry = rcr(false);
	// Normally if the chunk becomes zero then the carry is one as
	// the end of chunk marker is always the last to be shifted out.
	if (_chunk32b == 0) {
		_chunk32b = readSource();
		_crc ^= _chunk32b;
		carry = rcr(true); // Put the end of chunk marker in the most significant bit
	}
	return carry;
}

uint CineUnpacker::getBits(uint numBits) {
	uint c = 0;
	while (numBits--) {
		c <<= 1;
		c |= nextBit();
	}
	return c;
}

void CineUnpacker::unpackRawBytes(uint numBytes) {
	if (_dst >= _dstEnd || _dst - numBytes + 1 < _dstBegin) {
		_error = true;
		return; // Destination pointer is out of bounds for this operation
	}
	while (numBytes--) {
		*_dst = (byte)getBits(8);
		--_dst;
	}
}

void CineUnpacker::copyRelocatedBytes(uint offset, uint numBytes) {
	if (_dst + offset >= _dstEnd || _dst - numBytes + 1 < _dstBegin) {
		_error = true;
		return; // Destination pointer is out of bounds for this operation
	}
	while (numBytes--) {
		*_dst = *(_dst + offset);
		--_dst;
	}
}

bool CineUnpacker::unpack(const byte *src, uint srcLen, byte *dst, uint dstLen) {
	// Initialize variables used for detecting errors during unpacking
	_error    = false;
	_srcBegin = src;
	_srcEnd   = src + srcLen;
	_dstBegin = dst;
	_dstEnd   = dst + dstLen;

	// Initialize other variables
	_src = _srcBegin + srcLen - 4;
	uint32 unpackedLength = readSource(); // Unpacked length in bytes
	_dst = _dstBegin + unpackedLength - 1;
	_crc = readSource();
	_chunk32b = readSource();
	_crc ^= _chunk32b;

	while (_dst >= _dstBegin && !_error) {
		/*
		Bits  => Action:
		0 0   => unpackRawBytes(3 bits + 1)              i.e. unpackRawBytes(1..8)
		1 1 1 => unpackRawBytes(8 bits + 9)              i.e. unpackRawBytes(9..264)
		0 1   => copyRelocatedBytes(8 bits, 2)           i.e. copyRelocatedBytes(0..255, 2)
		1 0 0 => copyRelocatedBytes(9 bits, 3)           i.e. copyRelocatedBytes(0..511, 3)
		1 0 1 => copyRelocatedBytes(10 bits, 4)          i.e. copyRelocatedBytes(0..1023, 4)
		1 1 0 => copyRelocatedBytes(12 bits, 8 bits + 1) i.e. copyRelocatedBytes(0..4095, 1..256)
		*/
		if (!nextBit()) { // 0...
			if (!nextBit()) { // 0 0
				uint numBytes = getBits(3) + 1;
				unpackRawBytes(numBytes);
			} else { // 0 1
				uint numBytes = 2;
				uint offset   = getBits(8);
				copyRelocatedBytes(offset, numBytes);
			}
		} else { // 1...
			uint c = getBits(2);
			if (c == 3) { // 1 1 1
				uint numBytes = getBits(8) + 9;
				unpackRawBytes(numBytes);
			} else if (c < 2) { // 1 0 x
				uint numBytes = c + 3;
				uint offset   = getBits(c + 9);
				copyRelocatedBytes(offset, numBytes);
			} else { // 1 1 0
				uint numBytes = getBits(8) + 1;
				uint offset   = getBits(12);
				copyRelocatedBytes(offset, numBytes);
			}
		}
	}
	return !_error && (_crc == 0);
}

////////////////////////////////////////////////////////////////////////////

static void unpackFile(FILE *fp, const char *outDir) {
	char filePath[512], fileName[15];

	uint entryCount = freadUint16BE(fp); // How many entries?
	uint entrySize = freadUint16BE(fp); // How many bytes per entry?
	assert(entrySize == 0x1e);
	while (entryCount--) {
		fread(fileName, 14, 1, fp);
		fileName[14] = '\0';
		sprintf(filePath, "%s/%s", outDir, fileName);
		FILE *fpOut = fopen(filePath, "wb");
		
		uint32 offset = freadUint32BE(fp);
		unsigned int packedSize = freadUint32BE(fp);
		unsigned int unpackedSize = freadUint32BE(fp);
		freadUint32BE(fp);
		uint savedPos = ftell(fp);
		
		if (!fpOut) {
			printf("ERROR: unable to open '%s' for writing\n", filePath);
			continue;
		}
		printf("unpacking '%s' ... ", filePath);		
		
		fseek(fp, offset, SEEK_SET);		
		assert(unpackedSize >= packedSize);
		uint8 *data = (uint8 *)calloc(unpackedSize, 1);
		uint8 *packedData = (uint8 *)calloc(packedSize, 1);
		assert(data);
		assert(packedData);
		fread(packedData, packedSize, 1, fp);
		bool status = true;
		if (packedSize != unpackedSize) {
			CineUnpacker cineUnpacker;
			status = cineUnpacker.unpack(packedData, packedSize, data, unpackedSize);
		} else {
			memcpy(data, packedData, packedSize);
		}
		free(packedData);
		fwrite(data, unpackedSize, 1, fpOut);
		fclose(fpOut);
		free(data);
		
		if (!status) {
			printf("CRC ERROR");
		} else {
			printf("ok");
		}
		printf(", packedSize %u unpackedSize %u\n", packedSize, unpackedSize);
		fseek(fp, savedPos, SEEK_SET);
	}
}

void fixVolCnfFileName(char *dst, const uint8 *src) {
	char *ext, *end;
	
	memcpy(dst, src, 8);
	src += 8;
	dst[8] = 0;	
	ext = strchr(dst, ' ');	
	if (!ext) {
		ext = &dst[8];
	}
	if (*src == ' ') {
		*ext = 0;
	} else {
		*ext++ = '.';
		memcpy(ext, src, 3);
		end = strchr(ext, ' ');
		if (!end) {
			end = &ext[3];
		}
		*end = 0;
	}
}

void unpackAllResourceFiles(const char *filename, const char *outDir) { 
	FILE *fp = fopen(filename, "rb");
	if (!fp) {
		printf("Unable to open file '%s'\n", filename);
		exit(1);
	}
	
	uint32 unpackedSize, packedSize;
	{
		char header[8];
		fread(header, 8, 1, fp);
		if (memcmp(header, "ABASECP", 7) == 0) {
			unpackedSize = freadUint32BE(fp);
			packedSize = freadUint32BE(fp);
		} else {
			fseek(fp, 0, SEEK_END);
			unpackedSize = packedSize = ftell(fp); /* Get file size */
			fseek(fp, 0, SEEK_SET);
		}
	}
	assert(unpackedSize >= packedSize);
	uint8 *buf = (uint8 *)calloc(unpackedSize, 1);
	assert(buf);
	fread(buf, packedSize, 1, fp);
	fclose(fp);
	if (packedSize != unpackedSize) {
		CineUnpacker cineUnpacker;
		if (!cineUnpacker.unpack(buf, packedSize, buf, unpackedSize)) {
			printf("Error while unpacking 'vol.cnf' data");
			exit(1);
		}
	}
	uint resourceFilesCount = READ_BE_UINT16(&buf[0]);
	uint entrySize = READ_BE_UINT16(&buf[2]);
	printf("--- Unpacking all %d resource files from 'vol.cnf' (entrySize = %d):\n", resourceFilesCount, entrySize);
	char resourceFileName[9];
	for (unsigned int i = 0; i < resourceFilesCount; ++i) {
		memcpy(resourceFileName, &buf[4 + i * entrySize], 8);
		resourceFileName[8] = 0;
		FILE *fpResFile = fopen(resourceFileName, "rb");
		if (fpResFile) {
			printf("--- Unpacking resource file %s:\n", resourceFileName);
			unpackFile(fpResFile, outDir);
			fclose(fpResFile);
		} else {
			printf("ERROR: Unable to open resource file %s\n", resourceFileName);
		}
	}

	free(buf);
}

int showUsage() {
	printf("USAGE: cine_unpack [input file] [output directory]\n" \
		"Supports using Operation Stealth's 'vol.cnf' file as input.\n");
	return -1;
}

int main(int argc, char *argv[]) {
	int i;
	char tmp[512];
	
	if (argc == 3) {
		strcpy(tmp, argv[1]);
		for (i = 0; tmp[i] != 0; i++) {
			tmp[i] = toupper(tmp[i]);
		}
		if (!strcmp(tmp, "VOL.CNF")) {
			/* Unpack all archive files listed in 'vol.cnf' */
			unpackAllResourceFiles(argv[1], argv[2]);
		} else {
			/* Unpack a single archive file */
			FILE *fp = fopen(argv[1], "rb");
			if (fp) {
				unpackFile(fp, argv[2]);
				fclose(fp);
			} else {
				printf("Couldn't open input file '%s'\n", argv[1]);
				return -1;
			}
		}
	} else {
		return showUsage();
	}
	return 0;
}
