
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

#ifndef M4_GUI_GUI_SYS_H
#define M4_GUI_GUI_SYS_H

#include "m4/m4_types.h"
#include "m4/gui/gui_univ.h"

namespace M4 {

bool gui_system_init();
void gui_system_shutdown();

/**
 * Determine if an event happens, and if so, which window should process the event
 * This is the main "engine call" of the GUI.  It should be called once each time in the
 * applications main loop.  All "hot key" call backs, and evtHandlers are executed from here.
 */
void gui_system_event_handler();

/**
 * Add a "hot key" to the system (as opposed to a specific window).
 * @param myKey		The "key" which, when pressed, will cause the callback function to be executed.
 * @param callback	The function to be executed when "myKey" is pressed.
 * @remarks		If the view manager has not been initialized, or sizeof(Hotkey) memory is not available,
 * the procedure will be aborted.
 */
void AddSystemHotkey(int32 myKey, HotkeyCB callback);

/**
 * Remove a "hot key" from the system
 */
void RemoveSystemHotkey(int32 myKey);

/**
 * To find the callback associated with a "hot key" in the system
 */
HotkeyCB GetSystemHotkey(int32 myKey);

} // End of namespace M4

#endif
