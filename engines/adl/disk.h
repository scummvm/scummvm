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

// Used for disk image detection
int32 computeMD5(const Common::FSNode &node, Common::String &md5, uint32 md5Bytes);

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

		Common::SeekableReadStream *createReadStream() const override {
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
			_tracks(0),
			_sectorsPerTrack(0),
			_bytesPerSector(0),
			_sectorLimit(0),
			_firstSector(0) { }

	~DiskImage() {
		delete _stream;
	}

	bool open(const Common::String &filename);
	const DataBlockPtr getDataBlock(uint track, uint sector, uint offset = 0, uint size = 0) const;
	Common::SeekableReadStream *createReadStream(uint track, uint sector, uint offset = 0, uint size = 0, uint sectorsUsed = 0) const;
	void setSectorLimit(uint sectorLimit) { _sectorLimit = sectorLimit; } // Maximum number of sectors to read per track before stepping
	uint getBytesPerSector() const { return _bytesPerSector; }
	uint getSectorsPerTrack() const { return _sectorsPerTrack; }
	uint getTracks() const { return _tracks; }

protected:
	class DataBlock : public Adl::DataBlock {
	public:
		DataBlock(const DiskImage *disk, uint track, uint sector, uint offset, uint size, uint sectorLimit) :
				_track(track),
				_sector(sector),
				_offset(offset),
				_size(size),
				_sectorLimit(sectorLimit),
				_disk(disk) { }

		Common::SeekableReadStream *createReadStream() const override {
			return _disk->createReadStream(_track, _sector, _offset, _size, _sectorLimit);
		}

	private:
		uint _track, _sector, _offset, _size;
		uint _sectorLimit;
		const DiskImage *_disk;
	};

	Common::SeekableReadStream *_stream;
	uint _tracks, _sectorsPerTrack, _bytesPerSector, _firstSector;
	uint _sectorLimit;
};

// Data in plain files
class Files_Plain : public Files {
public:
	const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const override;
	Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const override;
};

// Data in files contained in Apple DOS 3.3 disk image
class Files_AppleDOS : public Files {
public:
	Files_AppleDOS();
	~Files_AppleDOS() override;

	bool open(const Common::String &filename, uint trackVTOC = 17);
	const DataBlockPtr getDataBlock(const Common::String &filename, uint offset = 0) const override;
	Common::SeekableReadStream *createReadStream(const Common::String &filename, uint offset = 0) const override;

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

	void readVTOC(uint trackVTOC);
	void readSectorList(TrackSector start, Common::Array<TrackSector> &list);
	Common::SeekableReadStream *createReadStreamText(const TOCEntry &entry) const;
	Common::SeekableReadStream *createReadStreamBinary(const TOCEntry &entry) const;

	DiskImage *_disk;
	Common::HashMap<Common::String, TOCEntry> _toc;
};

// On the Apple II, sector headers contain a disk volume number. This number
// is used by ADL multi-disk games. The PC port has the disk volume number
// as the first data byte of every sector that contains game data. We need
// to skip this number as we read in the data. Additionally, the data is now
// prefixed with an uint16 containing the data size.
class DataBlock_PC : public DataBlock {
public:
	DataBlock_PC(DiskImage *disk, byte track, byte sector, uint16 offset = 0) :
			_disk(disk),
			_track(track),
			_sector(sector),	
			_offset(offset) { }

	~DataBlock_PC() override { }

	Common::SeekableReadStream *createReadStream() const override;

private:
	void read(Common::SeekableReadStream &stream, byte *const dataPtr, const uint32 size) const;

	DiskImage *_disk;
	byte _track, _sector;
	uint16 _offset;
};

} // End of namespace Adl

#endif
