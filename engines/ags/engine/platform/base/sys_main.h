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

//=============================================================================
//
// The main backend interface.
//
// TODO: split up later if it gets filled with functions in all categories.
//
//=============================================================================

#include "ags/shared/core/platform.h"
#include "ags/shared/util/string.h"
#include "common/std/vector.h"
#include "ags/engine/gfx/gfx_defines.h"

namespace AGS3 {

// Initializes main backend system;
// should be called before anything else backend related.
// Returns 0 on success, non-0 on failure.
int  sys_main_init(/*config*/);
// Shutdown main backend system;
// should be called last, after everything else backend related is shutdown.
void sys_main_shutdown();
// Sets whether the engine wants to update while the window has no focus.
// TODO: this is a placeholder at the moment, check later if we need any implementation
void sys_set_background_mode(bool on);

// Display utilities.
//
// Queries the display index on which the window is currently positioned.
// Returns default display index in case window does not exist yet, or on any error.
int sys_get_window_display_index();
// Queries current desktop resolution.
int sys_get_desktop_resolution(int &width, int &height);
// Queries supported desktop modes.
void sys_get_desktop_modes(std::vector<AGS::Engine::DisplayMode> &dms, int color_depth = 0);
// Sets output driver for the backend's renderer
void sys_renderer_set_output(const AGS::Shared::String &name);

// Audio utilities.
//
// Tries to init the audio backend; optionally requests particular driver
bool sys_audio_init(const AGS::Shared::String &driver_name = "");
// Shutdown audio backend
void sys_audio_shutdown();

// Window utilities.
//
struct SDL_Window;
// Create a new single game window.
SDL_Window *sys_window_create(const char *window_title, int w, int h, AGS::Engine::WindowMode mode, int ex_flags = 0);
// Returns current game window, if one exists, or null.
SDL_Window *sys_get_window();
// Sets current window style, does nothing if window was not created.
void sys_window_set_style(AGS::Engine::WindowMode mode, int ex_flags = 0);
// Set new window size; optionally center new window on screen
bool sys_window_set_size(int w, int h, bool center);
// Centers the window on screen, optionally choose the display to position on
void sys_window_center(int display_index = -1);
// Reduces window's size to fit into the said display bounds, and repositions to the display's center
void sys_window_fit_in_display(int display_index);
// Shows or hides system cursor when it's in the game window
void sys_window_show_cursor(bool on);
// Locks on unlocks mouse inside the window.
// Returns new state of the mouse lock.
bool sys_window_lock_mouse(bool on);
// Sets mouse position within the game window
void sys_window_set_mouse(int x, int y);
// Destroy current game window, if one exists.
void sys_window_destroy();
// Set window title text.
void sys_window_set_title(const char *title);
// Set window icon.
// TODO: this is a placeholder, until we figure out the best way to set icon with SDL on wanted systems.
void sys_window_set_icon();

#if AGS_PLATFORM_OS_WINDOWS
// Returns game window's handle.
void *sys_win_get_window();
#endif

} // namespace AGS3
