/* Copyright (C) 1994-1998 Revolution Software Ltd.
 * Copyright (C) 2003-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "common/system.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/memory.h"
#include "sword2/resman.h"
#include "sword2/router.h"
#include "sword2/sound.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

// Welcome to the easy resource manager - written in simple code for easy
// maintenance
//
// The resource compiler will create two files
//
//	resource.inf which is a list of ascii cluster file names
//	resource.tab which is a table which tells us which cluster a resource
//	is located in and the number within the cluster

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct CdInf {
	uint8 clusterName[20];	// Null terminated cluster name.
	uint8 cd;		// Cd cluster is on and whether it is on the local drive or not.
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

ResourceManager::ResourceManager(Sword2Engine *vm) {
	_vm = vm;

	// Until proven differently, assume we're on CD 1. This is so the start
	// dialog will be able to play any music at all.
	_curCd = 1;

	// We read in the resource info which tells us the names of the
	// resource cluster files ultimately, although there might be groups
	// within the clusters at this point it makes no difference. We only
	// wish to know what resource files there are and what is in each

	Common::File file;
	uint32 size;
	byte *temp;

	_totalClusters = 0;
	_resConvTable = NULL;

	if (!file.open("resource.inf"))
		error("Cannot open resource.inf");

	size = file.size();

	// Get some space for the incoming resource file - soon to be trashed
	temp = (byte *)malloc(size);

	if (file.read(temp, size) != size) {
		file.close();
		error("init cannot *READ* resource.inf");
	}

	file.close();

	// Ok, we've loaded in the resource.inf file which contains a list of
	// all the files now extract the filenames.

	// Using this method the Gode generated resource.inf must have #0d0a on
	// the last entry

	uint32 i = 0;
	uint32 j = 0;

	do {
		// item must have an #0d0a
		while (temp[i] != 13) {
			_resFiles[_totalClusters].fileName[j] = temp[i];
			i++;
			j++;
		}

		// NULL terminate our extracted string
		_resFiles[_totalClusters].fileName[j] = '\0';
		_resFiles[_totalClusters].numEntries = -1;
		_resFiles[_totalClusters].entryTab = NULL;

		// Reset position in current slot between entries, skip the
		// 0x0a in the source and increase the number of clusters.

		j = 0;
		i += 2;
		_totalClusters++;

		// TODO: put overload check here
	} while (i != size);

	free(temp);

	// Now load in the binary id to res conversion table
	if (!file.open("resource.tab"))
		error("Cannot open resource.tab");

	// Find how many resources
	size = file.size();

	_totalResFiles = size / 4;

	// Table seems ok so malloc some space
	_resConvTable = (uint16 *)malloc(size);

	for (i = 0; i < size / 2; i++)
		_resConvTable[i] = file.readUint16LE();

	if (file.ioFailed()) {
		file.close();
		error("Cannot read resource.tab");
	}

	file.close();

	if (!file.open("cd.inf"))
		error("Cannot open cd.inf");

	CdInf *cdInf = new CdInf[_totalClusters];

	for (i = 0; i < _totalClusters; i++) {
		file.read(cdInf[i].clusterName, sizeof(cdInf[i].clusterName));

		cdInf[i].cd = file.readByte();

		if (file.ioFailed())
			error("Cannot read cd.inf");

		// It has been reported that there are two different versions
		// of the cd.inf file: One where all clusters on CD also have
		// the LOCAL_CACHE bit set. This bit is no longer used. To
		// avoid future problems, let's normalize the flag once and for
		// all here.

		if (cdInf[i].cd & LOCAL_PERM)
			cdInf[i].cd = LOCAL_PERM;
		else if (cdInf[i].cd & CD1)
			cdInf[i].cd = CD1;
		else if (cdInf[i].cd & CD2)
			cdInf[i].cd = CD2;
		else
			cdInf[i].cd = LOCAL_PERM;
	}

	file.close();

	for (i = 0; i < _totalClusters; i++) {
		for (j = 0; j < _totalClusters; j++) {
			if (scumm_stricmp((char *)cdInf[j].clusterName, _resFiles[i].fileName) == 0)
				break;
		}

		if (j == _totalClusters)
			error("%s is not in cd.inf", _resFiles[i].fileName);

		_resFiles[i].cd = cdInf[j].cd;
	}

	delete [] cdInf;

	debug(1, "%d resources in %d cluster files", _totalResFiles, _totalClusters);
	for (i = 0; i < _totalClusters; i++)
		debug(2, "filename of cluster %d: -%s", i, _resFiles[i].fileName);

	_resList = (Resource *)malloc(_totalResFiles * sizeof(Resource));

	for (i = 0; i < _totalResFiles; i++) {
		_resList[i].ptr = NULL;
		_resList[i].size = 0;
		_resList[i].refCount = 0;
		_resList[i].prev = _resList[i].next = NULL;
	}
	_cacheStart = _cacheEnd = NULL;
	_usedMem = 0;
}

ResourceManager::~ResourceManager() {
	Resource *res = _cacheStart;
	while (res) {
		_vm->_memory->memFree(res->ptr);
		res = res->next;
	}
	free(_resList);
	free(_resConvTable);
}

// Quick macro to make swapping in-place easier to write

#define SWAP16(x)	x = SWAP_BYTES_16(x)
#define SWAP32(x)	x = SWAP_BYTES_32(x)

void convertEndian(byte *file, uint32 len) {
	int i;
	StandardHeader *hdr = (StandardHeader *)file;

	file += sizeof(StandardHeader);

	SWAP32(hdr->compSize);
	SWAP32(hdr->decompSize);

	switch (hdr->fileType) {
	case ANIMATION_FILE: {
		AnimHeader *animHead = (AnimHeader *)file;

		SWAP16(animHead->noAnimFrames);
		SWAP16(animHead->feetStartX);
		SWAP16(animHead->feetStartY);
		SWAP16(animHead->feetEndX);
		SWAP16(animHead->feetEndY);
		SWAP16(animHead->blend);

		CdtEntry *cdtEntry = (CdtEntry *)(file + sizeof(AnimHeader));
		for (i = 0; i < animHead->noAnimFrames; i++, cdtEntry++) {
			SWAP16(cdtEntry->x);
			SWAP16(cdtEntry->y);
			SWAP32(cdtEntry->frameOffset);

			FrameHeader *frameHeader = (FrameHeader *)(file + cdtEntry->frameOffset);
			// Quick trick to prevent us from incorrectly applying the endian
			// fixes multiple times. This assumes that frames are less than 1 MB
			// and have height/width less than 4096.
			if ((frameHeader->compSize & 0xFFF00000) ||
				(frameHeader->width & 0xF000) ||
				(frameHeader->height & 0xF000)) {
				SWAP32(frameHeader->compSize);
				SWAP16(frameHeader->width);
				SWAP16(frameHeader->height);
			}
		}
		break;
	}
	case SCREEN_FILE: {
		MultiScreenHeader *mscreenHeader = (MultiScreenHeader *)file;

		SWAP32(mscreenHeader->palette);
		SWAP32(mscreenHeader->bg_parallax[0]);
		SWAP32(mscreenHeader->bg_parallax[1]);
		SWAP32(mscreenHeader->screen);
		SWAP32(mscreenHeader->fg_parallax[0]);
		SWAP32(mscreenHeader->fg_parallax[1]);
		SWAP32(mscreenHeader->layers);
		SWAP32(mscreenHeader->paletteTable);
		SWAP32(mscreenHeader->maskOffset);

		// screenHeader
		ScreenHeader *screenHeader = (ScreenHeader *)(file + mscreenHeader->screen);

		SWAP16(screenHeader->width);
		SWAP16(screenHeader->height);
		SWAP16(screenHeader->noLayers);

		// layerHeader
		LayerHeader *layerHeader = (LayerHeader *)(file + mscreenHeader->layers);
		for (i = 0; i < screenHeader->noLayers; i++, layerHeader++) {
			SWAP16(layerHeader->x);
			SWAP16(layerHeader->y);
			SWAP16(layerHeader->width);
			SWAP16(layerHeader->height);
			SWAP32(layerHeader->maskSize);
			SWAP32(layerHeader->offset);
		}

		// backgroundParallaxLayer
		Parallax *parallax;
		int offset;
		offset = mscreenHeader->bg_parallax[0];
		if (offset > 0) {
			parallax = (Parallax *)(file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->bg_parallax[1];
		if (offset > 0) {
			parallax = (Parallax *)(file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// backgroundLayer
		offset = mscreenHeader->screen + sizeof(ScreenHeader);
		if (offset > 0) {
			parallax = (Parallax *)(file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// foregroundParallaxLayer
		offset = mscreenHeader->fg_parallax[0];
		if (offset > 0) {
			parallax = (Parallax *)(file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->fg_parallax[1];
		if (offset > 0) {
			parallax = (Parallax *)(file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}
		break;
	}
	case GAME_OBJECT: {
		ObjectHub *objectHub = (ObjectHub *)file;

		objectHub->type = (int) SWAP_BYTES_32(objectHub->type);
		SWAP32(objectHub->logic_level);

		for (i = 0; i < TREE_SIZE; i++) {
			SWAP32(objectHub->logic[i]);
			SWAP32(objectHub->script_id[i]);
			SWAP32(objectHub->script_pc[i]);
		}
		break;
	}
	case WALK_GRID_FILE: {
		WalkGridHeader *walkGridHeader = (WalkGridHeader *)file;

		SWAP32(walkGridHeader->numBars);
		SWAP32(walkGridHeader->numNodes);

		BarData *barData = (BarData *)(file + sizeof(WalkGridHeader));
		for (i = 0; i < walkGridHeader->numBars; i++) {
			SWAP16(barData->x1);
			SWAP16(barData->y1);
			SWAP16(barData->x2);
			SWAP16(barData->y2);
			SWAP16(barData->xmin);
			SWAP16(barData->ymin);
			SWAP16(barData->xmax);
			SWAP16(barData->ymax);
			SWAP16(barData->dx);
			SWAP16(barData->dy);
			SWAP32(barData->co);
			barData++;
		}

		uint16 *node = (uint16 *)(file + sizeof(WalkGridHeader) + walkGridHeader->numBars * sizeof(BarData));
		for (i = 0; i < walkGridHeader->numNodes * 2; i++) {
			SWAP16(*node);
			node++;
		}

		break;
	}
	case GLOBAL_VAR_FILE:
		break;
	case PARALLAX_FILE_null:
		break;
	case RUN_LIST: {
		uint32 *list = (uint32 *)file;
		while (*list) {
			SWAP32(*list);
			list++;
		}
		break;
	}
	case TEXT_FILE: {
		TextHeader *textHeader = (TextHeader *)file;
		SWAP32(textHeader->noOfLines);
		break;
	}
	case SCREEN_MANAGER:
		break;
	case MOUSE_FILE:
		break;
	case ICON_FILE:
		break;
	}
}

/**
 * Returns the address of a resource. Loads if not in memory. Retains a count.
 */

