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
		break;
	default:
		warning("Unrecognized disk image '%s' of size %d bytes", filename.c_str(), filesize);
	}

	return true;
}

} // End of namespace Adl
