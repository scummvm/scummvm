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
#include "scummsys.h"
#include <stdio.h>
#include <stdlib.h>
#include "memman.h"
#include "resman.h"
#include "sworddefs.h"
#include "base/engine.h"
#include "common/util.h"
#include "swordres.h"

#define MAX_PATH_LEN 260

ResMan::ResMan(const char *resFile, MemMan *pMemoMan) {
	_memMan = pMemoMan;
	loadCluDescript(resFile);
}

ResMan::~ResMan(void) {
	freeCluDescript();
}

void ResMan::loadCluDescript(const char *fileName) {
	File resFile;
	resFile.open(fileName);
	if (!resFile.isOpen())
		error("ResMan::loadCluDescript(): File %s not found!", fileName);
	
	_prj.noClu = resFile.readUint32LE();
	_prj.clu = new BsClu*[_prj.noClu];

	uint32 *cluIndex = (uint32*)malloc(_prj.noClu * 4);
	resFile.read(cluIndex, _prj.noClu * 4);

	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++)
		if (cluIndex[clusCnt]) {
			BsClu *cluster = _prj.clu[clusCnt] = new BsClu;
			resFile.read(cluster->label, MAX_LABEL_SIZE);

			cluster->noGrp = resFile.readUint32LE();
			cluster->grp = new BsGrp*[cluster->noGrp];

			uint32 *grpIndex = (uint32*)malloc(cluster->noGrp * 4);
			resFile.read(grpIndex, cluster->noGrp * 4);

			for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++)
				if (grpIndex[grpCnt]) {
					BsGrp *group = cluster->grp[grpCnt] = new BsGrp;
					group->noRes = resFile.readUint32LE();
					group->resHandle = new BsMemHandle[group->noRes];
					group->offset = new uint32[group->noRes];
					group->length = new uint32[group->noRes];
					uint32 *resIdIdx = (uint32*)malloc(group->noRes * 4);
					resFile.read(resIdIdx, group->noRes * 4);

					for (uint32 resCnt = 0; resCnt < group->noRes; resCnt++) {
						if (resIdIdx[resCnt]) {
							group->offset[resCnt] = resFile.readUint32LE();
							group->length[resCnt] = resFile.readUint32LE();
							_memMan->initHandle(group->resHandle + resCnt);
						} else {
							group->offset[resCnt] = 0xFFFFFFFF;
							group->length[resCnt] = 0;
							_memMan->initHandle(group->resHandle + resCnt);
						}
					}
                    free(resIdIdx);
				} else
					cluster->grp[grpCnt] = NULL;
			free(grpIndex);
		} else
			_prj.clu[clusCnt] = NULL;
	free(cluIndex);
}

void ResMan::freeCluDescript(void) {
	
	for (uint32 clusCnt = 0; clusCnt < _prj.noClu; clusCnt++)
		if (BsClu *cluster = _prj.clu[clusCnt]) {
			for (uint32 grpCnt = 0; grpCnt < cluster->noGrp; grpCnt++)
				if (BsGrp *group = cluster->grp[grpCnt]) {
					_memMan->freeNow(group->resHandle);
					delete[] group->resHandle;
					delete[] group->offset;
					delete[] group->length;
					delete group;
				}
			delete[] cluster->grp;
			delete cluster;
		}
	delete[] _prj.clu;
}

void *ResMan::fetchRes(uint32 id) {
	BsMemHandle *memHandle = resHandle(id);
	if (!memHandle->data)
		error("fetchRes:: resource %d is not open!", id);
	return memHandle->data;
}

void *ResMan::openFetchRes(uint32 id) {
	resOpen(id);
	return fetchRes(id);
}

void ResMan::dumpRes(uint32 id) {
	char outn[30];
	sprintf(outn, "DUMP%08X.BIN", id);
	FILE *outf = fopen( outn, "wb");
	resOpen(id);
	BsMemHandle *memHandle = resHandle(id);
	fwrite(memHandle->data, 1, memHandle->size, outf);
	fclose(outf);
	resClose(id);
}

Header *ResMan::lockScript(uint32 scrID) {
	if (!_scriptList[scrID / ITM_PER_SEC])
		error("Script id %d not found.\n", scrID);
	scrID = _scriptList[scrID / ITM_PER_SEC];
#ifdef SCUMM_BIG_ENDIAN
	BsMemHandle *memHandle = resHandle(scrID);
	if (memHandle->cond == MEM_FREED)
		openScriptResourceBigEndian(scrID);
	else
		resOpen(scrID);
#else
	resOpen(scrID);
#endif
	return (Header*)resHandle(scrID)->data;
}

void ResMan::unlockScript(uint32 scrID) {
	resClose(_scriptList[scrID / ITM_PER_SEC]);
}

void *ResMan::cptResOpen(uint32 id) {
#ifdef SCUMM_BIG_ENDIAN
	BsMemHandle *memHandle = resHandle(id);
	if (memHandle->cond == MEM_FREED)
		openCptResourceBigEndian(id);
	else
		resOpen(id);
#else
	resOpen(id);
#endif
	return resHandle(id)->data;
}

