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
#ifndef DRAGONS_CURSOR_H
#define DRAGONS_CURSOR_H

#include "common/system.h"
#include "scriptopcodes.h"

namespace Dragons {

class Actor;
class ActorManager;
class DragonsEngine;
class DragonINIResource;

class Cursor {
public:
	int16 data_800728b0_cursor_seqID;
private:
	DragonsEngine *_vm;
	int16 _x;
	int16 _y;
	Actor *_actor;
	int32 _sequenceID;
	uint16 _iniUnderCursor;

	int16 data_either_5_or_0;
	int16 data_8007283c;
	int16 data_80072890;

public:
	Cursor(DragonsEngine *vm);
	void init(ActorManager *actorManager, DragonINIResource *dragonINIResource);
	void update();
	void updateVisibility();
	void updatePosition(int16 x, int16 y);
	int16 updateINIUnderCursor();
	int16 executeScript(ScriptOpCall &scriptOpCall, uint16 unkFlag);
private:
	int16 updateIniFromScene();
};

} // End of namespace Dragons

#endif //DRAGONS_CURSOR_H
