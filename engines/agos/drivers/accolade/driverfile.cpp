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

#include "audio/mididrv.h"
#include "common/error.h"
#include "common/file.h"

namespace AGOS {

// this reads and gets Accolade driver data
// we need it for channel mapping, instrument mapping and other things
// this driver data chunk gets passed to the actual music driver (MT32 / AdLib)
void MidiDriver_Accolade_readDriver(Common::String filename, MusicType requestedDriverType, byte *&driverData, uint16 &driverDataSize, bool &isMusicDrvFile) {
	Common::File *driverStream = new Common::File();

	isMusicDrvFile = false;

	if (!driverStream->open(filename)) {
		error("%s: unable to open file", filename.c_str());
	}

	if (filename == "INSTR.DAT") {
		// INSTR.DAT: used by Elvira 1
		uint32 streamSize = driverStream->size();
		uint32 streamLeft = streamSize;
		uint16 skipChunks = 0; // 1 for MT32, 0 for AdLib
		uint16 chunkSize  = 0;

		switch (requestedDriverType) {
		case MT_ADLIB:
			skipChunks = 0;
			break;
		case MT_MT32:
			skipChunks = 1; // Skip one entry for MT32
			break;
		default:
			assert(0);
			break;
		}

		do {
			if (streamLeft < 2)
				error("%s: unexpected EOF", filename.c_str());

			chunkSize = driverStream->readUint16LE();
			streamLeft -= 2;

			if (streamLeft < chunkSize)
				error("%s: unexpected EOF", filename.c_str());

			if (skipChunks) {
				// Skip the chunk
				driverStream->skip(chunkSize);
				streamLeft -= chunkSize;

				skipChunks--;
			}
		} while (skipChunks);

		// Seek over the ASCII string until there is a NUL terminator
		byte curByte = 0;

		do {
			if (chunkSize == 0)
				error("%s: no actual instrument data found", filename.c_str());

			curByte = driverStream->readByte();
			chunkSize--;
		} while (curByte);

		driverDataSize = chunkSize;

		// Read the requested instrument data entry
		driverData = new byte[driverDataSize];
		driverStream->read(driverData, driverDataSize);

	} else if (filename == "MUSIC.DRV") {
		// MUSIC.DRV / used by Elvira 2 / Waxworks / Simon 1 demo
		uint32 streamSize = driverStream->size();
		uint32 streamLeft = streamSize;
		uint16 getChunk   = 0; // 4 for MT32, 2 for AdLib

		switch (requestedDriverType) {
		case MT_ADLIB:
			getChunk = 2;
			break;
		case MT_MT32:
			getChunk = 4;
			break;
		default:
			assert(0);
			break;
		}

		if (streamLeft < 2)
			error("%s: unexpected EOF", filename.c_str());

		uint16 chunkCount = driverStream->readUint16LE();
		streamLeft -= 2;

		if (getChunk >= chunkCount)
			error("%s: required chunk not available", filename.c_str());

		uint16 headerOffset = 2 + (28 * getChunk);
		streamLeft -= (28 * getChunk);

		if (streamLeft < 28)
			error("%s: unexpected EOF", filename.c_str());

		// Seek to required chunk
		driverStream->seek(headerOffset);
		driverStream->skip(20); // skip over name
		streamLeft -= 20;

		uint16 musicDrvSignature = driverStream->readUint16LE();
		uint16 musicDrvType = driverStream->readUint16LE();
		uint16 chunkOffset = driverStream->readUint16LE();
		uint16 chunkSize   = driverStream->readUint16LE();

		// Security checks
		if (musicDrvSignature != 0xFEDC)
			error("%s: chunk signature mismatch", filename.c_str());
		if (musicDrvType != 1)
			error("%s: not a music driver", filename.c_str());
		if (chunkOffset >= streamSize)
			error("%s: driver chunk points outside of file", filename.c_str());

		streamLeft = streamSize - chunkOffset;
		if (streamLeft < chunkSize)
			error("%s: driver chunk is larger than file", filename.c_str());

		driverDataSize = chunkSize;

		// Read the requested instrument data entry
		driverData = new byte[driverDataSize];

		driverStream->seek(chunkOffset);
		driverStream->read(driverData, driverDataSize);
		isMusicDrvFile = true;
	}

	driverStream->close();
	delete driverStream;
}

} // End of namespace AGOS