void ResMan::resOpen(uint32 id) {  // load resource ID into memory
	BsMemHandle *memHandle = resHandle(id);
	if (memHandle->cond == MEM_FREED) { // memory has been freed
		uint32 size = resLength(id);
		_memMan->alloc(memHandle, size);
		File *clusFile = openClusterFile(id);
		clusFile->seek( resOffset(id) );
		clusFile->read( memHandle->data, size);
		if (clusFile->ioFailed())
			error("Can't read %d bytes from cluster %d\n", size, id);
		clusFile->close();
		delete clusFile;
	} else
		_memMan->setCondition(memHandle, MEM_DONT_FREE);
	memHandle->refCount++;
	if (memHandle->refCount > 20) {
		debug(1, "%d references to id %d. Guess there's something wrong.", memHandle->refCount, id);
	}
}

void ResMan::resClose(uint32 id) {
	BsMemHandle *handle = resHandle(id);
	if (!handle->refCount) {
		warning("Resource Manager fail: unlocking object with refCount 0. Id: %d\n", id);		
	} else
		handle->refCount--;
	if (!handle->refCount)
		_memMan->setCondition( handle, MEM_CAN_FREE);
}

FrameHeader *ResMan::fetchFrame(void *resourceData, uint32 frameNo) {
	uint8 *frameFile = (uint8*)resourceData;
    uint8 *idxData = frameFile + sizeof(Header);
	if (frameNo >= READ_LE_UINT32(idxData))
		error("fetchFrame:: frame %d doesn't exist in resource.", frameNo);
	frameFile += READ_LE_UINT32(idxData + (frameNo+1) * 4);
	return (FrameHeader*)frameFile;
}

File *ResMan::openClusterFile(uint32 id) {
	File *clusFile = new File();
	char fullPath[MAX_PATH_LEN];
	char fileName[15];
	makePathToCluster(fullPath);
	sprintf(fileName, "%s.CLU", _prj.clu[(id >> 24)-1]->label);
	clusFile->open(fileName);
	if (!clusFile->isOpen())
		error("Can't open cluster file %s in directory: %s\n", fileName, fullPath);
	return clusFile;
}

BsMemHandle *ResMan::resHandle(uint32 id) {
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	return &(_prj.clu[cluster]->grp[group]->resHandle[id & 0xFFFF]);
}

uint32 ResMan::resLength(uint32 id) {
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	return _prj.clu[cluster]->grp[group]->length[id & 0xFFFF];
}

uint32 ResMan::resOffset(uint32 id) {
	uint8 cluster = (uint8)((id >> 24) - 1);
	uint8 group = (uint8)(id >> 16);

	return _prj.clu[cluster]->grp[group]->offset[id & 0xFFFF];
}

void ResMan::makePathToCluster(char *str) {
	*str = '\0';
	// todo: add search stuff, cd1, cd2, etc.
}

void *ResMan::mouseResOpen(uint32 id) {
	BsMemHandle *memHandle = resHandle(id);
	if (memHandle->cond == MEM_FREED) {
		resOpen(id);
		uint16 *head = (uint16*)memHandle->data;
#ifdef SCUMM_BIG_ENDIAN
		for (uint8 endCnt = 0; endCnt < 5; endCnt++)
			head[endCnt] = READ_LE_UINT16(head + endCnt);
#endif
		// fix transparency:
		uint8 *rawData = (uint8*)memHandle->data;
		uint32 size = head[0] * head[1] * head[2];
		rawData += 0x3A;
		for (uint32 cnt = 0; cnt < size; cnt++)
			if (rawData[cnt] == 0)
				rawData[cnt] = 255;
		return memHandle->data;
	} else 
		return openFetchRes(id);
}

void ResMan::openCptResourceBigEndian(uint32 id) {
	resOpen(id);
	BsMemHandle *handle = resHandle(id);
	uint32 totSize = handle->size;
	uint8 *data = ((uint8*)handle->data) + sizeof(Header);
	totSize -= sizeof(Header);

	uint32 numCpts = *(uint32*)data = READ_LE_UINT32(data);
	data += 4;
	uint32 *dataIdx = (uint32*)data;

	for (uint32 cnt = 0; cnt < numCpts; cnt++) {
		uint32 cptSize;
		//uint32 cptPos = READ_LE_UINT32(data + cnt * 4);
		uint32 cptPos = dataIdx[cnt] = READ_LE_UINT32(dataIdx + cnt);
		if (cnt == numCpts-1)
			cptSize = totSize - cptPos - 4;
		else
			cptSize = READ_LE_UINT32(data + (cnt + 1) * 4) - cptPos;
		if (cptSize & 3)
			error("Odd compact size during endian conversion. Resource ID = %d, Cpt = %d of %d, Size %d\n", id, cnt, numCpts, cptSize);
		
		cptSize >>= 2;
		uint32 *cptData = (uint32*)(data + cptPos);
		for (uint32 elemCnt = 0; elemCnt < cptSize; elemCnt++)
			cptData[elemCnt] = READ_LE_UINT32(cptData + elemCnt);
	}
}

void ResMan::openScriptResourceBigEndian(uint32 id) {
	resOpen(id);
	BsMemHandle *handle = resHandle(id);
	// uint32 totSize = handle->size;
	Header *head = (Header*)handle->data;
	head->comp_length = FROM_LE_32(head->comp_length);
	head->decomp_length = FROM_LE_32(head->decomp_length);
	head->version = FROM_LE_16(head->version);
	uint32 *data = (uint32*)((uint8*)handle->data + sizeof(Header));
	uint32 size = handle->size - sizeof(Header);
	if (size & 3)
		error("Odd size during script endian conversion. Resource ID =%d, size = %d", id, size);
	size >>= 2;
	for (uint32 cnt = 0; cnt < size; cnt++) {
		*data = READ_LE_UINT32(data);
		data++;
	}
}
