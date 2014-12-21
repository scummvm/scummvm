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

#include "lab/stddefines.h"
#include "lab/parsetypes.h"

namespace Lab {

/* Have to make sure that ROOMBUFFERSIZE is bigger than the biggest piece of memory
   that we need */
#define ROOMBUFFERSIZE (2 * 20480L)
#define EMPTYROOM      ((uint16) -1)
#define MAXMARKERS     10

extern RoomData *Rooms;
extern uint16 ManyRooms;


typedef struct {
	uint16 RoomNum;
	void *Start0, *End0, *Start1, *End1;
} RoomMarker;



static RoomMarker RoomMarkers[MAXMARKERS];
static void *RoomBuffer = NULL;
static uint16 CurMarker  = 0;
static void *MemPlace = NULL, *NextMemPlace = NULL;
static int32 MemLeftInBuffer = 0L;

/*****************************************************************************/
/* Allocates the memory for the room buffers.                                */
/*****************************************************************************/
bool initRoomBuffer(void) {
	uint16 counter;

	CurMarker = 0;

	if ((RoomBuffer = calloc(ROOMBUFFERSIZE, 1))) {
		MemPlace = RoomBuffer;
		MemLeftInBuffer = ROOMBUFFERSIZE;

		for (counter = 0; counter < MAXMARKERS; counter++)
			RoomMarkers[counter].RoomNum = EMPTYROOM;

		return true;
	} else
		return false;
}




/*****************************************************************************/
/* Frees the memory for the room buffers.                                    */
/*****************************************************************************/
void freeRoomBuffer(void) {
	if (RoomBuffer)
		free(RoomBuffer);
}


/*****************************************************************************/
/* Frees a room's resources.                                                 */
/*****************************************************************************/
static void freeRoom(uint16 RMarker) {
	uint16 RoomNum;

	RoomNum = RoomMarkers[RMarker].RoomNum;

	if (RoomNum != EMPTYROOM) {
		Rooms[RoomNum].NorthView = NULL;
		Rooms[RoomNum].SouthView = NULL;
		Rooms[RoomNum].EastView  = NULL;
		Rooms[RoomNum].WestView  = NULL;
		Rooms[RoomNum].RuleList  = NULL;
		Rooms[RoomNum].RoomMsg   = NULL;
	}

	RoomMarkers[RMarker].RoomNum = EMPTYROOM;
	RoomMarkers[RMarker].Start0  = NULL;
	RoomMarkers[RMarker].End0    = NULL;
	RoomMarkers[RMarker].Start1  = NULL;
	RoomMarkers[RMarker].End1    = NULL;
}

/*****************************************************************************/
/* Gets a chunk of memory from the buffer.                                   */
/*****************************************************************************/
static void *getCurMem(uint16 Size) {
	uint16 counter;
	void *Ptr, *Start0, *Start1, *End0, *End1;

	if (((int32) Size) > MemLeftInBuffer) {
		MemPlace = RoomBuffer;
		MemLeftInBuffer = ROOMBUFFERSIZE;
		NextMemPlace = NULL;
	}

	Ptr = MemPlace;
	MemPlace = (char *)MemPlace + Size;
	MemLeftInBuffer -= Size;

	if (MemPlace > NextMemPlace) {
		NextMemPlace = NULL;

		for (counter = 0; counter < MAXMARKERS; counter++) {
			if (RoomMarkers[counter].RoomNum != EMPTYROOM) {
				Start0 = RoomMarkers[counter].Start0;
				Start1 = RoomMarkers[counter].Start1;
				End0   = RoomMarkers[counter].End0;
				End1   = RoomMarkers[counter].End1;

				if (((Start0 >= Ptr) && (Start0 < MemPlace))  ||
				        ((End0 >= Ptr) && (End0 < MemPlace))    ||
				        ((Ptr >= Start0) && (Ptr <= End0))        ||

				        ((Start1 >= Ptr) && (Start1 < MemPlace))  ||
				        ((End1 >= Ptr) && (End1 < MemPlace))    ||
				        ((Ptr >= Start1) && (Ptr <= End1))) {
					freeRoom(counter);
				} else {
					if (Start0 >= MemPlace)
						if ((NextMemPlace == NULL) || (Start0 < NextMemPlace))
							NextMemPlace = Start0;

					if (Start1 >= MemPlace)
						if ((NextMemPlace == NULL) || (Start1 < NextMemPlace))
							NextMemPlace = Start1;
				}
			}
		}

		if (NextMemPlace == NULL) {
			NextMemPlace = RoomBuffer;
			NextMemPlace = (char *)NextMemPlace + ROOMBUFFERSIZE;
		}
	}

	return Ptr;
}





/*****************************************************************************/
/* Grabs a chunk of memory from the room buffer, and manages it for a        */
/* particular room.                                                          */
/*****************************************************************************/
void allocRoom(void **Ptr, uint16 Size, uint16 RoomNum) {
	uint16 RMarker;
	bool doit = true;

	if (1 & Size)  /* Memory is required to be even aligned */
		Size++;

	RMarker = 0;

	while ((RMarker < MAXMARKERS) && doit) {
		if (RoomMarkers[RMarker].RoomNum == RoomNum)
			doit = false;
		else
			RMarker++;
	}

	if (RMarker >= MAXMARKERS) {
		RMarker = CurMarker;
		CurMarker++;

		if (CurMarker >= MAXMARKERS)
			CurMarker = 0;

		freeRoom(RMarker);
		RoomMarkers[RMarker].RoomNum = RoomNum;
	}

	*Ptr = getCurMem(Size);

	if (RoomMarkers[RMarker].Start0 == NULL) {
		RoomMarkers[RMarker].Start0 = *Ptr;
		RoomMarkers[RMarker].End0   = (void *)(((char *)(*Ptr)) + Size - 1);
	} else if (*Ptr < RoomMarkers[RMarker].Start0) {
		if (RoomMarkers[RMarker].Start1 == NULL)
			RoomMarkers[RMarker].Start1 = *Ptr;

		RoomMarkers[RMarker].End1 = (void *)(((char *)(*Ptr)) + Size - 1);
	} else
		RoomMarkers[RMarker].End0 = (void *)(((char *)(*Ptr)) + Size - 1);
}

} // End of namespace Lab
