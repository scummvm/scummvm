/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/engine.h"
#include "sky/disk.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/rnc_deco.h"

static const char *dataFilename = "sky.dsk";
static const char *dinnerFilename = "sky.dnr";

SkyDisk::SkyDisk(char *gameDataPath) {
	_gameDataPath = gameDataPath;

	_dataDiskHandle = new File();
	_dnrHandle = new File();

	uint32 entriesRead;

	_dnrHandle->open(dinnerFilename, _gameDataPath);
	if (_dnrHandle->isOpen() == false)
			error("Could not open %s%s!\n", _gameDataPath, dinnerFilename);

	if (!(_dinnerTableEntries = _dnrHandle->readUint32LE()))
		error("Error reading from sky.dnr!\n"); //even though it was opened correctly?!
	
	debug(1, "Entries in dinner table: %d", _dinnerTableEntries);

	_dinnerTableArea = (uint8 *)malloc(_dinnerTableEntries * 8);
	entriesRead = _dnrHandle->read(_dinnerTableArea, 8 * _dinnerTableEntries) / 8;

	if (entriesRead != _dinnerTableEntries)
		warning("entriesRead != dinnerTableEntries. [%d/%d]\n", entriesRead, _dinnerTableEntries);

	_dataDiskHandle->open(dataFilename, _gameDataPath);
	if (_dataDiskHandle->isOpen() == false) 
		error("Error opening %s%s!\n", _gameDataPath, dataFilename);
}

//load in file file_nr to address dest
//if dest == NULL, then allocate memory for this file
uint8 *SkyDisk::loadFile(uint16 fileNr, uint8 *dest) {
	
	uint8 cflag;
	int32 bytesRead;
	uint8 *filePtr, *inputPtr, *outputPtr;
	dataFileHeader fileHeader;

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	_compFile = fileNr;
	debug(2, "load file %d,%d (%d)", (fileNr >> 11), (fileNr & 2047), fileNr); 

	filePtr = getFileInfo(fileNr);
	if (filePtr == NULL) {
		printf("File %d not found!\n", fileNr);
		return NULL;
	}

	_fileFlags = READ_LE_UINT32((filePtr + 5));
	_fileSize = _fileFlags & 0x03fffff;
	_lastLoadedFileSize = _fileSize;
	
	_fileOffset = READ_LE_UINT32((filePtr + 2)) & 0x0ffffff;

	cflag = (uint8)((_fileOffset >> (23)) & 0x1);
	_fileOffset = (((1 << (23)) ^ 0xFFFFFFFF) & _fileOffset);

	if (cflag)
		_fileOffset <<= 4;

	_fixedDest = dest;
	_fileDest = dest;
	_compDest = dest;

	if (dest == NULL) //we need to allocate memory for this file
		_fileDest = (uint8 *)malloc(_fileSize);

	_dataDiskHandle->seek(_fileOffset, SEEK_SET);

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	//now read in the data
	bytesRead = _dataDiskHandle->read(_fileDest, 1 * _fileSize);

	if (bytesRead != (int32)_fileSize)
		printf("ERROR: Unable to read %d bytes from datadisk (%d bytes read)\n", _fileSize, bytesRead);

	cflag = (uint8)((_fileFlags >> (23)) & 0x1);

	//if cflag == 0 then file is compressed, 1 == uncompressed

	if (!cflag) {
		debug(2, "File is compressed...");

		memcpy(&fileHeader, _fileDest, sizeof(struct dataFileHeader));
		if ( (uint8)((FROM_LE_16(fileHeader.flag) >> 7) & 0x1)	 ) {
			debug(2, "with RNC!");

			_decompSize = (FROM_LE_16(fileHeader.flag) & 0xFFFFFF00) << 8;
			_decompSize |= FROM_LE_16((uint16)fileHeader.s_tot_size);

			if (_fixedDest == NULL) // is this valid?
				_compDest = (uint8 *)malloc(_decompSize);

			inputPtr = _fileDest;
			outputPtr = _compDest;

			if ( (uint8)(_fileFlags >> (22) & 0x1) ) //do we include the header?
				inputPtr += sizeof(struct dataFileHeader);
			else {
				memcpy(outputPtr, inputPtr, sizeof(struct dataFileHeader));
				inputPtr += sizeof(struct dataFileHeader);
				outputPtr += sizeof(struct dataFileHeader);
			}

			RncDecoder rncDecoder;
			int32 unPackLen = rncDecoder.unpackM1(inputPtr, outputPtr, 0);

			debug(3, "UnpackM1 returned: %d", unPackLen);

			if (unPackLen == 0) { //Unpack returned 0: file was probably not packed.
				if (_fixedDest == NULL)
					free(_compDest);
			
				return _fileDest;
			}

			if (! (uint8)(_fileFlags >> (22) & 0x1) ) { // include header?
				unPackLen += sizeof(struct dataFileHeader);

				if (unPackLen != (int32)_decompSize) {
					debug(1, "ERROR: invalid decomp size! (was: %d, should be: %d)", unPackLen, _decompSize);
				}
			}

			_lastLoadedFileSize = _decompSize; //including header
			
			if (_fixedDest == NULL)
				free(_fileDest);

		} else
			debug(2, "but not with RNC! (?!)");
	} else
		return _fileDest;

	return _compDest;
}

uint8 *SkyDisk::getFileInfo(uint16 fileNr) {
	
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)_dinnerTableArea;

	for (i = 0; i < _dinnerTableEntries; i++) {
		if (READ_LE_UINT16(dnrTbl16Ptr + (i * 4)) == fileNr) {
			debug(2, "file %d found!", fileNr);
			return (uint8 *)(dnrTbl16Ptr + (i * 4));
		}
	}

	// if file not found return NULL
	return (uint8 *)NULL;
}

void SkyDisk::dumpFile(uint16 fileNr) {
	char buf[128];
	File out;
	byte* filePtr;

	filePtr = loadFile(fileNr, NULL);
	sprintf(buf, "dumps/file-%d.dmp", fileNr);
	
	out.open(buf, "", 1);
	if (out.isOpen() == false) {
		out.open(buf, "", 2);
		if (out.isOpen() == false)
			return;
		out.write(filePtr, _lastLoadedFileSize);
	}
	out.close();
	free(filePtr);
}

uint32 SkyDisk::determineGameVersion() {
	//determine game version based on number of entries in dinner table
	switch (_dinnerTableEntries) {
	case 247:	
		//floppy demo (v0.0267)
		return 267;
	case 1404:
		//floppy (v0.0288)
		return 288;
	case 1413:
		//floppy (v0.0303)
		return 303;
	case 1445:
		//floppy (v0.0331)
		return 331;
	case 1711:
		//cd demo (v0.0365)
		return 365;
	case 5099:
		//cd (v0.0368)
		return 368;
	case 5097:
		//cd (v0.0372)
		return 372;
	default:
		//unknown version
		error("Unknown game version!");
		break;
	}
}

