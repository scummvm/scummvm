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

#ifndef AGS_ENGINE_AC_MOUSE_H
#define AGS_ENGINE_AC_MOUSE_H

#include "ags/engine/ac/dynobj/script_mouse.h"

namespace AGS3 {

void Mouse_SetVisible(int isOn);
int Mouse_GetVisible();
int Mouse_GetModeGraphic(int curs);
void Mouse_ChangeModeView(int curs, int newview, int delay);
void Mouse_ChangeModeView2(int curs, int newview);
// The Mouse:: functions are static so the script doesn't pass
// in an object parameter
void SetMousePosition(int newx, int newy);
int GetCursorMode();
void SetNextCursor();
void SetPreviousCursor();
// permanently change cursor graphic
void ChangeCursorGraphic(int curs, int newslot);
void ChangeCursorHotspot(int curs, int x, int y);
int IsButtonDown(int which);
int IsModeEnabled(int which);
void SetMouseBounds(int x1, int y1, int x2, int y2);
void RefreshMouse();
// mouse cursor functions:
// set_mouse_cursor: changes visual appearance to specified cursor
void set_mouse_cursor(int newcurs, bool force_update = false);
// set_default_cursor: resets visual appearance to current mode (walk, look, etc);
void set_default_cursor();
// set_cursor_mode: changes mode and appearance
void set_cursor_mode(int newmode);
void enable_cursor_mode(int modd);
void disable_cursor_mode(int modd);

// Try to enable or disable mouse speed control by the engine
void Mouse_EnableControl(bool on);
void SimulateMouseClick(int button_id);

//=============================================================================

int GetMouseCursor();
void update_script_mouse_coords();
void update_inv_cursor(int invnum);
void update_cached_mouse_cursor();
void set_new_cursor_graphic(int spriteslot);
int find_next_enabled_cursor(int startwith);
int find_previous_enabled_cursor(int startwith);

} // namespace AGS3

#endif
