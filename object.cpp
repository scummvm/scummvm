/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in putClass");

	cls &= 0x7F;
	checkRange(32,1,cls,"Class %d out of range in getClass");

	if (set)
		_classData[obj] |= (1<<(cls-1));
	else
		_classData[obj] &= ~(1<<(cls-1));
}

int Scumm::getOwner(int obj) {
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in getOwner");
	return _objectOwnerTable[obj];
}

void Scumm::putOwner(int act, int owner) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in putOwner");
	checkRange(0xFF, 0, owner, "Owner %d out of range in putOwner");
	_objectOwnerTable[act] = owner;
}

int Scumm::getState(int act) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in getState");
	return _objectStateTable[act];
}

void Scumm::putState(int act, int state) {
	checkRange(_numGlobalObjects-1, 0, act, "Object %d out of range in putState");
	checkRange(0xFF, 0, state, "State %d out of range in putState");
	_objectStateTable[act] = state;
}

int Scumm::getObjectRoom(int obj) {
	checkRange(_numGlobalObjects-1, 0, obj, "Object %d out of range in getObjectRoom");
	return _objectRoomTable[obj];
}

int Scumm::getObjectIndex(int object) {
	int i;

	/* OF_OWNER_ROOM should be 0xFF for full throttle, else 0xF */
	if (_objectOwnerTable[object] != OF_OWNER_ROOM) {
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

	if (object >= _numGlobalObjects)
		return WIO_NOT_FOUND;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM) {
		for (i=0; i<_maxInventoryItems; i++)
			if (_inventory[i] == object)
				return WIO_INVENTORY;
		return WIO_NOT_FOUND;
	}

	for (i=_numObjectsInRoom; i>0; i--)
		if (_objs[i].obj_nr == object) {
			if (_objs[i].fl_object_index)
				return WIO_FLOBJECT;
			return WIO_ROOM;
		}
	return WIO_NOT_FOUND;
}

int Scumm::getObjectOrActorXY(int object) {
	if (object < NUM_ACTORS) {
		return getActorXYPos(derefActorSafe(object, "getObjectOrActorXY"));
	}
	switch(whereIsObject(object)) {
	case WIO_NOT_FOUND:
		return -1;
	case WIO_INVENTORY:
		return getActorXYPos(derefActorSafe(_objectOwnerTable[object],"getObjectOrActorXY(2)"));
	}
	getObjectXYPos(object);
	return 0;
}

/* Return the position of an object.
   Returns X, Y and direction in angles
 */
void Scumm::getObjectXYPos(int object) {
	ObjectData *od = &_objs[getObjectIndex(object)];
	int state;
	byte *ptr;
	ImageHeader *imhd;
	int x,y;
	
        if(!(_features & GF_SMALL_HEADER)) {
                if (_features&GF_AFTER_V6) {
                        state = getState(object)-1;
                        if (state<0)
                                state = 0;

                        if (od->fl_object_index) {
                                ptr = getResourceAddress(rtFlObject, od->fl_object_index);
                                ptr = findResource(MKID('OBIM'), ptr);
                        } else {
                                ptr = getResourceAddress(rtRoom, _roomResource);
                                ptr += od->offs_obim_to_room;
                        }
                        assert(ptr);
                        imhd = (ImageHeader*)findResourceData(MKID('IMHD'), ptr);
			if( _features & GF_AFTER_V7) {
			        x = od->x_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].x);
				y = od->y_pos + (int16)READ_LE_UINT16(&imhd->v7.hotspot[state].y);
			} else {
                        	x = od->x_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].x);
	                        y = od->y_pos + (int16)READ_LE_UINT16(&imhd->old.hotspot[state].y);
			}
                } else {
                        x = od->walk_x;
						y = od->walk_y;
                }
		_xPos = x;
		_yPos = y;
		_dir = oldDirToNewDir(od->actordir&3);
        } else {
                x = od->walk_x;
				y = od->walk_y;
                _xPos = x;
                _yPos = y;
		_dir= oldDirToNewDir(od->actordir&3);
	
        }
}

int Scumm::getObjActToObjActDist(int a, int b) {
	int x,y;
	Actor *acta = NULL;
	Actor *actb = NULL;

	if (a<NUM_ACTORS)
		acta = derefActorSafe(a, "getObjActToObjActDist");

	if (b<NUM_ACTORS)
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
		AdjustBoxResult r = adjustXYToBeInBox(acta, _xPos, _yPos, 0);
		_xPos = r.x;
		_yPos = r.y;
	}

	y = abs(y-_yPos);
	x = abs(x-_xPos);

	if (y>x) x=y;
	return x;
}

