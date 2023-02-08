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

#include "mm/mm1/metaengine.h"
#include "mm/mm1/mm1.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace MM {
namespace MM1 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord MENU_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", "Escape", "ESCAPE", nullptr },
	{ KEYBIND_SELECT, "SELECT", "Select", "RETURN", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord PARTY_KEYS[] = {
	{ KEYBIND_VIEW_PARTY1, "PARTY1", "View Party Member 1", "1", nullptr },
	{ KEYBIND_VIEW_PARTY2, "PARTY2", "View Party Member 2", "2", nullptr },
	{ KEYBIND_VIEW_PARTY3, "PARTY3", "View Party Member 3", "3", nullptr },
	{ KEYBIND_VIEW_PARTY4, "PARTY4", "View Party Member 4", "4", nullptr },
	{ KEYBIND_VIEW_PARTY5, "PARTY5", "View Party Member 5", "5", nullptr },
	{ KEYBIND_VIEW_PARTY6, "PARTY6", "View Party Member 6", "6", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord COMBAT_KEYS[] = {
	{ KEYBIND_COMBAT_ATTACK, "COMBAT_ATTACK", "Attack", "a", nullptr },
	{ KEYBIND_COMBAT_BLOCK, "COMBAT_BLOCK", "Block", "b", nullptr },
	{ KEYBIND_COMBAT_CAST, "COMBAT_CAST", "Cast", "c", nullptr },
	{ KEYBIND_COMBAT_EXCHANGE, "COMBAT_EXCHANGE", "Exchange", "e", nullptr },
	{ KEYBIND_COMBAT_FIGHT, "COMBAT_FIGHT", "Fight", "f", nullptr },
	{ KEYBIND_QUICKREF, "QUICKREF", "Quick Reference", "q", nullptr },
	{ KEYBIND_COMBAT_RETREAT, "COMBAT_RETREAT", "Retreat", "r", nullptr },
	{ KEYBIND_COMBAT_SHOOT, "COMBAT_SHOOT", "Shoot", "s", nullptr },
	{ KEYBIND_COMBAT_USE, "COMBAT_USE", "Use", "u", nullptr },

	{ KEYBIND_DELAY, "DELAY", "Delay", "d", nullptr },
	{ KEYBIND_PROTECT, "PROTECT", "Protect", "p", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord NORMAL_KEYS[] = {
	{ KEYBIND_FORWARDS, "FORWARDS", "Forwards", "UP", nullptr },
	{ KEYBIND_BACKWARDS, "BACKWARDS", "Backwards", "DOWN", nullptr },
	{ KEYBIND_TURN_LEFT, "TURN_LEFT", "Turn Left", "LEFT", nullptr },
	{ KEYBIND_TURN_RIGHT, "TURN_RIGHT", "Turn Right", "RIGHT", nullptr },
	{ KEYBIND_STRAFE_LEFT, "STRAFE_LEFT", "Strafe Left", "C+LEFT", nullptr },
	{ KEYBIND_STRAFE_RIGHT, "STRAFE_RIGHT", "Strafe Right", "C+RIGHT", nullptr },

	{ KEYBIND_MAP, "MAP", "Show Map", "m", nullptr },
	{ KEYBIND_MINIMAP, "MINIMAP", "Toggle Minimap", "=", nullptr },
	{ KEYBIND_ORDER, "ORDER", "Reorder Party", "o", nullptr },
	{ KEYBIND_PROTECT, "PROTECT", "Protect", "p", nullptr },
	{ KEYBIND_REST, "REST", "Rest", "r", nullptr },
	{ KEYBIND_SEARCH, "SEARCH", "Search", "s", nullptr },
	{ KEYBIND_BASH, "BASH", "Bash", "b", nullptr },
	{ KEYBIND_UNLOCK, "UNLOCK", "Unlock", "u", nullptr },
	{ KEYBIND_QUICKREF, "QUICKREF", "Quick Reference", "q", nullptr },

	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord CHEAT_KEYS[] = {
	{ KEYBIND_CHEAT_GOTO, "CHEAT-GOTO", "Goto location", "A+g", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord MENU_RECORDS[] = {
	{ "mm1", "Might and Magic 1 - Menus", MENU_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord PARTY_MENU_RECORDS[] = {
	{ "mm1", "Might and Magic 1 - Menus", MENU_KEYS },
	{ "mm1_party", "Might and Magic 1 - Party", PARTY_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord COMBAT_MENU_RECORDS[] = {
	{ "mm1_combat", "Might and Magic 1 - Combat", COMBAT_KEYS },
	{ "mm1_party", "Might and Magic 1 - Party", PARTY_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord NORMAL_RECORDS[] = {
	{ "mm1", "Might and Magic 1", NORMAL_KEYS },
	{ "mm1_party", "Might and Magic 1 - Party", PARTY_KEYS },
	{ "mm1_cheats", "Might and Magic 1 - Cheats", CHEAT_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord *MODE_RECORDS[5] = {
	MENU_RECORDS,
	PARTY_MENU_RECORDS,
	NORMAL_RECORDS,
	COMBAT_MENU_RECORDS,
	nullptr		// TODO: combat keybindings
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

		for (const KeybindingRecord *r = recPtr->_keys; r->_id; ++r) {
			act = new Common::Action(r->_id, _(r->_desc));
			act->setCustomEngineActionEvent(r->_action);
			act->addDefaultInputMapping(r->_key);
			if (r->_joy)
				act->addDefaultInputMapping(r->_joy);

			if (r->_action == KEYBIND_FORWARDS || r->_action == KEYBIND_BACKWARDS ||
					r->_action == KEYBIND_TURN_LEFT || r->_action == KEYBIND_TURN_RIGHT ||
					r->_action == KEYBIND_STRAFE_LEFT || r->_action == KEYBIND_STRAFE_RIGHT)
				// Allow movement actions to be triggered on keyboard repeats
				act->allowKbdRepeats();

			keyMap->addAction(act);
		}
	}

	return keymapArray;
}

void MetaEngine::setKeybindingMode(KeybindingMode mode) {
	Common::Keymapper *const mapper = g_engine->getEventManager()->getKeymapper();
	mapper->cleanupGameKeymaps();

	Common::KeymapArray arr = initKeymaps(mode);

	for (uint idx = 0; idx < arr.size(); ++idx)
		mapper->addGameKeymap(arr[idx]);
}

void MetaEngine::executeAction(KeybindingAction keyAction) {
	g_engine->send(ActionMessage(keyAction));
}

} // End of namespace MM1
} // End of namespace MM
