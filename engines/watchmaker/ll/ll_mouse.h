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

#ifndef WATCHMAKER_LL_MOUSE_H
#define WATCHMAKER_LL_MOUSE_H

#include "watchmaker/types.h"

namespace Watchmaker {

class WGame;

// MOUSE form (llmouse.c)
extern int32 mPosy, mPosx, mMoveX, mMoveY, mMove, mCounter, mHotspotX, mHotspotY;
extern uint8 bLPressed, bRPressed, mHide;
void ProcessMouse(WGame &game);
void HandleMouseChanges();
void MoveHeadAngles(t3dF32 diffx, t3dF32 diffy);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_MOUSE_H
