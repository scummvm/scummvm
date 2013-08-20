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

#include "common/str.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/debug.h"
#include "common/endian.h"

#include "zvision/rlf_animation.h"


namespace ZVision {

RlfAnimation::RlfAnimation(const Common::String &fileName) 
		: _frames(0) {
	Common::File file;
	if (!file.open(fileName)) {
		warning("RLF animation file %s could not be opened", fileName.c_str());
		return;
	}

	if (file.readUint32BE() != MKTAG('F', 'E', 'L', 'R')) {
		warning("%s is not a RLF animation file. Wrong magic number", fileName.c_str());
		return;
	}

	// Read the header
	file.readUint32LE();				// Size1
	file.readUint32LE();				// Unknown1
	file.readUint32LE();				// Unknown2
	_frameCount = file.readUint32LE();	// Frame count

	// Since we don't need any of the data, we can just seek right to the
	// entries we need rather than read in all the individual entries.
	file.seek(136, SEEK_CUR);

	//// Read CIN header
	//file.readUint32BE();			// Magic number FNIC
	//file.readUint32LE();			// Size2
	//file.readUint32LE();			// Unknown3
	//file.readUint32LE();			// Unknown4
	//file.readUint32LE();			// Unknown5
	//file.seek(0x18, SEEK_CUR);	// VRLE
	//file.readUint32LE();			// LRVD
	//file.readUint32LE();			// Unknown6
	//file.seek(0x18, SEEK_CUR);	// HRLE
	//file.readUint32LE();			// ELHD
	//file.readUint32LE();			// Unknown7
	//file.seek(0x18, SEEK_CUR);	// HKEY
	//file.readUint32LE();			// ELRH

	//// Read MIN info header
	//file.readUint32BE();			// Magic number FNIM
	//file.readUint32LE();			// Size3
	//file.readUint32LE();			// OEDV
	//file.readUint32LE();			// Unknown8
	//file.readUint32LE();			// Unknown9
	//file.readUint32LE();			// Unknown10
	_width = file.readUint32LE();	// Width
	_height = file.readUint32LE();	// Height
	
	// Read time header
	file.readUint32BE(); // Magic number EMIT
	file.readUint32LE(); // Size4
	file.readUint32LE(); // Unknown11
	_frameTime = file.readUint32LE() / 10; // Frame time in microseconds

	// Read in each frame
	_frames = new uint16 *[_frameCount];
	for (uint i = 0; i < _frameCount; i++) {
		file.readUint32BE();					// Magic number MARF
		uint32 size = file.readUint32LE();		// Size
		file.readUint32LE();					// Unknown1
		file.readUint32LE();					// Unknown2
		uint32 type = file.readUint32BE();		// Either ELHD or ELRH
		uint32 offset = file.readUint32LE();	// Offset from the beginning of this frame to the frame data. Should always be 28
		file.readUint32LE();					// Unknown3

		int8 *buffer = new int8[size - headerSize];
		file.read(buffer, size - headerSize);

		_frames[i] = new uint16[_width * _height];
		uint frameByteSize = _width * _height * sizeof(uint16);
		memset(_frames[i], 0x7C00, frameByteSize);
		// Decode the data
		debug("Decoding frame %u", i);
		if (type == MKTAG('E', 'L', 'H', 'D')) {
			debug("Decoding with masked RLE");
			decodeMaskedRunLengthEncoding(buffer, (int8 *)_frames[i], size - headerSize, frameByteSize);
		} else if (type == MKTAG('E', 'L', 'R', 'H')) {
			debug("Decoding with simple RLE");
			decodeSimpleRunLengthEncoding(buffer, (int8 *)_frames[i], size - headerSize, frameByteSize);
		} else {
			warning("Frame %u of %s doesn't have type that can be decoded", i, fileName.c_str());
			return;
		}

		// Cleanup
		delete[] buffer;
	}
};

RlfAnimation::~RlfAnimation() {
	if (_frames != 0) {
		delete[] _frames;
	}
}



void RlfAnimation::decodeMaskedRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfSamples = abs(numberOfSamples);

			while (numberOfSamples > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, READ_LE_UINT16(source + sourceOffset));

				sourceOffset += 2;
				destOffset += 2;
				numberOfSamples--;
			}

		// If numberOfSamples is >= 0, move destOffset forward ((numberOfSamples * 2) + 2)
		// This function assumes the dest buffer has been memset with 0's.
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			} else if (destOffset + 1 >= destSize) {
				warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
				return;
			}

			destOffset += (numberOfSamples * 2) + 2;
		}
	}
}

void RlfAnimation::decodeSimpleRunLengthEncoding(int8 *source, int8 *dest, uint32 sourceSize, uint32 destSize) const {
	uint32 sourceOffset = 0;
	uint32 destOffset = 0;

	while (sourceOffset < sourceSize) {
		int8 numberOfSamples = source[sourceOffset];
		sourceOffset++;

		// If numberOfSamples is negative, the next abs(numberOfSamples) samples should
		// be copied directly from source to dest
		if (numberOfSamples < 0) {
			numberOfSamples = abs(numberOfSamples);

			while (numberOfSamples > 0) {
				if (sourceOffset + 1 >= sourceSize) {
					return;
				} else if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, READ_LE_UINT16(source + sourceOffset));

				sourceOffset += 2;
				destOffset += 2;
				numberOfSamples--;
			}

		// If numberOfSamples is >= 0, copy one sample from source to the 
		// next (numberOfSamples + 2) dest spots
		} else {
			if (sourceOffset + 1 >= sourceSize) {
				return;
			}

			uint16 sampleColor = READ_LE_UINT16(source + sourceOffset);
			sourceOffset += 2;

			numberOfSamples += 2;
			while (numberOfSamples > 0) {
				if (destOffset + 1 >= destSize) {
					warning("Frame decoding overflow\n\tsourceOffset=%u\tsourceSize=%u\n\tdestOffset=%u\tdestSize=%u", sourceOffset, sourceSize, destOffset, destSize);
					return;
				}

				WRITE_UINT16(dest + destOffset, sampleColor);
				destOffset += 2;
				numberOfSamples--;
			}
		}
	}
}


} // End of namespace ZVision
