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
 * Revision 1.4  2001/10/16 12:20:20  strigeus
 * made files compile on unix
 *
 * Revision 1.3  2001/10/16 10:01:47  strigeus
 * preliminary DOTT support
 *
 * Revision 1.2  2001/10/10 10:02:33  strigeus
 * alternative mouse cursor
 * basic save&load
 *
 * Revision 1.1.1.1  2001/10/09 14:30:14  strigeus
 *
 * initial revision
 *
 *
 */

#include "stdafx.h"
#include "scumm.h"

/* Open a room */
void Scumm::openRoom(int room) {
	uint room_offs;
	char buf[256];

	debug(9, "openRoom(%d)",room);

	/* Don't load the same room again */
	if (_lastLoadedRoom == room)
		return;
	_lastLoadedRoom = room;

	/* Room -1 means close file */
	if (room==-1) {
		_encbyte = 0;
		deleteRoomOffsets();
		fileClose(_fileHandle);
		_fileHandle = NULL;
		return;
	}

	/* Either xxx.lfl or monkey.xxx file name */
	while (!_resFilePrefix) {
#if REAL_CODE
		room_offs = _roomFileOffsets[room];
#else
		room_offs = room ? _roomFileOffsets[room] : 0;
#endif
		
		if (room_offs == 0xFFFFFFFF)
			break;

		if (room_offs != 0 && room != 0) {
			_fileOffset = _roomFileOffsets[room];
			return;
		}
		if (room==0) {
			sprintf(buf, "%s.000", _exe_name);
		} else {
			sprintf(buf, "%s.%.3d", _exe_name, res.roomno[1][room]);
		}
		_encbyte = 0x69;
		if (openResourceFile(buf)) {
			if (room==0)
				return;
			readRoomsOffsets();
			_fileOffset = _roomFileOffsets[room];

			if (_fileOffset != 8)
				return;

			error("Room %d not in %s", room, buf);
			return;
		}
		askForDisk();
	}

	do {
		sprintf(buf, "%.3d.lfl", room);
		_encbyte = 0;
		if (openResourceFile(buf)) 
			break;
		askForDisk();
	} while(1);

	deleteRoomOffsets();
	_fileOffset = 0; /* start of file */
}

/* Delete the currently loaded room offsets */
void Scumm::deleteRoomOffsets() {
	if (!_dynamicRoomOffsets)
		return;
	
	for (int i=0; i<_maxRooms; i++) {
		if (_roomFileOffsets[i]!=0xFFFFFFFF)
			_roomFileOffsets[i] = 0;
	}
}

/* Read room offsets */
void Scumm::readRoomsOffsets() {
	int num,room;

	debug(9, "readRoomOffsets()");

	deleteRoomOffsets();
	if (!_dynamicRoomOffsets)
		return;

	fileSeek(_fileHandle, 16, SEEK_SET);
	
	num = fileReadByte();
	while (num) {
		num--;
		
		room = fileReadByte();
		if (_roomFileOffsets[room]!=0xFFFFFFFF) {
			_roomFileOffsets[room] = fileReadDwordLE();
		} else {
			fileReadDwordLE();
		}
	}
}

bool Scumm::openResourceFile(const char *filename) {
	char buf[256];
	
	debug(9, "openResourceFile(%s)",filename);

	if (_resFilePath) {
		sprintf(buf, "%s.%d\\%s", _resFilePath, _resFilePathId, filename);
	} else if (_resFilePrefix) {
		sprintf(buf, "%s%s", _resFilePrefix, filename);
	} else {
		strcpy(buf, filename);
	}

	if (_fileHandle != NULL) {
		fileClose(_fileHandle);
		_fileHandle = NULL;
	}

	_fileHandle = fileOpen(buf, 1);
	
	return _fileHandle != NULL;
}

void Scumm::askForDisk() {
	/* TODO: Not yet implemented */
	error("askForDisk: not yet implemented");
}

#if !defined(DOTT)

