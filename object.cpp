/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
#include "scumm.h"

bool Scumm::getClass(int obj, int cls) {
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in getClass");

	cls &= 0x7F;
	checkRange(32,1,cls,"Class %d out of range in getClass");

	return (_classData[obj] & (1<<(cls-1))) != 0;
}

void Scumm::putClass(int obj, int cls, bool set) {
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in getClass");

	cls &= 0x7F;
	checkRange(32,1,cls,"Class %d out of range in getClass");

	if (set)
		_classData[obj] |= (1<<(cls-1));
	else
		_classData[obj] &= ~(1<<(cls-1));
}

int Scumm::getOwner(int obj) {
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in getOwner");
	return _objectFlagTable[obj]&0xF;
}

void Scumm::putOwner(int act, int owner) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in putOwner");
	checkRange(15, 0, owner, "Owner %d out of range in putOwner");
	_objectFlagTable[act] = (_objectFlagTable[act]&0xF0) | owner;
}

int Scumm::getState(int act) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in getState");
	return _objectFlagTable[act]>>4;
}

void Scumm::putState(int act, int state) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in putState");
	checkRange(15, 0, state, "State %d out of range in putState");
	_objectFlagTable[act] = (_objectFlagTable[act]&0x0F) | (state<<4);
}

int Scumm::getObjectIndex(int object) {
	int i;

	if ((_objectFlagTable[object]&0xF)!=0xF) {
		for (i=0; i<_maxInventoryItems; i++)
			if (_inventory[i] == object)
				return i;
		return -1;
	} else {
		for (i=_numObjectsInRoom; i>0; i--) {
			if (_objs[i].obj_nr==object)
				return i;
		}
		return -1;
	}
}

int Scumm::whereIsObject(int object) {
	int i;

	assert(object>=0 && object < _numGlobalObjects);

	if ((_objectFlagTable[object]&0xF)!=0xF) {
		for (i=0; i<_maxInventoryItems; i++)
			if (_inventory[i] == object)
				return 0;
		return -1;
	}

	for (i=_numObjectsInRoom; i>0; i--)
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return 4;
			return 1;
		}
	return -1;
}

int Scumm::getObjectOrActorXY(int object) {
	if (object <= _vars[VAR_NUM_ACTOR]) {
		return getActorXYPos(derefActorSafe(object, "getObjectOrActorXY"));
	}
	switch(whereIsObject(object)) {
	case -1:
		return -1;
	case 0:
		return getActorXYPos(derefActorSafe(_objectFlagTable[object]&0xF,"getObjectOrActorXY(2)"));
	}
	getObjectXYPos(object);
	return 0;
}

void Scumm::getObjectXYPos(int object) {
	ObjectData *od = &_objs[getObjectIndex(object)];
	int state;
	byte *ptr;
	ImageHeader *imhd;
	int x,y;
	AdjustBoxResult abr;

	if (_majorScummVersion==6) {
		state = getState(object)-1;
		if (state<0)
			state = 0;

		if (od->fl_object_index) {
			ptr = getResourceAddress(rtFlObject, od->fl_object_index);
			ptr = findResource(MKID('OBIM'), ptr, 0);
		} else {
			ptr = getResourceAddress(rtRoom, _roomResource);
			ptr += od->offs_obim_to_room;
		}

		imhd = (ImageHeader*)findResource(MKID('IMHD'), ptr, 0);
		x = od->x_pos*8 + (int16)READ_LE_UINT16(&imhd->hotspot[state].x);
		y = od->y_pos*8 + (int16)READ_LE_UINT16(&imhd->hotspot[state].y);
	} else {
		x = od->walk_x;
		y = od->walk_y;
	}

	abr = adjustXYToBeInBox(0, x, y);
	_xPos = abr.x;
	_yPos = abr.y;
	_dir = od->actordir&3;
}

int Scumm::getObjActToObjActDist(int a, int b) {
	int x,y;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (a<=_vars[VAR_NUM_ACTOR])
		acta = derefActorSafe(a, "getObjActToObjActDist");

	if (b<=_vars[VAR_NUM_ACTOR])
		actb = derefActorSafe(b, "getObjActToObjActDist(2)");

	if (acta && actb && acta->room==actb->room && acta->room &&
		acta->room != _currentRoom)
			return 0xFF;

	if (getObjectOrActorXY(a)==-1)
		return 0xFF;

	x = _xPos;
	y = _yPos;

	if (getObjectOrActorXY(b)==-1)
		return 0xFF;

	if (acta) {
		AdjustBoxResult r = adjustXYToBeInBox(acta, _xPos, _yPos);
		_xPos = r.x;
		_yPos = r.y;
	}

	y = abs(y-_yPos);
	x = abs(x-_xPos);

	if (y>x) x=y;
	return x;
}

