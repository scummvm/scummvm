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

#include "ultima/ultima4/meta_engine.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/ultima4.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"

namespace Ultima {
namespace Ultima4 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_method;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord KEYS[] = {
	{ KEYBIND_UP, "UP", "Up", "move up", "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", "Down", "move down", "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", "Left", "move left", "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", "Right", "move right", "RIGHT", nullptr },
	{ KEYBIND_ATTACK, "ATTACK", "Attack", "attack", "a", nullptr },
	{ KEYBIND_BOARD, "BOARD", "Board", "board", "b", nullptr },
	{ KEYBIND_CAST, "CAST", "Cast", "cast", "c", nullptr },
	{ KEYBIND_CLIMB, "CLIMB", "Climb", "climb", "k", nullptr },
	{ KEYBIND_DESCEND, "DESCEND", "Descend", "descend", "d", nullptr },
	{ KEYBIND_ENTER, "ENTER", "Enter", "enter", "e", nullptr },
	{ KEYBIND_EXIT, "EXIT", "Exit", "exit", "x", nullptr },
	{ KEYBIND_FIRE, "FIRE", "Fire", "fire", "f", nullptr },
	{ KEYBIND_GET, "GET", "Get Chest", "get", "g", nullptr },
	{ KEYBIND_HOLE_UP, "HOLE-UP", "Hole Up", "hole", "h", nullptr },
	{ KEYBIND_JIMMY, "JIMMY", "Jimmy", "jimmy", "j", nullptr },
	{ KEYBIND_IGNITE, "IGNITE", "Ignite", "ignite", "i", nullptr },
	{ KEYBIND_LOCATE, "LOCATE", "Locate Position", "locate", "l", nullptr },
	{ KEYBIND_MIX, "MIX", "Mix Reagents", "mix", "m", nullptr },
	{ KEYBIND_NEW_ORDER, "NEW-ORDER", "New Order", "order", "n", nullptr },
	{ KEYBIND_OPEN_DOOR, "OPEN-DOOR", "Open Door", "open", "o", nullptr },
	{ KEYBIND_PASS, "PASS", "Pass", "pass", "SPACE", nullptr },
	{ KEYBIND_PEER, "PEER", "Peer", "peer", "p", nullptr },
	{ KEYBIND_QUIT_SAVE, "QUIT-SAVE", "Quit and Save", "quitAndSave", "q", nullptr },
	{ KEYBIND_READY_WEAPON, "READY-WEAPON", "Ready Weapon", "ready", "r", nullptr },
	{ KEYBIND_SEARCH, "SEARCH", "Search", "search", "s", nullptr },
	{ KEYBIND_SPEED_UP, "SPEED-UP", "Speed Up", "speed up", "PLUS", nullptr },
	{ KEYBIND_SPEED_DOWN, "SPEED-DOWN", "Speed Down", "speed down", "MINUS", nullptr },
	{ KEYBIND_SPEED_NORMAL, "SPEED-NORMAL", "Speed Normal", "speed normal", "KP_ENTER", nullptr },
	{ KEYBIND_STATS, "STATS", "Stats", "stats", "z", nullptr },
	{ KEYBIND_TALK, "TALK", "Talk", "talk", "t", nullptr },
	{ KEYBIND_TOGGLE_MUSIC, "TOGGLE-MUSIC", "Toggle Music", "musicToggle", "v", nullptr },
	{ KEYBIND_USE, "USE", "Use", "use", "u", nullptr },
	{ KEYBIND_WEAR, "WEAR", "Wear Armor", "wear", "w", nullptr },
	{ KEYBIND_YELL, "YELL", "Yell", "yell", "y", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord PARTY_KEYS[] = {
	{ KEYBIND_PARTY0, "PARTY0", "Party - None", "party 0", "0", nullptr },
	{ KEYBIND_PARTY1, "PARTY1", "Party - Character #1", "party 1", "1", nullptr },
	{ KEYBIND_PARTY2, "PARTY2", "Party - Character #2", "party 2", "2", nullptr },
	{ KEYBIND_PARTY3, "PARTY3", "Party - Character #3", "party 3", "3", nullptr },
	{ KEYBIND_PARTY4, "PARTY4", "Party - Character #4", "party 4", "4", nullptr },
	{ KEYBIND_PARTY5, "PARTY5", "Party - Character #5", "party 5", "5", nullptr },
	{ KEYBIND_PARTY6, "PARTY6", "Party - Character #6", "party 6", "6", nullptr },
	{ KEYBIND_PARTY7, "PARTY7", "Party - Character #7", "party 7", "7", nullptr },
	{ KEYBIND_PARTY8, "PARTY8", "Party - Character #8", "party 8", "8", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};
static const KeysRecord KEYS_RECORDS[3] = {
	{ "ultima4", "Ultima IV", KEYS },
	{ "ultima4_party", "Ultima IV - Party", PARTY_KEYS },
	{ "ultima4_cheats", "Ultima IV - Cheats", CHEAT_KEYS },
};

Common::KeymapArray MetaEngine::initKeymaps() {
	Common::KeymapArray keymapArray;
	Common::Keymap *keyMap;
	Common::Action *act;

	for (int kCtr = 0; kCtr < 3; ++kCtr) {
		// Core keymaps
		keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame,
			KEYS_RECORDS[kCtr]._id, _(KEYS_RECORDS[kCtr]._desc));
		keymapArray.push_back(keyMap);

		if (kCtr == 0) {
			act = new Common::Action("LCLK", _("Interact via Left Click"));
			act->setLeftClickEvent();
			act->addDefaultInputMapping("MOUSE_LEFT");
			act->addDefaultInputMapping("JOY_A");
			keyMap->addAction(act);

			act = new Common::Action("RCLK", _("Interact via Right Click"));
			act->setRightClickEvent();
			act->addDefaultInputMapping("MOUSE_RIGHT");
			act->addDefaultInputMapping("JOY_B");
			keyMap->addAction(act);
		}

		for (const KeybindingRecord *r = KEYS_RECORDS[kCtr]._keys; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			keyMap->addAction(act);
		}
	}

	return keymapArray;
}

void MetaEngine::setKeybindingsActive(bool isActive) {
	g_engine->getEventManager()->getKeymapper()->setEnabled(isActive);
}


void MetaEngine::executeAction(KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

Common::String MetaEngine::getMethod(KeybindingAction keyAction) {
	for (int kCtr = 0; kCtr < 3; ++kCtr) {
		for (const KeybindingRecord *r = KEYS_RECORDS[kCtr]._keys; r->_id; ++r) {
			if (r->_action == keyAction)
				return r->_method;
		}
	}

	return Common::String();
}

} // End of namespace Ultima8
} // End of namespace Ultima
