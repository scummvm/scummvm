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

#ifndef QUEENLOGIC_H
#define QUEENLOGIC_H

#include "queen/queen.h"

class QueenLogic {

public:
	QueenLogic(QueenResource *resource);
	~QueenLogic();

	uint16 currentRoom();
	void currentRoom(uint16 room);

	void oldRoom(uint16 room);

	int16 *objectData(int index);
	uint16 roomData(int room);
	uint16 objMax(int room);

	int16 *area(int index, int subIndex);
	uint16 walkOffCount();
	uint16 *walkOffData(int index);


protected:
	uint8 *_jas;
	uint16 _numRooms;
	uint16 _currentRoom;
	uint16 _oldRoom;	
	uint16 _newRoom;	
	uint16 _numNames;
	uint16 _numObjects;
	uint16 _numDescriptions;
	uint16 _numItems;
	uint16 _numGraphics;

	uint16 _numObjectBoxes;
	uint16 _numWalkOffs;

	uint16 *_roomData;
	uint16 *_sfxName;
	uint16 *_objMax;
	uint16 *_areaMax;
	uint16 (*_objectBox)[4];
	uint16 (*_itemData)[5];
	uint16 (*_graphicData)[5];
	int16 (*_objectData)[8];
	uint16 (*_actorData)[12];

	int16 (*_area)[11][8];

	uint16 (*_walkOffData)[3];

	QueenResource *_resource;

	void initialise();
};

#endif
