/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm/scumm.h"
#include "scumm/actor.h"
#include "scumm/bomp.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/usage_bits.h"

namespace Scumm {

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct BompHeader {			/* Bomp header */
	union {
		struct {
			uint16 unk;
			uint16 width, height;
		} GCC_PACK old;

		struct {
			uint32 width, height;
		} GCC_PACK v8;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif


bool ScummEngine::getClass(int obj, int cls) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in getClass");

	if (_features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	return (_classData[obj] & (1 << (cls - 1))) != 0;
}

void ScummEngine::putClass(int obj, int cls, bool set) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putClass");
	cls &= 0x7F;
	checkRange(32, 1, cls, "Class %d out of range in putClass");

	if (_features & GF_SMALL_HEADER) {
		// Translate the new (V5) object classes to the old classes
		// (for those which differ).
		switch (cls) {
		case kObjectClassUntouchable:
			cls = 24;
			break;
		case kObjectClassPlayer:
			cls = 23;
			break;
		case kObjectClassXFlip:
			cls = 19;
			break;
		case kObjectClassYFlip:
			cls = 18;
			break;
		}
	}

	if (set)
		_classData[obj] |= (1 << (cls - 1));
	else
		_classData[obj] &= ~(1 << (cls - 1));

	if (1 <= obj && obj < _numActors) {
		_actors[obj].classChanged(cls, set);
	}
}

int ScummEngine::getOwner(int obj) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getOwner");
	return _objectOwnerTable[obj];
}

void ScummEngine::putOwner(int obj, int owner) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putOwner");
	checkRange(0xFF, 0, owner, "Owner %d out of range in putOwner");
	_objectOwnerTable[obj] = owner;
}

int ScummEngine::getState(int obj) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getState");

	if (!_copyProtection) {
		// I knew LucasArts sold cracked copies of the original Maniac Mansion,
		// at least as part of Day of the Tentacle. Apparently they also sold
		// cracked versions of the enhanced version. At least in Germany.
		//
		// This will keep the security door open at all times. I can only
		// assume that 182 and 193 each correspond to one particular side of
		// the it. Fortunately it does not prevent frustrated players from
		// blowing up the mansion, should they feel the urge to.

		if (_gameId == GID_MANIAC && (obj == 182 || obj == 193))
			_objectStateTable[obj] |= 0x08;
	}

	return _objectStateTable[obj];
}

void ScummEngine::putState(int obj, int state) {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in putState");
	checkRange(0xFF, 0, state, "State %d out of range in putState");
	_objectStateTable[obj] = state;
}

int ScummEngine::getObjectRoom(int obj) const {
	checkRange(_numGlobalObjects - 1, 0, obj, "Object %d out of range in getObjectRoom");
	return _objectRoomTable[obj];
}

int ScummEngine::getObjectIndex(int object) const {
	int i;

	if (object < 1)
		return -1;

	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object)
			return i;
	}
	return -1;
}

int ScummEngine::whereIsObject(int object) const {
	int i;

	if (object >= _numGlobalObjects)
		return WIO_NOT_FOUND;

	if (object < 1)
		return WIO_NOT_FOUND;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM) {
		for (i = 0; i < _numInventory; i++)
			if (_inventory[i] == object)
				return WIO_INVENTORY;
		return WIO_NOT_FOUND;
	}

	for (i = (_numLocalObjects-1); i > 0; i--)
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return WIO_FLOBJECT;
			return WIO_ROOM;
		}

	return WIO_NOT_FOUND;
}

int ScummEngine::getObjectOrActorXY(int object, int &x, int &y) {
	if (object < _numActors) {
		Actor *act = derefActorSafe(object, "getObjectOrActorXY");
		if (act)
			return act->getActorXYPos(x, y);
		else
			return -1;
	}

	switch (whereIsObject(object)) {
	case WIO_NOT_FOUND:
		return -1;
	case WIO_INVENTORY:
		if (_objectOwnerTable[object] < _numActors)
			return derefActor(_objectOwnerTable[object], "getObjectOrActorXY(2)")->getActorXYPos(x, y);
		else
			return -1;
	}
	getObjectXYPos(object, x, y);
	return 0;
}

/**
 * Return the position of an object.
 * Returns X, Y and direction in angles
 */
void ScummEngine::getObjectXYPos(int object, int &x, int &y, int &dir) {
	int idx = getObjectIndex(object);
	assert(idx >= 0);
	ObjectData &od = _objs[idx];
	int state;
	const byte *ptr;
	const ImageHeader *imhd;

	if (_version >= 6) {
		state = getState(object) - 1;
		if (state < 0)
			state = 0;

		ptr = getOBIMFromObject(od);
		if (!ptr) {
			// FIXME: We used to assert here, but it seems that in the nexus
			// in The Dig, this can happen, at least with old savegames, and
			// it's safe to continue...
			warning("getObjectXYPos: Can't find object %d", object);
			return;
		}
		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), ptr);
		assert(imhd);
		if (_version == 8) {
			x = od.x_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].x);
			y = od.y_pos + (int32)READ_LE_UINT32(&imhd->v8.hotspot[state].y);
		} else if (_version == 7) {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].y);
		} else {
			x = od.x_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].x);
			y = od.y_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].y);
		}
	} else {
		x = od.walk_x;
		y = od.walk_y;
	}
	if (_version == 8)
		dir = fromSimpleDir(1, od.actordir);
	else
		dir = oldDirToNewDir(od.actordir & 3);
}

static int getDist(int x, int y, int x2, int y2) {
	int a = ABS(y - y2);
	int b = ABS(x - x2);
	return MAX(a, b);
}

