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

#include "ultima/ultima1/metaengine.h"
#include "ultima/ultima1/ultima1.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace Ultima {
namespace Ultima1 {

struct KeybindingRecord {
	KeybindingAction _action;
	const char *_id;
	const char *_desc;
	const char *_key;
	const char *_joy;
};

static const KeybindingRecord MINIMAL_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord MENU_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord OVERWORLD_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

static const KeybindingRecord DUNGEON_KEYS[] = {
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord ALL_RECORDS[] = {
	{ "Ultima1", _s("Basic keys"), MINIMAL_KEYS },
	{ "menu", _s("Menu keys"), MENU_KEYS },
	{ "overworld", _s("Overworld keys"), OVERWORLD_KEYS },
	{ "dungeon", _s("Dungeon keys"), DUNGEON_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MINIMAL_RECORDS[] = {
	{ "Ultima1", _s("Basic keys"), MINIMAL_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MENU_RECORDS[] = {
	{ "Ultima1", _s("Basic keys"), MINIMAL_KEYS },
	{ "menu", _s("Menu keys"), MENU_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord OVERWORLD_RECORDS[] = {
	{ "Ultima1", _s("Basic keys"), MINIMAL_KEYS },
	{ "overworld", _s("Overworld keys"), OVERWORLD_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord DUNGEON_RECORDS[] = {
	{ "Ultima1", _s("Basic keys"), MINIMAL_KEYS },
	{ "dungeon", _s("Dungeon keys"), DUNGEON_KEYS },
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

} // namespace Ultima1
} // namespace Ultima
