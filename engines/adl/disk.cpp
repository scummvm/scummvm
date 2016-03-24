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

#include "common/stream.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "adl/disk.h"

namespace Adl {

DiskImage::DiskImage() :
		_tracks(0),
		_sectorsPerTrack(0),
		_bytesPerSector(0) {
	_f = new Common::File();
}

DiskImage::~DiskImage() {
	delete _f;
}

DiskImageDataBlock::DiskImageDataBlock(const DiskImage *disk, uint track, uint sector, uint offset, uint size) :
		_track(track),
		_sector(sector),
		_offset(offset),
		_size(size),
		_disk(disk) { }

bool DiskImageDataBlock::isValid() const {
	return _track != 0 || _sector != 0 || _offset != 0 || _size != 0;
}

Common::SeekableReadStream *DiskImageDataBlock::createReadStream() const {
	return _disk->createReadStream(_track, _sector, _offset, _size);
}

// .DSK disk image - 35 tracks, 16 sectors per track, 256 bytes per sector, raw sector layout
const DataBlockPtr DiskImage_DSK::getDataBlock(uint track, uint sector, uint offset, uint size) const {
	return Common::SharedPtr<DiskImageDataBlock>(new DiskImageDataBlock(this, track, sector, offset, size));
}

Common::SeekableReadStream *DiskImage_DSK::createReadStream(uint track, uint sector, uint offset, uint size) const {
	_f->seek((track * _sectorsPerTrack + sector) * _bytesPerSector + offset);
	Common::SeekableReadStream *stream = _f->readStream(size * _bytesPerSector + _bytesPerSector - offset);

	if (_f->eos() || _f->err())
		error("Error reading disk image");

	return stream;
}

bool DiskImage_DSK::open(const Common::String &filename) {
	assert(!_f->isOpen());

	if (!_f->open(filename))
		return false;

	uint filesize = _f->size();
	switch (filesize) {
	case 143360:
		_tracks = 35;
		_sectorsPerTrack = 16;
		_bytesPerSector = 256;
		return true;
	}

	warning("Unrecognized disk image '%s' of size %d bytes", filename.c_str(), filesize);
	return false;
}

Common::SeekableReadStream *FilesDataBlock::createReadStream() const {
	return _files->createReadStream(_filename, _offset);
}

const DataBlockPtr PlainFiles::getDataBlock(const Common::String &filename, uint offset) const {
	return Common::SharedPtr<FilesDataBlock>(new FilesDataBlock(this, filename, offset));
}

Common::SeekableReadStream *PlainFiles::createReadStream(const Common::String &filename, uint offset) const {
	Common::File *f(new Common::File());

	if (!f->open(filename))
		error("Failed to open '%s'", filename.c_str());

	if (offset == 0)
		return f;
	else
		return new Common::SeekableSubReadStream(f, offset, f->size(), DisposeAfterUse::YES);
}

Files_DOS33::~Files_DOS33() {
	delete _disk;
}

Files_DOS33::Files_DOS33() :
		_disk(nullptr) {
}

void Files_DOS33::readSectorList(TrackSector start, Common::Array<TrackSector> &list) {
	TrackSector index = start;

	while (index.track != 0) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(index.track, index.sector));

		stream->readByte();
		index.track = stream->readByte();
		index.sector = stream->readByte();

		stream->seek(9, SEEK_CUR);

		// This only handles sequential files
		TrackSector ts;
		ts.track = stream->readByte();
		ts.sector = stream->readByte();

		while (ts.track != 0) {
			list.push_back(ts);

			ts.track = stream->readByte();
			ts.sector = stream->readByte();

			if (stream->err())
				error("Error reading sector list");

			if (stream->eos())
				break;
		}
	}
}

void Files_DOS33::readVTOC() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(0x11, 0x00));
	stream->readByte();
	byte track = stream->readByte();
	byte sector = stream->readByte();

	while (track != 0) {
		char name[kFilenameLen + 1] = { };

		stream.reset(_disk->createReadStream(track, sector));
		stream->readByte();
		track = stream->readByte();
		sector = stream->readByte();
		stream->seek(8, SEEK_CUR);

		for (uint i = 0; i < 7; ++i) {
			TOCEntry entry;
			TrackSector sectorList;
			sectorList.track = stream->readByte();
			sectorList.sector = stream->readByte();
			entry.type = stream->readByte();
			stream->read(name, kFilenameLen);

			// Convert to ASCII
			for (uint j = 0; j < kFilenameLen; j++)
				name[j] &= 0x7f;

			// Strip trailing spaces
			for (int j = kFilenameLen - 1; j >= 0; --j) {
				if (name[j] == ' ')
					name[j] = 0;
				else
					break;
			}

			entry.totalSectors = stream->readUint16BE();

			if (sectorList.track != 0) {
				readSectorList(sectorList, entry.sectors);
				_toc[name] = entry;
			}
		}
	}
}

const DataBlockPtr Files_DOS33::getDataBlock(const Common::String &filename, uint offset) const {
	return Common::SharedPtr<FilesDataBlock>(new FilesDataBlock(this, filename, offset));
}

Common::SeekableReadStream *Files_DOS33::createReadStreamText(const TOCEntry &entry) const {
	byte *buf = (byte *)malloc(entry.sectors.size() * kSectorSize);
	byte *p = buf;

	for (uint i = 0; i < entry.sectors.size(); ++i) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(entry.sectors[i].track, entry.sectors[i].sector));

		assert(stream->size() == kSectorSize);

		while (true) {
			byte textChar = stream->readByte();

			if (stream->eos() || textChar == 0)
				break;

			if (stream->err())
				error("Error reading text file");

			*p++ = textChar;
		}
	}

	return new Common::MemoryReadStream(buf, p - buf, DisposeAfterUse::YES);
}

Common::SeekableReadStream *Files_DOS33::createReadStreamBinary(const TOCEntry &entry) const {
	byte *buf = (byte *)malloc(entry.sectors.size() * kSectorSize);

	Common::ScopedPtr<Common::SeekableReadStream> stream(_disk->createReadStream(entry.sectors[0].track, entry.sectors[0].sector));

	if (entry.type == kFileTypeBinary)
		stream->readUint16LE(); // Skip start address

	uint16 size = stream->readUint16LE();
	uint16 offset = 0;
	uint16 sectorIdx = 1;

	while (true) {
		offset += stream->read(buf + offset, size - offset);

		if (offset == size)
			break;

		if (stream->err())
			error("Error reading binary file");

		assert(stream->eos());

		if (sectorIdx == entry.sectors.size())
			error("Not enough sectors for binary file size");

		stream.reset(_disk->createReadStream(entry.sectors[sectorIdx].track, entry.sectors[sectorIdx].sector));
		++sectorIdx;
	}

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *Files_DOS33::createReadStream(const Common::String &filename, uint offset) const {
	if (!_toc.contains(filename))
		error("Failed to locate '%s'", filename.c_str());

	const TOCEntry &entry = _toc[filename];

	Common::SeekableReadStream *stream;

	switch(entry.type) {
	case kFileTypeText:
		stream = createReadStreamText(entry);
		break;
	case kFileTypeAppleSoft:
	case kFileTypeBinary:
		stream = createReadStreamBinary(entry);
		break;
	default:
		error("Unsupported file type %i", entry.type);
	}

	return new Common::SeekableSubReadStream(stream, offset, stream->size(), DisposeAfterUse::YES);
}

bool Files_DOS33::open(const Common::String &filename) {
	_disk = new DiskImage_DSK();
	if (!_disk->open(filename))
		return false;

	readVTOC();
	return true;
}

} // End of namespace Adl
