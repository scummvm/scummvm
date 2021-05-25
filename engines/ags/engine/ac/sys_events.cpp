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


// Converts SDL scan and key codes to the ags keycode
eAGSKeyCode ags_keycode_from_sdl(const SDL_Event &event) {
#ifdef TODO
	// Printable ASCII characters are returned only from SDL_TEXTINPUT event,
	// as it has key presses + mods correctly converted using current system locale already,
	// so no need to do that manually.
	// NOTE: keycodes such as SDLK_EXCLAIM ('!') could be misleading, as they are NOT
	// received when user presses for example Shift + 1 on regular keyboard, but only on
	// systems where single keypress can produce that symbol.
	// NOTE: following will not work for Unicode, will need to reimplement whole thing
	if (_G(event).type == SDL_TEXTINPUT) {
		unsigned char textch = _G(event).text.text[0];
		if (textch >= 32 && textch <= 255) {
			return static_cast<eAGSKeyCode>(textch);
		}
		return eAGSKeyCodeNone;
	}

	if (_G(event).type != SDL_KEYDOWN)
		return eAGSKeyCodeNone;

	const SDL_Keysym key = _G(event).key.keysym;
	const SDL_Keycode sym = key.sym;
	const Uint16 mod = key.mod;
	// Ctrl and Alt combinations realign the letter code to certain offset
	if (sym >= SDLK_a && sym <= SDLK_z) {
		if ((mod & KMOD_CTRL) != 0) // align letters to code 1
			return static_cast<eAGSKeyCode>(0 + (sym - SDLK_a) + 1);
		else if ((mod & KMOD_ALT) != 0) // align letters to code 301
			return static_cast<eAGSKeyCode>(AGS_EXT_KEY_SHIFT + (sym - SDLK_a) + 1);
	}

	// Remaining codes may match or not, but we use a big table anyway.
	// TODO: this is code by [sonneveld],
	// double check that we must use scan codes here, maybe can use sdl key (sym) too?
	switch (key.scancode) {
	case SDL_SCANCODE_BACKSPACE:
		return eAGSKeyCodeBackspace;
	case SDL_SCANCODE_TAB:
	case SDL_SCANCODE_KP_TAB:
		return eAGSKeyCodeTab;
	case SDL_SCANCODE_RETURN:
	case SDL_SCANCODE_RETURN2:
	case SDL_SCANCODE_KP_ENTER:
		return eAGSKeyCodeReturn;
	case SDL_SCANCODE_ESCAPE:
		return eAGSKeyCodeEscape;

	case SDL_SCANCODE_F1:
		return eAGSKeyCodeF1;
	case SDL_SCANCODE_F2:
		return eAGSKeyCodeF2;
	case SDL_SCANCODE_F3:
		return eAGSKeyCodeF3;
	case SDL_SCANCODE_F4:
		return eAGSKeyCodeF4;
	case SDL_SCANCODE_F5:
		return eAGSKeyCodeF5;
	case SDL_SCANCODE_F6:
		return eAGSKeyCodeF6;
	case SDL_SCANCODE_F7:
		return eAGSKeyCodeF7;
	case SDL_SCANCODE_F8:
		return eAGSKeyCodeF8;
	case SDL_SCANCODE_F9:
		return eAGSKeyCodeF9;
	case SDL_SCANCODE_F10:
		return eAGSKeyCodeF10;
	case SDL_SCANCODE_F11:
		return eAGSKeyCodeF11;
	case SDL_SCANCODE_F12:
		return eAGSKeyCodeF12;

	case SDL_SCANCODE_KP_7:
	case SDL_SCANCODE_HOME:
		return eAGSKeyCodeHome;
	case SDL_SCANCODE_KP_8:
	case SDL_SCANCODE_UP:
		return eAGSKeyCodeUpArrow;
	case SDL_SCANCODE_KP_9:
	case SDL_SCANCODE_PAGEUP:
		return eAGSKeyCodePageUp;
	case SDL_SCANCODE_KP_4:
	case SDL_SCANCODE_LEFT:
		return eAGSKeyCodeLeftArrow;
	case SDL_SCANCODE_KP_5:
		return eAGSKeyCodeNumPad5;
	case SDL_SCANCODE_KP_6:
	case SDL_SCANCODE_RIGHT:
		return eAGSKeyCodeRightArrow;
	case SDL_SCANCODE_KP_1:
	case SDL_SCANCODE_END:
		return eAGSKeyCodeEnd;
	case SDL_SCANCODE_KP_2:
	case SDL_SCANCODE_DOWN:
		return eAGSKeyCodeDownArrow;
	case SDL_SCANCODE_KP_3:
	case SDL_SCANCODE_PAGEDOWN:
		return eAGSKeyCodePageDown;
	case SDL_SCANCODE_KP_0:
	case SDL_SCANCODE_INSERT:
		return eAGSKeyCodeInsert;
	case SDL_SCANCODE_KP_PERIOD:
	case SDL_SCANCODE_DELETE:
		return eAGSKeyCodeDelete;

	default:
		return eAGSKeyCodeNone;
	}
#else
	error("TODO: ags_keycode_from_sdl");
#endif
	return eAGSKeyCodeNone;
}

