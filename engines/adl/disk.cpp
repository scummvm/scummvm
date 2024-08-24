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

#include "common/stream.h"
#include "common/substream.h"
#include "common/memstream.h"

#include "common/formats/disk_image.h"

#include "adl/disk.h"

namespace Adl {

void DataBlock_PC::read(Common::SeekableReadStream &stream, byte *const dataPtr, const uint32 size) const {
	uint32 ofs = 0;

	while (ofs < size) {
		const uint bps = _disk->getBytesPerSector();
		uint bytesToRead = bps - ((_offset + stream.pos()) % bps);

		if (bytesToRead == bps) {
			stream.readByte(); // Skip volume byte
			--bytesToRead;
		}

		if (bytesToRead > size - ofs)
			bytesToRead = size - ofs;

		if (stream.read(dataPtr + ofs, bytesToRead) < bytesToRead)
			error("Failed to read data block");

		ofs += bytesToRead;
	}
}

Common::SeekableReadStream *DataBlock_PC::createReadStream() const {
	const uint bps = _disk->getBytesPerSector();
	uint sectors = 0;

	// Every data sector starts with a volume byte that we need to skip,
	// so we need to take that into account during our computations here
	if (_offset == bps - 1)
		sectors = 1;

	Common::StreamPtr diskStream(_disk->createReadStream(_track, _sector, _offset, sectors));

	byte sizeBuf[2];
	read(*diskStream, sizeBuf, 2);

	uint16 blockSize = READ_LE_UINT16(sizeBuf);
	sectors = 0;

	const uint16 remSize = _disk->getBytesPerSector() - MAX<uint>(_offset, 1);

	if (blockSize + 2 > remSize)
		sectors = (blockSize + 2 - remSize - 1) / (_disk->getBytesPerSector() - 1) + 1;

	diskStream.reset(_disk->createReadStream(_track, _sector, _offset, sectors));
	read(*diskStream, sizeBuf, 2);

	byte *buf = static_cast<byte *>(malloc(blockSize));
	read(*diskStream, buf, blockSize);

	return new Common::MemoryReadStream(buf, blockSize, DisposeAfterUse::YES);
}

} // End of namespace Common
