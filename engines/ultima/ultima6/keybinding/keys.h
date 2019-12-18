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

#include "ultima/ultima6/keybinding/keys_enum.h"
#include "ultima/ultima6/core/nuvie_defs.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "common/events.h"

#ifdef HAVE_JOYSTICK_SUPPORT
typedef enum { AXES_PAIR1, AXES_PAIR2, AXES_PAIR3, AXES_PAIR4, UNHANDLED_AXES_PAIR } joy_axes_pairs;
#endif


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
	bool operator()(Common::KeyState k1, Common::KeyState k2) const {
		if (k1.keycode == k2.keycode)
			return k1.flags < k2.flags;
		else
			return k1.keycode < k2.keycode;
	}
};

typedef std::map<Common::KeyState, ActionType, ltSDLkeysym>   KeyMap;

class Configuration;

char get_ascii_char_from_keysym(Common::KeyState keysym);

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
	void AddKeyBinding(Common::KeyCode sym, int mod, const Action *action,
	                   int nparams, int *params);

	/* Delete keybinding */
//	void DelKeyBinding(Common::KeyCode sym, int mod); // unused

	/* Other methods */
	void Flush() {
		bindings.clear();
		keyhelp.clear();
		cheathelp.clear();
	}
	ActionType get_ActionType(Common::KeyState key);
	ActionKeyType GetActionKeyType(ActionType a);
	bool DoAction(ActionType const &a) const;
	KeyMap::iterator get_sdlkey_index(Common::KeyState key);
	bool HandleEvent(const Common::Event *event);

	void LoadFromFile(const char *filename);
	void LoadGameSpecificKeys();
	void LoadFromPatch();
	void handle_wrong_key_pressed();
	bool handle_always_available_keys(ActionType a);

	void ShowKeys() const;
#ifdef HAVE_JOYSTICK_SUPPORT
	uint8 get_axis(uint8 index);
	void set_axis(uint8 index, uint8 value);
	Common::KeyCode get_key_from_joy_walk_axes() {
		return get_key_from_joy_axis_motion(x_axis, true);
	}
	Common::KeyCode get_key_from_joy_axis_motion(int axis, bool repeating);
	Common::KeyCode get_key_from_joy_hat_button(uint8 hat_button);
	Common::KeyCode get_key_from_joy_events(Common::Event *event);
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
	Common::KeyCode get_key_from_joy_button(uint8 button);
	Common::KeyCode get_key_from_joy_hat(SDL_JoyHatEvent);
#endif
};

#ifdef HAVE_JOYSTICK_SUPPORT

