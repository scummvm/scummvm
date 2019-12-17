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

#ifndef ULTIMA6_KEYBINDING_KEYS_H
#define ULTIMA6_KEYBINDING_KEYS_H

#include "SDL_version.h"
//#include "sdl-compat.h" // Nuvie doesn't have this file but likely would whenever compatibility is added
#if SDL_VERSION_ATLEAST(1, 3, 0)
#include "SDL_stdinc.h"
#include "SDL_scancode.h"
#endif

#include "SDL_events.h"
#include "KeysEnum.h"
#include "ultima/ultima6/core/nuvie_defs.h"

#ifdef HAVE_JOYSTICK_SUPPORT
typedef enum { AXES_PAIR1, AXES_PAIR2, AXES_PAIR3, AXES_PAIR4, UNHANDLED_AXES_PAIR } joy_axes_pairs;
#endif

#include <vector>
#include <map>
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima6 {

struct str_int_pair {
	const char *str;
	int  num;
};

const int c_maxparams = 1;

struct Action;
struct ActionType {
	const Action *action;
	int params[c_maxparams];
};

struct ltSDLkeysym {
	bool operator()(SDL_Keysym k1, SDL_Keysym k2) const {
		if (k1.sym == k2.sym)
			return k1.mod < k2.mod;
		else
			return k1.sym < k2.sym;
	}
};

typedef std::map<SDL_Keysym, ActionType, ltSDLkeysym>   KeyMap;

class Configuration;

char get_ascii_char_from_keysym(SDL_Keysym keysym);

class KeyBinder {
private:
	KeyMap bindings;

	std::vector<std::string> keyhelp;
	std::vector<std::string> cheathelp;
#ifdef HAVE_JOYSTICK_SUPPORT
	SDL_Joystick *joystick;
	bool repeat_hat, joy_repeat_enabled; // repeat hat instead of axis when hat is found
	uint32 next_axes_pair_update, next_axes_pair2_update, next_axes_pair3_update,
	       next_axes_pair4_update, next_joy_repeat_time;
	uint16 pair1_delay, pair2_delay, pair3_delay, pair4_delay, joy_repeat_delay, x_axis_deadzone,
	       y_axis_deadzone, x_axis2_deadzone, y_axis2_deadzone, x_axis3_deadzone, y_axis3_deadzone,
	       x_axis4_deadzone, y_axis4_deadzone;
	uint8 x_axis, y_axis, x_axis2, y_axis2, x_axis3, y_axis3, x_axis4, y_axis4;
	sint8 enable_joystick;
#endif
	void LoadFromFileInternal(const char *filename);
public:
	KeyBinder(Configuration *config);
	~KeyBinder();
	/* Add keybinding */
	void AddKeyBinding(SDL_Keycode sym, int mod, const Action *action,
	                   int nparams, int *params);

	/* Delete keybinding */
//	void DelKeyBinding(SDL_Keycode sym, int mod); // unused

	/* Other methods */
	void Flush() {
		bindings.clear();
		keyhelp.clear();
		cheathelp.clear();
	}
	ActionType get_ActionType(SDL_Keysym key);
	ActionKeyType GetActionKeyType(ActionType a);
	bool DoAction(ActionType const &a) const;
	KeyMap::iterator get_sdlkey_index(SDL_Keysym key);
	bool HandleEvent(const SDL_Event *event);

	void LoadFromFile(const char *filename);
	void LoadGameSpecificKeys();
	void LoadFromPatch();
	void handle_wrong_key_pressed();
	bool handle_always_available_keys(ActionType a);

	void ShowKeys() const;
#ifdef HAVE_JOYSTICK_SUPPORT
	uint8 get_axis(uint8 index);
	void set_axis(uint8 index, uint8 value);
	SDL_Keycode get_key_from_joy_walk_axes() {
		return get_key_from_joy_axis_motion(x_axis, true);
	}
	SDL_Keycode get_key_from_joy_axis_motion(int axis, bool repeating);
	SDL_Keycode get_key_from_joy_hat_button(uint8 hat_button);
	SDL_Keycode get_key_from_joy_events(SDL_Event *event);
	void init_joystick(sint8 joy_num);
	SDL_Joystick *get_joystick() {
		return joystick;
	}
	uint32 get_next_joy_repeat_time() {
		return next_joy_repeat_time;
	}
	void set_enable_joy_repeat(bool val) {
		if (joy_repeat_delay == 10000) return;
		joy_repeat_enabled = val;
	}
	bool is_joy_repeat_enabled() {
		return joy_repeat_enabled;
	}
	bool is_hat_repeating() {
		return repeat_hat;
	}
	void set_hat_repeating(bool val) {
		repeat_hat = val;
	}
	sint8 get_enable_joystick() {
		return enable_joystick;
	}
	void set_enable_joystick(bool val) {
		enable_joystick = val;
	}
#endif

private:
	void ParseText(char *text, int len);
	void ParseLine(char *line);
	void FillParseMaps();
#ifdef HAVE_JOYSTICK_SUPPORT
	joy_axes_pairs get_axes_pair(int axis);
	uint16 get_x_axis_deadzone(joy_axes_pairs axes_pair);
	uint16 get_y_axis_deadzone(joy_axes_pairs axes_pair);
	SDL_Keycode get_key_from_joy_button(uint8 button);
	SDL_Keycode get_key_from_joy_hat(SDL_JoyHatEvent);
#endif
};

#ifdef HAVE_JOYSTICK_SUPPORT

static const SDL_Keycode FIRST_JOY = (SDL_Keycode)400;
const SDL_Keycode JOY_UP            = FIRST_JOY;               // PS d-pad when analog is disabled. left stick when enabled
const SDL_Keycode JOY_DOWN          = (SDL_Keycode)(FIRST_JOY + 1);
const SDL_Keycode JOY_LEFT          = (SDL_Keycode)(FIRST_JOY + 2);
const SDL_Keycode JOY_RIGHT         = (SDL_Keycode)(FIRST_JOY + 3);
const SDL_Keycode JOY_RIGHTUP       = (SDL_Keycode)(FIRST_JOY + 4);
const SDL_Keycode JOY_RIGHTDOWN     = (SDL_Keycode)(FIRST_JOY + 5);
const SDL_Keycode JOY_LEFTUP        = (SDL_Keycode)(FIRST_JOY + 6);
const SDL_Keycode JOY_LEFTDOWN      = (SDL_Keycode)(FIRST_JOY + 7);
const SDL_Keycode JOY_UP2           = (SDL_Keycode)(FIRST_JOY + 8); // PS right stick when analog is enabled
const SDL_Keycode JOY_DOWN2         = (SDL_Keycode)(FIRST_JOY + 9);
const SDL_Keycode JOY_LEFT2         = (SDL_Keycode)(FIRST_JOY + 10);
const SDL_Keycode JOY_RIGHT2        = (SDL_Keycode)(FIRST_JOY + 11);
const SDL_Keycode JOY_RIGHTUP2      = (SDL_Keycode)(FIRST_JOY + 12);
const SDL_Keycode JOY_RIGHTDOWN2    = (SDL_Keycode)(FIRST_JOY + 13);
const SDL_Keycode JOY_LEFTUP2       = (SDL_Keycode)(FIRST_JOY + 14);
const SDL_Keycode JOY_LEFTDOWN2     = (SDL_Keycode)(FIRST_JOY + 15);
const SDL_Keycode JOY_UP3           = (SDL_Keycode)(FIRST_JOY + 16);
const SDL_Keycode JOY_DOWN3         = (SDL_Keycode)(FIRST_JOY + 17);
const SDL_Keycode JOY_LEFT3         = (SDL_Keycode)(FIRST_JOY + 18);
const SDL_Keycode JOY_RIGHT3        = (SDL_Keycode)(FIRST_JOY + 19);
const SDL_Keycode JOY_RIGHTUP3      = (SDL_Keycode)(FIRST_JOY + 20);
const SDL_Keycode JOY_RIGHTDOWN3    = (SDL_Keycode)(FIRST_JOY + 21);
const SDL_Keycode JOY_LEFTUP3       = (SDL_Keycode)(FIRST_JOY + 22);
const SDL_Keycode JOY_LEFTDOWN3     = (SDL_Keycode)(FIRST_JOY + 23);
const SDL_Keycode JOY_UP4           = (SDL_Keycode)(FIRST_JOY + 24);
const SDL_Keycode JOY_DOWN4         = (SDL_Keycode)(FIRST_JOY + 25);
const SDL_Keycode JOY_LEFT4         = (SDL_Keycode)(FIRST_JOY + 26);
const SDL_Keycode JOY_RIGHT4        = (SDL_Keycode)(FIRST_JOY + 27);
const SDL_Keycode JOY_RIGHTUP4      = (SDL_Keycode)(FIRST_JOY + 28);
const SDL_Keycode JOY_RIGHTDOWN4    = (SDL_Keycode)(FIRST_JOY + 29);
const SDL_Keycode JOY_LEFTUP4       = (SDL_Keycode)(FIRST_JOY + 30);
const SDL_Keycode JOY_LEFTDOWN4     = (SDL_Keycode)(FIRST_JOY + 31);
const SDL_Keycode JOY_HAT_UP        = (SDL_Keycode)(FIRST_JOY + 32); // PS d-pad when analog is enabled
const SDL_Keycode JOY_HAT_DOWN      = (SDL_Keycode)(FIRST_JOY + 33);
const SDL_Keycode JOY_HAT_LEFT      = (SDL_Keycode)(FIRST_JOY + 34);
const SDL_Keycode JOY_HAT_RIGHT     = (SDL_Keycode)(FIRST_JOY + 35);
const SDL_Keycode JOY_HAT_RIGHTUP   = (SDL_Keycode)(FIRST_JOY + 36);
const SDL_Keycode JOY_HAT_RIGHTDOWN = (SDL_Keycode)(FIRST_JOY + 37);
const SDL_Keycode JOY_HAT_LEFTUP    = (SDL_Keycode)(FIRST_JOY + 38);
const SDL_Keycode JOY_HAT_LEFTDOWN  = (SDL_Keycode)(FIRST_JOY + 39);
const SDL_Keycode JOY0              = (SDL_Keycode)(FIRST_JOY + 40); // PS triangle
const SDL_Keycode JOY1              = (SDL_Keycode)(FIRST_JOY + 41); // PS circle
const SDL_Keycode JOY2              = (SDL_Keycode)(FIRST_JOY + 42); // PS x
const SDL_Keycode JOY3              = (SDL_Keycode)(FIRST_JOY + 43); // PS square
const SDL_Keycode JOY4              = (SDL_Keycode)(FIRST_JOY + 44); // PS L2
const SDL_Keycode JOY5              = (SDL_Keycode)(FIRST_JOY + 45); // PS R2
const SDL_Keycode JOY6              = (SDL_Keycode)(FIRST_JOY + 46); // PS L1
const SDL_Keycode JOY7              = (SDL_Keycode)(FIRST_JOY + 47); // PS R1
const SDL_Keycode JOY8              = (SDL_Keycode)(FIRST_JOY + 48); // PS select
const SDL_Keycode JOY9              = (SDL_Keycode)(FIRST_JOY + 49); // PS start
const SDL_Keycode JOY10             = (SDL_Keycode)(FIRST_JOY + 50); // PS L3 (analog must be enabled)
const SDL_Keycode JOY11             = (SDL_Keycode)(FIRST_JOY + 51); // PS R3 (analog must be enabled)
const SDL_Keycode JOY12             = (SDL_Keycode)(FIRST_JOY + 52);
const SDL_Keycode JOY13             = (SDL_Keycode)(FIRST_JOY + 53);
const SDL_Keycode JOY14             = (SDL_Keycode)(FIRST_JOY + 54);
const SDL_Keycode JOY15             = (SDL_Keycode)(FIRST_JOY + 55);
const SDL_Keycode JOY16             = (SDL_Keycode)(FIRST_JOY + 56);
const SDL_Keycode JOY17             = (SDL_Keycode)(FIRST_JOY + 57);
const SDL_Keycode JOY18             = (SDL_Keycode)(FIRST_JOY + 58);
const SDL_Keycode JOY19             = (SDL_Keycode)(FIRST_JOY + 59);
#endif /* HAVE_JOYSTICK_SUPPORT */

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
