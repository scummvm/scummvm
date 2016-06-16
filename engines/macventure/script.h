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

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef MACVENTURE_SCRIPT_H
#define MACVENTURE_SCRIPT_H

#include "macventure/macventure.h"

namespace MacVenture {

enum ControlAction {
	kNoCommand = 0,
	kStartOrResume = 1,
	kClose = 2,
	kTick = 3,
	kActivateObject = 4,
	kMoveObject = 5,
	kConsume = 6,
	kExamine = 7,
	kGo = 8,
	kHit = 9,
	kOpen = 10,
	kOperate = 11,
	kSpeak = 12,
	kBabble = 13,
	kTargetName = 14,
	kDebugObject = 15
};

typedef uint32 ObjID;
	
class ScriptEngine {
public:
	ScriptEngine() {}
	~ScriptEngine() {}

	bool runControl(ControlAction action, ObjID source, ObjID destination, Common::Point delta) {
		debug(7, "SCRIPT: Running control %d from obj %d into obj %d, at delta (%d, %d)", 
			action, source, destination, delta.x, delta.y);
		
		return false;
	}

	bool resume() {
		debug(7, "SCRIPT: Resume");

		return false;
	}
};

} // End of namespace MacVenture

#endif
