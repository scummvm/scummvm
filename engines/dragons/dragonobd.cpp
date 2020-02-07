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
#include "common/memstream.h"
#include "dragons/dragonobd.h"
#include "dragons/bigfile.h"

namespace Dragons {

DragonOBD::DragonOBD(BigfileArchive *bigfileArchive) {
	uint32 size;
	byte *optData = bigfileArchive->load("dragon.opt", size);
	_optReadStream = new Common::MemoryReadStream(optData, size, DisposeAfterUse::YES);

	byte *sptData = bigfileArchive->load("dragon.spt", size);
	_sptReadStream = new Common::MemoryReadStream(sptData, size, DisposeAfterUse::YES);

	_data = bigfileArchive->load("dragon.obd", _dataSize);
}

byte *DragonOBD::getObdAtOffset(uint32 offset) {
	assert(_data);
	assert(offset < _dataSize);
	return &_data[offset];
}

DragonOBD::~DragonOBD() {
	if (_data) {
		delete _data;
	}
	delete _optReadStream;
	delete _sptReadStream;
}

byte *DragonOBD::getFromOpt(uint32 index) {
	_optReadStream->seek(index * 8);
	return getObdAtOffset(_optReadStream->readUint32LE());
}

byte *DragonOBD::getFromSpt(uint32 index) {
	_sptReadStream->seek(index * 4);
	return getObdAtOffset(_sptReadStream->readUint32LE());
}

} // End of namespace Dragons
