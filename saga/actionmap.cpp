/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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

/* Action map module */
#include "saga/saga.h"

#include "saga/gfx.h"
#include "saga/console.h"

#include "saga/actionmap.h"
#include "saga/stream.h"

namespace Saga {


void ActionMap::load(const byte *resourcePointer, size_t resourceLength) {
	int i;

	if (resourceLength < 4) {
		error("ActionMap::load wrong resourceLength");
	}

	MemoryReadStreamEndian readS(resourcePointer, resourceLength, IS_BIG_ENDIAN);

	_stepZoneListCount = readS.readSint16();
	if (_stepZoneListCount < 0) {
		error("ActionMap::load _stepZoneListCount < 0");
	}

	if (_stepZoneList)
		error("ActionMap::load _stepZoneList != NULL");

	_stepZoneList = (HitZone **) malloc(_stepZoneListCount * sizeof(HitZone *));
	if (_stepZoneList == NULL) {
		error("ActionMap::load Memory allocation failure");
	}

	for (i = 0; i < _stepZoneListCount; i++) {
		 _stepZoneList[i] = new HitZone(&readS);
	}
}

void ActionMap::freeMem() {
	int i;

	if (_stepZoneList) {
		for (i = 0; i < _stepZoneListCount; i++) {
			delete _stepZoneList[i];
		}

		free(_stepZoneList);
		_stepZoneList = NULL;
	}
}

int ActionMap::getExitSceneNumber(int index) const {
	if (index >= _stepZoneListCount)
		error("ActionMap::getExitSceneNumber wrong index");

	return _stepZoneList[index]->getSceneNumber();
}

int ActionMap::hitTest(const Point &testPoint) {
	int i;

	// Loop through all scene objects
	for (i = 0; i < _stepZoneListCount; i++) {
		if (_stepZoneList[i]->hitTest(testPoint)) {
			return i;
		}
	}

	return -1;
}

int ActionMap::draw(SURFACE *ds, int color) {
	int i;

	for (i = 0; i < _stepZoneListCount; i++) {
		_stepZoneList[i]->draw(ds, color);
	}

	return SUCCESS;
}

void ActionMap::cmdInfo() {
	_vm->_console->DebugPrintf("%d step zone(s) loaded.\n\n", _stepZoneListCount);

	for (int i = 0; i < _stepZoneListCount; i++) {
		_vm->_console->DebugPrintf("StepZone %d: Exit to Scene number: %d\n", i, _stepZoneList[i]->getSceneNumber());
	}
}

} // End of namespace Saga
