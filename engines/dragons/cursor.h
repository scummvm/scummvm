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
#include "dragons/scriptopcodes.h"

namespace Dragons {

class Actor;
class ActorManager;
class DragonsEngine;
class DragonINIResource;

class Cursor {
public:
	int16 _data_800728b0_cursor_seqID;
	uint16 _iniUnderCursor;
	int32 _sequenceID;
	int16 _performActionTargetINI;
	int16 _x;
	int16 _y;
	int16 _objectInHandSequenceID;
	int16 _cursorActivationSeqOffset;
	uint16 _iniItemInHand;
	uint16 _handPointerSequenceID;

private:
	DragonsEngine *_vm;
	Actor *_actor;

public:
	Cursor(DragonsEngine *vm);
	void init(ActorManager *actorManager, DragonINIResource *dragonINIResource);
	void update();
	void updateSequenceID(int16 sequenceID);
	void updateVisibility();
	void updatePosition(int16 x, int16 y);
	void updateActorPosition(int16 x, int16 y);
	int16 updateINIUnderCursor();
	int16 executeScript(ScriptOpCall &scriptOpCall, uint16 unkFlag);
	void selectPreviousCursor();
	void selectNextCursor();
	void setActorFlag400();
	void clearActorFlag400();
	byte *getPalette();

private:
	int16 updateIniFromScene();
};

} // End of namespace Dragons

#endif //DRAGONS_CURSOR_H
