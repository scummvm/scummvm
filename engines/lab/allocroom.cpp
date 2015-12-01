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

#include "lab/parsetypes.h"

namespace Lab {

/* Have to make sure that ROOMBUFFERSIZE is bigger than the biggest piece of memory
   that we need */
#define ROOMBUFFERSIZE (2 * 20480L)
#define EMPTYROOM      ((uint16) -1)
#define MAXMARKERS     10

extern RoomData *_rooms;

typedef struct {
	uint16 _roomNum;
	void *_start0, *_end0, *_start1, *_end1;
} RoomMarker;

static RoomMarker _roomMarkers[MAXMARKERS];
static void *RoomBuffer = NULL;
static uint16 CurMarker  = 0;
static void *MemPlace = NULL, *NextMemPlace = NULL;
static int32 MemLeftInBuffer = 0L;

/*****************************************************************************/
/* Allocates the memory for the room buffers.                                */
/*****************************************************************************/
bool initRoomBuffer() {
	CurMarker = 0;

	if ((RoomBuffer = calloc(ROOMBUFFERSIZE, 1))) {
		MemPlace = RoomBuffer;
		MemLeftInBuffer = ROOMBUFFERSIZE;

		for (uint16 i = 0; i < MAXMARKERS; i++)
			_roomMarkers[i]._roomNum = EMPTYROOM;

		return true;
	} else
		return false;
}

/*****************************************************************************/
/* Frees the memory for the room buffers.                                    */
/*****************************************************************************/
void freeRoomBuffer() {
	if (RoomBuffer)
		free(RoomBuffer);
}

} // End of namespace Lab