int Scumm::findObject(int x, int y) {
	int i,b;
	byte a;

	for (i=1; i<=_numObjectsInRoom; i++) {
		if (!_objs[i].obj_nr || getClass(_objs[i].obj_nr, 32))
			continue;
		b = i;
		do {
			a = _objs[b].parentstate;
			b = _objs[b].parent;
			if (b==0) {
				if (_objs[i].x_pos <= x &&
						_objs[i].width + _objs[i].x_pos > x &&
						_objs[i].y_pos <= y &&
						_objs[i].height + _objs[i].y_pos > y)
						return _objs[i].obj_nr;
				break;
			}
		} while ( _objs[b].state == a);
	}
	return 0;
}

void Scumm::drawRoomObject(int i, int arg) {
	ObjectData *od;
	byte a;

	od = &_objs[i];
	if (!od->obj_nr || !od->state)
		return;

	do {
		a = od->parentstate;
		if (!od->parent) {
			drawObject(i, arg);
			break;
		}
		od = &_objs[od->parent];
	} while (od->state==a);
}

void Scumm::drawRoomObjects(int arg) {
	int i;

	if (_features & GF_DRAWOBJ_OTHER_ORDER) {
		for(i=1; i<=_numObjectsInRoom; i++)
			drawRoomObject(i,arg);
	} else {
		for(i=_numObjectsInRoom; i!=0; i--)
			drawRoomObject(i,arg);
	}
}

const uint32 IMxx_tags[] = {
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

	xpos = od->x_pos>>3;
	ypos = od->y_pos;
	width = od->width>>3;
	height = od->height &= 0xF8; // Ender

	if (width==0 || xpos > _screenEndStrip || xpos + width < _screenStartStrip)
		return;
	
	if (od->fl_object_index) {
		ptr = getResourceAddress(rtFlObject, od->fl_object_index);
		ptr = findResource(MKID('OBIM'), ptr); 
	} else {
		ptr = getResourceAddress(rtRoom, _roomResource);
		ptr = ptr + od->offs_obim_to_room;
	}

	if(_features & GF_SMALL_HEADER)
                ptr +=8;					
        else
                ptr = findResource(IMxx_tags[getState(od->obj_nr)], ptr);
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
		gfxUsageBits[tmp] |= 0x80000000;
		if (tmp < x)
			x = tmp;
		numstrip++;
	}

	if (numstrip!=0) {
		byte flags = Gdi::dbAllowMaskOr;
		if (_features&GF_AFTER_V7 && getClass(od->obj_nr, 22))
			flags |= Gdi::dbDrawMaskOnBoth;
		gdi.drawBitmap(ptr, _curVirtScreen, x, ypos, height, x-xpos, numstrip, flags);
	}
}

void Scumm::loadRoomObjects() {
	int i,j;
	ObjectData *od;
	byte *ptr;
	uint16 obim_id;
	byte *room,*searchptr;
	ImageHeader *imhd;
	RoomHeader *roomhdr;
    CodeHeader *cdhd;

	CHECK_HEAP
	
	room = getResourceAddress(rtRoom, _roomResource);
	roomhdr = (RoomHeader*)findResourceData(MKID('RMHD'), room);

	if(_features & GF_AFTER_V7)
		_numObjectsInRoom = READ_LE_UINT16(&(roomhdr->v7.numObjects));
	else
		_numObjectsInRoom = READ_LE_UINT16(&(roomhdr->old.numObjects));
	
	if (_numObjectsInRoom == 0)
		return;
	
	if (_numObjectsInRoom > _numLocalObjects)
		error("More than %d objects in room %d", _numLocalObjects, _roomResource);

	od = &_objs[1];
	searchptr = room;
	for (i=0; i<_numObjectsInRoom; i++,od++) {
		ptr = findResource(MKID('OBCD'), searchptr);
		if (ptr==NULL)
			error("Room %d missing object code block(s)", _roomResource);

		od->offs_obcd_to_room = ptr - room;
		cdhd = (CodeHeader*)findResourceData(MKID('CDHD'), ptr);

		if(_features & GF_AFTER_V7)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
		else
			if(_features & GF_AFTER_V6)
				od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
			else
				od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));

