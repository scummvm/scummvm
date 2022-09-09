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

#ifndef WATCHMAKER_ACT_H
#define WATCHMAKER_ACT_H

#include "watchmaker/types.h"
#include "watchmaker/t3d.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

void CheckCharacterWithoutBounds(WGame &game, int32 oc, uint8 *dpl, uint8 back);
bool CheckCharacterWithBounds(WGame &game, int32 oc, t3dV3F *Pos, uint8 dp, uint8 back);
void CharSetPosition(int32 oc, uint8 pos, const char *room);
void CharStop(int32 oc);
uint8 CharNextFrame(WGame &game, int32 oc);
bool CharGotoPosition(WGame &game, int32 oc, uint8 pos, uint8 back, int32 anim);
void FixPos(int32 oc);
void UpdateLook(int32 oc);
void BuildStepList(int32 oc, uint8 dp, uint8 back);
void UpdateChar(WGame &game, int32 oc, t3dF32 Speed, t3dF32 Rot );

} // End of namespace Watchmaker

#endif // WATCHMAKER_ACT_H
