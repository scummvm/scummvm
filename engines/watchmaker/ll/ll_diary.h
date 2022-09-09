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

#ifndef WATCHMAKER_LL_DIARY_H
#define WATCHMAKER_LL_DIARY_H

#include "watchmaker/work_dirs.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

void UpdateAllClocks(WGame &game);
void StopDiary(WGame &game, int32 room, int32 obj, uint8 only_overtime);
void StartDiary(WGame &game, int32 room, t3dV3F *pos);
void IncCurTime(WGame &game, int32 inc);
void DecCurTime(WGame &game, int32 dec);
void SetCurTime(WGame &game, int32 set);
int32 WhichRoomChar(Init &init, int32 ch);
int32 WhichAnimChar(Init &init, int32 ch);
void ContinueDiary(WGame &game, int32 an);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_DIARY_H