byte *ResourceManager::openResource(uint32 res, bool dump) {
	assert(res < _totalResFiles);

	// Is the resource in memory already? If not, load it.

	if (!_resList[res].ptr) {
		// Fetch the correct file and read in the correct portion.
		uint16 cluFileNum = _resConvTable[res * 2]; // points to the number of the ascii filename
		assert(cluFileNum != 0xffff);

		// Relative resource within the file
		// First we have to find the file via the _resConvTable
		uint16 actual_res = _resConvTable[(res * 2) + 1];

		debug(5, "openResource %s res %d", _resFiles[cluFileNum].fileName, res);

		// If we're loading a cluster that's only available from one
		// of the CDs, remember which one so that we can play the
		// correct music.

		if ((_resFiles[cluFileNum].cd == CD1) || (_resFiles[cluFileNum].cd == CD2))
			_curCd = _resFiles[cluFileNum].cd;

		// Actually, as long as the file can be found we don't really
		// care which CD it's on. But if we can't find it, keep asking
		// for the CD until we do.

		Common::File *file = openCluFile(cluFileNum);

		if (_resFiles[cluFileNum].entryTab == NULL) {
			// we didn't read from this file before, get its index table
			readCluIndex(cluFileNum, file);
		}

		uint32 pos = _resFiles[cluFileNum].entryTab[actual_res * 2 + 0];
		uint32 len = _resFiles[cluFileNum].entryTab[actual_res * 2 + 1];

		file->seek(pos, SEEK_SET);

		debug(6, "res len %d", len);

		// Ok, we know the length so try and allocate the memory.
		_resList[res].ptr = _vm->_memory->memAlloc(len, res);
		_resList[res].size = len;
		_resList[res].refCount = 0;

		file->read(_resList[res].ptr, len);

		if (dump) {
			StandardHeader *header = (StandardHeader *)_resList[res].ptr;
			char buf[256];
			const char *tag;
			Common::File out;

			switch (header->fileType) {
			case ANIMATION_FILE:
				tag = "anim";
				break;
			case SCREEN_FILE:
				tag = "layer";
				break;
			case GAME_OBJECT:
				tag = "object";
				break;
			case WALK_GRID_FILE:
				tag = "walkgrid";
				break;
			case GLOBAL_VAR_FILE:
				tag = "globals";
				break;
			case PARALLAX_FILE_null:
				tag = "parallax";	// Not used!
				break;
			case RUN_LIST:
				tag = "runlist";
				break;
			case TEXT_FILE:
				tag = "text";
				break;
			case SCREEN_MANAGER:
				tag = "screen";
				break;
			case MOUSE_FILE:
				tag = "mouse";
				break;
			case WAV_FILE:
				tag = "wav";
				break;
			case ICON_FILE:
				tag = "icon";
				break;
			case PALETTE_FILE:
				tag = "palette";
				break;
			default:
				tag = "unknown";
				break;
			}

#if defined(MACOS_CARBON)
			sprintf(buf, ":dumps:%s-%d.dmp", tag, res);
#else
			sprintf(buf, "dumps/%s-%d.dmp", tag, res);
#endif

			if (!out.exists(buf, "")) {
				if (out.open(buf, Common::File::kFileWriteMode, ""))
					out.write(_resList[res].ptr, len);
			}
		}

		// close the cluster
		file->close();
		delete file;

		_usedMem += len;
		checkMemUsage();

#ifdef SCUMM_BIG_ENDIAN
		convertEndian(_resList[res].ptr, len);
#endif
	} else if (_resList[res].refCount == 0)
		removeFromCacheList(_resList + res);

	_resList[res].refCount++;

	return _resList[res].ptr;
}