int Scumm::findObject(int x, int y) {
	int i,a,b;

	for (i=1; i<=_numObjectsInRoom; i++) {
		if (!_objs[i].obj_nr || getClass(_objs[i].obj_nr, 32))
			continue;
		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b==0) {
				if (_objs[i].x_pos <= (x>>3) &&
						_objs[i].numstrips + _objs[i].x_pos > (x>>3) &&
						_objs[i].y_pos <= (y>>3) &&
						_objs[i].height + _objs[i].y_pos > (y>>3))
						return _objs[i].obj_nr;
				break;
			}
		} while ( (_objs[b].ownerstate&0xF0) == a);
	}
	return 0;
}

void Scumm::drawRoomObjects(int arg) {
	int num = _numObjectsInRoom;
	ObjectData *od;
	int a;

	if (num==0)
		return;

	do {
		od = &_objs[num];
		if (!od->obj_nr || !(od->ownerstate&0xF0))
			continue;
		
		do {
			a = od->parentstate;
			if (!od->parent) {
				drawObject(num, arg);
				break;
			}
			od = &_objs[od->parent];
		} while ((od->ownerstate & 0xF0)==a);

	} while (--num);
}

const uint32 state_tags[] = {
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

void Scumm::drawObject(int obj, int arg) {
	ObjectData *od;
	int xpos, ypos, height, width;
	byte *ptr;
	int x,a,numstrip;
	int tmp;

	if (_BgNeedsRedraw)
		arg = 0;

	_curVirtScreen = &virtscr[0];
	
	od = &_objs[obj];

	xpos = od->x_pos;
	ypos = od->y_pos;
	width = od->numstrips;
	height = od->height;

	if (width==0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;
	
	if (od->fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od->fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr, 0); 
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		ptr = ptr + od->offs_obim_to_room;
	}

	ptr = findResource(state_tags[getState(od->obj_nr)], ptr, 0);
	if (!ptr)
		return;

	x = 0xFFFF;

	for (a=numstrip=0; a<width; a++) {
		tmp = xpos + a;
		if (arg==1 && _screenStartStrip!=tmp)
			continue;
		if (arg==2 && _screenEndStrip!=tmp)
			continue;
		if (tmp < _screenStartStrip || tmp > _screenEndStrip)
			continue;
		actorDrawBits[tmp] |= 0x8000;
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip!=0)
		gdi.drawBitmap(ptr, _curVirtScreen, x, ypos<<3, height<<3, x-xpos, numstrip, true);

//	_drawBmpY = ypos << 3;
//	gdi._numLinesToProcess = height << 3;
//	_drawBmpX = x;
//	drawBmp(ptr, x - xpos, b, 1, "Object", od->obj_nr);
}

void Scumm::loadRoomObjects() {
	int i,j;
	ObjectData *od;
	byte *ptr;
	uint16 obim_id;
	byte *room;
	ImageHeader *imhd;
	RoomHeader *roomhdr;

	CodeHeader *cdhd;

	CHECK_HEAP
	
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (RoomHeader*)findResource(MKID('RMHD'), room, 0);

	_numObjectsInRoom = READ_LE_UINT16(&roomhdr->numObjects);
	
	if (_numObjectsInRoom == 0)
		return;
	
	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	od = &_objs[1];
	for (i=0; i<_numObjectsInRoom; i++,od++) {
		ptr = findResource(MKID('OBCD'), room, i);
		if (ptr==NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->offs_obcd_to_room = ptr - room;
		cdhd = (CodeHeader*)findResource(MKID('CDHD'), ptr, 0);
		od->obj_nr = READ_LE_UINT16(&cdhd->obj_id);

#ifdef DUMP_SCRIPTS
		do {
			char buf[32];
			sprintf(buf,"roomobj-%d-",_roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		} while (0);
#endif
	}

	for (i=0; i<_numObjectsInRoom; i++) {
		ptr = findResource(MKID('OBIM'), room, i);
		if (ptr==NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		imhd = (ImageHeader*)findResource(MKID('IMHD'), ptr, 0);
		obim_id = READ_LE_UINT16(&imhd->obj_id);

		for(j=1; j<=_numObjectsInRoom; j++) {
			if (_objs[j].obj_nr==obim_id)
				_objs[j].offs_obim_to_room = ptr - room;
		}
	}

	od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		ptr = room + od->offs_obcd_to_room;
		cdhd = (CodeHeader*)findResource(MKID('CDHD'), ptr,0);
		od->obj_nr = READ_LE_UINT16(&cdhd->obj_id);

		if (_majorScummVersion == 6) {
			od->numstrips = READ_LE_UINT16(&cdhd->v6.w)>>3;
			od->height = READ_LE_UINT16(&cdhd->v6.h)>>3;
			od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x))>>3;
			od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y))>>3;
			if (cdhd->v6.flags == 0x80) {
				od->parentstate = 1<<4;
			} else {
				od->parentstate = (cdhd->v6.flags&0xF)<<4;
			}
			od->parent = cdhd->v6.parent;
			od->actordir = cdhd->v6.actordir;
		} else {
			od->numstrips = cdhd->v5.w;
			od->height = cdhd->v5.h;
			od->x_pos = cdhd->v5.x;
			od->y_pos = cdhd->v5.y;
			if (cdhd->v5.flags == 0x80) {
				od->parentstate = 1<<4;
			} else {
				od->parentstate = (cdhd->v5.flags&0xF)<<4;
			}
			od->parent = cdhd->v5.parent;
			od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
			od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
			od->actordir = cdhd->v5.actordir;
		}
		od->fl_object_index = 0;
	}

	CHECK_HEAP
}

