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

#include "common/events.h"
#include "ags/engine/ac/sys_events.h"
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
// Convert mouse button id to flags
const int MouseButton2Bits[kNumMouseButtons] =
	{ 0, MouseBitLeft, MouseBitRight, MouseBitMiddle };
static void(*_on_quit_callback)(void) = nullptr;
static void(*_on_switchin_callback)(void) = nullptr;
static void(*_on_switchout_callback)(void) = nullptr;

// ----------------------------------------------------------------------------
// KEYBOARD INPUT
// ----------------------------------------------------------------------------

KeyInput ags_keycode_from_scummvm(const Common::Event &event, bool old_keyhandle) {
	KeyInput ki;

	snprintf(ki.Text, KeyInput::UTF8_ARR_SIZE, "%c", event.kbd.ascii);
	ki.UChar = event.kbd.ascii;
	ki.Key = ::AGS::g_events->scummvm_key_to_ags_key(event, ki.Mod, old_keyhandle);
	ki.CompatKey = ::AGS::g_events->scummvm_key_to_ags_key(event, ki.Mod, true);
	if (!old_keyhandle && ki.CompatKey == eAGSKeyCodeNone)
		ki.CompatKey = ki.Key;
	return ki;
}

bool ags_keyevent_ready() {
	return ::AGS::g_events->keyEventPending();
}

Common::Event ags_get_next_keyevent() {
	return ::AGS::g_events->getPendingKeyEvent();
}

int ags_iskeydown(eAGSKeyCode ags_key) {
	return ::AGS::g_events->isKeyPressed(ags_key, _GP(game).options[OPT_KEYHANDLEAPI] == 0);
}

void ags_simulate_keypress(eAGSKeyCode ags_key, bool old_keyhandle) {
	Common::KeyCode keycode[3];
	if (!::AGS::EventsManager::ags_key_to_scancode(ags_key, keycode))
		return;

	// Push a key event to the event queue; note that this won't affect the key states array
	Common::Event e;
	e.type = Common::EVENT_KEYDOWN;
	e.kbd.keycode = keycode[0];
	e.kbd.ascii = (e.kbd.keycode >= 32 && e.kbd.keycode <= 127) ? e.kbd.keycode : 0;

	::AGS::g_events->pushKeyboardEvent(e);
	e.type = Common::EVENT_KEYUP;
	::AGS::g_events->pushKeyboardEvent(e);
}

// ----------------------------------------------------------------------------
// MOUSE INPUT
// ----------------------------------------------------------------------------

static int scummvm_button_to_mask(Common::EventType type) {
	switch (type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
		return MouseBitLeft;
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		return MouseBitRight;
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		return MouseBitMiddle;
	default:
		return 0;
	}
}

// Returns accumulated mouse button state and clears internal cache by timer
static int mouse_button_poll() {
	auto now = AGS_Clock::now();
	int result = _G(mouse_button_state) | _G(mouse_accum_button_state);
	if (now >= _G(mouse_clear_at_time)) {
		_G(mouse_accum_button_state) = 0;
		_G(mouse_clear_at_time) = now + std::chrono::milliseconds(50);
	}
	return result;
}

static void on_mouse_motion(const Common::Event &event) {
	_G(sys_mouse_x) = event.mouse.x;
	_G(sys_mouse_y) = event.mouse.y;
	_G(mouse_accum_relx) += event.relMouse.x;
	_G(mouse_accum_rely) += event.relMouse.y;
}

static void on_mouse_button(const Common::Event &event) {
	_G(sys_mouse_x) = event.mouse.x;
	_G(sys_mouse_y) = event.mouse.y;

	if (event.type == Common::EVENT_LBUTTONDOWN ||
			event.type == Common::EVENT_RBUTTONDOWN ||
			event.type == Common::EVENT_MBUTTONDOWN) {
		_G(mouse_button_state) |= scummvm_button_to_mask(event.type);
		_G(mouse_accum_button_state) |= scummvm_button_to_mask(event.type);
	} else {
		_G(mouse_button_state) &= ~scummvm_button_to_mask(event.type);
	}
}

static void on_mouse_wheel(const Common::Event &event) {
	if (event.type == Common::EVENT_WHEELDOWN)
		_G(sys_mouse_z)++;
	else
		_G(sys_mouse_z)--;
}

static eAGSMouseButton mgetbutton() {
	const int butis = mouse_button_poll();

	if ((butis > 0) & (_G(butwas) > 0))
		return kMouseNone;  // don't allow holding button down

	_G(butwas) = butis;
	if (butis & MouseBitLeft)
		return kMouseLeft;
	else if (butis & MouseBitRight)
		return kMouseRight;
	else if (butis & MouseBitMiddle)
		return kMouseMiddle;
	return kMouseNone;
}

bool ags_misbuttondown(eAGSMouseButton but) {
	return (mouse_button_poll() & MouseButton2Bits[but]) != 0;
}

eAGSMouseButton ags_mgetbutton() {
	if (_G(simulatedClick) > kMouseNone) {
		eAGSMouseButton mbut = _G(simulatedClick);
		_G(simulatedClick) = kMouseNone;
		return mbut;
	}
	return mgetbutton();
}

void ags_mouse_acquire_relxy(int &x, int &y) {
	x = _G(mouse_accum_relx);
	y = _G(mouse_accum_rely);
	_G(mouse_accum_relx) = 0;
	_G(mouse_accum_rely) = 0;
}

void ags_domouse() {
	_GP(mouse).Poll();
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

void ags_clear_input_state() {
	// Clear everything related to the input field
	::AGS::g_events->clearEvents();
	_G(mouse_button_state) = 0;
	_G(mouse_accum_button_state) = 0;
	_G(mouse_clear_at_time) = AGS_Clock::now();
	ags_clear_mouse_movement();
}

void ags_clear_input_buffer() {
	::AGS::g_events->clearEvents();
	// accumulated state only helps to not miss clicks
	_G(mouse_accum_button_state) = 0;
	// forget about recent mouse relative movement too
	ags_clear_mouse_movement();
}

void ags_clear_mouse_movement() {
	_G(mouse_accum_relx) = 0;
	_G(mouse_accum_rely) = 0;
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

static void sys_process_event(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		on_mouse_motion(event);
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MBUTTONUP:
		on_mouse_button(event);
		break;
	case Common::EVENT_WHEELDOWN:
	case Common::EVENT_WHEELUP:
		on_mouse_wheel(event);
		break;
	default:
		break;
	}
}

void sys_evt_process_pending(void) {
	::AGS::g_events->pollEvents();
	Common::Event e;

	while ((e = ::AGS::g_events->readEvent()).type != Common::EVENT_INVALID)
		sys_process_event(e);
}

void sys_flush_events(void) {
	::AGS::g_events->clearEvents();
	ags_clear_input_state();
}

} // namespace AGS3
