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

#ifndef AGS_ENGINE_AC_SYS_EVENTS_H
#define AGS_ENGINE_AC_SYS_EVENTS_H

#include "common/events.h"
#include "ags/shared/ac/keycode.h"

namespace AGS3 {

// Keyboard input handling
//
// avoid including SDL.h here, at least for now, because that leads to conflicts with allegro
union SDL_Event;

// Tells if key event refers to one of the mod-keys
inline bool is_mod_key(const Common::KeyState &ks) {
	return ks.keycode == Common::KEYCODE_LCTRL || ks.keycode == Common::KEYCODE_RCTRL ||
	       ks.keycode == Common::KEYCODE_LALT || ks.keycode == Common::KEYCODE_RALT ||
	       ks.keycode == Common::KEYCODE_LSHIFT || ks.keycode == Common::KEYCODE_RSHIFT ||
	       ks.keycode == Common::KEYCODE_MODE;
}

// Converts mod key into merged mod (left & right) for easier handling
inline int make_merged_mod(int mod) {
	int m_mod = 0;
	if ((mod & Common::KBD_CTRL) != 0) m_mod |= Common::KBD_CTRL;
	if ((mod & Common::KBD_SHIFT) != 0) m_mod |= Common::KBD_SHIFT;
	if ((mod & Common::KBD_ALT) != 0) m_mod |= Common::KBD_ALT;
	// what about Common::KBD_GUI, and there's also some Common::KEYCODE_MODE?
	return m_mod;
}

// Converts ScummVM key event to eAGSKeyCode if it is in proper range,
// see comments to eAGSKeyCode for details.
// Optionally works in bacward compatible mode (old_keyhandle)
extern KeyInput ags_keycode_from_scummvm(const Common::Event &event, bool old_keyhandle);

// Tells if there are any buffered key events
extern bool ags_keyevent_ready();
// Queries for the next key event in buffer; returns uninitialized data if none was queued
extern Common::Event ags_get_next_keyevent();
// Tells if the key is currently down, provided AGS key.
// NOTE: for particular script codes this function returns positive if either of two keys are down.
extern int ags_iskeydown(eAGSKeyCode ags_key);
// Simulates key press with the given AGS key
extern void ags_simulate_keypress(eAGSKeyCode ags_key, bool old_keyhandle);


// Mouse input handling
//
// Tells if the mouse button is currently down
extern bool ags_misbuttondown(eAGSMouseButton but);
// Returns last "clicked" mouse button
extern eAGSMouseButton ags_mgetbutton();
// Returns recent relative mouse movement; resets accumulated values
extern void ags_mouse_acquire_relxy(int &x, int &y);
// Updates mouse cursor position in game
extern void ags_domouse();
// Returns -1 for wheel down and +1 for wheel up
// TODO: introduce constants for this
extern int  ags_check_mouse_wheel();

// Other input utilities
//
// Clears buffered keypresses and mouse clicks;
// resets current key/mb states
void ags_clear_input_state();
// Clears buffered keypresses and mouse clicks, if any;
// does NOT reset current key/mb states
void ags_clear_input_buffer();
// Clears buffered mouse movement
void ags_clear_mouse_movement();
// Halts execution until any user input
// TODO: seriously not a good design, replace with event listening
extern void ags_wait_until_keypress();


// Events.
//

// Set engine callback for when quit event is received by the backend.
extern void sys_evt_set_quit_callback(void(*proc)(void));
// Set engine callback for when input focus is received or lost by the window.
extern void sys_evt_set_focus_callbacks(void(*switch_in)(void), void(*switch_out)(void));

// Process all events in the backend's queue.
extern void sys_evt_process_pending(void);
// Flushes system events following window initialization.
void sys_flush_events(void);

} // namespace AGS3

#endif
