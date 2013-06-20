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

#ifndef FULLPIPE_UTILS_H
#define FULLPIPE_UTILS_H

#include "common/hash-str.h"
#include "common/array.h"
#include "common/file.h"

namespace Fullpipe {

class CObject;

typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ClassMap;

class MfcArchive : public Common::SeekableReadStream {
	ClassMap _classMap;
	Common::Array<CObject *> _objectMap;
	Common::Array<int> _objectIdMap;

	int _lastIndex;
	int _level;

	Common::SeekableReadStream *_stream;

 public:
	MfcArchive(Common::SeekableReadStream *file);

	char *readPascalString(bool twoByte = false);
	int readCount();
	double readDouble();
	CObject *parseClass(bool *isCopyReturned);
	CObject *readClass();

	void incLevel() { _level++; }
	void decLevel() { _level--; }
	int getLevel() { return _level; }

	virtual bool eos() const { return _stream->eos(); }
	virtual uint32 read(void *dataPtr, uint32 dataSize) { return _stream->read(dataPtr, dataSize); }
	virtual int32 pos() const { return _stream->pos(); }
	virtual int32 size() const { return _stream->size(); }
	virtual bool seek(int32 offset, int whence = SEEK_SET) { return _stream->seek(offset, whence); }
};

class CObject {
 public:
	virtual bool load(MfcArchive &in) { return true; }
	virtual ~CObject() {}

	bool loadFile(const char *fname);
};

class CObList : public Common::List<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class MemoryObject {
	//CObject obj;
	int filename;
	int field_8;
	int field_C;
	int field_10;
	char field_14;
	char field_15;
	char field_16;
	char field_17;
	int data;
	int dataSize;
	int flags;
	int libHandle;
};

class CObArray : public Common::Array<CObject>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

class CDWordArray : public Common::Array<int32>, public CObject {
 public:
	virtual bool load(MfcArchive &file);
};

struct CNode {
	CNode *pNext;
	CNode *pPrev;
	void *data;
};

typedef Common::Array<void *> CPtrList;

char *genFileName(int superId, int sceneId, const char *ext);

} // End of namespace Fullpipe

#endif /* FULLPIPE_UTILS_H */