void ResourceManager::closeResource(uint32 res) {
	assert(res < _totalResFiles);

	// Don't try to close the resource if it has already been forcibly
	// closed, e.g. by fnResetGlobals().

	if (_resList[res].ptr == NULL)
		return;

	assert(_resList[res].refCount > 0);

	_resList[res].refCount--;
	if (_resList[res].refCount == 0)
		addToCacheList(_resList + res);

	// It's tempting to free the resource immediately when refCount
	// reaches zero, but that'd be a mistake. Closing a resource does not
	// mean "I'm not going to use this resource any more". It means that
	// "the next time I use this resource I'm going to ask for a new
	// pointer to it".
	//
	// Since the original memory manager had to deal with memory
	// fragmentation, keeping a resource open - and thus locked down to a
	// specific memory address - was considered a bad thing.
}

void ResourceManager::removeFromCacheList(Resource *res) {
	if (_cacheStart == res)
		_cacheStart = res->next;

	if (_cacheEnd == res)
		_cacheEnd = res->prev;

	if (res->prev)
		res->prev->next = res->next;
	if (res->next)
		res->next->prev = res->prev;
	res->prev = res->next = NULL;
}

void ResourceManager::addToCacheList(Resource *res) {
	res->prev = NULL;
	res->next = _cacheStart;
	if (_cacheStart)
		_cacheStart->prev = res;
	_cacheStart = res;
	if (!_cacheEnd)
		_cacheEnd = res;
}