static const Common::KeyCode FIRST_JOY = (Common::KeyCode)400;
const Common::KeyCode JOY_UP            = FIRST_JOY;               // PS d-pad when analog is disabled. left stick when enabled
const Common::KeyCode JOY_DOWN          = (Common::KeyCode)(FIRST_JOY + 1);
const Common::KeyCode JOY_LEFT          = (Common::KeyCode)(FIRST_JOY + 2);
const Common::KeyCode JOY_RIGHT         = (Common::KeyCode)(FIRST_JOY + 3);
const Common::KeyCode JOY_RIGHTUP       = (Common::KeyCode)(FIRST_JOY + 4);
const Common::KeyCode JOY_RIGHTDOWN     = (Common::KeyCode)(FIRST_JOY + 5);
const Common::KeyCode JOY_LEFTUP        = (Common::KeyCode)(FIRST_JOY + 6);
const Common::KeyCode JOY_LEFTDOWN      = (Common::KeyCode)(FIRST_JOY + 7);
const Common::KeyCode JOY_UP2           = (Common::KeyCode)(FIRST_JOY + 8); // PS right stick when analog is enabled
const Common::KeyCode JOY_DOWN2         = (Common::KeyCode)(FIRST_JOY + 9);
const Common::KeyCode JOY_LEFT2         = (Common::KeyCode)(FIRST_JOY + 10);
const Common::KeyCode JOY_RIGHT2        = (Common::KeyCode)(FIRST_JOY + 11);
const Common::KeyCode JOY_RIGHTUP2      = (Common::KeyCode)(FIRST_JOY + 12);
const Common::KeyCode JOY_RIGHTDOWN2    = (Common::KeyCode)(FIRST_JOY + 13);
const Common::KeyCode JOY_LEFTUP2       = (Common::KeyCode)(FIRST_JOY + 14);
const Common::KeyCode JOY_LEFTDOWN2     = (Common::KeyCode)(FIRST_JOY + 15);
const Common::KeyCode JOY_UP3           = (Common::KeyCode)(FIRST_JOY + 16);
const Common::KeyCode JOY_DOWN3         = (Common::KeyCode)(FIRST_JOY + 17);
const Common::KeyCode JOY_LEFT3         = (Common::KeyCode)(FIRST_JOY + 18);
const Common::KeyCode JOY_RIGHT3        = (Common::KeyCode)(FIRST_JOY + 19);
const Common::KeyCode JOY_RIGHTUP3      = (Common::KeyCode)(FIRST_JOY + 20);
const Common::KeyCode JOY_RIGHTDOWN3    = (Common::KeyCode)(FIRST_JOY + 21);
const Common::KeyCode JOY_LEFTUP3       = (Common::KeyCode)(FIRST_JOY + 22);
const Common::KeyCode JOY_LEFTDOWN3     = (Common::KeyCode)(FIRST_JOY + 23);
const Common::KeyCode JOY_UP4           = (Common::KeyCode)(FIRST_JOY + 24);
const Common::KeyCode JOY_DOWN4         = (Common::KeyCode)(FIRST_JOY + 25);
const Common::KeyCode JOY_LEFT4         = (Common::KeyCode)(FIRST_JOY + 26);
const Common::KeyCode JOY_RIGHT4        = (Common::KeyCode)(FIRST_JOY + 27);
const Common::KeyCode JOY_RIGHTUP4      = (Common::KeyCode)(FIRST_JOY + 28);
const Common::KeyCode JOY_RIGHTDOWN4    = (Common::KeyCode)(FIRST_JOY + 29);
const Common::KeyCode JOY_LEFTUP4       = (Common::KeyCode)(FIRST_JOY + 30);
const Common::KeyCode JOY_LEFTDOWN4     = (Common::KeyCode)(FIRST_JOY + 31);
const Common::KeyCode JOY_HAT_UP        = (Common::KeyCode)(FIRST_JOY + 32); // PS d-pad when analog is enabled
const Common::KeyCode JOY_HAT_DOWN      = (Common::KeyCode)(FIRST_JOY + 33);
const Common::KeyCode JOY_HAT_LEFT      = (Common::KeyCode)(FIRST_JOY + 34);
const Common::KeyCode JOY_HAT_RIGHT     = (Common::KeyCode)(FIRST_JOY + 35);
const Common::KeyCode JOY_HAT_RIGHTUP   = (Common::KeyCode)(FIRST_JOY + 36);
const Common::KeyCode JOY_HAT_RIGHTDOWN = (Common::KeyCode)(FIRST_JOY + 37);
const Common::KeyCode JOY_HAT_LEFTUP    = (Common::KeyCode)(FIRST_JOY + 38);
const Common::KeyCode JOY_HAT_LEFTDOWN  = (Common::KeyCode)(FIRST_JOY + 39);
const Common::KeyCode JOY0              = (Common::KeyCode)(FIRST_JOY + 40); // PS triangle
const Common::KeyCode JOY1              = (Common::KeyCode)(FIRST_JOY + 41); // PS circle
const Common::KeyCode JOY2              = (Common::KeyCode)(FIRST_JOY + 42); // PS x
const Common::KeyCode JOY3              = (Common::KeyCode)(FIRST_JOY + 43); // PS square
const Common::KeyCode JOY4              = (Common::KeyCode)(FIRST_JOY + 44); // PS L2
const Common::KeyCode JOY5              = (Common::KeyCode)(FIRST_JOY + 45); // PS R2
const Common::KeyCode JOY6              = (Common::KeyCode)(FIRST_JOY + 46); // PS L1
const Common::KeyCode JOY7              = (Common::KeyCode)(FIRST_JOY + 47); // PS R1
const Common::KeyCode JOY8              = (Common::KeyCode)(FIRST_JOY + 48); // PS select
const Common::KeyCode JOY9              = (Common::KeyCode)(FIRST_JOY + 49); // PS start
const Common::KeyCode JOY10             = (Common::KeyCode)(FIRST_JOY + 50); // PS L3 (analog must be enabled)
const Common::KeyCode JOY11             = (Common::KeyCode)(FIRST_JOY + 51); // PS R3 (analog must be enabled)
const Common::KeyCode JOY12             = (Common::KeyCode)(FIRST_JOY + 52);
const Common::KeyCode JOY13             = (Common::KeyCode)(FIRST_JOY + 53);
const Common::KeyCode JOY14             = (Common::KeyCode)(FIRST_JOY + 54);
const Common::KeyCode JOY15             = (Common::KeyCode)(FIRST_JOY + 55);
const Common::KeyCode JOY16             = (Common::KeyCode)(FIRST_JOY + 56);
const Common::KeyCode JOY17             = (Common::KeyCode)(FIRST_JOY + 57);
const Common::KeyCode JOY18             = (Common::KeyCode)(FIRST_JOY + 58);
const Common::KeyCode JOY19             = (Common::KeyCode)(FIRST_JOY + 59);
#endif /* HAVE_JOYSTICK_SUPPORT */

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
