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

static const KeybindingRecord NORMAL_KEYS[] = {
	{ KEYBIND_INTERACT, "INTERACT", "Interact", "interact", "RETURN", nullptr },
	{ KEYBIND_ESCAPE, "ESCAPE", "Abort Action", "", "ESCAPE", nullptr },
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
	{ KEYBIND_HOLE_UP, "HOLE-UP", "Hole Up & Camp", "camp", "h", nullptr },
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
	{ KEYBIND_STATS, "STATS", "Stats", "stats", "z", nullptr },
	{ KEYBIND_TALK, "TALK", "Talk", "talk", "t", nullptr },
	{ KEYBIND_USE, "USE", "Use", "use", "u", nullptr },
	{ KEYBIND_WEAR, "WEAR", "Wear Armor", "wear", "w", nullptr },
	{ KEYBIND_YELL, "YELL", "Yell", "yell", "y", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CONFIG_KEYS[] = {
	{ KEYBIND_SPEED_UP, "SPEED-UP", "Speed Up", "speed up", "KP_PLUS", nullptr },
	{ KEYBIND_SPEED_DOWN, "SPEED-DOWN", "Speed Down", "speed down", "KP_MINUS", nullptr },
	{ KEYBIND_SPEED_NORMAL, "SPEED-NORMAL", "Speed Normal", "speed normal", "KP_ENTER", nullptr },
	{ KEYBIND_COMBATSPEED_UP, "COMBATSPEED-UP", "Combat Speed Up", "combat_speed up", "A+KP_PLUS", nullptr },
	{ KEYBIND_COMBATSPEED_DOWN, "COMBATSPEED-DOWN", "Combat Speed Down", "combat_speed down", "A+KP_MINUS", nullptr },
	{ KEYBIND_COMBATSPEED_NORMAL, "COMBATSPEED-NORMAL", "Combat Speed Normal", "combat_speed normal", "A+KP_ENTER", nullptr },

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
	{ KEYBIND_CHEAT_DESTROY_CREATURES, "CHEAT-DESTROY_CREATURES", "Destroy All Creatures", "destroy_creatures", "A+a", nullptr },
	{ KEYBIND_CHEAT_COLLISIONS, "CHEAT-COLLISIONS", "Toggle Collision Handling", "collisions", "A+c", nullptr },
	{ KEYBIND_CHEAT_DESTROY, "CHEAT-DESTROY", "Destroy Object", "destroy", "A+d", nullptr },
	{ KEYBIND_CHEAT_EQUIPMENT, "CHEAT-EQUIPMENT", "Full Equipment", "equipment", "A+e", nullptr },
	{ KEYBIND_CHEAT_FLEE, "CHEAT_FLEE", "Flee Combat", "flee", "A+f", nullptr },
	{ KEYBIND_CHEAT_GOTO, "CHEAT-GOTO", "Goto location", "goto", "A+g", nullptr },
	{ KEYBIND_CHEAT_HELP, "CHEAT-HELP", "Help - Teleport to Lord British", "goto", "A+h", nullptr },
	{ KEYBIND_CHEAT_ITEMS, "CHEAT-ITEMS", "Give Items", "items", "A+i", nullptr },
	{ KEYBIND_CHEAT_KARMA, "CHEAT-KARMA", "List Karma", "karma", "A+k", nullptr },
	{ KEYBIND_CHEAT_LEAVE, "CHEAT-LEAVE", "Leave Location", "leave", "A+l", nullptr },
	{ KEYBIND_CHEAT_MIXTURES, "CHEAT-MIXTURES", "Give Mixtures", "mixtures", "A+m", nullptr },
	{ KEYBIND_CHEAT_NO_COMBAT, "CHEAT-NOCOMBAT", "Combat Encounters", "combat", "A+n", nullptr },
	{ KEYBIND_CHEAT_OVERHEAD, "CHEAT_OVERHEAD", "Toggle Overhead View", "overhead", "A+o", nullptr },
	{ KEYBIND_CHEAT_PARTY, "CHEAT-PARTY", "Full Party", "companions", "A+p", nullptr },
	{ KEYBIND_CHEAT_REAGENTS, "CHEAT-REAGENTS", "Give Reagents", "reagents", "A+r", nullptr },
	{ KEYBIND_CHEAT_STATS, "CHEAT-STATS", "Full Stats", "fullstats", "A+s", nullptr },
	{ KEYBIND_CHEAT_TRANSPORT, "CHEAT-TRANSPORT", "Create Transport", "transport", "A+t", nullptr },
	{ KEYBIND_CHEAT_UP, "CHEAT-UP", "Up Level", "up", "A+UP", nullptr },
	{ KEYBIND_CHEAT_DOWN, "CHEAT-DOWN", "Down Level", "down", "A+DOWN", nullptr },
	{ KEYBIND_CHEAT_VIRTUE, "CHEAT-VIRTUE", "Grant Virtue", "virtue", "A+v", nullptr },
 	{ KEYBIND_CHEAT_WIND, "CHEAT-WIND", "Change Wind", "wind", "A+w", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord INPUT_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", "Abort Action", "", "ESCAPE", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord DIRECTION_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", "Escape", nullptr, "ESCAPE", nullptr },
	{ KEYBIND_UP, "UP", "Up", nullptr, "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", "Down", nullptr, "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", "Left", nullptr, "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", "Right", nullptr, "RIGHT", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord MENU_KEYS[] = {
	{ KEYBIND_INTERACT, "INTERACT", "Interact", "interact", "RETURN", nullptr },
	{ KEYBIND_ESCAPE, "ESCAPE", "Escape", nullptr, "ESCAPE", nullptr },
	{ KEYBIND_UP, "UP", "Up", nullptr, "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", "Down", nullptr, "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", "Left", nullptr, "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", "Right", nullptr, "RIGHT", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord COMBAT_KEYS[] = {
	{ KEYBIND_PASS, "PASS", "Pass", "pass", "SPACE", nullptr },
	{ KEYBIND_UP, "UP", "Up", nullptr, "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", "Down", nullptr, "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", "Left", nullptr, "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", "Right", nullptr, "RIGHT", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord NORMAL_RECORDS[] = {
	{ "ultima4", "Ultima IV", NORMAL_KEYS },
	{ "ultima4_config", "Ultima IV - Configuration", CONFIG_KEYS },
	{ "ultima4_party", "Ultima IV - Party", PARTY_KEYS },
	{ "ultima4_cheats", "Ultima IV - Cheats", CHEAT_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord INPUT_RECORDS[] = {
	{ "ultima4", "Ultima IV", INPUT_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord DIRECTION_RECORDS[] = {
	{ "ultima4", "Ultima IV", DIRECTION_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MENU_RECORDS[] = {
	{ "ultima4", "Ultima IV", MENU_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord *MODE_RECORDS[5] = {
	NORMAL_RECORDS, INPUT_RECORDS, DIRECTION_RECORDS, MENU_RECORDS,
	NORMAL_RECORDS
};

Common::KeymapArray MetaEngine::initKeymaps(KeybindingMode mode) {
	Common::KeymapArray keymapArray;
	Common::Keymap *keyMap;
	Common::Action *act;
	const KeysRecord *recPtr = MODE_RECORDS[mode];

	for (int kCtr = 0; recPtr->_id; ++recPtr, ++kCtr) {
		// Core keymaps
		keyMap = new Common::Keymap(Common::Keymap::kKeymapTypeGame,
			recPtr->_id, recPtr->_desc);
		keymapArray.push_back(keyMap);

		if (kCtr == 0) {
			addMouseClickActions(*keyMap);
		}

		for (const KeybindingRecord *r = recPtr->_keys; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);
			if (r->_action == KEYBIND_UP || r->_action == KEYBIND_DOWN ||
					r->_action == KEYBIND_LEFT || r->_action == KEYBIND_RIGHT)
				// Allow movement actions to be triggered on keyboard repeats
				act->allowKbdRepeats();

			keyMap->addAction(act);
		}
	}

	return keymapArray;
}

void MetaEngine::addMouseClickActions(Common::Keymap &keyMap) {
	Common::Action *act;

	act = new Common::Action("LCLK", _("Interact via Left Click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap.addAction(act);

	act = new Common::Action("RCLK", _("Interact via Right Click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	keyMap.addAction(act);
}

void MetaEngine::setKeybindingMode(KeybindingMode mode) {
	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->cleanupGameKeymaps();

	Common::KeymapArray arr = initKeymaps(mode);

	for (uint idx = 0; idx < arr.size(); ++idx)
		mapper->addGameKeymap(arr[idx]);
}

void MetaEngine::executeAction(KeybindingAction keyAction) {
	Common::String methodName = getMethod(keyAction);
	if (!methodName.empty())
		g_debugger->executeCommand(methodName);
}

Common::String MetaEngine::getMethod(KeybindingAction keyAction) {
	for (int kCtr = 0; kCtr < 4; ++kCtr) {
		for (const KeybindingRecord *r = NORMAL_RECORDS[kCtr]._keys; r->_id; ++r) {
			if (r->_action == keyAction)
				return r->_method;
		}
	}

	return Common::String();
}

} // End of namespace Ultima8
} // End of namespace Ultima