int ScummEngine::getObjActToObjActDist(int a, int b) {
	int x, y, x2, y2;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (a < _numActors)
		acta = derefActorSafe(a, "getObjActToObjActDist");

	if (b < _numActors)
		actb = derefActorSafe(b, "getObjActToObjActDist(2)");

	if (acta && actb && acta->getRoom() == actb->getRoom() && acta->getRoom() && !acta->isInCurrentRoom())
		return 0;

	if (getObjectOrActorXY(a, x, y) == -1)
		return 0xFF;

	if (getObjectOrActorXY(b, x2, y2) == -1)
		return 0xFF;

	// Perform adjustXYToBeInBox() *only* if the first item is an
	// actor and the second is an object. This used to not check
	// whether the second item is a non-actor, which caused bug
	// #853874).
	if (acta && !actb) {
		AdjustBoxResult r = acta->adjustXYToBeInBox(x2, y2);
		x2 = r.x;
		y2 = r.y;
	}

	// Now compute the distance between the two points
	if (_version <= 2) {
		// For V1/V2 games, distances are measured in the original "character"
		// based coordinate system, instead of pixels. Otherwise various scripts
		// will break. See bugs #853874, #774529
		x /= 8;
		y /= 2;
		x2 /= 8;
		y2 /= 2;
	}

	return getDist(x, y, x2, y2);
}

int ScummEngine::findObject(int x, int y) {
	int i, b;
	byte a;
	const int mask = (_version <= 2) ? 0x8 : 0xF;

	for (i = 1; i < _numLocalObjects; i++) {
		if ((_objs[i].obj_nr < 1) || getClass(_objs[i].obj_nr, kObjectClassUntouchable))
			continue;
		if ((_version <= 2) && _objs[i].state & 0x2)
			continue;
		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b == 0) {
				if (_objs[i].x_pos <= x && _objs[i].width + _objs[i].x_pos > x &&
				    _objs[i].y_pos <= y && _objs[i].height + _objs[i].y_pos > y)
					return _objs[i].obj_nr;
				break;
			}
		} while ((_objs[b].state & mask) == a);
	}
	return 0;
}

void ScummEngine::drawRoomObject(int i, int arg) {
	ObjectData *od;
	byte a;
	const int mask = (_version <= 2) ? 0x8 : 0xF;

	od = &_objs[i];
	if ((i < 1) || (od->obj_nr < 1) || !od->state)
		return;

	do {
		a = od->parentstate;
		if (!od->parent) {
			drawObject(i, arg);
			break;
		}
		od = &_objs[od->parent];
	} while ((od->state & mask) == a);
}

void ScummEngine::drawRoomObjects(int arg) {
	int i;
	const int mask = (_version <= 2) ? 0x8 : 0xF;

	if (_features & GF_DRAWOBJ_OTHER_ORDER) {
		for (i = 1; i < _numLocalObjects; i++)
			if (_objs[i].obj_nr > 0)
				drawRoomObject(i, arg);
	} else {
		for (i = (_numLocalObjects-1); i > 0; i--)
			if (_objs[i].obj_nr > 0 && (_objs[i].state & mask)) {
				drawRoomObject(i, arg);
			}
	}
}

static const uint32 IMxx_tags[] = {
	MKID('IM00'),
	MKID('IM01'),
	MKID('IM02'),
	MKID('IM03'),
	MKID('IM04'),
	MKID('IM05'),
	MKID('IM06'),
	MKID('IM07'),
	MKID('IM08'),
	MKID('IM09'),
	MKID('IM0A'),
	MKID('IM0B'),
	MKID('IM0C'),
	MKID('IM0D'),
	MKID('IM0E'),
	MKID('IM0F')
};