// Converts ags key to SDL key scans (up to 3 values, because this is not a 1:1 match);
// NOTE: will ignore Ctrl+ or Alt+ script keys.
// TODO: double check and ammend later if anything is missing
bool ags_key_to_sdl_scan(eAGSKeyCode key, SDL_Scancode(&scan)[3]) {
#ifdef TODO
	scan[0] = SDL_SCANCODE_UNKNOWN;
	scan[1] = SDL_SCANCODE_UNKNOWN;
	scan[2] = SDL_SCANCODE_UNKNOWN;
	SDL_Keycode sym = SDLK_UNKNOWN;

	// SDL sym codes happen to match small ASCII letters, so lowercase ours if necessary
	if (key >= eAGSKeyCodeA && key <= eAGSKeyCodeZ) {
		sym = static_cast<SDL_Keycode>(key - eAGSKeyCodeA + SDLK_a);
	}
	// Rest of the printable characters seem to match (and match ascii codes)
	else if (key >= eAGSKeyCodeSpace && key <= eAGSKeyCodeBackquote) {
		sym = static_cast<SDL_Keycode>(key);
	}

	// If we have got key sym, convert it to SDL scancode using library's function
	if (sym != SDLK_UNKNOWN) {
		scan[0] = SDL_GetScancodeFromKey(sym);
		return true;
	}

	// Other keys are mapped directly to scancode (based on [sonneveld]'s code)
	switch (key) {
	case eAGSKeyCodeBackspace:
		scan[0] = SDL_SCANCODE_BACKSPACE;
		scan[1] = SDL_SCANCODE_KP_BACKSPACE;
		return true;
	case eAGSKeyCodeTab:
		scan[0] = SDL_SCANCODE_TAB;
		scan[1] = SDL_SCANCODE_KP_TAB;
		return true;
	case eAGSKeyCodeReturn:
		scan[0] = SDL_SCANCODE_RETURN;
		scan[1] = SDL_SCANCODE_RETURN2;
		scan[2] = SDL_SCANCODE_KP_ENTER;
		return true;
	case eAGSKeyCodeEscape:
		scan[0] = SDL_SCANCODE_ESCAPE;
		return true;

	case eAGSKeyCodeF1:
		scan[0] = SDL_SCANCODE_F1;
		return true;
	case eAGSKeyCodeF2:
		scan[0] = SDL_SCANCODE_F2;
		return true;
	case eAGSKeyCodeF3:
		scan[0] = SDL_SCANCODE_F3;
		return true;
	case eAGSKeyCodeF4:
		scan[0] = SDL_SCANCODE_F4;
		return true;
	case eAGSKeyCodeF5:
		scan[0] = SDL_SCANCODE_F5;
		return true;
	case eAGSKeyCodeF6:
		scan[0] = SDL_SCANCODE_F6;
		return true;
	case eAGSKeyCodeF7:
		scan[0] = SDL_SCANCODE_F7;
		return true;
	case eAGSKeyCodeF8:
		scan[0] = SDL_SCANCODE_F8;
		return true;
	case eAGSKeyCodeF9:
		scan[0] = SDL_SCANCODE_F9;
		return true;
	case eAGSKeyCodeF10:
		scan[0] = SDL_SCANCODE_F10;
		return true;
	case eAGSKeyCodeF11:
		scan[0] = SDL_SCANCODE_F11;
		return true;
	case eAGSKeyCodeF12:
		scan[0] = SDL_SCANCODE_F12;
		return true;

	case eAGSKeyCodeHome:
		scan[0] = SDL_SCANCODE_KP_7;
		scan[1] = SDL_SCANCODE_HOME;
		return true;
	case eAGSKeyCodeUpArrow:
		scan[0] = SDL_SCANCODE_KP_8;
		scan[1] = SDL_SCANCODE_UP;
		return true;
	case eAGSKeyCodePageUp:
		scan[0] = SDL_SCANCODE_KP_9;
		scan[1] = SDL_SCANCODE_PAGEUP;
		return true;
	case eAGSKeyCodeLeftArrow:
		scan[0] = SDL_SCANCODE_KP_4;
		scan[1] = SDL_SCANCODE_LEFT;
		return true;
	case eAGSKeyCodeNumPad5:
		scan[0] = SDL_SCANCODE_KP_5;
		return true;
	case eAGSKeyCodeRightArrow:
		scan[0] = SDL_SCANCODE_KP_6;
		scan[1] = SDL_SCANCODE_RIGHT;
		return true;
	case eAGSKeyCodeEnd:
		scan[0] = SDL_SCANCODE_KP_1;
		scan[1] = SDL_SCANCODE_END;
		return true;
	case eAGSKeyCodeDownArrow:
		scan[0] = SDL_SCANCODE_KP_2;
		scan[1] = SDL_SCANCODE_DOWN;
		return true;
	case eAGSKeyCodePageDown:
		scan[0] = SDL_SCANCODE_KP_3;
		scan[1] = SDL_SCANCODE_PAGEDOWN;
		return true;
	case eAGSKeyCodeInsert:
		scan[0] = SDL_SCANCODE_KP_0;
		scan[1] = SDL_SCANCODE_INSERT;
		return true;
	case eAGSKeyCodeDelete:
		scan[0] = SDL_SCANCODE_KP_PERIOD;
		scan[1] = SDL_SCANCODE_DELETE;
		return true;

	case eAGSKeyCodeLShift:
		scan[0] = SDL_SCANCODE_LSHIFT;
		return true;
	case eAGSKeyCodeRShift:
		scan[0] = SDL_SCANCODE_RSHIFT;
		return true;
	case eAGSKeyCodeLCtrl:
		scan[0] = SDL_SCANCODE_LCTRL;
		return true;
	case eAGSKeyCodeRCtrl:
		scan[0] = SDL_SCANCODE_RCTRL;
		return true;
	case eAGSKeyCodeLAlt:
		scan[0] = SDL_SCANCODE_LALT;
		return true;
	case eAGSKeyCodeRAlt:
		scan[0] = SDL_SCANCODE_RALT;
		return true;

	default:
		return false;
	}
#else
	error("TODO: ags_key_to_sdl_scan");
#endif
	return false;
}


