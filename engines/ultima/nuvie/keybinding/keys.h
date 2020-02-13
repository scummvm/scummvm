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

#ifndef NUVIE_KEYBINDING_KEYS_H
#define NUVIE_KEYBINDING_KEYS_H

#include "ultima/nuvie/keybinding/keys_enum.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "common/events.h"
#include "common/hash-str.h"

namespace Ultima {
namespace Nuvie {

enum joy_axes_pairs {
	AXES_PAIR1, AXES_PAIR2, AXES_PAIR3, AXES_PAIR4, UNHANDLED_AXES_PAIR
};

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

typedef Std::map<uint32, ActionType> KeyMap;

typedef Std::map<Common::String, Common::KeyCode> ParseKeyMap;
typedef Std::map<Common::String, const void *> ParseActionMap;

class Configuration;

char get_ascii_char_from_keysym(Common::KeyState keysym);

class KeyBinder {
private:
	KeyMap _bindings;

	Std::vector<Std::string> _keyHelp;
	Std::vector<Std::string> _cheatHelp;
	ParseKeyMap _keys;
	ParseActionMap _actions;
	int16 _joyAxisPositions[8];

	bool repeat_hat, joy_repeat_enabled; // repeat hat instead of axis when hat is found
	uint32 next_axes_pair_update, next_axes_pair2_update, next_axes_pair3_update,
		next_axes_pair4_update, next_joy_repeat_time;
	uint16 pair1_delay, pair2_delay, pair3_delay, pair4_delay, joy_repeat_delay;
	uint8 x_axis, y_axis, x_axis2, y_axis2, x_axis3, y_axis3, x_axis4, y_axis4;
	sint8 enable_joystick;

	void LoadFromFileInternal(const char *filename);
public:
	KeyBinder(Configuration *config);
	~KeyBinder();
	/* Add keybinding */
	void AddKeyBinding(Common::KeyCode sym, byte mod, const Action *action,
	                   int nparams, int *params);

	/* Delete keybinding */
//	void DelKeyBinding(Common::KeyCode sym, int mod); // unused

	/* Other methods */
	void Flush() {
		_bindings.clear();
		_keyHelp.clear();
		_cheatHelp.clear();
	}
	ActionType get_ActionType(const Common::KeyState &key);
	ActionKeyType GetActionKeyType(ActionType a);
	bool DoAction(ActionType const &a) const;
	KeyMap::iterator get_sdlkey_index(const Common::KeyState &key);
	bool HandleEvent(const Common::Event *event);

	void LoadFromFile(const char *filename);
	void LoadGameSpecificKeys();
	void LoadFromPatch();
	void handle_wrong_key_pressed();
	bool handle_always_available_keys(ActionType a);

	void ShowKeys() const;

	uint8 get_axis(uint8 index);
	void set_axis(uint8 index, uint8 value);
	Common::KeyCode get_key_from_joy_walk_axes() {
		return get_key_from_joy_axis_motion(x_axis, true);
	}
	Common::KeyCode get_key_from_joy_axis_motion(int axis, bool repeating);
	Common::KeyCode get_key_from_joy_hat_button(uint8 hat_button);
	Common::KeyCode get_key_from_joy_events(Common::Event *event);
	void init_joystick(sint8 joy_num);
//	SDL_Joystick *get_joystick() { return joystick; }
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

private:
	void ParseText(char *text, int len);
	void ParseLine(char *line);
	void FillParseMaps();

	joy_axes_pairs get_axes_pair(int axis);
	Common::KeyCode get_key_from_joy_button(uint8 button);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
