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
#ifndef DRAGONS_CUTSCENE_H
#define DRAGONS_CUTSCENE_H

#include "common/system.h"

namespace Dragons {

class Actor;
class DragonsEngine;

class CutScene {
private:
	DragonsEngine *_vm;

	Actor *_actor_80072de8;
	Actor *_actor_80072dec;
	Actor *_actor_800830c0;
	Actor *_actor_80072df0;
	Actor *_actor_80072e08;
	Actor *_actor_80072df4;
	Actor *_actor_80072df8;
	Actor *_actor_80072dfc;
	Actor *_actor_800830bc;
	Actor *_actor_800830b8;
	Actor *_actor_80072e0c;
	Actor *_actor_800830a0;
	Actor *_actor_800830d4;
	Actor *_actor_800830dc;
	Actor *_flameActor;

	uint16 _actor_80063514; //flags

	byte *_palettes;
public:
	CutScene(DragonsEngine *vm);

	virtual ~CutScene();

	void scene1();
	void diamondScene();
	void knightsSavedBackAtCastle();
	void flameReturnsCutScene();
	void knightsSavedAgain();
	void tournamentCutScene();

private:
	//Scene 1 related functions
	void fadeScreenAndResetActor(Actor *actor);
	void closeUpShotOnActor(uint16 resourceId, uint16 sequenceId, int16 x, uint32 param_4);
	void fun_8003d8e8(uint16 resourceId, uint16 sequenceId, int16 x, uint32 param_4);
	void wideAngleEveryoneAtTable();
	void fun_8003d388();
	void closeUpKnightsAtTable();
	uint16 fun_8003dab8(uint32 textId, uint16 x, uint16 y, uint16 param_4, int16 param_5);
	void cursorInventoryClearFlag400();
	void changeBackgroundPosition(uint16 newPosition, int16 sParm2);
	void loadPalettes();
};

} // End of namespace Dragons

#endif //DRAGONS_CUTSCENE_H
