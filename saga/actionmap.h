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

// Action map module - private header

#ifndef SAGA_ACTIONMAP_H_
#define SAGA_ACTIONMAP_H_

#include "saga/objectmap.h"

namespace Saga {


class ActionMap {
 public:
	 ActionMap(SagaEngine *vm): _vm(vm) {
		 _stepZoneList = NULL;
		 _stepZoneListCount = 0;
	 }
	 ~ActionMap(void) {
		 freeMem();
	 }
	
	void load(const byte *resourcePointer, size_t resourceLength);
	void freeMem();

	int getExitSceneNumber(int index) const;
	int hitTest(const Point &testPoint);
	int draw(SURFACE *ds, int color);
	const HitZone * getHitZone(int index) const {
		if ((index < 0) || (index >= _stepZoneListCount)) {
			error("ActionMap::getHitZone wrong index 0x%X", index);
		}
		return _stepZoneList[index];
	}
	void cmdInfo();

private:
	SagaEngine *_vm;

	int _stepZoneListCount;
	HitZone **_stepZoneList;
};

} // End of namespace Saga

#endif
