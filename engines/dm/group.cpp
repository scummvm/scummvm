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
	_activeGroups = nullptr;
}

GroupMan::~GroupMan() {
	delete[] _activeGroups;
}

void GroupMan::initActiveGroups() {
	if (_vm->_dungeonMan->_messages._newGame)
		_maxActiveGroupCount = 60;
	if (_activeGroups)
		delete[] _activeGroups;
	_activeGroups = new ActiveGroup[_maxActiveGroupCount];
	for (uint16 i = 0; i < _maxActiveGroupCount; ++i)
		_activeGroups[i]._groupThingIndex = -1;
}

uint16 GroupMan::getGroupCells(Group* group, int16 mapIndex) {
	byte cells;
	cells = group->_cells;
	if (mapIndex == _vm->_dungeonMan->_currMap._currPartyMapIndex)
		cells = _activeGroups[cells]._cells;
	return cells;
}
}