Common::File *ResourceManager::openCluFile(uint16 fileNum) {
	Common::File *file = new Common::File;
	while (!file->open(_resFiles[fileNum].fileName)) {
		// HACK: We have to check for this, or it'll be impossible to
		// quit while the game is asking for the user to insert a CD.
		// But recovering from this situation gracefully is just too
		// much trouble, so quit now.
		if (_vm->_quit)
			g_system->quit();

		// If the file is supposed to be on hard disk, or we're
		// playing a demo, then we're in trouble if the file
		// can't be found!

		if ((_vm->_features & GF_DEMO) || (_resFiles[fileNum].cd == LOCAL_PERM))
			error("Could not find '%s'", _resFiles[fileNum].fileName);

		getCd(_resFiles[fileNum].cd);
	}
	return file;
}

void ResourceManager::readCluIndex(uint16 fileNum, Common::File *file) {
	if (_resFiles[fileNum].entryTab == NULL) {
		// we didn't read from this file before, get its index table
		if (file == NULL)
			file = openCluFile(fileNum);
		else
			file->incRef();

		// 1st DWORD of a cluster is an offset to the look-up table
		uint32 table_offset = file->readUint32LE();
		debug(6, "table offset = %d", table_offset);
		uint32 tableSize = file->size() - table_offset; // the table is stored at the end of the file
		file->seek(table_offset);

		assert((tableSize % 8) == 0);
		_resFiles[fileNum].entryTab = (uint32*)malloc(tableSize);
		_resFiles[fileNum].numEntries = tableSize / 8;
		file->read(_resFiles[fileNum].entryTab, tableSize);
		if (file->ioFailed())
			error("unable to read index table from file %s\n", _resFiles[fileNum].fileName);
#ifdef SCUMM_BIG_ENDIAN
		for (int tabCnt = 0; tabCnt < _resFiles[fileNum].numEntries * 2; tabCnt++)
			_resFiles[fileNum].entryTab[tabCnt] = FROM_LE_32(_resFiles[fileNum].entryTab[tabCnt]);
#endif
		file->decRef();
	}
}

