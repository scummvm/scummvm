/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
#include "common/map.h"
#include "common/str.h"
#include "gui/message.h"
#include "scumm/bundle.h"
#include "scumm/dialogs.h"
#include "scumm/imuse.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"
#include "sound/mididrv.h" // Need MD_ enum values

static uint16 newTag2Old(uint32 oldTag);
static const char *resTypeFromId(int id);

/* Open a room */
void Scumm::openRoom(int room) {
	int room_offs, roomlimit;
	bool result;
	char buf[128];
	char buf2[128] = "";
	byte encByte = 0;

	debug(9, "openRoom(%d)", room);
	assert(room >= 0);

	/* Don't load the same room again */
	if (_lastLoadedRoom == room)
		return;
	_lastLoadedRoom = room;

	/* Room -1 means close file */
	if (room == -1) {
		deleteRoomOffsets();
		_fileHandle.close();
		return;
	}

	/* Either xxx.lfl or monkey.xxx file name */
	while (1) {
		if (_features & GF_SMALL_NAMES)
			roomlimit = 98;
		else
			roomlimit = 900;
		if (_features & GF_EXTERNAL_CHARSET && room >= roomlimit)
			room_offs = 0;
		else
			room_offs = room ? _roomFileOffsets[room] : 0;

		if (room_offs == -1)
			break;

		if (room_offs != 0 && room != 0) {
			_fileOffset = _roomFileOffsets[room];
			return;
		}
		if (!(_features & GF_SMALL_HEADER)) {

			if (_features & GF_AFTER_HEV7) {
				sprintf(buf, "%s.he%.1d", _exe_name, room == 0 ? 0 : 1);
			} else if (_version >= 7) {
				if (room > 0 && (_version == 8))
					VAR(VAR_CURRENTDISK) = res.roomno[rtRoom][room];
				sprintf(buf, "%s.la%d", _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
				sprintf(buf2, "%s.%.3d", _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
			} else if (_features & GF_HUMONGOUS)
				sprintf(buf, "%s.he%.1d", _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
			else {
				sprintf(buf, "%s.%.3d",  _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
				if (_gameId == GID_SAMNMAX)
					sprintf(buf2, "%s.sm%.1d",  _exe_name, room == 0 ? 0 : res.roomno[rtRoom][room]);
			}

			encByte = (_features & GF_USE_KEY) ? 0x69 : 0;
		} else if (!(_features & GF_SMALL_NAMES)) {
			if (room == 0 || room >= 900) {
				sprintf(buf, "%.3d.lfl", room);
				encByte = 0;
				if (openResourceFile(buf, encByte)) {
					return;
				}
				askForDisk(buf, room == 0 ? 0 : res.roomno[rtRoom][room]);

			} else {
				sprintf(buf, "disk%.2d.lec", room == 0 ? 0 : res.roomno[rtRoom][room]);
				encByte = 0x69;
			}
		} else {
			sprintf(buf, "%.2d.lfl", room);
			encByte = (_features & GF_USE_KEY) ? 0xFF : 0;
		}

		result = openResourceFile(buf, encByte);
		if ((result == false) && (buf2[0]))
			result = openResourceFile(buf2, encByte);
			
		if (result) {
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
		askForDisk(buf, room == 0 ? 0 : res.roomno[rtRoom][room]);
	}

	do {
		sprintf(buf, "%.3d.lfl", room);
		encByte = 0;
		if (openResourceFile(buf, encByte))
			break;
		askForDisk(buf, room == 0 ? 0 : res.roomno[rtRoom][room]);
	} while (1);

	deleteRoomOffsets();
	_fileOffset = 0;		// start of file
}

void Scumm::closeRoom() {
	if (_lastLoadedRoom != -1) {
		_lastLoadedRoom = -1;
		deleteRoomOffsets();
		_fileHandle.close();
	}
}

/** Delete the currently loaded room offsets. */
void Scumm::deleteRoomOffsets() {
	if (!(_features & GF_SMALL_HEADER) && !_dynamicRoomOffsets)
		return;

	for (int i = 0; i < _maxRooms; i++) {
		if (_roomFileOffsets[i] != 0xFFFFFFFF)
			_roomFileOffsets[i] = 0;
	}
}

/** Read room offsets */
void Scumm::readRoomsOffsets() {
	int num, room, i;
	byte *ptr;

	debug(9, "readRoomOffsets()");

	deleteRoomOffsets();
	if (_features & GF_SMALL_NAMES)
		return;

	if (_features & GF_AFTER_HEV7) {
		num = READ_LE_UINT16(_HEV7RoomOffsets);
		ptr = _HEV7RoomOffsets + 2;
		for (i = 0; i < num; i++) {
			_roomFileOffsets[i] = READ_LE_UINT32(ptr);	
			ptr += 4;
		}
		return;
	}
	
	if (!(_features & GF_SMALL_HEADER)) {
		if (!_dynamicRoomOffsets)
			return;

		_fileHandle.seek(16, SEEK_SET);
	} else {
		_fileHandle.seek(12, SEEK_SET);	// Directly searching for the room offset block would be more generic...
	}

	num = _fileHandle.readByte();
	while (num--) {
		room = _fileHandle.readByte();
		if (_roomFileOffsets[room] != 0xFFFFFFFF) {
			_roomFileOffsets[room] = _fileHandle.readUint32LE();
		} else {
			_fileHandle.readUint32LE();
		}
	}
}

bool Scumm::openResourceFile(const char *filename, byte encByte) {
	debug(9, "openResourceFile(%s)", filename);

	if (_fileHandle.isOpen()) {
		_fileHandle.close();
	}

	_fileHandle.open(filename, getGameDataPath(), 1, encByte);

	return _fileHandle.isOpen();
}

void Scumm::askForDisk(const char *filename, int disknum) {
	char buf[128];

	if (_version == 8) {
		char result;

		_sound->_bundle->closeVoiceFile();
		_sound->_bundle->closeMusicFile();

#ifdef MACOSX
		sprintf(buf, "Cannot find file: '%s'\nPlease insert disc %d.\nHit OK to retry, Cancel to exit", filename, disknum);
#else
		sprintf(buf, "Cannot find file: '%s'\nInsert disc %d into drive %s\nHit OK to retry, Cancel to exit", filename, disknum, getGameDataPath());
#endif

		result = displayError(true, buf);
		if (result == 2)
			error("Cannot find file: '%s'", filename);
	} else { 
		sprintf(buf, "Cannot find file: '%s'", filename);
		InfoDialog dialog(_newgui, this, (char*)buf);
		runDialog(dialog);
		error("Cannot find file: '%s'", filename);
	}
}

void Scumm::readIndexFile() {
	uint32 blocktype, itemsize;
	int numblock = 0;
	int num, i;
	bool stop = false;

	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	if (_version <= 5) {
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

	while (!stop) {
		blocktype = fileReadDword();

		if (_fileHandle.ioFailed())
			break;
		itemsize = _fileHandle.readUint32BE();

		numblock++;

		switch (blocktype) {
		case MKID('DCHR'):
		case MKID('DIRF'):
			readResTypeList(rtCharset, MKID('CHAR'), "charset");
			break;
		
		case MKID('DOBJ'):
			if (_version == 8)
				num = _fileHandle.readUint32LE();
			else
				num = _fileHandle.readUint16LE();
			assert(num == _numGlobalObjects);

			if (_version == 8) {	/* FIXME: Not sure.. */
				char buffer[40];
				for (i = 0; i < num; i++) {
					_fileHandle.read(buffer, 40);
					if (buffer[0]) {
						// Add to object name-to-id map
						_objectIDMap[buffer] = i;
					}
					_objectStateTable[i] = _fileHandle.readByte();
					_objectRoomTable[i] = _fileHandle.readByte();
					_classData[i] = _fileHandle.readUint32LE();
				}
				memset(_objectOwnerTable, 0xFF, num);
			} else if (_version == 7) {
				_fileHandle.read(_objectStateTable, num);
				_fileHandle.read(_objectRoomTable, num);
				memset(_objectOwnerTable, 0xFF, num);
			} else {
				_fileHandle.read(_objectOwnerTable, num);
				for (i = 0; i < num; i++) {
					_objectStateTable[i] = _objectOwnerTable[i] >> OF_STATE_SHL;
					_objectOwnerTable[i] &= OF_OWNER_MASK;
				}
				if (_features & GF_AFTER_HEV7) {
					// _objectRoomTable
					_fileHandle.seek(num * 4, SEEK_CUR);
					//_fileHandle.read(_objectRoomTable, num * 4);
				}
			}
			
			if (_version != 8) {
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
		
		case MKID('DLFL'):
			i = _fileHandle.readUint16LE();
			_fileHandle.seek(-2, SEEK_CUR);
			_HEV7RoomOffsets = (byte *)calloc(2 + (i * 4), 1);
			_fileHandle.read(_HEV7RoomOffsets, (2 + (i * 4)) );
			break;

		case MKID('DIRM'):
			_fileHandle.seek(itemsize - 8, SEEK_CUR);
			break;
			
		case MKID('DIRI'):
			num = _fileHandle.readUint16LE();
			_fileHandle.seek(num + (8 * num), SEEK_CUR);
			break;

		case MKID('ANAM'):
			_numAudioNames = _fileHandle.readUint16LE();
			_audioNames = (char*)malloc(_numAudioNames * 9);
			_fileHandle.read(_audioNames, _numAudioNames * 9);
			break;

		case MKID('DROO'):
			readResTypeList(rtRoom, MKID('ROOM'), "room");
			break;

		case MKID('DIRR'):
			readResTypeList(rtRoom, MKID('RMDA'), "room");
			break;

		case MKID('DRSC'):					// FIXME: Verify
			readResTypeList(rtRoomScripts, MKID('RMSC'), "room script");
			break;

		case MKID('DSCR'):
		case MKID('DIRS'):
			readResTypeList(rtScript, MKID('SCRP'), "script");
			break;

		case MKID('DCOS'):
		case MKID('DIRC'):
			readResTypeList(rtCostume, MKID('COST'), "costume");
			break;

		case MKID('MAXS'):
			readMAXS();
			break;

		case MKID('DSOU'):
			readResTypeList(rtSound, MKID('SOUN'), "sound");
			break;

		case MKID('DIRN'):
			readResTypeList(rtSound, MKID('DIRN'), "sound");
			break;

		case MKID('AARY'):
			debug(3, "Going to call readArrayFromIndexFile (pos = 0x%08x)", _fileHandle.pos());
			readArrayFromIndexFile();
			debug(3, "After readArrayFromIndexFile (pos = 0x%08x)", _fileHandle.pos());
			break;

		default:
			error("Bad ID %c%c%c%c found in directory!",
						(byte)blocktype,
						(byte)(blocktype >> 8),
						(byte)(blocktype >> 16),
						(byte)(blocktype >> 24));
			return;
		}
	}

//  if (numblock!=9)
//    error("Not enough blocks read from directory");

	closeRoom();
}

void Scumm::readArrayFromIndexFile() {
	int num;
	int a, b, c;

	if (_version == 8) {
		while ((num = _fileHandle.readUint32LE()) != 0) {
			a = _fileHandle.readUint32LE();
			b = _fileHandle.readUint32LE();
			
			// FIXME - seems the COMI scripts have a bug related to array 436.
			// and visible in script 2015, room 20. Basically, the dimensions
			// are swapped in the definition of the array, but its obvious
			// that this must be a script bug simply by looking at the defintions
			// of other arrays and how they are used.
			// Talk to fingolfin if you have questions about this :-)
			if (num == 436)
				defineArray(num, 5, b, a);
			else
				defineArray(num, 5, a, b);
		}
	} else {
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
}

void Scumm::readResTypeList(int id, uint32 tag, const char *name) {
	int num;
	int i;

	debug(9, "readResTypeList(%s,%x,%s)", resTypeFromId(id), FROM_LE_32(tag), name);

	if (_version == 8)
		num = _fileHandle.readUint32LE();
	else if (!(_features & GF_OLD_BUNDLE))
		num = _fileHandle.readUint16LE();
	else
		num = _fileHandle.readByte();

	if (_features & GF_OLD_BUNDLE) {
		if (num >= 0xFF) {
			error("Too many %ss (%d) in directory", name, num);
		}
	} else {
		if (num != res.num[id]) {
			error("Invalid number of %ss (%d) in directory", name, num);
		}
	}

	if (_features & GF_OLD_BUNDLE) {
		if (id == rtRoom) {
			for (i = 0; i < num; i++)
				res.roomno[id][i] = i;
			_fileHandle.seek(num, SEEK_CUR);
		} else {
			for (i = 0; i < num; i++)
				res.roomno[id][i] = _fileHandle.readByte();
		}
		for (i = 0; i < num; i++) {
			res.roomoffs[id][i] = _fileHandle.readUint16LE();
			if (res.roomoffs[id][i] == 0xFFFF)
				res.roomoffs[id][i] = 0xFFFFFFFF;
		}

	} else if (_features & GF_SMALL_HEADER) {
		for (i = 0; i < num; i++) {
			res.roomno[id][i] = _fileHandle.readByte();
			res.roomoffs[id][i] = _fileHandle.readUint32LE();
		}
	} else {
		for (i = 0; i < num; i++) {
			res.roomno[id][i] = _fileHandle.readByte();
		}
		for (i = 0; i < num; i++) {
			res.roomoffs[id][i] = _fileHandle.readUint32LE();
		}
		if (_features & GF_AFTER_HEV7) {
			_fileHandle.seek(4 * num, SEEK_CUR); // FIXME what are these additional offsets
		}
	}
}

void Scumm::allocResTypeData(int id, uint32 tag, int num, const char *name, int mode) {
	debug(9, "allocResTypeData(%s/%s,%x,%d,%d)", resTypeFromId(id), name, FROM_LE_32(tag), num, mode);
	assert(id >= 0 && id < (int)(ARRAYSIZE(res.mode)));

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

void Scumm::loadCharset(int no) {
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
	if (_features & GF_SMALL_HEADER)
		ptr -= 12;

	for (i = 0; i < 15; i++) {
		_charsetData[no][i + 1] = ptr[i + 14];
	}
}

void Scumm::nukeCharset(int i) {
	checkRange(_maxCharsets - 1, 1, i, "Nuking illegal charset %d");
	nukeResource(rtCharset, i);
}

void Scumm::ensureResourceLoaded(int type, int i) {
	void *addr = NULL;

	debug(9, "ensureResourceLoaded(%s,%d)", resTypeFromId(type), i);

	if (type == rtRoom && i > 0x7F && _version < 7) {
		i = _resourceMapper[i & 0x7F];
	}

	// FIXME - TODO: This check used to be "i==0". However, that causes
	// problems when using this function to ensure charset 0 is loaded.
	// This is done for many games, e.g. Zak256 or Indy3 (EGA and VGA).
	// For now we restrict the check to anything which is not a charset.
	// Question: Why was this check like that in the first place?
	// Answer: costumes with an index of zero in the newer games at least.
	// TODO: determine why the heck anything would try to load a costume
	// with id 0. Is that "normal", or is it caused by yet another bug in
	// our code base? After all we also have to add special cases for many
	// of our script opcodes that check for the (invalid) actor 0... so 
	// maybe both issues are related...
	if (type != rtCharset && i == 0)
		return;

	if (i <= res.num[type])
		addr = res.address[type][i];

	if (addr)
		return;

	loadResource(type, i);

	if (_version < 7 && !(_features & GF_SMALL_HEADER))
		if (type == rtRoom && i == _roomResource)
			VAR(VAR_ROOM_FLAG) = 1;
}

int Scumm::loadResource(int type, int idx) {
	int roomNr;
	uint32 fileOffs;
	uint32 size, tag;

	debug(2, "loadResource(%s,%d)", resTypeFromId(type),idx);

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
		if (_version == 8)
			fileOffs = 8;
		else
			fileOffs = 0;
	} else {
		fileOffs = res.roomoffs[type][idx];
		if (fileOffs == 0xFFFFFFFF)
			return 0;
	}

	openRoom(roomNr);

	_fileHandle.seek(fileOffs + _fileOffset, SEEK_SET);

	if (_features & GF_OLD_BUNDLE) {
		if ((_version == 3) && !(_features & GF_AMIGA) && (type == rtSound)) {
			return readSoundResourceSmallHeader(type, idx);
		} else {
			size = _fileHandle.readUint16LE();
			_fileHandle.seek(-2, SEEK_CUR);
		}
	} else if (_features & GF_SMALL_HEADER) {
		if (!(_features & GF_SMALL_NAMES))
			_fileHandle.seek(8, SEEK_CUR);
		size = _fileHandle.readUint32LE();
		tag = _fileHandle.readUint16LE();
		_fileHandle.seek(-6, SEEK_CUR);
		if ((type == rtSound) && !(_features & GF_AMIGA) && !(_features & GF_FMTOWNS)) {
			return readSoundResourceSmallHeader(type, idx);
		}
	} else {
		if (type == rtSound) {
			_fileHandle.readUint32LE();
			_fileHandle.readUint32LE();
			return readSoundResource(type, idx);
		}

		tag = fileReadDword();

		if (tag != res.tags[type]) {
			error("%s %d not in room %d at %d+%d in file %s",
					res.name[type], idx, roomNr,
					_fileOffset, fileOffs, _fileHandle.name());
		}

		size = _fileHandle.readUint32BE();
		_fileHandle.seek(-8, SEEK_CUR);
	}
	_fileHandle.read(createResource(type, idx, size), size);

	// dump the resource if requested
	if (_dumpScripts && type == rtScript) {
		dumpResource("script-", idx, getResourceAddress(rtScript, idx));
	}

	if (!_fileHandle.ioFailed()) {
		return 1;
	}

	nukeResource(type, idx);

	error("Cannot read resource");
}

int Scumm::readSoundResource(int type, int idx) {
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

	if (basetag == MKID('MIDI') || basetag == MKID('iMUS')) {
		if (_midiDriver != MD_PCSPK && _midiDriver != MD_PCJR) {
			_fileHandle.seek(-8, SEEK_CUR);
			_fileHandle.read(createResource(type, idx, total_size + 8), total_size + 8);
			return 1;
		}
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
				if (_midiDriver == MD_ADLIB)
					pri = 10;
				break;
			case MKID('AMI '):
				pri = 3;
				break;
			case MKID('ROL '):
				pri = 3;
				break;
			case MKID('GMD '):
				pri = 4;
				break;
			case MKID('MAC '):
				pri = 2;
				break;
			case MKID('SPK '):
				pri = -1;
//				if (_midiDriver == MD_PCSPK)
//					pri = 11;
				break;
			}

			if ((_midiDriver == MD_PCSPK || _midiDriver == MD_PCJR) && pri != 11)
				pri = -1;

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
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE() - 8;
		byte *ptr = (byte *)calloc(total_size, 1);
		_fileHandle.read(ptr, total_size);
//		dumpResource("sound-", idx, ptr);
		convertMac0Resource(type, idx, ptr, total_size);
		free(ptr);
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
	} else if (basetag == MKID('FMUS')) {
		// Used in 3DO version of puttputt joins the parade and probably others
		// Specifies a seperate file to be used for music from what I gather.
		int tmpsize;
		int i = 0;
		File dmuFile;
		char buffer[128];
		debug(1, "Found base tag FMUS in sound %d, size %d", idx, total_size);
		debug(1, "It was at position %d", _fileHandle.pos());
		
		_fileHandle.seek(4, SEEK_CUR);
		// HSHD size
		tmpsize = _fileHandle.readUint32BE();
		// skip to size part of the SDAT block
		_fileHandle.seek(tmpsize - 4, SEEK_CUR);
		// SDAT size
		tmpsize = _fileHandle.readUint32BE();
		
		// SDAT contains name of file we want
		for (i = 0; (buffer[i] != ' ') && (i < tmpsize - 8) ; i++) {
			buffer[i] = _fileHandle.readByte();
		}
		buffer[tmpsize - 11] = '\0';
		debug(1, "FMUS file %s", buffer);
		if (dmuFile.open(buffer, getGameDataPath()) == false) {
			warning("Can't open music file %s*", buffer);
			res.roomoffs[type][idx] = 0xFFFFFFFF;
			return 0;
		}
		dmuFile.seek(4, SEEK_SET);
		total_size = dmuFile.readUint32BE();
		debug(1, "dmu file size %d", total_size);
		dmuFile.seek(-8, SEEK_CUR);
		dmuFile.read(createResource(type, idx, total_size), total_size);
		dmuFile.close();
		return 1;
	} else if (basetag == MKID('Crea')) {
		_fileHandle.seek(-12, SEEK_CUR);
		total_size = _fileHandle.readUint32BE();
		_fileHandle.read(createResource(type, idx, total_size), total_size - 8);
		return 1;
		
	} else {
		warning("Unrecognized base tag 0x%08x in sound %d", basetag, idx);
	}
	res.roomoffs[type][idx] = 0xFFFFFFFF;
	return 0;
}

// Adlib MIDI-SYSEX to set MIDI instruments for small header games.
static byte ADLIB_INSTR_MIDI_HACK[95] = {
	0x00, 0xf0, 0x14, 0x7d, 0x00,  // sysex 00: part on/off
	0x00, 0x00, 0x03,              // part/channel  (offset  5)
	0x00, 0x00, 0x07, 0x0f, 0x00, 0x00, 0x08, 0x00, 
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0xf7,
	0x00, 0xf0, 0x41, 0x7d, 0x10,  // sysex 16: set instrument
	0x00, 0x01,                    // part/channel  (offset 28)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0xf7,
	0x00, 0xb0, 0x07, 0x64        // Controller 7 = 100 (offset 92)
};
			
static const byte map_param[7] = {
	0, 2, 3, 4, 8, 9, 0,
};

static const byte freq2note[128] = {
	/*128*/	6, 6, 6, 6,  
	/*132*/ 7, 7, 7, 7, 7, 7, 7,
	/*139*/ 8, 8, 8, 8, 8, 8, 8, 8, 8,
	/*148*/ 9, 9, 9, 9, 9, 9, 9, 9, 9,
	/*157*/ 10, 10, 10, 10, 10, 10, 10, 10, 10,
	/*166*/ 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
	/*176*/ 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
	/*186*/ 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
	/*197*/ 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
	/*209*/ 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	/*222*/ 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
	/*235*/ 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
	/*249*/ 18, 18, 18, 18, 18, 18, 18
};
			
static const uint16 num_steps_table[] = {
	1, 2, 4, 5,
	6, 7, 8, 9,
	10, 12, 14, 16,
	18, 21, 24, 30,
	36, 50, 64, 82,
	100, 136, 160, 192,
	240, 276, 340, 460,
	600, 860, 1200, 1600
};
int Scumm::convert_extraflags(byte * ptr, byte * src_ptr) {
	int flags = src_ptr[0];

	int t1, t2, t3, t4, time;
	int v1, v2, v3;

	if (!(flags & 0x80))
		return -1;

	t1 = (src_ptr[1] & 0xf0) >> 3;
	t2 = (src_ptr[2] & 0xf0) >> 3;
	t3 = (src_ptr[3] & 0xf0) >> 3 | (flags & 0x40 ? 0x80 : 0);
	t4 = (src_ptr[3] & 0x0f) << 1;
	v1 = (src_ptr[1] & 0x0f);
	v2 = (src_ptr[2] & 0x0f);
	v3 = 31;
	if ((flags & 0x7) == 0) {
	  v1 = v1 + 31 + 8;
	  v2 = v2 + 31 + 8;
	} else {
	  v1 = v1 * 2 + 31;
	  v2 = v2 * 2 + 31;
	}
	
	/* flags a */
	if ((flags & 0x7) == 6)
		ptr[0] = 0;
	else {
		ptr[0] = (flags >> 4) & 0xb;
		ptr[1] = map_param[flags & 0x7];
	}

	/* extra a */
	ptr[2] = 0;
	ptr[3] = 0;
	ptr[4] = t1 >> 4;
	ptr[5] = t1 & 0xf;
	ptr[6] = v1 >> 4;
	ptr[7] = v1 & 0xf;
	ptr[8] = t2 >> 4;
	ptr[9] = t2 & 0xf;
	ptr[10] = v2 >> 4;
	ptr[11] = v2 & 0xf;
	ptr[12] = t3 >> 4;
	ptr[13] = t3 & 0xf;
	ptr[14] = t4 >> 4;
	ptr[15] = t4 & 0xf;
	ptr[16] = v3 >> 4;
	ptr[17] = v3 & 0xf;

	time = num_steps_table[t1] + num_steps_table[t2]
		+ num_steps_table[t3 & 0x7f] + num_steps_table[t4];
	if (flags & 0x20) {
		int playtime = ((src_ptr[4] >> 4) & 0xf) * 118 + 
			(src_ptr[4] & 0xf) * 8;
		if (playtime > time)
			time = playtime;
	}
	/*
	time = ((src_ptr[4] >> 4) & 0xf) * 118 + 
		(src_ptr[4] & 0xf) * 8;
	*/
	return time;
}

#define kMIDIHeaderSize		46
static inline byte *writeMIDIHeader(byte *ptr, const char *type, int ppqn, int total_size) {
	uint32 dw = TO_BE_32(total_size);
	
	memcpy(ptr, type, 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;
	memcpy(ptr, "MDhd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 8;
	ptr += 4;
	memset(ptr, 0, 8), ptr += 8;
	memcpy(ptr, "MThd", 4); ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 6;
	ptr += 4;
	ptr[0] = 0; ptr[1] = 0; ptr[2] = 0; ptr[3] = 1; // MIDI format 0 with 1 track
	ptr += 4;
	
	*ptr++ = ppqn >> 8;
	*ptr++ = ppqn & 0xFF;

	memcpy(ptr, "MTrk", 4); ptr += 4;
	memcpy(ptr, &dw, 4); ptr += 4;

	return ptr;
}

static inline byte *writeVLQ(byte *ptr, int value) {
	if (value > 0x7f) {
		if (value > 0x3fff) {
			*ptr++ = (value >> 14) | 0x80;
			value &= 0x3fff;
		}
		*ptr++ = (value >> 7) | 0x80;
		value &= 0x7f;
	}
	*ptr++ = value;
	return ptr;
}

static inline byte Mac0ToGMInstrument(uint32 type) {
	switch (type) {
	case MKID('MARI'):	return 13;
	case MKID('PLUC'):	return 46;
	case MKID('HARM'):	return 23;
	case MKID('PIPE'):	return 110;	// 20 or 74 or 110 ?
	case MKID('TROM'):	return 58;
	case MKID('STRI'):	return 49;	// 49 or 50
	case MKID('HORN'):	return 61;	// 61 or 70
	case MKID('VIBE'):	return 12;
	case MKID('SHAK'):	return 78;
	case MKID('PANP'):	return 76;
	case MKID('WHIS'):	return 79;
	case MKID('ORGA'):	return 17;	// 17-21
	case MKID('BONG'):	return 116;
	case MKID('BASS'):	return 33;	// 33-40
	default:
		error("Unknown Mac0 instrument %c%c%c%c found",
					(byte)type,
					(byte)(type >> 8),
					(byte)(type >> 16),
					(byte)(type >> 24));
	}
}

void Scumm::convertMac0Resource(int type, int idx, byte *src_ptr, int size) {
	/*
	From Markus Magnuson (superqult) we got this information:
	Mac0
	---
	   4 bytes - 'SOUN'
	BE 4 bytes - block length
	
		   4 bytes  - 'Mac0'
		BE 4 bytes  - (blockLength - 27)
		   28 bytes - ???
	
		   do this three times (once for each channel):
			  4 bytes  - 'Chan'
		   BE 4 bytes  - channel length
			  4 bytes  - instrument name (e.g. 'MARI')
	
			  do this for ((chanLength-24)/4) times:
				 2 bytes  - note duration
				 1 byte   - note value
				 1 byte   - note velocity
	
			  4 bytes - ???
			  4 bytes - 'Loop'/'Done'
			  4 bytes - ???
	
	   1 byte - 0x09
	---
	
	Instruments (General Midi):
	"MARI" - Marimba (13)
	"PLUC" - Pizzicato Strings (46)
	"HARM" - Harmonica (23)
	"PIPE" - Church Organ? (20) or Flute? (74) or Bag Pipe (110)
	"TROM" - Trombone (58)
	"STRI" - String Ensemble (49 or 50)
	"HORN" - French Horn? (61) or English Horn? (70)
	"VIBE" - Vibraphone (12)
	"SHAK" - Shakuhachi? (78)
	"PANP" - Pan Flute (76)
	"WHIS" - Whistle (79) / Bottle (77)
	"ORGA" - Drawbar Organ (17; but could also be 18-21)
	"BONG" - Woodblock? (116)
	"BASS" - Bass (33-40)
	
	
	Now the task could be to convert this into MIDI, to be fed into iMuse.
	Or we do something similiar to what is done in Player_V3, assuming
	we can identify SFX in the MI datafiles for each of the instruments
	listed above.
	*/

#if 1
	byte *ptr = createResource(type, idx, size);
	memcpy(ptr, src_ptr, size);
#else
	const int ppqn = 480;
	byte *ptr, *start_ptr;
	
	int total_size = 0;
	total_size += kMIDIHeaderSize;	// Header
	total_size += 5;				// end of song sysex
	total_size += 3 * 2;			// Three programm change mesages
	
	int i, len;
	byte track_instr[3];
	int  current_note[3];
	int track_time[3];
	byte *track_data[3];
	int track_len[3];
	bool looped = false;

	// TODO: Decipher the unknown bytes in the header. For now, skip 'em
	src_ptr += 36;

	// Parse the three channels
	for (i = 0; i < 3; i++) {
		assert(READ_BE_UINT32(src_ptr) == MKID('Chan'));
		len = READ_BE_UINT32(src_ptr + 4);
		track_len[i] = (len - 24) / 4;
		track_instr[i] = Mac0ToGMInstrument(READ_BE_UINT32(src_ptr + 8));
		track_data[i] = src_ptr + 8;
		current_note[i] = -1;
		track_time[i] = -1;
		src_ptr += len;
		looped = (READ_BE_UINT32(src_ptr - 8) == MKID('Loop'));
		
		// For each note event, we need up to 3 bytes for the VLQ, and 3 bytes
		// for the note on. Finally, up to 3 bytes for the note off.
		// That means up to 9 bytes may be used for each note.
		total_size += 9 * track_len[i];
	}
	assert(*src_ptr == 0x09);
	
	// Create sound resource
	start_ptr = createResource(type, idx, total_size);
	
	// Insert MIDI header
	ptr = writeMIDIHeader(start_ptr, "GMD ", ppqn, total_size);

/*
	// Write a tempo change Meta event
	// 473 / 4 Hz, convert to micro seconds.
	// FIXME: This is copied from the SFX case in convertADResource()
	// and probably is not the proper value, but for now it's
	// sufficient to act as placeholder.
	uint32 dw = 1000000 * ppqn * 4 / 473;
	memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
	*ptr++ = (byte)((dw >> 16) & 0xFF);
	*ptr++ = (byte)((dw >> 8) & 0xFF);
	*ptr++ = (byte)(dw & 0xFF);
*/

	// Time 0
//	ptr = writeVLQ(ptr, 0);

	// Insert programm change status messages
	ptr[0] = 'C0'; ptr[1] = track_instr[0];
	ptr[2] = 'C1'; ptr[3] = track_instr[1];
	ptr[4] = 'C2'; ptr[5] = track_instr[2];
	ptr += 6;

	// TODO: now use the information computed above, and create a MIDI track, 
	// similiar to what is done in convertADResource
	// ...

	// Insert end of song sysex
	memcpy(ptr, "\x00\xff\x2f\x00\x00", 5); ptr += 5;
	
	assert(ptr <= start_ptr + total_size);
	
	// Rewrite MIDI header, this time with true size
	total_size = ptr - start_ptr;
	ptr = writeMIDIHeader(start_ptr, "GMD ", ppqn, total_size);
#endif
}

void Scumm::convertADResource(int type, int idx, byte *src_ptr, int size) {

	// We will ignore the PPQN in the original resource, because
	// it's invalid anyway. We use a constant PPQN of 480.
	const int ppqn = 480;
	uint32 dw;
	int i, ch;
	byte *ptr;
	int total_size = kMIDIHeaderSize + 7 + 8 * sizeof(ADLIB_INSTR_MIDI_HACK) + size;
	total_size += 24;	// Up to 24 additional bytes are needed for the jump sysex
	
	ptr = createResource(type, idx, total_size);

	src_ptr += 2;
	size -= 2;

	// 0x80 marks a music resource. Otherwise it's a SFX
	if (*src_ptr == 0x80) {
		byte ticks, play_once;
		byte num_instr;
		byte *channel, *instr, *track;

		ptr = writeMIDIHeader(ptr, "ADL ", ppqn, total_size);

		// The "speed" of the song
		ticks = *(src_ptr + 1);
		
		// Flag that tells us whether we should loop the song (0) or play it only once (1)
		play_once = *(src_ptr + 2);
		
		// Number of instruments used
		num_instr = *(src_ptr + 8);	// Normally 8
		
		// copy the pointer to instrument data
		channel = src_ptr + 9;
		instr   = src_ptr + 0x11;
		
		// skip over the rest of the header and copy the MIDI data into a buffer
		src_ptr  += 0x11 + 8 * 16;
		size -= 0x11 + 8 * 16;

		CHECK_HEAP 
			
		track = src_ptr;
		
		// Convert the ticks into a MIDI tempo. 
		// Unfortunate LOOM and INDY3 have different interpretation
		// of the ticks value.
		if (_gameId == GID_INDY3) {
			// Note: since we fix ppqn at 480, ppqn/473 is almost 1
			dw = 500000 * 256 / 473 * ppqn / ticks;
		} else if (_gameId == GID_LOOM) {
			dw = 500000 * ppqn / 4 / ticks;
		} else {
			dw = 500000 * 256 / ticks;
		}
		debug(4, "  ticks = %d, speed = %ld", ticks, dw);
			
		// Write a tempo change Meta event
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);
		
		// Copy our hardcoded instrument table into it
		// Then, convert the instrument table as given in this song resource
		// And write it *over* the hardcoded table.
		// Note: we deliberately.
		
		/* now fill in the instruments */
		for (i = 0; i < num_instr; i++) {
			ch = channel[i] - 1;
			if (ch < 0 || ch > 15)
				continue;

			if (instr[i*16 + 13])
				warning("Sound %d instrument %d uses percussion", idx, i);

			debug(4, "Sound %d: instrument %d on channel %d.", 
				  idx, i, ch);
			
			memcpy(ptr, ADLIB_INSTR_MIDI_HACK, 
				   sizeof(ADLIB_INSTR_MIDI_HACK));

			ptr[5]  += ch;
			ptr[28] += ch;
			ptr[92] += ch;
			
			/* flags_1 */
			ptr[30 + 0] = (instr[i * 16 + 3] >> 4) & 0xf;
			ptr[30 + 1] = instr[i * 16 + 3] & 0xf;
			
			/* oplvl_1 */
			ptr[30 + 2] = (instr[i * 16 + 4] >> 4) & 0xf;
			ptr[30 + 3] = instr[i * 16 + 4] & 0xf;
			
			/* atdec_1 */
			ptr[30 + 4] = ((~instr[i * 16 + 5]) >> 4) & 0xf;
			ptr[30 + 5] = (~instr[i * 16 + 5]) & 0xf;
			
			/* sustrel_1 */
			ptr[30 + 6] = ((~instr[i * 16 + 6]) >> 4) & 0xf;
			ptr[30 + 7] = (~instr[i * 16 + 6]) & 0xf;
			
			/* waveform_1 */
			ptr[30 + 8] = (instr[i * 16 + 7] >> 4) & 0xf;
			ptr[30 + 9] = instr[i * 16 + 7] & 0xf;
			
			/* flags_2 */
			ptr[30 + 10] = (instr[i * 16 + 8] >> 4) & 0xf;
			ptr[30 + 11] = instr[i * 16 + 8] & 0xf;
			
			/* oplvl_2 */
			ptr[30 + 12] = (instr[i * 16 + 9] >> 4) & 0xf;
			ptr[30 + 13] = instr[i * 16 + 9] & 0xf;
			
			/* atdec_2 */
			ptr[30 + 14] = ((~instr[i * 16 + 10]) >> 4) & 0xf;
			ptr[30 + 15] = (~instr[i * 16 + 10]) & 0xf;
			
			/* sustrel_2 */
			ptr[30 + 16] = ((~instr[i * 16 + 11]) >> 4) & 0xf;
			ptr[30 + 17] = (~instr[i * 16 + 11]) & 0xf;
			
			/* waveform_2 */
			ptr[30 + 18] = (instr[i * 16 + 12] >> 4) & 0xf;
			ptr[30 + 19] = instr[i * 16 + 12] & 0xf;
			
			/* feedback */
			ptr[30 + 20] = (instr[i * 16 + 2] >> 4) & 0xf;
			ptr[30 + 21] = instr[i * 16 + 2] & 0xf;
			ptr += sizeof(ADLIB_INSTR_MIDI_HACK);
		}
		
		// There is a constant delay of ppqn/3 before the music starts.
		if (ppqn / 3 >= 128)
			*ptr++ = (ppqn / 3 >> 7) | 0x80;
		*ptr++ = ppqn / 3 & 0x7f;
		
		// Now copy the actual music data 
		memcpy(ptr, track, size);
		ptr += size;
		
		if (!play_once) {
			// The song is meant to be looped. We achieve this by inserting just
			// before the song end a jump to the song start. More precisely we abuse
			// a S&M sysex, "maybe_jump" to achieve this effect. We could also
			// use a set_loop sysex, but it's a bit longer, a little more complicated,
			// and has no advantage either.
			
			// First, find the track end
			byte *end = ptr;
			ptr -= size;
			for (; ptr < end; ptr++) {
				if (*ptr == 0xff && *(ptr + 1) == 0x2f)
					break;
			}
			assert(ptr < end);
			
			// Now insert the jump. The jump offset is measured in ticks.
			// We have ppqn/3 ticks before the first note.

			const int jump_offset = ppqn / 3;
			memcpy(ptr, "\xf0\x13\x7d\x30\00", 5); ptr += 5;	// maybe_jump
			memcpy(ptr, "\x00\x00", 2); ptr += 2;			// cmd -> 0 means always jump
			memcpy(ptr, "\x00\x00\x00\x00", 4); ptr += 4;	// track -> there is only one track, 0
			memcpy(ptr, "\x00\x00\x00\x01", 4); ptr += 4;	// beat -> for now, 1 (first beat)
			// Ticks
			*ptr++ = (byte)((jump_offset >> 12) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 8) & 0x0F);
			*ptr++ = (byte)((jump_offset >> 4) & 0x0F);
			*ptr++ = (byte)(jump_offset & 0x0F);
			memcpy(ptr, "\x00\xf7", 2); ptr += 2;	// sysex end marker
		}
	} else {

		/* This is a sfx resource.  First parse it quickly to find the parallel
		 * tracks.
		 */
		ptr = writeMIDIHeader(ptr, "ASFX", ppqn, total_size);
	
		byte current_instr[3][14];
		int  current_note[3];
		int track_time[3];
		byte *track_data[3];
	
		int track_ctr = 0;
		byte chunk_type = 0;
		int delay, delay2, olddelay;
	
		// Write a tempo change Meta event
		// 473 / 4 Hz, convert to micro seconds.
		dw = 1000000 * ppqn * 4 / 473;
		memcpy(ptr, "\x00\xFF\x51\x03", 4); ptr += 4;
		*ptr++ = (byte)((dw >> 16) & 0xFF);
		*ptr++ = (byte)((dw >> 8) & 0xFF);
		*ptr++ = (byte)(dw & 0xFF);
	
		for (i = 0; i < 3; i++) {
			track_time[i] = -1;
			current_note[i] = -1;
		}
		while (size > 0) {
			assert(track_ctr < 3);
			track_data[track_ctr] = src_ptr;
			track_time[track_ctr] = 0;
			track_ctr++;
			while (size > 0) {
				chunk_type = *(src_ptr);
				if (chunk_type == 1) {
					src_ptr += 15;
					size -= 15;
				} else if (chunk_type == 2) {
					src_ptr += 11;
					size -= 11;
				} else if (chunk_type == 0x80) {
					src_ptr ++;
					size --;
				} else {
					break;
				}
			}
			if (chunk_type == 0xff)
				break;
			src_ptr++;
		}
		
		int curtime = 0;
		for (;;) {
			int mintime = -1;
			ch = -1;
			for (i = 0; i < 3; i++) {
				if (track_time[i] >= 0 && 
					(mintime == -1 || mintime > track_time[i])) {
					mintime = track_time[i];
					ch = i;
				}
			}
			if (mintime < 0)
				break;
	
			src_ptr = track_data[ch];
			chunk_type = *src_ptr;
	
	
			if (current_note[ch] >= 0) {
				delay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, delay);
				*ptr++ = 0x80 + ch; // key off channel;
				*ptr++ = current_note[ch];
				*ptr++ = 0;
				current_note[ch] = -1;
			}
			
	
			switch (chunk_type) {
			case 1:
				/* Instrument definition */
				memcpy(current_instr[ch], src_ptr+1, 14);
				src_ptr += 15;
				break;
	
			case 2:
				/* tone/parammodulation */
				memcpy(ptr, ADLIB_INSTR_MIDI_HACK, 
					   sizeof(ADLIB_INSTR_MIDI_HACK));
				
				ptr[5]  += ch;
				ptr[28] += ch;
				ptr[92] += ch;
	
				/* flags_1 */
				ptr[30 + 0] = (current_instr[ch][3] >> 4) & 0xf;
				ptr[30 + 1] = current_instr[ch][3] & 0xf;
				
				/* oplvl_1 */
				ptr[30 + 2] = (current_instr[ch][4] >> 4) & 0xf;
				ptr[30 + 3] = current_instr[ch][4] & 0xf;
				
				/* atdec_1 */
				ptr[30 + 4] = ((~current_instr[ch][5]) >> 4) & 0xf;
				ptr[30 + 5] = (~current_instr[ch][5]) & 0xf;
				
				/* sustrel_1 */
				ptr[30 + 6] = ((~current_instr[ch][6]) >> 4) & 0xf;
				ptr[30 + 7] = (~current_instr[ch][6]) & 0xf;
					
				/* waveform_1 */
				ptr[30 + 8] = (current_instr[ch][7] >> 4) & 0xf;
				ptr[30 + 9] = current_instr[ch][7] & 0xf;
					
				/* flags_2 */
				ptr[30 + 10] = (current_instr[ch][8] >> 4) & 0xf;
				ptr[30 + 11] = current_instr[ch][8] & 0xf;
				
				/* oplvl_2 */
				ptr[30 + 12] = ((current_instr[ch][9]) >> 4) & 0xf;
				ptr[30 + 13] = (current_instr[ch][9]) & 0xf;
				
				/* atdec_2 */
				ptr[30 + 14] = ((~current_instr[ch][10]) >> 4) & 0xf;
				ptr[30 + 15] = (~current_instr[ch][10]) & 0xf;
				
				/* sustrel_2 */
				ptr[30 + 16] = ((~current_instr[ch][11]) >> 4) & 0xf;
				ptr[30 + 17] = (~current_instr[ch][11]) & 0xf;
				
				/* waveform_2 */
				ptr[30 + 18] = (current_instr[ch][12] >> 4) & 0xf;
				ptr[30 + 19] = current_instr[ch][12] & 0xf;
				
				/* feedback */
				ptr[30 + 20] = (current_instr[ch][2] >> 4) & 0xf;
				ptr[30 + 21] = current_instr[ch][2] & 0xf;
	
				delay = mintime - curtime;
				curtime = mintime;
	
				{
					delay = convert_extraflags(ptr + 30 + 22, src_ptr + 1);
					delay2 = convert_extraflags(ptr + 30 + 40, src_ptr + 6);
					debug(4, "delays: %d / %d", delay, delay2);
					if (delay2 >= 0 && delay2 < delay)
						delay = delay2;
					if (delay == -1)
						delay = 0;
				}
								
				/* duration */
				ptr[30 + 58] = 0; // ((delay * 17 / 63) >> 4) & 0xf;
				ptr[30 + 59] = 0; // (delay * 17 / 63) & 0xf;
					
				ptr += sizeof(ADLIB_INSTR_MIDI_HACK);
	
				olddelay = mintime - curtime;
				curtime = mintime;
				ptr = writeVLQ(ptr, olddelay);
	
				{
					int freq = ((current_instr[ch][1] & 3) << 8)
						| current_instr[ch][0];
					if (!freq)
						freq = 0x80;
					freq <<= ((current_instr[ch][1] >> 2) & 7) + 1;
					int note = -11;
					while (freq >= 0x100) {
						note += 12;
						freq >>= 1;
					}
					debug(4, "Freq: %d (%x) Note: %d", freq, freq, note);
					if (freq < 0x80)
						note = 0;
					else
						note += freq2note[freq - 0x80];
	
					debug(4, "Note: %d", note);
					if (note <= 0)
						note = 1;
					else if (note > 127)
						note = 127;
					
					// Insert a note on event 
					*ptr++ = 0x90 + ch; // key on channel
					*ptr++ = note;
					*ptr++ = 63;
					current_note[ch] = note;
					track_time[ch] = curtime + delay;				
				}
				src_ptr += 11;
				break;
				
			case 0x80:
				track_time[ch] = -1;
				src_ptr ++;
				break;
				
			default:
				track_time[ch] = -1;
			}
			track_data[ch] = src_ptr;
		}
	}

	// Insert end of song sysex
	memcpy(ptr, "\x00\xff\x2f\x00\x00", 5); ptr += 5;
}


int Scumm::readSoundResourceSmallHeader(int type, int idx) {
	uint32 pos, total_size, size, tag;
	uint32 ad_size = 0, ad_offs = 0;
	uint32 ro_size = 0, ro_offs = 0;
	uint32 wa_size = 0, wa_offs = 0;

	debug(4, "readSoundResourceSmallHeader(%s,%d)", resTypeFromId(type), idx);

	if ((_gameId == GID_LOOM) && VAR(VAR_SOUNDCARD) == 4) {
		// Roland resources in Loom are tagless
		// So we add an RO tag to allow imuse to detect format
		byte *ptr, *src_ptr;
		ro_offs = _fileHandle.pos();
		ro_size = _fileHandle.readUint16LE();

		src_ptr = (byte *) calloc(ro_size - 4, 1);
		_fileHandle.seek(ro_offs + 4, SEEK_SET);
		_fileHandle.read(src_ptr, ro_size -4);

		ptr = createResource(type, idx, ro_size + 2);
		memcpy(ptr, "RO", 2); ptr += 2;
		memcpy(ptr, src_ptr, ro_size - 4); ptr += ro_size - 4;
		return 1;
	} else if (_features & GF_OLD_BUNDLE) {
		wa_offs = _fileHandle.pos();
		wa_size = _fileHandle.readUint16LE();
		_fileHandle.seek(wa_size - 2, SEEK_CUR);

		if (!(_features & GF_ATARI_ST || _features & GF_MACINTOSH)) {
			ad_offs = _fileHandle.pos();
			ad_size = _fileHandle.readUint16LE();
		}
		_fileHandle.seek(4, SEEK_CUR);
		total_size = wa_size + ad_size;
	} else {
		total_size = size = _fileHandle.readUint32LE();
		tag = _fileHandle.readUint16LE();
		debug(4, "  tag='%c%c', size=%d",
		      (char) (tag & 0xff),
		      (char) ((tag >> 8) & 0xff), size);
		
		if (tag == 0x4F52) { // RO
			ro_offs = _fileHandle.pos();
			ro_size = size;
		} else {
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
				if ((tag == 0x4441) && !(ad_offs)) { // AD
					ad_size = size;
					ad_offs = _fileHandle.pos();
				} else if ((tag == 0x4157) && !(wa_offs)) { // WA
					wa_size = size;
					wa_offs = _fileHandle.pos();
				} else { // other AD, WA and nested SO resources
					if (tag == 0x4F53) { // SO
						pos -= size;
						size = 6;
						pos += 6;
					}
				}
				_fileHandle.seek(size - 6, SEEK_CUR);
			}
		}
	}

	if ((_midiDriver == MD_ADLIB) && ad_offs != 0) {
		// AD resources have a header, instrument definitions and one MIDI track.
		// We build an 'ADL ' resource from that:
		//   8 bytes resource header
		//  16 bytes MDhd header
		//  14 bytes MThd header
		//   8 bytes MTrk header
		//   7 bytes MIDI tempo sysex
		//     + some default instruments
		byte *ptr;
		if (_features & GF_OLD_BUNDLE) {
			ptr = (byte *) calloc(ad_size - 4, 1);
			_fileHandle.seek(ad_offs + 4, SEEK_SET);
			_fileHandle.read(ptr, ad_size - 4);
			convertADResource(type, idx, ptr, ad_size - 4);
			free(ptr);
			return 1;
		} else {
			ptr = (byte *) calloc(ad_size - 6, 1);
			_fileHandle.seek(ad_offs, SEEK_SET);
			_fileHandle.read(ptr, ad_size - 6);
			convertADResource(type, idx, ptr, ad_size - 6);
			free(ptr);
			return 1;
		} 
	} else if (((_midiDriver == MD_PCJR) || (_midiDriver == MD_PCSPK)) && wa_offs != 0) {
		if (_features & GF_OLD_BUNDLE) {
			_fileHandle.seek(wa_offs, SEEK_SET);
			_fileHandle.read(createResource(type, idx, wa_size), wa_size);
		} else {
			_fileHandle.seek(wa_offs - 6, SEEK_SET);
			_fileHandle.read(createResource(type, idx, wa_size + 6), wa_size + 6);
		}
		return 1;
	} else if (ro_offs != 0) {
		_fileHandle.seek(ro_offs - 2, SEEK_SET);
		_fileHandle.read(createResource(type, idx, ro_size - 4), ro_size - 4);
		return 1;
	}
	res.roomoffs[type][idx] = 0xFFFFFFFF;
	return 0;
}


int Scumm::getResourceRoomNr(int type, int idx) {
	if (type == rtRoom)
		return idx;
	return res.roomno[type][idx];
}

int Scumm::getResourceSize(int type, int idx) {
	byte *ptr = getResourceAddress(type, idx);
	MemBlkHeader *hdr = (MemBlkHeader *)(ptr - sizeof(MemBlkHeader));
	
	return hdr->size;
}

byte *Scumm::getResourceAddress(int type, int idx) {
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

	debug(9, "getResourceAddress(%s,%d) == %p", resTypeFromId(type), idx, ptr + sizeof(MemBlkHeader));
	return ptr + sizeof(MemBlkHeader);
}

byte *Scumm::getStringAddress(int i) {
	byte *b = getResourceAddress(rtString, i);
	if (!b)
		return NULL;

	if (_features & GF_NEW_OPCODES)
		return ((ArrayHeader *)b)->data;
	return b;
}

byte *Scumm::getStringAddressVar(int i) {
	byte *addr;

	addr = getResourceAddress(rtString, _scummVars[i]);
	if (addr == NULL)
		// as this is used for string mapping in the gui
		// it must be allowed to return NULL
		// error("NULL string var %d slot %d", i, _scummVars[i]);
		return NULL;

	if (_features & GF_NEW_OPCODES)
		return ((ArrayHeader *)addr)->data;

	return (addr);
}

void Scumm::setResourceCounter(int type, int idx, byte flag) {
	res.flags[type][idx] &= ~RF_USAGE;
	res.flags[type][idx] |= flag;
}

/* 2 bytes safety area to make "precaching" of bytes in the gdi drawer easier */
#define SAFETY_AREA 2

byte *Scumm::createResource(int type, int idx, uint32 size) {
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

bool Scumm::validateResource(const char *str, int type, int idx) const {
	if (type < rtFirst || type > rtLast || (uint) idx >= (uint) res.num[type]) {
		warning("%s Illegal Glob type %s (%d) num %d", str, resTypeFromId(type), type, idx);
		return false;
	}
	return true;
}

void Scumm::nukeResource(int type, int idx) {
	byte *ptr;

	CHECK_HEAP
	if (!res.address[type])
		return;

	// FIXME hack around insanely high string resource idx's
	if ((_features & GF_HUMONGOUS) && (type == 7 && idx > res.num[7])) {
		warning("nukeResource: type %d index %d > max %d", type, idx,
				res.num[7]);
		return;
	}

	assert(idx >= 0 && idx < res.num[type]);

	if ((ptr = res.address[type][idx]) != NULL) {
		debug(9, "nukeResource(%s,%d)", resTypeFromId(type), idx);
		res.address[type][idx] = 0;
		res.flags[type][idx] = 0;
		_allocatedSize -= ((MemBlkHeader *)ptr)->size;
		free(ptr);
	}
}

const byte *Scumm::findResourceData(uint32 tag, const byte *ptr) {
	if (_features & GF_OLD_BUNDLE)
		error("findResourceData must not be used in GF_OLD_BUNDLE games");
	else if (_features & GF_SMALL_HEADER)
		ptr = findResourceSmall(tag, ptr);
	else
		ptr = findResource(tag, ptr);

	if (ptr == NULL)
		return NULL;
	return ptr + _resourceHeaderSize;
}

int Scumm::getResourceDataSize(const byte *ptr) const {
	if (ptr == NULL)
		return 0;

	if (_features & GF_OLD_BUNDLE)
		return READ_LE_UINT16(ptr) - 4;
	else if (_features & GF_SMALL_HEADER)
		return READ_LE_UINT32(ptr) - 6;
	else
		return READ_BE_UINT32(ptr - 4) - 8;
}

void Scumm::lock(int type, int i) {
	if (!validateResource("Locking", type, i))
		return;
	res.flags[type][i] |= RF_LOCK;

//  debug(1, "locking %d,%d", type, i);
}

void Scumm::unlock(int type, int i) {
	if (!validateResource("Unlocking", type, i))
		return;
	res.flags[type][i] &= ~RF_LOCK;

//  debug(1, "unlocking %d,%d", type, i);
}

bool Scumm::isResourceInUse(int type, int i) const {
	if (!validateResource("isResourceInUse", type, i))
		return false;
	switch (type) {
	case rtRoom:
		return _roomResource == (byte)i;
	case rtRoomScripts:
		return _roomResource == (byte)i;
	case rtScript:
		return isScriptInUse(i);
	case rtCostume:
		return isCostumeInUse(i);
	case rtSound:
		return _sound->isSoundInUse(i);
	default:
		return false;
	}
}

void Scumm::increaseResourceCounter() {
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

void Scumm::expireResources(uint32 size) {
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
					if (!(flag & RF_LOCK) && flag >= best_counter && res.address[i][j] && !isResourceInUse(i, j)) {
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

void Scumm::freeResources() {
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

void Scumm::loadPtrToResource(int type, int resindex, const byte *source) {
	byte *alloced;
	int i, len;

	nukeResource(type, resindex);

	len = resStrLen(source) + 1;

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

bool Scumm::isResourceLoaded(int type, int idx) const {
	if (!validateResource("isLoaded", type, idx))
		return false;
	return res.address[type][idx] != NULL;
}

void Scumm::resourceStats() {
	int i, j;
	uint32 lockedSize = 0, lockedNum = 0;
	byte flag;

	for (i = rtFirst; i <= rtLast; i++)
		for (j = res.num[i]; --j >= 0;) {
			flag = res.flags[i][j];
			if (flag & RF_LOCK && res.address[i][j]) {
				lockedSize += ((MemBlkHeader *)res.address[i][j])->size;
				lockedNum++;
			}
		}

	debug(1, "Total allocated size=%d, locked=%d(%d)", _allocatedSize, lockedSize, lockedNum);
}

void Scumm::readMAXS() {
	if (_version == 8) {                    // CMI
		_fileHandle.seek(50 + 50, SEEK_CUR);            // 176 - 8
		_numVariables = _fileHandle.readUint32LE();     // 1500
		_numBitVariables = _fileHandle.readUint32LE();  // 2048
		_fileHandle.readUint32LE();                     // 40
		_numScripts = _fileHandle.readUint32LE();       // 458
		_numSounds = _fileHandle.readUint32LE();        // 789
		_numCharsets = _fileHandle.readUint32LE();      // 1
		_numCostumes = _fileHandle.readUint32LE();      // 446
		_numRooms = _fileHandle.readUint32LE();         // 95
		_fileHandle.readUint32LE();                     // 80
		_numGlobalObjects = _fileHandle.readUint32LE(); // 1401
		_fileHandle.readUint32LE();                     // 60
		_numLocalObjects = _fileHandle.readUint32LE();  // 200
		_numNewNames = _fileHandle.readUint32LE();      // 100
		_numFlObject = _fileHandle.readUint32LE();      // 128
		_numInventory = _fileHandle.readUint32LE();     // 80
		_numArray = _fileHandle.readUint32LE();         // 200
		_numVerbs = _fileHandle.readUint32LE();         // 50

		_objectRoomTable = (byte *)calloc(_numGlobalObjects, 1);
		_numGlobalScripts = 2000;

		_shadowPaletteSize = NUM_SHADOW_PALETTE * 256;
	} else if (_version == 7) {
		_fileHandle.seek(50 + 50, SEEK_CUR);
		_numVariables = _fileHandle.readUint16LE();
		_numBitVariables = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();                      // 40 in FT; 16 in Dig
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
	// FIXME better check for the more recent windows based humongous games...
	} else if (_gameId == GID_PJSDEMO) {
		_fileHandle.readUint16LE();
		_numVariables = _fileHandle.readUint16LE();
		_numBitVariables = _fileHandle.readUint16LE();
		_numLocalObjects = _fileHandle.readUint16LE();
		_numArray = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();
		_fileHandle.readUint16LE();
		_numFlObject = _fileHandle.readUint16LE();
		_numInventory = _fileHandle.readUint16LE();
		_numRooms = _fileHandle.readUint16LE();
		_numScripts = _fileHandle.readUint16LE();
		_numSounds = _fileHandle.readUint16LE();
		_numCharsets = _fileHandle.readUint16LE();
		_numCostumes = _fileHandle.readUint16LE();
		_numGlobalObjects = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE(); 

		_objectRoomTable = (byte *)calloc(_numGlobalObjects * 4, 1);
		_numGlobalScripts = 200;
		_shadowPaletteSize = 256;
	} else if (_version == 6 && _gameId != GID_PJSDEMO) {
		_numVariables = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();                      // 16 in Sam/DOTT
		_numBitVariables = _fileHandle.readUint16LE();
		_numLocalObjects = _fileHandle.readUint16LE();
		_numArray = _fileHandle.readUint16LE();
		_fileHandle.readUint16LE();                      // 0 in Sam/DOTT
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
		_numVariables = _fileHandle.readUint16LE();      // 800
		_fileHandle.readUint16LE();                      // 16
		_numBitVariables = _fileHandle.readUint16LE();   // 2048
		_numLocalObjects = _fileHandle.readUint16LE();   // 200
		_numArray = 50;
		_numVerbs = 100;
		_numNewNames = 0;
		_objectRoomTable = NULL;

		_fileHandle.readUint16LE();                      // 50
		_numCharsets = _fileHandle.readUint16LE();       // 9
		_fileHandle.readUint16LE();                      // 100
		_fileHandle.readUint16LE();                      // 50
		_numInventory = _fileHandle.readUint16LE();      // 80
		_numGlobalScripts = 200;

		_shadowPaletteSize = 256;

		_numFlObject = 50;
	}

	if (_shadowPaletteSize)
		_shadowPalette = (byte *)calloc(_shadowPaletteSize, 1);

	allocateArrays();
	_dynamicRoomOffsets = true;
}

void Scumm::allocateArrays() {
	// Note: Buffers are now allocated in scummMain to allow for
	//     early GUI init.

	_objectOwnerTable = (byte *)calloc(_numGlobalObjects, 1);
	_objectStateTable = (byte *)calloc(_numGlobalObjects, 1);
	_classData = (uint32 *)calloc(_numGlobalObjects, sizeof(uint32));
	_newNames = (uint16 *)calloc(_numNewNames, sizeof(uint16));

	_inventory = (uint16 *)calloc(_numInventory, sizeof(uint16));
	_verbs = (VerbSlot *)calloc(_numVerbs, sizeof(VerbSlot));
	_objs = (ObjectData *)calloc(_numLocalObjects, sizeof(ObjectData));
	debug(2, "Allocated %d space in numObjects", _numLocalObjects);
	_scummVars = (int32 *)calloc(_numVariables, sizeof(int32));
	_bitVars = (byte *)calloc(_numBitVariables >> 3, 1);

	allocResTypeData(rtCostume, (_features & GF_NEW_COSTUMES) ? MKID('AKOS') : MKID('COST'),
								_numCostumes, "costume", 1);
	allocResTypeData(rtRoom, MKID('ROOM'), _numRooms, "room", 1);
	allocResTypeData(rtRoomScripts, MKID('RMSC'), _numRooms, "room script", 1);
	allocResTypeData(rtSound, MKID('SOUN'), _numSounds, "sound", 2);
	allocResTypeData(rtScript, MKID('SCRP'), _numScripts, "script", 1);
	allocResTypeData(rtCharset, MKID('CHAR'), _numCharsets, "charset", 1);
	allocResTypeData(rtObjectName, MKID('NONE'), _numNewNames, "new name", 0);
	allocResTypeData(rtInventory, MKID('NONE'), _numInventory, "inventory", 0);
	allocResTypeData(rtTemp, MKID('NONE'), 10, "temp", 0);
	allocResTypeData(rtScaleTable, MKID('NONE'), 5, "scale table", 0);
	allocResTypeData(rtActorName, MKID('NONE'), _numActors, "actor name", 0);
	allocResTypeData(rtVerb, MKID('NONE'), _numVerbs, "verb", 0);
	allocResTypeData(rtString, MKID('NONE'), _numArray, "array", 0);
	allocResTypeData(rtFlObject, MKID('NONE'), _numFlObject, "flobject", 0);
	allocResTypeData(rtMatrix, MKID('NONE'), 10, "boxes", 0);
}


bool Scumm::isGlobInMemory(int type, int idx) const{
	if (!validateResource("isGlobInMemory", type, idx))
		return false;

	return res.address[type][idx] != NULL;
}

void Scumm::dumpResource(const char *tag, int idx, const byte *ptr, int length) {
	char buf[256];
	File out;

	uint32 size;
	if (length >= 0)
		size = length;
	else if (_features & GF_OLD_BUNDLE)
		size = READ_LE_UINT16(ptr);
	else if (_features & GF_SMALL_HEADER)
		size = READ_LE_UINT32(ptr);
	else
		size = READ_BE_UINT32(ptr + 4);

#if defined(MACOS_CARBON)
	sprintf(buf, ":dumps:%s%d.dmp", tag, idx);
#else
	sprintf(buf, "dumps/%s%d.dmp", tag, idx);
#endif

	out.open(buf, "", 1);
	if (out.isOpen() == false) {
		out.open(buf, "", 2);
		if (out.isOpen() == false)
			return;
		out.write(ptr, size);
	}
	out.close();
}

ResourceIterator::ResourceIterator(const byte *searchin, bool smallHeader)
	: _ptr(searchin), _smallHeader(smallHeader) {
	assert(searchin);
	if (_smallHeader) {
		_size = READ_LE_UINT32(searchin);
		_pos = 6;
		_ptr = searchin + 6;
	} else {
		_size = READ_BE_UINT32(searchin + 4);
		_pos = 8;
		_ptr = searchin + 8;
	}
	
}

const byte *ResourceIterator::findNext(uint32 tag) {
	uint32 size = 0;
	const byte *result = 0;
	
	if (_smallHeader) {
		uint16 smallTag = newTag2Old(tag);
		do {
			if (_pos >= _size)
				return 0;
	
			result = _ptr;
			size = READ_LE_UINT32(result);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop
			
			_pos += size;
			_ptr += size;
		} while (READ_LE_UINT16(result + 4) != smallTag);
	} else {
		do {
			if (_pos >= _size)
				return 0;
	
			result = _ptr;
			size = READ_BE_UINT32(result + 4);
			if ((int32)size <= 0)
				return 0;	// Avoid endless loop
			
			_pos += size;
			_ptr += size;
		} while (READ_UINT32(result) != tag);
	}

	return result;
}

const byte *findResource(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;

	assert(searchin);

	searchin += 4;
	totalsize = READ_BE_UINT32(searchin);
	curpos = 8;
	searchin += 4;

	while (curpos < totalsize) {
		if (READ_UINT32(searchin) == tag)
			return searchin;

		size = READ_BE_UINT32(searchin + 4);
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

const byte *findResourceSmall(uint32 tag, const byte *searchin) {
	uint32 curpos, totalsize, size;
	uint16 smallTag;

	smallTag = newTag2Old(tag);
	if (smallTag == 0)
		return NULL;

	assert(searchin);

	totalsize = READ_LE_UINT32(searchin);
	searchin += 6;
	curpos = 6;

	while (curpos < totalsize) {
		size = READ_LE_UINT32(searchin);

		if (READ_LE_UINT16(searchin + 4) == smallTag)
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

uint16 newTag2Old(uint32 oldTag) {
	switch (oldTag) {
	case (MKID('RMHD')):
		return (0x4448);	// HD
	case (MKID('IM00')):
		return (0x4D42);	// BM
	case (MKID('EXCD')):
		return (0x5845);	// EX
	case (MKID('ENCD')):
		return (0x4E45);	// EN
	case (MKID('SCAL')):
		return (0x4153);	// SA
	case (MKID('LSCR')):
		return (0x534C);	// LS
	case (MKID('OBCD')):
		return (0x434F);	// OC
	case (MKID('OBIM')):
		return (0x494F);	// OI
	case (MKID('SMAP')):
		return (0x4D42);	// BM
	case (MKID('CLUT')):
		return (0x4150);	// PA
	case (MKID('BOXD')):
		return (0x5842);	// BX
	case (MKID('CYCL')):
		return (0x4343);	// CC
	default:
		return (0);
	}
}

const char *resTypeFromId(int id) {
	static char buf[100];

	switch (id) {
	case rtRoom:
		return "Room";
	case rtScript:
		return "Script";
	case rtCostume:
		return "Costume";
	case rtSound:
		return "Sound";
	case rtInventory:
		return "Inventory";
	case rtCharset:
		return "Charset";
	case rtString:
		return "String";
	case rtVerb:
		return "Verb";
	case rtActorName:
		return "ActorName";
	case rtBuffer:
		return "Buffer";
	case rtScaleTable:
		return "ScaleTable";
	case rtTemp:
		return "Temp";
	case rtFlObject:
		return "FlObject";
	case rtMatrix:
		return "Matrix";
	case rtBox:
		return "Box";
	case rtLast:
		return "Last";
	case rtNumTypes:
		return "NumTypes";
	default:
		sprintf(buf, "%d", id);
		return buf;
	}
}
