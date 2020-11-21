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

#include "ac/global_viewport.h"
#include "ac/draw.h"
#include "debug/debug_log.h"

void SetViewport(int offsx, int offsy) {
    offsx = data_to_game_coord(offsx);
    offsy = data_to_game_coord(offsy);
    play.GetRoomCamera(0)->LockAt(offsx, offsy);
}
void ReleaseViewport() {
    play.GetRoomCamera(0)->Release();
}
int GetViewportX () {
    return game_to_data_coord(play.GetRoomCamera(0)->GetRect().Left);
}
int GetViewportY () {
    return game_to_data_coord(play.GetRoomCamera(0)->GetRect().Top);
}
