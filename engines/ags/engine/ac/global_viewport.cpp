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

#include "ags/engine/ac/global_viewport.h"
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/globals.h"

namespace AGS3 {

void SetViewport(int offsx, int offsy) {
	offsx = data_to_game_coord(offsx);
	offsy = data_to_game_coord(offsy);
	_GP(play).GetRoomCamera(0)->LockAt(offsx, offsy);
}
void ReleaseViewport() {
	_GP(play).GetRoomCamera(0)->Release();
}
int GetViewportX() {
	return game_to_data_coord(_GP(play).GetRoomCamera(0)->GetRect().Left);
}
int GetViewportY() {
	return game_to_data_coord(_GP(play).GetRoomCamera(0)->GetRect().Top);
}

} // namespace AGS3