#ifdef DUMP_SCRIPTS
		do {
			char buf[32];
			sprintf(buf,"roomobj-%d-",_roomResource);
			dumpResource(buf, od->obj_nr, ptr);
		} while (0);
#endif
		searchptr = NULL;
	}

	searchptr = room;
	for (i=0; i<_numObjectsInRoom; i++) {
		ptr = findResource(MKID('OBIM'), searchptr);
		if (ptr==NULL)
			error("Room %d missing image blocks(s)", _roomResource);

		imhd = (ImageHeader*)findResourceData(MKID('IMHD'), ptr);
		if (_features & GF_AFTER_V7)
			obim_id = READ_LE_UINT16(&imhd->v7.obj_id);
		else
			obim_id = READ_LE_UINT16(&imhd->old.obj_id);

		for(j=1; j<=_numObjectsInRoom; j++) {
			if (_objs[j].obj_nr==obim_id)
				_objs[j].offs_obim_to_room = ptr - room;
		}
		searchptr = NULL;
	}

	od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		setupRoomObject(od, room);
	}

	CHECK_HEAP
}

void Scumm::loadRoomObjectsSmall() {
       int i,j;
       ObjectData *od;
       byte *ptr;
       uint16 obim_id;
       byte *room,*searchptr;       
       RoomHeader *roomhdr;       

       CHECK_HEAP
 
       room = getResourceAddress(rtRoom, _roomResource);
       roomhdr = (RoomHeader*)findResourceData(MKID('RMHD'), room);

       _numObjectsInRoom = READ_LE_UINT16(&(roomhdr->old.numObjects));

       if (_numObjectsInRoom == 0)
               return;
 
       if (_numObjectsInRoom > _numLocalObjects)
               error("More than %d objects in room %d", _numLocalObjects, _roomResource);

       od = &_objs[1];
       searchptr = room;
       for (i=0; i<_numObjectsInRoom; i++,od++) {
               ptr = findResourceSmall(MKID('OBCD'), searchptr);
               if (ptr==NULL)
                       error("Room %d missing object code block(s)", _roomResource);

               od->offs_obcd_to_room = ptr - room;
               od->obj_nr = READ_LE_UINT16(ptr+6);

#ifdef DUMP_SCRIPTS
               do {
                       char buf[32];
                       sprintf(buf,"roomobj-%d-",_roomResource);
                       dumpResource(buf, od->obj_nr, ptr);
               } while (0);
#endif
               searchptr = NULL;
       }

       searchptr = room;
       for (i=0; i<_numObjectsInRoom; i++) {
               ptr = findResourceSmall(MKID('OBIM'), searchptr);
               if (ptr==NULL)
                       error("Room %d missing image blocks(s)", _roomResource);

               obim_id = READ_LE_UINT16(ptr+6);

               for(j=1; j<=_numObjectsInRoom; j++) {
                       if (_objs[j].obj_nr==obim_id)
                               _objs[j].offs_obim_to_room = ptr - room;
               }
               searchptr = NULL;
       }

       od = &_objs[1];
       for (i=1; i<=_numObjectsInRoom; i++,od++) {
               setupRoomObject(od, room);
       }

       CHECK_HEAP
}

