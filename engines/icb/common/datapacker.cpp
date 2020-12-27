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

#include "engines/icb/common/datapacker.h"

#include "common/stream.h"
#include "common/textconsole.h"

namespace ICB {

// Just initialise the buffer position and the min, max values
// used for packing and error detection
DataPacker::DataPacker() {
	readMode = false;
	iMode = NO_MODE;
	iPackMode = NO_PACKMODE;
	pos = 0;
	packMin = -(1 << (PACK_BIT_SIZE - 1));
	packMax = +((1 << PACK_BIT_SIZE) - 1);

	// clear out the data buffer
	ClearBuffer();
}

// Could do something here - but not sure what !
// Could error check on pos = 0 : but might lead to trouble
DataPacker::~DataPacker() {}

// Start the bit-packing process : say if we are in READ or WRITE mode
DataPacker::ReturnCodes DataPacker::open(const ModeEnum mode, const PackModeEnum packMode) {
	if (pos != 0) {
		return BAD_POS;
	}

	if ((mode != READ) && (mode != WRITE)) {
		return BAD_MODE;
	}

	if ((packMode != PACK) && (packMode != DONT_PACK)) {
		return BAD_PACKMODE;
	}

	if (mode == READ) {
		readMode = true;
		pos = PACK_CHUNK_SIZE;
	}

	if (mode == WRITE) {
		readMode = false;
		pos = 0;
	}

	// clear out the data buffer
	ClearBuffer();

	iMode = mode;
	iPackMode = packMode;

	return OK;
}

// Put a value into the bit-stream
DataPacker::ReturnCodes DataPacker::put(const int value, Common::WriteStream *stream) {
	if (iMode != WRITE) {
		return BAD_MODE;
	}

	if ((iPackMode != PACK) && (iPackMode != DONT_PACK)) {
		return BAD_PACKMODE;
	}

	if ((pos < 0) || (pos >= PACK_CHUNK_SIZE)) {
		return BAD_POS;
	}

	// For DONT_PACK mode just write the data straight out
	if (iPackMode == DONT_PACK) {
		// Check it is a legal value : 16-bits
		int lvarMin = -(1 << 15);
		int lvarMax = +((1 << 15) - 1);
		if ((value < lvarMin) || (value > lvarMax)) {
			return BAD_VALUE;
		}

		int nItems = 2;
		short v16 = (short)value;
		int ret = stream->write((const void *)&v16, nItems);

		if (ret != nItems) {
			return WRITE_ERROR;
		}
		return OK;
	}

	// Convert the value to be within limits
	int v = value - packMin;

	// Check the value is within range
	if ((v < 0) || (v > packMax)) {
		return BAD_VALUE;
	}

	// Add the value in
	if (pos == 0) {
		buffer[0] = (unsigned char)((v >> 6) & 0xFF); // v's top 8-bits
		buffer[1] = (unsigned char)((v & 0x3F) << 2); // v's bottom 6-bits into top 6-bits
	} else if (pos == 1) {
		buffer[1] |= ((v >> 12) & 0x03);              // v's top 2-bits into bottom 2
		buffer[2] = (unsigned char)((v >> 4) & 0xFF); // v's middle 8-bits
		buffer[3] = (unsigned char)((v & 0x0F) << 4); // v's bottom 4-bits into top 4
	} else if (pos == 2) {
		buffer[3] |= ((v >> 10) & 0x0F);              // v's top 4-bits into bottom 4
		buffer[4] = (unsigned char)((v >> 2) & 0xFF); // v's middle 8-bits
		buffer[5] = (unsigned char)((v & 0x03) << 6); // v's bottom 2-bits into top 2
	} else if (pos == 3) {
		buffer[5] |= ((v >> 8) & 0x3F);        // v's top 6-bits into bottom 6
		buffer[6] = (unsigned char)(v & 0xFF); // v's bottom 8-bits
	}
	// Put data into the next position !
	pos++;

	// Do we need to output the current buffer ?
	if (pos == PACK_CHUNK_SIZE) {
#if 0
		printf("WRITE %X %X %X %X %X %X %X",
		       buffer[0], buffer[1], buffer[2], buffer[3],
		       buffer[4], buffer[5], buffer[6]);
#endif // #if 0
		// Write out the buffer
		int nItems = BUFFER_BYTE_SIZE;
		int ret = stream->write((const void *)buffer, nItems);

		if (ret != nItems) {
			return WRITE_ERROR;
		}

		pos = 0;
		ClearBuffer();
	}
	return OK;
}

// Get a value from the bit-stream
DataPacker::ReturnCodes DataPacker::Get(int32 &value, Common::SeekableReadStream *stream) {
	if (iMode != READ) {
		return BAD_MODE;
	}

	if ((iPackMode != PACK) && (iPackMode != DONT_PACK)) {
		return BAD_PACKMODE;
	}

	if ((pos < 0) || (pos > PACK_CHUNK_SIZE)) {
		return BAD_POS;
	}

	// For DONT_PACK mode just read the data straight in
	if (iPackMode == DONT_PACK) {
		int nItems = 2;
		short int v16;
		int ret = stream->read((void *)&v16, nItems);
		value = v16;

		if (ret != nItems) {
			return READ_ERROR;
		}
		return OK;
	}

	// Do we need to fill up the current buffer ?
	if (pos == PACK_CHUNK_SIZE) {
		// Read into the buffer
		int nItems = BUFFER_BYTE_SIZE;
		int ret = stream->read((void *)buffer, nItems);

		if (ret != nItems) {
			return READ_ERROR;
		}

#if 0
		printf("READ %X %X %X %X %X %X %X",
		       buffer[0], buffer[1], buffer[2], buffer[3],
		       buffer[4], buffer[5], buffer[6]);
#endif
		pos = 0;
	}

	int32 v = 0;
	// Get the value out of the buffer
	if (pos == 0) {
		v = (buffer[0] << 6);  // v's top 8-bits
		v |= (buffer[1] >> 2); // v's bottom 6-bits into top 6-bits
	} else if (pos == 1) {
		v = ((buffer[1] & 0x03) << 12); // v's top 2-bits into bottom 2
		v |= (buffer[2] << 4);          // v's middle 8-bits
		v |= (buffer[3] >> 4);          // v's bottom 4-bits into top 4
	} else if (pos == 2) {
		v = ((buffer[3] & 0x0F) << 10); // v's top 4-bits into bottom 4
		v |= (buffer[4] << 2);          // v's middle 8-bits
		v |= (buffer[5] >> 6);          // v's bottom 2-bits into top 2
	} else if (pos == 3) {
		v = (buffer[5] & 0x3F) << 8; // v's top 6-bits into bottom 6
		v |= buffer[6];              // v's bottom 8-bits
	}
	// Get data from the next position !
	pos++;

	// Check the value is within range
	// This should just not be possible !
	if ((v < 0) || (v > packMax)) {
		return BAD_VALUE;
	}

	// Convert the extracted value to be in normal signed/unsigned limits
	value = v + packMin;

	return OK;
}

// Stop the bit-packing process : will output any remaining data
DataPacker::ReturnCodes DataPacker::close(Common::WriteStream *stream) {
	if ((iMode == WRITE) && (pos != 0)) {
		// Write out the remaining data items
		int nItems = BUFFER_BYTE_SIZE;
		int ret = stream->write((const void *)buffer, nItems);

		if (ret != nItems) {
			return WRITE_ERROR;
		}
	} else {
		error("Wrong close-function called, passed WriteStream without being in WRITE-mode");
	}

	iMode = NO_MODE;
	iPackMode = NO_PACKMODE;

	pos = 0;

	ClearBuffer();

	return OK;
}

DataPacker::ReturnCodes DataPacker::close(Common::SeekableReadStream *stream) {
	// TODO: If write mode.
	if ((iMode == WRITE) && (pos != 0)) {
		error("Wrong close-function called, passed ReadStream in WRITE-mode");
	}
	iMode = NO_MODE;
	iPackMode = NO_PACKMODE;

	pos = 0;

	ClearBuffer();

	return OK;
}

// Copy constructor
DataPacker::DataPacker(DataPacker &src) { *this = src; }

// Assignment operator
DataPacker &DataPacker::operator=(DataPacker &b) {
	readMode = b.readMode;

	iMode = b.iMode;
	iPackMode = b.iPackMode;
	pos = b.pos;
	packMin = b.packMin;
	packMax = b.packMax;

	for (int i = 0; i < BUFFER_BYTE_SIZE; i++) {
		buffer[i] = b.buffer[i];
	}

	return *this;
}

// Clear out the data buffer
void DataPacker::ClearBuffer() {
	for (int i = 0; i < BUFFER_BYTE_SIZE; i++) {
		buffer[i] = 0x00;
	}
}

} // End of namespace ICB