/**
 * Returns true if resource is valid, otherwise false.
 */

bool ResourceManager::checkValid(uint32 res) {
	// Resource number out of range
	if (res >= _totalResFiles)
		return false;

	// Points to the number of the ascii filename
	uint16 parent_res_file = _resConvTable[res * 2];

	// Null & void resource
	if (parent_res_file == 0xffff)
		return false;

	return true;
}

/**
 * Returns the total file length of a resource - i.e. all headers are included
 * too.
 */

uint32 ResourceManager::fetchLen(uint32 res) {
	if (_resList[res].ptr)
		return _resList[res].size;

	// Does this ever happen?
	warning("fetchLen: Resource %u is not loaded; reading length from file", res);

	// Points to the number of the ascii filename
	uint16 parent_res_file = _resConvTable[res * 2];

	// relative resource within the file
	uint16 actual_res = _resConvTable[(res * 2) + 1];

	// first we have to find the file via the _resConvTable
	// open the cluster file

	if (_resFiles[parent_res_file].entryTab == NULL) {
		readCluIndex(parent_res_file);
	}
	return _resFiles[parent_res_file].entryTab[actual_res * 2 + 1];
}

void ResourceManager::checkMemUsage() {
	while (_usedMem > MAX_MEM_CACHE) {
		// we're using up more memory than we wanted to. free some old stuff.
		// Newly loaded objects are added to the start of the list,
		// we start freeing from the end, to free the oldest items first
		if (_cacheEnd) {
			Resource *tmp = _cacheEnd;
			assert((tmp->refCount == 0) && (tmp->ptr) && (tmp->next == NULL));
			removeFromCacheList(tmp);

			_vm->_memory->memFree(tmp->ptr);
			tmp->ptr = NULL;
			_usedMem -= tmp->size;
		} else {
			warning("%d bytes of memory used, but cache list is empty!\n");
			return;
		}
	}
}

