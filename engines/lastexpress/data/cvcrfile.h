/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef LASTEXPRESS_CVCRFILE_H
#define LASTEXPRESS_CVCRFILE_H

#include "lastexpress/lastexpress.h"
#include "lastexpress/shared.h"

namespace Common {
class MemorySeekableReadWriteStream;
class SeekableReadStream;
}

namespace LastExpress {

#define CVCRMODE_WB   0
#define CVCRMODE_RB   1
#define CVCRMODE_RWB  2

struct SVCRFileHeader {
	int32 magicNumber;
	int32 numVCRGames;
	int32 nextWritePos;
	int32 realWritePos;
	int32 lastIsTemporary;
	int32 brightness;
	int32 volume;
	int32 saveVersion;
};

struct SVCRSavePointHeader {
	int32 magicNumber;
	int32 type;
	int32 time;
	int32 size;
	int32 partNumber;
	int32 latestGameEvent;
	int32 emptyField1;
	int32 emptyField2;
};

class CVCRFile {
public:
	CVCRFile(LastExpressEngine *engine);
	~CVCRFile();

	bool open(const char *filename, int mode);
	uint32 read(void *dstBuf, uint32 elementSize, uint32 elementCount, bool forceSimpleReadOnRle, bool checkSize);
	uint32 write(void *srcBuf, uint32 elementSize, uint32 elementCount, bool forceSimpleReadOnRle);
	uint32 readRLE(void *dstBuf, uint32 elementSize, uint32 elementCount);
	uint32 writeRLE(void *srcBuf, uint32 elementSize, uint32 elementCount);
	void writeToRLEBuffer(uint8 operation, uint8 flag);
	uint32 flush();
	uint32 seek(int32 offset, int mode);
	void close();
	int32 tell();

	bool fileIsOpen() { return _virtualFileIsOpen; }

private:
	LastExpressEngine *_engine = nullptr;
	Common::MemorySeekableReadWriteStream *_rwStream = nullptr;
	Common::String _filename = "";
	bool _virtualFileIsOpen = false;
	uint32 _virtualFileSize = 0;
	uint32 _rwStreamSize = 0;
	uint8 _rleDstBuf[256];
	int32 _rleBufferPosition = -1;
	uint8 _rleByte = 0;
	uint8 _rleCount = 0;
	int32 _rleMode = 0;
	int32 _bytesProcessed = 0;
	int32 _rleStatus = 0;
	int32 _fileMode = 0;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_CVCRFILE_H
