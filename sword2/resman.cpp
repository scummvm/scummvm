/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "stdafx.h"
#include "sword2/sword2.h"
#include "sword2/driver/driver96.h"
#include "sword2/build_display.h"
#include "sword2/console.h"
#include "sword2/debug.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/interpreter.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/memory.h"
#include "sword2/mouse.h"	// for system setMouse & setLuggage routines
#include "sword2/protocol.h"
#include "sword2/resman.h"
#include "sword2/sound.h"	// for clearFxQueue() called from cacheNewCluster()
#include "sword2/router.h"

namespace Sword2 {

// ---------------------------------------------------------------------------
// welcome to the easy resource manager - written in simple code for easy
// maintenance
// 
// the resource compiler will create two files
//
//	resource.inf which is a list of ascii cluster file names
//	resource.tab which is a table which tells us which cluster a resource
//      is located in and the number within the cluster
// ---------------------------------------------------------------------------

#define NONE	 0
#define FETCHING 1

#define BUFFERSIZE	4096

ResourceManager	*res_man;	// declare the object global

// ---------------------------------------------------------------------------
//
//
//	resman.
//
//
// ---------------------------------------------------------------------------

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

struct _cd_inf {
	uint8 clusterName[20];	// Null terminated cluster name.
	uint8 cd;		// Cd cluster is on and whether it is on the local drive or not.
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

ResourceManager::ResourceManager(Sword2Engine *vm) {
	_vm = vm;

	// We read in the resource info which tells us the names of the
	// resource cluster files ultimately, although there might be groups
	// within the clusters at this point it makes no difference. We only
	// wish to know what resource files there are and what is in each

	File file;
	uint32 end;
	mem *temp;
	uint32 pos = 0;
	uint32 j = 0;

	_totalClusters = 0;

	if (!file.open("resource.inf")) {
		error("init cannot *OPEN* resource.inf");
	}

	end = file.size();

	//get some space for the incoming resource file - soon to be trashed
	temp = memory->allocMemory(end, MEM_locked, UID_temp);

	if (file.read(temp->ad, end) != end) {
		file.close();
		error("init cannot *READ* resource.inf");
	}

	file.close();

	// ok, we've loaded in the resource.inf file which contains a list of
	// all the files now extract the filenames
	do {
		// item must have an #0d0a
		while(temp->ad[j] != 13) {
			_resourceFiles[_totalClusters][pos] = temp->ad[j];
			j++;
			pos++;
		}

		// NULL terminate our extracted string
		_resourceFiles[_totalClusters][pos]=0;

		// Reset position in current slot between entries, skip the
		// 0x0a in the source and increase the number of clusters.

		pos = 0;
		j += 2;
		_totalClusters++;

		// TODO: put overload check here
	} while (j != end);	// using this method the Gode generated resource.inf must have #0d0a on the last entry

	// now load in the binary id to res conversion table
	if (!file.open("resource.tab")) {
		error("init cannot *OPEN* resource.tab");
	}

	// find how many resources
	end = file.size();

	_totalResFiles = end / 4;

	// table seems ok so malloc some space
	_resConvTable = (uint16 *) malloc(end);

	for (j = 0; j < end / 2; j++)
		_resConvTable[j] = file.readUint16LE();

	if (file.ioFailed()) {
		file.close();
		error("init cannot *READ* resource.tab");
	}

	file.close();

	if (!file.open("cd.inf")) {
		error("init cannot *OPEN* cd.inf");
	}

	_cd_inf *cdInf = new _cd_inf[_totalClusters];

	for (j = 0; j < _totalClusters; j++) {
		file.read(cdInf[j].clusterName, sizeof(cdInf[j].clusterName));
		cdInf[j].cd = file.readByte();
		
		if (file.ioFailed()) {
			error("init failed to read cd.inf. Insufficient entries?");
		}
	}

	file.close();

	for (j = 0; j < _totalClusters; j++) {
		uint32 i = 0;

		while (scumm_stricmp((char *) cdInf[i].clusterName, _resourceFiles[j]) != 0 && i < _totalClusters)
			i++;

		if (i == _totalClusters) {
			error("init, %s is not in cd.inf", _resourceFiles[j]);
		} else
			_cdTab[j] = cdInf[i].cd;
	}


	debug(5, "%d resources in %d cluster files", _totalResFiles, _totalClusters);
	for (j = 0; j < _totalClusters; j++)
		debug(5, "filename of cluster %d: -%s", j, _resourceFiles[j]);

	// create space for a list of pointers to mem's
	_resList = (mem **) malloc(_totalResFiles * sizeof(mem *));

	_age = (uint32 *) malloc(_totalResFiles * sizeof(uint32));
	_count = (uint16 *) malloc(_totalResFiles * sizeof(uint16));

	for (j = 0; j < _totalResFiles; j++) {
		// age must be 0 if the file is not in memory at all
		_age[j] = 0;
		_count[j] = 0;
	}

	_resTime = 1;	//cannot start at 0
	memory->freeMemory(temp);	//get that memory back

	// FIXME: Is this really needed?

	if (!file.open("revcd1.id")) {
		int index = 0;
/*
		// Scan for CD drives.
		for (char c = 'C'; c <= 'Z'; c++) {
			sprintf(_cdPath, "%c:\\", c);
			if (GetDriveType(_cdPath) == DRIVE_CDROM)
				_cdDrives[index++] = c;
		}
*/
		_cdDrives[index++] = 'C';
		
		if (index == 0) {
			error("init, cannot find CD drive");
		}

		while (index < 24)
			_cdDrives[index++] = 0;
	}
	else
		file.close();
}

ResourceManager::~ResourceManager(void) {
	// free up our mallocs
	free(_resList);
	free(_age);
	free(_count);
}

// Quick macro to make swapping in-place easier to write

#define SWAP16(x)	x = SWAP_BYTES_16(x)
#define SWAP32(x)	x = SWAP_BYTES_32(x)

void convertEndian(uint8 *file, uint32 len) {
	int i;
	_standardHeader *hdr = (_standardHeader *)file;
	
	file += sizeof(_standardHeader);

	SWAP32(hdr->compSize);
	SWAP32(hdr->decompSize);

	switch (hdr->fileType) {
	case ANIMATION_FILE: {
		_animHeader *animHead = (_animHeader *)file;

		SWAP16(animHead->noAnimFrames);
		SWAP16(animHead->feetStartX);
		SWAP16(animHead->feetStartY);
		SWAP16(animHead->feetEndX);
		SWAP16(animHead->feetEndY);
		SWAP16(animHead->blend);

		_cdtEntry *cdtEntry = (_cdtEntry *) (file + sizeof(_animHeader));
		for (i = 0; i < animHead->noAnimFrames; i++, cdtEntry++) {
			SWAP16(cdtEntry->x);
			SWAP16(cdtEntry->y);
			SWAP32(cdtEntry->frameOffset);

			_frameHeader *frameHeader = (_frameHeader *) (file + cdtEntry->frameOffset);
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
		_multiScreenHeader *mscreenHeader = (_multiScreenHeader *) file;

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
		_screenHeader *screenHeader = (_screenHeader*) (file + mscreenHeader->screen);

		SWAP16(screenHeader->width);
		SWAP16(screenHeader->height);
		SWAP16(screenHeader->noLayers);

		// layerHeader
		_layerHeader *layerHeader = (_layerHeader *) (file + mscreenHeader->layers);
		for (i = 0; i < screenHeader->noLayers; i++, layerHeader++) {
			SWAP16(layerHeader->x);
			SWAP16(layerHeader->y);
			SWAP16(layerHeader->width);
			SWAP16(layerHeader->height);
			SWAP32(layerHeader->maskSize);
			SWAP32(layerHeader->offset);
		}

		// backgroundParallaxLayer
		_parallax *parallax;
		int offset;
		offset = mscreenHeader->bg_parallax[0];
		if (offset > 0) {
			parallax = (_parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->bg_parallax[1];
		if (offset > 0) {
			parallax = (_parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// backgroundLayer
		offset = mscreenHeader->screen + sizeof(_screenHeader);
		if (offset > 0) {
			parallax = (_parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		// foregroundParallaxLayer
		offset = mscreenHeader->fg_parallax[0];
		if (offset > 0) {
			parallax = (_parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}

		offset = mscreenHeader->fg_parallax[1];
		if (offset > 0) {
			parallax = (_parallax *) (file + offset);
			SWAP16(parallax->w);
			SWAP16(parallax->h);
		}
		break;
	}
	case GAME_OBJECT: {
		_object_hub *objectHub = (_object_hub *)file;

		objectHub->type = (int)SWAP_BYTES_32(objectHub->type);
		SWAP32(objectHub->logic_level);

		for (i = 0; i < TREE_SIZE; i++) {
			SWAP32(objectHub->logic[i]);
			SWAP32(objectHub->script_id[i]);
			SWAP32(objectHub->script_pc[i]);
		}
		break;
	}
	case WALK_GRID_FILE: {
		_walkGridHeader	*walkGridHeader = (_walkGridHeader *)file;

		SWAP32(walkGridHeader->numBars);
		SWAP32(walkGridHeader->numNodes);

		_barData *barData = (_barData *) (file + sizeof(_walkGridHeader));
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

		uint16 *node = (uint16 *) (file + sizeof(_walkGridHeader) + walkGridHeader->numBars * sizeof(_barData));
		for (i = 0; i < walkGridHeader->numNodes*2; i++) {
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
		_textHeader *textHeader = (_textHeader *)file;
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

uint8 *ResourceManager::openResource(uint32 res) {
	// returns ad of resource. Loads if not in memory
	// retains a count
	// resource can be aged out of memory if count = 0
	// the resource is locked while count != 0 i.e. until a closeResource is
	// called

	File	file;
	uint16	parent_res_file;
	uint16	actual_res;
	uint32	pos, len;

	uint32	table_offset;

//#ifdef _SWORD2_DEBUG
	if (res >= _totalResFiles)
		error("open illegal resource %d (there are %d resources 0-%d)", res, _totalResFiles, _totalResFiles - 1);
//#endif

	// is the resource in memory already?
	// if the file is not in memory then age should and MUST be 0
	if (!_age[res]) {
		// fetch the correct file and read in the correct portion
		// if the file cannot fit then we must trash the oldest large
		// enough floating file

		// points to the number of the ascii filename
		parent_res_file = _resConvTable[res * 2];

//#ifdef _SWORD2_DEBUG
		if (parent_res_file == 0xffff)
			error("open tried to open null & void resource number %d", res);
//#endif

		// relative resource within the file
		actual_res = _resConvTable[(res * 2) + 1];

		// first we have to find the file via the _resConvTable

		debug(5, "resOpen %s res %d", _resourceFiles[parent_res_file], res);

		// ** at this point here we start to think about where the
		// ** file is and prompt the user for the right CD to be
		// ** inserted
		// **
		// ** we need to know the position that we're at within the
		// ** game - LINC should write this someplace.

/* these probably aren't necessary - khalek
		if (!(_cdTab[parent_res_file] & LOCAL_CACHE) && !(_cdTab[parent_res_file] & LOCAL_PERM)) {
			// This cluster is on a CD, we need to cache a new one.
			cacheNewCluster(parent_res_file);
		} else if (!(_cdTab[parent_res_file] & LOCAL_PERM)) {
			// Makes sure that the correct CD is in the drive.
			getCd(_cdTab[parent_res_file] & 3);
		}
*/

		// This part of the cluster caching is pretty useful though:
		//
		// If we're loading a cluster that's only available from one
		// of the CDs, remember which one so that we can play the
		// correct music.
		//
		// The code to ask for the correct CD will probably be needed
		// later, too.
		//
		// And there's some music / FX stuff in cacheNewCluster() that
		// might be needed as well.
		//
		// But this will do for now.

		if (!(_cdTab[parent_res_file] & LOCAL_PERM)) {
			_curCd = _cdTab[parent_res_file] & 3;
		}

		// open the cluster file
		if (!file.open(_resourceFiles[parent_res_file]))
			error("open cannot *OPEN* %s", _resourceFiles[parent_res_file]);


		// 1st DWORD of a cluster is an offset to the look-up table
		table_offset = file.readUint32LE();

		debug(5, "table offset = %d", table_offset);

		// 2 dwords per resource
		file.seek(table_offset + actual_res * 8, SEEK_SET);
		// get position of our resource within the cluster file
		pos = file.readUint32LE();
		// read the length
		len = file.readUint32LE();

		// ** get to position in file of our particular resource
		file.seek(pos, SEEK_SET);

		debug(5, "res len %d", len);

		// ok, we know the length so try and allocate the memory
		// if it can't then old files will be ditched until it works
		_resList[res] = memory->allocMemory(len, MEM_locked, res);

		// now load the file
		// hurray, load it in.
		file.read(_resList[res]->ad, len);

		// close the cluster
		file.close();

#ifdef SCUMM_BIG_ENDIAN
		convertEndian((uint8 *) _resList[res]->ad, len);
#endif
	} else {
		debug(5, "RO %d, already open count=%d", res, _count[res]);
	}

	// number of times opened - the file won't move in memory while count
	// is non zero
	_count[res]++;

	// update the accessed time stamp - touch the file in other words
	_age[res] = _resTime;

	// pass the address of the mem & lock the memory too
	// might be locked already (if count > 1)
	memory->lockMemory(_resList[res]);

	return (uint8 *) _resList[res]->ad;
}

uint8 ResourceManager::checkValid(uint32 res) {
	// returns '1' if resource is valid, otherwise returns '0'
	// used in startup.cpp to ignore invalid screen-manager resources

	uint16 parent_res_file;

	// resource number out of range
	if (res >= _totalResFiles)
		return 0;

	// points to the number of the ascii filename
	parent_res_file = _resConvTable[res * 2];

	// null & void resource
	if (parent_res_file == 0xffff)
		return 0;

	// ok
	return 1;
}

void ResourceManager::nextCycle(void) {
	// increment the cycle and calculate actual per-cycle memory useage

#ifdef _SWORD2_DEBUG
	uint32 j;
#endif

#ifdef _SWORD2_DEBUG
	_currentMemoryUsage = 0;

	for (j = 1; j < _totalResFiles; j++) {
		// was accessed last cycle
		if (_age[j] == _resTime)
			_currentMemoryUsage += _resList[j]->size;
	}
#endif

	_resTime++;

	// if you left the game running for a hundred years when this went to 0
	// there'd be a resource left stuck in memory - after another hundred
	// years there'd be another...
	//
	// Mind you, by then the our get_msecs() function will have wrapped
	// around too, probably causing a mess of other problems.

	if (!_resTime)
		_resTime++;
}

uint32 ResourceManager::fetchUsage(void) {
	// returns memory usage previous cycle
	return _currentMemoryUsage;
}

void ResourceManager::closeResource(uint32 res) {
	// decrements the count
	// resource floats when count = 0

//#ifdef _SWORD2_DEBUG
	if (res >= _totalResFiles)
		error("closing illegal resource %d (there are %d resources 0-%d)", res, _totalResFiles, _totalResFiles - 1);

	//closing but isnt open?
	if (!(_count[res]))
		error("closeResource: closing %d but it isn't open", res);
//#endif

	//one less has it open
	_count[res]--;

	//if noone has the file open then unlock and allow to float
	if (!_count[res]) {
		// pass the address of the mem
		memory->floatMemory(_resList[res]);
	}
}

uint32 ResourceManager::fetchLen(uint32 res) {
	// returns the total file length of a resource - i.e. all headers are
	// included too

	File fh;
	uint16 parent_res_file;
	uint16 actual_res;
	uint32 len;
	uint32 table_offset;

	// points to the number of the ascii filename
	parent_res_file = _resConvTable[res * 2];

	// relative resource within the file
	actual_res = _resConvTable[(res * 2) + 1];

	// first we have to find the file via the _resConvTable
	// open the cluster file

	if (!fh.open(_resourceFiles[parent_res_file]))
		error("fetchLen cannot *OPEN* %s", _resourceFiles[parent_res_file]);

	// 1st DWORD of a cluster is an offset to the look-up table
	table_offset = fh.readUint32LE();

	// 2 dwords per resource + skip the position dword
	fh.seek(table_offset + (actual_res * 8) + 4, SEEK_SET);

	// read the length
	len = fh.readUint32LE();
	return len;
}

char *ResourceManager::fetchCluster(uint32 res) {
	// returns a pointer to the ascii name of the cluster file which
	// contains resource res
	return _resourceFiles[_resConvTable[res * 2]];
}

uint32 ResourceManager::fetchAge(uint32 res) {
	// return the age of res
	return _age[res];
}

uint32 ResourceManager::fetchCount(uint32 res) {
	// return the open count of res
	return _count[res];
}

uint32 ResourceManager::helpTheAgedOut(void) {
	// remove from memory the oldest closed resource

	uint32 oldest_res;	// holds id of oldest found so far when we have to chuck stuff out of memory
	uint32 oldest_age;	// age of above during search
	uint32 j;
	uint32 largestResource = 0;

	oldest_age = _resTime;
	oldest_res = 0;

	for (j = 2; j < _totalResFiles; j++) {
		// not held open and older than this one
		if (!_count[j] && _age[j] && _age[j] <= oldest_age) {	
			if (_age[j] == oldest_age && _resList[j]->size > largestResource)	{
				// Kick old resource of oldest age and largest
				// size (Helps the poor defragger).
				oldest_res = j;
				largestResource = _resList[j]->size;
			} else if (_age[j] < oldest_age)	{
				oldest_res = j;
				oldest_age = _age[j];
				largestResource = _resList[j]->size;
			}
		}
	}

	// there was not a file we could release
	// no bytes released - oh dear, lets hope this never happens
	if (!oldest_res)
		return 0;

	debug(5, "removing %d, age %d, size %d", oldest_res, _age[oldest_res], _resList[oldest_res]->size);

	// trash this old resource

	_age[oldest_res] = 0;		// effectively gone from _resList
	memory->freeMemory(_resList[oldest_res]);	// release the memory too

	return _resList[oldest_res]->size;	// return bytes freed
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
	_standardHeader	*file_header;

	if (res < 0 || res >= (int) _totalResFiles)
		Debug_Printf("Illegal resource %d (there are %d resources 0-%d)\n", res, _totalResFiles, _totalResFiles - 1);
	else if (_resConvTable[res * 2] == 0xffff)
		Debug_Printf("%d is a null & void resource number\n", res);
	else {
		// open up the resource and take a look inside!
		file_header = (_standardHeader*) openResource(res);

		// Debug_Printf("%d\n", file_header->fileType);
		// Debug_Printf("%s\n", file_header->name);

		//----------------------------------------------------
		// resource types: (taken from header.h)

		// 1:  ANIMATION_FILE
		//      all normal animations & sprites including mega-sets &
		//	font files which are the same format
		// 2:  SCREEN_FILE
		//      each contains background, palette, layer sprites,
		//	parallax layers & shading mask
		// 3:  GAME_OBJECT
		//      each contains object hub + structures + script data
		// 4:  WALK_GRID_FILE
		//      walk-grid data
		// 5:  GLOBAL_VAR_FILE
		//      all the global script variables in one file; "there can
		//	be only one"
		// 6:  PARALLAX_FILE_null
		//      NOT USED
		// 7:  RUN_LIST
		//      each contains a list of object resource ids
		// 8:  TEXT_FILE
		//      each contains all the lines of text for a location or a
		//	character's conversation script
		// 9:  SCREEN_MANAGER
		//      one for each location; this contains special startup
		//	scripts
		// 10: MOUSE_FILE
		//      mouse pointers and luggage icons (sprites in General,
		//	Mouse pointers & Luggage icons)
		// 11: WAV_FILE
		//      NOT USED HERE
		// 12: ICON_FILE
		//      menu icon (sprites in General and Menu icons)
		// 13: PALETTE_FILE
		//      NOT USED HERE
		//----------------------------------------------------

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

	// if noone has the file open then unlock and allow to float
	if (!_count[res]) {
		if (_age[res]) {
			_age[res] = 0;		// effectively gone from _resList
			memory->freeMemory(_resList[res]);	// release the memory too
			Debug_Printf("Trashed %d\n", res);
		} else
			Debug_Printf("%d not in memory\n", res);
	} else
		Debug_Printf("File is open - cannot remove\n");
}

void ResourceManager::remove(uint32 res) {
	if (_age[res]) {
		_age[res] = 0;			// effectively gone from _resList
		memory->freeMemory(_resList[res]);	// release the memory too
		debug(5, " - Trashing %d", res);
	} else
		debug(5, "remove(%d) not even in memory!", res);
}

void ResourceManager::removeAll(void) {
	// remove all res files from memory - ready for a total restart
	// including player object & global variables resource

	int j;
	uint32 res;

	j = memory->_baseMemBlock;

	do {
		if (memory->_memList[j].uid < 65536) {	// a resource
			res = memory->_memList[j].uid;
			_age[res] = 0;		// effectively gone from _resList
			memory->freeMemory(_resList[res]);	// release the memory too
		}

		j = memory->_memList[j].child;
	} while	(j != -1);
}

void ResourceManager::killAll(bool wantInfo) {
	// remove all res files from memory
	// its quicker to search the mem blocs for res files than search
	// resource lists for those in memory

	int j;
	uint32 res;
	uint32 nuked = 0;
  	_standardHeader *header;

	j = memory->_baseMemBlock;

	do {
		if (memory->_memList[j].uid < 65536) {	// a resource
			res = memory->_memList[j].uid;

			// not the global vars which are assumed to be open in
			// memory & not the player object!
			if (res != 1 && res != CUR_PLAYER_ID) {
				header = (_standardHeader *) openResource(res);
				closeResource(res);

				_age[res] = 0;		// effectively gone from _resList
				memory->freeMemory(_resList[res]);	// release the memory too
				nuked++;

				// if this was called from the console,
				if (wantInfo) {
					Debug_Printf("Nuked %5d: %s\n", res, header->name);
					debug(5, " nuked %d: %s", res, header->name);
				}	
			}
		}
		j = memory->_memList[j].child;
	} while (j != -1);

	// if this was called from the console
	if (wantInfo) {
		Debug_Printf("Expelled %d resource(s)\n", nuked);
	}
}

//----------------------------------------------------------------------------
// Like killAll but only kills objects (except George & the variable table of
// course) - ie. forcing them to reload & restart their scripts, which
// simulates the effect of a save & restore, thus checking that each object's
// re-entrant logic works correctly, and doesn't cause a statuette to
// disappear forever, or some plaster-filled holes in sand to crash the game &
// get James in trouble again.

void ResourceManager::killAllObjects(bool wantInfo) {
	// remove all object res files from memory, excluding George
	// its quicker to search the mem blocs for res files than search
	// resource lists for those in memory

	int j;
	uint32 res;
	uint32 nuked = 0;
 	_standardHeader *header;

	j = memory->_baseMemBlock;

	do {
		if (memory->_memList[j].uid < 65536) {	// a resource
			res = memory->_memList[j].uid;
			//not the global vars which are assumed to be open in
			// memory & not the player object!
			if (res != 1 && res != CUR_PLAYER_ID) {
				header = (_standardHeader*) openResource(res);
				closeResource(res);

				if (header->fileType == GAME_OBJECT) {
					_age[res] = 0;		// effectively gone from _resList
					memory->freeMemory(_resList[res]);	// release the memory too
   					nuked++;

					// if this was called from the console
					if (wantInfo) {
						Debug_Printf("Nuked %5d: %s\n", res, header->name);
						debug(5, " nuked %d: %s", res, header->name);
					}	
				}
			}
		}
		j = memory->_memList[j].child;
	} while (j != -1);

	// if this was called from the console
	if (wantInfo)
		Debug_Printf("Expelled %d object resource(s)\n", nuked);
}

void ResourceManager::cacheNewCluster(uint32 newCluster) {
	// Stop any music from streaming off the CD before we start the
	// cluster-copy!
	//
	// eg. the looping restore-panel music will still be playing if we
	// restored a game to a different cluster on the same CD - and music
	// streaming would interfere with cluster copying, slowing it right
	// down - but if we restored to a different CD the music is stopped
	// in getCd() when it asks for the CD

	g_logic->fnStopMusic(NULL);

	_vm->clearFxQueue();	// stops all fx & clears the queue
	getCd(_cdTab[newCluster] & 3);

	// Kick out old cached cluster and load the new one.
	uint32 i = 0;
	while (!(_cdTab[i] & LOCAL_CACHE) && i < _totalClusters)
		i++;

	if (i < _totalClusters)	{
		SVM_SetFileAttributes(_resourceFiles[i], FILE_ATTRIBUTE_NORMAL);
		SVM_DeleteFile(_resourceFiles[i]);
		_cdTab[i] &= (0xff - LOCAL_CACHE);
		FILE *file;
		file = fopen("cd.inf", "r+b");

		if (file == NULL) {
			error("init cannot *OPEN* cd.inf");
		}	

		_cd_inf cdInf;
		
		do {
			fread(&cdInf, 1, sizeof(_cd_inf), file);
		} while ((scumm_stricmp((char *) cdInf.clusterName, _resourceFiles[i]) != 0) && !feof(file));

		if (feof(file)) {
			error("cacheNewCluster cannot find %s in cd.inf", _resourceFiles[i]);
		}

		fseek(file, -1, SEEK_CUR);
		fwrite(&_cdTab[i], 1, 1, file);
		fclose(file);
	}

	char buf[1024];
	sprintf(buf, "%sClusters\\%s", _cdPath, _resourceFiles[newCluster]);

	g_graphics->waitForFade();

	if (g_graphics->getFadeStatus() != RDFADE_BLACK) {
		g_graphics->fadeDown();
		g_graphics->waitForFade();
	}

	g_graphics->clearScene();

	_vm->setMouse(0);
	_vm->setLuggage(0);

	uint8 *bgfile;
	bgfile = openResource(2950);	// open the screen resource
	g_graphics->initialiseBackgroundLayer(NULL);
	g_graphics->initialiseBackgroundLayer(NULL);
	g_graphics->initialiseBackgroundLayer(_vm->fetchBackgroundLayer(bgfile));
	g_graphics->initialiseBackgroundLayer(NULL);
	g_graphics->initialiseBackgroundLayer(NULL);
	g_graphics->setPalette(0, 256, _vm->fetchPalette(bgfile), RDPAL_FADE);

	g_graphics->renderParallax(_vm->fetchBackgroundLayer(bgfile), 2);
	closeResource(2950);		// release the screen resource

	// Git rid of read-only status, if it is set.
	SVM_SetFileAttributes(_resourceFiles[newCluster], FILE_ATTRIBUTE_NORMAL);

	File inFile, outFile;
	
	inFile.open(buf);
	outFile.open(_resourceFiles[newCluster], NULL, File::kFileWriteMode);

	if (!inFile.isOpen() || !outFile.isOpen()) {
		error("Cache new cluster could not copy %s to %s", buf, _resourceFiles[newCluster]);
	}

	_spriteInfo textSprite;
	_spriteInfo barSprite;
	mem *text_spr;
	_frameHeader *frame;
	uint8 *loadingBar;
	_cdtEntry *cdt;

	text_spr = fontRenderer->makeTextSprite(_vm->fetchTextLine(openResource(2283), 8) + 2, 640, 187, _vm->_speechFontId);

	frame = (_frameHeader*) text_spr->ad;

	textSprite.x = g_graphics->_screenWide /2 - frame->width / 2;
	textSprite.y = g_graphics->_screenDeep /2 - frame->height / 2 - RDMENU_MENUDEEP;
	textSprite.w = frame->width;
	textSprite.h = frame->height;
	textSprite.scale = 0;
	textSprite.scaledWidth	= 0;
	textSprite.scaledHeight	= 0;
	textSprite.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	textSprite.blend = 0;
	textSprite.colourTable	= 0;

	closeResource(2283);

	loadingBar = openResource(2951);

	frame = _vm->fetchFrameHeader(loadingBar, 0);
	cdt   = _vm->fetchCdtEntry(loadingBar, 0);

	barSprite.x = cdt->x;
	barSprite.y = cdt->y;
	barSprite.w = frame->width;
	barSprite.h = frame->height;
	barSprite.scale = 0;
	barSprite.scaledWidth = 0;
	barSprite.scaledHeight = 0;
	barSprite.type = RDSPR_RLE256FAST | RDSPR_TRANS;
	barSprite.blend = 0;
	barSprite.colourTable = 0;

	closeResource(2951);

	loadingBar = openResource(2951);
	frame = _vm->fetchFrameHeader(loadingBar, 0);
	barSprite.data = (uint8 *) (frame + 1);
	closeResource(2951);

	int16 barX = barSprite.x;
	int16 barY = barSprite.y;
	int16 textX = textSprite.x;
	int16 textY = textSprite.y;

	g_graphics->drawSprite(&barSprite);
	barSprite.x = barX;
	barSprite.y = barY;

	textSprite.data	= text_spr->ad + sizeof(_frameHeader);
	g_graphics->drawSprite(&textSprite);
	textSprite.x = textX;
	textSprite.y = textY;

	g_graphics->fadeUp();
	g_graphics->waitForFade();

	uint32 size = inFile.size();

	char buffer[BUFFERSIZE];
	int stepSize = (size / BUFFERSIZE) / 100;
	uint32  read = 0;
	int step = stepSize;
	int fr = 0;
	uint32 realRead = 0;

	do {
		realRead = inFile.read(buffer, BUFFERSIZE);
		read += realRead;
		if (outFile.write(buffer, realRead) != realRead) {
			error("Cache new cluster could not copy %s to %s", buf, _resourceFiles[newCluster]);
		}

		if (step == stepSize) {
			step = 0;
			// open the screen resource
			bgfile = openResource(2950);
			g_graphics->renderParallax(_vm->fetchBackgroundLayer(bgfile), 2);
			// release the screen resource
			closeResource(2950);
			loadingBar = openResource(2951);
			frame = _vm->fetchFrameHeader(loadingBar, fr);
			barSprite.data = (uint8 *) (frame + 1);
			closeResource(2951);
			g_graphics->drawSprite(&barSprite);
			barSprite.x = barX;
			barSprite.y = barY;

			textSprite.data	= text_spr->ad + sizeof(_frameHeader);
			g_graphics->drawSprite(&textSprite);
			textSprite.x = textX;
			textSprite.y = textY;

			fr++;
		} else
			step++;

		g_graphics->updateDisplay();
	} while ((read % BUFFERSIZE) == 0);

	if (read != size) {
		error("Cache new cluster could not copy %s to %s", buf, _resourceFiles[newCluster]);
	}

	inFile.close();
	outFile.close();
	memory->freeMemory(text_spr);

	g_graphics->clearScene();

	g_graphics->fadeDown();
	g_graphics->waitForFade();
	g_graphics->fadeUp();

	// Git rid of read-only status.
	SVM_SetFileAttributes(_resourceFiles[newCluster], FILE_ATTRIBUTE_NORMAL);

	// Update cd.inf and _cdTab
	_cdTab[newCluster] |= LOCAL_CACHE;

	FILE *file;
	file = fopen("cd.inf", "r+b");

	if (file == NULL) {
		error("init cannot *OPEN* cd.inf");
	}

	_cd_inf cdInf;
	
	do {
		fread(&cdInf, 1, sizeof(_cd_inf), file);
	} while (scumm_stricmp((char *) cdInf.clusterName, _resourceFiles[newCluster]) != 0 && !feof(file));

	if (feof(file)) {
		error("cacheNewCluster cannot find %s in cd.inf", _resourceFiles[newCluster]);
	}

	fseek(file, -1, SEEK_CUR);
	fwrite(&_cdTab[newCluster], 1, 1, file);
	fclose(file);
}

void ResourceManager::getCd(int cd) {
	// TODO support a separate path for cd data?
	
	bool done = false;
	char sCDName[_MAX_PATH];
	uint32 dwMaxCompLength, dwFSFlags;
	mem *text_spr;
	_frameHeader *frame;
	_spriteInfo spriteInfo;
	int16 oldY;
	int16 oldX;
	FILE *file;
	char name[16];
	int offNetwork = 0;
	int index = 0;
	uint8 *textRes;

	// don't ask for CD's in the playable demo downloaded from our
	// web-site!
	if (_vm->_features & GF_DEMO)
		return;

#ifdef _PCGUIDE
	// don't ask for CD in the patch for the demo on "PC Guide" magazine
	return;
#endif

	sprintf(name, "revcd%d.id", cd);
	file = fopen(name, "r");

	if (file == NULL) {
		// Determine what CD is in the drive, and either use it or ask
		// the user to insert the correct CD.
		// Scan all CD drives for our CD as well.
		while(_cdDrives[index] != 0 && index < 24) {
			sprintf(_cdPath, "%c:\\", _cdDrives[index]);

			if (!SVM_GetVolumeInformation(_cdPath, sCDName, _MAX_PATH, NULL, &dwMaxCompLength, &dwFSFlags, NULL, 0))	{
				// Force the following code to ask for the correct CD.
				sCDName[0] = 0;
			}

			_curCd = cd;
		
			if (!scumm_stricmp(sCDName,CD1_LABEL)) {
				if (cd == CD1)
					return;
			} else if (!scumm_stricmp(sCDName,CD2_LABEL)) {
				if (cd == CD2)
					return;
			}

			index++;
		}
	} else {
		// must be running off the network, but still want to see
		// CD-requests to show where they would occur when playing
		// from CD
		debug(5, "RUNNING OFF NETWORK");

		fscanf(file, "%s", _cdPath);
		fclose(file);

		if (_curCd == cd)
			return;
		else
			_curCd = cd;

		// don't show CD-requests if testing anims or text/speech
		if (SYSTEM_TESTING_ANIMS || SYSTEM_TESTING_TEXT)
			return;

		offNetwork = 1;
	}

	// stop any music from playing - so the system no longer needs the
	// current CD - otherwise when we take out the CD, Windows will
	// complain!

	g_logic->fnStopMusic(NULL);

	textRes = openResource(2283);
	_vm->displayMsg(_vm->fetchTextLine(textRes, 5 + cd) + 2, 0);
	text_spr = fontRenderer->makeTextSprite(_vm->fetchTextLine(textRes, 5 + cd) + 2, 640, 187, _vm->_speechFontId);

	frame = (_frameHeader*) text_spr->ad;

	spriteInfo.x = g_graphics->_screenWide / 2 - frame->width / 2;
	spriteInfo.y = g_graphics->_screenDeep / 2 - frame->height / 2 - RDMENU_MENUDEEP;
	spriteInfo.w = frame->width;
	spriteInfo.h = frame->height;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth	= 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN | RDSPR_NOCOMPRESSION | RDSPR_TRANS;
	spriteInfo.blend = 0;
	spriteInfo.data = text_spr->ad + sizeof(_frameHeader);
	spriteInfo.colourTable	= 0;
	oldY = spriteInfo.y;
	oldX = spriteInfo.x;

	closeResource(2283);

	do {
		if (offNetwork == 1)
			done = true;
		else {
			index = 0;
			while (_cdDrives[index] != 0 && !done && index < 24) {
				sprintf(_cdPath, "%c:\\", _cdDrives[index]);

				if (!SVM_GetVolumeInformation(_cdPath, sCDName, _MAX_PATH, NULL, &dwMaxCompLength, &dwFSFlags, NULL, 0)) {
					sCDName[0] = 0;
				}

				if (!scumm_stricmp(sCDName, CD1_LABEL)) {
					if (cd == CD1)
						done = true;
				} else if (!scumm_stricmp(sCDName, CD2_LABEL)) {
					if (cd == CD2)
						done = true;
				}
				index++;
			}
		}
		
		g_graphics->updateDisplay();

		g_graphics->clearScene();
		g_graphics->drawSprite(&spriteInfo);	// Keep the message there even when the user task swaps.
		spriteInfo.y = oldY;		// Drivers change the y co-ordinate, don't know why...
		spriteInfo.x = oldX;
	} while (!done);

	memory->freeMemory(text_spr);
	_vm->removeMsg();
}

} // End of namespace Sword2
