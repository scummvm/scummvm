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

	if (dinnerTableEntries > 1600) 
		_isCDVersion = true;
	else
		_isCDVersion = false;
		
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
uint16 *SkyState::loadFile(uint16 fileNr, uint8 *dest) {
	
	uint8 cflag;
	int32 bytesRead;
	uint8 *filePtr, *inputPtr, *outputPtr;
	dataFileHeader fileHeader;

	#ifdef file_order_chk
		warning("File order checking not implemented yet!\n");
	#endif

	compFile = fileNr;
	debug(1, "load file %d,%d (%d)", (fileNr >> 11), (fileNr & 2047), fileNr); 

	filePtr = (uint8 *)getFileInfo(fileNr);
	if (filePtr == NULL) {
		printf("File %d not found!\n", fileNr);
		return NULL;
	}

	fileFlags = READ_LE_UINT32((filePtr + 5));
	fileSize = fileFlags & 0x03fffff;

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
		debug(1, "File is compressed...");

		memcpy(&fileHeader, fileDest, sizeof(struct dataFileHeader));
		if ( (uint8)((FROM_LE_16(fileHeader.flag) >> 7) & 0x1)	 ) {
			debug(1, "with RNC!");

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

			debug(2, "UnpackM1 returned: %d", unPackLen);

			if (unPackLen == 0) { //Unpack returned 0: file was probably not packed.
				if (fixedDest == NULL)
					free(compDest);
			
				return (uint16 *)fileDest;
			}

			if (! (uint8)(fileFlags >> (22) & 0x1) ) { // include header?
				unPackLen += sizeof(struct dataFileHeader);

				if (unPackLen != (int32)decompSize) {
					debug(1, "ERROR: invalid decomp size! (was: %d, should be: %d)", unPackLen, decompSize);
				}
			}

			if (fixedDest == NULL)
				free(fileDest);

		} else
			debug(1, "but not with RNC! (?!)");
	} else
		return (uint16 *)fileDest;

	return (uint16 *)compDest;
}

uint16 *SkyState::getFileInfo(uint16 fileNr) {
	
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)dinnerTableArea;

	for (i = 0; i < dinnerTableEntries; i++) {
		if (READ_LE_UINT16(dnrTbl16Ptr + (i * 4)) == fileNr) {
			debug(1, "file %d found!", fileNr);
			return (dnrTbl16Ptr + (i * 4));
		}
	}

	// if file is speech file then return NULL if not found
	printf("get_file_info() - speech file support not implemented yet!\n");
	return (uint16 *)NULL;
}