void ScummEngine::drawObject(int obj, int arg) {
	ObjectData &od = _objs[obj];
	int xpos, ypos, height, width;
	const byte *ptr;
	int x, a, numstrip;
	int tmp;

	if (_BgNeedsRedraw)
		arg = 0;

	if (od.obj_nr == 0)
		return;

	checkRange(_numGlobalObjects - 1, 0, od.obj_nr, "Object %d out of range in drawObject");

	xpos = od.x_pos / 8;
	ypos = od.y_pos;

	width = od.width / 8;
	height = od.height &= 0xFFFFFFF8;	// Mask out last 3 bits

	// Short circuit for objects which aren't visible at all.
	if (width == 0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;

	ptr = getOBIMFromObject(od);

	if (_features & GF_OLD_BUNDLE)
		ptr += 0;
	else
		ptr = getObjectImage(ptr, getState(od.obj_nr));

	if (!ptr)
		return;

	x = 0xFFFF;

	for (a = numstrip = 0; a < width; a++) {
		tmp = xpos + a;
		if (arg == 1 && _screenStartStrip != tmp)
			continue;
		if (arg == 2 && _screenEndStrip != tmp)
			continue;
		if (tmp < _screenStartStrip || tmp > _screenEndStrip)
			continue;
		setGfxUsageBit(tmp, USAGE_BIT_DIRTY);
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip != 0) {
		byte flags = Gdi::dbAllowMaskOr;
		if (_version == 1) {
			gdi._C64ObjectMode = true;
			gdi.decodeC64Gfx(ptr, gdi._C64ObjectMap, width * (height / 8) * 3);
		}
		// Sam & Max needs this to fix object-layering problems with
		// the inventory and conversation icons.
		if ((_version >= 7 || _gameId == GID_SAMNMAX) && getClass(od.obj_nr, kObjectClassIgnoreBoxes))
			flags |= Gdi::dbDrawMaskOnAll;
		gdi.drawBitmap(ptr, &virtscr[0], x, ypos, width * 8, height, x - xpos, numstrip, flags);
	}
}

void ScummEngine::clearRoomObjects() {
	int i;

	if (_features & GF_SMALL_HEADER) {
		for (i = 0; i < _numLocalObjects; i++) {
			_objs[i].obj_nr = 0;
		}
	} else {
		// FIXME: Locking/FlObjects stuff?
		for (i = 0; i < _numLocalObjects; i++) {
			if (_objs[i].obj_nr < 1)	// Optimise codepath
				continue;

			// Nuke all non-flObjects (flObjects are nuked in script.cpp)
			if (_objs[i].fl_object_index == 0) {
				_objs[i].obj_nr = 0;
			} else {
				// Nuke all unlocked flObjects
				if (!(res.flags[rtFlObject][_objs[i].fl_object_index] & RF_LOCK)) {
					nukeResource(rtFlObject, _objs[i].fl_object_index);
					_objs[i].obj_nr = 0;
					_objs[i].fl_object_index = 0;
				}
			}
		}
	}
}

void ScummEngine::loadRoomObjects() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room, *searchptr, *rootptr;
	const ImageHeader *imhd;
	const RoomHeader *roomhdr;
	const CodeHeader *cdhd;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), room);

	if (_version == 8)
		_numObjectsInRoom = (byte)READ_LE_UINT32(&(roomhdr->v8.numObjects));
	else if (_version == 7)
		_numObjectsInRoom = (byte) READ_LE_UINT16(&(roomhdr->v7.numObjects));
	else
		_numObjectsInRoom = (byte) READ_LE_UINT16(&(roomhdr->old.numObjects));

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_version == 8)
		searchptr = rootptr = getResourceAddress(rtRoomScripts, _roomResource);
	else
		searchptr = rootptr = room;
	assert(searchptr);

	// Load in new room objects
	ResourceIterator	obcds(searchptr, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKID('OBCD'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - rootptr;
		cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), ptr);

		if (_version >= 7)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
		else if (_version == 6)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
		else
			od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			ptr = findResource(MKID('VERB'), ptr);
			dumpResource(buf, od->obj_nr, ptr);
		}

	}

	searchptr = room;
	ResourceIterator	obims(room, false);
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = obims.findNext(MKID('OBIM'));
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), ptr);
		if (_version == 8)
			// In V8, IMHD has no obj_id, but rather a name string. We map the name
			// back to an object id using a table derived from the DOBJ resource.
			obim_id = _objectIDMap[imhd->v8.name];
		else if (_version == 7)
			obim_id = READ_LE_UINT16(&imhd->v7.obj_id);
		else
			obim_id = READ_LE_UINT16(&imhd->old.obj_id);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index)
			setupRoomObject(&_objs[i], room);
	}

	CHECK_HEAP
}

void ScummEngine::loadRoomObjectsOldBundle() {
	int i;
	ObjectData *od;
	const byte *room, *ptr;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);

	_numObjectsInRoom = room[20];

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	if (_version <= 2)
		ptr = room + 28;
	else
		ptr = room + 29;

	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		od->OBIMoffset = READ_LE_UINT16(ptr);
		od->OBCDoffset = READ_LE_UINT16(ptr + 2 * _numObjectsInRoom);
		setupRoomObject(od, room);

		ptr += 2;

		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, room + od->OBCDoffset);
		}
	}

	CHECK_HEAP
}

void ScummEngine::loadRoomObjectsSmall() {
	int i, j;
	ObjectData *od;
	const byte *ptr;
	uint16 obim_id;
	const byte *room;
	const RoomHeader *roomhdr;

	CHECK_HEAP
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), room);

	_numObjectsInRoom = (byte) READ_LE_UINT16(&(roomhdr->old.numObjects));

	if (_numObjectsInRoom == 0)
		return;

	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	ResourceIterator	obcds(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		od = &_objs[findLocalObjectSlot()];

		ptr = obcds.findNext(MKID('OBCD'));
		if (ptr == NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->OBCDoffset = ptr - room;
		od->obj_nr = READ_LE_UINT16(ptr + 6);
		if (_dumpScripts) {
			char buf[32];
			sprintf(buf, "roomobj-%d-", _roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		}
	}

	ResourceIterator	obims(room, true);
	for (i = 0; i < _numObjectsInRoom; i++) {
		ptr = obims.findNext(MKID('OBIM'));
		if (ptr == NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		obim_id = READ_LE_UINT16(ptr + 6);

		for (j = 1; j < _numLocalObjects; j++) {
			if (_objs[j].obj_nr == obim_id)
				_objs[j].OBIMoffset = ptr - room;
		}
	}

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr && !_objs[i].fl_object_index) {
			setupRoomObject(&_objs[i], room);
		}
	}

	CHECK_HEAP
}

