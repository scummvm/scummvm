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

#ifndef AGS_ENGINE_AC_MOUSE_H
#define AGS_ENGINE_AC_MOUSE_H

#include "ags/engine/ac/dynobj/scriptmouse.h"

namespace AGS3 {

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
void SetMousePosition(int newx, int newy);
int GetCursorMode();
void SetNextCursor();
// permanently change cursor graphic
void ChangeCursorGraphic(int curs, int newslot);
void ChangeCursorHotspot(int curs, int x, int y);
int IsButtonDown(int which);
void SetMouseBounds(int x1, int y1, int x2, int y2);
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
void set_new_cursor_graphic(int spriteslot);
int find_next_enabled_cursor(int startwith);
int find_previous_enabled_cursor(int startwith);

extern ScriptMouse scmouse;

extern int cur_mode;
extern int cur_cursor;

} // namespace AGS3

#endif
