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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef __AGS_EE_AC__ROOM_H
#define __AGS_EE_AC__ROOM_H

#include "ac/dynobj/scriptdrawingsurface.h"
#include "ac/characterinfo.h"
#include "script/runtimescriptvalue.h"
#include "game/roomstruct.h"

ScriptDrawingSurface* Room_GetDrawingSurfaceForBackground(int backgroundNumber);
ScriptDrawingSurface* Room_GetDrawingSurfaceForMask(RoomAreaMask mask);
int Room_GetObjectCount();
int Room_GetWidth();
int Room_GetHeight();
int Room_GetColorDepth();
int Room_GetLeftEdge();
int Room_GetRightEdge();
int Room_GetTopEdge();
int Room_GetBottomEdge();
int Room_GetMusicOnLoad();
const char* Room_GetTextProperty(const char *property);
int Room_GetProperty(const char *property);
const char* Room_GetMessages(int index);
RuntimeScriptValue Sc_Room_GetProperty(const RuntimeScriptValue *params, int32_t param_count);

//=============================================================================

void  save_room_data_segment ();
void  unload_old_room();
void  load_new_room(int newnum,CharacterInfo*forchar);
void  new_room(int newnum,CharacterInfo*forchar);
int   find_highest_room_entered();
void  first_room_initialization();
void  check_new_room();
void  compile_room_script();
void  on_background_frame_change ();
// Clear the current room pointer if room status is no longer valid
void  croom_ptr_clear();

// These functions convert coordinates between data resolution and region mask.
// In hi-res games region masks are 1:2 (or smaller) of the room size.
// In legacy games with low-res data resolution there's additional conversion
// between data and room coordinates.
//
// gets multiplier for converting from room mask to data coordinate
extern AGS_INLINE int get_roommask_to_data_mul();
// coordinate conversion data ---> room ---> mask
extern AGS_INLINE int room_to_mask_coord(int coord);
// coordinate conversion mask ---> room ---> data
extern AGS_INLINE int mask_to_room_coord(int coord);

struct MoveList;
// Convert move path from room's mask resolution to room resolution
void convert_move_path_to_room_resolution(MoveList *ml);

extern AGS::Common::RoomStruct thisroom;

#endif // __AGS_EE_AC__ROOM_H
