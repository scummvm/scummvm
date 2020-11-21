//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
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