void Scumm::setupRoomObject(ObjectData *od, byte *room) {      
	  CodeHeader *cdhd;
	  ImageHeader *imhd;

        if(_features & GF_SMALL_HEADER) {
               
		byte *ptr = room + od->offs_obcd_to_room;
		
		od->obj_nr = READ_LE_UINT16(ptr+6); // ok
		
		od->width = *(ptr+11)<<3; // ok		
		od->x_pos = *(ptr+9)<<3; // ok
		
		if(*(ptr+10) & 0x80) {
			od->parentstate = 1;  // it's 0x10 in the original code
		} else {
			od->parentstate = 0;
		}

		od->y_pos = ((*(ptr+10))&0x7F)<<3;
		
		od->parent = *(ptr+12);
		od->walk_x = READ_LE_UINT16(ptr+13);

		od->walk_y = READ_LE_UINT16(ptr+15);
		
		od->actordir = (*(ptr+17))&7;
		od->height = *(ptr+17); // ok
		
						
	       
               return;
        }

        cdhd = (CodeHeader*)findResourceData(MKID('CDHD'), room + od->offs_obcd_to_room);
	if(_features & GF_AFTER_V7)
		od->obj_nr = READ_LE_UINT16(&(cdhd->v7.obj_id));
	else
		if(_features & GF_AFTER_V6)
			od->obj_nr = READ_LE_UINT16(&(cdhd->v6.obj_id));
		else
		        od->obj_nr = READ_LE_UINT16(&(cdhd->v5.obj_id));
	
	if(!(_features & GF_AFTER_V7)) {
		if (_features & GF_AFTER_V6) {
			od->width = READ_LE_UINT16(&cdhd->v6.w);
			od->height = READ_LE_UINT16(&cdhd->v6.h);
			od->x_pos = ((int16)READ_LE_UINT16(&cdhd->v6.x));
			od->y_pos = ((int16)READ_LE_UINT16(&cdhd->v6.y));
			if (cdhd->v6.flags == 0x80) {
				od->parentstate = 1;
			} else {
				od->parentstate = (cdhd->v6.flags&0xF);
			}
			od->parent = cdhd->v6.parent;
			od->actordir = cdhd->v6.actordir;
		} else {
			od->width = cdhd->v5.w<<3;
			od->height = cdhd->v5.h<<3;
			od->x_pos = cdhd->v5.x<<3;
			od->y_pos = cdhd->v5.y<<3;
			if (cdhd->v5.flags == 0x80) {
				od->parentstate = 1;
			} else {
				od->parentstate = (cdhd->v5.flags&0xF);
			}
			od->parent = cdhd->v5.parent;
			od->walk_x = READ_LE_UINT16(&cdhd->v5.walk_x);
			od->walk_y = READ_LE_UINT16(&cdhd->v5.walk_y);
			od->actordir = cdhd->v5.actordir;
		}
	} else {
		od->parent = cdhd->v7.parent;
		od->parentstate = cdhd->v7.parentstate;

		imhd = (ImageHeader*)findResourceData(MKID('IMHD'), room + od->offs_obim_to_room);
		od->x_pos = READ_LE_UINT16(&imhd->v7.x_pos);
		od->y_pos = READ_LE_UINT16(&imhd->v7.y_pos);
		od->width =	READ_LE_UINT16(&imhd->v7.width);
		od->height = READ_LE_UINT16(&imhd->v7.height);
		od->actordir = READ_LE_UINT16(&imhd->v7.actordir);

	}
	od->fl_object_index = 0;
}

void Scumm::fixObjectFlags() {
	int i;
	ObjectData *od = &_objs[1];
	for (i=1; i<=_numObjectsInRoom; i++,od++) {
		od->state = _objectStateTable[od->obj_nr];
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

	stopObjectScript(obj);
	
	if (getOwner(obj)==OF_OWNER_ROOM) {
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
			if (j==WIO_INVENTORY) {
				nukeResource(rtInventory, i);
				_inventory[i] = 0;
			}
			a = &_inventory[1];
			for (i=1; i < _maxInventoryItems-1; i++,a++) {
				if (!a[0] && a[1]) {
					a[0] = a[1];
					a[1] = 0;
					_baseInventoryItems[i] = _baseInventoryItems[i+1];
					_baseInventoryItems[i+1] = NULL;
				}
			}
			return;
		}
	}
}

void Scumm::removeObjectFromRoom(int obj) {
	int i,cnt;
	uint32 *ptr;
	
	for(i=1; i<=_numObjectsInRoom; i++) {
		if (_objs[i].obj_nr==(uint16)obj) {
			if (_objs[i].width != 0) {
				ptr = &gfxUsageBits[_objs[i].x_pos>>3];
				cnt = _objs[i].width>>3;
				do {
					*ptr++ |= 0x80000000;
				} while (--cnt);
			}
			_BgNeedsRedraw = true;
			return;
		}
	}
}

void Scumm::addObjectToDrawQue(int object) {
	_drawObjectQue[_drawObjectQueNr++] = object;
	if ((unsigned int)_drawObjectQueNr > sizeof(_drawObjectQue)/sizeof(_drawObjectQue[0]))
		error("Draw Object Que overflow");
}

void Scumm::clearDrawObjectQueue() {
	_drawObjectQueNr = 0;
}

