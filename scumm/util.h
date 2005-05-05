/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SCUMM_UTIL_H
#define SCUMM_UTIL_H

#include "common/file.h"
#include "common/stream.h"

namespace Scumm {

class BaseScummFile : public File {
public:
	virtual void setEnc(byte value) = 0;
	
	virtual bool open(const char *filename, AccessMode mode = kFileReadMode) = 0;
	virtual bool openSubFile(const char *filename) = 0;

	virtual bool eof() = 0;
	virtual uint32 pos() = 0;
	virtual uint32 size() = 0;
	virtual void seek(int32 offs, int whence = SEEK_SET) = 0;
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;
	virtual uint32 write(const void *dataPtr, uint32 dataSize) = 0;
};

class ScummFile : public BaseScummFile {
private:
	byte _encbyte;
	uint32	_subFileStart;
	uint32	_subFileLen;
public:
	ScummFile();
	void setEnc(byte value);
	
	void setSubfileRange(uint32 start, uint32 len);
	void resetSubfile();

	bool open(const char *filename, AccessMode mode = kFileReadMode);
	bool openSubFile(const char *filename);

	bool eof();
	uint32 pos();
	uint32 size();
	void seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);
	uint32 write(const void *dataPtr, uint32 dataSize);
};

class ScummNESFile : public BaseScummFile {
public:
	enum ROMset {
		kROMsetUSA,
		kROMsetEurope,
		kROMsetSweden,
		kROMsetFrance,
		kROMsetNum 
	};

	struct Resource;

private:
	Common::MemoryReadStream *_stream;
	ROMset _ROMset;
	byte *_buf;

	bool generateIndex();
	bool generateResource(int res);
	uint16 extractResource(Common::WriteStream *out, Resource *res);

	uint32 resOffset(Resource *res);
	uint16 resLength(Resource *res);

	byte fileReadByte();
	uint16 fileReadUint16LE();

public:
	ScummNESFile();
	void setEnc(byte value);
	
	bool open(const char *filename, AccessMode mode = kFileReadMode);
	bool openSubFile(const char *filename);

	void close();
	bool eof() { return _stream->eos(); }
	uint32 pos() { return _stream->pos(); }
	uint32 size() { return _stream->size(); }
	void seek(int32 offs, int whence = SEEK_SET) { _stream->seek(offs, whence); }
	uint32 read(void *dataPtr, uint32 dataSize) { return _stream->read(dataPtr, dataSize); }
	uint32 write(const void *dataPtr, uint32 dataSize);
};


// This is a constant lookup table of reverse bit masks
extern const byte revBitMask[8];

/* Direction conversion functions (between old dir and new dir format) */
int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

int normalizeAngle(int angle);
int fromSimpleDir(int dirtype, int dir);
int toSimpleDir(int dirtype, int dir);

void checkRange(int max, int min, int no, const char *str);

const char *tag2str(uint32 tag);

} // End of namespace Scumm

#endif
