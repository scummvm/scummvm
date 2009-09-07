/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */
 
#ifndef STARK_ARCHIVE_H
#define STARK_ARCHIVE_H

#include "common/array.h"
#include "common/file.h"
#include "common/str.h"
#include "common/stream.h"

namespace Stark {
	
class StarkArchive {
public:
	StarkArchive() {}
	virtual ~StarkArchive() {}

	virtual bool open(Common::String filename) = 0;
	virtual void close() = 0;
	
	virtual Common::SeekableReadStream *getRawData(uint32 fileNum) { return NULL; }

	// TODO: Some get data function or something
	
protected:
	Common::File _file;
	
	struct FileEntry {
		Common::String filename;
		uint32 offset;
		uint32 length;
		// For non-graph.arc files, there is more here. 12 other bytes of data.
	} *_fileEntries;
};

class XARCArchive : public StarkArchive {
public:
	XARCArchive() : StarkArchive() {}
	~XARCArchive() { close(); }

	bool open(Common::String filename);
	void close();
	
	Common::SeekableReadStream *getRawData(uint32 fileNum);
	
	inline static Common::String readString(Common::SeekableReadStream *stream) {
		Common::String ret = Common::String::emptyString;
		byte ch;
		while ((ch = stream->readByte()) != 0x20)
			ret += ch;

		return ret;
	}

private:
	uint32 _fileCount;
	uint32 *_offsets;
};

}

#endif