void Scumm::readIndexFile(int mode) {
	uint32 blocktype,itemsize;
	int numblock = 0;
#if defined(SCUMM_BIG_ENDIAN)
	int i;
#endif

	debug(9, "readIndexFile(%d)",mode);

	openRoom(-1);
	openRoom(0);
	
	while (1) {
		blocktype = fileReadDword();

		if (fileReadFailed(_fileHandle))
			break;
		itemsize = fileReadDwordBE();

		numblock++;

		switch(blocktype) {
		case MKID('DCHR'):
			readResTypeList(6,MKID('CHAR'),"charset");
			break;

		case MKID('DOBJ'):
			_numGlobalObjects = fileReadWordLE();
			_objectFlagTable = (byte*)alloc(_numGlobalObjects);
			if (mode==1) {
				fileSeek(_fileHandle, itemsize - 10, 1);
				break;
			}

			_classData = (uint32*)alloc(_numGlobalObjects * sizeof(uint32));
			fileRead(_fileHandle, _objectFlagTable, _numGlobalObjects);
			fileRead(_fileHandle, _classData, _numGlobalObjects * sizeof(uint32));
#if defined(SCUMM_BIG_ENDIAN)
			for (i=0; i<_numGlobalObjects; i++)
				_classData[i] = FROM_LE_32(_classData[i]);
#endif
			break;

		case MKID('RNAM'):
			fileSeek(_fileHandle, itemsize-8,1);
			break;

		case MKID('DROO'):
			readResTypeList(1,MKID('ROOM'),"room");
			break;

		case MKID('DSCR'):
			readResTypeList(2,MKID('SCRP'),"script");
			break;

		case MKID('DCOS'):
			readResTypeList(3,MKID('COST'),"costume");
			break;

		case MKID('MAXS'):
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			fileReadWordLE();
			break;

		case MKID('DSOU'):
			readResTypeList(4,MKID('SOUN'),"sound");
			break;

		default:
			error("Bad ID %c%c%c%c found in directory!", blocktype&0xFF, blocktype>>8, blocktype>>16, blocktype>>24);
			return;
		}
	}

	clearFileReadFailed(_fileHandle);

	if (numblock!=8)
		error("Not enough blocks read from directory");

	openRoom(-1);
	
	_numGlobalScripts = _maxScripts;
	_dynamicRoomOffsets = true;
}
#else


void Scumm::readIndexFile() {
	uint32 blocktype,itemsize;
	int numblock = 0;
	int num, i;

	debug(9, "readIndexFile()");

	openRoom(-1);
	openRoom(0);
	
	while (1) {
		blocktype = fileReadDword();

		if (fileReadFailed(_fileHandle))
			break;
		itemsize = fileReadDwordBE();

		numblock++;

			switch(blocktype) {
		case MKID('DCHR'):
			readResTypeList(6,MKID('CHAR'),"charset");
			break;

		case MKID('DOBJ'):
			 num = fileReadWordLE();
			 assert(num == _numGlobalObjects);
			fileRead(_fileHandle, _objectFlagTable, num);
			fileRead(_fileHandle, _classData, num * sizeof(uint32));
#if defined(SCUMM_BIG_ENDIAN)
			for (i=0; i<_numGlobalObjects; i++)
				_classData[i] = FROM_LE_32(_classData[i]);
#endif
			break;

		case MKID('RNAM'):
			fileSeek(_fileHandle, itemsize-8,1);
			break;

		case MKID('DROO'):
			readResTypeList(1,MKID('ROOM'),"room");
			break;

		case MKID('DSCR'):
			readResTypeList(2,MKID('SCRP'),"script");
			break;

		case MKID('DCOS'):
			readResTypeList(3,MKID('COST'),"costume");
			break;

		case MKID('MAXS'):
			readMAXS();
			break;

		case MKID('DSOU'):
			readResTypeList(4,MKID('SOUN'),"sound");
			break;

		case MKID('AARY'):
			readArrayFromIndexFile();
			break;

		default:
			error("Bad ID %c%c%c%c found in directory!", blocktype&0xFF, blocktype>>8, blocktype>>16, blocktype>>24);
			return;
		}
	}

	clearFileReadFailed(_fileHandle);

	if (numblock!=9)
		error("Not enough blocks read from directory");

	openRoom(-1);
}
#endif


#if defined(DOTT)
void Scumm::readArrayFromIndexFile() {
	int num;
	int a,b,c;

	while ((num = fileReadWordLE()) != 0) {
		a = fileReadWordLE();
		b = fileReadWordLE();
		c = fileReadWordLE();
		if (c==1)
			defineArray(num, 1, a, b);
		else
			defineArray(num, 5, a, b);
	}
}

#endif

#if defined(DOTT)

void Scumm::readResTypeList(int id, uint32 tag, const char *name) {
	int num,i;
	
	debug(9, "readResTypeList(%d,%x,%s)",id,FROM_LE_32(tag),name);

	num = fileReadWordLE();
	assert(num == res.num[id]);

	fileRead(_fileHandle, res.roomno[id], num*sizeof(uint8));
	fileRead(_fileHandle, res.roomoffs[id], num*sizeof(uint32));
#if defined(SCUMM_BIG_ENDIAN)
	for (i=0; i<num; i++)
		res.roomoffs[id][i] = FROM_LE_32(res.roomoffs[id][i]);
#endif

}

	

