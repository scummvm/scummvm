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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "stdafx.h"

#include "base/engine.h"
#include "common/file.h"

#include "driver/driver96.h"
#include "build_display.h"
#include "console.h"
#include "debug.h"
#include "defs.h"
#include "function.h"
#include "header.h"
#include "interpreter.h"
#include "maketext.h"
#include "memory.h"
#include "mouse.h"	// for system Set_mouse & Set_luggage routines
#include "protocol.h"
#include "resman.h"
#include "sound.h"	// (James22july97) for Clear_fx_queue() called from CacheNewCluster()
#include "sword2.h"	// (James11aug97) for CloseGame()
#include "router.h"

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

resMan	res_man;	//declare the object global

// ---------------------------------------------------------------------------
//
//
//	resman.
//
//
// ---------------------------------------------------------------------------

#define BOTH		0x0		// Cluster is on both CDs
#define CD1		0x1		// Cluster is on CD1 only
#define CD2		0x2		// Cluster is on CD2 only
#define LOCAL_CACHE	0x4		// Cluster is cached on HDD
#define LOCAL_PERM	0x8		// Cluster is on HDD.

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

typedef struct {
	uint8 clusterName[20];	// Null terminated cluster name.
	uint8 cd;		// Cd cluster is on and whether it is on the local drive or not.
} GCC_PACK _cd_inf;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

// ---------------------------------------------------------------------------

void resMan::InitResMan(void) { //Tony29May96
	// We read in the resource info which tells us the names of the
	// resource cluster files ultimately, although there might be groups
	// within the clusters at this point it makes no difference. We only
	// wish to know what resource files there are and what is in each

	File file;
	uint32 end;
	mem *temp;
	uint32 pos = 0;
	uint32 j = 0;

	total_clusters = 0;

	if (!file.open("resource.inf")) {
		Zdebug("InitResMan cannot *OPEN* resource.inf");
		ExitWithReport("InitResMan cannot *OPEN* resource.inf [file=%s line=%u]", __FILE__, __LINE__);
	}

	end = file.size();

	// Zdebug("seek end %d %d", fh, end);

	//get some space for the incoming resource file - soon to be trashed
	temp = Twalloc(end, MEM_locked, UID_temp);

	if (file.read(temp->ad, end) != end) {
		file.close();
		Zdebug("InitResMan cannot *READ* resource.inf");
		ExitWithReport("InitResMan cannot *READ* resource.inf [file=%s line=%u]", __FILE__, __LINE__);
	}

	file.close();

	// ok, we've loaded in the resource.inf file which contains a list of
	// all the files now extract the filenames
	do {
		// item must have an #0d0a
		while(temp->ad[j] != 13) {
			resource_files[total_clusters][pos] = temp->ad[j];
			j++;
			pos++;
		}

		// NULL terminate our extracted string
		resource_files[total_clusters][pos]=0;

		// Reset position in current slot between entries, skip the
		// 0x0a in the source and increase the number of clusters.

		pos = 0;
		j += 2;
		total_clusters++;

		// TODO: put overload check here
	} while (j != end);	// using this method the Gode generated resource.inf must have #0d0a on the last entry

	// now load in the binary id to res conversion table
	if (!file.open("resource.tab")) {
		Zdebug("InitResMan cannot *OPEN* resource.tab");
		ExitWithReport("InitResMan cannot *OPEN* resource.tab [file=%s line=%u]", __FILE__, __LINE__);
	}

	// find how many resources
	end = file.size();

	total_res_files = end / 4;

	// table seems ok so malloc some space
	res_conv_table = (uint16 *) malloc(end);

	for (j = 0; j < end / 2; j++)
		res_conv_table[j] = file.readUint16LE();

	if (file.ioFailed()) {
		file.close();
		Zdebug("InitResMan cannot *READ* resource.tab");
		ExitWithReport("InitResMan cannot *READ* resource.tab [file=%s line=%u]", __FILE__, __LINE__);
	}

	file.close();

	if (!file.open("cd.inf")) {
		Zdebug("InitResMan cannot *OPEN* cd.inf");
		ExitWithReport("InitResMan cannot *OPEN* cd.inf [file=%s line=%u]", __FILE__, __LINE__);
	}

	_cd_inf *cdInf = new _cd_inf[total_clusters];

	for (j = 0; j < total_clusters; j++) {
		file.read(cdInf[j].clusterName, sizeof(cdInf[j].clusterName));
		cdInf[j].cd = file.readByte();
		
		if (file.ioFailed()) {
			Zdebug("InitResMan failed to read cd.inf. Insufficient entries?");
			ExitWithReport("InitResMan failed to read cd.inf. Insufficient entries? [file=%s line=%u]", __FILE__, __LINE__);
		}
	}

	file.close();

	for (j = 0; j < total_clusters; j++) {
		uint32 i = 0;

		while (scumm_stricmp((char *) cdInf[i].clusterName, resource_files[j]) != 0 && i < total_clusters)
			i++;

		if (i == total_clusters) {
			Zdebug("InitResMan, %s is not in cd.inf", resource_files[j]);
			ExitWithReport("InitResMan, %s is not in cd.inf [file=%s line=%u]",resource_files[j], __FILE__, __LINE__);
		} else
			cdTab[j] = cdInf[i].cd;
	}


	Zdebug("\n%d resources in %d cluster files", total_res_files, total_clusters);
	for (j = 0; j < total_clusters; j++)
		Zdebug("filename of cluster %d: -%s", j, resource_files[j]);
	Zdebug("");

	// create space for a list of pointers to mem's
	resList = (mem **) malloc(total_res_files * sizeof(mem *));

	age = (uint32 *) malloc(total_res_files * sizeof(uint32));
	// status = (uint16 *) malloc(total_res_files * sizeof(uint16));
	count = (uint16 *) malloc(total_res_files * sizeof(uint16));

	for (j = 0; j < total_res_files; j++) {
		// age must be 0 if the file is not in memory at all
		age[j] = 0;
		count[j] = 0;
	}

	resTime = 1;	//cannot start at 0
	Free_mem(temp);	//get that memory back

	// FIXME: Is this really needed?

	if (!file.open("revcd1.id")) {
		int index = 0;
/*
		// Scan for CD drives.
		for (char c = 'C'; c <= 'Z'; c++) {
			sprintf(cdPath, "%c:\\", c);
			if (GetDriveType(cdPath) == DRIVE_CDROM)
				cdDrives[index++] = c;
		}
*/
		cdDrives[index++] = 'C';
		
		if (index == 0) {
			Zdebug("InitResMan, cannot find CD drive.");
			ExitWithReport("InitResMan, cannot find CD drive. [file=%s line=%u]", __FILE__, __LINE__);
		}

		while (index < 24)
			cdDrives[index++] = 0;
	}
	else
		file.close();
}

