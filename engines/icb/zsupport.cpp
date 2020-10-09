/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

#include "engines/icb/common/px_common.h"
#include "engines/icb/p4.h"
#include "engines/icb/zsupport.h"
#include "engines/icb/res_man_pc.h"

namespace ICB {

#define BUFLEN 65536

//static char buf[BUFLEN];

uint32 fileGetZipLength(const char *inFn) {
	Common::SeekableReadStream *fileStream = openDiskFileForBinaryStreamRead(inFn);
	uint len = fileGetZipLength2(fileStream);
	delete fileStream;
	return len;
}

uint32 fileGetZipLength2(Common::SeekableReadStream *fileStream) {
	error("TODO: Fix compression");
#if 0
	gzFile in;
	uint32 t = 0;

	int noOfFile = fileno(file);
	in = gzdopen(noOfFile , "rb");

	if (in == NULL)
		return 0;

	gzread(in, &t, sizeof(int32));

	return t;
#endif
}

unsigned int memUncompress(unsigned char *outMem, const char *inFn) {
	Common::SeekableReadStream *fileStream = openDiskFileForBinaryStreamRead(inFn);

	uint retVal = memUncompress(outMem, inFn, fileStream);

	delete fileStream;

	return retVal;
}

unsigned int memUncompress(unsigned char *outMem, const char *inFn, Common::SeekableReadStream *file) {
	error("TODO: Fix compression");
#if 0
	gzFile in;
	unsigned char *myp;
	uint uncompressedLen;


	int noOfFile = fileno(file);
	in = gzdopen(noOfFile , "rb");

	if (in == NULL) {
		fprintf(stderr, "error opening zipped file %s\n", inFn);
		return 0;
	}
	// Read the length
	if (gzread(in, &uncompressedLen, 4) != 4) {
		fprintf(stderr, "error in zipped file %s\n", inFn);
		return 0;
	}

	uint bytesRead = 0;
	uint bytesToRead = uncompressedLen;

	myp = outMem;

	while (bytesToRead) {
		// Work out how much to read this go
		uint bytesToReadThisPass = bytesToRead;
		// Only fill a buffer
		if (bytesToReadThisPass > BUFLEN)
			bytesToReadThisPass = BUFLEN;

		// Read in those bytes
		uint bytesReadThisPass = gzread(in, buf, bytesToReadThisPass);
		if (bytesReadThisPass == 0)
			break;

		// Uypdate the counters and pointers
		bytesRead += bytesReadThisPass;
		bytesToRead -= bytesReadThisPass;

		// And copy the data over
		memcpy(myp, buf, bytesReadThisPass);
		myp += bytesReadThisPass;
	}
#endif
	return 1;
}

} // End of namespace ICB
