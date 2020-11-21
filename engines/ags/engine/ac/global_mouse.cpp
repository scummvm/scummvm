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

#include "ac/global_mouse.h"
#include "ac/gamestate.h"

extern GameState play;

void HideMouseCursor () {
    play.mouse_cursor_hidden = 1;
}

void ShowMouseCursor () {
    play.mouse_cursor_hidden = 0;
}