#else

void Scumm::readResTypeList(int id, uint32 tag, const char *name) {
	int num;
	int i;
	
	debug(9, "readResTypeList(%d,%x,%s)",id,FROM_LE_32(tag),name);
	
	num = fileReadWordLE();

	if (num>=0xFF) {
		error("Too many %ss (%d) in directory", name, num);
	}

	allocResTypeData(id, tag, num, name, 1);
	
	fileRead(_fileHandle, res.roomno[id], num*sizeof(uint8));
	fileRead(_fileHandle, res.roomoffs[id], num*sizeof(uint32));

#if defined(SCUMM_BIG_ENDIAN)
	for (i=0; i<num; i++)
		res.roomoffs[id][i] = FROM_LE_32(res.roomoffs[id][i]);
#endif
}

#endif

void Scumm::allocResTypeData(int id, uint32 tag, int num, const char *name, int mode) {
	debug(9, "allocResTypeData(%d,%x,%d,%s,%d)",id,FROM_LE_32(tag),num,name,mode);
	assert(id>=0 && id<sizeof(res.mode)/sizeof(res.mode[0]));

	if (num>=512) {
		error("Too many %ss (%d) in directory", name, num);
	}

	res.mode[id] = mode;
	res.num[id] = num;
	res.tags[id] = tag;
	res.name[id] = name;
	res.address[id] = (byte**)alloc(num*sizeof(void*));
	res.flags[id] = (byte*)alloc(num*sizeof(byte));

	if (mode) {
		res.roomno[id] = (byte*)alloc(num*sizeof(byte));
		res.roomoffs[id] = (uint32*)alloc(num*sizeof(uint32));
	}
}

void Scumm::loadCharset(int no) {
	int i;
	byte *ptr;

	debug(9, "loadCharset(%d)",no);

	checkRange(_maxCharsets-1, 1, no, "Loading illegal charset %d");
	ensureResourceLoaded(6, no);
	ptr = getResourceAddress(6, no);

	for (i=0; i<15; i++) {
		_charsetData[no][i+1] = ptr[i+14];
	}
}

void Scumm::nukeCharset(int i) {
	checkRange(_maxCharsets-1, 1, i, "Nuking illegal charset %d");
	nukeResource(6, i);
}

void Scumm::ensureResourceLoaded(int type, int i) {
	void *addr;

	debug(9, "ensureResourceLoaded(%d,%d)", type, i);

	if (type==1 && i>127) {
		i = _resourceMapper[i&127];
	}

	if (i==0)
		return;

	addr = res.address[type][i];
	if (addr)
		return;

	loadResource(type, i);

	if (type==1 && i==_roomResource)
		_vars[VAR_ROOM_FLAG] = 1;
}

int Scumm::loadResource(int type, int index) {
	int roomNr, i;
	uint32 fileOffs;
	uint32 size, tag;
	
	debug(9, "loadResource(%d,%d)", type,index);

	roomNr = getResourceRoomNr(type, index);
	if (roomNr == 0 || index >= res.num[type]) {
		error("%s %d undefined", 
			res.name[type],index);
	}

	if (type==1) {
		fileOffs = 0;
	} else {
		fileOffs = res.roomoffs[type][index];
		if (fileOffs==0xFFFFFFFF)
			return 0;
	}
		
	do {
		for (i=0; i<5; i++) {
			openRoom(roomNr);

			fileSeek(_fileHandle, fileOffs + _fileOffset, SEEK_SET);
			if (type==4) {
				fileReadDwordLE();
				fileReadDwordLE();
				return readSoundResource(type, index);
			}
			
			tag = fileReadDword();

			if (tag != res.tags[type]) {
				error("%s %d not in room %d at %d+%d", 
					res.name[type], type, roomNr, _fileOffset, fileOffs);
			}

			size = fileReadDwordBE();
			fileSeek(_fileHandle, -8, SEEK_CUR);

			fileRead(_fileHandle, createResource(type, index, size), size);

			/* dump the resource */
#ifdef DUMP_SCRIPTS
			if(type==2) {
				dumpResource("script-", index, getResourceAddress(2, index));
			}
#endif

			if (!fileReadFailed(_fileHandle)) {
				_scummTimer = 0;
				return 1;
			}

			nukeResource(type, index);
		}

		error("Cannot read resource");
	} while(1);
}