// ----------------------------------------------------------------------------
// KEYBOARD INPUT
// ----------------------------------------------------------------------------

#ifdef TODO
// Because our game engine still uses input polling, we have to accumulate
// key events for our internal use whenever engine have to query key input.
static Common::Queue<Common::Event> g_keyEvtQueue;
#endif

bool ags_keyevent_ready() {
#ifdef TODO
	return g_keyEvtQueue.size() > 0;
#else
	return false;
#endif
}

#ifdef TODO
SDL_Event ags_get_next_keyevent() {
	if (g_keyEvtQueue.size() > 0) {
		auto evt = g_keyEvtQueue.front();
		g_keyEvtQueue.pop_front();
		return evt;
	}
	SDL_Event empty = {};
	return empty;
}
#endif

int ags_iskeydown(eAGSKeyCode ags_key) {
#ifdef TODO
	SDL_PumpEvents();
	const Uint8 *state = SDL_GetKeyboardState(NULL);
	SDL_Scancode scan[3];
	if (!ags_key_to_sdl_scan(ags_key, scan))
		return -1;
	return (state[scan[0]] || state[scan[1]] || state[scan[2]]);
#else
	return 0;
#endif
}

void ags_simulate_keypress(eAGSKeyCode ags_key) {
#ifdef TODO
	SDL_Scancode scan[3];
	if (!ags_key_to_sdl_scan(ags_key, scan))
		return;
	// Push a key event to the event queue; note that this won't affect the key states array
	SDL_Event sdlevent = {};
	sdl_G(event).type = SDL_KEYDOWN;
	sdl_G(event).key.keysym.sym = SDL_GetKeyFromScancode(scan[0]);
	sdl_G(event).key.keysym.scancode = scan[0];
	SDL_PushEvent(&sdlevent);
#endif
}

#ifdef TODO
static void on_sdl_key_down(const SDL_Event &event) {
	// Engine is not structured very well yet, and we cannot pass this event where it's needed;
	// instead we save it in the queue where it will be ready whenever any component asks for one.
	g_keyEvtQueue.push_back(event);
}
#endif

#ifdef TODO
static void on_sdl_textinput(const SDL_Event &event) {
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
	int result = mouse_button_state | mouse_accum_button_state;
	if (now >= mouse_clear_at_time) {
		mouse_accum_button_state = 0;
		mouse_clear_at_time = now + std::chrono::milliseconds(50);
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
		mouse_button_state |= sdl_button_to_mask(_G(event).button);
		mouse_accum_button_state |= sdl_button_to_mask(_G(event).button);
	} else {
		mouse_button_state &= ~sdl_button_to_mask(_G(event).button);
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
#ifdef TODO
	g_keyEvtQueue.clear();
	mouse_button_state = 0;
	mouse_accum_button_state = 0;
	mouse_clear_at_time = AGS_Clock::now() + std::chrono::milliseconds(50);
	_G(mouse_accum_relx) = 0;
	_G(mouse_accum_rely) = 0;
#endif
}

// TODO: this is an awful function that should be removed eventually.
// Must replace with proper updateable game state.
void ags_wait_until_keypress() {
	do {
		sys_evt_process_pending();
		_G(platform)->YieldCPU();
	} while (!ags_keyevent_ready());
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
void sys_evt_process_one(const SDL_Event &event) {
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
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		sys_evt_process_one(event);
	}
#endif
}

} // namespace AGS3