void ScummEngine::setupRoomObject(ObjectData *od, const byte *room, const byte *searchptr) {
	const CodeHeader *cdhd = NULL;
	const ImageHeader *imhd = NULL;

	assert(room);

	if (_features & GF_SMALL_HEADER) {

		const byte *ptr = room + od->OBCDoffset;

		if (_features & GF_OLD_BUNDLE)
			ptr -= 2;

		od->obj_nr = READ_LE_UINT16(ptr + 6);

		od->x_pos = *(ptr + 9) * 8;
		od->y_pos = ((*(ptr + 10)) & 0x7F) * 8;

		od->parentstate = (*(ptr + 10) & 0x80) ? 1 : 0;
		if (_version <= 2)
			od->parentstate *= 8;

		od->width = *(ptr + 11) * 8;

		od->parent = *(ptr + 12);

		if (_version <= 2) {
			od->walk_x = *(ptr + 13) * 8;
			od->walk_y = (*(ptr + 14) & 0x1f) * 8;
			od->actordir = (*(ptr + 15)) & 7;
			od->height = *(ptr + 15) & 0xf8;
		} else {
			od->walk_x = READ_LE_UINT16(ptr + 13);
			od->walk_y = READ_LE_UINT16(ptr + 15);
			od->actordir = (*(ptr + 17)) & 7;
			od->height = *(ptr + 17) & 0xf8;
		}

		return;
	}

	if (searchptr == NULL) {
		if (_version == 8)
			searchptr = getResourceAddress(rtRoomScripts, _roomResource);
		else
			searchptr = room;
	}
		
	cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), searchptr + od->OBCDoffset);
	if (cdhd == NULL)
		error("Room %d missing CDHD blocks(s)", _roomResource);

	if (_version == 8) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), room + od->OBIMoffset);
		od->x_pos = (int)READ_LE_UINT32(&imhd->v8.x_pos);
		od->y_pos = (int)READ_LE_UINT32(&imhd->v8.y_pos);
		od->width = (uint)READ_LE_UINT32(&imhd->v8.width);
		od->height = (uint)READ_LE_UINT32(&imhd->v8.height);
		// HACK: This is done sinec an angle doesn't fit into a byte (360 > 256)
		od->actordir = toSimpleDir(1, READ_LE_UINT32(&imhd->v8.actordir));

	} else if (_version == 7) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));

		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), room + od->OBIMoffset);
		od->x_pos = READ_LE_UINT16(&imhd->v7.x_pos);
		od->y_pos = READ_LE_UINT16(&imhd->v7.y_pos);
		od->width = READ_LE_UINT16(&imhd->v7.width);
		od->height = READ_LE_UINT16(&imhd->v7.height);
		od->actordir = (byte) READ_LE_UINT16(&imhd->v7.actordir);

	} else if (_version == 6) {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));

		od->width = READ_LE_UINT16(&cdhd->v6.w);
		od->height = READ_LE_UINT16(&cdhd->v6.h);
		od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x));
		od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y));
		if (cdhd->v6.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v6.flags & 0xF);
		}
		od->parent = cdhd->v6.parent;
		od->actordir = cdhd->v6.actordir;
	} else {
		od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

		od->width = cdhd->v5.w * 8;
		od->height = cdhd->v5.h * 8;
		od->x_pos = cdhd->v5.x * 8;
		od->y_pos = cdhd->v5.y * 8;
		if (cdhd->v5.flags == 0x80) {
			od->parentstate = 1;
		} else {
			od->parentstate = (cdhd->v5.flags & 0xF);
		}
		od->parent = cdhd->v5.parent;
		od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
		od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
		od->actordir = cdhd->v5.actordir;
	}

	od->fl_object_index = 0;
}

void ScummEngine::fixObjectFlags() {
	int i;
	ObjectData *od = &_objs[1];
	for (i = 1; i < _numLocalObjects; i++, od++) {
		if (od->obj_nr > 0)
			od->state = getState(od->obj_nr);
	}
}

void ScummEngine::processDrawQue() {
	int i, j;
	for (i = 0; i < _drawObjectQueNr; i++) {
		j = _drawObjectQue[i];
		if (j)
			drawObject(j, 0);
	}
	_drawObjectQueNr = 0;
}

void ScummEngine::addObjectToDrawQue(int object) {
	if ((unsigned int)_drawObjectQueNr >= ARRAYSIZE(_drawObjectQue))
		error("Draw Object Que overflow");
	_drawObjectQue[_drawObjectQueNr++] = object;
}

void ScummEngine::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

void ScummEngine::clearOwnerOf(int obj) {
	int i, j;
	uint16 *a;

	stopObjectScript(obj);

	if (getOwner(obj) == OF_OWNER_ROOM) {
		i = 0;
		do {
			if (_objs[i].obj_nr == obj) {
				if (!_objs[i].fl_object_index)
					return;
				nukeResource(rtFlObject, _objs[i].fl_object_index);
				_objs[i].obj_nr = 0;
				_objs[i].fl_object_index = 0;
			}
		} while (++i < _numLocalObjects);
		return;
	}

	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == obj) {
			j = whereIsObject(obj);
			if (j == WIO_INVENTORY) {
				nukeResource(rtInventory, i);
				_inventory[i] = 0;
			}
			a = _inventory;
			for (i = 0; i < _numInventory - 1; i++, a++) {
				if (!a[0] && a[1]) {
					a[0] = a[1];
					a[1] = 0;
					_baseInventoryItems[i] = _baseInventoryItems[i + 1];
					_baseInventoryItems[i + 1] = NULL;
				}
			}
			return;
		}
	}
}

/**
 * Mark the rectangle covered by the given object as dirty, thus eventually
 * ensuring a redraw of that area. This function is typically invoked when an
 * object gets removed from the current room, or when its state changed.
 */
void ScummEngine::markObjectRectAsDirty(int obj) {
	int i, strip;

	for (i = 1; i < _numLocalObjects; i++) {
		if (_objs[i].obj_nr == (uint16)obj) {
			if (_objs[i].width != 0) {
				const int minStrip = MAX(_screenStartStrip, _objs[i].x_pos / 8);
				const int maxStrip = MIN(_screenEndStrip+1, _objs[i].x_pos / 8 + _objs[i].width / 8);
				for (strip = minStrip; strip < maxStrip; strip++) {
					setGfxUsageBit(strip, USAGE_BIT_DIRTY);
				}
			}
			_BgNeedsRedraw = true;
			return;
		}
	}
}

const byte *ScummEngine::getObjOrActorName(int obj) {
	byte *objptr;
	int i;

	if (obj < _numActors)
		return derefActor(obj, "getObjOrActorName")->getActorName();

	if (_version >= 6) {
		for (i = 0; i < _numNewNames; i++) {
			if (_newNames[i] == obj) {
				debug(5, "Found new name for object %d at _newNames[%d]", obj, i);
				return getResourceAddress(rtObjectName, i);
				break;
			}
		}
	}

	objptr = getOBCDFromObject(obj);
	if (objptr == NULL)
		return NULL;

	if (_features & GF_SMALL_HEADER) {
		byte offset = 0;

		if (_version <= 2)
			offset = *(objptr + 14);
		else if (_features & GF_OLD_BUNDLE)
			offset = *(objptr + 16);
		else
			offset = *(objptr + 18);

		return (objptr + offset);
	}

	return findResourceData(MKID('OBNA'), objptr);
}

