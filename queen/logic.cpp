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

#include "queen/logic.h"

QueenLogic::QueenLogic(QueenResource *resource) {
	_resource = resource;  
	_jas = _resource->loadJAS();
	
	initialise();
}

QueenLogic::~QueenLogic() {
	free (_jas);
	//free(_graphicData);
}

void QueenLogic::initialise() {
	uint8 *ptr = _jas;

	//_display->loadFont();
	
	_numRooms = READ_BE_UINT16(ptr);
	ptr += 2;
	_numNames = READ_BE_UINT16(ptr);
	ptr += 2;
	_numObjects = READ_BE_UINT16(ptr);
	ptr += 2;
	_numDescriptions = READ_BE_UINT16(ptr);
	ptr += 2;
	

	//Object data
	_objectData = (uint16 (*)[8])malloc((_numObjects + 1) * sizeof(_objectData[0]));
	//clear first object
	for (uint16 j = 0; j < 8; j++)
		_objectData[0][j] = 0;
	
	for (uint16 i = 1; i < (_numObjects + 1); i++)
		for (uint16 j = 0; j < 8; j++) {
			_objectData[i][j] = READ_BE_UINT16(ptr);
			ptr += 2;
		}
	
	//Room data
	_roomData = (uint16 *)malloc((_numRooms + 2) * sizeof(_roomData[0]));
	for (uint16 i = 1; i < (_numRooms + 2); i++) {
		_roomData[i] = READ_BE_UINT16(ptr);
		ptr += 2;
	}

	_roomData[_numRooms + 1] = _numObjects;

	//SFX Name
	_sfxName = (uint16 *)malloc((_numRooms + 1) * sizeof(_sfxName[0]));
	for (uint16 i = 0; i < (_numRooms + 1); i++) {
		_sfxName[i] = READ_BE_UINT16(ptr);
		ptr += 2;
	}	

	//Item information
	_numItems = READ_BE_UINT16(ptr);
	ptr += 2;

	_itemData = (uint16 (*)[5])malloc((_numItems + 1) * sizeof(_itemData[0]));
	for (uint16 i = 1; i < (_numItems + 1); i++) {
		_itemData[i][0] = READ_BE_UINT16(ptr);
		ptr += 2;
		for (uint16 j = 1; j < 5; j++) {
		       _itemData[i][j] = READ_BE_UINT16(ptr);
		       ptr += 2;
		}
	}
		
	//Graphic Image Data

	_numGraphics = READ_BE_UINT16(ptr);
	ptr += 2;

	_graphicData = (uint16 (*)[5])malloc((_numGraphics + 1) * sizeof(_graphicData[0]));

	for (uint16 i = 1; i < _numGraphics; i++)
		for (uint16 j = 0; j < 5; j++) {
			_graphicData[i][j] = READ_BE_UINT16(ptr);
			ptr += 2;
		}
	
	_objMax = (uint16 *)malloc((_numRooms + 1) * sizeof(_objMax[0]));
	_areaMax = (uint16 *)malloc((_numRooms + 1) * sizeof(_areaMax[0]));
	_area = (uint16 (*)[11][8])malloc((_numRooms + 1) * sizeof(_area[0]));
/*
	for (uint16 i = 1; i < (_numRooms + 1); i++) {
		_objMax[i] = READ_BE_UINT16(ptr);
		ptr += 2;
		_areaMax[i] = READ_BE_UINT16(ptr);
		ptr += 2;
		
		for (uint16 j = 1; j < (_areaMax[i] + 1); j++)
			for (uint16 k = 0; k < 8; k++) {
				_area[i][j][k] = READ_BE_UINT16(ptr);
				ptr += 2;
			}
				
	}	
	
	_objectBox = (uint16 (*)[4])malloc((_numObjects + 1) * sizeof(_objectBox[0]));
	for (uint16 i = 1; i < (_numObjects + 1); i++)
		for (uint16 j = 0; j < 4; j++) {
			_objectBox[i][j] = READ_BE_UINT16(ptr);
			ptr += 2;
		}
		

*/	

}

uint16 QueenLogic::currentRoom() {
	return _currentRoom;
}

