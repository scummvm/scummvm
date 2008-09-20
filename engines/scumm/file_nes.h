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
 * $URL$
 * $Id$
 *
 */

#ifndef SCUMM_FILE_NES_H
#define SCUMM_FILE_NES_H

#include "common/file.h"

#include "scumm/file.h"

namespace Scumm {

class ScummNESFile : public BaseScummFile {
public:
	enum ROMset {
		kROMsetUSA,
		kROMsetEurope,
		kROMsetSweden,
		kROMsetFrance,
		kROMsetGermany,
		kROMsetSpain,
		kROMsetNum
	};

	struct Resource;
	struct LFLEntry;
	struct LFL;

private:
	Common::MemoryReadStream *_stream;
	ROMset _ROMset;
	byte *_buf;

	bool generateIndex();
	bool generateResource(int res);
	uint16 extractResource(Common::WriteStream *out, const Resource *res);

	byte fileReadByte();
	uint16 fileReadUint16LE();

public:
	ScummNESFile();
	void setEnc(byte value);

	bool open(const Common::String &filename);
	bool openSubFile(const Common::String &filename);

	void close();
	bool eos() { return _stream->eos(); }
	int32 pos() { return _stream->pos(); }
	int32 size() { return _stream->size(); }
	bool seek(int32 offs, int whence = SEEK_SET) { return _stream->seek(offs, whence); }
	uint32 read(void *dataPtr, uint32 dataSize) { return _stream->read(dataPtr, dataSize); }
};

} // End of namespace Scumm

#endif
