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
#include "common/debug.h"

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

	virtual Common::SeekableReadStream *createReadStream() const = 0;
};

typedef Common::SharedPtr<DataBlock> DataBlockPtr;
typedef Common::ScopedPtr<Common::SeekableReadStream> StreamPtr;

class Files {
public:
	virtual ~Files() { }

	virtual const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const = 0;
	virtual Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const = 0;

protected:
	class DataBlock : public Adl::DataBlock {
	public:
		DataBlock(const Files *files, const Common::String &filename, uint offset) :
				_files(files),
				_filename(filename),
				_offset(offset) { }

		Common::SeekableReadStream *createReadStream() const {
			return _files->createReadStream(_filename, _offset);
		}

	private:
		const Common::String _filename;
		uint _offset;
		const Files *_files;
	};
};

class DiskImage {
public:
	DiskImage() :
			_stream(nullptr),
			_mode13(false) { }

	~DiskImage() {
		delete _stream;
	}

	bool open(const Common::String &filename);
	const DataBlockPtr getDataBlock(uint track, uint sector, uint offset = 0, uint size = 0) const;
	Common::SeekableReadStream *createReadStream(uint track, uint sector, uint offset = 0, uint size = 0, uint sectorsPerTrackToRead = 16) const;
	void setMode13(bool enable) { _mode13 = enable; }

protected:
	class DataBlock : public Adl::DataBlock {
	public:
		DataBlock(const DiskImage *disk, uint track, uint sector, uint offset, uint size, bool mode13) :
				_track(track),
				_sector(sector),
				_offset(offset),
				_size(size),
				_mode13(mode13),
				_disk(disk) { }

		Common::SeekableReadStream *createReadStream() const {
			return _disk->createReadStream(_track, _sector, _offset, _size, (_mode13 ? 13 : 16));
		}

	private:
		uint _track, _sector, _offset, _size;
		bool _mode13;
		const DiskImage *_disk;
	};

	Common::SeekableReadStream *_stream;
	bool _mode13; // Older 13-sector format
};

// Data in plain files
class Files_Plain : public Files {
public:
	const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const;
	Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const;
};

// Data in files contained in Apple DOS 3.3 disk image
class Files_DOS33 : public Files {
public:
	Files_DOS33();
	~Files_DOS33();

	bool open(const Common::String &filename);
	const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const;
	Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const;

private:
	enum FileType {
		kFileTypeText = 0,
		kFileTypeAppleSoft = 2,
		kFileTypeBinary = 4 
	};

	enum {
		kSectorSize = 256,
		kFilenameLen = 30
	};

	struct TrackSector {
		byte track;
		byte sector;
	};

	struct TOCEntry {
		byte type;
		uint16 totalSectors;
		Common::Array<TrackSector> sectors;
	};

	void readVTOC();
	void readSectorList(TrackSector start, Common::Array<TrackSector> &list);
	Common::SeekableReadStream *createReadStreamText(const TOCEntry &entry) const;
	Common::SeekableReadStream *createReadStreamBinary(const TOCEntry &entry) const;

	DiskImage *_disk;
	Common::HashMap<Common::String, TOCEntry> _toc;
};

} // End of namespace Adl

#endif
