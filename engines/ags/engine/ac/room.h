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

#ifndef AGS_ENGINE_AC_ROOM_H
#define AGS_ENGINE_AC_ROOM_H

#include "ags/engine/ac/dynobj/script_drawing_surface.h"
#include "ags/shared/ac/character_info.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/shared/game/room_struct.h"

namespace AGS3 {

ScriptDrawingSurface *Room_GetDrawingSurfaceForBackground(int backgroundNumber);
ScriptDrawingSurface *Room_GetDrawingSurfaceForMask(RoomAreaMask mask);
int Room_GetObjectCount();
int Room_GetWidth();
int Room_GetHeight();
int Room_GetColorDepth();
int Room_GetLeftEdge();
int Room_GetRightEdge();
int Room_GetTopEdge();
int Room_GetBottomEdge();
int Room_GetMusicOnLoad();
const char *Room_GetTextProperty(const char *property);
int Room_GetProperty(const char *property);
bool Room_SetProperty(const char *property, int value);
bool Room_SetTextProperty(const char *property, const char *value);
const char *Room_GetMessages(int index);
bool Room_Exists(int room);
RuntimeScriptValue Sc_Room_GetProperty(const RuntimeScriptValue *params, int32_t param_count);
ScriptDrawingSurface *GetDrawingSurfaceForWalkableArea();
ScriptDrawingSurface *GetDrawingSurfaceForWalkbehind();
ScriptDrawingSurface *Hotspot_GetDrawingSurface();
ScriptDrawingSurface *Region_GetDrawingSurface();

//=============================================================================

void  save_room_data_segment();
void  unload_old_room();
void  load_new_room(int newnum, CharacterInfo *forchar);
void  new_room(int newnum, CharacterInfo *forchar);
// Sets up a placeholder room object; this is used to avoid occasional crashes
// in case an API function was called that needs to access a room, while no real room is loaded
void  set_room_placeholder();
int   find_highest_room_entered();
void  first_room_initialization();
void  check_new_room();
void  compile_room_script();
void  on_background_frame_change();
// Clear the current room pointer if room status is no longer valid
void  croom_ptr_clear();

// Following functions convert coordinates between room resolution and region mask.
// Region masks can be 1:N of the room size: 1:1, 1:2 etc.
// In contemporary games this is simply multiplying or dividing on mask resolution.
// In legacy upscale mode (and generally pre-3.* high-res games) things are more
// complicated, as first we need to make an additional conversion between data coords
// and upscale game coordinates.
//
// coordinate conversion (data) ---> game ---> (room mask)
extern int room_to_mask_coord(int coord);
// coordinate conversion (room mask) ---> game ---> (data)
extern int mask_to_room_coord(int coord);

struct MoveList;
// Convert move path from room's mask resolution to room resolution
void convert_move_path_to_room_resolution(MoveList *ml, int from_step = 0, int to_step = -1);

} // namespace AGS3

#endif
