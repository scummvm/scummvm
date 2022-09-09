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

#ifndef WATCHMAKER_LL_ANIM_H
#define WATCHMAKER_LL_ANIM_H

#include "watchmaker/types.h"
#include "watchmaker/work_dirs.h"
#include "watchmaker/globvar.h"
#include "watchmaker/game.h"

namespace Watchmaker {

void StopObjAnim(WGame &game, int32 obj);
bool CheckAndLoadMoglieSupervisoreModel(WorkDirs &workDirs, int32 c);
void StartAnim(WGame &game, int32 an);
void StopAnim(WGame &game, int32 an);
void PauseAnim(Init &init, int32 an);
void ContinueAnim(Init &init, int32 an);
void StopAllAnims(Init &init);
void StopPlayingGame(WGame &game);
void ProcessATF(WGame &game, int32 an, int32 atf);
void ProcessATFDO(WGame &game, int32 in);
void ProcessAnims(WGame &game);

} // End of namespace Watchmaker

#endif // WATCHMAKER_LL_ANIM_H