void Scumm::fixObjectFlags() {
	int i;
	ObjectData *od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		od->ownerstate = _objectFlagTable[od->obj_nr];
	}
}

void Scumm::processDrawQue() {
	int i, j;
	for (i=0; i<_drawObjectQueNr; i++) {
		j = _drawObjectQue[i];
		if (j)
			drawObject(j,0);
	}
	_drawObjectQueNr = 0;
}

void Scumm::clearOwnerOf(int obj) {
	int i,j;
	uint16 *a;
	byte *ptr;

	stopObjectScript(obj);
	
	if (getOwner(obj)==0xF) {
		i = 0;
		do {
			if (_objs[i].obj_nr==obj) {
				if (!_objs[i].fl_object_index)
					return;
				nukeResource(rtFlObject, _objs[i].fl_object_index);
				_objs[i].obj_nr = 0;
				_objs[i].fl_object_index = 0;
			}
		} while(++i <= _numObjectsInRoom);
		return;
	}
	for (i=1; i<_maxInventoryItems; i++) {
		if (_inventory[i] == obj) {
			j = whereIsObject(obj);
			if (j==0) {
				nukeResource(rtInventory, i);
				_inventory[i] = 0;
			}
			a = &_inventory[1];
			for (i=1; i < _maxInventoryItems-1; i++,a++) {
				if (!a[0] && a[1]) {
					a[0] = a[1];
					a[1] = 0;
					ptr = getResourceAddress(rtInventory, i+1);
					_baseInventoryItems[i] = _baseInventoryItems[i+1];
					_baseInventoryItems[i+1] = 0;
					/* TODO: some wacky write is done here */
				}
			}
			return;
		}
	}
}

void Scumm::removeObjectFromRoom(int obj) {
	int i,cnt;
	uint16 *ptr;
	
	for(i=1; i<=_numObjectsInRoom; i++) {
		if (_objs[i].obj_nr==obj) {
			if (_objs[i].numstrips != 0) {
				ptr = &actorDrawBits[_objs[i].x_pos];
				cnt = _objs[i].numstrips;
				do {
					*ptr++ |= 0x8000;
				} while (--cnt);
			}
			_BgNeedsRedraw = 1;
			return;
		}
	}
}

void Scumm::addObjectToDrawQue(int object) {
	_drawObjectQue[_drawObjectQueNr++] = object;
	if (_drawObjectQueNr > sizeof(_drawObjectQue)/sizeof(_drawObjectQue[0]))
		error("Draw Object Que overflow");
}

void Scumm::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

byte *Scumm::getObjOrActorName(int obj) {
	byte *objptr;

	if (obj <= _vars[VAR_NUM_ACTOR])
		return getActorName(derefActorSafe(obj, "getObjOrActorName"));

	objptr = getObjectAddress(obj);
	if (objptr==NULL)
		return (byte*)" ";
	
	return findResource(MKID('OBNA'), objptr, 0) + 8;
}

