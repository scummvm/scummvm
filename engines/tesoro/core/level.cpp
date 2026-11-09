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

#include "tesoro/core/level.h"
#include "tesoro/core/cameraframe.h"
#include "tesoro/core/spriteobject.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Tesoro {

static const uint32 kHeaderSize = 0xc;
static const uint32 kRoomRecordSize = 0x28;
static const uint32 kObjectRecordSize = 0x20;
static const uint32 kMaxCount = 100000;
static const uint32 kMaxExtra = 0x1000;

Level::~Level() {
	// for (uint i = 0; i < _owned.size(); ++i)
	// 	delete _owned[i];
}

static bool readExactly(Common::SeekableReadStream &s, byte *buf, uint32 size) {
	return size == 0 || s.read(buf, size) == size;
}

bool Level::load(Common::SeekableReadStream &dat, uint32 entryOffset, int mode, int32 now) {
	_loadTime = now;
	initPlain();
	_camera.init();

	if (!dat.seek(entryOffset + 4, SEEK_SET))
		return false;

	byte head[kHeaderSize];
	if (!readExactly(dat, head, sizeof(head)))
		return false;
	const uint32 roomCount = READ_LE_UINT32(head);
	const uint32 bankCount = READ_LE_UINT32(head + 4);
	if (roomCount > kMaxCount || bankCount > kMaxCount) {
		warning("Level: implausible counts (%u rooms, %u levelPlayers)", roomCount, bankCount);
		return false;
	}

	_objects.clear();
	// _movedObjects.clear();

	// The banks follow the header one after the other; back to back
	_levelPlayers.resize(bankCount);
	for (uint32 i = 0; i < bankCount; ++i) {
		if (!_levelPlayers[i].load(dat, (uint32)dat.pos(), mode))
			return false;
	}

	_rooms.resize(roomCount);
	int32 sortOffset = 0;
	for (uint32 r = 0; r < roomCount; ++r) {
		// read each room record
		byte rec[kRoomRecordSize];
		if (!readExactly(dat, rec, sizeof(rec)))
			return false;
		const uint16 objectCount = READ_LE_UINT16(rec + 0x20);
		const uint16 extent = READ_LE_UINT16(rec + 0x22);
		_rooms[r].init(r, READ_LE_INT32(rec + 0x24), (int16)sortOffset);
		sortOffset += extent;

		for (uint16 o = 0; o < objectCount; ++o) {
			// read each object record
			byte orec[kObjectRecordSize];
			if (!readExactly(dat, orec, sizeof(orec)))
				return false;

			ObjectRecord record;
			record.x = READ_LE_INT32(orec);
			record.y = READ_LE_INT32(orec + 4);
			record.sortOffset = (int16)READ_LE_UINT16(orec + 8);
			memcpy(record.name, orec + 0xc, 12);
			record.name[12] = 0;
			record.kind = READ_LE_UINT32(orec + 0x18);

			// Kind specific data, then the indices of the object's banks.
			const uint32 extraSize = READ_LE_UINT32(orec + 0x1c);
			if (extraSize > kMaxExtra) {
				warning("Level: object record with %u bytes of data", extraSize);
				return false;
			}
			record.extra.resize(extraSize);
			record.banks.resize(orec[0xb]);
			if (!readExactly(dat, extraSize ? &record.extra[0] : nullptr, extraSize) ||
					!readExactly(dat, record.banks.empty() ? nullptr : &record.banks[0], record.banks.size()))
				return false;

			Object *object = createObject(record, _rooms[r]);
			if (record.kind == 1)
				_special = object;
			if (object) {
				object->setKindId((byte)record.kind);
				object->setRoom(&_rooms[r]);
				_owned.push_back(object);
				_objects.push_back(object);
			}
		}
	}
	// default camera bouinds
	_camera.setBounds(-320, 239, 319, -240);
	_deferredReloaded = false;
	return true;
}

Object *Level::createObject(const ObjectRecord &record, Room &room) {
	if (record.kind == 0) {
		Object *object = new Object();
		object->initAnimated(record, room.sortOffset(), _levelPlayers, 0);
		return object;
	}

	if (record.kind == 1) {
		CameraFrame *object = new CameraFrame();
		object->init(record, room.sortOffset(), _levelPlayers);
		return object;
	}

	if (record.kind == 3) {
		SpriteObject *object = new SpriteObject();
		object->init(record, room.sortOffset(), _levelPlayers, _loadTime);
		return object;
	}

	warning("Level: object '%s' of kind %u is not ported yet, skipped", record.name, record.kind);
	return nullptr;
}

void Level::attachObjects() {
	attachAll(_objects, this);
}

Object *Level::findObject(uint64 key) {
	for (uint i = 0; i < _objects.size(); ++i) {
		if (_objects[i]->nameKey() == key)
			return _objects[i];
	}
	return nullptr;
}

} // End of namespace Tesoro
