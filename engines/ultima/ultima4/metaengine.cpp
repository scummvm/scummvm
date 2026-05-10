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

#include "ultima/ultima4/metaengine.h"
#include "ultima/ultima4/core/debugger.h"
#include "ultima/ultima4/ultima4.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

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
	{ KEYBIND_INTERACT, "INTERACT", _s("Interact"), "interact", "RETURN", nullptr },
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Abort action"), "", "ESCAPE", nullptr },
	{ KEYBIND_UP, "UP", _s("Up"), "move up", "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", _s("Down"), "move down", "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", _s("Left"), "move left", "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), "move right", "RIGHT", nullptr },
	{ KEYBIND_ATTACK, "ATTACK", _s("Attack"), "attack", "a", nullptr },
	{ KEYBIND_BOARD, "BOARD", _s("Board"), "board", "b", nullptr },
	{ KEYBIND_CAST, "CAST", _s("Cast"), "cast", "c", nullptr },
	{ KEYBIND_CLIMB, "CLIMB", _s("Climb"), "climb", "k", nullptr },
	{ KEYBIND_DESCEND, "DESCEND", _s("Descend"), "descend", "d", nullptr },
	{ KEYBIND_ENTER, "ENTER", _s("Enter"), "enter", "e", nullptr },
	{ KEYBIND_EXIT, "EXIT", _s("Exit"), "exit", "x", nullptr },
	{ KEYBIND_FIRE, "FIRE", _s("Fire"), "fire", "f", nullptr },
	{ KEYBIND_GET, "GET", _s("Get chest"), "get", "g", nullptr },
	{ KEYBIND_HOLE_UP, "HOLE-UP", _s("Hole up & camp"), "camp", "h", nullptr },
	// I18N: Jimmy is used to open locked doors
	{ KEYBIND_JIMMY, "JIMMY", _s("Jimmy"), "jimmy", "j", nullptr },
	{ KEYBIND_IGNITE, "IGNITE", _s("Ignite"), "ignite", "i", nullptr },
	{ KEYBIND_LOCATE, "LOCATE", _s("Locate position"), "locate", "l", nullptr },
	{ KEYBIND_MIX, "MIX", _s("Mix reagents"), "mix", "m", nullptr },
	{ KEYBIND_NEW_ORDER, "NEW-ORDER", _s("New order"), "order", "n", nullptr },
	{ KEYBIND_OPEN_DOOR, "OPEN-DOOR", _s("Open door"), "open", "o", nullptr },
	// I18N: Going over something carefully
	{ KEYBIND_PASS, "PASS", _s("Pass"), "pass", "SPACE", nullptr },
	// I18N: Peer is used to look through keyholes or other small openings
	{ KEYBIND_PEER, "PEER", _s("Peer"), "peer", "p", nullptr },
	{ KEYBIND_QUIT_SAVE, "QUIT-SAVE", _s("Quit and save"), "quitAndSave", "q", nullptr },
	{ KEYBIND_READY_WEAPON, "READY-WEAPON", _s("Ready weapon"), "ready", "r", nullptr },
	{ KEYBIND_SEARCH, "SEARCH", _s("Search"), "search", "s", nullptr },
	{ KEYBIND_STATS, "STATS", _s("Stats"), "stats", "z", nullptr },
	{ KEYBIND_TALK, "TALK", _s("Talk"), "talk", "t", nullptr },
	{ KEYBIND_USE, "USE", _s("Use"), "use", "u", nullptr },
	{ KEYBIND_WEAR, "WEAR", _s("Wear armor"), "wear", "w", nullptr },
	{ KEYBIND_YELL, "YELL", _s("Yell"), "yell", "y", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CONFIG_KEYS[] = {
	{ KEYBIND_SPEED_UP, "SPEED-UP", _s("Speed up"), "speed up", "KP_PLUS", nullptr },
	{ KEYBIND_SPEED_DOWN, "SPEED-DOWN", _s("Speed down"), "speed down", "KP_MINUS", nullptr },
	{ KEYBIND_SPEED_NORMAL, "SPEED-NORMAL", _s("Speed normal"), "speed normal", "KP_ENTER", nullptr },
	{ KEYBIND_COMBATSPEED_UP, "COMBATSPEED-UP", _s("Combat speed up"), "combat_speed up", "A+KP_PLUS", nullptr },
	{ KEYBIND_COMBATSPEED_DOWN, "COMBATSPEED-DOWN", _s("Combat speed down"), "combat_speed down", "A+KP_MINUS", nullptr },
	{ KEYBIND_COMBATSPEED_NORMAL, "COMBATSPEED-NORMAL", _s("Combat speed normal"), "combat_speed normal", "A+KP_ENTER", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord PARTY_KEYS[] = {
	{ KEYBIND_PARTY0, "PARTY0", _s("Party - None"), "party 0", "0", nullptr },
	{ KEYBIND_PARTY1, "PARTY1", _s("Party - Character #1"), "party 1", "1", nullptr },
	{ KEYBIND_PARTY2, "PARTY2", _s("Party - Character #2"), "party 2", "2", nullptr },
	{ KEYBIND_PARTY3, "PARTY3", _s("Party - Character #3"), "party 3", "3", nullptr },
	{ KEYBIND_PARTY4, "PARTY4", _s("Party - Character #4"), "party 4", "4", nullptr },
	{ KEYBIND_PARTY5, "PARTY5", _s("Party - Character #5"), "party 5", "5", nullptr },
	{ KEYBIND_PARTY6, "PARTY6", _s("Party - Character #6"), "party 6", "6", nullptr },
	{ KEYBIND_PARTY7, "PARTY7", _s("Party - Character #7"), "party 7", "7", nullptr },
	{ KEYBIND_PARTY8, "PARTY8", _s("Party - Character #8"), "party 8", "8", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ KEYBIND_CHEAT_DESTROY_CREATURES, "CHEAT-DESTROY_CREATURES", _s("Destroy all creatures"), "destroy_creatures", "A+a", nullptr },
	{ KEYBIND_CHEAT_COLLISIONS, "CHEAT-COLLISIONS", _s("Toggle collision handling"), "collisions", "A+c", nullptr },
	{ KEYBIND_CHEAT_DESTROY, "CHEAT-DESTROY", _s("Destroy object"), "destroy", "A+d", nullptr },
	{ KEYBIND_CHEAT_EQUIPMENT, "CHEAT-EQUIPMENT", _s("Full equipment"), "equipment", "A+e", nullptr },
	{ KEYBIND_CHEAT_FLEE, "CHEAT_FLEE", _s("Flee combat"), "flee", "A+f", nullptr },
	{ KEYBIND_CHEAT_GOTO, "CHEAT-GOTO", _s("Goto location"), "goto", "A+g", nullptr },
	{ KEYBIND_CHEAT_HELP, "CHEAT-HELP", _s("Help - Teleport to Lord British"), "goto", "A+h", nullptr },
	{ KEYBIND_CHEAT_ITEMS, "CHEAT-ITEMS", _s("Give items"), "items", "A+i", nullptr },
	{ KEYBIND_CHEAT_KARMA, "CHEAT-KARMA", _s("List karma"), "karma", "A+k", nullptr },
	{ KEYBIND_CHEAT_LEAVE, "CHEAT-LEAVE", _s("Leave location"), "leave", "A+l", nullptr },
	{ KEYBIND_CHEAT_MIXTURES, "CHEAT-MIXTURES", _s("Give mixtures"), "mixtures", "A+m", nullptr },
	{ KEYBIND_CHEAT_NO_COMBAT, "CHEAT-NOCOMBAT", _s("Combat encounters"), "combat", "A+n", nullptr },
	{ KEYBIND_CHEAT_OVERHEAD, "CHEAT_OVERHEAD", _s("Toggle overhead view"), "overhead", "A+o", nullptr },
	{ KEYBIND_CHEAT_PARTY, "CHEAT-PARTY", _s("Full party"), "companions", "A+p", nullptr },
	{ KEYBIND_CHEAT_REAGENTS, "CHEAT-REAGENTS", _s("Give reagents"), "reagents", "A+r", nullptr },
	{ KEYBIND_CHEAT_STATS, "CHEAT-STATS", _s("Full stats"), "fullstats", "A+s", nullptr },
	{ KEYBIND_CHEAT_TRANSPORT, "CHEAT-TRANSPORT", _s("Create transport"), "transport", "A+t", nullptr },
	{ KEYBIND_CHEAT_UP, "CHEAT-UP", _s("Up level"), "up", "A+UP", nullptr },
	{ KEYBIND_CHEAT_DOWN, "CHEAT-DOWN", _s("Down level"), "down", "A+DOWN", nullptr },
	{ KEYBIND_CHEAT_VIRTUE, "CHEAT-VIRTUE", _s("Grant virtue"), "virtue", "A+v", nullptr },
 	{ KEYBIND_CHEAT_WIND, "CHEAT-WIND", _s("Change wind"), "wind", "A+w", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord INPUT_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Abort action"), "", "ESCAPE", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord DIRECTION_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Abort action"), nullptr, "ESCAPE", nullptr },
	{ KEYBIND_UP, "UP", _s("Up"), nullptr, "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", _s("Down"), nullptr, "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", _s("Left"), nullptr, "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), nullptr, "RIGHT", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord MENU_KEYS[] = {
	{ KEYBIND_INTERACT, "INTERACT", _s("Interact"), "interact", "RETURN", nullptr },
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Abort action"), nullptr, "ESCAPE", nullptr },
	{ KEYBIND_UP, "UP", _s("Up"), nullptr, "UP", nullptr },
	{ KEYBIND_DOWN, "DOWN", _s("Down"), nullptr, "DOWN", nullptr },
	{ KEYBIND_LEFT, "LEFT", _s("Left"), nullptr, "LEFT", nullptr },
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), nullptr, "RIGHT", nullptr },
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

	act = new Common::Action(Common::kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	keyMap.addAction(act);

	act = new Common::Action(Common::kStandardActionRightClick, _("Right click"));
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
