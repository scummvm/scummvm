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

#include "common/ptr.h"
#include "common/file.h"
#include "common/debug.h"

#include "common/formats/disk_image.h"

#ifndef ADL_DISK_H
#define ADL_DISK_H

namespace Common {
class SeekableReadStream;
class String;
}

namespace Adl {

// On the Apple II, sector headers contain a disk volume number. This number
// is used by ADL multi-disk games. The PC port has the disk volume number
// as the first data byte of every sector that contains game data. We need
// to skip this number as we read in the data. Additionally, the data is now
// prefixed with an uint16 containing the data size.
class DataBlock_PC : public Common::DataBlock {
public:
	DataBlock_PC(Common::DiskImage *disk, byte track, byte sector, uint16 offset = 0) :
			_disk(disk),
			_track(track),
			_sector(sector),
			_offset(offset) { }

	~DataBlock_PC() override { }

	Common::SeekableReadStream *createReadStream() const override;

private:
	void read(Common::SeekableReadStream &stream, byte *const dataPtr, const uint32 size) const;

	Common::DiskImage *_disk;
	byte _track, _sector;
	uint16 _offset;
};

} // End of namespace Adl

#endif
