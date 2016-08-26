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

#include "group.h"
#include "dungeonman.h"
#include "champion.h"



namespace DM {


GroupMan::GroupMan(DMEngine* vm) : _vm(vm) {
	_g375_activeGroups = nullptr;
	_g376_maxActiveGroupCount = 60;
}

GroupMan::~GroupMan() {
	delete[] _g375_activeGroups;
}

void GroupMan::f196_initActiveGroups() {
	if (_vm->_g298_newGame)
		_g376_maxActiveGroupCount = 60;
	if (_g375_activeGroups)
		delete[] _g375_activeGroups;
	_g375_activeGroups = new ActiveGroup[_g376_maxActiveGroupCount];
	for (uint16 i = 0; i < _g376_maxActiveGroupCount; ++i)
		_g375_activeGroups[i]._groupThingIndex = -1;
}

uint16 GroupMan::f145_getGroupCells(Group* group, int16 mapIndex) {
	byte cells;
	cells = group->_cells;
	if (mapIndex == _vm->_dungeonMan->_g309_partyMapIndex)
		cells = _g375_activeGroups[cells]._cells;
	return cells;
}

byte gGroupDirections[4] = {0x00, 0x55, 0xAA, 0xFF}; // @ G0258_auc_Graphic559_GroupDirections

uint16 GroupMan::f147_getGroupDirections(Group* group, int16 mapIndex) {
	if (mapIndex == _vm->_dungeonMan->_g309_partyMapIndex)
		return _g375_activeGroups[group->getActiveGroupIndex()]._directions;

	return gGroupDirections[group->getDir()];
}

int16 GroupMan::f176_getCreatureOrdinalInCell(Group* group, uint16 cell) {
	uint16 currMapIndex = _vm->_dungeonMan->_g272_currMapIndex;
	byte groupCells = f145_getGroupCells(group, currMapIndex);
	if (groupCells == k255_CreatureTypeSingleCenteredCreature)
		return _vm->M0_indexToOrdinal(0);

	byte creatureIndex = group->getCount();
	if (getFlag(g243_CreatureInfo[group->_type]._attributes, k0x0003_MaskCreatureInfo_size) == k1_MaskCreatureSizeHalf) {
		if ((f147_getGroupDirections(group, currMapIndex) & 1) == (cell & 1))
			cell = returnPrevVal(cell);

		do {
			byte creatureCell = M50_getCreatureValue(groupCells, creatureIndex);
			if (creatureCell == cell || creatureCell == returnNextVal(cell))
				return _vm->M0_indexToOrdinal(creatureIndex);
		} while (creatureIndex--);
	} else {
		do {
			if (M50_getCreatureValue(groupCells, creatureIndex) == cell)
				return _vm->M0_indexToOrdinal(creatureIndex);
		} while (creatureIndex--);
	}
	return 0;
}

uint16 GroupMan::M50_getCreatureValue(uint16 groupVal, uint16 creatureIndex) {
	return (groupVal >> (creatureIndex << 1)) & 0x3;
}
}
