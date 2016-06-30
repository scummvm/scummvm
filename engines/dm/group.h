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
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/


#ifndef DM_GROUP_H
#define DM_GROUP_H

#include "dm.h"

namespace DM {

class ActiveGroup {
public:
	int _groupThingIndex;
	byte _directions;
	byte _cells;
	byte _lastMoveTime;
	byte _delayFleeingFromTarget;
	byte _targetMapX;
	byte _targetMapY;
	byte _priorMapX;
	byte _priorMapY;
	byte _homeMapX;
	byte _homeMapY;
	byte _aspect[4];
}; // @ ACTIVE_GROUP


class Group {
public:
	Thing _nextThing;
	Thing _slot;
	byte _type;
	byte _cells;
	uint16 _health[4];
private:
	uint16 _flags;
public:
	explicit Group(uint16 *rawDat) : _nextThing(rawDat[0]), _slot(rawDat[1]), _type(rawDat[2]),
		_cells(rawDat[3]), _flags(rawDat[8]) {
		_health[0] = rawDat[4];
		_health[1] = rawDat[5];
		_health[2] = rawDat[6];
		_health[3] = rawDat[7];
	}

	byte &getActiveGroupIndex() { return _cells; }

	uint16 getBehaviour() { return _flags & 0xF; }
	uint16 getCount() { return (_flags >> 5) & 0x3; }
	direction getDir() { return (direction)((_flags >> 8) & 0x3); }
	uint16 getDoNotDiscard() { return (_flags >> 10) & 0x1; }
}; // @ GROUP


class GroupMan {
	DMEngine *_vm;
public:
	uint16 _maxActiveGroupCount = 60; // @ G0376_ui_MaximumActiveGroupCount
	ActiveGroup *_activeGroups; // @ G0375_ps_ActiveGroups
	GroupMan(DMEngine *vm);
	~GroupMan();
	void initActiveGroups(); // @ F0196_GROUP_InitializeActiveGroups
	uint16 getGroupCells(Group *group, int16 mapIndex); // @ F0145_DUNGEON_GetGroupCells
};



}

#endif
