/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/stdafx.h"
#include "scumm/scumm.h"
#include "scumm/intern.h"
#include "scumm/util.h"

namespace Scumm {

extern const char *resTypeFromId(int id);

void ScummEngine_v3old::readResTypeList(int id, uint32 tag, const char *name) {
	int num;
	int i;

	debug(9, "readResTypeList(%s,%s,%s)", resTypeFromId(id), tag2str(TO_BE_32(tag)), name);

	num = _fileHandle->readByte();

	if (num >= 0xFF) {
		error("Too many %ss (%d) in directory", name, num);
	}

	if (id == rtRoom) {
		for (i = 0; i < num; i++)
			res.roomno[id][i] = i;
		_fileHandle->seek(num, SEEK_CUR);
	} else {
		for (i = 0; i < num; i++)
			res.roomno[id][i] = _fileHandle->readByte();
	}
	for (i = 0; i < num; i++) {
		res.roomoffs[id][i] = _fileHandle->readUint16LE();
		if (res.roomoffs[id][i] == 0xFFFF)
			res.roomoffs[id][i] = 0xFFFFFFFF;
	}
}

void ScummEngine_v3old::readIndexFile() {
	int magic = 0;
	debug(9, "readIndexFile()");

	closeRoom();
	openRoom(0);

	magic = _fileHandle->readUint16LE();
	if (magic != 0x0100)
		error("The magic id doesn't match (0x%X)", magic);

	_numGlobalObjects = _fileHandle->readUint16LE();
	_fileHandle->seek(_numGlobalObjects * 4, SEEK_CUR);
	_numRooms = _fileHandle->readByte();
	_fileHandle->seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle->readByte();
	_fileHandle->seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle->readByte();
	_fileHandle->seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle->readByte();

	_fileHandle->clearIOFailed();
	_fileHandle->seek(0, SEEK_SET);

	readMAXS(0);
	allocateArrays();

	_fileHandle->readUint16LE(); /* version magic number */
	readGlobalObjects();
	readResTypeList(rtRoom, MKID('ROOM'), "room");
	readResTypeList(rtCostume, MKID('COST'), "costume");
	readResTypeList(rtScript, MKID('SCRP'), "script");
	readResTypeList(rtSound, MKID('SOUN'), "sound");

	closeRoom();
}

void ScummEngine_v3::readRoomsOffsets() {
}

void ScummEngine_v3::loadCharset(int no) {
	uint32 size;
	memset(_charsetData, 0, sizeof(_charsetData));

	checkRange(2, 0, no, "Loading illegal charset %d");
	closeRoom();

	Common::File file;
	char buf[20];

	sprintf(buf, "%02d.LFL", 99 - no);
	file.open(buf);

	if (file.isOpen() == false) {
		error("loadCharset(%d): Missing file charset: %s", no, buf);
	}

	size = file.readUint16LE();
	file.read(res.createResource(rtCharset, no, size), size);
}

} // End of namespace Scumm
