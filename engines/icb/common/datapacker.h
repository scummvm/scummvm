/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ICB_DATAPACKER_HH
#define ICB_DATAPACKER_HH

#include "common/stream.h"

namespace ICB {

// Pack: 4 values into 4*14 bits = 56-bits = 7 bytes
#define BUFFER_BYTE_SIZE (7)
#define PACK_BIT_SIZE (14)
#define PACK_CHUNK_SIZE (4)

class DataPacker {
public:
	DataPacker();
	~DataPacker();

	// Copy constructor
	DataPacker(DataPacker &src);

	// Assignment operator
	DataPacker &operator=(DataPacker &b);

	enum ModeEnum { NO_MODE, READ, WRITE };

	enum PackModeEnum { NO_PACKMODE, PACK, DONT_PACK };

	enum ReturnCodes { OK, BAD_POS, BAD_MODE, BAD_PACKMODE, READ_ERROR, WRITE_ERROR, BAD_READFUNC, BAD_WRITEFUNC, BAD_VALUE };

	// Start the bit-packing process : say if we are in READ or WRITE mode
	ReturnCodes open(const ModeEnum mode, const PackModeEnum packMode);

	// Put a value into the bit-stream
	ReturnCodes put(const int32 value, Common::WriteStream *fh);

	// Get a value from the bit-stream
	ReturnCodes Get(int32 &value, Common::SeekableReadStream *stream);

	// Stop the bit-packing process : will output any remaining data
	ReturnCodes close(Common::WriteStream *stream);
	ReturnCodes close(Common::SeekableReadStream *stream);
	// Simple inspectors
	int32 Pos() const { return pos; }
	int32 PackMin() const { return packMin; }
	int32 PackMax() const { return packMax; }

private:
	void ClearBuffer();

	bool readMode;

	ModeEnum iMode;
	PackModeEnum iPackMode;
	int32 pos;
	int32 packMin;
	int32 packMax;

	uint8 buffer[BUFFER_BYTE_SIZE];
};

} // End of namespace ICB

#endif // #ifndef DATAPACKER_HH