int Scumm::readSoundResource(int type, int index) {
	uint32 resStart, size, tag, size2;
	byte *ptr;
	int i;

	debug(9, "readSoundResource(%d,%d)", type, index);

	resStart = 0;

	fileReadDwordLE();
	size = fileReadDwordBE();

	while (size>resStart) {
		tag = fileReadDword();
		size2 = fileReadDwordBE();
		
		resStart += size2 + 8;
		
		for (i=0,ptr=_soundTagTable; i<_numSoundTags; i++,ptr+=5) {
/* endian OK, tags are in native format */
			if (READ_UINT32_UNALIGNED(ptr) == tag) {
				fileSeek(_fileHandle, -8, SEEK_CUR);
				fileRead(_fileHandle,createResource(type, index, size2+8), size+8);
				return 1;
			}
		}

		fileSeek(_fileHandle, size2, SEEK_CUR);
	}

	res.roomoffs[type][index] = 0xFFFFFFFF;
	return 0;
}

int Scumm::getResourceRoomNr(int type, int index) {
	if (type==1)
		return index;
	return res.roomno[type][index];
}

byte *Scumm::getResourceAddress(int type, int index) {
	byte *ptr;

	debug(9, "getResourceAddress(%d,%d)", type, index);

	CHECK_HEAP

	validateResource("getResourceAddress", type, index);
	
	if (res.mode[type] && !res.address[type][index]) {
		ensureResourceLoaded(type, index);
	}	

	setResourceFlags(type, index, 1);

	ptr=(byte*)res.address[type][index];
	if (!ptr)
		return NULL;

	return ptr + sizeof(ResHeader);
}

void Scumm::setResourceFlags(int type, int index, byte flag) {
	res.flags[type][index] &= 0x80;
	res.flags[type][index] |= flag;
}

byte *Scumm::createResource(int type, int index, uint32 size) {
	byte *ptr;

	CHECK_HEAP

	debug(9, "createResource(%d,%d,%d)", type, index,size);

	if (size > 65536*4+37856)
		error("Invalid size allocating");

	validateResource("allocating", type, index);
	nukeResource(type, index);

	CHECK_HEAP
	
	ptr = (byte*)alloc(size + sizeof(ResHeader));
	if (ptr==NULL) {
		error("Out of memory while allocating %d", size);
	}

	res.address[type][index] = ptr;

	((ResHeader*)ptr)->size = size;

	setResourceFlags(type, index, 1);

	return ptr + sizeof(ResHeader); /* skip header */
}

void Scumm::validateResource(const char *str, int type, int index) {
	if (type<1 || type>15 || index<0 || index >= res.num[type]) {
		error("%d Illegal Glob type %d num %d", str, type, index);
	}
}

void Scumm::nukeResource(int type, int index) {
	
	debug(9, "nukeResource(%d,%d)", type, index);

	CHECK_HEAP
	assert( res.address[type] );
	assert( index>=0 && index < res.num[type]);

	if (res.address[type][index]) {
		free(res.address[type][index]);
		res.address[type][index] = 0;
	}
}

void Scumm::unkResourceProc() {
	int i;
	for (i=1; i<16; i++) {
	}
	/* TODO: not implemented */
	warning("unkResourceProc: not implemented");
}


byte *Scumm::findResource(uint32 tag, byte *searchin) {
	uint32 size;

	if (searchin) {
		searchin+=4;
		_findResSize = READ_BE_UINT32_UNALIGNED(searchin);
		_findResHeaderSize = 8;
		_findResPos = searchin+4;
		goto startScan;
	}

	do {
		size = READ_BE_UINT32_UNALIGNED(_findResPos+4);
		if ((int32)size <= 0) {
			error("(%c%c%c%c) Not found in %d... illegal block len %d", 
				tag&0xFF,(tag>>8)&0xFF,(tag>>16)&0xFF,(tag>>24)&0xFF,
				0,
				size);
			return NULL;
		}
		_findResHeaderSize += size;
		_findResPos += size;
		
startScan:;
		if (_findResHeaderSize >= _findResSize)
			return NULL;
/* endian OK, tags are in native format */
	} while (READ_UINT32_UNALIGNED(_findResPos) != tag);

	return _findResPos;
}

