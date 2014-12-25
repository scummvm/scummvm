/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 */

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "lab/lab.h"
#include "lab/stddefines.h"
#include "lab/labfun.h"
#include "lab/modernsavegame.h"

namespace Lab {

/* The version string */
#if defined(DOSCODE)
#define SAVEVERSION         "LBS2"
#else
#define SAVEVERSION         "LBS3"
#define SAVEVERSION_COMPAT  "LBS2"
#endif

#define BOOKMARK  0
#define CARDMARK  1
#define FLOPPY    2

typedef void *LABFH;
#define INVALID_LABFH   NULL

uint16 FileType, FileNum;



/*----- The Amiga specific area of saveGame.c -----*/


/*****************************************************************************/
/* Opens a file to write to from disk.                                       */
/*****************************************************************************/
static LABFH saveGameOpen(char *filename, bool iswrite) {
	warning("STUB: saveGameOpen");
	return 0;

#if 0
	if (iswrite) {
		unlink(filename);
		return fopen(filename, "wb");
	} else
		return fopen(filename, "rb");
#endif
}




/*****************************************************************************/
/* Closes a file.                                                            */
/*****************************************************************************/
static void saveGameClose(LABFH file, bool iswrite) {
	warning("STUB: saveGameClose");
	return;

#if 0
	if (file != INVALID_LABFH)
		fclose(file);
#endif
}




/*****************************************************************************/
/* Writes a block of memory to whatever it is that we're writing to.         */
/*****************************************************************************/
static void saveGameWriteBlock(LABFH file, void *data, uint32 size) {
	warning("STUB: saveGameWriteBlock");
	return;

	//fwrite(data, 1, size, file);
}



/*****************************************************************************/
/* Writes a block of memory to whatever it is that we're writing to.         */
/*****************************************************************************/
static void saveGameReadBlock(LABFH file, void *data, uint32 size) {
	warning("STUB: saveGameReadBlock");
	return;

	//fread(data, 1, size, file);
}




/*----- The machine independent section of saveGame.c -----*/


/* Lab: Labyrinth specific */
extern uint16 combination[6];
extern uint16 CurTile[4] [4];

#if !defined(DOSCODE)
extern CrumbData BreadCrumbs[MAX_CRUMBS];
extern uint16 NumCrumbs;
extern bool DroppingCrumbs;
extern bool FollowingCrumbs;
#endif

/*****************************************************************************/
/* Writes the game out to disk.                                              */
/* Assumes that the file has already been openned and is there.              */
/*****************************************************************************/
static bool saveGame(uint16 RoomNum, uint16 Direction, uint16 Quarters, LABFH file) {
#if !defined(DOSCODE)
	uint16 temp;
	CrumbData crumbs[sizeof(BreadCrumbs) / sizeof(CrumbData)];
#endif
	uint16 last, counter, counter1;
	char c;

	saveGameWriteBlock(file, (void *)SAVEVERSION, 4L);
#if defined(DOSCODE)
	saveGameWriteBlock(file, &RoomNum, 2L);
	saveGameWriteBlock(file, &Direction, 2L);
	saveGameWriteBlock(file, &Quarters, 2L);
#else
	temp = swapUShort(RoomNum);
	saveGameWriteBlock(file, &temp, 2L);
	temp = swapUShort(Direction);
	saveGameWriteBlock(file, &temp, 2L);
	temp = swapUShort(Quarters);
	saveGameWriteBlock(file, &temp, 2L);
#endif

	last = g_lab->_conditions->_lastElement / 8;
	saveGameWriteBlock(file, g_lab->_conditions->_array, (uint32) last);

	last = g_lab->_roomsFound->_lastElement / 8;
	saveGameWriteBlock(file, g_lab->_roomsFound->_array, (uint32) last);

	/* LAB: the combination lock and tile stuff */
	for (counter = 0; counter < 6; counter++) {
		c = (char)combination[counter];
		saveGameWriteBlock(file, &c, 1L);
	}

	for (counter = 0; counter < 4; counter++)
		for (counter1 = 0; counter1 < 4; counter1++)
#if defined(DOSCODE)
			saveGameWriteBlock(file, &(CurTile[counter] [counter1]), 2L);

#else
		{
			temp = swapUShort(CurTile[counter] [counter1]);
			saveGameWriteBlock(file, &temp, 2L);
		}
#endif

#if !defined(DOSCODE)
	saveGameWriteBlock(file, g_SaveGameImage, SAVED_IMAGE_SIZE);
	memcpy(crumbs, BreadCrumbs, sizeof BreadCrumbs);
	swapUShortPtr(&crumbs[0].RoomNum, sizeof(BreadCrumbs) / sizeof(uint16));
	saveGameWriteBlock(file, crumbs, sizeof BreadCrumbs);
#endif

	saveGameClose(file, true);

	return true;
}



/*****************************************************************************/
/* Reads the game from disk.                                                 */
/* Assumes that the file has already been openned and is there.              */
/*****************************************************************************/
static bool loadGame(uint16 *RoomNum, uint16 *Direction, uint16 *Quarters, LABFH file) {
#if !defined(DOSCODE)
	uint16 t;
	CrumbData crumbs[sizeof(BreadCrumbs) / sizeof(CrumbData)];
#endif
	char temp[5], c;
	uint16 last, counter, counter1;

	saveGameReadBlock(file, temp, 4L);
	temp[4] = 0;

	/*
	   if (strcmp(temp, SAVEVERSION) != 0)
	   {
	    saveGameClose(file, false);
	    return false;
	   }
	 */

#if defined(DOSCODE)
	saveGameReadBlock(file, RoomNum, 2L);
	saveGameReadBlock(file, Direction, 2L);
	saveGameReadBlock(file, Quarters, 2L);
#else
	saveGameReadBlock(file, &t, 2L);
	*RoomNum = swapUShort(t);
	saveGameReadBlock(file, &t, 2L);
	*Direction = swapUShort(t);
	saveGameReadBlock(file, &t, 2L);
	*Quarters = swapUShort(t);
#endif

	last = g_lab->_conditions->_lastElement / 8;
	saveGameReadBlock(file, g_lab->_conditions->_array, (uint32) last);

	last = g_lab->_roomsFound->_lastElement / 8;
	saveGameReadBlock(file, g_lab->_roomsFound->_array, (uint32) last);

	/* LAB: the combination lock and tile stuff */
	for (counter = 0; counter < 6; counter++) {
		saveGameReadBlock(file, &c, 1L);
		combination[counter] = c;
	}

	for (counter = 0; counter < 4; counter++)
		for (counter1 = 0; counter1 < 4; counter1++)
#if defined(DOSCODE)
			saveGameReadBlock(file, &(CurTile[counter] [counter1]), 2L);

#else
		{
			saveGameReadBlock(file, &t, 2L);
			CurTile[counter] [counter1] = swapUShort(t);
		}
#endif

	if (strcmp(temp, SAVEVERSION) == 0) {
		saveGameReadBlock(file, g_SaveGameImage, SAVED_IMAGE_SIZE);

		memset(crumbs, 0, sizeof BreadCrumbs);
		saveGameReadBlock(file, crumbs, sizeof BreadCrumbs);
		swapUShortPtr(&crumbs[0].RoomNum, sizeof(BreadCrumbs) / sizeof(uint16));
		memcpy(BreadCrumbs, crumbs, sizeof BreadCrumbs);
		DroppingCrumbs = (BreadCrumbs[0].RoomNum != 0);
		FollowingCrumbs = false;

		for (counter = 0; counter < MAX_CRUMBS; counter++)
			if (BreadCrumbs[counter].RoomNum == 0) break;

		NumCrumbs = counter;
	}

	saveGameClose(file, false);

	return true;
}




/*****************************************************************************/
/* Saves the game to the floppy disk.                                        */
/*****************************************************************************/
bool saveFloppy(char *path, uint16 RoomNum, uint16 Direction, uint16 NumQuarters, uint16 filenum, uint16 type) {
	LABFH FPtr;

	g_music->checkMusic();

	FileType = type;
	FileNum  = filenum;

	if ((FPtr = saveGameOpen(path, true)) != INVALID_LABFH)
		saveGame(RoomNum, Direction, NumQuarters, FPtr);
	else
		return false;

	return true;
}




/*****************************************************************************/
/* Reads the game from the floppy disk.                                      */
/*****************************************************************************/
bool readFloppy(char *path, uint16 *RoomNum, uint16 *Direction, uint16 *NumQuarters, uint16 filenum, uint16 type) {
	LABFH FPtr;

	g_music->checkMusic();

	FileType = type;
	FileNum  = filenum;

	if ((FPtr = saveGameOpen(path, false)) != INVALID_LABFH) {
		if (!loadGame(RoomNum, Direction, NumQuarters, FPtr))
			return false;
	} else
		return false;

	return true;
}

} // End of namespace Lab
