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
#include "resource.h"
#include "scumm/sound.h"

#include <stdio.h>


uint16 newTag2Old(uint32 oldTag);


/* Open a room */
void Scumm::openRoom(int room)
{
	int room_offs, roomlimit;
	char buf[256];

	debug(9, "openRoom(%d)", room);

	/* Don't load the same room again */
	if (_lastLoadedRoom == room)
		return;
	_lastLoadedRoom = room;

	/* Room -1 means close file */
	if (room == -1) {
		_encbyte = 0;
		deleteRoomOffsets();
		_fileHandle.close();
		return;
	}

	/* Either xxx.lfl or monkey.xxx file name */
	while (!_resFilePrefix) {
		if (_features & GF_SMALL_NAMES)
			roomlimit = 98;
		else
			roomlimit = 900;
		if (_features & GF_EXTERNAL_CHARSET && room >= roomlimit)
			room_offs = 0;
		else
			room_offs = room ? _roomFileOffsets[room] : 0;

		if (room_offs == (int)0xFFFFFFFF)
			break;

		if (room_offs != 0 && room != 0) {
			_fileOffset = _roomFileOffsets[room];
			return;
		}
		if (!(_features & GF_SMALL_HEADER)) {

			if (_features & GF_AFTER_V7)
				sprintf(buf, "%s.la%d", _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
			else if (_features & GF_HUMONGOUS)
				sprintf(buf, "%s.he%.1d", _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
			else
				sprintf(buf, "%s.%.3d",  _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);

			_encbyte = (_features & GF_USE_KEY) ? 0x69 : 0;
		} else if (!(_features & GF_SMALL_NAMES)) {
			if (room == 0 || room >= 900) {
				sprintf(buf, "%.3d.lfl", room);
				_encbyte = 0;
				if (openResourceFile(buf)) {
					return;
				}
				askForDisk(buf);

			} else {
				sprintf(buf, "disk%.2d.lec",  res.roomno[rtRoom][room]);
				_encbyte = 0x69;
			}
		} else {
			sprintf(buf, "%.2d.lfl", room);
			if (_features & GF_OLD_BUNDLE)
				_encbyte = 0xFF;
			else
				_encbyte = 0;
		}

		if (openResourceFile(buf)) {
			if (room == 0)
				return;
			if (_features & GF_EXTERNAL_CHARSET && room >= roomlimit)
				return;
			readRoomsOffsets();
			_fileOffset = _roomFileOffsets[room];

			if (_fileOffset != 8)
				return;

			error("Room %d not in %s", room, buf);
			return;
		}
		askForDisk(buf);
	}

	do {
		sprintf(buf, "%.3d.lfl", room);
		_encbyte = 0;
		if (openResourceFile(buf))
			break;
		askForDisk(buf);
	} while (1);

	deleteRoomOffsets();
	_fileOffset = 0;							/* start of file */
}

/* Delete the currently loaded room offsets */
void Scumm::deleteRoomOffsets()
{
	if (!(_features & GF_SMALL_HEADER) && !_dynamicRoomOffsets)
		return;

	for (int i = 0; i < _maxRooms; i++) {
		if (_roomFileOffsets[i] != 0xFFFFFFFF)
			_roomFileOffsets[i] = 0;
	}
}

/* Read room offsets */
void Scumm::readRoomsOffsets()
{
	int num, room;

	debug(9, "readRoomOffsets()");

	deleteRoomOffsets();
	if (_features & GF_SMALL_NAMES)
		return;

	if (!(_features & GF_SMALL_HEADER)) {
		if (!_dynamicRoomOffsets)
			return;

		_fileHandle.seek(16, SEEK_SET);
	} else {
		_fileHandle.seek(12, SEEK_SET);	// Directly searching for the room offset block would be more generic...
	}

	num = _fileHandle.readByte();
	while (num) {
		num--;

		room = _fileHandle.readByte();
		if (_roomFileOffsets[room] != 0xFFFFFFFF) {
			_roomFileOffsets[room] = _fileHandle.readUint32LE();
		} else {
			_fileHandle.readUint32LE();
		}
	}
}

bool Scumm::openResourceFile(const char *filename)
{
	char buf[256];

	debug(9, "openResourceFile(%s)", filename);

	if (_fileHandle.isOpen() == true) {
		_fileHandle.close();
	}

	strcpy(buf, filename);
	_fileHandle.open(buf, getGameDataPath(), 1, _encbyte);

	return _fileHandle.isOpen();
}

void Scumm::askForDisk(const char *filename)
{
	error("ask Cannot find '%s'", filename);
}

void Scumm::readIndexFile()
{
	uint32 blocktype, itemsize;
	int numblock = 0;
	int num, i;

	debug(9, "readIndexFile()");

	openRoom(-1);
	openRoom(0);

	if (!(_features & GF_AFTER_V6)) {
		/* Figure out the sizes of various resources */
		while (!_fileHandle.eof()) {
			blocktype = fileReadDword();
			itemsize = _fileHandle.readUint32BE();
			if (_fileHandle.ioFailed())
				break;
			switch (blocktype) {
			case MKID('DOBJ'):
				_numGlobalObjects = _fileHandle.readUint16LE();
				itemsize -= 2;
				break;
			case MKID('DROO'):
				_numRooms = _fileHandle.readUint16LE();
				itemsize -= 2;
				break;

			case MKID('DSCR'):
				_numScripts = _fileHandle.readUint16LE();
				itemsize -= 2;
				break;

			case MKID('DCOS'):
				_numCostumes = _fileHandle.readUint16LE();
				itemsize -= 2;
				break;

			case MKID('DSOU'):
				_numSounds = _fileHandle.readUint16LE();
				itemsize -= 2;
				break;
			}
			_fileHandle.seek(itemsize - 8, SEEK_CUR);
		}
		_fileHandle.clearIOFailed();
		_fileHandle.seek(0, SEEK_SET);
	}

	while (1) {
		blocktype = fileReadDword();

		if (_fileHandle.ioFailed())
			break;
		itemsize = _fileHandle.readUint32BE();

		numblock++;

		switch (blocktype) {
		case MKID('DCHR'):
			readResTypeList(rtCharset, MKID('CHAR'), "charset");
			break;

		case MKID('DOBJ'):
			if (_features & GF_AFTER_V8)
				num = _fileHandle.readUint32LE();
			else
				num = _fileHandle.readUint16LE();
			assert(num == _numGlobalObjects);

			if (_features & GF_AFTER_V8) {	/* FIXME: Not sure.. */
				for (i = 0; i < num; i++) {
					_fileHandle.seek(40, SEEK_CUR);
					_objectStateTable[i] = _fileHandle.readByte();
					_objectRoomTable[i] = _fileHandle.readByte();
					_classData[i] = _fileHandle.readUint32LE();
				}
				memset(_objectOwnerTable, 0xFF, num);
			} else if (_features & GF_AFTER_V7) {
				_fileHandle.read(_objectStateTable, num);
				_fileHandle.read(_objectRoomTable, num);
				memset(_objectOwnerTable, 0xFF, num);
			} else {
				_fileHandle.read(_objectOwnerTable, num);
				for (i = 0; i < num; i++) {
					_objectStateTable[i] = _objectOwnerTable[i] >> OF_STATE_SHL;
					_objectOwnerTable[i] &= OF_OWNER_MASK;
				}
			}

			if (!(_features & GF_AFTER_V8)) {
				_fileHandle.read(_classData, num * sizeof(uint32));

				// Swap flag endian where applicable
#if defined(SCUMM_BIG_ENDIAN)
				for (i = 0; i != num; i++)
					_classData[i] = FROM_LE_32(_classData[i]);
#endif
			}
			break;

		case MKID('RNAM'):
			_fileHandle.seek(itemsize - 8, SEEK_CUR);
			break;

		case MKID('ANAM'):
			_numAudioNames = _fileHandle.readUint16LE();
			_audioNames = (char*)malloc(_numAudioNames * 9);
			_fileHandle.read(_audioNames, _numAudioNames * 9);
			break;

		case MKID('DROO'):
			readResTypeList(rtRoom, MKID('ROOM'), "room");
			break;

		case MKID('DRSC'):					// FIXME: Verify
			readResTypeList(rtRoomScripts, MKID('RMSC'), "room script");
			break;

		case MKID('DSCR'):
			readResTypeList(rtScript, MKID('SCRP'), "script");
			break;

		case MKID('DCOS'):
			readResTypeList(rtCostume, MKID('COST'), "costume");
			break;

		case MKID('MAXS'):
			readMAXS();
			break;

		case MKID('DSOU'):
			readResTypeList(rtSound, MKID('SOUN'), "sound");
			break;

		case MKID('AARY'):
			readArrayFromIndexFile();
			break;

		default:
			error("Bad ID %c%c%c%c found in directory!", blocktype & 0xFF,
						blocktype >> 8, blocktype >> 16, blocktype >> 24);
			return;
		}
	}

//  if (numblock!=9)
//    error("Not enough blocks read from directory");

	openRoom(-1);
}

void Scumm::readArrayFromIndexFile()
{
	int num;
	int a, b, c;

	while ((num = _fileHandle.readUint16LE()) != 0) {
		a = _fileHandle.readUint16LE();
		b = _fileHandle.readUint16LE();
		c = _fileHandle.readUint16LE();
		if (c == 1)
			defineArray(num, 1, a, b);
		else
			defineArray(num, 5, a, b);
	}
}

void Scumm::readResTypeList(int id, uint32 tag, const char *name)
{
	int num;
	int i;

	debug(9, "readResTypeList(%s,%x,%s)", resTypeFromId(id), FROM_LE_32(tag), name);

	if (_features & GF_AFTER_V8)
		num = _fileHandle.readUint32LE();
	else if (!(_features & GF_OLD_BUNDLE))
		num = _fileHandle.readUint16LE();
	else
		num = _fileHandle.readByte();

	if (1 || _features & GF_AFTER_V6) {
		if (num != res.num[id]) {
			error("Invalid number of %ss (%d) in directory", name, num);
		}
	} else {
		if (num >= 0xFF) {
			error("Too many %ss (%d) in directory", name, num);
		}
		allocResTypeData(id, tag, num, name, 1);
	}

	if (_features & GF_OLD_BUNDLE) {
		if (id == rtRoom){
			for (i = 0; i < num; i++)
				res.roomno[id][i] = i;
			_fileHandle.seek(num, SEEK_CUR);
		} else {
			for (i = 0; i < num; i++)
				res.roomno[id][i] = _fileHandle.readByte();
		}
		for (i = 0; i < num; i++)
			res.roomoffs[id][i] = _fileHandle.readUint16LE();
	} else if (_features & GF_SMALL_HEADER) {
		for (i = 0; i < num; i++) {
			res.roomno[id][i] = _fileHandle.readByte();
			res.roomoffs[id][i] = fileReadDword();
		}
	} else {
		_fileHandle.read(res.roomno[id], num * sizeof(uint8));
		_fileHandle.read(res.roomoffs[id], num * sizeof(uint32));
	}

#if defined(SCUMM_BIG_ENDIAN)
	for (i = 0; i < num; i++)
		res.roomoffs[id][i] = FROM_LE_32(res.roomoffs[id][i]);
#endif
}


void Scumm::allocResTypeData(int id, uint32 tag, int num, const char *name, int mode)
{
	debug(9, "allocResTypeData(%s/%s,%x,%d,%d)", resTypeFromId(id), name, FROM_LE_32(tag), num, mode);
	assert(id >= 0 && id < (int)(sizeof(res.mode) / sizeof(res.mode[0])));

	if (num >= 2000) {
		error("Too many %ss (%d) in directory", name, num);
	}

	res.mode[id] = mode;
	res.num[id] = num;
	res.tags[id] = tag;
	res.name[id] = name;
	res.address[id] = (byte **)calloc(num, sizeof(void *));
	res.flags[id] = (byte *)calloc(num, sizeof(byte));

	if (mode) {
		res.roomno[id] = (byte *)calloc(num, sizeof(byte));
		res.roomoffs[id] = (uint32 *)calloc(num, sizeof(uint32));
	}
}

void Scumm::loadCharset(int no)
{
	int i;
	byte *ptr;

	debug(9, "loadCharset(%d)", no);

	/* FIXME - hack around crash in Indy4 (occurs if you try to load after dieing) */
	if (_gameId == GID_INDY4 && no == 0)
		no = 1;

	assert(no < (int)sizeof(_charsetData) / 16);
	checkRange(_maxCharsets - 1, 1, no, "Loading illegal charset %d");

//  ensureResourceLoaded(rtCharset, no);
	ptr = getResourceAddress(rtCharset, no);

	for (i = 0; i < 15; i++) {
		_charsetData[no][i + 1] = ptr[i + 14];
	}
}

void Scumm::nukeCharset(int i)
{
	checkRange(_maxCharsets - 1, 1, i, "Nuking illegal charset %d");
	nukeResource(rtCharset, i);
}

void Scumm::ensureResourceLoaded(int type, int i)
{
	void *addr;

	debug(9, "ensureResourceLoaded(%s,%d)", resTypeFromId(type), i);

	if (type == rtRoom && i > 127 && !(_features & GF_AFTER_V7)) {
		i = _resourceMapper[i & 127];
	}

	if (i == 0)
		return;

	addr = res.address[type][i];
	if (addr)
		return;

	loadResource(type, i);

	if (!(_features & GF_AFTER_V7) && !(_features & GF_SMALL_HEADER))
		if (type == rtRoom && i == _roomResource)
			_vars[VAR_ROOM_FLAG] = 1;
}

int Scumm::loadResource(int type, int idx)
{
	int roomNr, i;
	uint32 fileOffs;
	uint32 size, tag;

	// debug(1, "loadResource(%s,%d)", resTypeFromId(type),idx);

	if (type == rtCharset && (_features & GF_SMALL_HEADER)) {
		loadCharset(idx);
		return (1);
	}

	roomNr = getResourceRoomNr(type, idx);

	if (idx >= res.num[type])
		error("%s %d undefined %d %d", res.name[type], idx, res.num[type], roomNr);

	if (roomNr == 0)
		roomNr = _roomResource;

	if (type == rtRoom) {
		fileOffs = 0;
	} else {
		fileOffs = res.roomoffs[type][idx];
		if (fileOffs == 0xFFFFFFFF)
			return 0;
	}

	for (i = 0; i < 5; i++) {
		openRoom(roomNr);

		_fileHandle.seek(fileOffs + _fileOffset, SEEK_SET);

		if (_features & GF_OLD_BUNDLE) {
			size = _fileHandle.readUint16LE();
		} else if (_features & GF_SMALL_HEADER) {
			if (!(_features & GF_SMALL_NAMES))
				_fileHandle.seek(8, SEEK_CUR);
			size = _fileHandle.readUint32LE();
			tag = _fileHandle.readUint16LE();
			_fileHandle.seek(-6, SEEK_CUR);
			/* FIXME */
			if ((type == rtSound) && (_gameId != GID_ZAK256))
				return readSoundResourceSmallHeader(type, idx);
		} else {
			if (type == rtSound) {
				_fileHandle.readUint32LE();
				_fileHandle.readUint32LE();
				return readSoundResource(type, idx);
			}

			tag = fileReadDword();

			if (tag != res.tags[type]) {
				error("%s %d not in room %d at %d+%d", res.name[type], type, roomNr, _fileOffset, fileOffs);
			}

			size = _fileHandle.readUint32BE();
			_fileHandle.seek(-8, SEEK_CUR);
		}
		_fileHandle.read(createResource(type, idx, size), size);

		/* dump the resource */
#ifdef DUMP_SCRIPTS
		if (type == rtScript) {
			dumpResource("script-", idx, getResourceAddress(rtScript, idx));
		}
#endif

		if (!_fileHandle.ioFailed()) {
			return 1;
		}

		nukeResource(type, idx);
	}

	error("Cannot read resource");
}

int Scumm::readSoundResource(int type, int idx)
{
	uint32 pos, total_size, size, tag, basetag;
	int pri, best_pri;
	uint32 best_size = 0, best_offs = 0;

	debug(9, "readSoundResource(%s,%d)", resTypeFromId(type), idx);

	pos = 0;

	basetag = fileReadDword();
	total_size = _fileHandle.readUint32BE();

	debug(8, "  basetag: %c%c%c%c, total_size=%d",
				(char)((basetag >> 24) & 0xff),
				(char)((basetag >> 16) & 0xff),
				(char)((basetag >> 8) & 0xff), (char)(basetag & 0xff), total_size);

	//if (_gameId == GID_SAMNMAX || _features & GF_AFTER_V7) {
	if (basetag == MKID('MIDI') || basetag == MKID('iMUS')) {
		_fileHandle.seek(-8, SEEK_CUR);
		_fileHandle.read(createResource(type, idx, total_size + 8), total_size + 8);
		return 1;
	} else if (basetag == MKID('SOU ')) {
		best_pri = -1;
		while (pos < total_size) {
			tag = fileReadDword();
			size = _fileHandle.readUint32BE() + 8;
			pos += size;

			pri = -1;

			switch (tag) {
			case MKID('SBL '):
				pri = 15;
				break;
			case MKID('ADL '):
				pri = 1;
				if (_use_adlib)
					pri = 10;
				break;
			case MKID('ROL '):
//				if (!_use_adlib)
					pri = 3;
				break;
			case MKID('GMD '):
//				if (!_use_adlib)
					pri = 4;
				break;
			case MKID('MAC '):
//				if (!_use_adlib)
					pri = 2;
				break;
			case MKID('SPK '):
				if (!_use_adlib)
					pri = 0;
				break;
			}

			debug(8, "    tag: %c%c%c%c, total_size=%d, pri=%d",
						(char)((tag >> 24) & 0xff),
						(char)((tag >> 16) & 0xff), (char)((tag >> 8) & 0xff), (char)(tag & 0xff), size, pri);


			if (pri > best_pri) {
				best_pri = pri;
				best_size = size;
				best_offs = _fileHandle.pos();
			}

			_fileHandle.seek(size - 8, SEEK_CUR);
		}

		if (best_pri != -1) {
			_fileHandle.seek(best_offs - 8, SEEK_SET);
			_fileHandle.read(createResource(type, idx, best_size), best_size);
			return 1;
		}
	} else if (FROM_LE_32(basetag) == 24) {
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.seek(-8, SEEK_CUR);
		_fileHandle.read(createResource(type, idx, total_size), total_size);
		return 1;
	} else if (basetag == MKID('Mac0')) {
		debug(1, "Found base tag Mac0 in sound %d, size %d", idx, total_size);
		debug(1, "It was at position %d", _fileHandle.pos());

		/* Offset
		   0x14, 0x1C, 0x20, 0x24 - offsets of channel 1/2/3/4 chunk-
		   Each channel has tag "Chan", followed by its length. At the end
		   of each chan follows either an empty "Done" chunk (length 0) or an
		   empty "Loop" chunk. Maybe "Loop" indicates the song should be
		   played forever?!?.

		   There can be various different subchunks it seems. The
		   following combinations appear in Monkey Island:
		   100: ORGA, TROM, BASS, 
		   101: ORGA, SHAK, BASS, 
		   103: PIPE, PIPE, PIPE, 
		   104: VIBE, WHIS, BASS, 
		   108: ORGA, MARI, BASS, 
		   110: ORGA, SHAK, VIBE, 
		   111: MARI, SHAK, BASS, 
		   115: PLUC, SHAK, WHIS, 
		   One guess is that these are instrument names: Organ, Marimba, Whistle...
		   Maybe there is a mapping table someplace? Maybe these are even mapped to
		   Mac1 type "instruments" ?

		   What follows are four byte "commands" it seems, like this (hex):
		   01 68 4F 49
		   01 68 00 40
		   01 68 4F 49
		   ...
		   01 68 00 40
		   02 1C 5B 40
		   00 B4 00 40
		   ...
		   01 68 37 3C
		   00 18 37 38
		   04 20 3E 34
		   01 68 4A 3C

		   More data:
		   00 09 3E 10
		   01 5F 00 40
		   00 9C 36 40
		   00 CC 00 40
		   00 18 42 49
		   00 18 45 3C
		   01 29 4A 3C
		   00 0F 00 40

		   Maybe I am mistaken when I think it's four byte, some other parts
		   seem to suggest it's 2 byte oriented, or even variable length...
		 */
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.read(createResource(type, idx, total_size), total_size - 8);
		return 1;
	} else if (basetag == MKID('Mac1')) {
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.read(createResource(type, idx, total_size), total_size - 8);
		return 1;
	} else if (basetag == MKID('DIGI')) {
		// Use in Putt-Putt Demo
		debug(1, "Found base tag DIGI in sound %d, size %d", idx, total_size);
		debug(1, "It was at position %d", _fileHandle.pos());

		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.read(createResource(type, idx, total_size), total_size - 8);
		return 1;
	} else if (basetag == MKID('Crea')) {
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.read(createResource(type, idx, total_size), total_size - 8);
		return 1;
	} else {
		fprintf(stderr, "WARNING: Unrecognized base tag 0x%08x in sound %d\n", basetag, idx);
	}
	res.roomoffs[type][idx] = 0xFFFFFFFF;
	return 0;
}

// FIXME: some default MIDI instruments for INDY3/MI1
static char OLD256_MIDI_HACK[] =
	// 0
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x00\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x00"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb0\x07\x64"      // Controller 7 = 100
	// 1
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x01\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x01"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb1\x07\x64"      // Controller 7 = 100
	// 2
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x02\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x02"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb2\x07\x64"      // Controller 7 = 100
	// 3
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x03\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x03"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb3\x07\x64"      // Controller 7 = 100
	// 4
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x04\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x04"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb4\x07\x64"      // Controller 7 = 100
	// 5
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x05\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x05"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb5\x07\x64"      // Controller 7 = 100
	// 6
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x06\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x06"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb6\x07\x64"      // Controller 7 = 100
	// 7
	"\x00\xf0\x14\x7d\x00"  // sysex 00: part on/off
	"\x07\x00\x03"          // part/channel
	"\x00\x00\x07\x0f\x00\x00\x08\x00\x00\x00\x00\x02\x00\x00\xf7"
	"\x04\xf0\x41\x7d\x10"  // sysex 16: set instrument
	"\x07"                  // part/channel
	"\x01\x06\x02\x0a\x08\x09\x0d\x08\x04\x04"
	"\x04\x06\x02\x02\x03\x07\x0f\x0d"
	"\x05\x04\x0c\x00\x03\x01\x01\x00"
	"\x00\x00\x01\x01\x0e\x00\x02\x02"
	"\x01\x00\x01\x00\x01\x02\x00\x01"
	"\x08\x00\x00\x00\x01\x02\x04\x00"
	"\x06\x02\x00\x00\x04\x00\x03\x02"
	"\x04\x00\x00\xf7"
	"\x00\xb7\x07\x64";     // Controller 7 = 100

int Scumm::readSoundResourceSmallHeader(int type, int idx)
{
	uint32 pos, total_size, size, dw, tag;
	uint32 best_size = 0, best_offs = 0;
	byte *ptr, *track, *instr;
	uint16 ticks, skip;

	debug(4, "readSoundResourceSmallHeader(%s,%d)", resTypeFromId(type), idx);

	//if (_rescache->readResource(roomNr, type, idx))
	//		return 1;

	total_size = size = _fileHandle.readUint32LE();
	tag = _fileHandle.readUint16LE();
	debug(4, "  tag='%c%c', size=%d",
					(char) (tag & 0xff),
					(char) ((tag >> 8) & 0xff), size);

	pos = 6;
	while (pos < total_size) {
		size = _fileHandle.readUint32LE();
		tag = _fileHandle.readUint16LE();
  	debug(4, "  tag='%c%c', size=%d",
					(char) (tag & 0xff),
					(char) ((tag >> 8) & 0xff), size);
		pos += size;

		// MI1 and Indy3 uses one or more nested SO resources, which contains AD and WA
		// resources.
		if ((tag == 0x4441) && !(best_offs)) { // AD
				best_size = size;
				best_offs = _fileHandle.pos();
		} else { // other AD, WA and nested SO resources
			if (tag == 0x4F53) {
				pos -= size;
				size = 6;
				pos += 6;
			}
		}
		_fileHandle.seek(size - 6, SEEK_CUR);
	}

	// AD resources have a header, instrument definitions and one MIDI track.
	// We build an 'ADL ' resource from that:
	//   8 bytes resource header
	//  16 bytes MDhd header
	//  14 bytes MThd header
	//   8 bytes MTrk header
	//   7 MIDI speed event
	//     + some default instruments
	// TODO:  - make some real MIDI instrument definitions
	//        - no sound looping
	//        - proper handling of the short AD resources format
	//        - check the LE/BE handling for platforms other than PC

	if (best_offs != 0) {
		_fileHandle.seek(best_offs - 6, SEEK_SET);

		ptr = createResource(type, idx, best_size);
		_fileHandle.read(ptr, best_size);

		ticks = READ_BE_UINT16(ptr + 9);
		size = best_size;
		if (size < 0x98) {
				// FIXME: OLD256 music file w/o instruments
				// perhaps we should use then the other "WA" resource in the "SO" resource
				// and play it raw
			skip = 0x0a;    // let's give it a try
		} else {
			skip = 0x98;
		}

		/* copy the instrument data in another memory area */
		if (size >= 0x19 + 8*16) {
			instr = (byte *)calloc(8 * 16, 1);
			if (instr)
				memcpy(instr, ptr + 0x19, 8*16);
		} else {
			instr = 0;
		}

		ptr  += skip;                     // size + instruments
		size -= skip;		          // drop instruments for now
		CHECK_HEAP 
		track = (byte *)calloc(size, 1);
		if (track == NULL) {
			error("Out of memory while allocating %d", size);
		}
		memcpy(track, ptr, size);         // saving MIDI track data

		nukeResource(type, idx);

		total_size = 8 + 16 + 14 + 8 + 7 + sizeof(OLD256_MIDI_HACK) - 1 + size;
		ptr = createResource(type, idx, total_size);
		memcpy(ptr, "ADL ", 4); ptr += 4;
		dw = READ_BE_UINT32(&total_size);
		memcpy(ptr, &dw, 4); ptr += 4;
		memcpy(ptr, "MDhd", 4); ptr += 4;
		ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 8;
		ptr += 4;
		dw = 0;
		memcpy(ptr, &dw, 4); ptr += 4;
		memcpy(ptr, &dw, 4); ptr += 4;
		memcpy(ptr, "MThd", 4); ptr += 4;
		ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 6;
		ptr += 4;
		ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 1; // MIDI format 0 with 1 track
		ptr += 4;
		memcpy(ptr, &ticks, 2); ptr += 2;  // FIXME: care of BE/LE needed ?
		memcpy(ptr, "MTrk", 4); ptr += 4;
		*ptr++ = ((sizeof(OLD256_MIDI_HACK) - 1 + size + 7) >> 24) & 0xFF;
		*ptr++ = ((sizeof(OLD256_MIDI_HACK) - 1 + size + 7) >> 16) & 0xFF;
		*ptr++ = ((sizeof(OLD256_MIDI_HACK) - 1 + size + 7) >>  8) & 0xFF;
		*ptr++ = ((sizeof(OLD256_MIDI_HACK) - 1 + size + 7)      ) & 0xFF;
		// speed hacks
		dw = 1100000 - (ticks * 10);
		if ((dw > 900000) || (dw < 200000))
			dw = 500000;  // for sanity
		debug(4, "  ticks = %d, speed = %ld", ticks, dw);
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);
		memcpy(ptr, OLD256_MIDI_HACK, sizeof(OLD256_MIDI_HACK) - 1);

		if (instr) {

			/* now fill in the instruments */
			for (int i = 0; i < 8; i++) {

				/* flags_1 */
				ptr[95 * i + 30 + 0] = (instr[i * 16 + 3] >> 4) & 0xf;
				ptr[95 * i + 30 + 1] = instr[i * 16 + 3] & 0xf;

				/* oplvl_1 */
				ptr[95 * i + 30 + 2] = (instr[i * 16 + 4] >> 4) & 0xf;
				ptr[95 * i + 30 + 3] = instr[i * 16 + 4] & 0xf;

				/* atdec_1 */
				ptr[95 * i + 30 + 4] = ((~instr[i * 16 + 5]) >> 4) & 0xf;
				ptr[95 * i + 30 + 5] = (~instr[i * 16 + 5]) & 0xf;

				/* sustrel_1 */
				ptr[95 * i + 30 + 6] = ((~instr[i * 16 + 6]) >> 4) & 0xf;
				ptr[95 * i + 30 + 7] = (~instr[i * 16 + 6]) & 0xf;

				/* waveform_1 */
				ptr[95 * i + 30 + 8] = (instr[i * 16 + 7] >> 4) & 0xf;
				ptr[95 * i + 30 + 9] = instr[i * 16 + 7] & 0xf;

				/* flags_2 */
				ptr[95 * i + 30 + 10] = (instr[i * 16 + 8] >> 4) & 0xf;
				ptr[95 * i + 30 + 11] = instr[i * 16 + 8] & 0xf;

				/* oplvl_2 */
				ptr[95 * i + 30 + 12] = 3;
				ptr[95 * i + 30 + 13] = 0xF;

				/* atdec_2 */
				ptr[95 * i + 30 + 14] = ((~instr[i * 16 + 10]) >> 4) & 0xf;
				ptr[95 * i + 30 + 15] = (~instr[i * 16 + 10]) & 0xf;

				/* sustrel_2 */
				ptr[95 * i + 30 + 16] = ((~instr[i * 16 + 11]) >> 4) & 0xf;
				ptr[95 * i + 30 + 17] = (~instr[i * 16 + 11]) & 0xf;

				/* waveform_2 */
				ptr[95 * i + 30 + 18] = (instr[i * 16 + 12] >> 4) & 0xf;
				ptr[95 * i + 30 + 19] = instr[i * 16 + 12] & 0xf;

				/* feedback */
				ptr[95 * i + 30 + 20] = (instr[i * 16 + 2] >> 4) & 0xf;
				ptr[95 * i + 30 + 21] = instr[i * 16 + 2] & 0xf;
			}

			free(instr);
		}

		ptr += sizeof(OLD256_MIDI_HACK) - 1;
		memcpy(ptr, track, size);
		free(track);
		//hexdump(ptr = getResourceAddress(type, idx), 32);
		return 1;
	}
	res.roomoffs[type][idx] = 0xFFFFFFFF;
	return 0;
}


int Scumm::getResourceRoomNr(int type, int idx)
{
	if (type == rtRoom)
		return idx;
	return res.roomno[type][idx];
}

byte *Scumm::getResourceAddress(int type, int idx)
{
	byte *ptr;

	CHECK_HEAP
	if (!validateResource("getResourceAddress", type, idx))
		return NULL;

	if (!res.address[type]) {
		debug(9, "getResourceAddress(%s,%d), res.address[type] == NULL", resTypeFromId(type), idx);
		return NULL;
	}

	if (res.mode[type] && !res.address[type][idx]) {
		ensureResourceLoaded(type, idx);
	}

	if (!(ptr = (byte *)res.address[type][idx])) {
		debug(9, "getResourceAddress(%s,%d) == NULL", resTypeFromId(type), idx);
		return NULL;
	}

	setResourceCounter(type, idx, 1);

	debug(9, "getResourceAddress(%s,%d) == %ld", resTypeFromId(type), idx, ptr + sizeof(MemBlkHeader));
	return ptr + sizeof(MemBlkHeader);
}

byte *Scumm::getStringAddress(int i)
{
	byte *b = getResourceAddress(rtString, i);
	if (!b)
		return b;

	if (_features & GF_NEW_OPCODES)
		return ((ArrayHeader *)b)->data;
	return b;
}



byte *Scumm::getStringAddressVar(int i)
{
	byte *addr;

	addr = getResourceAddress(rtString, _vars[i]);
	if (addr == NULL)
		error("NULL string var %d slot %d", i, _vars[i]);

	if (_features & GF_NEW_OPCODES)
		return ((ArrayHeader *)addr)->data;

	return (addr);
}

void Scumm::setResourceCounter(int type, int idx, byte flag)
{
	res.flags[type][idx] &= ~RF_USAGE;
	res.flags[type][idx] |= flag;
}

/* 2 bytes safety area to make "precaching" of bytes in the gdi drawer easier */
#define SAFETY_AREA 2

byte *Scumm::createResource(int type, int idx, uint32 size)
{
	byte *ptr;

	CHECK_HEAP
	debug(9, "createResource(%s,%d,%d)", resTypeFromId(type), idx, size);

	if (!validateResource("allocating", type, idx))
		return NULL;
	nukeResource(type, idx);

	expireResources(size);

	CHECK_HEAP
	ptr = (byte *)calloc(size + sizeof(MemBlkHeader) + SAFETY_AREA, 1);
	if (ptr == NULL) {
		error("Out of memory while allocating %d", size);
	}

	_allocatedSize += size;

	res.address[type][idx] = ptr;
	((MemBlkHeader *)ptr)->size = size;
	setResourceCounter(type, idx, 1);
	return ptr + sizeof(MemBlkHeader);	/* skip header */
}

bool Scumm::validateResource(const char *str, int type, int idx)
{
	if (type < rtFirst || type > rtLast || (uint) idx >= (uint) res.num[type]) {
		warning("%s Illegal Glob type %s (%d) num %d", str, resTypeFromId(type), type, idx);
		return false;
	}
	return true;
}

void Scumm::nukeResource(int type, int idx)
{
	byte *ptr;

	CHECK_HEAP
	if (!res.address[type])
		return;

	assert(idx >= 0 && idx < res.num[type]);

	if ((ptr = res.address[type][idx]) != NULL) {
		debug(9, "nukeResource(%s,%d)", resTypeFromId(type), idx);
		res.address[type][idx] = 0;
		res.flags[type][idx] = 0;
		_allocatedSize -= ((MemBlkHeader *)ptr)->size;
		free(ptr);
	}
}

byte *Scumm::findResourceData(uint32 tag, byte *ptr)
{
	if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(tag, ptr, 0);
	else
		ptr = findResource(tag, ptr, 0);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

int Scumm::getResourceDataSize(byte *ptr)
{
	if (ptr == NULL)
		return 0;

	if (_features & GF_SMALL_HEADER)
		return READ_LE_UINT32(ptr) - 6;
	else
		return READ_BE_UINT32_UNALIGNED(ptr - 4) - 8;
}

struct FindResourceState {
	uint32 size, pos;
	byte *ptr;
};

/* just O(N) complexity when iterating with this function */
byte *findResource(uint32 tag, byte *searchin)
{
	uint32 size;
	static FindResourceState frs;
	FindResourceState *f = &frs;	/* easier to make it thread safe like this */

	if (searchin) {
		f->size = READ_BE_UINT32_UNALIGNED(searchin + 4);
		f->pos = 8;
		f->ptr = searchin + 8;
		goto StartScan;
	}

	do {
		size = READ_BE_UINT32_UNALIGNED(f->ptr + 4);
		if ((int32)size <= 0)
			return NULL;

		f->pos += size;
		f->ptr += size;

	StartScan:
		if (f->pos >= f->size)
			return NULL;
	} while (READ_UINT32_UNALIGNED(f->ptr) != tag);

	return f->ptr;
}

byte *findResourceSmall(uint32 tag, byte *searchin)
{
	uint32 size;
	static FindResourceState frs;
	FindResourceState *f = &frs;	/* easier to make it thread safe like this */
	uint16 smallTag;

	smallTag = newTag2Old(tag);

	if (searchin) {
		f->size = READ_LE_UINT32(searchin);
		f->pos = 6;
		f->ptr = searchin + 6;
		goto StartScan;
	}

	do {
		size = READ_LE_UINT32(f->ptr);
		if ((int32)size <= 0)
			return NULL;

		f->pos += size;
		f->ptr += size;

	StartScan:
		if (f->pos >= f->size)
			return NULL;
	} while (READ_LE_UINT16(f->ptr + 4) != smallTag);

	return f->ptr;
}

byte *findResource(uint32 tag, byte *searchin, int idx)
{
	uint32 curpos, totalsize, size;

	assert(searchin);

	searchin += 4;
	totalsize = READ_BE_UINT32_UNALIGNED(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_UINT32_UNALIGNED(searchin) == tag && !idx--)
			return searchin;

		size = READ_BE_UINT32_UNALIGNED(searchin + 4);
		if ((int32)size <= 0) {
			error("(%c%c%c%c) Not found in %d... illegal block len %d",
						tag & 0xFF, (tag >> 8) & 0xFF, (tag >> 16) & 0xFF, (tag >> 24) & 0xFF, 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

byte *findResourceSmall(uint32 tag, byte *searchin, int idx)
{
	uint32 curpos, totalsize, size;
	uint16 smallTag;

	smallTag = newTag2Old(tag);

	assert(searchin);

	totalsize = READ_LE_UINT32(searchin);
	searchin += 6;
	curpos = 6;

	while (curpos < totalsize) {
		size = READ_LE_UINT32(searchin);

		if (READ_LE_UINT16(searchin + 4) == smallTag && !idx--)
			return searchin;

		if ((int32)size <= 0) {
			error("(%c%c%c%c) Not found in %d... illegal block len %d",
						tag & 0xFF, (tag >> 8) & 0xFF, (tag >> 16) & 0xFF, (tag >> 24) & 0xFF, 0, size);
			return NULL;
		}

		curpos += size;
		searchin += size;
	}

	return NULL;
}

void Scumm::lock(int type, int i)
{
	if (!validateResource("Locking", type, i))
		return;
	res.flags[type][i] |= RF_LOCK;

//  debug(1, "locking %d,%d", type, i);
}

void Scumm::unlock(int type, int i)
{
	if (!validateResource("Unlocking", type, i))
		return;
	res.flags[type][i] &= ~RF_LOCK;

//  debug(1, "unlocking %d,%d", type, i);
}

bool Scumm::isResourceInUse(int type, int i)
{
	if (!validateResource("isResourceInUse", type, i))
		return false;
	switch (type) {
	case rtRoom:
		return _roomResource == (byte)i;
	case rtScript:
		return isScriptInUse(i);
	case rtCostume:
		return isCostumeInUse(i);
	case rtSound:
		return _sound->isSoundRunning(i) != 0;
	default:
		return false;
	}
}

void Scumm::increaseResourceCounter()
{
	int i, j;
	byte counter;

	for (i = rtFirst; i <= rtLast; i++) {
		for (j = res.num[i]; --j >= 0;) {
			counter = res.flags[i][j] & RF_USAGE;
			if (counter && counter < RF_USAGE_MAX) {
				setResourceCounter(i, j, counter + 1);
			}
		}
	}
}

void Scumm::expireResources(uint32 size)
{
	int i, j;
	byte flag;
	byte best_counter;
	int best_type, best_res = 0;
	uint32 oldAllocatedSize;

//  return;

	if (_expire_counter != 0xFF) {
		_expire_counter = 0xFF;
		increaseResourceCounter();
	}

	if (size + _allocatedSize < _maxHeapThreshold)
		return;

	oldAllocatedSize = _allocatedSize;

	do {
		best_type = 0;
		best_counter = 2;

		for (i = rtFirst; i <= rtLast; i++)
			if (res.mode[i]) {
				for (j = res.num[i]; --j >= 0;) {
					flag = res.flags[i][j];
					if (!(flag & 0x80) && flag >= best_counter && res.address[i][j] && !isResourceInUse(i, j)) {
						best_counter = flag;
						best_type = i;
						best_res = j;
					}
				}
			}

		if (!best_type)
			break;
		nukeResource(best_type, best_res);
	} while (size + _allocatedSize > _minHeapThreshold);

	increaseResourceCounter();

	debug(5, "Expired resources, mem %d -> %d", oldAllocatedSize, _allocatedSize);
}

void Scumm::freeResources()
{
	int i, j;
	for (i = rtFirst; i <= rtLast; i++) {
		for (j = res.num[i]; --j >= 0;) {
			if (isResourceLoaded(i, j))
				nukeResource(i, j);
		}
		free(res.address[i]);
		free(res.flags[i]);
		free(res.roomno[i]);
		free(res.roomoffs[i]);
	}
}

void Scumm::loadPtrToResource(int type, int resindex, byte *source)
{
	byte *alloced;
	int i, len;

	nukeResource(type, resindex);

	len = getStringLen(source);

	if (len <= 0)
		return;

	alloced = createResource(type, resindex, len);

	if (!source) {
		alloced[0] = fetchScriptByte();
		for (i = 1; i < len; i++)
			alloced[i] = *_scriptPointer++;
	} else {
		for (i = 0; i < len; i++)
			alloced[i] = source[i];
	}
}

bool Scumm::isResourceLoaded(int type, int idx)
{
	if (!validateResource("isLoaded", type, idx))
		return false;
	return res.address[type][idx] != NULL;
}

void Scumm::resourceStats()
{
	int i, j;
	uint32 lockedSize = 0, lockedNum = 0;
	byte flag;

	for (i = rtFirst; i <= rtLast; i++)
		for (j = res.num[i]; --j >= 0;) {
			flag = res.flags[i][j];
			if (flag & 0x80 && res.address[i][j]) {
				lockedSize += ((MemBlkHeader *)res.address[i][j])->size;
				lockedNum++;
			}
		}

	printf("Total allocated size=%d, locked=%d(%d)\n", _allocatedSize, lockedSize, lockedNum);
}

void Scumm::heapClear(int mode)
{
}

void Scumm::unkHeapProc2(int a, int b)
{
}

void Scumm::readMAXS()
{
	if (_features & GF_AFTER_V8) {
		_fileHandle.seek(50 + 50, SEEK_CUR);
		_numVariables = _fileHandle.readUint32LE();	/* ? 1500 */
		_numBitVariables = _fileHandle.readUint32LE();	/* ? 2048 */
		_fileHandle.readUint32LE();					/* 40 */
		_numScripts = _fileHandle.readUint32LE();
		_numSounds = _fileHandle.readUint32LE();
		_numCharsets = _fileHandle.readUint32LE();
		_numCostumes = _fileHandle.readUint32LE();
		_numRooms = _fileHandle.readUint32LE();
		_numInventory = _fileHandle.readUint32LE();
		_numGlobalObjects = _fileHandle.readUint32LE();
		_numFlObject = _fileHandle.readUint32LE();
		_numLocalObjects = _fileHandle.readUint32LE();
		_numVerbs = _fileHandle.readUint32LE();
		_numNewNames = _fileHandle.readUint32LE();
		_fileHandle.readUint32LE();
		_fileHandle.readUint32LE();
		_numArray = _fileHandle.readUint32LE();

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2000;

		_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	} else if (_features & GF_AFTER_V7) {
		_fileHandle.seek(50 + 50, SEEK_CUR);
		_numVariables = _fileHandle.readUint16LE();
		_numBitVariables = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();
		_numGlobalObjects = _fileHandle.readUint16LE();
		_numLocalObjects = _fileHandle.readUint16LE();
		_numNewNames = _fileHandle.readUint16LE();
		_numVerbs = _fileHandle.readUint16LE();
		_numFlObject = _fileHandle.readUint16LE();
		_numInventory = _fileHandle.readUint16LE();
		_numArray = _fileHandle.readUint16LE();
		_numRooms = _fileHandle.readUint16LE();
		_numScripts = _fileHandle.readUint16LE();
		_numSounds = _fileHandle.readUint16LE();
		_numCharsets = _fileHandle.readUint16LE();
		_numCostumes = _fileHandle.readUint16LE();

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2000;

		_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	} else if (_features & GF_AFTER_V6) {
		_numVariables = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();
		_numBitVariables = _fileHandle.readUint16LE();
		_numLocalObjects = _fileHandle.readUint16LE();
		_numArray = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();
		_numVerbs = _fileHandle.readUint16LE();
		_numFlObject = _fileHandle.readUint16LE();
		_numInventory = _fileHandle.readUint16LE();
		_numRooms = _fileHandle.readUint16LE();
		_numScripts = _fileHandle.readUint16LE();
		_numSounds = _fileHandle.readUint16LE();
		_numCharsets = _fileHandle.readUint16LE();
		_numCostumes = _fileHandle.readUint16LE();
		_numGlobalObjects = _fileHandle.readUint16LE();
		_numNewNames = 50;

		_objectRoomTable = NULL;
		_numGlobalScripts = 200;

		_shadowPaletteSize = 256;
	} else {
		_numVariables = _fileHandle.readUint16LE();	/* 800 */
		_fileHandle.readUint16LE();						/* 16 */
		_numBitVariables = _fileHandle.readUint16LE();	/* 2048 */
		_numLocalObjects = _fileHandle.readUint16LE();	/* 200 */
		_numArray = 50;
		_numVerbs = 100;
		_numNewNames = 0;
		_objectRoomTable = NULL;

		_fileHandle.readUint16LE();						/* 50 */
		_numCharsets = _fileHandle.readUint16LE();	/* 9 */
		_fileHandle.readUint16LE();						/* 100 */
		_fileHandle.readUint16LE();						/* 50 */
		_numInventory = _fileHandle.readUint16LE();	/* 80 */
		_numGlobalScripts = 200;

		_shadowPaletteSize = 256;

		_numFlObject = 50;
	}

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);

	allocateArrays();
	_dynamicRoomOffsets = 1;
}

void Scumm::allocateArrays()
{
	// Note: Buffers are now allocated in scummMain to allow for
	//     early GUI init.

	_objectOwnerTable = (byte *)calloc(_numGlobalObjects, 1);
	_objectStateTable = (byte *)calloc(_numGlobalObjects, 1);
	_classData = (uint32 *)calloc(_numGlobalObjects, sizeof(uint32));
	_arrays = (byte *)calloc(_numArray, 1);
	_newNames = (uint16 *)calloc(_numNewNames, sizeof(uint16));

	_inventory = (uint16 *)calloc(_numInventory, sizeof(uint16));
	_verbs = (VerbSlot *)calloc(_numVerbs, sizeof(VerbSlot));
	_objs = (ObjectData *)calloc(_numLocalObjects, sizeof(ObjectData));
	_vars = (int16 *)calloc(_numVariables, sizeof(int16));
	_bitVars = (byte *)calloc(_numBitVariables >> 3, 1);

	allocResTypeData(rtCostume,
									 (_features & GF_NEW_COSTUMES) ? MKID('AKOS') :
									 MKID('COST'), _numCostumes, "costume", 1);

	allocResTypeData(rtRoom, MKID('ROOM'), _numRooms, "room", 1);
	allocResTypeData(rtRoomScripts, MKID('RMSC'), _numRooms, "room script", 1);
	allocResTypeData(rtSound, MKID('SOUN'), _numSounds, "sound", 1);
	allocResTypeData(rtScript, MKID('SCRP'), _numScripts, "script", 1);
	allocResTypeData(rtCharset, MKID('CHAR'), _numCharsets, "charset", 1);
	allocResTypeData(rtObjectName, MKID('NONE'), _numNewNames, "new name", 0);
	allocResTypeData(rtInventory, MKID('NONE'), _numInventory, "inventory", 0);
	allocResTypeData(rtTemp, MKID('NONE'), 10, "temp", 0);
	allocResTypeData(rtScaleTable, MKID('NONE'), 5, "scale table", 0);
	allocResTypeData(rtActorName, MKID('NONE'), NUM_ACTORS, "actor name", 0);
	allocResTypeData(rtVerb, MKID('NONE'), _numVerbs, "verb", 0);
	allocResTypeData(rtString, MKID('NONE'), _numArray, "array", 0);
	allocResTypeData(rtFlObject, MKID('NONE'), _numFlObject, "flobject", 0);
	allocResTypeData(rtMatrix, MKID('NONE'), 10, "boxes", 0);
}




bool Scumm::isGlobInMemory(int type, int idx)
{
	if (!validateResource("isGlobInMemory", type, idx))
		return false;

	return res.address[type][idx] != NULL;
}


uint16 newTag2Old(uint32 oldTag)
{
	switch (oldTag) {
	case (MKID('RMHD')):
		return (0x4448);	// HD
		break;
	case (MKID('IM00')):
		return (0x4D42);	// BM
		break;
	case (MKID('EXCD')):
		return (0x5845);	// EX
		break;
	case (MKID('ENCD')):
		return (0x4E45);	// EN
		break;
	case (MKID('SCAL')):
		return (0x4153);	// SA
		break;
	case (MKID('LSCR')):
		return (0x534C);	// LS
		break;
	case (MKID('OBCD')):
		return (0x434F);	// OC
		break;
	case (MKID('OBIM')):
		return (0x494F);	// OI
		break;
	case (MKID('SMAP')):
		return (0x4D42);	// BM
		break;
	case (MKID('CLUT')):
		return (0x4150);	// PA
		break;
	case (MKID('BOXD')):
		return (0x5842);	// BX
		break;
	default:
		return (0);
	}
}


char *Scumm::resTypeFromId(int id)
{
	static char buf[100];

	switch (id) {
	case rtRoom:
		sprintf(buf, "Room");
		break;
	case rtScript:
		sprintf(buf, "Script");
		break;
	case rtCostume:
		sprintf(buf, "Costume");
		break;
	case rtSound:
		sprintf(buf, "Sound");
		break;
	case rtInventory:
		sprintf(buf, "Inventory");
		break;
	case rtCharset:
		sprintf(buf, "Charset");
		break;
	case rtString:
		sprintf(buf, "String");
		break;
	case rtVerb:
		sprintf(buf, "Verb");
		break;
	case rtActorName:
		sprintf(buf, "ActorName");
		break;
	case rtBuffer:
		sprintf(buf, "Buffer");
		break;
	case rtScaleTable:
		sprintf(buf, "ScaleTable");
		break;
	case rtTemp:
		sprintf(buf, "Temp");
		break;
	case rtFlObject:
		sprintf(buf, "FlObject");
		break;
	case rtMatrix:
		sprintf(buf, "Matrix");
		break;
	case rtBox:
		sprintf(buf, "Box");
		break;
	case rtLast:
		sprintf(buf, "Last");
		break;
	case rtNumTypes:
		sprintf(buf, "NumTypes");
		break;
	default:
		sprintf(buf, "%d", id);
	}
	return buf;
}