uint32 Scumm::getOBCDOffs(int object) {
	int i;

	if ((_objectFlagTable[object]&0xF)!=0xF)
		return 0;
	for (i=_numObjectsInRoom; i>0; i--) {
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index!=0)
				return 8;
			return _objs[i].offs_obcd_to_room;
		}
	}
	return 0;
}

byte *Scumm::getObjectAddress(int obj) {
	int i;

	if ((_objectFlagTable[obj]&0xF)!=0xF) {
		for(i=0; i<_maxInventoryItems; i++) {
			if (_inventory[i] == obj)
				return getResourceAddress(rtInventory, i);
		}
	} else {
		for(i=_numObjectsInRoom; i>0; --i) {
			if (_objs[i].obj_nr==obj) {
				if (_objs[i].fl_object_index)
					return getResourceAddress(rtFlObject, _objs[i].fl_object_index)+8;
				return getResourceAddress(rtRoom, _roomResource) + _objs[i].offs_obcd_to_room;
			}
		}
	}
	return 0;
}

void Scumm::addObjectToInventory(uint obj, uint room) {
	int i, slot;
	byte *ptr,*obcdptr;
	uint32 size,cdoffs;
	int numobj;
	byte *roomptr;
	CodeHeader *cdhd;
	RoomHeader *roomhdr;

	debug(1,"Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP

	if (whereIsObject(obj)==4) {
		i = getObjectIndex(obj);
		ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 64;
		size = READ_BE_UINT32_UNALIGNED(ptr+4);
		slot = getInventorySlot();
		_inventory[slot] = obj;
		createResource(rtInventory, slot, size);
		ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 64;
		memcpy(getResourceAddress(rtInventory, slot), ptr, size);
		CHECK_HEAP
		return;
	}
//	ensureResourceLoaded(rtRoom, room);
	roomptr = getResourceAddress(rtRoom, room);
	roomhdr = (RoomHeader*)findResource(MKID('RMHD'), roomptr, 0);
	numobj = READ_LE_UINT16(&roomhdr->numObjects);
	if (numobj==0)
		error("addObjectToInventory: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("addObjectToInventory: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	for (i=0; i<numobj; i++) {
		obcdptr = findResource(MKID('OBCD'), roomptr, i);
		if(obcdptr==NULL)
			error("addObjectToInventory: Not enough code blocks in room %d", room);
		cdhd = (CodeHeader*)findResource(MKID('CDHD'), obcdptr, 0);
		if ( READ_LE_UINT16(&cdhd->obj_id) == obj) {
			cdoffs = obcdptr - roomptr;
			size = READ_BE_UINT32_UNALIGNED(obcdptr+4);
			slot = getInventorySlot();
			_inventory[slot] = obj;
			createResource(rtInventory, slot, size);
			obcdptr = getResourceAddress(rtRoom, room) + cdoffs;
			memcpy(getResourceAddress(rtInventory,slot),obcdptr,size);
			CHECK_HEAP
			return;
		}
	}

	error("addObjectToInventory: Object %d not found in room %d", obj, room);
}

int Scumm::getInventorySlot() {
	int i;
	for (i=1; i<=_maxInventoryItems; i++) {
		if (_inventory[i]==0)
			return i;
	}
	error("Inventory full, %d max items", _maxInventoryItems);
}

void Scumm::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;
	if (owner==0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->type==0 && _inventory[ss->number]==obj) {
			putOwner(obj, 0);
			runHook(0);
			stopObjectCode();
			return;
		}
	}
	putOwner(obj, owner);
	runHook(0);
}

int Scumm::getObjX(int obj) {
	if (obj <= _vars[VAR_NUM_ACTOR]) {
		return derefActorSafe(obj,"getObjX")->x;
	} else {
		if (whereIsObject(obj)==-1)
			return -1;
		getObjectOrActorXY(obj);
		return _xPos;
	}
}

int Scumm::getObjY(int obj) {
	if (obj <= _vars[VAR_NUM_ACTOR]) {
		return derefActorSafe(obj,"getObjY")->y;
	} else {
		if (whereIsObject(obj)==-1)
			return -1;
		getObjectOrActorXY(obj);
		return _yPos;
	}
}