void ResourceManager::remove(int res) {
	if (_resList[res].ptr) {
		removeFromCacheList(_resList + res);

		_vm->_memory->memFree(_resList[res].ptr);
		_resList[res].ptr = NULL;
		_resList[res].refCount = 0;
		_usedMem -= _resList[res].size;
	}
}

/**
 * Remove all res files from memory - ready for a total restart. This includes
 * the player object and global variables resource.
 */

void ResourceManager::removeAll() {
	// We need to clear the FX queue, because otherwise the sound system
	// will still believe that the sound resources are in memory, and that
	// it's ok to close them.

	_vm->_sound->clearFxQueue();

	for (uint i = 0; i < _totalResFiles; i++)
		remove(i);
}

/**
 * Remove all resources from memory.
 */

void ResourceManager::killAll(bool wantInfo) {
	int nuked = 0;

	// We need to clear the FX queue, because otherwise the sound system
	// will still believe that the sound resources are in memory, and that
	// it's ok to close them.

	_vm->_sound->clearFxQueue();

	for (uint i = 0; i < _totalResFiles; i++) {
		// Don't nuke the global variables or the player object!
		if (i == 1 || i == CUR_PLAYER_ID)
			continue;

		if (_resList[i].ptr) {
			StandardHeader *header = (StandardHeader *)_resList[i].ptr;

			if (wantInfo)
				Debug_Printf("Nuked %5d: %s\n", i, header->name);

			remove(i);
			nuked++;
		}
	}

	if (wantInfo)
		Debug_Printf("Expelled %d resources\n", nuked);
}

/**
 * Like killAll but only kills objects (except George & the variable table of
 * course) - ie. forcing them to reload & restart their scripts, which
 * simulates the effect of a save & restore, thus checking that each object's
 * re-entrant logic works correctly, and doesn't cause a statuette to
 * disappear forever, or some plaster-filled holes in sand to crash the game &
 * get James in trouble again.
 */

void ResourceManager::killAllObjects(bool wantInfo) {
	int nuked = 0;

	for (uint i = 0; i < _totalResFiles; i++) {
		// Don't nuke the global variables or the player object!
		if (i == 1 || i == CUR_PLAYER_ID)
			continue;

		if (_resList[i].ptr) {
			StandardHeader *header = (StandardHeader *)_resList[i].ptr;

			if (header->fileType == GAME_OBJECT) {
				if (wantInfo)
					Debug_Printf("Nuked %5d: %s\n", i, header->name);

				remove(i);
				nuked++;
			}
		}
	}

	if (wantInfo)
		Debug_Printf("Expelled %d resources\n", nuked);
}

int ResourceManager::whichCd() {
	return _curCd;
}

void ResourceManager::getCd(int cd) {
	byte *textRes;

	// Stop any music from playing - so the system no longer needs the
	// current CD - otherwise when we take out the CD, Windows will
	// complain!

	_vm->_sound->stopMusic(true);

	textRes = openResource(2283);
	_vm->_screen->displayMsg(_vm->fetchTextLine(textRes, 5 + cd) + 2, 0);
	closeResource(2283);

	// The original code probably determined automagically when the correct
	// CD had been inserted, but our backend doesn't support that, and
	// anyway I don't know if all systems allow that sort of thing. So we
	// wait for the user to press any key instead, or click the mouse.
	//
	// But just in case we ever try to identify the CDs by their labels,
	// they should be:
	//
	// CD1: "RBSII1" (or "PCF76" for the PCF76 version, whatever that is)
	// CD2: "RBSII2"
}

} // End of namespace Sword2