char *resMan::GetCdPath(void) {
	return cdPath;
}

void resMan::Close_ResMan(void) { //Tony29May96
	// free up our mallocs
	free(resList);
	free(age);
	// free(status);
	free(count);
}

#ifdef SCUMM_BIG_ENDIAN
// Quick macro to make swapping in-place easier to write
#define SWAP16(x)	x = SWAP_BYTES_16(x)
#define SWAP32(x)	x = SWAP_BYTES_32(x)
static void convertEndian(uint8 *file, uint32 len) {
	int i;
	_standardHeader *hdr = (_standardHeader *)file;
	
	file += sizeof(_standardHeader);

	SWAP32(hdr->compSize);
	SWAP32(hdr->decompSize);

	switch(hdr->fileType) {
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
			_multiScreenHeader *mscreenHeader = (_multiScreenHeader *)file;

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
#endif

uint8 *resMan::Res_open(uint32 res) {	//BHTony30May96
	// returns ad of resource. Loads if not in memory
	// retains a count
	// resource can be aged out of memory if count = 0
	// the resource is locked while count != 0 i.e. until a res_close is
	// called

	File	file;
	uint16	parent_res_file;
	uint16	actual_res;
	uint32	pos, len;

	uint32	table_offset;

//#ifdef _SWORD2_DEBUG
	if (res >= total_res_files)
		Con_fatal_error("Res_open illegal resource %d (there are %d resources 0-%d)", res, total_res_files, total_res_files - 1);
//#endif

	// is the resource in memory already?
	// if the file is not in memory then age should and MUST be 0
	if (!age[res]) {
		// fetch the correct file and read in the correct portion
		// if the file cannot fit then we must trash the oldest large
		// enough floating file

		// points to the number of the ascii filename
		parent_res_file = res_conv_table[res * 2];

//#ifdef _SWORD2_DEBUG
		if (parent_res_file == 0xffff)
			Con_fatal_error("Res_open tried to open null & void resource number %d", res);
//#endif

		// relative resource within the file
		actual_res = res_conv_table[(res * 2) + 1];

		// first we have to find the file via the res_conv_table

		// Zdebug("resOpen %s res %d", resource_files[parent_res_file], res);

		// ** at this point here we start to think about where the
		// ** file is and prompt the user for the right CD to be
		// ** inserted
		// **
		// ** we need to know the position that we're at within the
		// ** game - LINC should write this someplace.

/* these probably aren't necessary - khalek
		if (!(cdTab[parent_res_file] & LOCAL_CACHE) && !(cdTab[parent_res_file] & LOCAL_PERM)) {
			// This cluster is on a CD, we need to cache a new one.
			CacheNewCluster(parent_res_file);
		} else if (!(cdTab[parent_res_file] & LOCAL_PERM)) {
			// Makes sure that the correct CD is in the drive.
			GetCd(cdTab[parent_res_file] & 3);
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
		// And there's some music / FX stuff in CacheNewCluster() that
		// might be needed as well.
		//
		// But this will do for now.

		if (!(cdTab[parent_res_file] & LOCAL_PERM)) {
			curCd = cdTab[parent_res_file] & 3;
		}

		// open the cluster file
		if (!file.open(resource_files[parent_res_file]))
			Con_fatal_error("Res_open cannot *OPEN* %s", resource_files[parent_res_file]);


		// 1st DWORD of a cluster is an offset to the look-up table
		table_offset = file.readUint32LE();

		// Zdebug("table offset = %d", table_offset);

		// 2 dwords per resource
		file.seek(table_offset + actual_res *8, SEEK_SET);
		// get position of our resource within the cluster file
		pos = file.readUint32LE();
		// read the length
		len = file.readUint32LE();

		// ** get to position in file of our particular resource
		file.seek(pos, SEEK_SET);

		// Zdebug("res len %d", len);

		// ok, we know the length so try and allocate the memory
		// if it can't then old files will be ditched until it works
		resList[res] = Twalloc(len, MEM_locked, res);

/* This probably isn't needed
		// Do a quick ServiceWindows to stop the music screwing up.
		ServiceWindows();
*/

		// now load the file
		// hurray, load it in.
		file.read(resList[res]->ad, len);

		//close the cluster
		file.close();

#ifdef SCUMM_BIG_ENDIAN
		convertEndian((uint8 *)resList[res]->ad, len);
#endif
	} else {
		// Zdebug("RO %d, already open count=%d", res, count[res]);
	}

	// number of times opened - the file won't move in memory while count
	// is non zero
	count[res]++;

	// update the accessed time stamp - touch the file in other words
	age[res] = resTime;

	// pass the address of the mem & lock the memory too
	// might be locked already (if count > 1)
	Lock_mem(resList[res]);

	return (uint8 *) resList[res]->ad;
}

uint8 resMan::Res_check_valid(uint32 res) {	// James 12mar97
	// returns '1' if resource is valid, otherwise returns '0'
	// used in startup.cpp to ignore invalid screen-manager resources

	uint16 parent_res_file;

	// resource number out of range
	if (res >= total_res_files)
		return 0;

	// points to the number of the ascii filename
	parent_res_file = res_conv_table[res * 2];

	// null & void resource
	if (parent_res_file == 0xffff)
		return 0;

	// ok
	return 1;
}

void resMan::Res_next_cycle(void) {	//Tony8Feb97
	// increment the cycle and calculate actual per-cycle memory useage

#ifdef _SWORD2_DEBUG
	uint32 j;
#endif

#ifdef _SWORD2_DEBUG
	current_memory_useage = 0;

	for (j = 1; j < total_res_files; j++) {
		// was accessed last cycle
		if (age[j] == resTime)
			current_memory_useage += resList[j]->size;
	}
#endif

	resTime++;

	// if you left the game running for a hundred years when this went to 0
	// there'd be a resource left stuck in memory - after another hundred
	// years there'd be another...
	//
	// Mind you, by then the our get_msecs() function will have wrapped
	// around too, probably causing a mess of other problems.

	if (!resTime)
		resTime++;
}

uint32 resMan::Res_fetch_useage(void) {		//Tony8Feb97
	// returns memory usage previous cycle
	return current_memory_useage;
}

void resMan::Res_close(uint32 res) {		//Tony30May96
	// decrements the count
	// resource floats when count = 0

//#ifdef _SWORD2_DEBUG
	if (res >= total_res_files)
		Con_fatal_error("Res_closeing illegal resource %d (there are %d resources 0-%d)", res, total_res_files, total_res_files - 1);

	//closing but isnt open?
	if (!(count[res]))
		Con_fatal_error("Res_close closing %d but it isn't open", res);
//#endif

	//one less has it open
	count[res]--;

	//if noone has the file open then unlock and allow to float
	if (!count[res]) {
		Float_mem(resList[res]);	// pass the address of the mem
		// *(status+res) -= RES_locked;	// unlock the resource
	}
}

uint32 resMan::Res_fetch_len( uint32 res ) {	//Tony27Jan96
	// returns the total file length of a resource - i.e. all headers are
	// included too

	File fh;
	uint16 parent_res_file;
	uint16 actual_res;
	uint32 len;
	uint32 table_offset;

	// points to the number of the ascii filename
	parent_res_file = res_conv_table[res * 2];

	// relative resource within the file
	actual_res = res_conv_table[(res * 2) + 1];

	// first we have to find the file via the res_conv_table
	// open the cluster file

	if (!fh.open(resource_files[parent_res_file]))
		Con_fatal_error("Res_fetch_len cannot *OPEN* %s", resource_files[parent_res_file]);

	// 1st DWORD of a cluster is an offset to the look-up table
	table_offset = fh.readUint32LE();

	// 2 dwords per resource + skip the position dword
	fh.seek(table_offset + (actual_res * 8) + 4, SEEK_SET);

	// read the length
	len = fh.readUint32LE();
	return len;
}

char *resMan::Fetch_cluster(uint32 res) {	//Tony3June96
	// returns a pointer to the ascii name of the cluster file which
	// contains resource res
	return resource_files[res_conv_table[res * 2]];
}

uint32 resMan::Fetch_age(uint32 res) {		//Tony3June96
	// return the age of res
	return age[res];
}

uint32 resMan::Fetch_count(uint32 res) {	//Tony3June96
	// return the open count of res
	return count[res];
}

uint32 resMan::Help_the_aged_out(void) {	//Tony10Oct96
	// remove from memory the oldest closed resource

	uint32 oldest_res;	//holds id of oldest found so far when we have to chuck stuff out of memory
	uint32 oldest_age;	//age of above during search
	uint32 j;
	uint32 largestResource = 0;

	oldest_age = resTime;
	oldest_res = 0;

	for (j = 2; j < total_res_files; j++) {
		// not held open and older than this one
		if (!count[j] && age[j] && age[j] <= oldest_age) {	
			if (age[j] == oldest_age && resList[j]->size > largestResource)	{
				// Kick old resource of oldest age and largest
				// size (Helps the poor defragger).
				oldest_res = j;
				largestResource = resList[j]->size;
			} else if (age[j] < oldest_age)	{
				oldest_res = j;
				oldest_age = age[j];
				largestResource = resList[j]->size;
			}
		}
	}

	// there was not a file we could release
	// no bytes released - oh dear, lets hope this never happens
	if (!oldest_res)
		return 0;

	// Zdebug(42,"removing %d, age %d, size %d", oldest_res, age[oldest_res], resList[oldest_res]->size);

	// trash this old resource

	age[oldest_res] = 0;		// effectively gone from resList
	Free_mem(resList[oldest_res]);	// release the memory too

	return resList[oldest_res]->size;	//return bytes freed
}

void resMan::Print_console_clusters(void) {	//Tony10Oct96
	uint32 j;

	if (total_clusters) {
		for (j = 0; j < total_clusters; j++)
			Print_to_console(" %s", resource_files[j]);
		Print_to_console(" %d resources", total_res_files);
	} else
		Print_to_console(" argh! No resources");

	Scroll_console();
}

void resMan::Examine_res(uint8 *input) {	//Tony23Oct96
	uint32 j = 0;
	uint32 res;
	_standardHeader	*file_header;

	do {
		if (input[j] >= '0' && input[j] <= '9')
			j++;
		else
			break;
	} while(input[j]);

	// didn't quit out of loop on a non numeric chr$
	if (!input[j]) {
		res = atoi((char*) input);

		if (!res)
			Print_to_console("illegal resource");
		else if (res >= total_res_files)
			Print_to_console("illegal resource %d (there are %d resources 0-%d)", res, total_res_files, total_res_files - 1);
		else if (res_conv_table[res * 2] == 0xffff)
			Print_to_console("%d is a null & void resource number", res);
		else {
			//open up the resource and take a look inside!
			file_header = (_standardHeader*) res_man.Res_open(res);

			// Print_to_console("%d", file_header->fileType);
			// Print_to_console("%s", file_header->name);

			//----------------------------------------------------
			// resource types: (taken from header.h)

			// 1:  ANIMATION_FILE
			//      all normal animations & sprites including
			//      mega-sets & font files which are the same
			//      format
			// 2:  SCREEN_FILE
			//      each contains background, palette, layer
			//      sprites, parallax layers & shading mask
			// 3:  GAME_OBJECT
			//      each contains object hub + structures + script
			//      data
			// 4:  WALK_GRID_FILE
			//      walk-grid data
			// 5:  GLOBAL_VAR_FILE
			//      all the global script variables in one file;
			//      "there can be only one"
			// 6:  PARALLAX_FILE_null
			//      NOT USED
			// 7:  RUN_LIST
			//      each contains a list of object resource id's
			// 8:  TEXT_FILE
			//      each contains all the lines of text for a
			//      location or a character's conversation script
			// 9:  SCREEN_MANAGER
			//      one for each location; this contains special
			//      startup scripts
			// 10: MOUSE_FILE
			//      mouse pointers and luggage icons (sprites in
			//      General \ Mouse pointers & Luggage icons)
			// 11: WAV_FILE
			//      NOT USED HERE
			// 12: ICON_FILE
			//      menu icon (sprites in General \ Menu icons)
			// 13: PALETTE_FILE
			//      NOT USED HERE
			//----------------------------------------------------

			switch(file_header->fileType) {
				case ANIMATION_FILE:
					Print_to_console(" <anim> %s", file_header->name);
					break;
				case SCREEN_FILE:
					Print_to_console(" <layer> %s", file_header->name);
					break;
				case GAME_OBJECT:
					Print_to_console(" <game object> %s", file_header->name);
					break;
				case WALK_GRID_FILE:
					Print_to_console(" <walk grid> %s", file_header->name);
					break;
				case GLOBAL_VAR_FILE:
					Print_to_console(" <global variables> %s", file_header->name);
					break;
				case PARALLAX_FILE_null:
					Print_to_console(" <parallax file NOT USED!> %s", file_header->name);
					break;
				case RUN_LIST:
					Print_to_console(" <run list> %s", file_header->name);
					break;
				case TEXT_FILE:
					Print_to_console(" <text file> %s", file_header->name);
					break;
				case SCREEN_MANAGER:
					Print_to_console(" <screen manager> %s", file_header->name);
					break;
				case MOUSE_FILE:
					Print_to_console(" <mouse pointer> %s", file_header->name);
					break;
				case ICON_FILE:
					Print_to_console(" <menu icon> %s", file_header->name);
					break;
				default:
					Print_to_console(" unrecognised fileType %d", file_header->fileType);
					break;
			}
			res_man.Res_close(res);
		}
	} else {
		Print_to_console("try typing a number");
	}
}

void resMan::Kill_res(uint8 *input) {	//Tony23Oct96
	int j = 0;
	uint32 res;

	do {
		if (input[j] >= '0' && input[j] <= '9')
			j++;
		else
			break;
	} while (input[j]);

	// didn't quit out of loop on a non numeric chr$
	if (!input[j]) {
		res = atoi((char*) input);

//#ifdef _SWORD2_DEBUG
		if (!res)
			Print_to_console("illegal resource");

		if (res >= total_res_files)
			Con_fatal_error(" llegal resource %d (there are %d resources 0-%d)", res, total_res_files, total_res_files - 1);
//#endif

		// if noone has the file open then unlock and allow to float
		if (!count[res]) {
			if (age[res]) {
				age[res] = 0;		//effectively gone from resList
				Free_mem(resList[res]);	//release the memory too
				Print_to_console(" trashed %d", res);
			} else
				Print_to_console("%d not in memory", res);
		} else
			Print_to_console(" file is open - cannot remove");
	} else {
		Print_to_console("try typing a number");
	}
}

void resMan::Remove_res(uint32 res) {	//Tony10Jan97
	if (age[res]) {
		age[res] = 0;		//effectively gone from resList
		Free_mem(resList[res]);	//release the memory too
		// Zdebug(" - Trashing %d", res);
	} else
		Zdebug("Remove_res(%d) not even in memory!",res);
}

void resMan::Remove_all_res(void) {	// James24mar97
	// remove all res files from memory - ready for a total restart
	// including player object & global variables resource

	int j;
	uint32 res;

	j = base_mem_block;

	do {
		if (mem_list[j].uid < 65536) {	// a resource
			res = mem_list[j].uid;
			age[res] = 0;		// effectively gone from resList
			Free_mem(resList[res]);	// release the memory too
		}

		j = mem_list[j].child;
	} while	(j != -1);
}

void resMan::Kill_all_res(uint8 wantInfo) {	//Tony29Nov96
	// remove all res files from memory
	// its quicker to search the mem blocs for res files than search
	// resource lists for those in memory

	int j;
	uint32 res;
	uint32 nuked = 0;
  	_standardHeader *header;
	int scrolls = 0;
	char c;

	j = base_mem_block;

	do {
		if (mem_list[j].uid < 65536) {	// a resource
			res = mem_list[j].uid;

			// not the global vars which are assumed to be open in
			// memory & not the player object! (James17jan97)
			if (res != 1 && res != CUR_PLAYER_ID) {
				header = (_standardHeader*) res_man.Res_open(res);
				res_man.Res_close(res);

				age[res] = 0;		// effectively gone from resList
				Free_mem(resList[res]);	// release the memory too
				nuked++;

				// if this was called from the console + we
				// want info
				if (wantInfo && console_status) {
					Print_to_console(" nuked %5d: %s", res, header->name);
					Zdebug(" nuked %d: %s", res, header->name);
					Build_display();

					scrolls++;
					if (scrolls == 18) {
						Temp_print_to_console("- Press ESC to stop or any other key to continue");
						Build_display();

						do {
							ServiceWindows();
						} while(!KeyWaiting());

						ReadKey(&c);	//kill the key we just pressed
						if (c == 27)	//ESC
							break;

						// clear the Press Esc message ready for the new line
						Clear_console_line();
						scrolls = 0;
					}
				}	
			}
		}
		j = mem_list[j].child;
	} while (j != -1);

	// if this was called from the console + we want info!
	if (wantInfo && console_status) {
		Scroll_console();
		Print_to_console(" expelled %d resource(s)", nuked);
	}
}

//----------------------------------------------------------------------------
// Like Kill_all_res but only kills objects (except George & the variable
// table of course) - ie. forcing them to reload & restart their scripts,
// which simulates the effect of a save & restore, thus checking that each
// object's re-entrant logic works correctly, and doesn't cause a statuette to
// disappear forever, or some plaster-filled holes in sand to crash the game &
// get James in trouble again.

void resMan::Kill_all_objects(uint8 wantInfo) {		// James17jan97
	// remove all object res files from memory, excluding George
	// its quicker to search the mem blocs for res files than search
	// resource lists for those in memory

	int j;
	uint32 res;
	uint32 nuked = 0;
 	_standardHeader *header;
	int scrolls = 0;
	char c;

	j = base_mem_block;

	do {
		if (mem_list[j].uid < 65536) {	//a resource
			res=mem_list[j].uid;
			//not the global vars which are assumed to be open in
			// memory & not the player object! (James17jan97)
			if (res != 1 && res != CUR_PLAYER_ID) {
				header = (_standardHeader*) res_man.Res_open(res);
				res_man.Res_close(res);

				if (header->fileType == GAME_OBJECT) {
					age[res] = 0;		// effectively gone from resList
					Free_mem(resList[res]);	// release the memory too
   					nuked++;

					// if this was called from the console + we want info
					if (wantInfo && console_status)	{
						Print_to_console(" nuked %5d: %s", res, header->name);
						Zdebug(" nuked %d: %s", res, header->name);
						Build_display();

						scrolls++;
						if (scrolls==18) {
							Print_to_console("- Press ESC to stop or any other key to continue");
							Build_display();

							do {
								ServiceWindows();
							} while(!KeyWaiting());


							ReadKey(&c);	//kill the key we just pressed
							if (c == 27)	// ESC
								break;

							// clear the Press Esc message ready for the new line
							Clear_console_line();
							scrolls = 0;
						}
					}	
				}
			}
		}
		j = mem_list[j].child;
	} while (j != -1);

	// if this was called from the console + we want info
	if (wantInfo && console_status)
		Print_to_console(" expelled %d object resource(s)", nuked);
}

void resMan::CacheNewCluster(uint32 newCluster) {
	// Stop any music from streaming off the CD before we start the
	// cluster-copy!
	//
	// eg. the looping restore-panel music will still be playing if we
	// restored a game to a different cluster on the same CD - and music
	// streaming would interfere with cluster copying, slowing it right
	// down - but if we restored to a different CD the music is stopped
	// in GetCd() when it asks for the CD

	FN_stop_music(NULL);	// (James16sep97)

	Clear_fx_queue();	// stops all fx & clears the queue (James22july97)
	GetCd(cdTab[newCluster] & 3);

	// Kick out old cached cluster and load the new one.
	uint32 i = 0;
	while (!(cdTab[i] & LOCAL_CACHE) && i < total_clusters)
		i++;

	if (i < total_clusters)	{
		SVM_SetFileAttributes(resource_files[i], FILE_ATTRIBUTE_NORMAL);
		SVM_DeleteFile(resource_files[i]);
		cdTab[i] &= (0xff - LOCAL_CACHE);
		FILE *file;
		file = fopen("cd.inf", "r+b");

		if (file == NULL) {
			Zdebug("CacheNewCluster cannot *OPEN* cd.inf");
			Con_fatal_error("InitResMan cannot *OPEN* cd.inf [file=%s line=%u]", __FILE__, __LINE__);
		}

		_cd_inf cdInf;
		
		do {
			fread(&cdInf, 1, sizeof(_cd_inf), file);
		} while ((scumm_stricmp((char *) cdInf.clusterName, resource_files[i]) != 0) && !feof(file));

		if (feof(file)) {
			Zdebug("CacheNewCluster cannot find %s in cd.inf", resource_files[i]);
			Con_fatal_error("CacheNewCluster cannot find %s in cd.inf", resource_files[i]);
		}

		fseek(file, -1, SEEK_CUR);
		fwrite(&cdTab[i], 1, 1, file);
		fclose(file);
	}

	char buf[1024];
	sprintf(buf, "%sClusters\\%s", cdPath, resource_files[newCluster]);

	WaitForFade();

	if (GetFadeStatus() != RDFADE_BLACK) {
		FadeDown((float) 0.75);
		WaitForFade();
	}

	EraseBackBuffer();

	Set_mouse(0);
	Set_luggage(0);	//tw28Aug

	uint8 *bgfile;
	bgfile = res_man.Res_open(2950);	// open the screen resource
	InitialiseBackgroundLayer(NULL);
	InitialiseBackgroundLayer(NULL);
	InitialiseBackgroundLayer(FetchBackgroundLayer(bgfile));
	InitialiseBackgroundLayer(NULL);
	InitialiseBackgroundLayer(NULL);
	BS2_SetPalette(0, 256, FetchPalette(bgfile), RDPAL_FADE);

	RenderParallax(FetchBackgroundLayer(bgfile), 2);
	res_man.Res_close(2950);		// release the screen resource

	// Git rid of read-only status, if it is set.
	SVM_SetFileAttributes(resource_files[newCluster], FILE_ATTRIBUTE_NORMAL);

	File inFile, outFile;
	
	inFile.open(buf);
	outFile.open(resource_files[newCluster], NULL, File::kFileWriteMode);

	if (!inFile.isOpen() || !outFile.isOpen()) {
		Zdebug("Cache new cluster could not copy %s to %s", buf, resource_files[newCluster]);
		Con_fatal_error("Cache new cluster could not copy %s to %s [file=%s line=%u]", buf, resource_files[newCluster], __FILE__, __LINE__);
	}

	_spriteInfo textSprite;
	_spriteInfo barSprite;
	mem *text_spr;
	_frameHeader *frame;
	uint8 *loadingBar;
	_cdtEntry *cdt;

	text_spr = MakeTextSprite( FetchTextLine(res_man.Res_open(2283), 8) + 2, 640, 187, speech_font_id);

	frame = (_frameHeader*) text_spr->ad;

	textSprite.x = screenWide /2 - frame->width / 2;
	textSprite.y = screenDeep /2 - frame->height / 2 - RDMENU_MENUDEEP;
	textSprite.w = frame->width;
	textSprite.h = frame->height;
	textSprite.scale = 0;
	textSprite.scaledWidth	= 0;
	textSprite.scaledHeight	= 0;
	textSprite.type = RDSPR_DISPLAYALIGN + RDSPR_NOCOMPRESSION + RDSPR_TRANS;
	textSprite.blend = 0;
	textSprite.colourTable	= 0;

	res_man.Res_close(2283);

	loadingBar = res_man.Res_open(2951);

	frame = FetchFrameHeader(loadingBar, 0);
	cdt   = FetchCdtEntry(loadingBar, 0);

	barSprite.x = cdt->x;
	barSprite.y = cdt->y;
	barSprite.w = frame->width;
	barSprite.h = frame->height;
	barSprite.scale = 0;
	barSprite.scaledWidth = 0;
	barSprite.scaledHeight = 0;
	barSprite.type = RDSPR_RLE256FAST + RDSPR_TRANS;
	barSprite.blend = 0;
	barSprite.colourTable = 0;

	res_man.Res_close(2951);

	loadingBar = res_man.Res_open(2951);
	frame = FetchFrameHeader(loadingBar, 0);
	barSprite.data = (uint8 *) (frame + 1);
	res_man.Res_close(2951);

	int16 barX = barSprite.x;
	int16 barY = barSprite.y;
	int16 textX = textSprite.x;
	int16 textY = textSprite.y;

	DrawSprite(&barSprite);
	barSprite.x = barX;
	barSprite.y = barY;

	textSprite.data	= text_spr->ad + sizeof(_frameHeader);
	DrawSprite(&textSprite);
	textSprite.x = textX;
	textSprite.y = textY;

	CopyScreenBuffer();
	FadeUp((float) 0.75);

	WaitForFade();

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
			Zdebug("Cache new cluster could not copy %s to %s", buf, resource_files[newCluster]);
			Con_fatal_error("Cache new cluster could not copy %s to %s [file=%s line=%u]", buf, resource_files[newCluster], __FILE__, __LINE__);
		}

		if (step == stepSize) {
			step = 0;
			// open the screen resource
			bgfile = res_man.Res_open(2950);
			RenderParallax(FetchBackgroundLayer(bgfile), 2);
			// release the screen resource
			res_man.Res_close(2950);
			loadingBar = res_man.Res_open(2951);
			frame = FetchFrameHeader(loadingBar, fr);
			barSprite.data = (uint8 *) (frame + 1);
			res_man.Res_close(2951);
			DrawSprite(&barSprite);
			barSprite.x = barX;
			barSprite.y = barY;

			textSprite.data	= text_spr->ad + sizeof(_frameHeader);
			DrawSprite(&textSprite);
			textSprite.x = textX;
			textSprite.y = textY;

			CopyScreenBuffer();
			fr++;
		} else
			step++;


		//--------------------------------------------------
		// Service windows

		ServiceWindows();
 		//--------------------------------------------------
	} while ((read % BUFFERSIZE) == 0);

	if (read != size) {
		Zdebug("Cache new cluster could not copy %s to %s", buf, resource_files[newCluster]);
		Con_fatal_error("Cache new cluster could not copy %s to %s [file=%s line=%u]", buf, resource_files[newCluster], __FILE__, __LINE__);
	}

	inFile.close();
	outFile.close();
	Free_mem(text_spr);

	EraseBackBuffer();

	FadeDown((float) 0.75);
	WaitForFade();
	CopyScreenBuffer();
	FadeUp((float) 0.75);

	// Git rid of read-only status.
	SVM_SetFileAttributes(resource_files[newCluster], FILE_ATTRIBUTE_NORMAL);

	// Update cd.inf and cdTab
	cdTab[newCluster] |= LOCAL_CACHE;

	FILE *file;
	file = fopen("cd.inf", "r+b");

	if (file == NULL) {
		Zdebug("CacheNewCluster cannot *OPEN* cd.inf");
		Con_fatal_error("InitResMan cannot *OPEN* cd.inf [file=%s line=%u]", __FILE__, __LINE__);
	}

	_cd_inf cdInf;
	
	do {
		fread(&cdInf, 1, sizeof(_cd_inf), file);
	} while (scumm_stricmp((char *) cdInf.clusterName, resource_files[newCluster]) != 0 && !feof(file));

	if (feof(file)) {
		Zdebug("CacheNewCluster cannot find %s in cd.inf", resource_files[newCluster]);
		Con_fatal_error("CacheNewCluster cannot find %s in cd.inf", resource_files[newCluster]);
	}

	fseek(file, -1, SEEK_CUR);
	fwrite(&cdTab[newCluster], 1, 1, file);
	fclose(file);
}

