/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef CINE_BGLIST_H
#define CINE_BGLIST_H


#include "common/scummsys.h"
#include "common/list.h"

namespace Cine {

struct BGIncrust {
	byte *unkPtr;
	int16 objIdx;
	int16 param;
	int16 x;
	int16 y;
	int16 frame;
	int16 part;
	int16 bgIdx;
};

extern uint32 var8;

void addToBGList(int16 objIdx);
void addSpriteFilledToBGList(int16 idx);
void removeBgIncrustsWithBgIdx(int16 bgIdx);

void createBgIncrustListElement(int16 objIdx, int16 param);
void resetBgIncrustList();
void loadBgIncrustFromSave(Common::SeekableReadStream &fHandle, bool hasBgIdx = false);

} // End of namespace Cine

#endif
