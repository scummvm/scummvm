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

#include "ultima/ultima0/metaengine.h"
#include "ultima/ultima0/ultima0.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace Ultima {
namespace Ultima0 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord MINIMAL_KEYS[] = {
	{ KEYBIND_ESCAPE, "ESCAPE", _s("Escape"), "ESCAPE", "JOY_Y" },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord MENU_KEYS[] = {
	{ KEYBIND_UP, "UP", _s("Up"), "UP", "JOY_UP" },
	{ KEYBIND_DOWN, "DOWN", _s("Down"), "DOWN", "JOY_DOWN" },
	{ KEYBIND_SELECT, "SELECT", _s("Select"), "SPACE", nullptr },
	{ KEYBIND_QUIT, "QUIT", _s("Quit"), "q", nullptr },
	{ KEYBIND_SWING, "SWING", _s("Swing"), "s", nullptr },
	{ KEYBIND_THROW, "THROW", _s("Throw"), "t", nullptr },
	{ KEYBIND_FOOD, "FOOD", _s("Food"), "f", nullptr },
	{ KEYBIND_RAPIER, "RAPIER", _s("Rapier"), "r", nullptr },
	{ KEYBIND_AXE, "AXE", _s("Axe"), "a", nullptr },
	{ KEYBIND_SHIELD, "SHIELD", _s("Shield"), "s", nullptr },
	{ KEYBIND_BOW, "BOW", _s("Bow & Arrow"), "b", nullptr },
	{ KEYBIND_AMULET, "AMULET", _s("Magic Amulet"), "m", nullptr },
	{ KEYBIND_AMULET1, "AMULET1", _s("Amulet Option 1"), "1", nullptr },
	{ KEYBIND_AMULET2, "AMULET2", _s("Amulet Option 2"), "2", nullptr },
	{ KEYBIND_AMULET3, "AMULET3", _s("Amulet Option 3"), "3", nullptr },
	{ KEYBIND_AMULET4, "AMULET4", _s("Amulet Option 4"), "4", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord OVERWORLD_KEYS[] = {
	{ KEYBIND_UP, "UP", _s("North"), "UP", "JOY_UP" },
	{ KEYBIND_DOWN, "DOWN", _s("South"), "DOWN", "JOY_DOWN" },
	{ KEYBIND_LEFT, "LEFT", _s("West"), "LEFT", "JOY_LEFT" },
	{ KEYBIND_RIGHT, "RIGHT", _s("East"), "RIGHT", "JOY_RIGHT" },
	{ KEYBIND_ENTER, "ENTER", _s("Enter/Exit"), "e", "JOY_B" },
	{ KEYBIND_INFO, "INFO", _s("Info"), "z", "JOY_X" },
	{ KEYBIND_QUIT, "QUIT", _s("Quit"), "q", nullptr },
	{ KEYBIND_PASS, "PASS", _s("Pass/Wait"), "SPACE", nullptr },
	{ KEYBIND_MINIMAP, "MINIMAP", _s("Minimap"), "m", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord DUNGEON_KEYS[] = {
	{ KEYBIND_UP, "UP", _s("Move Forward"), "UP", "JOY_UP" },
	{ KEYBIND_DOWN, "DOWN", _s("Turn Around"), "DOWN", "JOY_DOWN" },
	{ KEYBIND_LEFT, "LEFT", _s("Turn Left"), "LEFT", "JOY_LEFT" },
	{ KEYBIND_RIGHT, "RIGHT", _s("Turn Right"), "RIGHT", "JOY_RIGHT" },
	{ KEYBIND_ATTACK, "ATTACK", _s("Attack"), "a", "JOY_A" },
	{ KEYBIND_ENTER, "ENTER", _s("Enter/Exit"), "e", "JOY_B" },
	{ KEYBIND_INFO, "INFO", _s("Info"), "z", "JOY_X" },
	{ KEYBIND_QUIT, "QUIT", _s("Quit"), "q", nullptr },
	{ KEYBIND_PASS, "PASS", _s("Pass/Wait"), "SPACE", nullptr },
	{ KEYBIND_MINIMAP, "MINIMAP", _s("Minimap"), "m", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord ALL_RECORDS[] = {
	{ "ultima0", _s("Basic Keys"), MINIMAL_KEYS },
	{ "menu", _s("Menu Keys"), MENU_KEYS },
	{ "overworld", _s("Overworld Keys"), OVERWORLD_KEYS },
	{ "dungeon", _s("Dungeon Keys"), DUNGEON_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MINIMAL_RECORDS[] = {
	{ "ultima0", _s("Basic Keys"), MINIMAL_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MENU_RECORDS[] = {
	{ "ultima0", _s("Basic Keys"), MINIMAL_KEYS },
	{ "menu", _s("Menu Keys"), MENU_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord OVERWORLD_RECORDS[] = {
	{ "ultima0", _s("Basic Keys"), MINIMAL_KEYS },
	{ "overworld", _s("Overworld Keys"), OVERWORLD_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord DUNGEON_RECORDS[] = {
	{ "ultima0", _s("Basic Keys"), MINIMAL_KEYS },
	{ "dungeon", _s("Dungeon Keys"), DUNGEON_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord *MODE_RECORDS[] = {
	ALL_RECORDS,
	MINIMAL_RECORDS,
	MENU_RECORDS,
	OVERWORLD_RECORDS,
	DUNGEON_RECORDS,
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
			if (r->_action == KEYBIND_ENTER)
				act->addDefaultInputMapping("x");	// x also works to eXit
			else if (r->_action == KEYBIND_SELECT)
				act->addDefaultInputMapping("RETURN");
			else if (r->_action == KEYBIND_QUIT)
				act->addDefaultInputMapping("p");	// Map "Pause" to Quit

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

} // End of namespace Ultima0
} // End of namespace Ultima
