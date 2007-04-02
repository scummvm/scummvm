/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_DATAIO_H
#define GOB_DATAIO_H

#include "common/stdafx.h"
#include "common/endian.h"

#include "common/file.h"

namespace Gob {

#define MAX_FILES	30
#define MAX_DATA_FILES	8
#define MAX_SLOT_COUNT	4

class DataIO {
public:
	struct ChunkDesc {
		char chunkName[13];
		uint32 size;
		uint32 offset;
		byte packed;
		ChunkDesc() : size(0), offset(0), packed(0) { chunkName[0] = 0; }
	};

	int32 unpackData(byte *src, byte *dest);

	void openDataFile(const char *src, bool itk = 0);
	void closeDataFile(bool itk = 0);
	byte *getUnpackedData(const char *name);
	void closeData(int16 handle);
	int16 openData(const char *path,
			Common::File::AccessMode mode = Common::File::kFileReadMode);
	int32 readData(int16 handle, byte *buf, uint16 size);
	byte readByte(int16 handle);
	uint16 readUint16(int16 handle);
	uint32 readUint32(int16 handle);
	int32 writeData(int16 handle, byte *buf, uint16 size);
	void seekData(int16 handle, int32 pos, int16 from);
	uint32 getPos(int16 handle);
	int32 getDataSize(const char *name);
	byte *getData(const char *path);

	DataIO(class GobEngine *vm);
	~DataIO();

protected:
	Common::File _filesHandles[MAX_FILES];
	struct ChunkDesc *_dataFiles[MAX_DATA_FILES];
	int16 _numDataChunks[MAX_DATA_FILES];
	int16 _dataFileHandles[MAX_DATA_FILES];
	bool _dataFileItk[MAX_DATA_FILES];
	int32 _chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
	bool _isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _packedSize;

	class GobEngine *_vm;

	int16 file_open(const char *path,
			Common::File::AccessMode mode = Common::File::kFileReadMode);
	Common::File *file_getHandle(int16 handle);

	int16 getChunk(const char *chunkName);
	char freeChunk(int16 handle);
	int32 readChunk(int16 handle, byte *buf, uint16 size);
	int16 seekChunk(int16 handle, int32 pos, int16 from);
	uint32 getChunkPos(int16 handle);
	int32 getChunkSize(const char *chunkName);
};

} // End of namespace Gob

#endif // GOB_DATAIO_H
