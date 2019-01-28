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
#include <common/debug.h>
#include "common/memstream.h"
#include "dragonini.h"

namespace Dragons {

#define DRAGON_INI_STRUCT_SIZE 0x22
DragonINIResource::DragonINIResource(BigfileArchive *bigfileArchive) {
	uint32 fileSize;
	byte *data = bigfileArchive->load("dragon.ini", fileSize);
	Common::SeekableReadStream *readStream = new Common::MemoryReadStream(data, fileSize, DisposeAfterUse::YES);

	_count = fileSize / DRAGON_INI_STRUCT_SIZE;

	_dragonINI = new DragonINI[_count];

	for(int i=0; i < _count; i++) {
		_dragonINI[i].id = (uint16)i;
		_dragonINI[i].iptIndex_maybe = readStream->readSint16LE();
		_dragonINI[i].field_2 = readStream->readSint16LE();
		_dragonINI[i].actorResourceId = readStream->readSint16LE();
		_dragonINI[i].frameIndexId_maybe = readStream->readSint16LE();
		_dragonINI[i].field_8 = readStream->readSint16LE();
		assert(readStream->readUint16LE() == 0); // actorId
		_dragonINI[i].actor = NULL;
		_dragonINI[i].sceneId = readStream->readUint16LE();
		_dragonINI[i].field_e = readStream->readSint16LE();
		_dragonINI[i].field_10 = readStream->readSint16LE();
		_dragonINI[i].field_12 = readStream->readSint16LE();
		_dragonINI[i].field_14 = readStream->readSint16LE();
		_dragonINI[i].x = readStream->readSint16LE();
		_dragonINI[i].y = readStream->readSint16LE();
		_dragonINI[i].field_1a_flags_maybe = readStream->readUint16LE();
		_dragonINI[i].field_1c = readStream->readSint16LE();
		_dragonINI[i].field_1e = readStream->readSint16LE();
		_dragonINI[i].field_20_actor_field_14 = readStream->readUint16LE();
	}

	_flickerINI = &_dragonINI[0];
}

DragonINI *DragonINIResource::getRecord(uint16 index) {
	assert (index < _count);

	return &_dragonINI[index];
}

void DragonINIResource::setFlickerRecord(DragonINI *dragonINI) {
	_flickerINI = dragonINI;
}

bool DragonINIResource::isFlicker(uint16 index) {
	return _flickerINI && _flickerINI->id == index;
}


} // End of namespace Dragons
