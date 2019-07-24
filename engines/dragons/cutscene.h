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

	Actor *DAT_80072de8;
	Actor *DAT_80072dec;
	Actor *DAT_800830c0;
	Actor *DAT_80072df0;
	Actor *DAT_80072e08;
	Actor *DAT_80072df4;
	Actor *DAT_80072df8;
	Actor *DAT_80072dfc;

	uint16 DAT_80063514; //flags
public:
	CutScene(DragonsEngine *vm);

	void scene1();

private:
	//Scene 1 related functions
	void FUN_8003c108(Actor *actor);
	void FUN_8003d97c(uint16 resourceId, uint16 sequenceId, int16 x, uint32 param_4);
	void FUN_8003d8e8(uint16 resourceId,uint16 sequenceId,int16 x,uint32 param_4);
	void FUN_8003d7d4();
	void FUN_8003d388();
	void FUN_8003d7fc();
};

} // End of namespace Dragons

#endif //DRAGONS_CUTSCENE_H