byte *Scumm::getObjOrActorName(int obj) {
	byte *objptr;

	if (obj < NUM_ACTORS)
		return getActorName(derefActorSafe(obj, "getObjOrActorName"));

	if(_features & GF_SMALL_HEADER)	{
		byte offset;

		objptr = getOBCDFromObject(obj);
		if (objptr)
			offset = *(objptr+18);
		return(objptr+offset);
        }

	objptr = getOBCDFromObject(obj);
	if (objptr==NULL)
		return (byte*)" ";
	
	return findResourceData(MKID('OBNA'), objptr);
}

uint32 Scumm::getOBCDOffs(int object) {
	int i;

	if (_objectOwnerTable[object] != OF_OWNER_ROOM)
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

byte *Scumm::getOBCDFromObject(int obj) {
	int i;

	if (_objectOwnerTable[obj] != OF_OWNER_ROOM) {
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
	uint32 size;
	byte *obcdptr,*ptr;
	FindObjectInRoom foir;

	debug(1,"Adding object %d from room %d into inventory", obj, room);

	CHECK_HEAP

	if (whereIsObject(obj)==WIO_FLOBJECT) {
		i = getObjectIndex(obj);
		ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 8;
		size = READ_BE_UINT32_UNALIGNED(ptr+4);
		slot = getInventorySlot();
		_inventory[slot] = obj;
		createResource(rtInventory, slot, size);
		ptr = getResourceAddress(rtFlObject, _objs[i].fl_object_index) + 8;
		memcpy(getResourceAddress(rtInventory, slot), ptr, size);
	} else {
		findObjectInRoom(&foir, foCodeHeader, obj, room);
                if(_features & GF_SMALL_HEADER )
                        size = READ_LE_UINT32(foir.obcd);
                else
                        size = READ_BE_UINT32_UNALIGNED(foir.obcd+4);
		slot = getInventorySlot();
		_inventory[slot] = obj;
		createResource(rtInventory, slot, size);
		obcdptr = getResourceAddress(rtRoom, room) - foir.roomptr + foir.obcd;
		memcpy(getResourceAddress(rtInventory,slot),obcdptr,size);
	}

	CHECK_HEAP
}

void Scumm::findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint id, uint room) {
	CodeHeader *cdhd;
	int i, numobj;
	byte *roomptr,*obcdptr,*obimptr,*searchptr;
	RoomHeader *roomhdr;
	ImageHeader *imhd;
	int id2;
	int id3;
	
	if (findWhat&foCheckAlreadyLoaded && getObjectIndex(id) != -1) {
		fo->obcd = obcdptr = getOBCDFromObject(id);
		assert((byte*)obcdptr > (byte*)256);
		fo->obim = obimptr = obcdptr + READ_BE_UINT32_UNALIGNED(&((ResHdr*)obcdptr)->size);
		fo->cdhd = (CodeHeader*)findResourceData(MKID('CDHD'), obcdptr);
		fo->imhd = (ImageHeader*)findResourceData(MKID('IMHD'), obimptr);
		return;
	}

	fo->roomptr = roomptr = getResourceAddress(rtRoom, room);
	roomhdr = (RoomHeader*)findResourceData(MKID('RMHD'), roomptr);
	
	if(_features & GF_AFTER_V7)
		numobj = READ_LE_UINT16(&(roomhdr->v7.numObjects));
	else
		numobj = READ_LE_UINT16(&(roomhdr->old.numObjects));
	
	if (numobj==0)
		error("findObjectInRoom: No object found in room %d", room);
	if (numobj > _numLocalObjects)
		error("findObjectInRoom: More (%d) than %d objects in room %d", numobj, _numLocalObjects, room);

	if (findWhat & foCodeHeader) {
		searchptr = roomptr;
		for (i=0;;) {
                        if(_features & GF_SMALL_HEADER)
                               obcdptr = findResourceSmall(MKID('OBCD'), searchptr);
                        else
                               obcdptr = findResource(MKID('OBCD'), searchptr);
			if(obcdptr==NULL)
				error("findObjectInRoom: Not enough code blocks in room %d", room);
                        if ( _features & GF_SMALL_HEADER) {
                                if ( READ_LE_UINT16(obcdptr+6) == (uint16)id) {
                                        fo->cdhd = NULL;
                                        fo->obcd = obcdptr;
                                        break;
                                }
                        } else {
                               cdhd = (CodeHeader*)findResourceData(MKID('CDHD'), obcdptr);
				if ( _features & GF_AFTER_V7)
					id2 = READ_LE_UINT16(&(cdhd->v7.obj_id));
				else
					if( _features & GF_AFTER_V6)
						id2 = READ_LE_UINT16(&(cdhd->v6.obj_id));
					else
						id2 = READ_LE_UINT16(&(cdhd->v5.obj_id));
			       
                                if ( id2 == (uint16)id) {
                                        fo->cdhd = cdhd;
                                        fo->obcd = obcdptr;
                                        break;
                                }
                        }
			if (++i == numobj)
				error("findObjectInRoom: Object %d not found in room %d", id, room);
			searchptr = NULL;
		}
	}

	if (findWhat & foImageHeader) {
		searchptr = roomptr;
		for(i=0;;) {
                        if(_features & GF_SMALL_HEADER)
                                obimptr = findResourceSmall(MKID('OBIM'), searchptr);
                        else
                                obimptr = findResource(MKID('OBIM'), searchptr);
			if (obimptr==NULL)
				error("findObjectInRoom: Not enough image blocks in room %d", room);
			imhd = (ImageHeader*)findResourceData(MKID('IMHD'), obimptr);
                        if(_features & GF_SMALL_HEADER){
                                        if (READ_LE_UINT16(obimptr+6) == (uint16)id) {
                                                fo->obim = obimptr;
                                                fo->imhd = imhd;
                                        break;
                                }
                        } else {
				if(_features & GF_AFTER_V7)
					id3 = READ_LE_UINT16(&imhd->v7.obj_id);
				else
					id3 = READ_LE_UINT16(&imhd->old.obj_id);
                                if (id3 == (uint16)id) {
                                        fo->obim = obimptr;
                                        fo->imhd = imhd;
                                        break;
                                }
                        }                 
			if (++i==numobj)
				error("findObjectInRoom: Object %d image not found in room %d", id, room);
			searchptr = NULL;
		}
	}
}

int Scumm::getInventorySlot() {
	int i;
	for (i=1; i<=_maxInventoryItems; i++) {
		if (_inventory[i]==0)
			return i;
	}
	error("Inventory full, %d max items", _maxInventoryItems);
}

void Scumm::SamInventoryHack(int obj) {	// FIXME: Sam and Max hack
	int base = 6;

	while (base < 80) {
		int value = readArray(178, 0, base);
		if (value == obj) return;
		if (value == 0) {
				_vars[179]++;
				printf("Adding item %d to slot %d\n", obj, base);
				writeArray(178, 0, base, obj);
				return;
		}
		base++;
	}
}

void Scumm::setOwnerOf(int obj, int owner) {
	ScriptSlot *ss;
	if (owner==0) {
		clearOwnerOf(obj);
		ss = &vm.slot[_currentScript];
		if (ss->where==WIO_INVENTORY && _inventory[ss->number]==obj) {
			putOwner(obj, 0);
			runHook(0);
			stopObjectCode();
			return;
		}
	}
	if ((owner == 2) && (_gameId == GID_SAMNMAX))
		SamInventoryHack(obj);

	putOwner(obj, owner);
	runHook(0);
}

int Scumm::getObjX(int obj) {
	if (obj < NUM_ACTORS) {
		if (obj<1)
			return 0; /* fix for indy4's map */
		return derefActorSafe(obj,"getObjX")->x;
	} else {
		if (whereIsObject(obj)==WIO_NOT_FOUND)
			return -1;
		getObjectOrActorXY(obj);
		return _xPos;
	}
}

int Scumm::getObjY(int obj) {
	if (obj < NUM_ACTORS) {
		if (obj<1)
			return 0; /* fix for indy4's map */
		return derefActorSafe(obj,"getObjY")->y;
	} else {
		if (whereIsObject(obj)==WIO_NOT_FOUND)
			return -1;
		getObjectOrActorXY(obj);
		return _yPos;
	}
}

int Scumm::getObjOldDir(int obj) {
	if (obj < NUM_ACTORS) {
		return newDirToOldDir(derefActorSafe(obj,"getObjOldDir")->facing);
	} else {
		getObjectXYPos(obj);
		return _dir;
	}
}

int Scumm::getObjNewDir(int obj) {
	if (obj < NUM_ACTORS) {
		return derefActorSafe(obj,"getObjNewDir")->facing;
	} else {
		getObjectXYPos(obj);
		return oldDirToNewDir(_dir);
	}
}

int Scumm::findInventory(int owner, int idx) {
	int count = 1, i, obj;
	for (i=0; i!=_maxInventoryItems; i++) {
		obj = _inventory[i];
		if (obj && getOwner(obj)==owner && count++ == idx)
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
	if (i==-1) {
		warning("setObjectState: no such object");
		return;
	}

	if (x != -1) {
		_objs[i].x_pos = x<<3;
		_objs[i].y_pos = y<<3;
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
		if (b < NUM_ACTORS)
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
		if (e < NUM_ACTORS)
			j = derefActorSafe(e, "unkObjProc1(2)")->scalex;
		x2 = _xPos;
		y2 = _yPos;
	} else {
		x2 = e;
		y2 = f;
	}

	return getDist(x,y,x2,y2) * 0xFF / ((i + j)>>1);
}

void Scumm::setCursorImg(uint img, uint room, uint imgindex) {	
	int w,h;
	byte *dataptr,*bomp;
	uint32 size;
	FindObjectInRoom foir;
	
	if (room==(uint)-1)
		room = getObjectRoom(img);

	findObjectInRoom(&foir, foCodeHeader | foImageHeader | foCheckAlreadyLoaded, img, room);

	if(_features & GF_AFTER_V7)
		setCursorHotspot2(
			READ_LE_UINT16(&foir.imhd->v7.hotspot[0].x),
			READ_LE_UINT16(&foir.imhd->v7.hotspot[0].y));
	else
		setCursorHotspot2(
			READ_LE_UINT16(&foir.imhd->old.hotspot[0].x),
			READ_LE_UINT16(&foir.imhd->old.hotspot[0].y));
	

	if(!(_features & GF_AFTER_V7)) {
		w = READ_LE_UINT16(&foir.cdhd->v6.w)>>3;
		h = READ_LE_UINT16(&foir.cdhd->v6.h)>>3;
	} else {
		w = READ_LE_UINT16(&foir.imhd->v7.width)>>3;
		h = READ_LE_UINT16(&foir.imhd->v7.height)>>3;
	}

	dataptr = findResource(IMxx_tags[imgindex],foir.obim);
	if (dataptr==NULL)
		error("setCursorImg: No such image");

	size = READ_BE_UINT32_UNALIGNED(dataptr+4);
	if (size > sizeof(_grabbedCursor))
		error("setCursorImg: Cursor image too large");

	if ((bomp = findResource(MKID('BOMP'), dataptr)) != NULL)
		useBompCursor(bomp, w, h);
	else
		useIm01Cursor(dataptr, w, h);

}

void Scumm::nukeFlObjects(int min, int max) {
	ObjectData *od;
	int i;

	warning("nukeFlObjects(%d,%d)", min, max);

	for (i=_numObjectsInRoom,od=_objs; --i>=0; od++)
		if (od->fl_object_index && od->obj_nr>=min && od->obj_nr<=max) {
			nukeResource(rtFlObject, od->fl_object_index);
			od->obj_nr = 0;
			od->fl_object_index = 0;
		}
}

void Scumm::enqueueObject(int a, int b, int c, int d, int e, int f, int g, int h, int mode) {
	EnqueuedObject *eo;
	ObjectData *od;

	if (_enqueuePos==sizeof(_enqueuedObjects)/sizeof(_enqueuedObjects[0]))
		error("enqueueObject: overflow");

	eo = &_enqueuedObjects[_enqueuePos++];
	eo->a = a;
	eo->b = _enqueue_b;
	eo->c = _enqueue_c;
	eo->d = _enqueue_d;
	eo->e = _enqueue_e;
	eo->x = b;
	eo->y = c;
	if (d==0) {
		od = &_objs[getObjectIndex(a)];
		eo->width = od->width;
	} else {
		eo->width = d;
	}
	if (e==0) {
		od = &_objs[getObjectIndex(a)];
		eo->height = od->height;
	} else {
		eo->height = e;
	}

	eo->j = f;
	eo->k = g;
	eo->l = h;
}

void Scumm::drawEnqueuedObjects() {
	EnqueuedObject *eo;
	int i;

	eo = _enqueuedObjects;
	for(i=0; i < _enqueuePos; i++,eo++) {
		drawEnqueuedObject(eo);
	}
}


void Scumm::drawEnqueuedObject(EnqueuedObject *eo) {
	VirtScreen *vs;
	byte *roomptr,*bomp;
	byte *ptr;
	int idx;
	ObjectData *od;

	BompDrawData bdd;

	vs = &virtscr[0];

	_lastXstart = vs->xstart;

	if (eo->l==0) {
		roomptr = getResourceAddress(1, _roomResource);
		idx = getObjectIndex(eo->a);
		assert(idx != -1);
		ptr = roomptr + _objs[idx].offs_obim_to_room;
	} else if (eo->a!=0) {
		od = &_objs[getObjectIndex(eo->a)];
		ptr = getResourceAddress(rtFlObject, od->fl_object_index);
		assert(ptr);
		ptr = findResource(MKID('OBIM'), ptr);
	} else {
		warning("drawEnqueuedObject: invalid");
		return;
	}

	if(eo->l == 0)
		eo->l = 1;
	
	assert(ptr);
	ptr = findResource(IMxx_tags[eo->l], ptr);
//        assert(ptr);
        if (!ptr)               /* FIXME: Sam and Max highway subgame */
                return;
	bomp = findResourceData(MKID('BOMP'), ptr);

	bdd.srcwidth = READ_LE_UINT16(&((BompHeader*)bomp)->width);
	bdd.srcheight = READ_LE_UINT16(&((BompHeader*)bomp)->height);

	bdd.out = vs->screenPtr + vs->xstart;
	bdd.outwidth = 320;
	bdd.outheight = vs->height;
	bdd.dataptr = bomp + 10;
	bdd.x = eo->x;
	bdd.y = eo->y;
	bdd.scale_x = (unsigned char)eo->j;
	bdd.scale_y = (unsigned char)eo->k;

	updateDirtyRect(vs->number, bdd.x, bdd.x+bdd.srcwidth, bdd.y, bdd.y+bdd.srcheight, 0);

	if (eo->a) {
		drawBomp(&bdd);
	}
}

void Scumm::removeEnqueuedObjects() {
	EnqueuedObject *eo;
	int i;

	eo = _enqueuedObjects;
	for(i=0; i < _enqueuePos; i++,eo++) {
		removeEnqueuedObject(eo);
	}

	clearEnqueue();
}

void Scumm::removeEnqueuedObject(EnqueuedObject *eo) {
	restoreBG(eo->x, eo->y, eo->x + eo->width, eo->y + eo->height);
}

int Scumm::findFlObjectSlot() {
	int i;
	for(i=1; i<_maxFLObject; i++) {
		if (_baseFLObject[i] == NULL)
			return i;
	}
	error("findFlObjectSlot: Out of FLObject slots");
}

void Scumm::loadFlObject(uint object, uint room) {
	FindObjectInRoom foir;
	int slot;
	ObjectData *od;
	byte *flob,*roomptr;
	uint32 obcd_size, obim_size, flob_size;

	/* Don't load an already loaded object */
	if (whereIsObject(object) != WIO_NOT_FOUND)
		return;

	/* Locate the object in the room resource */
	findObjectInRoom(&foir, foImageHeader | foCodeHeader, object, room);

	/* Add an entry for the new floating object in the local object table */
	if (++_numObjectsInRoom > _numLocalObjects)
		error("loadFlObject: Local Object Table overflow");
	od = &_objs[_numObjectsInRoom];

	/* Setup sizes */
	obcd_size = READ_BE_UINT32_UNALIGNED(foir.obcd + 4);
	od->offs_obcd_to_room = 8;
	od->offs_obim_to_room = obcd_size + 8;
	obim_size = READ_BE_UINT32_UNALIGNED(foir.obim + 4);
	flob_size = obcd_size + obim_size + 8;

	/* Allocate slot & memory for floating object */
	slot = findFlObjectSlot();
	createResource(rtFlObject, slot, flob_size);

	/* Copy object code + object image to floating object */
	roomptr = getResourceAddress(rtRoom, room);
	flob = getResourceAddress(rtFlObject, slot);
	((uint32*)flob)[0] = MKID('FLOB');
	((uint32*)flob)[1] = TO_BE_32(flob_size);
	memcpy(flob + 8, roomptr - foir.roomptr + foir.obcd, obcd_size);
	memcpy(flob + 8 + obcd_size, roomptr - foir.roomptr + foir.obim, obim_size);

	/* Setup local object flags */
	setupRoomObject(od, flob);

	od->fl_object_index = slot;
}
