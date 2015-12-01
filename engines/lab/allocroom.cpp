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

/*****************************************************************************/
/* Frees a room's resources.                                                 */
/*****************************************************************************/
static void freeRoom(uint16 roomMarkerId) {
	uint16 roomNum = _roomMarkers[roomMarkerId]._roomNum;

	if (roomNum != EMPTYROOM) {
		_rooms[roomNum]._northView = nullptr;
		_rooms[roomNum]._southView = nullptr;
		_rooms[roomNum]._eastView = nullptr;
		_rooms[roomNum]._westView = nullptr;

		RuleList *rules = _rooms[roomNum]._rules;
		for (RuleList::iterator rule = rules->begin(); rule != rules->end(); ++rule)
			delete *rule;
		_rooms[roomNum]._rules->clear();
		delete _rooms[roomNum]._rules;
		_rooms[roomNum]._rules = nullptr;
		_rooms[roomNum]._roomMsg = nullptr;
	}

	_roomMarkers[roomMarkerId]._roomNum = EMPTYROOM;
	_roomMarkers[roomMarkerId]._start0  = nullptr;
	_roomMarkers[roomMarkerId]._end0    = nullptr;
	_roomMarkers[roomMarkerId]._start1  = nullptr;
	_roomMarkers[roomMarkerId]._end1    = nullptr;
}

/*****************************************************************************/
/* Gets a chunk of memory from the buffer.                                   */
/*****************************************************************************/
static void *getCurMem(uint16 Size) {
	if (((int32) Size) > MemLeftInBuffer) {
		MemPlace = RoomBuffer;
		MemLeftInBuffer = ROOMBUFFERSIZE;
		NextMemPlace = NULL;
	}

	void *Ptr = MemPlace;
	MemPlace = (char *)MemPlace + Size;
	MemLeftInBuffer -= Size;

	if (MemPlace > NextMemPlace) {
		NextMemPlace = NULL;

		for (uint16 i = 0; i < MAXMARKERS; i++) {
			if (_roomMarkers[i]._roomNum != EMPTYROOM) {
				void *Start0 = _roomMarkers[i]._start0;
				void *Start1 = _roomMarkers[i]._start1;
				void *End0   = _roomMarkers[i]._end0;
				void *End1   = _roomMarkers[i]._end1;

				if (((Start0 >= Ptr) && (Start0 < MemPlace))  ||
				        ((End0 >= Ptr) && (End0 < MemPlace))    ||
				        ((Ptr >= Start0) && (Ptr <= End0))        ||

				        ((Start1 >= Ptr) && (Start1 < MemPlace))  ||
				        ((End1 >= Ptr) && (End1 < MemPlace))    ||
				        ((Ptr >= Start1) && (Ptr <= End1))) {
					freeRoom(i);
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
void allocRoom(void **Ptr, uint16 size, uint16 roomNum) {
	if (1 & size)  /* Memory is required to be even aligned */
		size++;

	uint16 roomMarkerId = 0;

	while ((roomMarkerId < MAXMARKERS)) {
		if (_roomMarkers[roomMarkerId]._roomNum == roomNum)
			break;
		else
			roomMarkerId++;
	}

	if (roomMarkerId >= MAXMARKERS) {
		roomMarkerId = CurMarker;
		CurMarker++;

		if (CurMarker >= MAXMARKERS)
			CurMarker = 0;

		freeRoom(roomMarkerId);
		_roomMarkers[roomMarkerId]._roomNum = roomNum;
	}

	*Ptr = getCurMem(size);

	if (!_roomMarkers[roomMarkerId]._start0) {
		_roomMarkers[roomMarkerId]._start0 = *Ptr;
		_roomMarkers[roomMarkerId]._end0   = (void *)(((char *)(*Ptr)) + size - 1);
	} else if (*Ptr < _roomMarkers[roomMarkerId]._start0) {
		if (_roomMarkers[roomMarkerId]._start1 == nullptr)
			_roomMarkers[roomMarkerId]._start1 = *Ptr;

		_roomMarkers[roomMarkerId]._end1 = (void *)(((char *)(*Ptr)) + size - 1);
	} else
		_roomMarkers[roomMarkerId]._end0 = (void *)(((char *)(*Ptr)) + size - 1);
}

} // End of namespace Lab
