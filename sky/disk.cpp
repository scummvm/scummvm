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
#include "base/engine.h"
#include "sky/disk.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/rnc_deco.h"

static const char *dataFilename = "sky.dsk";
static const char *dinnerFilename = "sky.dnr";

SkyDisk::SkyDisk(char *gameDataPath) {
	_prefRoot = NULL;

	// Set default file directory
	File::setDefaultDirectory(gameDataPath);

	_dataDiskHandle = new File();
	_dnrHandle = new File();

	uint32 entriesRead;

	_dnrHandle->open(dinnerFilename);
	if (_dnrHandle->isOpen() == false)
			error("Could not open %s%s", gameDataPath, dinnerFilename);

	if (!(_dinnerTableEntries = _dnrHandle->readUint32LE()))
		error("Error reading from sky.dnr"); //even though it was opened correctly?!

	_dinnerTableArea = (uint8 *)malloc(_dinnerTableEntries * 8);
	entriesRead = _dnrHandle->read(_dinnerTableArea, 8 * _dinnerTableEntries) / 8;

	if (entriesRead != _dinnerTableEntries)
		warning("entriesRead != dinnerTableEntries. [%d/%d]", entriesRead, _dinnerTableEntries);

	_dataDiskHandle->open(dataFilename);
	if (_dataDiskHandle->isOpen() == false) 
		error("Error opening %s%s", gameDataPath, dataFilename);

	printf("Found BASS version v0.0%d (%d dnr entries)\n", determineGameVersion(), _dinnerTableEntries);

	memset(_buildList, 0, 60 * 2);
	memset(_loadedFilesList, 0, 60 * 4);
}

SkyDisk::~SkyDisk(void) {

	PrefFile *fEntry = _prefRoot;
	while (fEntry) {
		free(fEntry->data);
		PrefFile *fTemp = fEntry;
		fEntry = fEntry->next;
		delete fTemp;
	}
	if (_dnrHandle->isOpen()) _dnrHandle->close();
	if (_dataDiskHandle->isOpen()) _dataDiskHandle->close();
	delete _dnrHandle;
	delete _dataDiskHandle;
}

void SkyDisk::flushPrefetched(void) {

	PrefFile *fEntry = _prefRoot;
	while (fEntry) {
		free(fEntry->data);
		PrefFile *fTemp = fEntry;
		fEntry = fEntry->next;
		delete fTemp;
	}
	_prefRoot = NULL;
}

bool SkyDisk::fileExists(uint16 fileNr) {
	
	return (getFileInfo(fileNr) != NULL);
}

