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
#include "tesoro/core/collision.h"
#include "tesoro/core/cursor.h"
#include "tesoro/core/spriteobject.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "tesoro/core/streamutil.h"

namespace Tesoro {

static const uint32 kHeaderSize = 0xc;
static const uint32 kRoomRecordSize = 0x28;
static const uint32 kObjectRecordSize = 0x20;
static const uint32 kMaxCount = 100000;
static const uint32 kMaxExtra = 0x1000;

Level::~Level() {
	for (uint i = 0; i < _owned.size(); ++i)
		delete _owned[i];
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
	_movedObjects.clear();

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

	if (record.kind == 10) {
		Cursor *object = new Cursor();
		object->initMain(record, room.sortOffset(), _levelPlayers, _loadTime);
		return object;
	}

	warning("Level: object '%s' of kind %u is not ported yet, skipped", record.name, record.kind);
	return nullptr;
}

void Level::attachObjects() {
	for (uint i = 0; i < _objects.size(); ++i)
		_objects[i]->attachToLevel(this);
}

void Level::objectMoved(Object *object) {
	if (!(object->flags() & kCollidable))
		return;

	for (uint i = 0; i < _movedObjects.size(); ++i) {
		if (_movedObjects[i] == object)
			return;
	}
	_movedObjects.push_back(object);
}

void Level::collisionPass(int32 now) {
	_touches.clear();

	// The objects that moved get their bounds worked out first.
	for (uint i = 0; i < _movedObjects.size(); ++i) {
		Object *object = _movedObjects[i];
		object->updateBounds(now);
		object->setFlags(object->flags() | kProcessed);
	}

	// So do the collidable objects sharing a room with them.
	for (uint i = 0; i < _movedObjects.size(); ++i) {
		ObjectList &room = _movedObjects[i]->room()->objects();
		for (uint j = 0; j < room.size(); ++j) {
			Object *other = room[j];
			const uint32 flags = other->flags();
			if ((flags & kCollidable) && (flags & kActive) && !(flags & kProcessed))
				other->updateBounds(now);
		}
	}

	for (uint i = 0; i < _movedObjects.size(); ++i)
		_movedObjects[i]->setFlags(_movedObjects[i]->flags() & ~kProcessed);

	// Every moved collidable object against everything collidable and active in its room.
	for (uint i = 0; i < _movedObjects.size(); ++i) {
		Object *object = _movedObjects[i];
		if (!(object->flags() & kCollidable))
			continue;

		ObjectList &room = object->room()->objects();
		for (uint j = 0; j < room.size(); ++j) {
			Object *other = room[j];
			if (other == object || !(other->flags() & kCollidable) || !(other->flags() & kActive))
				continue;

			if (object->isHitter(other))
				resolveOverlap(object, other);
			else if (other->isHitter(object))
				resolveOverlap(other, object);
		}
	}

	for (uint i = 0; i < _touches.size(); ++i) {
		const Touch touch = _touches[i];
		touch.hitter->onTouched(touch.other, touch.time);
	}
	_touches.clear();

	// The moved objects go back to where they were before the move.
	for (uint i = 0; i < _movedObjects.size(); ++i) {
		Object *object = _movedObjects[i];
		object->setPosition(object->previousX(), object->previousY());
	}

	_movedObjects.clear();
	_loadTime = now;
}

void Level::resolveOverlap(Object *a, Object *b) {
	const ObjectRect &am = a->movedArea(), &bm = b->movedArea();
	const ObjectRect &ah = a->hitRect(), &bh = b->hitRect();

	// How far a moves relative to b, and a's hit rectangle where it stood when b was last drawn.
	const int32 dx = (am.x2 - am.x1) - (bm.x2 - bm.x1);
	const int32 dy = (am.y2 - bm.y2) - am.y1 + bm.y1;
	const int32 offsetX = am.x1 - bm.x1;
	const int32 offsetY = am.y1 - bm.y1;
	const int32 time = sweepRects(ah.x1 + offsetX, ah.y1 + offsetY, ah.x2 + offsetX, ah.y2 + offsetY,
	                              bh.x1, bh.y1, bh.x2, bh.y2, dx, dy);
	if (time < 0)
		return;

	// Objects with an outline refine the rectangle result; the touch is dropped when this gives -1.
	const AnimStep *aShape = shapeOf(a), *bShape = shapeOf(b);
	int32 touchTime = time;
	if (aShape && bShape && !aShape->entries.empty() && !bShape->entries.empty())
		touchTime = shapedContactTime(a, b, aShape, bShape, time, dx, dy);
	if (touchTime == -1)
		return;

	// Touches are ordered by time, then by depth, a later one going behind an equal one.
	uint at = 0;
	while (at < _touches.size() && !(touchTime < _touches[at].time || (touchTime == _touches[at].time && (int)depth < (int)_touches[at].depth)))
		++at;

	Touch touch;
	touch.hitter = a;
	touch.other = b;
	touch.time = touchTime;
	touch.depth = (int16)depth;
	_touches.insert_at(at, touch);
}

int Level::update() {
	collisionPass(_frameTime);
	return 1;
}

Object *Level::findObject(uint64 key) {
	for (uint i = 0; i < _objects.size(); ++i) {
		if (_objects[i]->nameKey() == key)
			return _objects[i];
	}
	return nullptr;
}

void Level::drawRooms(int32 now, Display &display) {
	for (uint r = 0; r < _rooms.size(); ++r) {
		Room &room = _rooms[r];
		_camera.updateToward(room.cameraDivisor(), now);

		ObjectList &list = room.objects();
		for (uint i = 0; i < list.size(); ++i) {
			Object *object = list[i];
			const uint32 flags = object->flags();
			if (!(flags & kProcessed) && (flags & kVisible) && (flags & kActive)) {
				object->updateBounds(now);
				object->draw(_camera, display);
			}
		}
	}
}

} // End of namespace Tesoro
