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
 * Change Log:
 * $Log$
 * Revision 1.3  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 * Revision 1.2  2001/10/09 18:35:02  strigeus
 * fixed object parent bug
 * fixed some signed/unsigned comparisons
 *
 * Revision 1.1.1.1  2001/10/09 14:30:14  strigeus
 *
 * initial revision
 *
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

#if defined(DOTT)
void Scumm::getObjectXYPos(int object) {
	ObjectData *od = &_objs[getObjectIndex(object)];
	int state;
	byte *ptr;
	ImageHeader *imhd;
	int x,y;
	AdjustBoxResult abr;

	state = getState(object)-1;
	if (state<0)
		state = 0;

	if (od->fl_object_index) {
		ptr = getResourceAddress(0xD, od->fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr);
	} else {
		ptr = getResourceAddress(1, _roomResource);
		ptr += od->offs_obim_to_room;
	}

	imhd = (ImageHeader*)findResource2(MKID('IMHD'), ptr);
	x = od->x_pos*8 + imhd->hotspot[state].x;
	y = od->y_pos*8 + imhd->hotspot[state].y;

	abr = adjustXYToBeInBox(0, x, y);
	_xPos = abr.x;
	_yPos = abr.y;
	_dir = od->actordir&3;
}
#else
void Scumm::getObjectXYPos(int object) {
	ObjectData *od = &_objs[getObjectIndex(object)];
	AdjustBoxResult abr;
	abr = adjustXYToBeInBox(0, od->cdhd_10, od->cdhd_12);
	_xPos = abr.x;
	_yPos = abr.y;
	_dir = od->actordir&3;
}
#endif

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

	/* XXX: bug here? should be <= */
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
	int x,a,b;

	if (_BgNeedsRedraw)
		arg = 0;

	gdi.virtScreen = 0;
	
	od = &_objs[obj];

	xpos = od->x_pos;
	ypos = od->y_pos;
	width = od->numstrips;
	height = od->height;

	if (width==0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;
	
	if (od->fl_object_index) {
		ptr = getResourceAddress(0xD, od->fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr); 
	} else {
		ptr = getResourceAddress(1, _roomResource);
		ptr = ptr + od->offs_obim_to_room;
	}

	ptr = findResource(state_tags[getState(od->obj_nr)], ptr);
	if (!ptr)
		return;

	x = 0xFFFF;

	for (a=b=0; a<width; a++) {
		_drawBmpX = xpos + a;
		if (arg==1 && _screenStartStrip!=_drawBmpX)
			continue;
		if (arg==2 && _screenEndStrip!=_drawBmpX)
			continue;
		if (_screenStartStrip > _drawBmpX || _drawBmpX > _screenEndStrip)
			continue;
		actorDrawBits[_drawBmpX] |= 0x8000;
		if (_drawBmpX < x)
			x = _drawBmpX;
		b++;
	}

	if (b==0)
		return;

	_drawBmpY = ypos << 3;
	gdi.numLinesToProcess = height << 3;

	_drawBmpX = x;
	drawBmp(ptr, x - xpos, b, 1, "Object", od->obj_nr);
}

