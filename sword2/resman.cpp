/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "common/file.h"
#include "sword2/sword2.h"
#include "sword2/console.h"
#include "sword2/defs.h"
#include "sword2/logic.h"
#include "sword2/resman.h"
#include "sword2/driver/d_draw.h"

#define Debug_Printf _vm->_debugger->DebugPrintf

namespace Sword2 {

// Welcome to the easy resource manager - written in simple code for easy
// maintenance
// 
// The resource compiler will create two files
//
//	resource.inf which is a list of ascii cluster file names
//	resource.tab which is a table which tells us which cluster a resource
//      is located in and the number within the cluster

// If 0, resouces are expelled immediately when they are closed. At the moment
// this causes the sound queue to run out of slots. My only theory is that it's
// a script that gets reloaded over and over. That'd clear its local variables
// which I guess may cause it to set up the sounds over and over.

#define CACHE_CLUSTERS 1

// Resources age every time a new room is entered. This constant indicates how
// long a cached resource (i.e. one that has been closed) is allowed to live
// before it dies of old age. This may need some tuning, but I picked three
// because so many areas in the game seem to consist of about three rooms.

#define MAX_CACHE_AGE 3

enum {
	BOTH		= 0x0,		// Cluster is on both CDs
	CD1		= 0x1,		// Cluster is on CD1 only
	CD2		= 0x2,		// Cluster is on CD2 only
	LOCAL_CACHE	= 0x4,		// Cluster is cached on HDD
	LOCAL_PERM	= 0x8		// Cluster is on HDD.
};

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

	File file;
	uint32 size;
	byte *temp;

	_totalClusters = 0;
	_resConvTable = NULL;

	if (!file.open("resource.inf"))
		error("Cannot open resource.inf");

	size = file.size();

	// Get some space for the incoming resource file - soon to be trashed
	temp = (byte *) malloc(size);

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
			_resourceFiles[_totalClusters][j] = temp[i];
			i++;
			j++;
		}

		// NULL terminate our extracted string
		_resourceFiles[_totalClusters][j] = 0;

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
	_resConvTable = (uint16 *) malloc(size);

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
	}

	file.close();

	for (i = 0; i < _totalClusters; i++) {
		for (j = 0; j < _totalClusters; j++) {
			if (scumm_stricmp((char *) cdInf[j].clusterName, _resourceFiles[i]) == 0)
				break;
		}

		if (j == _totalClusters)
			error("%s is not in cd.inf", _resourceFiles[i]);

		_cdTab[i] = cdInf[j].cd;
	}

	delete [] cdInf;

	debug(1, "%d resources in %d cluster files", _totalResFiles, _totalClusters);
	for (i = 0; i < _totalClusters; i++)
		debug(2, "filename of cluster %d: -%s", i, _resourceFiles[i]);

	_resList = (Resource *) malloc(_totalResFiles * sizeof(Resource));

	for (i = 0; i < _totalResFiles; i++) {
		_resList[i].ptr = NULL;
		_resList[i].size = 0;
		_resList[i].refCount = 0;
		_resList[i].refTime = 0;
	}

	_resTime = 0;
}

ResourceManager::~ResourceManager(void) {
	free(_resList);
	free(_resConvTable);
}

// Quick macro to make swapping in-place easier to write

#define SWAP16(x)	x = SWAP_BYTES_16(x)
#define SWAP32(x)	x = SWAP_BYTES_32(x)