//load in file file_nr to address dest
//if dest == NULL, then allocate memory for this file
uint8 *SkyDisk::loadFile(uint16 fileNr, uint8 *dest) {
	
	uint8 cflag;
	int32 bytesRead;
	uint8 *filePtr, *inputPtr, *outputPtr;
	dataFileHeader fileHeader;

	uint8 *prefData = givePrefetched(fileNr, &_lastLoadedFileSize);
	if (prefData) {
		if (dest == NULL) return prefData;
		else {
			memcpy(dest, prefData, _lastLoadedFileSize);
			free(prefData);
			return dest;
		}
	}

	#ifdef file_order_chk
		warning("File order checking not implemented yet");
	#endif

	_compFile = fileNr;
	debug(2, "load file %d,%d (%d)", (fileNr >> 11), (fileNr & 2047), fileNr); 

	filePtr = getFileInfo(fileNr);
	if (filePtr == NULL) {
		debug(1, "File %d not found", fileNr);
		return NULL;
	}

	_fileFlags = READ_LE_UINT32((filePtr + 5));
	_fileSize = _fileFlags & 0x03fffff;
	_lastLoadedFileSize = _fileSize;
	
	_fileOffset = READ_LE_UINT32((filePtr + 2)) & 0x0ffffff;

	cflag = (uint8)((_fileOffset >> 23) & 0x1);
	_fileOffset &= 0x7FFFFF;

	if (cflag) {
		if (SkyState::_systemVars.gameVersion == 331)
			_fileOffset <<= 3;
		else
			_fileOffset <<= 4;
	}

	_fixedDest = dest;
	_fileDest = dest;
	_compDest = dest;

	if (dest == NULL) //we need to allocate memory for this file
		_fileDest = (uint8 *)malloc(_fileSize + 4);

	_dataDiskHandle->seek(_fileOffset, SEEK_SET);

	#ifdef file_order_chk
		warning("File order checking not implemented yet");
	#endif

	//now read in the data
	bytesRead = _dataDiskHandle->read(_fileDest, 1 * _fileSize);

	if (bytesRead != (int32)_fileSize)
		warning("ERROR: Unable to read %d bytes from datadisk (%d bytes read)", _fileSize, bytesRead);

	cflag = (uint8)((_fileFlags >> (23)) & 0x1);

	//if cflag == 0 then file is compressed, 1 == uncompressed

	if ((!cflag) && ((FROM_LE_16(((dataFileHeader *)_fileDest)->flag) >> 7)&1)) {
		debug(2, "File is RNC compressed.");

		memcpy(&fileHeader, _fileDest, sizeof(struct dataFileHeader));
		_decompSize = (FROM_LE_16(fileHeader.flag) & 0xFFFFFF00) << 8;
		_decompSize |= FROM_LE_16((uint16)fileHeader.s_tot_size);

		if (_fixedDest == NULL) // is this valid?
			_compDest = (uint8 *)malloc(_decompSize);

		inputPtr = _fileDest;
		outputPtr = _compDest;

		if ( (uint8)(_fileFlags >> (22) & 0x1) ) //do we include the header?
			inputPtr += sizeof(struct dataFileHeader);
		else {
#ifdef SCUMM_BIG_ENDIAN
			// Convert dataFileHeader to BE (it only consists of 16 bit words)
			uint16 *headPtr = (uint16 *)_fileDest;
			for (uint i = 0; i < sizeof(struct dataFileHeader) / 2; i++)
				*(headPtr + i) = READ_LE_UINT16(headPtr + i);
#endif
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
	} else {
#ifdef SCUMM_BIG_ENDIAN
		if (!cflag) {
			warning("patching header for uncompressed file %d", fileNr);
			uint16 *headPtr = (uint16 *)_fileDest;
		    for (uint i = 0; i < sizeof(struct dataFileHeader) / 2; i++)
				*(headPtr + i) = READ_LE_UINT16(headPtr + i);
		}
#endif
		return _fileDest;
	}

	return _compDest;
}

void SkyDisk::prefetchFile(uint16 fileNr) {

	PrefFile **fEntry = &_prefRoot;
	bool found = false;
	while (*fEntry) {
		if ((*fEntry)->fileNr == fileNr) found = true;
		fEntry = &((*fEntry)->next);
	}
	if (found) {
		debug(1,"SkyDisk::prefetchFile: File %d was already prefetched",fileNr);
		return ;
	}
	uint8 *temp = loadFile(fileNr, NULL);
	*fEntry = new PrefFile;
	(*fEntry)->data = temp;
	(*fEntry)->fileSize = _lastLoadedFileSize;
	(*fEntry)->fileNr = fileNr;
	(*fEntry)->next = NULL;
}

uint8 *SkyDisk::givePrefetched(uint16 fileNr, uint32 *fSize) {
	
	PrefFile **fEntry = &_prefRoot;
	bool found = false;
	while ((*fEntry) && (!found)) {
		if ((*fEntry)->fileNr == fileNr) found = true;
		else fEntry = &((*fEntry)->next);
	}
	if (!found) {
		*fSize = 0;
		return NULL;
	}
	uint8 *retPtr = (*fEntry)->data;
	PrefFile *retStr = *fEntry;
	*fEntry = (*fEntry)->next;
	*fSize = retStr->fileSize;
	delete retStr;
    return retPtr;
}

uint8 *SkyDisk::getFileInfo(uint16 fileNr) {
	
	uint16 i;
	uint16 *dnrTbl16Ptr = (uint16 *)_dinnerTableArea;

	for (i = 0; i < _dinnerTableEntries; i++) {
		if (READ_LE_UINT16(dnrTbl16Ptr) == fileNr) {
			debug(2, "file %d found!", fileNr);
			return (uint8 *)dnrTbl16Ptr;
		}
		dnrTbl16Ptr += 4;
	}

	// if file not found return 0
	return 0;
}

void SkyDisk::fnCacheChip(uint32 list) {

	// fnCacheChip is called after fnCacheFast
	uint16 cnt = 0;
	while (_buildList[cnt]) cnt++;
	uint16 *fList = (uint16 *)SkyState::fetchCompact(list);
	uint16 fCnt = 0;
	do {
		_buildList[cnt + fCnt] = fList[fCnt] & 0x7FFFU;
		fCnt++;
	} while (fList[fCnt-1]);
	fnCacheFiles();
}

void SkyDisk::fnCacheFast(uint32 list) {

	if (list == 0) return;
	uint8 cnt = 0;
	uint16 *fList = (uint16 *)SkyState::fetchCompact(list);
	do {
		_buildList[cnt] = fList[cnt] & 0x7FFFU;
		cnt++;
	} while (fList[cnt-1]);
}

void SkyDisk::fnCacheFiles(void) {

	uint16 lCnt, bCnt, targCnt;
	targCnt = lCnt = 0;
	bool found;
	while (_loadedFilesList[lCnt]) {
		bCnt = 0;
		found = false;
		while (_buildList[bCnt] && (!found)) {
			if ((_buildList[bCnt] & 0x7FFFU) == _loadedFilesList[lCnt]) found = true;
			else bCnt++;
		}
		if (found) {
			_loadedFilesList[targCnt] = _loadedFilesList[lCnt];
			targCnt++;
		} else {
			free(SkyState::_itemList[_loadedFilesList[lCnt] & 2047]);
			SkyState::_itemList[_loadedFilesList[lCnt] & 2047] = NULL;		
		}
		lCnt++;
	}
	_loadedFilesList[targCnt] = 0; // mark end of list
	bCnt = 0;
	while (_buildList[bCnt]) {
		if ((_buildList[bCnt] & 0x7FF) == 0x7FF) {
			// amiga dummy files
			bCnt++;
			continue;
		}
		lCnt = 0;
		found = false;
		while (_loadedFilesList[lCnt] && (!found)) {
			if (_loadedFilesList[lCnt] == (_buildList[bCnt] & 0x7FFFU)) found = true;
			lCnt++;
		}
		if (found) {
			bCnt++;
			continue;
		}
		// ok, we really have to load the file.
		_loadedFilesList[targCnt] = _buildList[bCnt] & 0x7FFFU;
		targCnt++;
		_loadedFilesList[targCnt] = 0;
		SkyState::_itemList[_buildList[bCnt] & 2047] = (void**)loadFile(_buildList[bCnt] & 0x7FFF, NULL);
		if (!SkyState::_itemList[_buildList[bCnt] & 2047])
			warning("fnCacheFiles: SkyDisk::loadFile() returned NULL for file %d",_buildList[bCnt] & 0x7FFF);
		bCnt++;
	}
	_buildList[0] = 0;
}

void SkyDisk::refreshFilesList(uint32 *list) {

	uint8 cnt = 0;
	while (_loadedFilesList[cnt]) {
		if (SkyState::_itemList[_loadedFilesList[cnt] & 2047])
			free(SkyState::_itemList[_loadedFilesList[cnt] & 2047]);
		SkyState::_itemList[_loadedFilesList[cnt] & 2047] = NULL;
		cnt++;
	}
	cnt = 0;
	while (list[cnt]) {
		_loadedFilesList[cnt] = list[cnt];
		SkyState::_itemList[_loadedFilesList[cnt] & 2047] = (void**)loadFile((uint16)(_loadedFilesList[cnt] & 0x7FFF), NULL);
		cnt++;
	}
	_loadedFilesList[cnt] = 0;
}

void SkyDisk::fnMiniLoad(uint16 fileNum) {

	uint16 cnt = 0;
	while (_loadedFilesList[cnt]) {
		if (_loadedFilesList[cnt] == fileNum) return ;
		cnt++;
	}
	_loadedFilesList[cnt] = fileNum & 0x7FFFU;
	_loadedFilesList[cnt + 1] = 0;
	SkyState::_itemList[fileNum & 2047] = (void**)loadFile(fileNum, NULL);
}

void SkyDisk::fnFlushBuffers(void) {

	// dump all loaded sprites
	uint8 lCnt = 0;
	while (_loadedFilesList[lCnt]) {
		free(SkyState::_itemList[_loadedFilesList[lCnt] & 2047]);
		SkyState::_itemList[_loadedFilesList[lCnt] & 2047] = 0;
		lCnt++;
	}
	_loadedFilesList[0] = 0;
}

void SkyDisk::dumpFile(uint16 fileNr) {
	char buf[128];
	File out;
	byte* filePtr;

	filePtr = loadFile(fileNr, NULL);
	sprintf(buf, "dumps/file-%d.dmp", fileNr);
	
	out.open(buf, "", File::kFileReadMode);
	if (out.isOpen() == false) {
		out.open(buf, "", File::kFileWriteMode);
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
	case 243:
		// pc gamer demo (v0.0109)
		return 109;
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
		//floppy (v0.0331 or v0.0348)
		if (_dataDiskHandle->size() == 8830435) return 348;
		else return 331;
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
		error("Unknown game version! %d dinner table entries", _dinnerTableEntries);
		break;
	}
}
