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
#include "common/file.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/rnc_deco.h"

#define MAX_FILES_IN_LIST		60

const char *dataFilename = "sky.dsk";
const char *dinnerFilename = "sky.dnr";
uint8 *dinnerTableArea, *fixedDest, *fileDest, *compDest;
uint32 dinnerTableEntries, fileFlags, fileOffset, fileSize, decompSize, compFile;
uint16 buildList[MAX_FILES_IN_LIST];
uint32 loadedFileList[MAX_FILES_IN_LIST];

File *dataDiskHandle = new File();
File *dnrHandle = new File();

void SkyState::initialiseDisk() {

	uint32 entriesRead;

	dnrHandle->open(dinnerFilename, _gameDataPath);
	if (dnrHandle->isOpen() == false)
			error("Could not open %s%s!\n", _gameDataPath, dinnerFilename);

	if (!(dinnerTableEntries = dnrHandle->readUint32LE()))
		error("Error reading from sky.dnr!\n"); //even though it was opened correctly?!
	
	debug(1, "Entries in dinner table: %d", dinnerTableEntries);

	determineGameVersion(dinnerTableEntries);
		
	dinnerTableArea = (uint8 *)malloc(dinnerTableEntries * 8);
	entriesRead = dnrHandle->read(dinnerTableArea, 8 * dinnerTableEntries) / 8;

	if (entriesRead != dinnerTableEntries)
		warning("entriesRead != dinnerTableEntries. [%d/%d]\n", entriesRead, dinnerTableEntries);

	dataDiskHandle->open(dataFilename, _gameDataPath);
	if (dataDiskHandle->isOpen() == false) 
		error("Error opening %s%s!\n", _gameDataPath, dataFilename);
}

//load in file file_nr to address dest
//if dest == NULL, then allocate memory for this file
uint8 *SkyState::loadFile(uint16 fileNr, uint8 *dest) {
	
	uint8 cflag;
	int32 bytesRead;
	uint8 *filePtr, *inputPtr, *outputPtr;
	dataFileHeader fileHeader;

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	compFile = fileNr;
	debug(2, "load file %d,%d (%d)", (fileNr >> 11), (fileNr & 2047), fileNr); 

	filePtr = getFileInfo(fileNr);
	if (filePtr == NULL) {
		printf("File %d not found!\n", fileNr);
		return NULL;
	}

	fileFlags = READ_LE_UINT32((filePtr + 5));
	fileSize = fileFlags & 0x03fffff;
	_lastLoadedFileSize = fileSize;
	
	fileOffset = READ_LE_UINT32((filePtr + 2)) & 0x0ffffff;

	cflag = (uint8)((fileOffset >> (23)) & 0x1);
	fileOffset = (((1 << (23)) ^ 0xFFFFFFFF) & fileOffset);

	if (cflag)
		fileOffset <<= 4;

	fixedDest = dest;
	fileDest = dest;
	compDest = dest;

	if (dest == NULL) //we need to allocate memory for this file
		fileDest = (uint8 *)malloc(fileSize);

	dataDiskHandle->seek(fileOffset, SEEK_SET);

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	//now read in the data
	bytesRead = dataDiskHandle->read(fileDest, 1 * fileSize);

	if (bytesRead != (int32)fileSize)
		printf("ERROR: Unable to read %d bytes from datadisk (%d bytes read)\n", fileSize, bytesRead);

	cflag = (uint8)((fileFlags >> (23)) & 0x1);

	//if cflag == 0 then file is compressed, 1 == uncompressed

	if (!cflag) {
		debug(2, "File is compressed...");

		memcpy(&fileHeader, fileDest, sizeof(struct dataFileHeader));
		if ( (uint8)((FROM_LE_16(fileHeader.flag) >> 7) & 0x1)	 ) {
			debug(2, "with RNC!");

			decompSize = (FROM_LE_16(fileHeader.flag) & 0xFFFFFF00) << 8;
			decompSize |= FROM_LE_16((uint16)fileHeader.s_tot_size);

			if (fixedDest == NULL) // is this valid?
				compDest = (uint8 *)malloc(decompSize);

			inputPtr = fileDest;
			outputPtr = compDest;

			if ( (uint8)(fileFlags >> (22) & 0x1) ) //do we include the header?
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
				if (fixedDest == NULL)
					free(compDest);
			
				return fileDest;
			}

			if (! (uint8)(fileFlags >> (22) & 0x1) ) { // include header?
				unPackLen += sizeof(struct dataFileHeader);

				if (unPackLen != (int32)decompSize) {
					debug(1, "ERROR: invalid decomp size! (was: %d, should be: %d)", unPackLen, decompSize);
				}
			}

			_lastLoadedFileSize = decompSize; //including header
			
			if (fixedDest == NULL)
				free(fileDest);

		} else
			debug(2, "but not with RNC! (?!)");
	} else
		return fileDest;

	return compDest;
}

uint8 *SkyState::getFileInfo(uint16 fileNr) {
	
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)dinnerTableArea;

	for (i = 0; i < dinnerTableEntries; i++) {
		if (READ_LE_UINT16(dnrTbl16Ptr + (i * 4)) == fileNr) {
			debug(2, "file %d found!", fileNr);
			return (uint8 *)(dnrTbl16Ptr + (i * 4));
		}
	}

	// if file not found return NULL
	return (uint8 *)NULL;
}

void SkyState::determineGameVersion(uint32 dnrEntries) {

	//determine game version based on number of entries in dinner table
	
	switch (dnrEntries) {

	case 247:	
		//floppy demo (v0.0267)
		_isDemo = true;
		_isCDVersion = false;
		_gameVersion = 267;
		break;
		
	case 1404:
		//floppy (v0.0288)
		_isDemo = false;
		_isCDVersion = false;
		_gameVersion = 288;
		break;
	
	case 1445:
		//floppy (v0.0331)
		_isDemo = false;
		_isCDVersion = false;
		_gameVersion = 331;
		break;
	
	case 1711:
		//cd demo (v0.0365)
		_isDemo = true;
		_isCDVersion = true;
		_gameVersion = 365;
		break;
		
	case 5099:
		//cd (v0.0368)
		_isDemo = false;
		_isCDVersion = true;
		_gameVersion = 368;
		break;
	
	default:
		//unknown version
		error("Unknown game version!");
		break;
	}
	
}

void SkyState::dumpFile(uint16 fileNr) {
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