void resMan::GetCd(int cd) {
	// TODO support a seperate path for cd data?
	
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
	if (g_sword2->_gameId == GID_SWORD2_DEMO)
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
		while(cdDrives[index] != 0 && index < 24) {
			sprintf(cdPath, "%c:\\", cdDrives[index]);

			if (!SVM_GetVolumeInformation(cdPath, sCDName, _MAX_PATH, NULL, &dwMaxCompLength, &dwFSFlags, NULL, 0))	{
				// Force the following code to ask for the correct CD.
				sCDName[0] = 0;
			}

			curCd = cd;
		
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
		Zdebug("RUNNING OFF NETWORK");

		fscanf(file, "%s", cdPath);
		fclose(file);

		if (curCd == cd)
			return;
		else
			curCd = cd;

		// don't show CD-requests if testing anims or text/speech
		if (SYSTEM_TESTING_ANIMS || SYSTEM_TESTING_TEXT)
			return;

		offNetwork = 1;
	}

	// stop any music from playing - so the system no longer needs the
	// current CD - otherwise when we take out the CD, Windows will
	// complain!

	FN_stop_music(NULL);	// (James29aug97)

	textRes = res_man.Res_open(2283);
	DisplayMsg(FetchTextLine(textRes, 5 + cd) + 2, 0);
	text_spr = MakeTextSprite(FetchTextLine(textRes, 5 + cd) + 2, 640, 187, speech_font_id);

	frame = (_frameHeader*) text_spr->ad;

	spriteInfo.x = screenWide / 2 - frame->width / 2;
	spriteInfo.y = screenDeep / 2 - frame->height / 2 - RDMENU_MENUDEEP;
	spriteInfo.w = frame->width;
	spriteInfo.h = frame->height;
	spriteInfo.scale = 0;
	spriteInfo.scaledWidth	= 0;
	spriteInfo.scaledHeight	= 0;
	spriteInfo.type = RDSPR_DISPLAYALIGN + RDSPR_NOCOMPRESSION + RDSPR_TRANS;
	spriteInfo.blend = 0;
	spriteInfo.data = text_spr->ad + sizeof(_frameHeader);
	spriteInfo.colourTable	= 0;
	oldY = spriteInfo.y;
	oldX = spriteInfo.x;

	res_man.Res_close(2283);

	do {
		if (offNetwork == 1)
			done = TRUE;
		else {
			index = 0;
			while (cdDrives[index] != 0 && !done && index < 24) {
				sprintf(cdPath, "%c:\\", cdDrives[index]);

				if (!SVM_GetVolumeInformation(cdPath, sCDName, _MAX_PATH, NULL, &dwMaxCompLength, &dwFSFlags, NULL, 0))	{
					sCDName[0] = 0;
				}

				if (!scumm_stricmp(sCDName,CD1_LABEL)) {
					if (cd == CD1)
						done = TRUE;
				} else if (!scumm_stricmp(sCDName,CD2_LABEL)) {
					if (cd == CD2)
						done = TRUE;
				}
				index++;
			}
		}
		
		//--------------------------------------------------
		// Service windows

		ServiceWindows();

		//--------------------------------------------------
	
		EraseBackBuffer();
		DrawSprite(&spriteInfo);	// Keep the message there even when the user task swaps.
		spriteInfo.y = oldY;		// Drivers change the y co-ordinate, don't know why...
		spriteInfo.x = oldX;
		CopyScreenBuffer();
	} while (!done);

	Free_mem(text_spr);
	RemoveMsg();
}
