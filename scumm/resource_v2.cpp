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
 */

#include "stdafx.h"
#include "scumm.h"
#include "resource.h"


void Scumm_v2::readIndexFile()
{
	debug(9, "readIndexFile()");

	openRoom(-1);
	openRoom(0);

	if (fileReadWordLE() != 0x0100)
		warning("The magic id doesn't match\n");

	_numGlobalObjects = fileReadWordLE();
	fileSeek(_fileHandle, _numGlobalObjects, SEEK_CUR); // Skip object flags
	_numRooms = fileReadByte();
	fileSeek(_fileHandle, _numRooms * 3, SEEK_CUR);
	_numCostumes = fileReadByte();
	fileSeek(_fileHandle, _numCostumes * 3, SEEK_CUR);
	_numScripts = fileReadByte();
	fileSeek(_fileHandle, _numScripts * 3, SEEK_CUR);
	_numSounds = fileReadByte();

	clearFileReadFailed(_fileHandle);
	fileSeek(_fileHandle, 0, SEEK_SET);

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
	_numFlObject = 50;
	allocateArrays();

	fileReadWordLE(); /* version magic number */
	fileReadWordLE(); /* nb global objects */
	fileSeek(_fileHandle, _numGlobalObjects, SEEK_CUR); // Skip object flags
	readResTypeList(rtRoom, MKID('ROOM'), "room");
	readResTypeList(rtCostume, MKID('COST'), "costume");
	readResTypeList(rtScript, MKID('SCRP'), "script");
	readResTypeList(rtSound, MKID('SOUN'), "sound");

	openRoom(-1);
}
