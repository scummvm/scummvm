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
#ifndef __AGS_EE_AC__MOUSE_H
#define __AGS_EE_AC__MOUSE_H

#include "ac/dynobj/scriptmouse.h"

#define DOMOUSE_UPDATE 0
#define DOMOUSE_ENABLE 1
#define DOMOUSE_DISABLE 2
#define DOMOUSE_NOCURSOR 5
// are these mouse buttons? ;/
// note: also defined in ac_cscidialog as const ints
#define NONE -1
#define LEFT  0
#define RIGHT 1

void Mouse_SetVisible(int isOn);
int Mouse_GetVisible();
int Mouse_GetModeGraphic(int curs);
void Mouse_ChangeModeView(int curs, int newview);
// The Mouse:: functions are static so the script doesn't pass
// in an object parameter
void SetMousePosition (int newx, int newy);
int GetCursorMode();
void SetNextCursor ();
// permanently change cursor graphic
void ChangeCursorGraphic (int curs, int newslot);
void ChangeCursorHotspot (int curs, int x, int y);
int IsButtonDown(int which);
void SetMouseBounds (int x1, int y1, int x2, int y2);
void RefreshMouse();
// mouse cursor functions:
// set_mouse_cursor: changes visual appearance to specified cursor
void set_mouse_cursor(int newcurs);
// set_default_cursor: resets visual appearance to current mode (walk, look, etc);
void set_default_cursor();
// set_cursor_mode: changes mode and appearance
void set_cursor_mode(int newmode);
void enable_cursor_mode(int modd);
void disable_cursor_mode(int modd);

// Try to enable or disable mouse speed control by the engine
void Mouse_EnableControl(bool on);

//=============================================================================

int GetMouseCursor();
void update_script_mouse_coords();
void update_inv_cursor(int invnum);
void update_cached_mouse_cursor();
void set_new_cursor_graphic (int spriteslot);
int find_next_enabled_cursor(int startwith);
int find_previous_enabled_cursor(int startwith);

extern ScriptMouse scmouse;

extern int cur_mode;
extern int cur_cursor;

#endif // __AGS_EE_AC__MOUSE_H