uint32 ScummEngine::getOBCDOffs(int object) const {
	int i;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM)
		return 0;
	for (i = (_numLocalObjects-1); i > 0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index != 0)
				return 8;
			return _objs[i].OBCDoffset;
		}
	}
	return 0;
}

byte *ScummEngine::getOBCDFromObject(int obj) {
	int i;
	byte *ptr;

	if (_objectOwnerTable[obj] != OF_OWNER_ROOM) {
		for (i = 0; i < _numInventory; i++) {
			if (_inventory[i] == obj)
				return getResourceAddress(rtInventory, i);
		}
	} else {
		for (i = (_numLocalObjects-1); i > 0; --i) {
			if (_objs[i].obj_nr == obj) {
				if (_objs[i].fl_object_index) {
					assert(_objs[i].OBCDoffset == 8);
					ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index);
				} else if (_version == 8)
					ptr = getResourceAddress(rtRoomScripts, _roomResource);
				else
					ptr = getResourceAddress(rtRoom, _roomResource);
				assert(ptr);
				return ptr + _objs[i].OBCDoffset;
			}
		}
	}
	return 0;
}

const byte *ScummEngine::getOBIMFromObject(const ObjectData &od) {
	const byte *ptr;

	if (od.fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od.fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr);
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		if (ptr)
			ptr += od.OBIMoffset;
	}
	return ptr;
}

const byte *ScummEngine::getObjectImage(const byte *ptr, int state) {
	assert(ptr);
	if (_features & GF_OLD_BUNDLE)
		ptr += 0;
	else if (_features & GF_SMALL_HEADER) {
		ptr += 8;
	} else if (_version == 8) {
		// The OBIM contains an IMAG, which in turn contains a WRAP, which contains
		// an OFFS chunk and multiple BOMP/SMAP chunks. To find the right BOMP/SMAP,
		// we use the offsets in the OFFS chunk,
		ptr = findResource(MKID('IMAG'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKID('WRAP'), ptr);
		if (!ptr)
			return 0;

		ptr = findResource(MKID('OFFS'), ptr);
		if (!ptr)
			return 0;

		// Get the address of the specified SMAP (corresponding to IMxx)
		ptr += READ_LE_UINT32(ptr + 4 + 4*state);
	} else {
		ptr = findResource(IMxx_tags[state], ptr);
	}
	
	return ptr;
}

void ScummEngine::addObjectToInventory(uint obj, uint room) {
	int idx, slot;
	uint32 size;
	const byte *ptr;
	byte *dst;
	FindObjectInRoom foir;

	debug(1, "Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP
	if (whereIsObject(obj) == WIO_FLOBJECT) {
		idx = getObjectIndex(obj);
		assert(idx >= 0);
		ptr = getResourceAddress(rtFlObject, _objs[idx].fl_object_index) + 8;
		size = READ_BE_UINT32(ptr + 4);
	} else {
		findObjectInRoom(&foir, foCodeHeader, obj, room);
		if (_features & GF_OLD_BUNDLE)
			size = READ_LE_UINT16(foir.obcd);
		else if (_features & GF_SMALL_HEADER)
			size = READ_LE_UINT32(foir.obcd);
		else
			size = READ_BE_UINT32(foir.obcd + 4);
		ptr = foir.obcd;
	}

	slot = getInventorySlot();
	_inventory[slot] = obj;
	dst = createResource(rtInventory, slot, size);
	assert(dst);
	memcpy(dst, ptr, size);

	CHECK_HEAP
}

void ScummEngine::findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint id, uint room) {

	const CodeHeader *cdhd;
	int i, numobj;
	const byte *roomptr, *obcdptr, *obimptr, *searchptr;
	const ImageHeader *imhd;
	int id2;
	int id3;

	if (findWhat & foCheckAlreadyLoaded && getObjectIndex(id) != -1) {
		if (_features & GF_OLD_BUNDLE) {
			// I am not sure if this is even needed for old games...
			// but using RES_SIZE definitely won't work with OLD_BUNDLE, since it
			// assumes the size is 32 bit but in old games it's 16 bit
			error("findObjectInRoom foCheckAlreadyLoaded NYI for GF_OLD_BUNDLE (id = %d, room = %d)", id, room);
		}
		fo->obcd = obcdptr = getOBCDFromObject(id);
		assert(obcdptr);
		fo->obim = obimptr = obcdptr + RES_SIZE(obcdptr);
		fo->cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), obcdptr);
		fo->imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obimptr);
		return;
	}

	fo->roomptr = roomptr = getResourceAddress(rtRoom, room);
	if (!roomptr)
		error("findObjectInRoom: failed getting roomptr to %d", room);

	if (_features & GF_OLD_BUNDLE) {
		numobj = roomptr[20];
	} else {
		const RoomHeader *roomhdr = (const RoomHeader *)findResourceData(MKID('RMHD'), roomptr);
	
		if (_version == 8)
			numobj = READ_LE_UINT32(&(roomhdr->v8.numObjects));
		else if (_version == 7)
			numobj = READ_LE_UINT16(&(roomhdr->v7.numObjects));
		else
			numobj = READ_LE_UINT16(&(roomhdr->old.numObjects));
	}
	
	if (numobj == 0)
		error("findObjectInRoom: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("findObjectInRoom: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	if (_features & GF_OLD_BUNDLE) {
		if (_version <= 2)
			searchptr = roomptr + 28;
		else
			searchptr = roomptr + 29;

		for (i = 0; i < numobj; i++) {
			obimptr = roomptr + READ_LE_UINT16(searchptr);
			obcdptr = roomptr + READ_LE_UINT16(searchptr + 2 * _numObjectsInRoom);
			id2 = READ_LE_UINT16(obcdptr + 4);

			if (id2 == (uint16)id) {
				if (findWhat & foCodeHeader) {
					fo->obcd = obcdptr;
					fo->cdhd = (const CodeHeader *)(obcdptr + 10);	// TODO - FIXME
				}
				if (findWhat & foImageHeader) {
					fo->obim = obimptr;
					fo->imhd = NULL;
				}
				break;
			}
			searchptr += 2;
		}
		return;
	}

	if (findWhat & foCodeHeader) {
		if (_version == 8)
			searchptr = getResourceAddress(rtRoomScripts, room);
		else
			searchptr = roomptr;
		assert(searchptr);
		ResourceIterator	obcds(searchptr, (_features & GF_SMALL_HEADER) != 0);
		for (i = 0;;) {
			obcdptr = obcds.findNext(MKID('OBCD'));
			if (obcdptr == NULL)
				error("findObjectInRoom: Not enough code blocks in room %d", room);
			cdhd = (const CodeHeader *)findResourceData(MKID('CDHD'), obcdptr);

			if (_features & GF_SMALL_HEADER)
				id2 = READ_LE_UINT16(obcdptr + 6);
			else if (_version >= 7)
				id2 = READ_LE_UINT16(&(cdhd->v7.obj_id));
			else if (_version == 6)
				id2 = READ_LE_UINT16(&(cdhd->v6.obj_id));
			else
				id2 = READ_LE_UINT16(&(cdhd->v5.obj_id));

			if (id2 == (uint16)id) {
				fo->obcd = obcdptr;
				fo->cdhd = cdhd;
				break;
			}
			if (++i == numobj)
				error("findObjectInRoom: Object %d not found in room %d", id, room);
		}
	}

	roomptr = fo->roomptr;
	if (findWhat & foImageHeader) {
		ResourceIterator	obims(roomptr, (_features & GF_SMALL_HEADER) != 0);
		for (i = 0;;) {
			obimptr = obims.findNext(MKID('OBIM'));
			if (obimptr == NULL)
				error("findObjectInRoom: Not enough image blocks in room %d", room);
			imhd = (const ImageHeader *)findResourceData(MKID('IMHD'), obimptr);
			if (_features & GF_SMALL_HEADER)
				id3 = READ_LE_UINT16(obimptr + 6);
			else if (_version == 8)
				// In V8, IMHD has no obj_id, but rather a name string. We map the name
				// back to an object id using a table derived from the DOBJ resource.
				id3 = _objectIDMap[imhd->v8.name];
			else if (_version == 7)
				id3 = READ_LE_UINT16(&imhd->v7.obj_id);
			else
				id3 = READ_LE_UINT16(&imhd->old.obj_id);

			if (id3 == (uint16)id) {
				fo->obim = obimptr;
				fo->imhd = imhd;
				break;
			}
			if (++i == numobj)
				error("findObjectInRoom: Object %d image not found in room %d", id, room);
		}
	}
}

