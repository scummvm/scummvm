/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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


void Scumm_v2::readIndexFile()
{
	debug(9, "readIndexFile()");

	openRoom(-1);
	openRoom(0);

	if (_fileHandle.readUint16LE() != 0x0100)
		warning("The magic id doesn't match\n");

	_numGlobalObjects = _fileHandle.readUint16LE();
	_fileHandle.seek(_numGlobalObjects, SEEK_CUR); // Skip object flags
	_numRooms = _fileHandle.readByte();
	_fileHandle.seek(_numRooms * 3, SEEK_CUR);
	_numCostumes = _fileHandle.readByte();
	_fileHandle.seek(_numCostumes * 3, SEEK_CUR);
	_numScripts = _fileHandle.readByte();
	_fileHandle.seek(_numScripts * 3, SEEK_CUR);
	_numSounds = _fileHandle.readByte();

	_fileHandle.clearIOFailed();
	_fileHandle.seek(0, SEEK_SET);

	// FIXME - I'm not sure for those values yet, they will have to be rechecked

	_numVariables = 800;				/* 800 */
	_numBitVariables = 4096;			/* 2048 */
	_numLocalObjects = 200;				/* 200 */
	_numArray = 50;
	_numVerbs = 100;
	_numNewNames = 0;
	_objectRoomTable = NULL;
	_numCharsets = 9;					/* 9 */
	_numInventory = 80;					/* 80 */
	_numGlobalScripts = 200;

	_shadowPaletteSize = 256;
	_shadowPalette = (byte *) calloc(_shadowPaletteSize, 1);	// FIXME - needs to be removed later

	// Jamieson630: palManipulate variable initialization
	_palManipCounter = 0;
	_palManipPalette = 0; // Will allocate when needed
	_palManipIntermediatePal = 0; // Will allocate when needed

	_numFlObject = 50;
	allocateArrays();

	_fileHandle.readUint16LE(); /* version magic number */
	_fileHandle.readUint16LE(); /* nb global objects */
	_fileHandle.seek(_numGlobalObjects, SEEK_CUR); // Skip object flags
	readResTypeList(rtRoom, MKID('ROOM'), "room");
	readResTypeList(rtCostume, MKID('COST'), "costume");
	readResTypeList(rtScript, MKID('SCRP'), "script");
	readResTypeList(rtSound, MKID('SOUN'), "sound");

	openRoom(-1);
}