void convertEndian(byte *file, uint32 len) {
	int i;
	StandardHeader *hdr = (StandardHeader *) file;
	
	file += sizeof(StandardHeader);

	SWAP32(hdr->compSize);
	SWAP32(hdr->decompSize);

	switch (hdr->fileType) {
	case ANIMATION_FILE: {
		AnimHeader *animHead = (AnimHeader *) file;

		SWAP16(animHead->noAnimFrames);
		SWAP16(animHead->feetStartX);
		SWAP16(animHead->feetStartY);
		SWAP16(animHead->feetEndX);
		SWAP16(animHead->feetEndY);
		SWAP16(animHead->blend);

		CdtEntry *cdtEntry = (CdtEntry *) (file + sizeof(AnimHeader));
		for (i = 0; i < animHead->noAnimFrames; i++, cdtEntry++) {
			SWAP16(cdtEntry->x);
			SWAP16(cdtEntry->y);
			SWAP32(cdtEntry->frameOffset);

			FrameHeader *frameHeader = (FrameHeader *) (file + cdtEntry->frameOffset);
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
		MultiScreenHeader *mscreenHeader = (MultiScreenHeader *) file;

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
		ScreenHeader *screenHeader = (ScreenHeader *) (file + mscreenHeader->screen);

		SWAP16(screenHeader->width);
		SWAP16(screenHeader->height);
		SWAP16(screenHeader->noLayers);

		// layerHeader
		LayerHeader *layerHeader = (LayerHeader *) (file + mscreenHeader->layers);
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
			parallax = (Parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->bg_parallax[1];
		if (offset > 0) {
			parallax = (Parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// backgroundLayer
		offset = mscreenHeader->screen + sizeof(ScreenHeader);
		if (offset > 0) {
			parallax = (Parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// foregroundParallaxLayer
		offset = mscreenHeader->fg_parallax[0];
		if (offset > 0) {
			parallax = (Parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->fg_parallax[1];
		if (offset > 0) {
			parallax = (Parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}
		break;
	}
	case GAME_OBJECT: {
		ObjectHub *objectHub = (ObjectHub *) file;

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
		WalkGridHeader *walkGridHeader = (WalkGridHeader *) file;

		SWAP32(walkGridHeader->numBars);
		SWAP32(walkGridHeader->numNodes);

		BarData *barData = (BarData *) (file + sizeof(WalkGridHeader));
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

		uint16 *node = (uint16 *) (file + sizeof(WalkGridHeader) + walkGridHeader->numBars * sizeof(BarData));
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
		uint32 *list = (uint32 *) file;
		while (*list) {
			SWAP32(*list);
			list++;
		}
		break;
	}
	case TEXT_FILE: {
		TextHeader *textHeader = (TextHeader *) file;
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

		// points to the number of the ascii filename
		uint16 parent_res_file = _resConvTable[res * 2];

		assert(parent_res_file != 0xffff);

		// Relative resource within the file
		uint16 actual_res = _resConvTable[(res * 2) + 1];

		// First we have to find the file via the _resConvTable

		debug(5, "openResource %s res %d", _resourceFiles[parent_res_file], res);

		// If we're loading a cluster that's only available from one
		// of the CDs, remember which one so that we can play the
		// correct music.

		if (!(_cdTab[parent_res_file] & LOCAL_PERM))
			_curCd = _cdTab[parent_res_file] & 3;

		// Actually, as long as the file can be found we don't really
		// care which CD it's on. But if we can't find it, keep asking
		// for the CD until we do.

		File file;

		while (!file.open(_resourceFiles[parent_res_file])) {
			// If the file is supposed to be on hard disk, or we're
			// playing a demo, then we're in trouble if the file
			// can't be found!

			if ((_vm->_features & GF_DEMO) || (_cdTab[parent_res_file] & LOCAL_PERM))
				error("Could not find '%s'", _resourceFiles[parent_res_file]);

			getCd(_cdTab[parent_res_file] & 3);
		}

		// 1st DWORD of a cluster is an offset to the look-up table
		uint32 table_offset = file.readUint32LE();

		debug(6, "table offset = %d", table_offset);

		file.seek(table_offset + actual_res * 8, SEEK_SET);

		uint32 pos = file.readUint32LE();
		uint32 len = file.readUint32LE();

		file.seek(pos, SEEK_SET);

		debug(6, "res len %d", len);

		// Ok, we know the length so try and allocate the memory.
		// If it can't then old files will be ditched until it works.
		_resList[res].ptr = _vm->_memory->memAlloc(len, res);
		_resList[res].size = len;
		_resList[res].refCount = 0;

		file.read(_resList[res].ptr, len);

		if (dump) {
			StandardHeader *header = (StandardHeader *) _resList[res].ptr;
			char buf[256];
			char tag[10];
			File out;

			switch (header->fileType) {
			case ANIMATION_FILE:
				strcpy(tag, "anim");
				break;
			case SCREEN_FILE:
				strcpy(tag, "layer");
				break;
			case GAME_OBJECT:
				strcpy(tag, "object");
				break;
			case WALK_GRID_FILE:
				strcpy(tag, "walkgrid");
				break;
			case GLOBAL_VAR_FILE:
				strcpy(tag, "globals");
				break;
			case PARALLAX_FILE_null:
				strcpy(tag, "parallax");	// Not used!
				break;
			case RUN_LIST:
				strcpy(tag, "runlist");
				break;
			case TEXT_FILE:
				strcpy(tag, "text");
				break;
			case SCREEN_MANAGER:
				strcpy(tag, "screen");
				break;
			case MOUSE_FILE:
				strcpy(tag, "mouse");
				break;
			case WAV_FILE:
				strcpy(tag, "wav");
				break;
			case ICON_FILE:
				strcpy(tag, "icon");
				break;
			case PALETTE_FILE:
				strcpy(tag, "palette");
				break;
			default:
				strcpy(tag, "unknown");
				break;
			}

#if defined(MACOS_CARBON)
			sprintf(buf, ":dumps:%s-%d.dmp", tag, res);
#else
			sprintf(buf, "dumps/%s-%d.dmp", tag, res);
#endif

			if (!out.open(buf, "")) {
				if (out.open(buf, "", File::kFileWriteMode))
					out.write(_resList[res].ptr, len);
			}

			if (out.isOpen())
				out.close();
		}

		// close the cluster
		file.close();

#ifdef SCUMM_BIG_ENDIAN
		convertEndian(_resList[res].ptr, len);
#endif
	}

	_resList[res].refCount++;
	_resList[res].refTime = _resTime;

	return _resList[res].ptr;
}

void ResourceManager::closeResource(uint32 res) {
	assert(res < _totalResFiles);
	assert(_resList[res].refCount > 0);

	_resList[res].refCount--;
	_resList[res].refTime = _resTime;

#if !CACHE_CLUSTERS
	// Maybe it's useful on some platforms if memory is released as
	// quickly as possible?

	if (_resList[res].refCount == 0)
		remove(res);
#endif
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

void ResourceManager::passTime() {
	// In the original game this was called every game cycle. This allowed
	// for a more exact measure of when a loaded resouce was most recently
	// used. When the memory pool got too fragmented, the oldest and
	// largest of the closed resources would be expelled from the cache.

	// With the new memory manager, there is no single memory block that
	// can become fragmented. Therefore, it makes more sense to me to
	// measure an object's age in how many rooms ago it was last used.

	// Therefore, this function is now called when a new room is loaded.

	_resTime++;
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

	File file;

	if (!file.open(_resourceFiles[parent_res_file]))
		error("Cannot open %s", _resourceFiles[parent_res_file]);

	// 1st DWORD of a cluster is an offset to the look-up table
	uint32 table_offset = file.readUint32LE();

	// 2 dwords per resource + skip the position dword
	file.seek(table_offset + (actual_res * 8) + 4, SEEK_SET);

	return file.readUint32LE();
}

void ResourceManager::expelOldResources() {
	int nuked = 0;

	for (uint i = 0; i < _totalResFiles; i++) {
		if (_resList[i].ptr && _resList[i].refCount == 0 && _resTime - _resList[i].refTime >= MAX_CACHE_AGE) {
			remove(i);
			nuked++;
		}
	}

	debug(1, "%d resources died of old age", nuked);
}

void ResourceManager::printConsoleClusters(void) {
	if (_totalClusters) {
		for (uint i = 0; i < _totalClusters; i++) {
			Debug_Printf("%-20s ", _resourceFiles[i]);
			if (!(_cdTab[i] & LOCAL_PERM)) {
				switch (_cdTab[i] & 3) {
				case BOTH:
					Debug_Printf("CD 1 & 2\n");
					break;
				case CD1:
					Debug_Printf("CD 1\n");
					break;
				case CD2:
					Debug_Printf("CD 2\n");
					break;
				default:
					Debug_Printf("CD 3? Huh?!\n");
					break;
				}
			} else
				Debug_Printf("HD\n");
		}
		Debug_Printf("%d resources\n", _totalResFiles);
	} else
		Debug_Printf("Argh! No resources!\n");
}

void ResourceManager::examine(int res) {
	if (res < 0 || res >= (int) _totalResFiles)
		Debug_Printf("Illegal resource %d (there are %d resources 0-%d)\n", res, _totalResFiles, _totalResFiles - 1);
	else if (_resConvTable[res * 2] == 0xffff)
		Debug_Printf("%d is a null & void resource number\n", res);
	else {
		// open up the resource and take a look inside!
		StandardHeader *file_header = (StandardHeader *) openResource(res);

		switch (file_header->fileType) {
		case ANIMATION_FILE:
			Debug_Printf("<anim> %s\n", file_header->name);
			break;
		case SCREEN_FILE:
			Debug_Printf("<layer> %s\n", file_header->name);
			break;
		case GAME_OBJECT:
			Debug_Printf("<game object> %s\n", file_header->name);
			break;
		case WALK_GRID_FILE:
			Debug_Printf("<walk grid> %s\n", file_header->name);
			break;
		case GLOBAL_VAR_FILE:
			Debug_Printf("<global variables> %s\n", file_header->name);
			break;
		case PARALLAX_FILE_null:
			Debug_Printf("<parallax file NOT USED!> %s\n", file_header->name);
			break;
		case RUN_LIST:
			Debug_Printf("<run list> %s\n", file_header->name);
			break;
		case TEXT_FILE:
			Debug_Printf("<text file> %s\n", file_header->name);
			break;
		case SCREEN_MANAGER:
			Debug_Printf("<screen manager> %s\n", file_header->name);
			break;
		case MOUSE_FILE:
			Debug_Printf("<mouse pointer> %s\n", file_header->name);
			break;
		case ICON_FILE:
			Debug_Printf("<menu icon> %s\n", file_header->name);
			break;
		default:
			Debug_Printf("unrecognised fileType %d\n", file_header->fileType);
			break;
		}
		closeResource(res);
	}
}

void ResourceManager::kill(int res) {
	if (res < 0 || res >= (int) _totalResFiles) {
		Debug_Printf("Illegal resource %d (there are %d resources 0-%d)\n", res, _totalResFiles, _totalResFiles - 1);
		return;
	}

	if (!_resList[res].ptr) {
		Debug_Printf("Resource %d is not in memory\n", res);
		return;
	}

	if (_resList[res].refCount) {
		Debug_Printf("Resource %d is open - cannot remove\n", res);
		return;
	}

	remove(res);
	Debug_Printf("Trashed %d\n", res);
}

void ResourceManager::remove(int res) {
	if (_resList[res].ptr) {
		_vm->_memory->memFree(_resList[res].ptr);
		_resList[res].ptr = NULL;
		_resList[res].refCount = 0;
	}
}

/**
 * Remove all res files from memory - ready for a total restart. This includes
 * the player object and global variables resource.
 */

void ResourceManager::removeAll(void) {
	// We need to clear the FX queue, because otherwise the sound system
	// will still believe that the sound resources are in memory, and that
	// it's ok to close them.

	_vm->clearFxQueue();

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

	_vm->clearFxQueue();

	for (uint i = 0; i < _totalResFiles; i++) {
		// Don't nuke the global variables or the player object!
		if (i == 1 || i == CUR_PLAYER_ID)
			continue;

		if (_resList[i].ptr) {
			StandardHeader *header = (StandardHeader *) _resList[i].ptr;

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
			StandardHeader *header = (StandardHeader *) _resList[i].ptr;

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

void ResourceManager::getCd(int cd) {
	byte *textRes;

	// stop any music from playing - so the system no longer needs the
	// current CD - otherwise when we take out the CD, Windows will
	// complain!

	_vm->_logic->fnStopMusic(NULL);

	textRes = openResource(2283);
	_vm->displayMsg(_vm->fetchTextLine(textRes, 5 + cd) + 2, 0);
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

	while (1) {
		MouseEvent *me = _vm->mouseEvent();
		if (me && (me->buttons & (RD_LEFTBUTTONDOWN | RD_RIGHTBUTTONDOWN)))
			break;

		if (_vm->keyboardEvent())
			break;

		_vm->_graphics->updateDisplay();
		_vm->_system->delay_msecs(50);
	}

	_vm->removeMsg();
}

} // End of namespace Sword2
