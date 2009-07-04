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

#ifndef GOB_DATAIO_H
#define GOB_DATAIO_H


#include "common/endian.h"

#include "common/file.h"

namespace Gob {

#define MAX_FILES	30
#define MAX_DATA_FILES	8
#define MAX_SLOT_COUNT	8

class DataIO;

class DataStream : public Common::SeekableReadStream {
public:
	DataStream(DataIO &io, int16 handle, uint32 dSize, bool dispose = false);
	DataStream(byte *buf, uint32 dSize, bool dispose = true);
	virtual ~DataStream();

	virtual int32 pos() const;
	virtual int32 size() const;

	virtual bool seek(int32 offset, int whence = SEEK_SET);

	virtual bool eos() const;

	virtual uint32 read(void *dataPtr, uint32 dataSize);

private:
	DataIO *_io;
	int16 _handle;
	uint32 _size;
	byte *_data;
	Common::MemoryReadStream *_stream;
	bool _dispose;
};

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
	int16 openData(const char *path);
	bool existData(const char *path);
	DataStream *openAsStream(int16 handle, bool dispose = false);

	int32 getDataSize(const char *name);
	byte *getData(const char *path);
	DataStream *getDataStream(const char *path);

	DataIO(class GobEngine *vm);
	~DataIO();

protected:
	Common::File _filesHandles[MAX_FILES];
	struct ChunkDesc *_dataFiles[MAX_DATA_FILES];
	uint16 _numDataChunks[MAX_DATA_FILES];
	int16 _dataFileHandles[MAX_DATA_FILES];
	bool _dataFileItk[MAX_DATA_FILES];
	int32 _chunkPos[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkOffset[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _chunkSize[MAX_SLOT_COUNT * MAX_DATA_FILES];
	bool _isCurrentSlot[MAX_SLOT_COUNT * MAX_DATA_FILES];
	int32 _packedSize;

	class GobEngine *_vm;

	int16 file_open(const char *path);
	Common::File *file_getHandle(int16 handle);
	const Common::File *file_getHandle(int16 handle) const;

	int16 getChunk(const char *chunkName);
	char freeChunk(int16 handle);
	int32 readChunk(int16 handle, byte *buf, uint16 size);
	int16 seekChunk(int16 handle, int32 pos, int16 from);
	uint32 getChunkPos(int16 handle) const;
	int32 getChunkSize(const char *chunkName);

	uint32 getPos(int16 handle);
	void seekData(int16 handle, int32 pos, int16 from);
	int32 readData(int16 handle, byte *buf, uint16 size);

friend class DataStream;
};

} // End of namespace Gob

#endif // GOB_DATAIO_H