byte *Scumm::findResource2(uint32 tag, byte *searchin) {
	uint32 size;

	if (searchin) {
		searchin+=4;
		_findResSize2 = READ_BE_UINT32_UNALIGNED(searchin);
		_findResHeaderSize2 = 8;
		_findResPos2 = searchin+4;
		goto startScan;
	}

	do {
		size = READ_BE_UINT32_UNALIGNED(_findResPos2+4);
		if ((int32)size <= 0) {
			error("(%c%c%c%c) Not found in %d... illegal block len %d", 
				tag&0xFF,(tag>>8)&0xFF,(tag>>16)&0xFF,(tag>>24)&0xFF,
				0,
				size);
			return NULL;
		}
		_findResHeaderSize2 += size;
		_findResPos2 += size;
		
startScan:;
		if (_findResHeaderSize2 >= _findResSize2)
			return NULL;
/* endian OK, tags are in native format */
	} while (READ_UINT32_UNALIGNED(_findResPos2) != tag);
	return _findResPos2;
}

void Scumm::lock(int type, int i) {
	validateResource("Locking", type, i);
	res.flags[type][i] |= 0x80;

}

void Scumm::unlock(int type, int i) {
	validateResource("Unlocking", type, i);
	res.flags[type][i] &= ~0x7F;
}

void Scumm::loadPtrToResource(int type, int resindex, byte *source) {
	byte *ptr, *alloced;
	int i,len;

	nukeResource(type, resindex);

	len = getStringLen(source);

	if (len <= 1)
		return;

	alloced = createResource(type, resindex, len);

	if (!source) {
		alloced[0] = fetchScriptByte();
		for (i=1; i<len; i++)
			alloced[i] = *_scriptPointer++;
	} else {
		for(i=0; i<len; i++)
			alloced[i] = source[i];
	}
}

void Scumm::heapClear(int mode) {
	/* TODO: implement this */
	warning("heapClear: not implemented");
}


void Scumm::unkHeapProc2(int a, int b) {
	warning("unkHeapProc2: not implemented");
} 

void Scumm::unkResProc(int a, int b) {
	warning("unkResProc:not implemented");
}


#if defined(DOTT)
void Scumm::readMAXS() {
	_numVariables = fileReadWordLE();
	fileReadWordLE();
	_numBitVariables = fileReadWordLE();
	_numLocalObjects = fileReadWordLE();
	_numArray = fileReadWordLE();
	fileReadWordLE();
	_numVerbs = fileReadWordLE();
	_numFlObject = fileReadWordLE();
	_numInventory = fileReadWordLE();
	_numRooms = fileReadWordLE();
	_numScripts = fileReadWordLE();
	_numSounds = fileReadWordLE();
	_numCharsets = fileReadWordLE();
	_numCostumes = fileReadWordLE();
	_numGlobalObjects = fileReadWordLE();

	allocResTypeData(3, MKID('COST'), _numCostumes, "costume", 1);
	allocResTypeData(1, MKID('ROOM'), _numRooms, "room", 1);
	allocResTypeData(4, MKID('SOUN'), _numSounds, "sound", 1);
	allocResTypeData(2, MKID('SCRP'), _numScripts, "script", 1);
	allocResTypeData(6, MKID('CHAR'), _numCharsets, "charset", 1);
	allocResTypeData(5, MKID('NONE'),	_numInventory, "inventory", 0);
	allocResTypeData(8, MKID('NONE'), _numVerbs,"verb", 0);
	allocResTypeData(7, MKID('NONE'), _numArray,"array", 0);
	allocResTypeData(13, MKID('NONE'),_numFlObject,"flobject", 0);
	allocResTypeData(12,MKID('NONE'),10, "temp", 0);
	allocResTypeData(11,MKID('NONE'),5, "scale table", 0);
	allocResTypeData(9, MKID('NONE'),13,"actor name", 0);
	allocResTypeData(10, MKID('NONE'),10,"buffer", 0);
	allocResTypeData(14, MKID('NONE'),10,"boxes", 0);
	allocResTypeData(16, MKID('NONE'),50,"new name", 0);

	_objectFlagTable = (byte*)alloc(_numGlobalObjects);
	_inventory = (uint16*)alloc(_numInventory * sizeof(uint16));
	_arrays = (byte*)alloc(_numArray);
	_verbs = (VerbSlot*)alloc(_numVerbs * sizeof(VerbSlot));
	_objs = (ObjectData*)alloc(_numLocalObjects * sizeof(ObjectData));
	_vars = (int16*)alloc(_numVariables * sizeof(int16));
	_bitVars = (byte*)alloc(_numBitVariables >> 3);
	_newNames = (uint16*)alloc(50 * sizeof(uint16));
	_classData = (uint32*)alloc(_numGlobalObjects * sizeof(uint32));

	_numGlobalScripts = 200;
	_dynamicRoomOffsets = 1;
}
#endif


