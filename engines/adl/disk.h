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
 */

#include "common/ptr.h"
#include "common/file.h"

#ifndef ADL_DISK_H
#define ADL_DISK_H

namespace Common {
class SeekableReadStream;
class String;
}

namespace Adl {

class DataBlock {
public:
	virtual ~DataBlock() { }

	virtual bool isValid() const = 0;
	virtual Common::SeekableReadStream *createReadStream() const = 0;
};

typedef Common::SharedPtr<DataBlock> DataBlockPtr;

class Files {
public:
	virtual ~Files() { }

	virtual const DataBlockPtr getDataBlock(const Common::String &filename, uint offset) const = 0;
	virtual Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset) const = 0;
};

class FilesDataBlock : public DataBlock {
public:
	FilesDataBlock(const Files *files, const Common::String &filename, uint offset) : _files(files), _filename(filename), _offset(offset) { }

	bool isValid() const { return true; }
	Common::SeekableReadStream *createReadStream() const;

private:
	const Common::String _filename;
	uint _offset;
	const Files *_files;
};

class PlainFiles : public Files {
public:
	const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const;
	Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const;
};

class DiskImage {
public:
	DiskImage();
	virtual ~DiskImage();

	virtual bool open(const Common::String &filename) = 0;
	virtual const DataBlockPtr getDataBlock(uint track, uint sector, uint offset, uint size) const = 0;
	virtual Common::SeekableReadStream *createReadStream(uint track, uint sector, uint offset, uint size) const = 0;

protected:
	Common::File *_f;
	uint _tracks, _sectorsPerTrack, _bytesPerSector;
};

class DiskImageDataBlock : public DataBlock {
public:
	DiskImageDataBlock(const DiskImage *disk, uint track, uint sector, uint offset, uint size);

	bool isValid() const;
	Common::SeekableReadStream *createReadStream() const;

private:
	uint _track, _sector, _offset, _size;
	const DiskImage *_disk;
};

class DiskImage_DSK : public DiskImage {
public:
	bool open(const Common::String &filename);
	const DataBlockPtr getDataBlock(uint track, uint sector, uint offset, uint size) const;
	Common::SeekableReadStream *createReadStream(uint track, uint sector, uint offset = 0, uint size = 0) const;
};

} // End of namespace Adl

#endif
