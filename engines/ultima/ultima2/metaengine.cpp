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

#include "ultima/ultima2/metaengine.h"
#include "ultima/ultima2/ultima2.h"
#include "common/translation.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/standard-actions.h"

namespace Ultima {
namespace Ultima2 {

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

static const KeybindingRecord GAMEPLAY_KEYS[] = {
	{ KEYBIND_UP, "UP", _s("Up"), "UP", "JOY_UP"},
	{ KEYBIND_DOWN, "DOWN", _s("Down"), "DOWN", "JOY_DOWN"},
	{ KEYBIND_LEFT, "LEFT", _s("Left"), "LEFT", "JOY_LEFT"},
	{ KEYBIND_RIGHT, "RIGHT", _s("Right"), "RIGHT", "JOY_RIGHT"},
	{ KEYBIND_ATTACK, "ATTACK", _s("Attack"), "a", nullptr },
	{ KEYBIND_BOARD, "BOARD", _s("Board"), "b", nullptr },
	{ KEYBIND_CAST, "CAST", _s("Cast"), "c", nullptr },
	{ KEYBIND_DESCEND, "DESCEND", _s("Descend"), "d", nullptr },
	{ KEYBIND_ENTER, "ENTER", _s("Enter"), "e", nullptr },
	{ KEYBIND_FIRE, "FIRE", _s("Fire"), "f", nullptr },
	{ KEYBIND_GET, "GET", _s("Get"), "g", nullptr },
	{ KEYBIND_HYPER, "HYPER", _s("Hyper"), "h", nullptr },
	{ KEYBIND_IGNITE_TORCH, "IGNITE_TORCH", _s("Ignite torch"), "i", nullptr },
	{ KEYBIND_JUMP, "JUMP", _s("Jump"), "j", nullptr },
	{ KEYBIND_KLIMB, "KLIMB", _s("Klimb"), "k", nullptr },
	{ KEYBIND_LAUNCH, "LAUNCH", _s("Launch"), "l", nullptr },
	{ KEYBIND_MAGIC, "MAGIC", _s("Magic"), "m", nullptr },
	{ KEYBIND_NEGATE_TIME, "NEGATE_TIME", _s("Negate time"), "n", nullptr },
	{ KEYBIND_OFFER, "OFFER", _s("Offer"), "o", nullptr },
	{ KEYBIND_PASS, "PASS", _s("Pass"), "p", nullptr },
	{ KEYBIND_QUIT, "QUIT", _s("Quit"), "q", nullptr },
	{ KEYBIND_READY, "READY", _s("Ready"), "r", nullptr },
	{ KEYBIND_STEAL, "STEAL", _s("Steal"), "s", nullptr },
	{ KEYBIND_TRANSACT, "TRANSACT", _s("Transact"), "t", nullptr },
	{ KEYBIND_UNLOCK, "UNLOCK", _s("Unlock"), "u", nullptr },
	{ KEYBIND_VIEW, "VIEW", _s("View"), "v", nullptr },
	{ KEYBIND_WEAR_ARMOR, "WEAR_ARMOR", _s("Wear armor"), "w", nullptr },
	{ KEYBIND_X_IT, "X_IT", _s("eXit"), "x", nullptr },
	{ KEYBIND_YELL, "YELL", _s("Yell"), "y", nullptr },
	{ KEYBIND_ZSTATS, "ZSTATS", _s("Ztats"), "z", nullptr },
	{ KEYBIND_NONE, nullptr, nullptr, nullptr, nullptr }
};

struct KeysRecord {
	const char *_id;
	const char *_desc;
	const KeybindingRecord *_keys;
};

static const KeysRecord ALL_RECORDS[] = {
	{ "Ultima2", _s("Basic keys"), MINIMAL_KEYS },
	{ "gameplay", _s("Gameplay keys"), GAMEPLAY_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord MINIMAL_RECORDS[] = {
	{ "Ultima2", _s("Basic keys"), MINIMAL_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord GAMEPLAY_RECORDS[] = {
	{ "Ultima2", _s("Basic keys"), MINIMAL_KEYS },
	{ "gameplay", _s("Gameplay keys"), GAMEPLAY_KEYS },
	{ nullptr, nullptr, nullptr }
};

static const KeysRecord *MODE_RECORDS[] = {
	ALL_RECORDS,
	MINIMAL_RECORDS,
	GAMEPLAY_RECORDS
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

} // namespace Ultima2
} // namespace Ultima
