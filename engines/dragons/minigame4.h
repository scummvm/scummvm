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
#ifndef DRAGONS_MINIGAME4_H
#define DRAGONS_MINIGAME4_H

#include "common/system.h"

namespace Dragons {

class DragonsEngine;

class Actor;

class Minigame4 {
private:
	DragonsEngine *_vm;

	Actor *DAT_80090434;
	Actor *DAT_80090440;
	Actor *DAT_80090430;
	Actor *DAT_80090438;
	Actor *DAT_8009043c;


public:
	uint16 DAT_80090428_videoUpdateRelated;

	Minigame4(DragonsEngine *vm);

	void run();
private:
	void actorTalk(Actor *actorId,ushort param_2,uint32 textIndex);
	void actorDialog(Actor *actorId,ushort param_2,uint32 textIndex);
	uint16 runDanceBattle();
	uint16 singleDanceRound(uint16 desiredPosition, uint16 duration);
	void resetActors();
	void updateFlickerFromInput(uint16 desiredPosition);
	uint16 FUN_8009009c(uint16 unk);
};

} // End of namespace Dragons

#endif //DRAGONS_MINIGAME4_H
