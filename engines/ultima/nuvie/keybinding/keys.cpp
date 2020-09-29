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

#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/keybinding/key_actions.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/files/utils.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/shared/conf/xml_tree.h"
#include "common/hash-str.h"

#define ENCODE_KEY(key, mod) ((uint32)(key) | ((uint32)(mod) << 24))

namespace Ultima {
namespace Nuvie {

static  class Chardata { // ctype-like character lists
public:
	string  whitespace;
	Chardata() {
		for (size_t i = 0; i < 256; i++)
			if (Common::isSpace(i))
				whitespace += static_cast<char>(i);
	}
} chardata;

typedef void(*ActionFunc)(int const *);

struct Action {
	const char *s;
	ActionFunc func; // called on keydown
	const char *desc;
	enum {
		dont_show = 0,
		normal_keys,
		cheat_keys
	} key_type;
	bool allow_in_vehicle;
	ActionKeyType action_key_type;
};

const Action NuvieActions[] = {
	{ "WALK_WEST", ActionWalkWest, "Walk west", Action::normal_keys, true, WEST_KEY  },
	{ "WALK_EAST", ActionWalkEast, "Walk east", Action::normal_keys, true, EAST_KEY },
	{ "WALK_NORTH", ActionWalkNorth, "Walk north", Action::normal_keys, true, NORTH_KEY },
	{ "WALK_SOUTH", ActionWalkSouth, "Walk south", Action::normal_keys, true, SOUTH_KEY },
	{ "WALK_NORTH_EAST", ActionWalkNorthEast, "Walk north-east", Action::normal_keys, true, NORTH_EAST_KEY },
	{ "WALK_SOUTH_EAST", ActionWalkSouthEast, "Walk south-east", Action::normal_keys, true, SOUTH_EAST_KEY },
	{ "WALK_NORTH_WEST", ActionWalkNorthWest, "Walk north-west", Action::normal_keys, true, NORTH_WEST_KEY },
	{ "WALK_SOUTH_WEST", ActionWalkSouthWest, "Walk south-west", Action::normal_keys, true, SOUTH_WEST_KEY },
	{ "CAST", ActionCast, "Cast", Action::normal_keys, true, OTHER_KEY }, // allow_in_vehicle is true, so the right message or cancel is done for WOU games
	{ "LOOK", ActionLook, "Look", Action::normal_keys, true, OTHER_KEY },
	{ "TALK", ActionTalk, "Talk", Action::normal_keys, true, OTHER_KEY },
	{ "USE", ActionUse, "Use", Action::normal_keys, true, OTHER_KEY },
	{ "GET", ActionGet, "Get", Action::normal_keys, false, OTHER_KEY },
	{ "MOVE", ActionMove, "Move", Action::normal_keys, false, OTHER_KEY },
	{ "DROP", ActionDrop, "Drop", Action::normal_keys, false, OTHER_KEY },
	{ "TOGGLE_COMBAT", ActionToggleCombat, "Toggle combat", Action::normal_keys, false, OTHER_KEY },
	{ "ATTACK", ActionAttack, "Attack", Action::normal_keys, true, OTHER_KEY },
	{ "REST", ActionRest, "Rest", Action::normal_keys, true, OTHER_KEY },
	{ "MULTI_USE", ActionMultiUse, "Multi-use", Action::normal_keys, true, OTHER_KEY },
	{ "SELECT_COMMAND_BAR", ActionSelectCommandBar, "Select Command Bar", Action::normal_keys, true, OTHER_KEY },
	{ "NEW_COMMAND_BAR", ActionSelectNewCommandBar, "Select New Command Bar", Action::normal_keys, true, NEW_COMMAND_BAR_KEY },
	{ "DOLL_GUMP", ActionDollGump, "Doll gump", Action::normal_keys, true, OTHER_KEY },
	{ "SHOW_STATS", ActionShowStats, "Shows the party member's stats", Action::normal_keys, true, OTHER_KEY },
	{ "INVENTORY", ActionInventory, "inventory", Action::normal_keys, true, OTHER_KEY },
	{ "PREVIOUS_PARTY_MEMBER", ActionPreviousPartyMember, "Previous party member", Action::normal_keys, true, PREVIOUS_PARTY_MEMBER_KEY },
	{ "NEXT_PARTY_MEMBER", ActionNextPartyMember, "Next party member", Action::normal_keys, true, NEXT_PARTY_MEMBER_KEY },
	{ "HOME_KEY", ActionHome, "Home key", Action::normal_keys, true, HOME_KEY },
	{ "END_KEY", ActionEnd, "End key", Action::normal_keys, true, END_KEY },
	{ "TOGGLE_VIEW", ActionToggleView, "Toggle between inventory and actor view", Action::normal_keys, true, OTHER_KEY },
	{ "PARTY_VIEW", ActionPartyView, "Party view", Action::normal_keys, true, OTHER_KEY },
	{ "SOLO_MODE", ActionSoloMode, "Solo mode", Action::normal_keys, true, OTHER_KEY },
	{ "PARTY_MODE", ActionPartyMode, "Party mode", Action::normal_keys, true, OTHER_KEY },
	{ "SAVE_MENU", ActionSaveDialog, "Save menu", Action::normal_keys, true, OTHER_KEY },
	{ "LOAD_LATEST_SAVE", ActionLoadLatestSave, "Load latest save", Action::normal_keys, true, OTHER_KEY },
	{ "QUICK_SAVE", ActionQuickSave, "Quick save", Action::normal_keys, true, OTHER_KEY },
	{ "QUICK_LOAD", ActionQuickLoad, "Quick load", Action::normal_keys, true, OTHER_KEY },
	{ "QUIT", ActionQuitDialog, "Quit", Action::normal_keys, true, QUIT_KEY },
	{ "QUIT_NO_DIALOG", ActionQuitNODialog, "Quit without confirmation", Action::normal_keys, true, QUIT_KEY },
	{ "GAME_MENU_DIALOG", ActionGameMenuDialog, "Show game menu; Cancel action if in middle of action", Action::normal_keys, true, CANCEL_ACTION_KEY },
	{ "TOGGLE_FULLSCREEN", ActionToggleFullscreen, "Toggle Fullscreen", Action::normal_keys, true, TOGGLE_FULLSCREEN_KEY },
	{ "TOGGLE_CURSOR", ActionToggleCursor, "Toggle Cursor", Action::normal_keys, true, TOGGLE_CURSOR_KEY },
	{ "TOGGLE_COMBAT_STRATEGY", ActionToggleCombatStrategy, "Toggle combat strategy", Action::normal_keys, true, OTHER_KEY },
	{ "TOGGLE_FPS_DISPLAY", ActionToggleFps, "Toggle frames per second display", Action::normal_keys, true, TOGGLE_FPS_KEY },
	{ "TOGGLE_AUDIO", ActionToggleAudio, "Toggle audio", Action::normal_keys, true, TOGGLE_AUDIO_KEY },
	{ "TOGGLE_MUSIC", ActionToggleMusic, "Toggle music", Action::normal_keys, true, TOGGLE_MUSIC_KEY },
	{ "TOGGLE_SFX", ActionToggleSFX, "Toggle sfx", Action::normal_keys, true, TOGGLE_SFX_KEY },
	{ "TOGGLE_ORIGINAL_STYLE_COMMAND_BAR", ActionToggleOriginalStyleCommandBar, "Show/hide original style command bar", Action::normal_keys, true, OTHER_KEY },
	{ "DO_ACTION", ActionDoAction, "Do action", Action::normal_keys, true, DO_ACTION_KEY },
	{ "CANCEL_ACTION", ActionCancelAction, "Cancel action", Action::normal_keys, true, CANCEL_ACTION_KEY },
	{ "MSG_SCROLL_UP", ActionMsgScrollUP, "Msg scroll up", Action::normal_keys, true, MSGSCROLL_UP_KEY },
	{ "MSG_SCROLL_DOWN", ActionMsgScrollDown, "Msg scroll down", Action::normal_keys, true, MSGSCROLL_DOWN_KEY },
	{ "SHOW_KEYS", ActionShowKeys, "Show keys", Action::normal_keys, true, OTHER_KEY },
	{ "DECREASE_DEBUG", ActionDecreaseDebug, "Decrease debug", Action::normal_keys, true, DECREASE_DEBUG_KEY },
	{ "INCREASE_DEBUG", ActionIncreaseDebug, "Increase debug", Action::normal_keys, true, INCREASE_DEBUG_KEY },
	{ "CLOSE_GUMPS", ActionCloseGumps, "Close gumps", Action::normal_keys, true, OTHER_KEY },
	{ "USE_ITEM", ActionUseItem, "Use item", Action::normal_keys, true, OTHER_KEY },
	{ "SHOW_EGGS", ActionShowEggs, "Show eggs", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_HACKMOVE", ActionToggleHackmove, "Toggle hack move", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_EGG_SPAWN", ActionToggleEggSpawn, "Toggle egg spawn", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_UNLIMITED_CASTING", ActionToggleUnlimitedCasting, "Toggle unlimited casting", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_NO_DARKNESS", ActionToggleNoDarkness, "Toggle no darkness", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_PICKPOCKET_MODE", ActionTogglePickpocket, "Toggle pickpocket mode", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_GOD_MODE",  ActionToggleGodMode, "Toggle god mode", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_ETHEREAL",  ActionToggleEthereal, "Toggle ethereal mode", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_X_RAY",  ActionToggleX_Ray, "Toggle X-ray mode", Action::cheat_keys, true, OTHER_KEY },
	{ "HEAL_PARTY", ActionHealParty, "Heal party", Action::cheat_keys, true, OTHER_KEY },
	{ "TELEPORT_TO_CURSOR", ActionTeleportToCursor, "Teleport to cursor", Action::cheat_keys, true, OTHER_KEY },
	{ "TOGGLE_CHEATS", ActionToggleCheats, "Toggle cheats", Action::normal_keys, true, OTHER_KEY },
	{ "DO_NOTHING", ActionDoNothing, "", Action::dont_show, true, OTHER_KEY },
	{ "", 0, "", Action::dont_show, false, OTHER_KEY } //terminator
};

struct KeycodeString {
	const char *s;
	Common::KeyCode k;
};
const KeycodeString StringTable[] = {
	{"LCTRL",     Common::KEYCODE_LCTRL},
	{"RCTRL",     Common::KEYCODE_RCTRL},
	{"LALT",      Common::KEYCODE_LALT},
	{"RALT",      Common::KEYCODE_RALT},
	{"LSHIFT",    Common::KEYCODE_LSHIFT},
	{"RSHIFT",    Common::KEYCODE_RSHIFT},
	{"BACKSPACE", Common::KEYCODE_BACKSPACE},
	{"TAB",       Common::KEYCODE_TAB},
	{"ENTER",     Common::KEYCODE_RETURN},
	{"PAUSE",     Common::KEYCODE_PAUSE},
	{"ESC",       Common::KEYCODE_ESCAPE},
	{"SPACE",     Common::KEYCODE_SPACE},
	{"DEL",       Common::KEYCODE_DELETE},
	{"KP0",       Common::KEYCODE_KP0},
	{"KP1",       Common::KEYCODE_KP1},
	{"KP2",       Common::KEYCODE_KP2},
	{"KP3",       Common::KEYCODE_KP3},
	{"KP4",       Common::KEYCODE_KP4},
	{"KP5",       Common::KEYCODE_KP5},
	{"KP6",       Common::KEYCODE_KP6},
	{"KP7",       Common::KEYCODE_KP7},
	{"KP8",       Common::KEYCODE_KP8},
	{"KP9",       Common::KEYCODE_KP9},
	{"KP.",       Common::KEYCODE_KP_PERIOD},
	{"KP/",       Common::KEYCODE_KP_DIVIDE},
	{"KP*",       Common::KEYCODE_KP_MULTIPLY},
	{"KP-",       Common::KEYCODE_KP_MINUS},
	{"KP+",       Common::KEYCODE_KP_PLUS},
	{"KP_ENTER",  Common::KEYCODE_KP_ENTER},
	{"UP",        Common::KEYCODE_UP},
	{"DOWN",      Common::KEYCODE_DOWN},
	{"RIGHT",     Common::KEYCODE_RIGHT},
	{"LEFT",      Common::KEYCODE_LEFT},
	{"INSERT",    Common::KEYCODE_INSERT},
	{"HOME",      Common::KEYCODE_HOME},
	{"END",       Common::KEYCODE_END},
	{"PAGEUP",    Common::KEYCODE_PAGEUP},
	{"PAGEDOWN",  Common::KEYCODE_PAGEDOWN},
	{"F1",        Common::KEYCODE_F1},
	{"F2",        Common::KEYCODE_F2},
	{"F3",        Common::KEYCODE_F3},
	{"F4",        Common::KEYCODE_F4},
	{"F5",        Common::KEYCODE_F5},
	{"F6",        Common::KEYCODE_F6},
	{"F7",        Common::KEYCODE_F7},
	{"F8",        Common::KEYCODE_F8},
	{"F9",        Common::KEYCODE_F9},
	{"F10",       Common::KEYCODE_F10},
	{"F11",       Common::KEYCODE_F11},
	{"F12",       Common::KEYCODE_F12},
	{"F13",       Common::KEYCODE_F13},
	{"F14",       Common::KEYCODE_F14},
	{"F15",       Common::KEYCODE_F15},
	// hackishly map joystick to unused keycode values
	{"JOY_UP",            JOY_UP},
	{"JOY_DOWN",          JOY_DOWN},
	{"JOY_LEFT",          JOY_LEFT},
	{"JOY_RIGHT",         JOY_RIGHT},
	{"JOY_RIGHTUP",       JOY_RIGHTUP},
	{"JOY_RIGHTDOWN",     JOY_RIGHTDOWN},
	{"JOY_LEFTUP",        JOY_LEFTUP},
	{"JOY_LEFTDOWN",      JOY_LEFTDOWN},
	{"JOY_UP2",           JOY_UP2},
	{"JOY_DOWN2",         JOY_DOWN2},
	{"JOY_LEFT2",         JOY_LEFT2},
	{"JOY_RIGHT2",        JOY_RIGHT2},
	{"JOY_RIGHTUP2",      JOY_RIGHTUP2},
	{"JOY_RIGHTDOWN2",    JOY_RIGHTDOWN2},
	{"JOY_LEFTUP2",       JOY_LEFTUP2},
	{"JOY_LEFTDOWN2",     JOY_LEFTDOWN2},
	{"JOY_UP3",           JOY_UP3},
	{"JOY_DOWN3",         JOY_DOWN3},
	{"JOY_LEFT3",         JOY_LEFT3},
	{"JOY_RIGHT3",        JOY_RIGHT3},
	{"JOY_RIGHTUP3",      JOY_RIGHTUP3},
	{"JOY_RIGHTDOWN3",    JOY_RIGHTDOWN3},
	{"JOY_LEFTUP3",       JOY_LEFTUP3},
	{"JOY_LEFTDOWN3",     JOY_LEFTDOWN3},
	{"JOY_UP4",           JOY_UP4},
	{"JOY_DOWN4",         JOY_DOWN4},
	{"JOY_LEFT4",         JOY_LEFT4},
	{"JOY_RIGHT4",        JOY_RIGHT4},
	{"JOY_RIGHTUP4",      JOY_RIGHTUP4},
	{"JOY_RIGHTDOWN4",    JOY_RIGHTDOWN4},
	{"JOY_LEFTUP4",       JOY_LEFTUP4},
	{"JOY_LEFTDOWN4",     JOY_LEFTDOWN4},
	{"JOY_HAT_UP",        JOY_HAT_UP},
	{"JOY_HAT_DOWN",      JOY_HAT_DOWN},
	{"JOY_HAT_LEFT",      JOY_HAT_LEFT},
	{"JOY_HAT_RIGHT",     JOY_HAT_RIGHT},
	{"JOY_HAT_RIGHTUP",   JOY_HAT_RIGHTUP},
	{"JOY_HAT_RIGHTDOWN", JOY_HAT_RIGHTDOWN},
	{"JOY_HAT_LEFTUP",    JOY_HAT_LEFTUP},
	{"JOY_HAT_LEFTDOWN",  JOY_HAT_LEFTDOWN},
	{"JOY0",              JOY0},
	{"JOY1",              JOY1},
	{"JOY2",              JOY2},
	{"JOY3",              JOY3},
	{"JOY4",              JOY4},
	{"JOY5",              JOY5},
	{"JOY6",              JOY6},
	{"JOY7",              JOY7},
	{"JOY8",              JOY8},
	{"JOY9",              JOY9},
	{"JOY10",             JOY10},
	{"JOY11",             JOY11},
	{"JOY12",             JOY12},
	{"JOY13",             JOY13},
	{"JOY14",             JOY14},
	{"JOY15",             JOY15},
	{"JOY16",             JOY16},
	{"JOY17",             JOY17},
	{"JOY18",             JOY18},
	{"JOY19",             JOY19},
	{"", Common::KEYCODE_INVALID} // terminator
};

const Action doNothingAction = { "DO_NOTHING", ActionDoNothing, "", Action::dont_show, true, OTHER_KEY };

KeyBinder::KeyBinder(Configuration *config) {
	FillParseMaps();

	Std::string keyfilename, dir;
	config->value("config/keys", keyfilename, "(default)");
	bool key_file_exists = fileExists(keyfilename.c_str());

	if (keyfilename != "(default)" && !key_file_exists)
		::error("Couldn't find the default key setting at %s - trying defaultkeys.txt in the data directory\n", keyfilename.c_str());
	if (keyfilename == "(default)" || !key_file_exists) {
		config->value("config/datadir", dir, "./data");
		keyfilename = dir + "/defaultkeys.txt";
	}
	LoadFromFile(keyfilename.c_str());

	LoadGameSpecificKeys(); // won't load if file isn't found
	LoadFromPatch(); // won't load if file isn't found

	int config_int;
	uint16 max_delay = 10000; // 10 seconds but means no repeat
	config->value("config/joystick/repeat_hat", repeat_hat, false);

	config->value("config/joystick/repeat_delay", config_int, 50);
	joy_repeat_delay = config_int < max_delay ? config_int : max_delay;
	if (joy_repeat_delay == max_delay)
		joy_repeat_enabled = false;
	else
		joy_repeat_enabled = true;

	Common::fill(_joyAxisPositions, _joyAxisPositions + 8, 0);

	// AXES_PAIR1
	config->value("config/joystick/axes_pair1/x_axis", config_int, 0);
	x_axis = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair1/y_axis", config_int, 1);
	y_axis = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair1/delay", config_int, 110);
	pair1_delay = config_int < max_delay ? config_int : max_delay;
	// AXES_PAIR2
	config->value("config/joystick/axes_pair2/x_axis", config_int, 3);
	x_axis2 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair2/y_axis", config_int, 2);
	y_axis2 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair2/delay", config_int, 110);
	pair2_delay = config_int < max_delay ? config_int : max_delay;
	// AXES_PAIR3
	config->value("config/joystick/axes_pair3/x_axis", config_int, 4);
	x_axis3 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair3/y_axis", config_int, 5);
	y_axis3 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair3/delay", config_int, 110);
	pair3_delay = config_int < max_delay ? config_int : max_delay;
	// AXES_PAIR4
	config->value("config/joystick/axes_pair4/x_axis", config_int, 6);
	x_axis4 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair4/y_axis", config_int, 7);
	y_axis4 = config_int < 255 ? config_int : 255;
	config->value("config/joystick/axes_pair4/delay", config_int, 110);
	pair4_delay = config_int < max_delay ? config_int : max_delay;

	next_axes_pair_update = next_axes_pair2_update = next_axes_pair3_update = 0;
	next_axes_pair4_update = next_joy_repeat_time = 0;
}

KeyBinder::~KeyBinder() {
}

void KeyBinder::AddKeyBinding(Common::KeyCode key, byte mod, const Action *action,
                              int nparams, int *params) {
	Common::KeyState k;
	ActionType a;
	a.action = action;

	int i;  // For MSVC
	for (i = 0; i < c_maxparams && i < nparams; i++)
		a.params[i] = params[i];
	for (i = nparams; i < c_maxparams; i++)
		a.params[i] = -1;

	_bindings[ENCODE_KEY(key, mod)] = a;
}

ActionType KeyBinder::get_ActionType(const Common::KeyState &key) {
	KeyMap::iterator sdlkey_index = get_sdlkey_index(key);
	if (sdlkey_index == _bindings.end()) {
		ActionType actionType = {&doNothingAction, {0}};
		return actionType;
	}
	return (*sdlkey_index)._value;
}

ActionKeyType KeyBinder::GetActionKeyType(ActionType a) {
	return a.action->action_key_type;
}

bool KeyBinder::DoAction(ActionType const &a) const {
	if (!a.action->allow_in_vehicle && Game::get_game()->get_player()->is_in_vehicle() && Game::get_game()->get_game_type() != NUVIE_GAME_MD) {
		Game::get_game()->get_event()->display_not_aboard_vehicle();
		return true;
	} else if (a.action->key_type == Action::cheat_keys && !Game::get_game()->are_cheats_enabled()) {
		new TextEffect("Cheats are disabled");
		return true;
	}
	a.action->func(a.params);

	return true;
}

KeyMap::iterator KeyBinder::get_sdlkey_index(const Common::KeyState &key) {
	return _bindings.find(ENCODE_KEY(key.keycode, key.flags));
}

bool KeyBinder::HandleEvent(const Common::Event *ev) {
	Common::KeyState key = ev->kbd.keycode;
	KeyMap::iterator sdlkey_index;

	if (ev->type != Common::EVENT_KEYDOWN)
		return false;

	sdlkey_index = get_sdlkey_index(key);
	if (sdlkey_index != _bindings.end())
		return DoAction((*sdlkey_index)._value);

	if (ev->kbd.keycode != Common::KEYCODE_LALT && ev->kbd.keycode != Common::KEYCODE_RALT
	        && ev->kbd.keycode != Common::KEYCODE_LCTRL && ev->kbd.keycode != Common::KEYCODE_RCTRL) {
		handle_wrong_key_pressed();
	}

	return false;
}

void KeyBinder::handle_wrong_key_pressed() {
	if (Game::get_game()->get_event()->get_mode() != MOVE_MODE)
		Game::get_game()->get_event()->cancelAction();
	else {
		Game::get_game()->get_scroll()->display_string("what?\n\n");
		Game::get_game()->get_scroll()->display_prompt();
	}
}

bool KeyBinder::handle_always_available_keys(ActionType a) {
	switch (a.action->action_key_type) {
	case TOGGLE_AUDIO_KEY: // FIXME - Shutting off the music in cutscenes will not have any music play when
	case TOGGLE_MUSIC_KEY: //         you toggle it back on. It has to wait for the engine to play another song
	case TOGGLE_SFX_KEY:
	case TOGGLE_FPS_KEY: // fps is not available in intro or when viewing ending with command line cheat
	case TOGGLE_FULLSCREEN_KEY:
	case DECREASE_DEBUG_KEY:
	case INCREASE_DEBUG_KEY:
	case QUIT_KEY: // FIXME - doesn't currently work in intro or when viewing ending with command line cheat
		a.action->func(a.params);
		return true;
	default:
		return false;
	}
}

void KeyBinder::ShowKeys() const { // FIXME This doesn't look very good, the font is missing
	// some characters, and it is longer than msgscroll can hold
//	if(Game::get_game()->is_orig_style())
	{
		Std::vector<string>::const_iterator iter;
		string keysStr;
		MsgScroll *scroll = Game::get_game()->get_scroll();
		scroll->set_autobreak(true);

		for (iter = _keyHelp.begin(); iter != _keyHelp.end(); ++iter) {
			keysStr = "\n";
			keysStr.append(iter->c_str());
			scroll->display_string(keysStr, 1);
		}
		scroll->message("\n\n\t");
	}
}

void KeyBinder::ParseText(char *text, int len) {
	char *ptr, *end;
	const char LF = '\n';

	ptr = text;

	// last (useful) line must end with LF
	while ((ptr - text) < len && (end = strchr(ptr, LF)) != 0) {
		*end = '\0';
		ParseLine(ptr);
		ptr = end + 1;
	}
}

static void skipspace(string &s) {
	size_t i = s.findFirstNotOf(chardata.whitespace);
	if (i && i != string::npos)
		s.erase(0, i);
}


void KeyBinder::ParseLine(char *line) {
	size_t i;
	Common::KeyState k;
	ActionType a;
	k.keycode = Common::KEYCODE_INVALID;
	k.flags = 0;
	string s = line, u;
	string d, desc, keycode;
	bool show;

	skipspace(s);

	// comments and empty lines
	if (s.empty() || s.hasPrefix("#"))
		return;

	u = s;
	u = Std::to_uppercase(u);

	// get key
	while (!s.empty() && !Common::isSpace(s[0])) {
		// check modifiers
		if (u.hasPrefix("ALT-")) {
			k.flags = k.flags | Common::KBD_ALT;
			s.erase(0, 4);
			u.erase(0, 4);
		} else if (u.hasPrefix("CTRL-")) {
			k.flags = k.flags | Common::KBD_CTRL;
			s.erase(0, 5);
			u.erase(0, 5);
		} else if (u.hasPrefix("SHIFT-")) {
			k.flags = k.flags | Common::KBD_SHIFT;
			s.erase(0, 6);
			u.erase(0, 6);
		} else {
			i = s.findFirstOf(chardata.whitespace);

			keycode = s.substr(0, i);
			s.erase(0, i);
			string t = Std::to_uppercase(keycode);

			if (t.empty()) {
				::error("Keybinder: parse error in line: %s", s.c_str());
			} else if (t.length() == 1) {
				// translate 1-letter keys straight to Common::KeyCode
				char c = t[0];
				if (c >= 33 && c <= 122 && c != 37) {
					if (c >= 'A' && c <= 'Z')
						c += 32; // need lowercase
					k.keycode = static_cast<Common::KeyCode>(c);
				} else {
					::error("Keybinder: unsupported key: %s", keycode.c_str());
				}
			} else {
				// lookup in table
				ParseKeyMap::iterator key_index;
				key_index = _keys.find(t);
				if (key_index != _keys.end()) {
					k.keycode = (*key_index)._value;
				} else {
					::error("Keybinder: unsupported key: %s", keycode.c_str());
				}
			}
		}
	}

	if (k.keycode == Common::KEYCODE_INVALID) {
		::error("Keybinder: parse error in line: %s", s.c_str());
	}

	// get function
	skipspace(s);

	i = s.findFirstOf(chardata.whitespace);
	string t = s.substr(0, i);
	s.erase(0, i);
	t = Std::to_uppercase(t);

	ParseActionMap::iterator action_index;
	action_index = _actions.find(t);
	if (action_index != _actions.end()) {
		a.action = (const Action *)(*action_index)._value;
	} else {
		::error("Keybinder: unsupported action: %s", t.c_str());
	}

	// get params
	skipspace(s);

	int np = 0;
	while (!s.empty() && s[0] != '#' && np < c_maxparams) {
		i = s.findFirstOf(chardata.whitespace);
		string tmp = s.substr(0, i);
		s.erase(0, i);
		skipspace(s);

		int p = atoi(tmp.c_str());
		a.params[np++] = p;
	}

	// read optional help comment
	if (!s.empty() && s[0] == '#') {
		if (s.length() >= 2 && s[1] == '-') {
			show = false;
		} else {
			s.erase(0, 1);
			skipspace(s);
			d = s;
			show = true;
		}
	} else {
		d = a.action->desc;
		show = a.action->key_type != Action::dont_show;
	}

	if (show) {
		desc = "";
		if (k.flags & Common::KBD_CTRL)
			desc += "Ctrl-";
#if defined(MACOS) || defined(MACOSX)
		if (k.flags & Common::KBD_ALT)
			desc += "Cmd-";
#else
		if (k.flags & Common::KBD_ALT)
			desc += "Alt-";
#endif
		if (k.flags & Common::KBD_SHIFT)
			desc += "Shift-";
		if (keycode == "`")
			desc += "grave";
		else
			desc += keycode;
		desc += " - " + d;

		// add to help list
		if (a.action->key_type == Action::normal_keys)
			_keyHelp.push_back(desc);
		else if (a.action->key_type == Action::cheat_keys)
			_cheatHelp.push_back(desc);
	}

	// bind key
	AddKeyBinding(k.keycode, k.flags, a.action, np, a.params);
}

void KeyBinder::LoadFromFileInternal(const char *filename) {
	Common::ReadStream *keyfile;

	openFile(keyfile, filename);
	char temp[1024]; // 1024 should be long enough
	while (!keyfile->eos()) {
		strgets(temp, 1024, keyfile);
		if (strlen(temp) >= 1023) {
			::error("Keybinder: parse error: line too long. Skipping rest of file");
		}
		ParseLine(temp);
	}
	delete keyfile;
}

void KeyBinder::LoadFromFile(const char *filename) {

	Flush();

	ConsoleAddInfo("Loading keybindings from file %s", filename);
	LoadFromFileInternal(filename);
}

void KeyBinder::LoadGameSpecificKeys() {
	string key_path_str;
	string default_key_path;
	Configuration *config = Game::get_game()->get_config();
	config->value("config/datadir", default_key_path, "./data");
	nuvie_game_t game_type = get_game_type(config);

	if (game_type == NUVIE_GAME_U6)
		default_key_path += "/u6keys.txt";
	else if (game_type ==  NUVIE_GAME_MD)
		default_key_path += "/mdkeys.txt";
	else // SE
		default_key_path += "/sekeys.txt";

	config->value(config_get_game_key(config) + "/game_specific_keys", key_path_str, default_key_path.c_str());
	const char *key_path;
	if (key_path_str == "(default)")
		key_path = default_key_path.c_str();
	else
		key_path = key_path_str.c_str();
	if (fileExists(key_path)) {
		ConsoleAddInfo("Loading %s", key_path);
		LoadFromFileInternal(key_path);
	} else // These aren't critical so failing to load doesn't matter much
		ConsoleAddInfo("Couldn't find %s", key_path);
}

void KeyBinder::LoadFromPatch() { // FIXME default should probably be system specific
	string PATCH_KEYS;
	Configuration *config = Game::get_game()->get_config();

	config->value(config_get_game_key(config) + "/patch_keys", PATCH_KEYS, "./patchkeys.txt");
	if (fileExists(PATCH_KEYS.c_str())) {
		ConsoleAddInfo("Loading patch keybindings");
		LoadFromFileInternal(PATCH_KEYS.c_str());
	}
}

// codes used in keybindings-files. (use uppercase here)
void KeyBinder::FillParseMaps() {
	for (int i = 0; strlen(StringTable[i].s) > 0; i++)
		_keys[StringTable[i].s] = StringTable[i].k;

	for (int i = 0; strlen(NuvieActions[i].s) > 0; i++)
		_actions[NuvieActions[i].s] = &(NuvieActions[i]);
}

uint8 KeyBinder::get_axis(uint8 index) {
	switch (index) {
	case 0:
		return x_axis;
	case 1:
		return y_axis;
	case 2:
		return x_axis2;
	case 3:
		return y_axis2;
	case 4:
		return x_axis3;
	case 5:
		return y_axis3;
	case 6:
		return x_axis4;
	case 7:
	default:
		return y_axis4;
	}
}

void KeyBinder::set_axis(uint8 index, uint8 value) {
	switch (index) {
	case 0:
		x_axis = value;
		break;
	case 1:
		y_axis = value;
		break;
	case 2:
		x_axis2 = value;
		break;
	case 3:
		y_axis2 = value;
		break;
	case 4:
		x_axis3 = value;
		break;
	case 5:
		y_axis3 = value;
		break;
	case 6:
		x_axis4 = value;
		break;
	case 7:
	default:
		y_axis4 = value;
		break;
	}
}

joy_axes_pairs KeyBinder::get_axes_pair(int axis) {
	if (axis == x_axis || axis == y_axis)
		return AXES_PAIR1;
	else if (axis == x_axis2 || axis == y_axis2)
		return AXES_PAIR2;
	else if (axis == x_axis3 || axis == y_axis3)
		return AXES_PAIR3;
	else if (axis == x_axis4 || axis == y_axis4)
		return AXES_PAIR4;
	else
		return UNHANDLED_AXES_PAIR;
}

Common::KeyCode KeyBinder::get_key_from_joy_axis_motion(int axis, bool repeating) {
	joy_axes_pairs axes_pair =  get_axes_pair(axis);
	if (axes_pair == UNHANDLED_AXES_PAIR)
		return Common::KEYCODE_INVALID;

	sint8 xoff = 0;
	sint8 yoff = 0;
	int xaxis, yaxis;

	switch (axes_pair) {
	case AXES_PAIR1:
		xaxis = x_axis;
		yaxis = y_axis;
		break;
	case AXES_PAIR2:
		xaxis = x_axis2;
		yaxis = y_axis2;
		break;
	case AXES_PAIR3:
		xaxis = x_axis3;
		yaxis = y_axis3;
		break;
	case AXES_PAIR4:
		xaxis = x_axis4;
		yaxis = y_axis4;
		break;
	default:
		return Common::KEYCODE_INVALID; // shouldn't happen
	}

	if (xaxis != 255 && _joyAxisPositions[xaxis] != 0)
		xoff = _joyAxisPositions[xaxis] < 0 ? -1 : 1;
	if (yaxis != 255 && _joyAxisPositions[yaxis] != 0)
		yoff = _joyAxisPositions[yaxis] < 0 ? -1 : 1;

	uint8 dir = get_direction_code(xoff, yoff);
	if (axes_pair == AXES_PAIR1) {
		if (dir == NUVIE_DIR_NONE) {
			next_axes_pair_update = 0; // centered so okay to reset
			if (!repeat_hat)
				next_joy_repeat_time = SDL_GetTicks() + joy_repeat_delay;
			return Common::KEYCODE_INVALID;
		} else if ((repeating && next_joy_repeat_time > SDL_GetTicks()) || (!repeating && next_axes_pair_update > SDL_GetTicks())) // don't repeat too fast
			return Common::KEYCODE_INVALID;

		next_axes_pair_update = SDL_GetTicks() + pair1_delay;
		if (!repeat_hat)
			next_joy_repeat_time = SDL_GetTicks() + joy_repeat_delay;
		switch (dir) {
		case NUVIE_DIR_N:
			return JOY_UP;
		case NUVIE_DIR_E:
			return JOY_RIGHT;
		case NUVIE_DIR_S:
			return JOY_DOWN;
		case NUVIE_DIR_W:
			return JOY_LEFT;
		case NUVIE_DIR_NE:
			return JOY_RIGHTUP;
		case NUVIE_DIR_SE:
			return JOY_RIGHTDOWN;
		case NUVIE_DIR_SW:
			return JOY_LEFTDOWN;
		case NUVIE_DIR_NW:
			return JOY_LEFTUP;
		default:
			return Common::KEYCODE_INVALID; // shouldn't happen
		}
	} else if (axes_pair == AXES_PAIR2) {
		if (dir == NUVIE_DIR_NONE) {
			next_axes_pair2_update = 0; // centered so okay to reset
			return Common::KEYCODE_INVALID;
		} else if (next_axes_pair2_update > SDL_GetTicks()) // don't repeat too fast
			return Common::KEYCODE_INVALID;
		else
			next_axes_pair2_update = SDL_GetTicks() + pair2_delay;
		switch (dir) {
		case NUVIE_DIR_N:
			return JOY_UP2;
		case NUVIE_DIR_E:
			return JOY_RIGHT2;
		case NUVIE_DIR_S:
			return JOY_DOWN2;
		case NUVIE_DIR_W:
			return JOY_LEFT2;
		case NUVIE_DIR_NE:
			return JOY_RIGHTUP2;
		case NUVIE_DIR_SE:
			return JOY_RIGHTDOWN2;
		case NUVIE_DIR_SW:
			return JOY_LEFTDOWN2;
		case NUVIE_DIR_NW:
			return JOY_LEFTUP2;
		default:
			return Common::KEYCODE_INVALID; // shouldn't happen
		}
	} else if (axes_pair == AXES_PAIR3) {
		if (dir == NUVIE_DIR_NONE) {
			next_axes_pair3_update = 0; // centered so okay to reset
			return Common::KEYCODE_INVALID;
		} else if (next_axes_pair3_update > SDL_GetTicks()) // don't repeat too fast
			return Common::KEYCODE_INVALID;
		else
			next_axes_pair3_update = SDL_GetTicks() + pair3_delay;
		switch (dir) {
		case NUVIE_DIR_N:
			return JOY_UP3;
		case NUVIE_DIR_E:
			return JOY_RIGHT3;
		case NUVIE_DIR_S:
			return JOY_DOWN3;
		case NUVIE_DIR_W:
			return JOY_LEFT3;
		case NUVIE_DIR_NE:
			return JOY_RIGHTUP3;
		case NUVIE_DIR_SE:
			return JOY_RIGHTDOWN3;
		case NUVIE_DIR_SW:
			return JOY_LEFTDOWN3;
		case NUVIE_DIR_NW:
			return JOY_LEFTUP3;
		default:
			return Common::KEYCODE_INVALID; // shouldn't happen
		}
	} else { // AXES_PAIR4
		if (dir == NUVIE_DIR_NONE) {
			next_axes_pair4_update = 0; // centered so okay to reset
			return Common::KEYCODE_INVALID;
		} else if (next_axes_pair4_update > SDL_GetTicks()) // don't repeat too fast
			return Common::KEYCODE_INVALID;

		next_axes_pair4_update = SDL_GetTicks() + pair4_delay;
		switch (dir) {
		case NUVIE_DIR_N:
			return JOY_UP4;
		case NUVIE_DIR_E:
			return JOY_RIGHT4;
		case NUVIE_DIR_S:
			return JOY_DOWN4;
		case NUVIE_DIR_W:
			return JOY_LEFT4;
		case NUVIE_DIR_NE:
			return JOY_RIGHTUP4;
		case NUVIE_DIR_SE:
			return JOY_RIGHTDOWN4;
		case NUVIE_DIR_SW:
			return JOY_LEFTDOWN4;
		case NUVIE_DIR_NW:
			return JOY_LEFTUP4;
		default:
			return Common::KEYCODE_INVALID; // shouldn't happen
		}
	}
}

Common::KeyCode KeyBinder::get_key_from_joy_button(uint8 button) {
	switch (button) {
	case 0:
		return JOY0;
	case 1:
		return JOY1;
	case 2:
		return JOY2;
	case 3:
		return JOY3;
	case 4:
		return JOY4;
	case 5:
		return JOY5;
	case 6:
		return JOY6;
	case 7:
		return JOY7;
	case 8:
		return JOY8;
	case 9:
		return JOY9;
	case 10:
		return JOY10;
	case 11:
		return JOY11;
	case 12:
		return JOY12;
	case 13:
		return JOY13;
	case 14:
		return JOY14;
	case 15:
		return JOY15;
	case 16:
		return JOY16;
	case 17:
		return JOY17;
	case 18:
		return JOY18;
	case 19:
		return JOY19;
	default:
		return Common::KEYCODE_INVALID; // unhandled button
	}
}

Common::KeyCode KeyBinder::get_key_from_joy_events(Common::Event *event) {
	if (event->type == Common::EVENT_JOYBUTTON_UP) {
		return get_key_from_joy_button(event->joystick.button);
	} else if (event->type == Common::EVENT_JOYAXIS_MOTION && event->joystick.axis < 8) {
		_joyAxisPositions[event->joystick.axis] = event->joystick.position;
		return get_key_from_joy_axis_motion(event->joystick.axis, false);
	} else {
		return Common::KEYCODE_INVALID;
	}
}

char get_ascii_char_from_keysym(Common::KeyState keysym) {
	char ascii = 0;
	if (keysym.keycode < 128) {
		ascii = (char)keysym.keycode;
		if (ascii >= 97 && ascii <= 122 && keysym.flags & (Common::KBD_SHIFT | Common::KBD_CAPS)) {
			ascii -= 32;
		}
	}
	return ascii;
}

} // End of namespace Nuvie
} // End of namespace Ultima