int ScummEngine::getInventorySlot() {
	int i;
	for (i = 0; i < _numInventory; i++) {
		if (_inventory[i] == 0)
			return i;
	}
	error("Inventory full, %d max items", _numInventory);
	return -1;
}

void ScummEngine::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;
	if (owner == 0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->where == WIO_INVENTORY && _inventory[ss->number] == obj) {
			putOwner(obj, 0);
			runInventoryScript(0);
			stopObjectCode();
			return;
		}
	}

	putOwner(obj, owner);
	runInventoryScript(0);
}

int ScummEngine::getObjX(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjX")->_pos.x;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return x;
	}
}

int ScummEngine::getObjY(int obj) {
	if (obj < _numActors) {
		if (obj < 1)
			return 0;									/* fix for indy4's map */
		return derefActor(obj, "getObjY")->_pos.y;
	} else {
		if (whereIsObject(obj) == WIO_NOT_FOUND)
			return -1;
		int x, y;
		getObjectOrActorXY(obj, x, y);
		return y;
	}
}

int ScummEngine::getObjOldDir(int obj) {
	return newDirToOldDir(getObjNewDir(obj));
}

int ScummEngine::getObjNewDir(int obj) {
	int dir;
	if (obj < _numActors) {
		dir = derefActor(obj, "getObjNewDir")->getFacing();
	} else {
		int x, y;
		getObjectXYPos(obj, x, y, dir);
	}
	return dir;
}

int ScummEngine::findInventory(int owner, int idx) {
	int count = 1, i, obj;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner && count++ == idx)
			return obj;
	}
	return 0;
}

int ScummEngine::getInventoryCount(int owner) {
	int i, obj;
	int count = 0;
	for (i = 0; i < _numInventory; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner)
			count++;
	}
	return count;
}

void ScummEngine::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i == -1) {
		warning("setObjectState: no such object %d", obj);
		return;
	}

	if (x != -1) {
		_objs[i].x_pos = x * 8;
		_objs[i].y_pos = y * 8;
	}

	addObjectToDrawQue(i);
	putState(obj, state);
}

int ScummEngine::getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f) {
	int i, j;
	int x, y;
	int x2, y2;

	j = i = 0xFF;

	if (is_obj_1) {
		if (getObjectOrActorXY(b, x, y) == -1)
			return -1;
		if (b < _numActors)
			i = derefActor(b, "unkObjProc1")->scalex;
	} else {
		x = b;
		y = c;
	}

	if (is_obj_2) {
		if (getObjectOrActorXY(e, x2, y2) == -1)
			return -1;
		if (e < _numActors)
			j = derefActor(e, "unkObjProc1(2)")->scalex;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x, y, x2, y2) * 0xFF / ((i + j) / 2);
}