void Scumm::loadRoomObjects() {
	int i,j;
	ObjectData *od;
	byte *ptr;
	uint16 obim_id;
	byte *room,*tmp_room;
	ImageHeader *imhd;
	RoomHeader *roomhdr;

	CodeHeader *cdhd;

	CHECK_HEAP
	
	room = getResourceAddress(1, _roomResource);
	roomhdr = (RoomHeader*)findResource(MKID('RMHD'), room);

	_numObjectsInRoom = READ_LE_UINT16(&roomhdr->numObjects);
	
	if (_numObjectsInRoom == 0)
		return;
	
	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	tmp_room = room;

	od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		ptr = findResource(MKID('OBCD'), tmp_room);
		if (ptr==NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->offs_obcd_to_room = ptr - room;
		cdhd = (CodeHeader*)findResource2(MKID('CDHD'), ptr);
		od->obj_nr = READ_LE_UINT16(&cdhd->obj_id);

#ifdef DUMP_SCRIPTS
		do {
			char buf[32];
			sprintf(buf,"roomobj-%d-",_roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		} while (0);
#endif
		tmp_room = NULL;
	}

	tmp_room = room;
	for (i=1; i<=_numObjectsInRoom; i++) {
		ptr = findResource(MKID('OBIM'), tmp_room);
		if (ptr==NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		imhd = (ImageHeader*)findResource2(MKID('IMHD'), ptr);
		obim_id = READ_LE_UINT16(&imhd->obj_id);

		for(j=1; j<=_numObjectsInRoom; j++) {
			if (_objs[j].obj_nr==obim_id)
				_objs[j].offs_obim_to_room = ptr - room;
		}
		tmp_room = NULL;
	}

	od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		ptr = room + _objs[i].offs_obcd_to_room;
		cdhd = (CodeHeader*)findResource2(MKID('CDHD'), ptr);
		_objs[i].obj_nr = READ_LE_UINT16(&cdhd->obj_id);

#if defined(DOTT)
		_objs[i].numstrips = cdhd->w>>3;
		_objs[i].height = cdhd->h>>3;
		_objs[i].x_pos = cdhd->x>>3;
		_objs[i].y_pos = cdhd->y>>3;
#else
		_objs[i].numstrips = cdhd->w;
		_objs[i].height = cdhd->h;
		_objs[i].x_pos = cdhd->x;
		_objs[i].y_pos = cdhd->y;
#endif
		if (cdhd->flags == 0x80) {
			_objs[i].parentstate = 1<<4;
		} else {
			_objs[i].parentstate = (cdhd->flags&0xF)<<4;
		}
		_objs[i].parent = cdhd->parent;
#if !defined(DOTT)
		_objs[i].cdhd_10 = READ_LE_UINT16(&cdhd->unk2);
		_objs[i].cdhd_12 = READ_LE_UINT16(&cdhd->unk3);
#endif
		_objs[i].actordir = cdhd->actordir;
		_objs[i].fl_object_index = 0;
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
				nukeResource(0xD, _objs[i].fl_object_index);
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
				nukeResource(5, i);
				_inventory[i] = 0;
			}
			a = &_inventory[2];
			for (i=1; i < _maxInventoryItems-1; i++) {
				if (!a[-1] && a[0]) {
					a[-1] = a[0];
					ptr = getResourceAddress(5, i+1);
					_baseInventoryItems[i] = _baseInventoryItems[i+1];
					/* TODO: some wacky write is done here */
					error("clearOwnerOf: not fully implemented");
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
	if (_drawObjectQueNr > 200)
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
	
	return findResource(MKID('OBNA'), objptr) + 8;
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
				return getResourceAddress(5, i);
		}
	} else {
		for(i=_numObjectsInRoom; i>0; --i) {
			if (_objs[i].obj_nr==obj) {
				if (_objs[i].fl_object_index)
					return getResourceAddress(0xD, _objs[i].fl_object_index)+8;
				return getResourceAddress(1, _roomResource) + _objs[i].offs_obcd_to_room;
			}
		}
	}
	return 0;
}

void Scumm::addObjectToInventory(int obj, int room) {
	int i, slot;
	byte *ptr,*obcdptr;
	uint32 size,cdoffs;
	int numobj;
	byte *tmp_roomptr,*roomptr;
	CodeHeader *cdhd;
	RoomHeader *roomhdr;

	debug(1,"Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP

	if (whereIsObject(obj)==4) {
		i = getObjectIndex(obj);
		ptr = getResourceAddress(0xD, _objs[i].fl_object_index) + 64;
		size = READ_BE_UINT32_UNALIGNED(ptr+4);
		slot = getInventorySlot();
		_inventory[slot] = obj;
		createResource(5, slot, size);
		ptr = getResourceAddress(0xD, _objs[i].fl_object_index) + 64;
		memcpy(getResourceAddress(5, slot), ptr, size);
		CHECK_HEAP
		return;
	}
	ensureResourceLoaded(1, room);
	roomptr = getResourceAddress(1, room);
	roomhdr = (RoomHeader*)findResource(MKID('RMHD'), roomptr);
	numobj = READ_LE_UINT16(&roomhdr->numObjects);
	if (numobj==0)
		error("addObjectToInventory: No object found in room %d", room);
	if (numobj > 200)
		error("addObjectToInventory: More (%d) than %d objects in room %d", numobj, 200, room);

	tmp_roomptr = roomptr;
	for (i=1; i<=numobj; i++) {
		obcdptr = findResource(MKID('OBCD'), tmp_roomptr);
		if(obcdptr==NULL)
			error("addObjectToInventory: Not enough code blocks in room %d", room);
		cdhd = (CodeHeader*)findResource2(MKID('CDHD'), obcdptr);
		if ( READ_LE_UINT16(&cdhd->obj_id) == obj) {
			cdoffs = obcdptr - roomptr;
			size = READ_BE_UINT32_UNALIGNED(obcdptr+4);
			slot = getInventorySlot();
			_inventory[slot] = obj;
			createResource(5, slot, size);
			obcdptr = getResourceAddress(1, room) + cdoffs;
			memcpy(getResourceAddress(5,slot),obcdptr,size);
			CHECK_HEAP
			return;
		}
		tmp_roomptr = NULL;
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

#if defined(DOTT)
void Scumm::setObjectState(int obj, int state, int x, int y) {
	int i;

	i = getObjectIndex(obj);
	if (i==-1)
		return;

	if (x != -1) {
		_objs[i].x_pos = x;
		_objs[i].x_pos = y;
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
#endif