int Scumm::getObjDir(int obj) {
	if (obj <= _vars[VAR_NUM_ACTOR]) {
		return derefActorSafe(obj,"getObjDir")->facing;
	} else {
		getObjectXYPos(obj);
		return _dir;
	}
}

int Scumm::findInventory(int owner, int index) {
	int count = 1, i, obj;
	for (i=0; i!=_maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj)==owner && count++ == index)
			return obj;
	}
	return 0;	
}

int Scumm::getInventoryCount(int owner) {
	int i,obj;
	int count = 0;
	for (i=0; i!=_maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj) == owner)
			count++;
	}
	return count;
}

void Scumm::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i==-1)
		return;

	if (x != -1) {
		_objs[i].x_pos = x;
		_objs[i].y_pos = y;
	}

	addObjectToDrawQue(i);
	putState(obj, state);
}

static int getDist(int x, int y, int x2, int y2) {
	int a = abs(y-y2);
	int b = abs(x-x2);
	if (a>b)
		return a;
	return b;
}

int Scumm::getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f) {
	int i,j;
	int x,y;
	int x2,y2;
	
	j = i = 0xFF;

	if (is_obj_1) {
		if (getObjectOrActorXY(b)==-1)
			return -1;
		if (b < _vars[VAR_NUM_ACTOR])
			i = derefActorSafe(b, "unkObjProc1")->scalex;
		x = _xPos;
		y = _yPos;
	} else {
		x = b;
		y = c;
	}

	if (is_obj_2) {
		if (getObjectOrActorXY(e)==-1)
			return -1;
		if (e < _vars[VAR_NUM_ACTOR])
			j = derefActorSafe(e, "unkObjProc1(2)")->scalex;
		x2 = _xPos;
		y2 = _yPos;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x,y,x2,y2) * 0xFF / ((i + j)>>1);
}

void Scumm::setCursorImg(uint room, uint img) {
	byte *ptr;
	int index;
	CodeHeader *cdhd;
	ImageHeader *imhd;
	int w,h;
	byte *roomptr,*obcd,*obim,*dataptr,*bomp;
	RoomHeader *rmhd;
	int i,numobj;
	uint32 size;

	if (getObjectIndex(img)!=-1) {
		obim = getObjectAddress(img);
		ptr = obim + READ_BE_UINT32(&((ImageHeader*)obim)->size);
		cdhd = (CodeHeader*)findResource(MKID('CDHD'), obim, 0);
		imhd = (ImageHeader*)findResource(MKID('IMHD'), ptr, 0);
	} else {
		ensureResourceLoaded(1, room);
		roomptr = getResourceAddress(1, room);
		rmhd = (RoomHeader*)findResource(MKID('RMHD'), roomptr, 0);

		numobj = READ_LE_UINT16(&rmhd->numObjects);
		for(i=0; ;i++) {
			if (i>=numobj)
				error("setCursorImg: object %d code not found in room %d", img, room);
			
			obcd = findResource(MKID('OBCD'), roomptr, i);
			if (obcd==NULL)
				error("setCursorImg: not enough code blocks in room %d", room);
			cdhd = (CodeHeader*)findResource(MKID('CDHD'), obcd, 0);
			if (READ_LE_UINT16(&cdhd->obj_id) == img)
				break;
		}

		for(i=0; ;i++) {
			if (i>=numobj)
				error("setCursorImg: object %d image not found in room %d", img, room);
			obim = findResource(MKID('OBIM'), roomptr, i);
			if (obim==NULL)
				error("setCursorImg: not enough image blocks in room %d", room);
			imhd = (ImageHeader*)findResource(MKID('IMHD'), obim, 0);
			if (READ_LE_UINT16(&imhd->obj_id) == img)
				break;
		}
	}

	setCursorHotspot2(
		READ_LE_UINT16(&imhd->hotspot[0].x),
		READ_LE_UINT16(&imhd->hotspot[0].y));

	w = READ_LE_UINT16(&cdhd->v6.w)>>3;
	h = READ_LE_UINT16(&cdhd->v6.h)>>3;

	size = READ_BE_UINT32(&cdhd->size);
	if (size > 1000)
		error("setCursorImg: Cursor image too large");
	
	dataptr = findResource(MKID('IM01'),obim, 0);

	if ((bomp = findResource(MKID('BOMP'), dataptr, 0)) != NULL)
		useBompCursor(bomp, w, h);
	else
		useIm01Cursor(dataptr, w, h);

}