void ScummEngine::setCursorImg(uint img, uint room, uint imgindex) {
	int w, h;
	const byte *dataptr, *bomp;
	uint32 size;
	FindObjectInRoom foir;

	if (room == (uint) - 1)
		room = getObjectRoom(img);

	findObjectInRoom(&foir, foCodeHeader | foImageHeader | foCheckAlreadyLoaded, img, room);

	if (_version == 8) {
		setCursorHotspot(READ_LE_UINT32(&foir.imhd->v8.hotspot[0].x),
		                  READ_LE_UINT32(&foir.imhd->v8.hotspot[0].y));
		w = READ_LE_UINT32(&foir.imhd->v8.width) / 8;
		h = READ_LE_UINT32(&foir.imhd->v8.height) / 8;
	} else if (_version == 7) {
		setCursorHotspot(READ_LE_UINT16(&foir.imhd->v7.hotspot[0].x),
		                  READ_LE_UINT16(&foir.imhd->v7.hotspot[0].y));
		w = READ_LE_UINT16(&foir.imhd->v7.width) / 8;
		h = READ_LE_UINT16(&foir.imhd->v7.height) / 8;
	} else {
		setCursorHotspot(READ_LE_UINT16(&foir.imhd->old.hotspot[0].x),
		                  READ_LE_UINT16(&foir.imhd->old.hotspot[0].y));
		w = READ_LE_UINT16(&foir.cdhd->v6.w) / 8;
		h = READ_LE_UINT16(&foir.cdhd->v6.h) / 8;
	}

	dataptr = getObjectImage(foir.obim, imgindex);
	assert(dataptr);
	if (_version == 8) {
		bomp = dataptr;
	} else {
		size = READ_BE_UINT32(dataptr + 4);
		if (size > sizeof(_grabbedCursor))
			error("setCursorImg: Cursor image too large");
		
		bomp = findResource(MKID('BOMP'), dataptr);
	}

	if (bomp != NULL)
		useBompCursor(bomp, w, h);
	else
		useIm01Cursor(dataptr, w, h);

}

void ScummEngine::nukeFlObjects(int min, int max) {
	ObjectData *od;
	int i;

	warning("nukeFlObjects(%d,%d)", min, max);

	for (i = (_numLocalObjects-1), od = _objs; --i >= 0; od++)
		if (od->fl_object_index && od->obj_nr >= min && od->obj_nr <= max) {
			nukeResource(rtFlObject, od->fl_object_index);
			od->obj_nr = 0;
			od->fl_object_index = 0;
		}
}

void ScummEngine::enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
								int objectHeight, int scaleX, int scaleY, int image, int mode) {
	BlastObject *eo;
	ObjectData *od;

	if (_blastObjectQueuePos >= (int)ARRAYSIZE(_blastObjectQueue)) {
		warning("enqueueObject: overflow");
		return;
	}
	
	int idx = getObjectIndex(objectNumber);
	assert(idx >= 0);

	eo = &_blastObjectQueue[_blastObjectQueuePos++];
	eo->number = objectNumber;
	eo->rect.left = objectX + (camera._cur.x & 7);
	eo->rect.top = objectY + _screenTop;
	if (objectWidth == 0) {
		od = &_objs[idx];
		eo->rect.right = eo->rect.left + od->width;
	} else {
		eo->rect.right = eo->rect.left + objectWidth;
	}
	if (objectHeight == 0) {
		od = &_objs[idx];
		eo->rect.bottom = eo->rect.top + od->height;
	} else {
		eo->rect.bottom = eo->rect.top + objectHeight;
	}

	eo->scaleX = scaleX;
	eo->scaleY = scaleY;
	eo->image = image;

	eo->mode = mode;
}

void ScummEngine::drawBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		drawBlastObject(eo);
	}
}

void ScummEngine::drawBlastObject(BlastObject *eo) {
	VirtScreen *vs;
	const byte *bomp, *ptr;
	int objnum;
	BompDrawData bdd;

	vs = &virtscr[0];

	checkRange(_numGlobalObjects - 1, 30, eo->number, "Illegal Blast object %d");

	objnum = getObjectIndex(eo->number);
	if (objnum == -1)
		error("drawBlastObject: getObjectIndex on BlastObject %d failed", eo->number);

	ptr = getOBIMFromObject(_objs[objnum]);
	if (!ptr)
		error("BlastObject object %d image not found", eo->number);

	const byte *img = getObjectImage(ptr, eo->image);
	if (_version == 8) {
		assert(img);
		bomp = img + 8;
	} else {
		if (!img)
			img = getObjectImage(ptr, 1);	// Backward compatibility with samnmax blast objects
		assert(img);
		bomp = findResourceData(MKID('BOMP'), img);
	}

	if (!bomp)
		error("object %d is not a blast object", eo->number);

	if (_version == 8) {
		bdd.srcwidth = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.width);
		bdd.srcheight = READ_LE_UINT32(&((const BompHeader *)bomp)->v8.height);
	} else {
		bdd.srcwidth = READ_LE_UINT16(&((const BompHeader *)bomp)->old.width);
		bdd.srcheight = READ_LE_UINT16(&((const BompHeader *)bomp)->old.height);
	}
	
	bdd.out = vs->screenPtr + vs->xstart;
	bdd.outwidth = vs->width;
	bdd.outheight = vs->height;
	if (_version == 8) {
		bdd.dataptr = bomp + 8;	// Why this? See also useBompCursor
	} else {
		bdd.dataptr = bomp + 10;	// Why this? See also useBompCursor
	}
	bdd.x = eo->rect.left;
	bdd.y = eo->rect.top;
	bdd.scale_x = (byte)eo->scaleX;
	bdd.scale_y = (byte)eo->scaleY;


	if ((bdd.scale_x != 255) || (bdd.scale_y != 255)) {
		byte bomp_scaling_x[64], bomp_scaling_y[64];
		bdd.scalingXPtr = bomp_scaling_x;
		bdd.scalingYPtr = bomp_scaling_y;
		bdd.scaleRight = setupBompScale(bomp_scaling_x, bdd.srcwidth, bdd.scale_x);
		bdd.scaleBottom = setupBompScale(bomp_scaling_y, bdd.srcheight, bdd.scale_y);
		bdd.shadowMode = 0;
		drawBomp(bdd, false);
	} else {
		bdd.scalingXPtr = NULL;
		bdd.scalingYPtr = NULL;
		bdd.scaleRight = 0;
		bdd.scaleBottom = 0;
		bdd.shadowMode = eo->mode;
		drawBomp(bdd, false);
	}

	markRectAsDirty(vs->number, bdd.x, bdd.x + bdd.srcwidth, bdd.y, bdd.y + bdd.srcheight);
}

