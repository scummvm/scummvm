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

#include "common/events.h"
#include "ags/engine/ac/sys_events.h"
//include <deque>
#include "ags/shared/core/platform.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/ac/keycode.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/timer.h"
#include "ags/engine/device/mouse_w32.h"
#include "ags/engine/platform/base/ags_platform_driver.h"
#include "ags/engine/main/engine.h"
#include "ags/ags.h"
#include "ags/events.h"
#include "ags/globals.h"

// TODO: Replace me
typedef int SDL_Scancode;


namespace AGS3 {

// TODO: check later, if this may be useful in other places (then move to header)
enum eAGSMouseButtonMask {
	MouseBitLeft = 0x01,
	MouseBitRight = 0x02,
	MouseBitMiddle = 0x04,
	MouseBitX1 = 0x08,
	MouseBitX2 = 0x10
};

using namespace AGS::Shared;
using namespace AGS::Engine;

extern void domouse(int str);
const int MB_ARRAY[3] = { MouseBitLeft, MouseBitRight, MouseBitMiddle };
static void(*_on_quit_callback)(void) = nullptr;
static void(*_on_switchin_callback)(void) = nullptr;
static void(*_on_switchout_callback)(void) = nullptr;

// ----------------------------------------------------------------------------
// KEYBOARD INPUT
// ----------------------------------------------------------------------------

bool ags_keyevent_ready() {
	return ::AGS::g_events->keypressed();
}

Common::Event ags_get_next_keyevent() {
	return ::AGS::g_events->readKey();
}

int ags_iskeydown(eAGSKeyCode ags_key) {
	return ::AGS::g_events->isKeyPressed(ags_key);
}

void ags_simulate_keypress(eAGSKeyCode ags_key) {
	Common::KeyCode keycode[3];
	if (!::AGS::EventsManager::ags_key_to_scancode(ags_key, keycode))
		return;

	// Push a key event to the event queue; note that this won't affect the key states array
	Common::Event e;
	e.type = Common::EVENT_KEYDOWN;
	e.kbd.keycode = keycode[0];
	e.kbd.ascii = (e.kbd.keycode >= 32 && e.kbd.keycode <= 127) ? e.kbd.keycode : 0;

	::AGS::g_events->pushKeyboardEvent(e);
}

#ifdef TODO
static void on_sdl_key_down(const Common::Event &event) {
	// Engine is not structured very well yet, and we cannot pass this event where it's needed;
	// instead we save it in the queue where it will be ready whenever any component asks for one.
	g_keyEvtQueue.push_back(event);
}
#endif

#ifdef TODO
static void on_sdl_textinput(const Common::Event &event) {
	// We also push text input events to the same queue, as this is only valid way to get proper
	// text interpretation of the pressed key combination based on current system locale.
	g_keyEvtQueue.push_back(event);
}
#endif


// ----------------------------------------------------------------------------
// MOUSE INPUT
// ----------------------------------------------------------------------------

#ifdef TODO
static int sdl_button_to_mask(int button) {
	switch (button) {
	case SDL_BUTTON_LEFT:
		return MouseBitLeft;
	case SDL_BUTTON_RIGHT:
		return MouseBitRight;
	case SDL_BUTTON_MIDDLE:
		return MouseBitMiddle;
	case SDL_BUTTON_X1:
		return MouseBitX1;
	case SDL_BUTTON_X2:
		return MouseBitX2;
	}

	return 0;
}
#endif

// Returns accumulated mouse button state and clears internal cache by timer
static int mouse_button_poll() {
#ifdef TODO
	auto now = AGS_Clock::now();
	int result = _G(mouse_button_state) | _G(mouse_accum_button_state);
	if (now >= _G(mouse_clear_at_time)) {
		_G(mouse_accum_button_state) = 0;
		_G(mouse_clear_at_time) = now + std::chrono::milliseconds(50);
	}
	return result;
#else
	return 0;
#endif
}

#ifdef TODO
static void on_sdl_mouse_motion(const SDL_MouseMotionEvent &event) {
	_G(sys_mouse_x) = _G(event).x;
	_G(sys_mouse_y) = _G(event).y;
	_G(mouse_accum_relx) += _G(event).xrel;
	_G(mouse_accum_rely) += _G(event).yrel;
}

static void on_sdl_mouse_button(const SDL_MouseButtonEvent &event) {
	_G(sys_mouse_x) = _G(event).x;
	_G(sys_mouse_y) = _G(event).y;

	if (_G(event).type == SDL_MOUSEBUTTONDOWN) {
		_G(mouse_button_state) |= sdl_button_to_mask(_G(event).button);
		_G(mouse_accum_button_state) |= sdl_button_to_mask(_G(event).button);
	} else {
		_G(mouse_button_state) &= ~sdl_button_to_mask(_G(event).button);
	}
}

static void on_sdl_mouse_wheel(const SDL_MouseWheelEvent &event) {
	_G(sys_mouse_z) += _G(event).y;
}
#endif

int mgetbutton() {
#ifdef TODO
	int toret = MouseNone;
	int butis = mouse_button_poll();

	if ((butis > 0) & (_G(butwas) > 0))
		return MouseNone;  // don't allow holding button down

	if (butis & MouseBitLeft)
		toret = MouseLeft;
	else if (butis & MouseBitRight)
		toret = MouseRight;
	else if (butis & MouseBitMiddle)
		toret = MouseMiddle;

	_G(butwas) = butis;
	return toret;

	// TODO: presumably this was a hack for 1-button Mac mouse;
	// is this still necessary?
	// find an elegant way to reimplement this; e.g. allow to configure key->mouse mappings?!
#define AGS_SIMULATE_RIGHT_CLICK (AGS_PLATFORM_OS_MACOS)
#if defined (AGS_SIMULATE_RIGHT_CLICK__FIXME)
	// j Ctrl-left click should be right-click
	if (ags_iskeypressed(__allegro_KEY_LCONTROL) || ags_iskeypressed(__allegro_KEY_RCONTROL)) {
		toret = RIGHT;
	}
#endif
#endif
	return 0;
}

bool ags_misbuttondown(int but) {
	return mouse_button_poll() & MB_ARRAY[but];
}

int ags_mgetbutton() {
#ifdef TODO
	int result;

	if (_G(pluginSimulatedClick) > MouseNone) {
		result = _G(pluginSimulatedClick);
		_G(pluginSimulatedClick) = MouseNone;
	} else {
		result = mgetbutton();
	}
	return result;
#endif
	return 0;
}

void ags_mouse_get_relxy(int &x, int &y) {
	x = _G(mouse_accum_relx);
	y = _G(mouse_accum_rely);
	_G(mouse_accum_relx) = 0;
	_G(mouse_accum_rely) = 0;
}

void ags_domouse(int what) {
	// do mouse is "update the mouse x,y and also the cursor position", unless DOMOUSE_NOCURSOR is set.
	if (what == DOMOUSE_NOCURSOR)
		mgetgraphpos();
	else
		domouse(what);
}

int ags_check_mouse_wheel() {
	if (_GP(game).options[OPT_MOUSEWHEEL] == 0) {
		return 0;
	}
	if (_G(sys_mouse_z) == _G(mouse_z_was)) {
		return 0;
	}

	int result = 0;
	if (_G(sys_mouse_z) > _G(mouse_z_was))
		result = 1;   // eMouseWheelNorth
	else
		result = -1;  // eMouseWheelSouth
	_G(mouse_z_was) = _G(sys_mouse_z);
	return result;
}



void ags_clear_input_buffer() {
	::AGS::g_events->clearEvents();
	_G(mouse_accum_relx) = 0;
	_G(mouse_accum_rely) = 0;
	_G(mouse_button_state) = 0;
	_G(mouse_accum_button_state) = 0;
	_G(mouse_clear_at_time) = AGS_Clock::now() + std::chrono::milliseconds(50);
}

// TODO: this is an awful function that should be removed eventually.
// Must replace with proper updateable game state.
void ags_wait_until_keypress() {
	do {
		sys_evt_process_pending();
		_G(platform)->YieldCPU();
	} while (!SHOULD_QUIT && !ags_keyevent_ready());
	ags_clear_input_buffer();
}


// ----------------------------------------------------------------------------
// EVENTS
// ----------------------------------------------------------------------------

void sys_evt_set_quit_callback(void(*proc)(void)) {
	_on_quit_callback = proc;
}

void sys_evt_set_focus_callbacks(void(*switch_in)(void), void(*switch_out)(void)) {
	_on_switchin_callback = switch_in;
	_on_switchout_callback = switch_out;
}

#ifdef TODO
void sys_evt_process_one(const Common::Event &event) {
	switch (_G(event).type) {
	// GENERAL
	case SDL_QUIT:
		if (_on_quit_callback) {
			_on_quit_callback();
		}
		break;
	// WINDOW
	case SDL_WINDOWEVENT:
		switch (_G(event).window.event) {
		case SDL_WINDOWEVENT_FOCUS_GAINED:
			if (_on_switchin_callback) {
				_on_switchin_callback();
			}
			break;
		case SDL_WINDOWEVENT_FOCUS_LOST:
			if (_on_switchout_callback) {
				_on_switchout_callback();
			}
			break;
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			engine_on_window_changed(Size(_G(event).window.data1, _G(event).window.data2));
			break;
		}
		break;
	// INPUT
	case SDL_KEYDOWN:
		on_sdl_key_down(event);
		break;
	case SDL_TEXTINPUT:
		on_sdl_textinput(event);
		break;
	case SDL_MOUSEMOTION:
		on_sdl_mouse_motion(_G(event).motion);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		on_sdl_mouse_button(_G(event).button);
		break;
	case SDL_MOUSEWHEEL:
		on_sdl_mouse_wheel(_G(event).wheel);
		break;
	}
}
#endif

void sys_evt_process_pending(void) {
#ifdef TODO
	Common::Event event;
	while (SDL_PollEvent(&event)) {
		sys_evt_process_one(event);
	}
#endif
}

} // namespace AGS3