void ScummEngine::removeBlastObjects() {
	BlastObject *eo;
	int i;

	eo = _blastObjectQueue;
	for (i = 0; i < _blastObjectQueuePos; i++, eo++) {
		removeBlastObject(eo);
	}

	clearEnqueue();
}

void ScummEngine::removeBlastObject(BlastObject *eo) {
	VirtScreen *vs = &virtscr[0];

	Common::Rect r;
	int left_strip, right_strip;
	int i;

	r = eo->rect;

	if (r.bottom < 0 || r.right < 0 || r.top > vs->height || r.left > vs->width)
		return;

	if (r.top < 0)
		r.top = 0;
	if (r.bottom > vs->height)
		r.bottom = vs->height;
	if (r.left < 0)
		r.left = 0;
	if (r.right > vs->width)
		r.right = vs->width;

	left_strip = r.left / 8;
	right_strip = (r.right - 1) / 8;

	if (left_strip < 0)
		left_strip = 0;
	if (_version >= 7) {
		if (right_strip > 409)
			right_strip = 409;
	} else {
		if (right_strip >= 200)
			right_strip = 200;
	}
	for (i = left_strip; i <= right_strip; i++)
		gdi.resetBackground(r.top, r.bottom, i);

	markRectAsDirty(kMainVirtScreen, r, USAGE_BIT_RESTORED);
}

int ScummEngine::findLocalObjectSlot() {
	int i;

	for (i = 1; i < _numLocalObjects; i++) {
		if (!_objs[i].obj_nr) {
			memset(&_objs[i], 0, sizeof(_objs[i]));
			return i;
		}
	}

	return -1;
}

int ScummEngine::findFlObjectSlot() {
	int i;
	for (i = 1; i < _numFlObject; i++) {
		if (_baseFLObject[i] == NULL)
			return i;
	}
	error("findFlObjectSlot: Out of FLObject slots");
	return -1;
}

void ScummEngine::loadFlObject(uint object, uint room) {
	FindObjectInRoom foir;
	int slot, objslot;
	ObjectData *od;
	byte *flob;
	uint32 obcd_size, obim_size, flob_size;
	bool isRoomLocked, isRoomScriptsLocked;

	// Don't load an already loaded object
	if (whereIsObject(object) != WIO_NOT_FOUND)
		return;

	// Locate the object in the room resource
	findObjectInRoom(&foir, foImageHeader | foCodeHeader, object, room);

	// Add an entry for the new floating object in the local object table
	if (!(objslot = findLocalObjectSlot()))
		error("loadFlObject: Local Object Table overflow");

	od = &_objs[objslot];

	// Dump object script
	if (_dumpScripts) {
		char buf[32];
		const byte *ptr = foir.obcd;
		sprintf(buf, "roomobj-%d-", room);
		ptr = findResource(MKID('VERB'), ptr);
		dumpResource(buf, object, ptr);
	}

	// Setup sizes
	obcd_size = READ_BE_UINT32(foir.obcd + 4);
	od->OBCDoffset = 8;
	od->OBIMoffset = obcd_size + 8;
	obim_size = READ_BE_UINT32(foir.obim + 4);
	flob_size = obcd_size + obim_size + 8;

	// Lock room/roomScripts for the given room. They contains the OBCD/OBIM
	// data, and a call to createResource might expire them, hence we lock them.
	isRoomLocked = ((res.flags[rtRoom][room] & RF_LOCK) != 0);
	isRoomScriptsLocked = ((res.flags[rtRoomScripts][room] & RF_LOCK) != 0);
	if (!isRoomLocked)
		lock(rtRoom, room);
	if (_version == 8 && !isRoomScriptsLocked)
		lock(rtRoomScripts, room);

	// Allocate slot & memory for floating object
	slot = findFlObjectSlot();
	flob = createResource(rtFlObject, slot, flob_size);
	assert(flob);

	// Copy object code + object image to floating object
	((uint32 *)flob)[0] = MKID('FLOB');
	((uint32 *)flob)[1] = TO_BE_32(flob_size);

	memcpy(flob + 8, foir.obcd, obcd_size);
	memcpy(flob + 8 + obcd_size, foir.obim, obim_size);

	// Unlock room/roomScripts
	if (!isRoomLocked)
		unlock(rtRoom, room);
	if (_version == 8 && !isRoomScriptsLocked)
		unlock(rtRoomScripts, room);

	// Setup local object flags
	setupRoomObject(od, flob, flob);

	od->fl_object_index = slot;
}

} // End of namespace Scumm
